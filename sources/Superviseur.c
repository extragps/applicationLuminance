/*
 * Superviseur.c
 * ===================
 *
 *  Objet : Le module contient la procedure de lancement principale
 *
 *  Created on: 23 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: Superviseur.c,v $
 * Revision 1.3  2018/06/14 08:36:22  xg
 * Suppression des references a TPE
 *
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.8  2008/10/13 08:18:14  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.7  2008/10/03 16:21:46  xag
 * *** empty log message ***
 *
 * Revision 1.6  2008/09/29 07:58:52  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.5  2008/09/22 13:38:51  xag
 * Ajout d'un parametre a supInit pour les traces des performances.
 *
 * Revision 1.4  2008/09/22 07:53:42  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/09/12 14:57:09  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	 */

#include <stdlib.h>
#include <time.h>
#include "Superviseur.h"
#include "mon_def.h"
#include "standard.h"
#include "mon_debug.h"
#include "define.h"                    /* definition des constantes                     */
#include "x01_str.h"                   /* definition des structures globales            */
#include "x01_var.h"                   /* definition des variables globales             */
#include "xdg_var.h"
#include "tac_conf.h"
#include "etaSyst.h"
#include "lcr_p.h"
#include "tac_conf.h"
#include "mgpLib.h"
#include "io/iolib.h"

/* Ce qui suit est vraiment trop sale... */
/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	 */

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif
/* ********************************
 * PROTOTYPE DES FONCTIONS UTILISES LOCALEMENT
 * ********************************	 */

/* TODO: La fonction est en passe d'être supprimee */

void hdl_crt_init(void);

/*DOC****************************************************************************/
/*  FICHE DE  FONCTION  :   Superviseur                                         */
/********************************************************************************/
/* OBJET DE LA FONCTION :                                                       */
/* PARAMETRES           :                                                       */
/* VALEURS RENDUES      :                                                       */
/* FONCTIONS APPELEES   :                                                       */
/****************************************************************************DOC*/

void *superviseurMain(int typeBoot,bool perf)
{
	int Status;
	/* Controle de presence des cartes dans le rack */
	if(false==mgpInit())
	{
		printDebug("Probleme d'ouverture de la librairie mgp... La carte est elle presente?");
	}
	/* Suite : lecture des switch pour regarder si on
	 * doit effectuer un demarrage en mode systeme ou en mode application.
	 * */
	if ((typeBoot%2) == 0)
	{
		if(false==ioIsAppli())
		{
			printf("Demarrage systeme\n");
		}
		else
		{
			typeBoot++;
		}
	}
	/* Adaptation de la configuration
	 * suivant le type de carte utili-
	 * see. Cette partie pourra eventu-
	 * ellement disparaitre quand les
	 * premieres cartes ne seront plus
	 * utilisees.   */
	switch (typeBoot)
	{
	case 0:
		/* Dans ce mode, il y a bloquage de l'affichage... */
		pipSetEtat(PIP_VEILLE);
	case 1:
		xdg_las[PORT_SERIE_2].nomDev = "/dev/ttyM1";
#ifdef CPU432
		xdg_las[PORT_SERIE].nomDev = "/dev/ttyM2";
		xdg_las[PORT_LUMINANCE].nomDev  = "/dev/ttyN0";
#else
		xdg_las[PORT_SERIE].nomDev = "/dev/ttyS0";
		xdg_las[PORT_LUMINANCE].nomDev 	= "/dev/ttyM3";
#endif
		break;
	case 2:
	case 3:
		xdg_las[PORT_SERIE].nomDev = "/tyCo/5";
#ifdef PMV
		/* Dans le cadre des tests, on realise une sortie sur le port serie
		 * si on est dans le mode MOD=1. */
		xdg_las[PORT_LUMINANCE].nomDev = "/tyCo/4";
#endif
		break;
	} /* endswitch(typeBoot) */

	/* --------------------------------  */
	/* INITIALISATION                    */
	/* --------------------------------  */
	Status = supInit(&supGestion, typeBoot % 2,perf);
	if (Status == ERROR)
	{
		exit(-1);
	}
	/* --------------------------------  */
	/* PHASE DE FONCTIONNEMENT           */
	/* --------------------------------  */
	Status = supRun(&supGestion, typeBoot % 2);
	if (Status == ERROR)
	{
		exit(-1);
	}
	/* ---------------------------------
	 * FIN DE Superviseur
	 * --------------------------------     */
	return OK;
}

/* *********************************
 * FIN DE Superviseur.c
 * ********************************* */
