/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: lcr_tst_cfetoile.c
 * Objet	: Non encore implemente
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
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
#include "lcr_tst_cfetoile.h"
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

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

int lcr_tst_cfetoile(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin, INT * bloc)
{
	char *ptCour = (char*) &buffer[position];
	int lgCour = lg_mess - position;
	int nbCar = 0;
	int erreur = 0;
	int config = 0;
	uint16 degrade = configGetModeDegrade();
	int longueurCommande = strlen(LCR_PCP_TST_CFETOILE);

	ptCour += longueurCommande;
	lgCour -= longueurCommande;
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);

	while ((lgCour > 0) && (0 == erreur)) {
		erreur = 3;
		ptCour++;
		lgCour--;
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	}
	if (0 == erreur) {
		tst_send_bloc(las, mode, buffer, &nbCar, bloc, (TRUE == flg_fin ? FALSE : TRUE),pt_mess, "%s %s", LCR_PCP_TST_CFETOILE,"NON ENCORE IMPLEMENTE");
	} else {
		tedi_ctrl_ext(las, mode, erreur);
		config = 0;
	}
	return (config);
}
