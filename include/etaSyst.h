
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: etaSyst.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _ETA_SYST_H
#define _ETA_SYST_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

int etaSystTesterAppli (void);
int etaSystTesterSysteme (void);
void etaSystIncrCompteur (int numero);
void etaSystResetWatchDog (void);
void etaSystSetPos(int numero,int cpt);


/* ********************************
 * FIN DE etaSyst.h
 * ********************************	*/
#endif
