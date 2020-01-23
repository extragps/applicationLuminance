
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: lcr_util.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _LCR_UTIL_H
#define _LCR_UTIL_H

/* ********************************
 * INCLUDES
 * ========
 * L'inclusion dans un fichier d'inter-
 * face doit etre limitee le plus
 * possible.
 * ********************************	*/

#include "time.h"
#include "mon_str.h"

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

#define LCR_UTIL_MAX_BUFFER 1024

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* --------------------------------
 * T_lcr_util_buffer
 * =================
 * Buffer pour les traces.
 * --------------------------------	*/

typedef struct T_lcr_util_buffer
{
  int nbCar;
  int nbCarMax;
  int guillemet;
  char buffer[LCR_UTIL_MAX_BUFFER];
} T_lcr_util_buffer;


/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

void tst_send_bloc (INT, INT, STRING, INT *, INT *, INT, T_usr_ztf *,
                    char *, ...);
char *tst_passe_blanc (char *, int, int *);
char *tst_passe_slash (char *, int, int *);
char *tst_passe_egale (char *, int, int *);
char *tst_lit_char (char *, int, int *, char *);
char *tst_lit_short (char *, int, int *, short *);
char *tst_lit_entier (char *, int, int *, int *);
char *tst_lit_param (char *, int, int *, char *);
char *tst_lit_param_at (char *, int, int *, char *);
char *tst_lit_param_af (char *, int, int *, char *);
char *tst_lit_param_symb(char *, int, int *, char *);
char *tst_lit_date (char *, int, int *, struct tm *, int *, int *);
char *tst_lit_jour (char *, int, int *, struct tm *, int *);
char *tst_lit_heure (char *, int, int *, struct tm *, int *);
char *tst_lit_dv (char *, int, int *, long *, int *);
char *tst_lit_am (char *, int, int *, int *, int *, int *);
char *tst_lit_am_bis (char *, int, int *, int *, int *, int *);
char *tst_lit_param_usr (char *, int, int *, char *, char *, int *);
char *tst_lit_param_cftp (char *, int, int *, char *, int *);

int lcr_util_buffer_init (T_lcr_util_buffer *, int);
int lcr_util_buffer_ajouter (T_lcr_util_buffer *, char *, int);
char *lcr_util_buffer_lire_buffer (T_lcr_util_buffer *);
int lcr_util_buffer_lire_nb_car (T_lcr_util_buffer *);
int lcr_util_buffer_recopier (T_lcr_util_buffer * buf, char *mess, int nbCar);
int lcr_util_buffer_tester_fin (T_lcr_util_buffer * buf);

/* ********************************
 * FIN DE lcr_util.h
 * ********************************	*/
#endif
