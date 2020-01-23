/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 28/03/01
 * Fichier	: lcr_tc_e.c
 * Objet	: Traitement de la commande TC_E
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Apr01,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "standard.h"
#include "define.h"
#include "x01_str.h"
#include "vct_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "ted_prot.h"
#include "lcr_tc_e.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define TC_E_CMD				"TC E"

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
 * lcr_tc_e
 * =============
 * Analyse de la commande "TC E"
 * --------------------------------	*/

int lcr_tc_e(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin,
		INT * bloc)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int flag_err = 0;
	int modifConfig = 0;

	ptCour += strlen(TC_E_CMD);
	lgCour -= strlen(TC_E_CMD);
	flag_err = 4;
	if (0 != flag_err)
	{
		tedi_ctrl_ext(las, mode, flag_err);
		modifConfig = 0;
	} /* endif(0==flag_err)                                   */
	/* ------------------------------------
	 * FIN DE lcr_tc_e
	 * ------------------------------------ */
	return modifConfig;
}
