
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 12/02/01
 * Fichier	: seqLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,12Feb01,xag  creation.
 * ************************************************************************	*/

#ifndef _SEQ_LIB_H
#define _SEQ_LIB_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

int seqGetPeriode (char);
int seqGetPeriodeFichier (int);
int seqGetFichier (char);
char seqGetType (int);
void seqSetPeriode (short, int);

/* ********************************
 * FIN DE seqLib.h
 * ********************************	*/
#endif
