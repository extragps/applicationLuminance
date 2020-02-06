
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 01/12/00
 * Fichier	: mon_pro.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,01Dec00,xag  Modification du prototype de la fonction mnemuart.
 * ************************************************************************	*/

#ifndef 	MON_PRO_H
#define  	MON_PRO_H
                                                                                 /* ********************************
                                                                                 * PROTOTYPAGE DES FONCTIONS
                                                                                 * ********************************     */

INT monMessRec (mqd_t, int, char *);
void monMessSend (mqd_t, T_usr_ztf **, unsigned);
T_usr_ztf *monMessInit (mqd_t, T_usr_ztf *);
void mncbal (struct usr_anc *);
void mnecal (STRING);
void mnprecal (STRING);
struct usr_ztf *mnzmes (struct usr_anc *, STRING);
void mnemsusp (INT, struct usr_anc *, struct mon_mes *);
void mnresusp (INT, struct usr_anc *, struct mon_mes *);
LONG mnemsig (LONG);
//UINT8 mninput (UINT8 *);
//UINT16 mninput16 (UINT16 *);
void mnlcal (STRING);
ULONG mnlcalSec ();
void mnoutput (UINT8 *, UINT8);
void mnoutput16 (UINT16 *, UINT16);
void mnisop (STRING, STRING, LONG);
void mnsusp (INT);
void mnsuspIniPrendre ();
void mnrqrel (void);
void mncfuart (LONG, LONG, LONG, LONG, LONG, LONG);
void mnSetTempo(int,int);
int  mnGetTempo(int);

int mnemuart (int);

void mnXOFFuart (INT);
void mnXONuart (INT);
void mnecho (UINT8, UINT8);
void mnrts (UINT8);
UINT mnrqitd (void);
void mnrqresdet (void);
void mnemdebug (STRING, LONG);
void mnrestart (void);
void mnwrtfl (UINT *, UINT, UINT8 *);
void mnrazfl (void *, UINT8 *);
void mnsusp(INT tempo);

void mnTraitInitialisation (void);
void mnTraitPeriodique(void);

#endif
