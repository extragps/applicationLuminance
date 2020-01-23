
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 26/06/01
 * Fichier	: teleDetLib.h
 * Objet	: Definitions et protypes pour le telechargement des detecteurs.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,26Jun01,xag  creation.
 * ************************************************************************	*/

#ifndef _TELE_DET_LIB_H
#define _TELE_DET_LIB_H

/* ********************************	*/
/* DECLARATION DES CONSTANTES		*/
/* ********************************	*/

#define TELE_DET_PROT_SD	0x68
#define TELE_DET_PROT_SA	0x00
#define TELE_DET_PROT_ED	0x16
#define TELE_DET_PROT_ACK	0x90
#define TELE_DET_PROT_DA	0x80
#define TELE_DET_MAX_DATA	300

#define TELE_DET_CMD_ID			0X00
#define TELE_DET_CMD_CLEAR		0X01
#define TELE_DET_CMD_PROG       0X02
#define TELE_DET_CMD_MAKEVALID  0X03
#define TELE_DET_CMD_CHECKBLANK	0x04
#define TELE_DET_CMD_GETCHKSUM	0x05
#define TELE_DET_CMD_VERIFY		0x06
#define TELE_DET_CMD_START		0x07
#define TELE_DET_CMD_READ		0x08
#define TELE_DET_CMD_SET_MODE	0x10
#define TELE_DET_CMD_GET_ETAT	0x11
#define TELE_DET_CMD_GET_VERS 	0x12
#define TELE_DET_CMD_INACTIF 	0x20
#define TELE_DET_CMD_LANCER 	0x21

/* ********************************	
 * PROTOTYPE DES FONCTIONS DU MODULE
 * ********************************	*/

void teleDetTester (void);
void teleDetTraiter (int fdLiaisonLas[NBPORT]);

/* ********************************	
 * FIN DE teleLib.h
 * ********************************	*/

#endif
