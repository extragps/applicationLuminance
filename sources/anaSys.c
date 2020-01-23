/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/11/00
 * Fichier	: anaSys.c
 * Objet	: Preanalyseur syntaxique pour distinguer les nouvelles commandes
 * 		comme EXPTACHES, INIT et VIDE.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,24Nov00,xag  creation.
 * 01b,19Feb01,xag  Remplacement des fonctions locales par les fonctions
 * 					du module utilitaire.
 * 01c,22Mar01,xag  Analyse correcte de la commande VIDE...
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include "portage.h"
#include "stdio.h"
#include "standard.h"
#include "define.h"
#include "mon_inc.h"
#include "lcr_util.h"
#include "anaSys.h"
#include "supRun.h"
#include "teleLib.h"
#include "tac_conf.h"
#include "mon_def.h"
#include "extLib.h"
#include "supRun.h"
#include "tac_conf.h"
#include "lcr_util.h"
#include "etaSyst.h"
#include "Superviseur.h"
#include "io/iolib.h"

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif
/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * anaSysListeTache
 * ================
 * La methode retourne la liste
 * des taches en fonctionnement
 * --------------------------------	*/

static void anaSysListeTache(T_lcr_util_buffer * reponse)
{
	int idCour;
	/* Il s'agit de lister ici les taches presentes dans le systeme
	 * et leur etat. La syntaxe est reprise de ce qui a ete propose pour
	 * le PMV.      */

	printDebug("Liste des taches\n");
	for (idCour = 0; idCour < supGestion.nbTache; idCour++)
	{
		char temp[100];
		T_supDescTache *tache = &supGestion.Tache[idCour];
		if((1==supRunLireEtat())||(0==tache->sequence))
		{
		int32 diffMs_udw=ktickGetDeltaCurrent(tache->lastTick_dw);
			sprintf(temp, "PID=%2lu NOM=%20s ETAT=%s\n\r",
			 kthreadGetIdThread(tache->tid), tache->nomTache,(diffMs_udw<10000?"EXE":"NOK"));
			lcr_util_buffer_recopier(reponse, temp, strlen(temp));
			/* Trace de l'etat de la tache... */
			printDebug("PID=%2lu NOM=%20s ETAT=%s(%ld)\n",
			 kthreadGetIdThread(tache->tid), tache->nomTache,(diffMs_udw<10000?"EXE":"NOK"),diffMs_udw);
		}
	} /* endfor(idCour                                        */
	/* --------------------------------
	 * FIN DE anaSysListeTache
	 * --------------------------------     */
}

/* --------------------------------
 * anaSys
 * ======
 * Fonction d'analyse des commandes
 * systeme.
 * -------------------------------- */

int anaSys(int las, char *message, int longueur, T_lcr_util_buffer * reponse)
{
	int reconnue = 0;
	if ((0 == longueur) || (CR == message[0]))
	{
		if (extGet(EXT_ECE))
		{
			if (1 == supRunLireEtat())
			{
				lcr_util_buffer_recopier(reponse, "%1", strlen("%1"));
			}
		} else
		{
			lcr_util_buffer_recopier(reponse, "\\", strlen("\\"));
			/* lcr_util_buffer_recopier(reponse,"?",strlen("?")); */
		}
		reconnue = 1;
	} else if (strncmp(message, ANA_SYS_EXPTACHES, strlen(ANA_SYS_EXPTACHES)) == 0)
	{
		char *ptCour = message + strlen(ANA_SYS_EXPTACHES);
		int lgCour = longueur - strlen(ANA_SYS_EXPTACHES);
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		if (lgCour > 0)
		{
			switch (ptCour[0])
			{
			case 'S':
				printf("Lancement des taches\n");
				ksleep(500);
				if (1==supRunLireEtat())
				{
					lcr_util_buffer_recopier(reponse, "EXPTACHES S ST=0001", strlen("EXPTACHES S ST=0001"));
					printf("C'est deja l'appli\n");
				} else
				{
					supRunDemarrer();
					lcr_util_buffer_recopier(reponse, "!", 1);
					tac_ctrl_set_force(TRUE);
				} /* endif(etaSystTesterAppli             */
				break;
			case 'Z':
				if (1==supRunLireEtat())
				{
					/* Dans le cas de l'application
					 * on relance le noyau de base.
					 */
					supRunArreter();
					lcr_util_buffer_recopier(reponse, "!", 1);
					tac_ctrl_set_force(TRUE);
#ifndef CPU432
					/* Pour realiser les tests de perte de memoire. On se concentre sur les defaut de
					 * desallocation memoire. */
					kmmEndDefault();
					exit(0);
#endif
				} else
				{
					lcr_util_buffer_recopier(reponse, "EXPTACHES Z ST=0002", strlen("EXPTACHES Z ST=0002"));
					/* TODO : Expliquer a quoi correspond au juste ce traitement??? */

					if (!ioIsAppli())
					{
						/* Cas special de la recette.   */
						sysToMonitor(2);
					}

					printf("C'est deja le noyau de base\n");
				}
				break;
			case '\r':
			case '\n':
				anaSysListeTache(reponse);
				break;
			default:
				if (extGet(EXT_ECE))
				{
					lcr_util_buffer_recopier(reponse, "%2", strlen("%2"));
				} else
				{
					lcr_util_buffer_recopier(reponse, "?", strlen("?"));
				}
				break;
			}
		} else
		{
			anaSysListeTache(reponse);
		} /* endif(l >0                                           */
		reconnue = 1;
	} else if (strncmp(message, ANA_SYS_EXPTACHEC, strlen(ANA_SYS_EXPTACHEC)) == 0)
	{
		/* Traitement special. Si il s'agit
		 * d'une commande d'abonnement, la
		 * reponse doit se faire sur MCS.
		 */
		int retour;
		/* --------------------------------
		 * TRAITEMENT TELECHARGEMENT
		 * --------------------------------     */
		retour = teleAnalyser(las, message, longueur, reponse);
		reconnue = 1 + retour;
		printDebug("Sortie de anaSys avec reconnue %d\n",reconnue);
	} else if (strncmp(message, ANA_SYS_VIDE, strlen(ANA_SYS_VIDE)) == 0)
	{
		lcr_util_buffer_recopier(reponse, "!", 1);
		reconnue = 3;
	}
	return (reconnue);
}
