/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: extLib.c
 * Objet	: Gestion des extensions des codes d'erreur et des traces.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/


#include "standard.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "mon_def.h"
#include "mon_str.h"
#include "mon_pro.h"
#include "mgpLib.h"
#include "io/iolib.h"

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/


/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif
                                                                                /* Pour l'heure, la simulation est
                                                                                 * limitee e 200 vehicules sur dix
                                                                                 * voies....    */
/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/


/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

int mgpPositionnerSortie(int numSortie,int val)
{
	return (int)ioSetSortie(numSortie,(val==0?true:false));
}

int mgpLireEntree(int numEntree)
{
int retour=-1;
bool sortie_b=ioGetEntree(numEntree);
	retour=(sortie_b==true?0:1);
	return retour;
}


int mgpLireSortie(int numSortie)
{
int retour=-1;
bool val=false;

	//	ioReadCommande(_mgpInfo_pt->desc_pt,numero,&val);
	retour=(val==false?0:1);
	return retour;
}

bool mgpInit(void)
{
	ioInit();
	return true;
}

bool mgpTerm(void)
{
	return true;
}

