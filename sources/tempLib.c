/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 20 decembre 2005
 * Fichier	: tempLib.c
 * Objet	: Le fichier permet de traiter les delestages temperature...
 * ************************************************************************
 * Historique des modifications
 * ----------------------------j
 * 01a,20Dec05,xag  creation.
 * ************************************************************************	*/

/* ********************************	
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include "stdio.h"
#ifdef VXWORKS
#include "vxworks.h"
#include "semLib.h"
#endif
#include "define.h"
#include "vct_str.h"
#include "mon_def.h"
#include "mon_debug.h"
#include "mon_str.h"
#include "x01_str.h"
#include "x01_var.h"
#include "tempLib.h"


/* ********************************		
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************	
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************	
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static T_tempInfos sTempInfos;
static T_tempInfos *tempInfos=&sTempInfos;
static int tempValeur=0;

/* ********************************	
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * tempInit
 * ========
 * --------------------------------	*/

void tempInit ()
{
	tempSetAlerte(0);	
}

int tempGetValeur()
{
    return tempValeur;
}

void tempSetValeur(int val)
{
	tempValeur=val;
}
/* --------------------------------
 * tempGetAlerte
 * =============
 * Retourne la valeur de l'alerte
 * temperature. 
 * Pas d'alerte en cas d'init incor-
 * recte.
 * --------------------------------	*/

int tempGetAlerte()
{
int retour=0;
	if(NULL!=tempInfos)
	{
		retour=tempInfos->alerte;
	}
	return retour;
}

/* ---------------------------------
 * tempSetAlerte
 * =============
 * Positionne l'alerte temperature.
 * --------------------------------	*/

void tempSetAlerte(int val)
{
		if(NULL!=tempInfos)
		{
				tempInfos->alerte=val;
			}
}

/* --------------------------------
 * tempDecTempo
 * ============
 * La fonction decremente la tempo et
 * retourne la valeur du compteur. 
 * Retourne 0 si deje a zero ou si non
 * initialise.
 * --------------------------------	*/

int tempDecTempo()
{
int val=0;
		if(NULL!=tempInfos)
		{
				if(tempInfos->hyster>0)
				{
					val=--tempInfos->hyster;
				}
		}
		return val;
	}
/* ---------------------------------
 * tempSetTempo
 * ============
 * Positionne la valeur de la tempo.
 * --------------------------------	*/

void tempSetTempo(int val)
{
		if(NULL!=tempInfos)
		{
				tempInfos->hyster=val;
			}
	}

/* ********************************	
 * FIN DE tempLib.c
 * ********************************	*/


