
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: ted_prot.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _TED_PROT_H
#define _TED_PROT_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/


VOID tedi_prot_reception (INT, INT);
VOID tedi_prot_emission (INT);
VOID tedi_prot_time (INT);
VOID tedi_ctrl (INT, INT, INT);
VOID tedi_ctrl_ext (INT, INT, INT);
INT tedi_bcc (STRING, INT);
VOID tedi_em (INT, T_usr_ztf *, int);
INT tedi_securite (VOID);
INT tedi_passwd (INT);
VOID tedi_erreur (INT, INT);
INT tedi_test_bloc (INT, INT, STRING, INT *,INT *, T_usr_ztf *);
VOID tedi_send_bloc (INT, INT, STRING, INT, INT, INT, T_usr_ztf *);
VOID tedi_send_abt (INT, INT, STRING, INT, INT, INT, T_usr_ztf *);
VOID tedi_send_ancrage (INT, INT, STRING, INT, INT, INT,T_usr_ztf *, mqd_t *);
VOID tedi_fin_emission (INT, INT);

/* ********************************
 * FIN DE ted_prot.h
 * ********************************	*/
#endif
