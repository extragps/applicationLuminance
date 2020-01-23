/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: lcr_temp.c
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

#include <stdio.h>
#include "portage.h"
#include "define.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "tempLib.h"
#include "ted_prot.h"
#include "lcr_temp.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
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

int lcr_temp(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int nbCar = 0;
	int bloc = 0;
	int erreur = 0;
	int config = 0;
	ptCour += 8;
	lgCour -= 8;
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);

	if (lgCour > 0)
	{
		erreur = 2;
	}
	if (0 == erreur)
	{
		tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess, "TST TEMP %d.%d", tempGetValeur() / 10,
				tempGetValeur() % 10);
	} else
	{
		tedi_ctrl_ext(las, mode, erreur);
		config = 0;
	}
	return (config);
}

/* *********************************
 * FIN DE lcr_temp.c
 * ********************************* */
