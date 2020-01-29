/****************************************************************************/
/* 		FICHIER: %M%      REL: %I% DATE: %G%                           		*/
/*DOC************************************************************************/
/*  					    SIAT                                			*/
/*			8, parc technologique de la Sainte Victoire             		*/
/*					Le Cannet                               				*/
/*				13590 Meyreuil - FRANCE                         			*/
/*				  Tel. 04 42 58 63 71                           			*/
/****************************************************************************/
/*  Fichier : EtatSysteme.c                                                 */
/*  MODULE  : EtatSysteme                                                   */
/****************************************************************************/
/* Auteur :   Herve ZEITTOUN                                                */
/* Date de creation : 12 avril 2000                                         */
/****************************************************************************/
/* OBJET DU MODULE      :                                                   */
/*                                                                          */
/************************************************************************DOC*/
/* Liste des fonctions du fichier :                                         */
/****************************************************************************/
/*                           Modifications                                  */
/*                      ***********************                             */
/* Auteur :                                                                 */
/* Date de la modification :                                                */
/* Description :                                                            */
/*                      ***********************                             */
/****************************************************************************/

/* ********************************	*/
/* FICHIERS D'INCLUDE           	*/
/* ********************************	*/

#include "standard.h"
#include "Tache.h"
#include "Superviseur.h"
#include "mon_debug.h"
#include "mon_ext.h"
#include "limits.h"
#include "etaSyst.h"
#include "watchdog/watchdog.h"

/* ********************************	*/
/* DECLARATION DES CONSTANTES		*/
/* ********************************	*/

//#define CPU_WD_PORT		8001	/* Numéro de port IP utilise pour la communication entre appli et wd */
#define ETA_SYST_MAX	15000

/* ********************************	*/
/* DECLARATION VARIABLES LOCALES	*/
/* ********************************	*/


#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* defines */

#define SECONDS (1)
#define DELAY SECONDS/4
#define DELAI_RAF 250


/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* *********************************
 * FONCTONS DU MODULE
 * ********************************	*/

/* ---------------------------------
 * etaSystTaskPresence
 * ===================
 * Fonction de test de presence des
 * taches de l'application.
 * --------------------------------- */

static bool _etaSystTaskPresence (int *numTache)
{
bool retour_b=true;
int indice;
bool appli_b=(1==supGestion.appli);
  for (indice = 0; (indice < supGestion.nbTache) && (retour_b == true); indice++)
    {
	 T_supDescTache *tache_pt=&supGestion.Tache[indice];
		 if((0==tache_pt-> sequence)||appli_b)
		 {
			if(ktickGetDeltaCurrent(tache_pt->lastTick_dw)>ETA_SYST_MAX)
			{
				*numTache=indice;
				retour_b=false;
			}
		 }
    }
  /* -------------------------------
   * FIN DE etaSystTaskPresence
   * --------------------------------     */
  return retour_b;
}


/*DOC************************************************************************/
/*  FICHE DE  FONCTION  :                                            		*/
/****************************************************************************/
/* OBJET DE LA FONCTION :                                                   */
/*			Tache de supervision de l'etat du systeme                       */
/* PARAMETRES           :        Aucun                                      */
/* VALEURS RENDUES      :        Aucun                                      */
/* FONCTIONS APPELEES   :        Aucune                                     */
/************************************************************************DOC*/

void etaSyst (T_supGestion * pGes)
{
  Watchdog watchdog_t;
  bool notFin=true;
  int cpt=0;

  /* ***********************************
   * INITIALISATION DU WATCHDOG
   * ************************************ */

  watchdogStart(&watchdog_t,1) ; //,CPU_WD_PORT);

  /* Pendant la phase d'initialisation, on limite la portee
   * du watchdog. */

  for(cpt=0;cpt<6;cpt++)
  {
   	watchdogSignal(&watchdog_t);
  	ksleep(5000);
  }

  /* --------------------------------     */
  /* TOUT EST OK                                          */
  /* --------------------------------     */
  printDebug ("EtatSysteme : Init OK\n\n");
  /* Ma question est pourquoi ne sort-on
   * pas de la tache, on n'a plus rien
   * a faire.
   */
  while(notFin)
  {

    int numTache;
    bool correct=true;
    /* Test des taches */
    correct=_etaSystTaskPresence(&numTache);
    /* Si les taches sont OK, on signale le watchdog. */
    if(correct)
    {
    	watchdogSignal(&watchdog_t);
    }
    else
    {
    	printDebug("_etatSyst: Probleme de rafraichissement de tache %s (delta %ld)\n",
    		supGestion.Tache[numTache].nomTache,
    		ktickGetDeltaCurrent(supGestion.Tache[numTache].lastTick_dw) );
    }

    /* Attente une seconde. */
    ksleep (1000);
  }

  watchdogStop(&watchdog_t);

  /* --------------------------------     */
  /* FIN DE etaSyst                       */
  /* --------------------------------     */
}

/* -----------------------------------
 * etaSystIncrCompteur
 * ===================
 * Increment du compteur associe e
 * une tache.
 * --------------------------------	*/


void etaSystIncrCompteur (int numero)
{
  /* Incrementer ici le compteur associe e la tache dont le numero est
   * passe en parametre.  */
	if(numero<supGestion.nbTache)
	{
	T_supDescTache *tache_pt=&supGestion.Tache[numero];
		tache_pt->bidon++;
		tache_pt->lastTick_dw=ktickGetCurrent();
	}
}

//void etaSystImprCompteur ()
//{
//int numero=0;
//  /* Incrementer ici le compteur associe e la tache dont le numero est
//   * passe en parametre.  */
//	for(;numero<supGestion.nbTache;numero++)
//	{
//		printf("Tache %s : cpt :%d\n",
//						supGestion.Tache[numero].nomTache,
//						supGestion.Tache[numero].bidon);
//	}
//}

void etaSystSetPos(int numero,int cpt)
{
	if(numero<supGestion.nbTache)
	{
		supGestion.Tache[numero].position=cpt;
	}
}

