
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 21/03/01
 * Fichier	: sir_dv1.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,21Mar01,xag  creation.
 * ************************************************************************	*/

#ifndef _SIR_DV1_H
#define _SIR_DV1_H

#include "vct_str.h"

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

/* INT dprintf2 (STRING, ...); */
/*INT dv1_sprintf (STRING, STRING, INT); */
INT dv1_ord_don (INT, INT, INT);
STRING dv1_fdonn (INT, INT, INT, INT, INT);
INT dv1_slen (STRING);
INT dv1_car (STRING, INT, INT);
INT dv1_separateur (STRING, INT);
INT dv1_scmp (STRING, STRING, INT);
INT dv1_scmp_and_skip (STRING,INT, STRING, INT,STRING *,INT *);
INT dv1_scpy (STRING, STRING, INT);
INT dv1_set (STRING, INT, INT);
VOID dv1_itoh (STRING, INT, INT);
INT dv1_itoa (STRING, INT, INT);
INT dv1_atoi (STRING, INT, INT *);
INT dv1_htoi (STRING, INT, INT *);
INT dv1_htol (STRING, INT, LONG *);
INT dv1_ltoa (STRING, LONG, INT);
INT dv1_car (STRING, INT, INT);
ULONG dv1_chk (STRING, STRING);
INT dv1_num (STRING, INT);
INT dv1_utoh (STRING, UINT, INT);
INT dv1_ltoh (STRING, LONG, INT);
INT dv1_atol (STRING, INT, LONG *);
INT dv1_str_atoi (STRING *, INT *);
INT dv1_str_atol (STRING *, LONG *);
INT dv1_str_symb (STRING *, INT *);
INT dv1_str_atoh (STRING * ptr, UINT * ent);
INT dv1_str_litt (STRING *, STRING, INT);
ULONG dv1_chk_prom (STRING debut, STRING fin);

INT dv1_test_date (STRING pt_date);
VOID dv1_init_date (VOID);
ULONG dv1_sec (STRING pt_date);
INT dv1_cherchefin (STRING buffer, INT lgbuff);
INT dv1_format_date (STRING buff, STRING date, INT format);

/* ********************************
 * FIN DE sir_dv1h.h
 * ********************************	*/

#endif
