
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 13/04/01
 * Fichier	: rec_util.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _REC_UTIL_H
#define _REC_UTIL_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

UINT8 rec_util_tester_alerte (T_vct_critere * critere, UINT valeur);
char *rec_util_lire_relation (UINT8 relation);
char *rec_util_lire_alerte (UINT8 nature);
char *rec_int_lire_type (UINT8 nature);
char *rec_ing_lire_type (UINT8 nature);

/* ********************************
 * FIN DE rec_util.h
 * ********************************	*/
#endif
