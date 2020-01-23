
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 13/11/00
 * Fichier	: anaSys.h
 * Objet	: prototype des fonctions de l'analyseur systeme (appele avant 
 * 			l'analyseur LCR...
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,24Nov00,xag  creation.
 * ************************************************************************	*/

#ifndef _ANA_SYS_H
#define _ANA_SYS_H

/* ********************************	*/
/* DECLARATION DES CONSTANTES		*/
/* ********************************	*/

#define ANA_SYS_EXPTACHES		"EXPTACHES"
#define ANA_SYS_EXPTACHEC		"EXPTACHEC"
#define ANA_SYS_INIT			"INIT"
#define ANA_SYS_VIDE			"VIDE"
#define ANA_SYS_MAX_TID			100     /* Nombre max de taches examinees       */

/* ********************************	
 * PROTOTYPE DES FONCTIONS DU MODULE
 * ********************************	*/

int anaSys (int, char *, int, T_lcr_util_buffer *);

/* ********************************	
 * FIN DE anaSys.h
 * ********************************	*/

#endif
