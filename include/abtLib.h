
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 12/02/01
 * Fichier	: abtLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,12Feb01,xag  creation.
 * ************************************************************************	*/

#ifndef _ABT_LIB_H
#define _ABT_LIB_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

void abtMain (int);
int abtEnregistrerAI (int, int, int, int, int, int, int);
int abtEnregistrer (int, int, int, int, char *, int, int, int);
int abtEnregistrerTotal (int, int, int, int, char *, int, int,
                         BOOL, struct tm, BOOL, struct tm, BOOL, long);
void abtSignalerAI (void);
void abtSignaler (int);
void abtInit (void);


/* ********************************
 * FIN DE seqLib.h
 * ********************************	*/
#endif
