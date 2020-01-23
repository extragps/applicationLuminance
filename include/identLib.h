
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: identLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _IDENT_LIB_H
#define _IDENT_LIB_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

void identMettreAZero (void);
void identStandard (void);
BOOL identAjouter (char *, char *, int);
int identTesterIdentEtPassword (char *, char *);
int identTesterPassword (char *);
char *identLireIdent (char *);
void identAutoriser (int, int, BOOL);
void identInterdire (int, BOOL);
int identLireUtilisateur (int);
BOOL identLireAutorisation (int);
BOOL identLireAutorisationAvecNiveau (int, int);

/* ********************************
 * FIN DE ioPortLib.h
 * ********************************	*/

#endif
