/*
 * trt_alt.c
 * ===================
 *
 *  Created on: 23 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: TRT_ALT.c,v $
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.7  2009/09/18 08:18:21  xgaillard
 * Petite correction de formattage.
 *
 * Revision 1.6  2009/08/21 14:16:49  xgaillard
 * Complement de trace.
 *
 * Revision 1.5  2008/10/13 08:18:16  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.4  2008/10/03 16:21:46  xag
 * *** empty log message ***
 *
 * Revision 1.3  2008/09/12 14:57:10  xag
 * *** empty log message ***
 *
 * Revision 1.2  2008/07/04 17:05:06  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
#include <time.h>

#include "standard.h"                  /* redefinition des types                         */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_idfh.h"
#include "lcr_trc.h"
#include "lcr_st1.h"
#include "xversion.h"
#include "vct_str.h"
#include "rec_util.h"
#include "eriLib.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "ficTrace.h"
#include "tac_conf.h"
#include "stAlLib.h"


/* Nouvelles declation pour la gestion de la configuration */
#include "configuration.h"
#include "configurationSram.h"

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif


/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/
void trt_alt (void);
static void trt_alt_status_temps_reel (void);

int trt_alt_tester_alerte (void)
{
  int retour = 0;
  int indice;
  int modifConfig = 0;
  UINT8 flag_alerte[MAX_CRT_SY];
  if (vct_cf_al_sy.flag == TRUE)
    {

      for (indice = 0; indice < MAX_CRT_SY; indice++)
        {
          flag_alerte[indice] = TRUE;
        }
      for (indice = 0; indice < MAX_CRT_SY; indice++)
        {
          T_vct_critere *pt_critere = &vct_cf_al_sy.critere[indice];
          if (pt_critere->nature != N_AFF)
            {
              if (TRUE == flag_alerte[indice])
                {
                  switch (pt_critere->nature)
                    {
                    case AL_CKS:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, vct_cks_prom);
                      break;
                    case AL_EDF:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, x01_status3.edf);
                      break;
                    case AL_GAR:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, configGetGar());
                      break;
                    case AL_RST:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, configGetRst());
                      break;
                    case AL_INI:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, configGetIni());
                      break;
                    case AL_TRM:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, x01_status3.trm);
                      break;
                    case AL_ERR:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, x01_status3.err);
                      break;
                    case AL_ER1:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, x01_status3.er1);
                      break;
                    case AL_ER2:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, x01_status3.er2);
                      break;
                    case AL_ER3:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, x01_status3.er3);
                      break;
                    case AL_BCL:
                       break;
                    case AL_BTR:
                      flag_alerte[indice] =
                        rec_util_tester_alerte (pt_critere, 0);
                      break;
                    }
                  if ((((flag_alerte[indice] == TRUE)
                        && (pt_critere->etat != REL_ETAT_ON))
                       || ((flag_alerte[indice] == FALSE)
                           && (pt_critere->etat == REL_ETAT_ON)))
                      && (pt_critere->oplog == OP_OR))
                    {
                      char txt[256];
                      struct timespec heure;
                      int premInd;
                      if (flag_alerte[indice] == TRUE)
                        {
                          sprintf (txt, "+");
                        }
                      else
                        {
                          sprintf (txt, "-");
                        }
                      /* Recherche du precedent                       */
                      /* Remonter ici jusque au dernier
                       * OP_AND */
                      for (premInd = indice - 1; premInd >= 0; premInd--)
                        {
                          if (vct_cf_al_sy.critere[premInd].oplog == OP_OR)
                            {
                              /*      On a atteint le precedent, on
                               *      peut passer au suivant.                 */
                              break;
                            }          /* endif(vct_cf_al_my                           */
                        }              /* endfor(premInd=i-1                           */
                      /* Le premier est le suivant            */
                      for (premInd = premInd + 1; premInd <= indice;
                           premInd++)
                        {
                          char txtTemp[256];
                          T_vct_critere *crit =
                            &vct_cf_al_sy.critere[premInd];
                          /* Composer la sequence                         */
                          sprintf (txtTemp, " %s%s",
                                   rec_util_lire_alerte (crit->nature),
                                   rec_util_lire_relation (crit->relation));
                          strcat (txt, txtTemp);
                          if (REL_DIFF != crit->relation)
                            {
                              sprintf (txtTemp, "%d", crit->seuil);
                              strcat (txt, txtTemp);
                            }
                          if (premInd != indice)
                            {
                              strcat (txt, " &");
                            }          /* endif(premInd!=i                                     */
                        }              /* endfor(premInd=premInd+1;premI       */
                      /* Enregistrer l'alerte.                        */
                      clock_gettime (CLOCK_REALTIME, &heure);
                      {
                        T_ficTraceAdresse adresse;
                        ficTraceAdresseSetStandard (&adresse, "I00001");
                        ficTraceAjouterEnreg (F_IA, &heure,
                                              &adresse, strlen (txt), txt);
                      }
                      /* Lancement de l'alerte.                       */
                      if ('+' == txt[0])
                        {
                          if (stAlLancer () == -1)
                            {
                              printDebug ("Neutralisation en cours\n");
                            }
                        }
                      /* trouve = TRUE;                       */
                      /* on positionne l'alerte       */
                    }                  /* endif((flag_alerte[indice]==TRUE) */
                }

              if ((flag_alerte[indice] == FALSE)
                  && (pt_critere->oplog == OP_AND))
                {
                  if (indice < MAX_CRT_SY - 1)
                    flag_alerte[indice + 1] = FALSE;
                }
              if (pt_critere->etat !=
                  (flag_alerte[indice] == TRUE ? REL_ETAT_ON : REL_ETAT_OFF))
                {
                  pt_critere->etat =
                    (flag_alerte[indice] ==
                     TRUE ? REL_ETAT_ON : REL_ETAT_OFF);
                  modifConfig = 1;
                }
            }
        }
    }
  if (modifConfig)
    {
      tac_conf_cfg_ecr_conf ();
    }
  return retour;
}


static void trt_alt_status_temps_reel ()
{
  UINT8 st;


  if (vct_ST_TR != nv_vct_ST_TR)
    {
      mnlcal (x01_status3.evt.date);

      /* il y a un changement de status temps reel */
      st = vct_ST_TR ^ nv_vct_ST_TR;

      if (st & vct_ST_TR & STATUS_TR_INIT)
        {
    	           /* initialisation  */
          switch (vct_type_init)
            {
            case TYPE_INIT_INIT:
              dv1_scpy (x01_status3.evt.nom, (STRING) "INI", 3);
              /* La trace est affichee lorsque l'on pas en mode exploitation. */
              cmd_trc_tr ("INI=%03lu RST=%03lu SPC=%03lu GAR=%03lu",
            		  configGetIni(),configGetRst(),configGetSpc(),configGetGar());
              break;
            case TYPE_INIT_RST:
              dv1_scpy (x01_status3.evt.nom, (STRING) "RST", 3);
              /* on trace */
              cmd_trc_tr ("RST  : %03lu",configGetRst());
              break;
            case TYPE_INIT_WDG:
              dv1_scpy (x01_status3.evt.nom, (STRING) "WDG", 3);
              /* on trace */
              cmd_trc_tr ("WDG  : %03lu",configGetGar());
              break;
            }
        }

      if (st & vct_ST_TR & STATUS_TR_EDF)
        {
          /* disparition EDF */
          dv1_scpy (x01_status3.evt.nom, (STRING) "EDF", 3);
          /* on trace */
          cmd_trc_tr ("ABSENCE EDF", 0);
        }
      else if (st & nv_vct_ST_TR & STATUS_TR_EDF)
        {
          /* on trace */
          cmd_trc_tr ("PRESENCE EDF", 0);
        }

      if (st & vct_ST_TR & STATUS_TR_TERM)
        {
          /* disparition EDF */
          dv1_scpy (x01_status3.evt.nom, (STRING) "TRM", 3);
          /* on trace */
          cmd_trc_tr ("CONNEXION TERMINAL", 0);
        }
      else if (st & nv_vct_ST_TR & STATUS_TR_TERM)
        {
          /* on trace */
          cmd_trc_tr ("DECONNEXION TERMINAL", 0);
        }

      if (st & vct_ST_TR & STATUS_TR_ERR_MAJ)
        {
          /* erreur MAJ */
          dv1_scpy (x01_status3.evt.nom, (STRING) "ERR", 3);
          if (0 == x01_status3.err)
            {
              x01_status3.err = 1;
              trt_alt_tester_alerte ();
            }
        }
      nv_vct_ST_TR = vct_ST_TR;
    }
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : trt_alt                                                /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 15/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : trt_alt.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : traitement des alertes                                 /
/-------------------------------------------------------------------------DOC*/

void trt_alt (void)
{
  trt_alt_status_temps_reel ();
  /* --------------------------------
   * FIN DE trt_alt
   * --------------------------------     */
  return;
}
