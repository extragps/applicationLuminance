
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 21/10/05
 * Fichier	: lcr_sc.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _LCR_SC_H
#define _LCR_SC_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

void lcr_tst_sc_init (void);
int lcr_tst_sc_pmv (INT, INT, INT, STRING, INT, struct usr_ztf *);
void lcr_tst_sc_ft_init (void);
void lcr_tst_sc_ft (INT, INT, INT, STRING, INT, struct usr_ztf *);
void lcr_tst_sc_cc_init (void);
void lcr_tst_sc_cc (INT, INT, INT, STRING, INT, struct usr_ztf *);
void lcr_tst_sc_frc_init (void);
void lcr_tst_sc_frc (INT, INT, INT, STRING, INT, struct usr_ztf *);

/* ********************************
 * FIN DE lcr_sc.h
 * ********************************	*/
#endif
