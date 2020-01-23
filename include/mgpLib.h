
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 02/03/06
 * Fichier	: mgpLib.h
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * ************************************************************************	*/

#ifndef _MGP_LIB_H
#define _MGP_LIB_H

/* ********************************
 * PROTOTYPE DES TYPES
 * ********************************	*/

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

int mgpPositionnerSortie(int,int);
int mgpLireEntree(int);
int mgpEcrireSorties(void);
void mgpLireEntrees(void);
int mgpLireSortie(int numSortie);
bool  mgpInit(void);
bool  mgpTerm(void);
void mgpRelaisInit(void);
void mgpTesterCartes(void);

/* ********************************
 * FIN DE mgpLib.h
 * ********************************	*/
#endif
