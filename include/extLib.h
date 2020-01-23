
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: extLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _EXT_LIB_H
#define _EXT_LIB_H

/* ********************************
 * PROTOTYPE DES TYPES
 * ********************************	*/


typedef enum T_extType
{
  EXT_ECE,
  EXT_ESA
} T_extType;


/* --------------------------------
 * T_extValeurs
 * ============
 * Contient les valeurs des extensions
 * --------------------------------	*/

typedef struct T_extValeurs
{
  BOOL ece;
  BOOL esa;
} T_extValeurs;


/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

BOOL extGet (T_extType);
BOOL extSet (T_extType, BOOL);

/* ********************************
 * FIN DE extLib.h
 * ********************************	*/
#endif
