
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: lcr_cf3h.h
 * Objet	: Prototype des fonctions du module lcr_cf3.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,16Jan01,xag  creation.
 * ************************************************************************	*/

#ifndef _LCR_CF3H_H
#define _LCR_CF3H_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

INT cf3_cfdd (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
INT cf3_cfld (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
VOID cf3_init_fic_tra (void);
VOID cf3_init_fic_mes (INT);
INT cf3_szenrfic (INT, INT);
VOID cf3_conf_memoire (VOID);
char cf3_get_char_seq(int seq);

/* ********************************
 * FIN DE lcr_cf3h.h
 * ********************************	*/

#endif
