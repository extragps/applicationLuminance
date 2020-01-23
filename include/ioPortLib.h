
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: ioPortLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _IO_PORT_LIB_H
#define _IO_PORT_LIB_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

int ioPortLireAdresse (volatile char *);
BOOL ioPortLireParafoudre (void);
BOOL ioPortLireAlimDetecteur (void);
BOOL ioPortLireTerminal (void);
void ioPortEcrireAlimDetecteur (BOOL);
void ioPortResetDetecteur (void);

/* ********************************
 * FIN DE ioPortLib.h
 * ********************************	*/

#endif
