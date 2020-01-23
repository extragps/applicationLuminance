
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/11/00
 * Fichier	: supInitLib.h
 * Objet	: Prototypes des fonctions du module
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,27Nov00,xag  creation.
 * ************************************************************************	*/

#ifndef SUP_INIT_LIB_H
#define SUP_INIT_LIB_H

/* ********************************
 * PROTOTYPE DES FONCTIONS DU MODULE
 * ********************************	*/

int supInitAppli (T_supGestion * pGes, int typeBoot);
int supInitAncrage (void);
int supInitVar (T_supGestion * pGes);
int supTermAncrage (void);
/* int supInitTache (T_supGestion *, char *, int, short, FUNCPTR, int); */
int supTermTaches (T_supGestion *);
int supInitAppli (T_supGestion *, int);
int supTermAppli (T_supGestion *);
void supInitTaskId(int numTache);
void supInitThreadId(int numTache, TASK_ID threadPt);

/* ********************************
 * FIN DE supInitLib.h
 * ********************************	*/

#endif
