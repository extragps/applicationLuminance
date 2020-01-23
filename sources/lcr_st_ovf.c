/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 11/06/01
 * Fichier	: lcr_st_ovf.c
 * Objet	: Commande de lecture de la version logicielle;
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,11Jun01,xag  creation.
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
#include "lcr_st_divers.h"

/* Nouvelles declation pour la gestion de la configuration */
#include "configuration.h"
#include "configurationSram.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define ST_OVF_CMD				"ST OVF"

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
 * lcr_st_ovf
 * =============
 * Analyse de la commande "ST OVF"
 * --------------------------------	*/

int lcr_st_ovf(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin,
		INT * bloc)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int modifConfig = 0;
	int nbCar = 0;

	ptCour += strlen(ST_OVF_CMD);
	lgCour -= strlen(ST_OVF_CMD);
	tst_send_bloc(las, mode, buffer, &nbCar, bloc, TRUE, pt_mess, "STATUS OVF=%03lu", configGetOvf());
	/* ------------------------------------
	 * FIN DE lcr_st_ovf
	 * ------------------------------------ */
	return modifConfig;
}
