/*
 * lcr_tst_cnf.c
 * ===================
 * Objet	: Analyse et traitement de la commande de positionnement des
 * 			seuils de donnees aberrantes et autres informations.
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_tst_cnf.c,v $
 * Revision 1.2  2018/06/04 08:38:38  xg
 * Passage en version V122
 *
 * Revision 1.2  2008/07/04 17:05:06  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:06:03  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "portage.h"
#include "define.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "abeLib.h"
#include "vct_str.h"
#include "mon_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_tst.h"
#include "ted_prot.h"
#include "tac_conf.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * lcr_tst_cnf
 * ===========
 * Traitement de la configuration des
 * donnees aberrantes.
 * --------------------------------	*/

void lcr_tst_cnf(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int nbCar = 0;
	int bloc = 0;
	int erreur = 0;
	ptCour += strlen("TST CNF");
	lgCour -= strlen("TST CNF");
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);

	if (lgCour > 0)
	{
		erreur = 2;
	} /* endif(trouve                                         */
	if (0 == erreur)
	{
		tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess, "TST CNF=%04X", tac_conf_cfg_get_cnf());
	} else
	{
		tedi_ctrl_ext(las, mode, erreur);
	} /* endif(0!=erreur                                      */
	/* --------------------------------
	 * FIN DE lcr_tst_cnf
	 * --------------------------------     */
}
