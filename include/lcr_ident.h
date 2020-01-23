
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 14/12/00
 * Fichier	: lcr_ident.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,14Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _LCR_IDENT_H
#define _LCR_IDENT_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

int lcr_ident (INT, INT, INT, STRING, INT, struct usr_ztf *);
int lcr_ident_param (INT, INT, INT, STRING, INT, struct usr_ztf *);
char *lcr_ident_substituer (char *, int, int *, T_lcr_util_buffer *);
char *lcr_ident_param_substituer (char *, int, int *, T_lcr_util_buffer *);

/* ********************************
 * FIN DE lcr_ident.h
 * ********************************	*/
#endif
