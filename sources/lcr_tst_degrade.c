/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: lcr_tst_sc.c
 * Objet	: Analyse et traitement de la commande TST,SC pour le traitement
 * 		des extensions des codes d'erreur.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "portage.h"
#include "define.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "lcr_tst_degrade.h"
#include "extLib.h"
#include "ted_prot.h"
#include "configuration.h"
#include "lcr_pcp_cmd.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define LCR_TST_DEGRADE_NON "N"
#define LCR_TST_DEGRADE_OUI "O"
#define LCR_TST_DEGRADE_FORCE "F"

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

static uint8 _lcr_tst_degrade_get_code(uint16 degrade) {
	uint8 retour_uc = 'N';
	switch (degrade) {
	case 1:
		retour_uc = 'O';
		break;
	case 2:
		retour_uc = 'F';
		break;
	}
	return retour_uc;
}

int lcr_tst_degrade(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin, INT * bloc)
{
	char *ptCour = (char*) &buffer[position];
	int lgCour = lg_mess - position;
	int nbCar = 0;
	int erreur = 0;
	int config = 0;
	uint16 degrade = configGetModeDegrade();
	int longueurCommande = strlen(LCR_PCP_TST_DEGRADE);

	ptCour += longueurCommande;
	lgCour -= longueurCommande;
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);

	while ((lgCour > 0) && (0 == erreur)) {
		if (strncmp(ptCour, LCR_TST_DEGRADE_OUI, strlen(LCR_TST_DEGRADE_OUI))
				== 0) {
			config = 1;
			degrade = 1;
		} else if (strncmp(ptCour, LCR_TST_DEGRADE_NON,
				strlen(LCR_TST_DEGRADE_NON)) == 0) {
			config = 1;
			degrade = 0;
		} else if (strncmp(ptCour, LCR_TST_DEGRADE_FORCE,
				strlen(LCR_TST_DEGRADE_FORCE)) == 0) {
			config = 1;
			degrade = 2;
		} else {
			erreur = 3;
		}
		ptCour++;
		lgCour--;
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	}
	if (0 == erreur) {
		if (config) {
			configSetModeDegrade(degrade);
		}
		tst_send_bloc(las, mode, buffer, &nbCar, bloc, (TRUE == flg_fin ? FALSE : TRUE),pt_mess, "%s %c", LCR_PCP_TST_DEGRADE, _lcr_tst_degrade_get_code(degrade));
	} else {
		tedi_ctrl_ext(las, mode, erreur);
		config = 0;
	}
	return (config);
}
