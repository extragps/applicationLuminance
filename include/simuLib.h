
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 01/12/00
 * Fichier	: simuLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,01Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _SIMU_LIB_H
#define _SIMU_LIB_H

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

void simuCreer (void);
void simuDetruire (void);
int simuSetActif (int);
int simuGetActif (void);
int simuSetStandard (void);
void simuEffacer (void);
int simuAjouterVehicule (char, char, short, short, short);
void simuImprimer (INT, INT, STRING, INT *, INT *, INT, struct usr_ztf *);

/* ********************************
 * FIN DE simuLib.h
 * ********************************	*/
#endif
