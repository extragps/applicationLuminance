
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 26/07/01
 * Fichier	: lcr_st_abe.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,26Jul01,xag  creation.
 * ************************************************************************	*/

#ifndef _ST_ABE_H
#define _ST_ABE_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

void lcr_st_abe (INT, INT, INT, STRING, INT, struct usr_ztf *);

int lcr_st_eri (INT, INT, INT, STRING, INT, struct usr_ztf *);
void lcr_st_lcom (INT, INT, INT, STRING, INT, struct usr_ztf *);
int lcr_st_ovf (INT, INT, INT, STRING, INT, struct usr_ztf *,INT,INT *);
int lcr_st_cac (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
int lcr_st_ver (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
int lcr_tst_version(INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
int lcr_st_edf (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
int lcr_st_btr (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
int lcr_st_dbg (INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess);
int lcr_trace(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess);
int lcr_tst_info_eri(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess) ;

/* ********************************
 * FIN DE lcr_st_abe.h
 * ********************************	*/
#endif
