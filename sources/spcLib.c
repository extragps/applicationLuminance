/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/04/01
 * Fichier	: spcLib.c
 * Objet	: Il s'agit de la librairie permettant de gerer le compteur de
 * 		scrutation PC.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Apr01,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <stdio.h>
#include <time.h>
#include <limits.h>
#include "standard.h"
#include "mon_debug.h"
#include "define.h"
#include "mon_inc.h"
#include "simuLib.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "x01_str.h"
#include "x01_var.h"
#include "spcLib.h"
#include "eriLib.h"
#include "portage.h"

/* Nouvelles declation pour la gestion de la configuration */
#include "configuration.h"
#include "configurationSram.h"

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
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define SPC_TRM_TEMPO	10

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

                                                                                /* Compteur utilise pour decompter
                                                                                 * la scrutation.       */
static int spcCompteur = 0;
static int spcTrmCompteur = 0;

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * spcSignaler
 * ===========
 * Il s'agit de remettre le compteur
 * de scrutation e la valeur indiquee
 * -------------------------------- */

void spcSignaler (void)
{
  if (vct_tempo_spc)
    {
      spcCompteur = vct_tempo_spc;
    }
}

/* --------------------------------
 * spcTrmSignaler
 * ===========
 * Il s'agit de remettre le compteur
 * de scrutation e la valeur indiquee
 * -------------------------------- */

void spcTrmSignaler (void)
{
  spcTrmCompteur = SPC_TRM_TEMPO;
  x01_status3.trm = 1;
  vct_ST_TR |= STATUS_TR_TERM;
}

/* --------------------------------
 * spcTester
 * =========
 * Il s'agit de verifier que le compteur
 * n'est pas ecoule.
 * Attention, la methode doit etre
 * appelee toutes les secondes.
 * -------------------------------- */

void spcTester (void)
{
  if (vct_tempo_spc)
    {
      if (--spcCompteur == 0)
        {
    		configIncrSpc();
    		sysToMonitor(3);
        }
    }
}

/* --------------------------------
 * spcTrmTester
 * =========
 * Il s'agit de verifier que le compteur
 * n'est pas ecoule.
 * Attention, la methode doit etre
 * appelee toutes les secondes.
 * -------------------------------- */

void spcTrmTester (void)
{
  if (0 == spcTrmCompteur)
    {
      x01_status3.trm = 0;
      vct_ST_TR &= ~STATUS_TR_TERM;
    }
  else
    {
      spcTrmCompteur--;
      x01_status3.trm = 1;
      vct_ST_TR |= STATUS_TR_TERM;
    }
}
