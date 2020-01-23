
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

#define IO_NB_BITS	8
#define IO_BIT0	0x0001
#define IO_BIT1	0x0002
#define IO_BIT2	0x0008
#define IO_BIT3	0x0010
#define IO_BIT4	0x0020
#define IO_BIT5	0x0100
#define IO_BIT6	0x0200
#define IO_BIT7	0x0400

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

int ioPortLireAdresse (char *);
BOOL ioPortLireParafoudre (void);
BOOL ioPortLireAlimDetecteur (void);
BOOL ioPortLireTerminal (void);
void ioPortEcrireAlimDetecteur (BOOL);
void ioPortResetDetecteur (void);

/* Nouvelles fonctions */

void ioPortLecture(void);
UINT8 ioPortLireFlags(BOOL);

/* ********************************
 * FIN DE ioPortLib.h
 * ********************************	*/

#endif
