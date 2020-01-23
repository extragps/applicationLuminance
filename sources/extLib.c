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

#ifndef LINUX
#include "vxworks.h"
#include "tickLib.h"
#endif
#include "stdio.h"
#include "time.h"
#include "limits.h"
#include "standard.h"
#include "mon_debug.h"
#include "define.h"
#include "extLib.h"
#include "vct_str.h"                   /* Pour la manipulation des donnees */
#include "x01_vcth.h"
                                                                                /* brute.                                                       */

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

static T_extValeurs extValeurs = { FALSE, FALSE };

/* ********************************	
 * FONCTIONS DU MODULE
 * ********************************	*/

BOOL extSet (T_extType type, BOOL valeur)
{
  BOOL retour = valeur;
  switch (type)
    {
    case EXT_ECE:
      if (valeur != extValeurs.ece)
        {
          extValeurs.ece = valeur;
        }
      break;
    case EXT_ESA:
      if (valeur != extValeurs.esa)
        {
          extValeurs.esa = valeur;
        }
      break;
    default:
      retour = FALSE;
      break;
    }
  return retour;
}


BOOL extGet (T_extType type)
{
  BOOL retour;
  switch (type)
    {
    case EXT_ECE:
      retour = extValeurs.ece;
      break;
    case EXT_ESA:
      retour = extValeurs.esa;
      break;
    default:
      retour = FALSE;
      break;
    }
  return retour;
}
