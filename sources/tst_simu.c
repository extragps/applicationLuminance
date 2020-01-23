
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 30/11/00
 * Fichier	: tst_simu.c
 * Objet	: Dans un premier temps, le fichier contient la procedure assu-
 * 		rant l'analyse le la commande TST SIMU.
 * 		A terme, il contiendra peut etre les fonctions auxiliaires de
 * 		gestion de la simulation.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,30Nov00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "portage.h"
#include "mon_str.h"
#include "tst_simu.h"
#include "mon_debug.h"
#include "lcr_util.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define TST_SIMU_ON			"ON"
#define TST_SIMU_OFF		"OFF"
#define TST_SIMU_ZERO		"Z"
#define TST_SIMU_STANDARD	"S"

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * tst_simu_dial
 * =============
 * Analyse de la commande "TST SIMU".
 * Entree :
 * - numero de la liaison sur lequel
 *   se fait le transfert.
 * - mode : mode de transmission,
 * - nombre de caracteres dans le
 *   buffer,
 * - position dans le buffer,
 * - pointeur vers un message de
 *   transmission
 * --------------------------------	*/

int
tst_simu_dial (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
               struct usr_ztf *pt_mess)
{
  char *ptCour = (char *) &buffer[position];
  int lgCour = lg_mess - position;
  int nbCar = 0;
  int bloc = 0;

  ptCour += 8;
  lgCour -= 8;
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
  if (strncmp (ptCour, TST_SIMU_ON, strlen (TST_SIMU_ON)) == 0)
    {
      ptCour += strlen (TST_SIMU_ON);
      lgCour -= strlen (TST_SIMU_ON);
      ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
      simuSetActif (1);
    }
  else if (strncmp (ptCour, TST_SIMU_OFF, strlen (TST_SIMU_OFF)) == 0)
    {
      ptCour += strlen (TST_SIMU_OFF);
      lgCour -= strlen (TST_SIMU_OFF);
      ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
      simuSetActif (0);
    }
  else if (strncmp (ptCour, TST_SIMU_ZERO, strlen (TST_SIMU_ZERO)) == 0)
    {
      ptCour += strlen (TST_SIMU_ZERO);
      lgCour -= strlen (TST_SIMU_ZERO);
      ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
      simuEffacer ();
    }
  else if (strncmp (ptCour, TST_SIMU_STANDARD, strlen (TST_SIMU_STANDARD)) ==
           0)
    {
      ptCour += strlen (TST_SIMU_STANDARD);
      lgCour -= strlen (TST_SIMU_STANDARD);
      ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
      simuSetStandard ();
    }
  else
    {
      /* Programmation de vehicules...                */
      while (((*ptCour >= '0') && (*ptCour <= '9')) || (*ptCour == '*'))
        {
          /* Valeurs par defaut pour tous les     */
          /* parametres.                                                  */
          char voie = -1;
          char sens = 1;
          short vitesse = 80;
          short longueur = 450;
          short tiv = 20;


          ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
          /* --- RECUPERATION DU NUMERO DE VOIE   */
          if ((*ptCour >= '0') && (*ptCour <= '9'))
            {
              ptCour = tst_lit_char (ptCour, lgCour, &lgCour, &voie);
            }
          else if (*ptCour == '*')
            {
              ptCour++;
              lgCour--;
            }
          ptCour = tst_passe_egale (ptCour, lgCour, &lgCour);
          if (lgCour > 0)
            {
              /* --- RECUPERATION DU SENS                             */
              if (*ptCour != '/')
                {
                  ptCour = tst_lit_char (ptCour, lgCour, &lgCour, &sens);
                }
              ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
              if (lgCour > 0)
                {

                  /* --- RECUP DE LA VITESSE                              */
                  if (*ptCour != '/')
                    {
                      ptCour =
                        tst_lit_short (ptCour, lgCour, &lgCour, &vitesse);
                    }
                  ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
                  if (lgCour > 0)
                    {
                      /* --- RECUP DE LA LONGUEUR                             */
                      if (*ptCour != '/')
                        {
                          ptCour =
                            tst_lit_short (ptCour, lgCour, &lgCour,
                                           &longueur);
                        }
                      ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
                      /* --- RECUPERATION DU TIV                              */
                      if (lgCour > 0)
                        {
                          if (*ptCour != '/')
                            {
                              ptCour =
                                tst_lit_short (ptCour, lgCour, &lgCour, &tiv);
                            }
                          simuAjouterVehicule (voie, sens, vitesse, longueur,
                                               tiv);
                        }
                    }
                }
            }
          ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
        }                              /* endwhile((*ptCour>='0')||(*ptCour    */
    }
  simuImprimer (las, mode, buffer, &nbCar, &bloc, FALSE, pt_mess);
  tst_send_bloc (las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess, "");
  /* ------------------------------------
   * FIN DE tst_simu_dial
   * ------------------------------------ */
  return 0;
}
