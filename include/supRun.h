
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 19/02/01
 * Fichier	: supRun.h
 * Objet	: Prototypes des fonctions du module
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,19Feb01,xag  creation.
 * ************************************************************************	*/

#ifndef _SUP_RUN_H
#define _SUP_RUN_H


/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define SUP_RUN_ATTENTE 6

/* ********************************
 * PROTOTYPE DES FONCTIONS DU MODULE
 * ********************************	*/

void supRunInit (void);
int supRunDemarrer (void);
int supRunArreter (void);
int supRunLireEtat (void);

/* ********************************
 * FIN DE supInitLib.h
 * ********************************	*/

#endif
