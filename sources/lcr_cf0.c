/*
 * lcr_cf0.c
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_cf0.c,v $
 * Revision 1.2  2018/06/04 08:38:36  xg
 * Passage en version V122
 *
 * Revision 1.2  2008/10/16 07:41:29  xag
 * Mise en conformité de la commande CF*
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include <stdio.h>
#include <string.h>
#include "standard.h"
#include "mon_inc.h"
#include "define.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_cf0h.h"
#include "lcr_cf2h.h"
#include "lcr_cf5h.h"
#include "lcr_cfpp.h"
#include "lcr_cfes.h"
#include "lcr_cfet.h"
#include "ted_prot.h"
#include "identLib.h"

/* *******************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* --------------------------------
 * T_cf0Infos
 * ==========
 * Structure contenant les infos
 * necessaires pour l'appel d'une
 * fonction configuree automatiquement
 * pas CF*.
 * -------------------------------- */

typedef struct T_cf0Infos
{
  char *commande;
  int (*fct) (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
  char macro;
} T_cf0Infos;

/* *********************************
 * DEFINTION DES VARIABLES LOCALES
 * ********************************	*/

static T_cf0Infos cf0Infos[] = {
  {"CFAL", cf5_cfal,'Z'},
  {"CFF", cf2_cff,'Z'},
  {"CFET", lcr_cfet,'S'},
  {"CFES", lcr_cfes,'S'},
  {"CFPP", lcr_cfpp,'S'},
  {NULL, NULL}
};


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cf0_cf                                                 /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/05/1993                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf0.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : mise a jour de la liste canal capteur                  /
/-------------------------------------------------------------------------DOC*/

INT cf0_cf (INT las, INT mode, INT lg_mess, STRING buffer,
            INT position, struct usr_ztf *pt_mess)
{
  INT flag_err = 0;
  INT i = position + 3;
  UINT8 fini;
  INT bloc = 0;
  INT config = FALSE;
  /* on elimine les separateurs de
   * parametres */
  i += dv1_separateur (&buffer[i], lg_mess - i);
  /* si la commande comporte des
   * parametres */
  if (i < lg_mess)
    {
      /* dans ce cas, on est en ecriture,
         il faut tester le mot de passe */
      if (identLireAutorisationAvecNiveau (las, 0))
        {
          /* on initialise la variable de
           * controle de la boucle while */
          fini = FALSE;
          /* tant que l'on n'a pas atteint la
           * fin du message */
          while (!fini)
            {
              /* on elimine les separateurs de
               * parametres */
              i += dv1_separateur (&buffer[i], lg_mess - i);
              /* on recherche le parametre S ou Z */
              if ((i < lg_mess) && ((buffer[i] == 'S') || (buffer[i] == 'Z')))
                {
                  /* si on a trouve un S ou un Z, on
                   * arrette le decodage et on ne
                   * retient que ces parametres */
                  if ((buffer[i] == 'Z') || (buffer[i] == 'S'))
                    {
                      /* on arrete prematurement le
                       * decodage */
                      if ((i + 1 < lg_mess) &&
                          ((buffer[i + 1] != ' ') && (buffer[i + 1] != ',')))
                        {
                          /* on genere une erreur de syntaxe */
                          flag_err = 1;

                        }
                      else
                        {
                          int indice = 0;
                          char macro = buffer[i];
                          /* reinitialisation de la
                           * configuration */
                          while (NULL != cf0Infos[indice].commande)
                            {
                              INT notFin =
                                (NULL != cf0Infos[indice + 1].commande);
                              sprintf (buffer, "%s %c",
                                       cf0Infos[indice].commande,( macro=='S'?macro:cf0Infos[indice].macro));
                              config |=
                                cf0Infos[indice].fct (las, mode,
                                                      strlen (buffer), buffer,
                                                      0, pt_mess, notFin,
                                                      &bloc);
                              indice++;
                            }
                          buffer[0] = 0;
                        }              /* endif ((i+1 < lg_mess) &&            */
                    }                  /* endif ((buffer[i] == 'Z')            */
                  /* on arrete le decodage */
                  fini = TRUE;
                }
              else
                {
                  /* si on est pas au bout du buffer */
                  if (i < lg_mess)
                    {
                      flag_err = 1;
                    }
                }                      /* endif ((i < lg_mess) &&((buff        */
              /* on passe au caractere suivant */
              i++;
              /* si on a passe en revue tout le
               * buffer */
              if ((i >= lg_mess) || (flag_err != 0))
                {
                  fini = TRUE;
                }
            }
        }
        else
        {
       		tedi_ctrl_ext (las, mode,  5 );
        }
    }
  else
    {
      int indice = 0;
      while (NULL != cf0Infos[indice].commande)
        {
          INT notFin = (NULL != cf0Infos[indice + 1].commande);
          sprintf (buffer, "%s", cf0Infos[indice].commande);
          config |= cf0Infos[indice].fct (las, mode,
                                          strlen (buffer), buffer, 0, pt_mess,
                                          notFin, &bloc);
          indice++;
        }
      buffer[0] = 0;
    }
  /* --------------------------------
   * FIN DE cf0_cf
   * --------------------------------     */
  return (config);
}
