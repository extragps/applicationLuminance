
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: lcr_tst_sc.c
 * Objet	: Analyse et traitement de la commande TST,SC pour le traitement
 * 		des extensions des codes d'erreur.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "portage.h"
#include "define.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "lcr_tst_sc.h"
#include "extLib.h"
#include "ted_prot.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define LCR_TST_SC_ECE "ECE"
#define LCR_TST_SC_ESA "ESA"


/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

int
lcr_tst_sc (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
            struct usr_ztf *pt_mess)
{
  char *ptCour = (char *) &buffer[position];
  int lgCour = lg_mess - position;
  int nbCar = 0;
  int bloc = 0;
  int erreur = 0;
  int config = 0;
  int esa = extGet (EXT_ESA);
  int ece = extGet (EXT_ECE);

  ptCour += 6;
  lgCour -= 6;
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);

  while ((lgCour > 0) && (0 == erreur))
    {
      if (strncmp (ptCour, LCR_TST_SC_ECE, strlen (LCR_TST_SC_ECE)) == 0)
        {
          ptCour += strlen (LCR_TST_SC_ECE);
          lgCour -= strlen (LCR_TST_SC_ECE);
          ptCour = tst_passe_egale (ptCour, lgCour, &lgCour);
          if ('O' == *ptCour)
            {
              if (TRUE != ece)
                {
                  config = 1;
                  ece = TRUE;
                }
            }
          else if ('N' == *ptCour)
            {
              if (FALSE != ece)
                {
                  ece = FALSE;
                  config = 1;
                }
            }
          else
            {
              printDebug ("Il y a erreur 3 : %s", ptCour);
              erreur = 3;
            }
          ptCour++;
          lgCour--;
        }
      else if (strncmp (ptCour, LCR_TST_SC_ESA, strlen (LCR_TST_SC_ESA)) == 0)
        {
          ptCour += strlen (LCR_TST_SC_ESA);
          lgCour -= strlen (LCR_TST_SC_ESA);
          ptCour = tst_passe_egale (ptCour, lgCour, &lgCour);
          if ('O' == *ptCour)
            {
              if (TRUE != esa)
                {
                  config = 1;
                  esa = TRUE;
                }
            }
          else if ('N' == *ptCour)
            {
              if (FALSE != esa)
                {
                  config = 1;
                  esa = FALSE;
                }
            }
          else
            {
              printDebug ("Il y a erreur 3 : %s", ptCour);
              erreur = 3;
            }
          ptCour++;
          lgCour--;
        }
      else
        {
          printDebug ("Il y a erreur 2 : %s", ptCour);
          erreur = 2;
        }
      ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
    }
  if (0 == erreur)
    {
      if (config)
        {
          extSet (EXT_ECE, ece);
          extSet (EXT_ESA, esa);
        }
      tst_send_bloc (las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess,
                     "TST SC %s=%c %s=%c",
                     LCR_TST_SC_ECE, ((extGet (EXT_ECE) == TRUE) ? 'O' : 'N'),
                     LCR_TST_SC_ESA,
                     ((extGet (EXT_ESA) == TRUE) ? 'O' : 'N'));
    }
  else
    {
      tedi_ctrl_ext (las, mode, erreur);
      config = 0;
    }
  return (config);
}
