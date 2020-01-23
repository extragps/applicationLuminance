
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 21/02/01
 * Fichier	: teleLib.h
 * Objet	: prototype des fonctions des fonction d'analyse du telechargement
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,21Feb01,xag  creation.
 * ************************************************************************	*/

#ifndef _TELE_LIB_H
#define _TELE_LIB_H

/* ********************************	*/
/* DECLARATION DES CONSTANTES		*/
/* ********************************	*/

/* ********************************	
 * PROTOTYPE DES FONCTIONS DU MODULE
 * ********************************	*/

int teleAnalyser (int las, char *message, int longueur,
                  T_lcr_util_buffer * reponse);

/* ********************************	
 * FIN DE teleLib.h
 * ********************************	*/

#endif
