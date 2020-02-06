/*
 * SupInit.c
 * ===================
 *
 * Objet :
 * 	Le ficheir
 *
 *  Created on: 30 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: SupInit.c,v $
 * Revision 1.3  2018/06/14 08:36:49  xg
 * Suppression des references a TPE
 *
 * Revision 1.2  2018/06/04 08:38:36  xg
 * Passage en version V122
 *
 * Revision 1.7  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.6  2008/10/03 16:21:46  xag
 * *** empty log message ***
 *
 * Revision 1.5  2008/09/29 07:58:51  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.4  2008/09/22 13:39:02  xag
 * Ajout du parametre performance pour l'impression par defaut des traces de performance.
 *
 * Revision 1.3  2008/09/15 13:34:56  xag
 * Correction d'une anomalie.
 *
 * Revision 1.2  2008/09/12 14:57:10  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */


/* *******************************
 * LISTE DES INCLUDES
 * ******************************* */

#include <pipe.h>
#include <superviseur.h>
#include "semaphore.h"
#include "standard.h"
#include "mon_inc.h"
#include "define.h"
#include "x01_str.h"
#include "x01_var.h"
#include "xdg_var.h"
#include "mon_ext.h"
#include "rec_main.h"
#include "mon_debug.h"
#include "mon_str.h"
#include "mon_pro.h"
#include <termios.h>
#include <unistd.h>
#include "fcntl.h"
#include "supInitLib.h"
#include "ypc_las.h"
#include "configuration.h"

/* *******************************
 * DECLARATION DES VARIABLES GLOBALES
 * ******************************* */

T_supGestion supGestion;

#define DEBUG 1

/* Afin de supprimer les directives
 * de compilation qui sillonnent le
 * code, petite astuce du pere
 * Gaillard.
 */
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* Fonction définie dans LCR_TST.c */

void lcr_cmd_tst_print_version(void);

/* *******************************
 * FONCTIONS DU MODULE
 * ******************************* */

/*
 * supInitTaskId
 * =============
 * La fonction permet de positionner l'id de la tache a partir du numero de
 * tache....
 * C'est la premiere fonction a appeler dans une tache cela pourrait d'ailleurs
 * etre encapsule.
 * */

void supInitTaskId(int numTache)
{
	if ((numTache >= 0) && (numTache < supGestion.nbTache))
	{
		T_supDescTache *tache = &supGestion.Tache[numTache];
		kthreadIsStopped(tache->tid);
	}
}

/*
 * supInitTreadId
 * ==============
 * Permet d'affecter l'identifiant de thread a la tache.
 */
void supInitThreadId(int numTache, TASK_ID threadPt)
{
	if ((numTache >= 0) && (numTache < supGestion.nbTache))
	{
		tacheSetId(&supGestion.Tache[numTache], threadPt);
	}
}

void *supInitGetConnLuminance(void)
{
T_xdg_las *las_pt = &xdg_las[PORT_LUMINANCE];
	return las_pt->conn_pt;
}

static void _tac_ip_demarre(int port,bool serveur,int attente)
{
	T_xdg_cf_las *cfgLas_pt=&xdg_cf_las[port];
	T_xdg_las *las_pt = &xdg_las[port];
	CommCnxParam *param_pt = commCnxTcpParamNew(port, "localhost", cfgLas_pt->numPortIp, serveur);
	if (NULL != param_pt)
	{
		commCnxTcpParamSetAttente(param_pt,attente);
		commCnxParamSetInfo( param_pt,"permanent","1");
		las_pt->conn_pt = commCnxNewSimple(param_pt);
		commCnxParamDelete(param_pt);
		if(NULL!=las_pt->conn_pt) {
			printDebug("Tache IP demarree port %d\n",cfgLas_pt->numPortIp);
		}

	}
	cfgLas_pt->type_e=LIAISON_IP;
	cnxLiaison[port] = las_pt->conn_pt;
}

static void _tac_pass_demarre(int port)
{
	T_xdg_cf_las *cfgLas_pt=&xdg_cf_las[port];
	cfgLas_pt->type_e=LIAISON_PASS;
}

/*
 * supInit
 * =======
 * Fonction d'initialisation des superviseurs avec
 * lancement de toutes les taches.
 * La fonction est utilisee par la tache superviseur.
 */
int supInit(T_supGestion * pGes, int typeBoot,bool perf)
{ /* --------------------------------     */
	int retour = ERROR;
	/* PHASE D'INITIALISATION DES VAR       */
	/* --------------------------------     */
	if (supInitVar(pGes) != ERROR)
	{
		monDebugSet(MON_DEBUG_PERF_SUP,(perf?1:0));
		/* --------------------------------
		 * INITIALISATION DES ANCRAGES
		 * -------------------------------- */
		if (supInitAncrage() != ERROR)
		{
			/* --------------------------------
			 * APPEL DES FONCTIONS D'INITIALISATION DES MODULES.
			 * -------------------------------- */
			/* - initialisation de la tache de controle notamment  */
			mnTraitInitialisation();
			/* - initialisation pour le traitement luminance. */
			DialogueLuminanceInit();
			/* --------------------------------
			 * INITIALISATION DES LIAISON
			 * -------------------------------- */
			tac_las_init();
			/* --------------------------------
			 * OUVERTURE DES PORTS TEDI
			 * -------------------------------- */
			tac_las_demarre(PORT_SERIE);

			/* --------------------------------
			 * OUVERTURE DU PORT TPE
			 * -------------------------------- */

			/* --------------------------------
			 * OUVERTURE DU PORT LUMINANCE
			 * -------------------------------- */
#ifdef CPU432
			tac_las_demarre(PORT_LUMINANCE);
#else

			{
				T_xdg_las *las_pt = &xdg_las[PORT_LUMINANCE];
				CommCnxParam *param_pt = commCnxTcpParamNew(PORT_LUMINANCE, "localhost", 2701, false);
				if (NULL != param_pt)
				{
					commCnxTcpParamSetAttente(param_pt,5000);
					commCnxParamSetInfo( param_pt,"permanent","1");
					las_pt->conn_pt = commCnxNewSimple(param_pt);
					commCnxParamDelete(param_pt);
				}
				cnxLiaison[PORT_LUMINANCE] = las_pt->conn_pt;
			}
#endif
			/* --------------------------------
			 * OUVERTURE DU SECOND PORT (IP)
			 * -------------------------------- */
//			tac_las_demarre(PORT_SERIE_2);

			_tac_ip_demarre(PORT_IP_LCR,true,5000);

			xdg_cf_las[PORT_IP_LCR_2].numPortIp=configGetNumPortIp();
			_tac_ip_demarre(PORT_IP_LCR_2,true,5000);

			_tac_pass_demarre(PORT_PASS);

			/* --------------------------------
			 * CREATION DES TACHES APPLICATIVES
			 * --------------------------------     */
			if (supInitAppli(pGes, typeBoot) != ERROR)
			{
				/* --------------------------------     */
				/* DIALOGUES LCR                                        */
				/* --------------------------------     */
				tacheCreerBis(&pGes->Tache[pGes->nbTache++], (FUNCPTR) DialogueLCR, "DialogueLCR", PORT_SERIE);
				/* --------------------------------     */
				/* DIALOGUES LCR 2                      */
				/* --------------------------------     */
				// tacheCreerBis(&pGes->Tache[pGes->nbTache++], (FUNCPTR) DialogueLCR, "DialogueBis", PORT_SERIE_2);
				/* --------------------------------     */
				/* DIALOGUES LCR IP                      */
				/* --------------------------------     */
				tacheCreerBis(&pGes->Tache[pGes->nbTache++], (FUNCPTR) DialogueLCR, "DialogueIp", PORT_IP_LCR);

				tacheCreerBis(&pGes->Tache[pGes->nbTache++], (FUNCPTR) DialogueLCR, "DialogueIp2", PORT_IP_LCR_2);
				/* --------------------------------
				 * DIALOGUES FIP
				 * --------------------------------     */
#ifndef SANS_WORLDFIP
				tacheCreerBis(&pGes->Tache[pGes->nbTache++], (FUNCPTR) DialogueFip, "DialogueFIP", 0);
#endif

				tacheCreerBis(&pGes->Tache[pGes->nbTache++], (FUNCPTR) DialoguePass, "DialoguePASS", 0);

				/* --------------------------------     */
				/* DIALOGUE LUMINANCE                   */
				/* --------------------------------     */
				tacheCreerBis(&pGes->Tache[pGes->nbTache++], (FUNCPTR) DialogueLuminance, "DialogueLuminance", PORT_LUMINANCE);
				/* --------------------------------
				 * DIALOGUES FIP
				 * --------------------------------     */
				tacheCreerBis(&pGes->Tache[pGes->nbTache++], (FUNCPTR) supIncrement, "supIncrement", 0);
				retour = OK;
			} else
			{
				printDebug("Erreur e l'initialisation de l'application\n");
			}
		} else
		{
			printDebug("Erreur e l'initialisation des ancrages\n");
		}
	} else
	{
		printDebug("Erreur e l'initialisation des variables\n");
	}
	/* -------------------------------- */
	lcr_cmd_tst_print_version();
	/* --------------------------------
	 * FIN DE SupInit
	 * --------------------------------     */

	return retour;
}

/* *********************************
 * FIN DE SupInit.c
 * ********************************* */

