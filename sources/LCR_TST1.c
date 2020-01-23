/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst1                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 01/03/1991                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Regroupe les fonctions de traitement particulier  /
/                                                                             /
/-----------------------------------------------------------------------------/
/                                HISTORIQUE                                   /
/-----------------------------------------------------------------------------/
/ DATE   |   AUTEUR    | VERSION | No de fiche de modification                /
/-----------------------------------------------------------------------------/
/        |             |         |                                            /
/-----------------------------------------------------------------------------/
/        |             |         |                                            /
/-----------------------------------------------------------------------------/
/                    INTERFACES EXTERNES DU SOUS-MODULE                       /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/-----------------------------------------------------------------------------/
/ DONNEES EXPORTEES :                                                         /
/-----------------------------------------------------------------------------/
/ FONCTIONS IMPORTEES :                                                       /
/-----------------------------------------------------------------------------/
/ FONCTIONS EXPORTEES :                                                       /
/-----------------------------------------------------------------------------/
/               DONNEES ET FONCTIONS INTERNES DU SOUS-MODULE                  /
/-----------------------------------------------------------------------------/
/ DONNEES INTERNES :                                                          /
/-----------------------------------------------------------------------------/
/ FONCTIONS INTERNES :                                                        /
/-------------------------------------------------------------------------DOC*/
#ifdef VXWORKS
#include "vxworks.h"
#include "string.h"
#else
#include <memory.h>
#endif

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
#include "lcr_cf3h.h"
#include "rec_main.h"
#include "x01_trch.h"
#include "xversion.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/
INT lcr_tst1_cde_tstAI (STRING, INT);
INT lcr_tst1_par_al_vit (STRING, INT, INT *, INT *, INT *);
INT lcr_tst1_par_acquisition (STRING, INT);
INT lcr_tst1_par_divers (STRING, INT, INT *);
INT lcr_tst1_par_aff (STRING, INT, INT *);
INT lcr_tst1_par_alarme (STRING, INT);

#ifdef RAD
/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst1_cde_tstAI                                          /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 04/07/1996                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des parametres mso pour alertes ind vit   /
/-------------------------------------------------------------------------DOC*/

INT lcr_tst1_cde_tstAI (buffer, lg_par)
     STRING buffer;             /* buffer contenant le parametre et sa valeur              */
     INT lg_par;                /* taille du buffer                                        */
{
  INT flag_err;
  INT i;
  INT k;
  INT fini;
  INT taille;
  INT canal;
  INT tor;
  INT duree;


  flag_err = 0;
  i = 0;
  fini = FALSE;

  while (!fini)
    {
      i += dv1_separateur (&buffer[i], lg_par - i);
      if ((i < lg_par) &&
          (((buffer[i] >= '0') && (buffer[i] <= '9')) ||
           (buffer[i] == 'Z') || (buffer[i] == 'S')))
        {
          /* si on a trouve S ou Z on arrete le decodage */
          if ((buffer[i] == 'Z') || (buffer[i] == 'S'))
            {
              if ((i + 1 < lg_par)
                  && ((buffer[i + 1] != ' ') && (buffer[i + 1] != ',')))
                {
                  flag_err = 1;
                }
              else
                {
                  for (k = 0; k < NBCANAL; k++)
                    {
                      vct_ind_vit_alarme[k].num_tor = N_AFF;
                      vct_ind_vit_alarme[k].temps_acti = 0;
                    }
                  if (buffer[i] == 'S')
                    {
                      for (k = 0; k < NBCANAL; k++)
                        {
                          vct_ind_vit_alarme[k].num_tor = k;
                          vct_ind_vit_alarme[k].temps_acti = 5;
                        }

                    }
                }
              /* on arrete le decodage */
              fini = TRUE;
            }
          else
            {
              taille = dv1_cherchefin (&buffer[i], lg_par - i);
              /* on decode la definition du capteur */
              flag_err =
                lcr_tst1_par_al_vit (&buffer[i], taille, &canal, &tor,
                                     &duree);
              if (flag_err != 0)
                {
                  /* on sort du decodage */
                  fini = TRUE;
                }
              else
                {
                  vct_ind_vit_alarme[canal].num_tor = tor;
                  vct_ind_vit_alarme[canal].temps_acti = duree;
                  i += (taille - 1);
                }
            }
        }
      else
        {
          if (i < lg_par)
            /* on genere une erreur */
            flag_err = 1;
        }
      i++;
      /* si on a passe en revue tout le buffer */
      if ((i >= lg_par) || (flag_err != 0))
        fini = TRUE;
    }
  return (flag_err);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst1_par_al_vit                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 08/07/1996                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des no de tor et duree pour alertes vit   /
/-------------------------------------------------------------------------DOC*/

INT lcr_tst1_par_al_vit (buffer, taille, canal, tor, duree)
     STRING buffer;
     INT taille;
     INT *canal;
     INT *tor;
     INT *duree;

{
  INT i;
  INT j;
  INT erreur;
  INT flag_err;

  flag_err = 0;
  erreur = 0;
  i = 0;
  if (i < taille)
    {
      /* on lit le numero du canal */
      i += dv1_car (&buffer[i], taille - i, '=');
      if ((i == 0) || (i > 2))
        {
          flag_err = 1;
        }
      else
        {
          if (!dv1_atoi (&buffer[0], i, canal))
            {
              flag_err = 1;
            }
          else
            {
              if (*canal >= NBCANAL)
                {
                  flag_err = 2;
                }
            }
        }
      if (flag_err == 0)
        {
          i++;
          j = 0;
          j += dv1_car (&buffer[i], taille - i, '/');
          if ((j != 1) && (j != 2))
            {
              flag_err = 1;
            }
          else
            {

              if (!dv1_atoi (&buffer[i], j, tor))
                {
                  flag_err = 2;
                }
              else
                {
                  if (*tor >= NB_CARTE * 16)
                    {
                      flag_err = 2;
                    }
                }
            }
          i += j + 1;
          if (i >= taille)
            {
              flag_err = 1;
            }
          else
            {

              if (!dv1_atoi (&buffer[i], taille - i, duree))
                {
                  flag_err = 2;
                }
              else
                {
                  if (*duree >= MAX_DUREE_TOR)
                    {
                      flag_err = 2;
                    }
                }
            }
        }
    }
  return (flag_err);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst1_par_acquisition                                        /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 15/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification de la configuration de l'acquisition      /
/-------------------------------------------------------------------------DOC*/

INT lcr_tst1_par_acquisition (buffer, taille)
     STRING buffer;             /* buffer contenant le parametre et sa valeur              */
     INT taille;                /* taille du buffer                                        */

{
  INT i;                        /* variable de boucle */
  INT val;                      /* valeur entiere pour la conversion chaine --> entier */
  LONG vall;                    /* valeur entiere pour la conversion chaine --> entier */
  INT flag_err;                 /* flag d'erreur ds l'analyse de la question */

/* on suppose pas d'erreur */
  flag_err = 0;

  if (taille == 1)
    {
      if ((buffer[0] == 'S') || (buffer[0] == 'Z'))
        {
          /*  configuration standart de l'acquisition */
          /* configuration des seuils de l'acquisition */
          vct_seuil_va_max = 2;        /* valeur analogique          */
          vct_seuil_tp_max = 10L * 60L * 1000L; /* temps de presence : 10 mn  */
          vct_seuil_tp_min = 40L;      /* temps de presence : 40 ms  */
          vct_tempo_coincidence = 8L;  /* tempo d'attente   : 400 ms */
          vct_tempo_attente_piezo = 32L;        /* tempo d'attente   : 400 ms */
          vct_tempo_voie = 10L * 60L * 1000L;   /* tempo d'attente   : 10 mn  */
          vct_seuil_D1 = 100;          /* antichevauchement : 60 va max */
          vct_seuil_D2 = 100;          /* antichevauchement : 50 va moy */
          vct_seuil_D3 = 60L;          /* antichevauchement : 10 ms  */
          vct_seuil_D4 = 65L;          /* antichevauchement : 20 ms  */
          vct_seuil_D5 = 90L;          /* antichevauchement : 50 va max */

          vct_seuil_diff_va = 20;
          vct_seuil_diff_tp = 30;
          vct_algo_sens = TRUE;


        }
      else
        {
          /* erreur de syntaxe */
          flag_err = 1;
        }
    }
  else
    {
      i = dv1_car (buffer, taille, '=');

      if ((i == taille) || (i < 2))
        flag_err = 1;
      else
        {
          if (dv1_scmp (buffer, (STRING) "VA", 2))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_va_max = val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "TP_MAX", 6))
            {
              /* le parametre est il un nombre */
              if (!dv1_atol (&buffer[i + 1], taille - i - 1, &vall))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_tp_max = vall;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "TP_MIN", 6))
            {
              /* le parametre est il un nombre */
              if (!dv1_atol (&buffer[i + 1], taille - i - 1, &vall))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_tp_min = vall;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "TAC", 3))
            {
              /* le parametre est il un nombre */
              if (!dv1_atol (&buffer[i + 1], taille - i - 1, &vall))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_tempo_coincidence = vall;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "VOIE", 4))
            {
              /* le parametre est il un nombre */
              if (!dv1_atol (&buffer[i + 1], taille - i - 1, &vall))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_tempo_voie = vall;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "D1", 2))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_D1 = (INT) val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "D2", 2))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_D2 = (INT) val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "D3", 2))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_D3 = (LONG) val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "D4", 2))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_D4 = (LONG) val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "D5", 2))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_D5 = (LONG) val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "TPZ", 3))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_tempo_attente_piezo = (LONG) val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "DIFTP", 5))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_diff_tp = (INT) val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "DIFVA", 5))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_seuil_diff_va = (INT) val;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "RTSENS", 5))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  if ((val == 0) || (val == 1))
                    vct_algo_sens = ((val == 1) ? TRUE : FALSE);
                  else
                    flag_err = 1;
                }
            }
        }
    }

  return (flag_err);
}


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst1_par_divers                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 15/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des parametres divers                     /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/                     buffer = buffer comprenant le parametre a traiter       /
/                     taille = taille du buffer                               /
/                     voie   = numero du detecteur voulu ou 99 tous les canaux    /
/-------------------------------------------------------------------------DOC*/

INT lcr_tst1_par_divers (STRING buffer,INT taille,INT * reinit)
{
  INT i;                        /* variable de boucle */
  INT val;                      /* valeur entiere pour la conversion chaine --> entier */
  INT flag_err;                 /* flag d'erreur ds l'analyse de la question */

/* on suppose pas d'erreur */
  flag_err = 0;

  if (taille == 1)
    {
      if ((buffer[0] == 'S') || (buffer[0] == 'Z'))
        {
          /*  configuration standard */
          vct_poids = TRUE;
          vct_perco = 600L;
          vct_perco_ms = 600000L;
        }
      else
        {
          /* erreur de syntaxe */
          flag_err = 1;
        }
    }
  else
    {
      i = dv1_car (buffer, taille, '=');

      if ((i == taille) || (i < 2))
        flag_err = 1;
      else
        {
          if (dv1_scmp (buffer, (STRING) "POIDS", 5))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  vct_poids = val;
                }
            }

          if (dv1_scmp (buffer, (STRING) "UTIL=SEC", 8))
            {
              /* si il y a changement */
              if (vct_per_util == MINUTE)
                {
                  /* on reinitialise les variables de calcul */
                  *reinit = TRUE;
                  vct_per_util = SECONDE;
                }
            }
          if (dv1_scmp (buffer, (STRING) "UTIL=MIN", 8))
            {
              /* si il y a changement */
              if (vct_per_util == SECONDE)
                {
                  /* on reinitialise les variables de calcul */
                  *reinit = TRUE;
                  vct_per_util = MINUTE;
                }
            }
        }
    }

  return (flag_err);
}


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst1_par_aff                                                /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 15/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des parametres d'affichage                /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/                     buffer = buffer comprenant le parametre a traiter       /
/                     taille = taille du buffer                               /
/                     voie   = numero du detecteur voulu ou 99 tous les canaux    /
/-------------------------------------------------------------------------DOC*/

INT lcr_tst1_par_aff (buffer, taille, reinit)
     STRING buffer;             /* buffer contenant le parametre et sa valeur              */
     INT taille;                /* taille du buffer                                        */
     INT *reinit;

{
  INT j;
  INT k;
  UINT8 fini;
  INT i;                        /* variable de boucle */
  INT val;                      /* valeur entiere pour la conversion chaine --> entier */
  INT flag_err;                 /* flag d'erreur ds l'analyse de la question */
  struct vct_hmvl sv_vct_hmvl;

  memcpy ((STRING) & sv_vct_hmvl, (STRING) & vct_hmvl,
          sizeof (struct vct_hmvl));

/* on suppose pas d'erreur */
  flag_err = 0;

  if (taille == 2)
    {
      if ((buffer[0] == 'S') || (buffer[0] == 'Z'))
        {
          /*  configuration standard */
          /* configuration des formats individuels */
          /*  configuration standard pour GECANTAUR */
          dv1_scpy (x01_status1.ver, (STRING) "SIR", 3);

          vct_hmvl.version = VERSION_SIR;
          vct_hmvl.flag = FALSE;
          vct_hmvl.cd_canal = 2;
          vct_hmvl.CC = TRUE;          /* capteur                */
          vct_hmvl.VT = FALSE;         /* vitesse                */
          vct_hmvl.LG = FALSE;         /* longueur               */
          vct_hmvl.TT = FALSE;         /* temps d'occupation     */
          vct_hmvl.TIV = FALSE;        /* temps intervehiculaire */
          vct_hmvl.SIL = FALSE;        /* silhouette             */
          vct_hmvl.PDS = FALSE;        /* poids                  */
          vct_hmvl.NE = FALSE;         /* nombre d'essieu        */
          vct_hmvl.PE = FALSE;         /* poids par essieu       */
          vct_hmvl.point = TRUE;
          vct_hmvl.blanc = FALSE;

          vct_hmvl.sz_QT[QT_V] = SZ_QT_V;
          vct_hmvl.sz_QT[QT_B] = SZ_QT_B;
          vct_hmvl.sz_QT[QT_H] = SZ_QT_H;
          vct_hmvl.sz_QT[QT_J] = SZ_QT_J;

          vct_hmvl.st_bcl = 0;

          if (buffer[1] == '0')
            {
              vct_hmvl.st_def_min = 0x20;
              vct_hmvl.st_def_maj = 0x08;
            }
          else
            {
              vct_hmvl.st_def_min = 0;
              vct_hmvl.st_def_maj = 0;
            }
          vct_hmvl.st_btr = TRUE;
          vct_hmvl.st_cks = TRUE;
        }
      else
        {
          /* erreur de syntaxe */
          flag_err = 1;
        }
    }
  else
    if ((taille == 3) && (buffer[0] == 'G') && (buffer[1] == 'E')
        && (buffer[2] == 'C'))
    {
      /*  configuration standard pour GECANTAUR */
      dv1_scpy (x01_status1.ver, (STRING) "GEC", 3);
      vct_hmvl.version = VERSION_GEC;

      vct_hmvl.flag = TRUE;
      vct_hmvl.cd_canal = 2;
      vct_hmvl.CC = FALSE;             /* capteur                */
      vct_hmvl.VT = TRUE;              /* vitesse                */
      vct_hmvl.LG = TRUE;              /* longueur               */
      vct_hmvl.TT = FALSE;             /* temps d'occupation     */
      vct_hmvl.TIV = TRUE;             /* temps intervehiculaire */
      vct_hmvl.SIL = FALSE;            /* silhouette             */
      vct_hmvl.PDS = FALSE;            /* poids                  */
      vct_hmvl.NE = FALSE;             /* nombre d'essieu        */
      vct_hmvl.PE = FALSE;             /* poids par essieu       */
      vct_hmvl.point = TRUE;
      vct_hmvl.blanc = FALSE;

      vct_hmvl.sz_QT[QT_V] = SZ_QT_V;
      vct_hmvl.sz_QT[QT_B] = 4;
      vct_hmvl.sz_QT[QT_H] = SZ_QT_H;
      vct_hmvl.sz_QT[QT_J] = SZ_QT_J;

      vct_hmvl.st_bcl = 0x08;
      vct_hmvl.st_def_min = 0;
      vct_hmvl.st_def_maj = 8;
      vct_hmvl.st_btr = FALSE;
      vct_hmvl.st_cks = FALSE;

    }
  else
    if ((taille == 3) && (buffer[0] == 'E') && (buffer[1] == 'S')
        && (buffer[2] == 'C'))
    {
      /*  configuration standard pour ESCOTA */
      dv1_scpy (x01_status1.ver, (STRING) "ESC", 3);
      vct_hmvl.version = VERSION_ESC;

      vct_hmvl.flag = FALSE;
      vct_hmvl.cd_canal = 2;
      vct_hmvl.CC = TRUE;              /* capteur                */
      vct_hmvl.VT = FALSE;             /* vitesse                */
      vct_hmvl.LG = FALSE;             /* longueur               */
      vct_hmvl.TT = FALSE;             /* temps d'occupation     */
      vct_hmvl.TIV = FALSE;            /* temps intervehiculaire */
      vct_hmvl.SIL = FALSE;            /* silhouette             */
      vct_hmvl.PDS = FALSE;            /* poids                  */
      vct_hmvl.NE = FALSE;             /* nombre d'essieu        */
      vct_hmvl.PE = FALSE;             /* poids par essieu       */
      vct_hmvl.point = FALSE;
      vct_hmvl.blanc = TRUE;

      vct_hmvl.sz_QT[QT_V] = SZ_QT_V;
      vct_hmvl.sz_QT[QT_B] = SZ_QT_B;
      vct_hmvl.sz_QT[QT_H] = SZ_QT_H;
      vct_hmvl.sz_QT[QT_J] = SZ_QT_J;

      vct_hmvl.st_bcl = 0;
      vct_hmvl.st_def_min = 0;
      vct_hmvl.st_def_maj = 8;
      vct_hmvl.st_btr = TRUE;
      vct_hmvl.st_cks = TRUE;

    }
  else
    if ((taille == 3) && (buffer[0] == 'G') && (buffer[1] == 'E')
        && (buffer[2] == 'N'))
    {
      /*  configuration standard pour GENEVE */
      dv1_scpy (x01_status1.ver, (STRING) "GEN", 3);
      vct_hmvl.version = VERSION_GEN;

      vct_hmvl.flag = FALSE;
      vct_hmvl.cd_canal = 2;
      vct_hmvl.CC = TRUE;              /* capteur                */
      vct_hmvl.VT = FALSE;             /* vitesse                */
      vct_hmvl.LG = FALSE;             /* longueur               */
      vct_hmvl.TT = FALSE;             /* temps d'occupation     */
      vct_hmvl.TIV = FALSE;            /* temps intervehiculaire */
      vct_hmvl.SIL = FALSE;            /* silhouette             */
      vct_hmvl.PDS = FALSE;            /* poids                  */
      vct_hmvl.NE = FALSE;             /* nombre d'essieu        */
      vct_hmvl.PE = FALSE;             /* poids par essieu       */
      vct_hmvl.point = TRUE;
      vct_hmvl.blanc = FALSE;

      vct_hmvl.sz_QT[QT_V] = SZ_QT_V;
      vct_hmvl.sz_QT[QT_B] = SZ_QT_B;
      vct_hmvl.sz_QT[QT_H] = SZ_QT_H;
      vct_hmvl.sz_QT[QT_J] = SZ_QT_J;

      vct_hmvl.st_bcl = 0x04;
      vct_hmvl.st_def_min = 0;
      vct_hmvl.st_def_maj = 8;
      vct_hmvl.st_btr = FALSE;
      vct_hmvl.st_cks = FALSE;

    }

  else
    {
      i = dv1_car (buffer, taille, '=');

      if ((i == taille) || (i < 2))
        flag_err = 1;
      else
        {
          if (dv1_scmp (buffer, (STRING) "CAN", 3))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  if ((val == 1) || (val == 2))
                    vct_hmvl.cd_canal = (UINT8) val;
                  else
                    flag_err = 1;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "POINT", 5))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  if (val == 0)
                    vct_hmvl.point = FALSE;
                  else if (val == 1)
                    vct_hmvl.point = TRUE;
                  else
                    flag_err = 1;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "ST_BCL", 6))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  switch (val)
                    {
                    case 0:
                      vct_hmvl.st_bcl = 0;
                      break;
                    case 0x04:
                      vct_hmvl.st_bcl = 4;
                      break;
                    case 0x08:
                      vct_hmvl.st_bcl = 8;
                      break;
                    default:
                      flag_err = 1;
                      break;
                    }
                }
            }
          else if (dv1_scmp (buffer, (STRING) "BLANC", 5))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val))
                {
                  flag_err = 1;
                }
              else
                {
                  if (val == 0)
                    vct_hmvl.blanc = FALSE;
                  else if (val == 1)
                    vct_hmvl.blanc = TRUE;
                  else
                    flag_err = 1;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "HMVL", 4))
            {
              fini = FALSE;
              i++;
              /* tant que on est pas au bout du buffer, on continue le decodage */
              while (!fini)
                {
                  /* on cherche le caractere / qui indique la fin du parametre */
                  j = dv1_car (&buffer[i], taille - i, '/');

                  if (j != 2)
                    {
                      /* on genere une erreur de syntaxe */
                      flag_err = 1;
                      fini = TRUE;
                    }
                  else
                    {
                      if (dv1_scmp (&buffer[i], "CC", 2))
                        {
                          vct_hmvl.CC = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                      else if (dv1_scmp (&buffer[i], "VT", 2))
                        {
                          vct_hmvl.VT = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                      else if (dv1_scmp (&buffer[i], "LG", 2))
                        {
                          vct_hmvl.LG = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                      else if (dv1_scmp (&buffer[i], "TT", 2))
                        {
                          vct_hmvl.TT = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                      else if (dv1_scmp (&buffer[i], "TI", 2))
                        {
                          vct_hmvl.TIV = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                      else if (dv1_scmp (&buffer[i], "KK", 2))
                        {
                          vct_hmvl.SIL = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                      else if (dv1_scmp (&buffer[i], "PP", 2))
                        {
                          vct_hmvl.PDS = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                      else if (dv1_scmp (&buffer[i], "NE", 2))
                        {
                          vct_hmvl.NE = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                      else if (dv1_scmp (&buffer[i], "PE", 2))
                        {
                          vct_hmvl.PE = TRUE;
                          vct_hmvl.flag = TRUE;
                        }
                    }
                  i += j + 1;
                  /* on decale l'index de lecture */
                  if (i >= taille)
                    fini = TRUE;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "SZ_QT", 5))
            {
              i++;
              fini = FALSE;
              k = 0;
              /* tant que on est pas au bout du buffer, on continue le decodage */
              while (!fini)
                {
                  /* on cherche le caractere / qui indique la fin du parametre */
                  j = dv1_car (&buffer[i], taille - i, '/');

                  if (j != 1)
                    {
                      /* on genere une erreur de syntaxe */
                      flag_err = 1;
                      fini = TRUE;
                    }
                  else
                    {
                      /* le parametre est il un nombre */
                      if (!dv1_atoi (&buffer[i], j, &val))
                        {
                          flag_err = 1;
                        }
                      else
                        {
                          if (val > 9)
                            flag_err = 1;
                          else
                            vct_hmvl.sz_QT[k++] = (UINT8) val;
                          if (k == NB_QT)
                            fini = TRUE;
                        }
                    }
                  i += j + 1;
                  /* on decale l'index de lecture */
                  if (i >= taille)
                    fini = TRUE;
                }
            }
        }
    }


  if (flag_err != 0)
    memcpy ((STRING) & vct_hmvl, (STRING) & sv_vct_hmvl,
            sizeof (struct vct_hmvl));

  *reinit = TRUE;

  return (flag_err);
}

#endif

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst1_par_alarme                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 27/12/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des parametres d'alarme                   /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/                     buffer = buffer comprenant le parametre a traiter       /
/                     taille = taille du buffer                               /
/                     voie   = numero du detecteur voulu ou 99 tous les canaux    /
/-------------------------------------------------------------------------DOC*/

INT lcr_tst1_par_alarme (buffer, taille)
     STRING buffer;             /* buffer contenant le parametre et sa valeur              */
     INT taille;                /* taille du buffer                                        */

{
  INT i;                        /* variable de boucle */
  INT j;
  INT k;
  INT l;                        /* variable de boucle */
  UINT8 fini;
  INT val1, val2;               /* valeur entiere pour la conversion chaine --> entier */
  INT flag_err;                 /* flag d'erreur ds l'analyse de la question */
/* on suppose pas d'erreur */
  flag_err = 0;

  if (taille == 1)
    {
      if ((buffer[0] == 'S') || (buffer[0] == 'Z'))
        {
          /*  configuration standard */
          /* configuration des formats individuels */
          for (i = 0; i < NB_ALARME; i++)
            {
              vct_cf_alarme[i].type = 0;
              vct_cf_alarme[i].num_tor = 0;
              vct_cf_alarme[i].num_log = 0;
              vct_cf_alarme[i].N_SAT = 0;       /* nombre de sequence avant saturation    */
              vct_cf_alarme[i].S_Q = 0; /* seuil QT                               */
              vct_cf_alarme[i].N_Q = 0; /* nombre de sequence QT                  */
              vct_cf_alarme[i].S_T = 0; /* seuil TT                               */
              vct_cf_alarme[i].N_T = 0; /* nombre de sequence TT                  */
              vct_cf_alarme[i].S_V = 0; /* seuil VT                               */
              vct_cf_alarme[i].N_V = 0; /* nombre de sequence VT                  */

            }
        }
      else
        {
          /* erreur de syntaxe */
          flag_err = 1;
        }
    }
  else
    {

      if (taille < 3)
        flag_err = 1;
      else
        {
          i = dv1_car (buffer, taille, '=');
          if (dv1_scmp (buffer, (STRING) "PWR", 3))
            {
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val1))
                {
                  flag_err = 1;
                }
              else
                {
                  if (val1 < NB_CARTE * 16)
                    {
                      fini = FALSE;
                      j = 0;
                      while ((j < NB_ALARME) && (!fini))
                        {
                          if ((vct_cf_alarme[j].type == ALARME_EDF) ||
                              (vct_cf_alarme[j].type == ALARME_NULLE))
                            fini = TRUE;
                          else
                            j++;
                        }
                      if (j < NB_ALARME)
                        {
                          vct_cf_alarme[j].type = ALARME_EDF;
                          vct_cf_alarme[j].num_tor = (UINT8) val1;
                        }
                    }
                  else
                    flag_err = 1;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "DET", 3))
            {
              i = dv1_car (buffer, taille, '=');
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[3], i - 3, &val1))
                {
                  flag_err = 1;
                }
              else
                {
                  if (val1 < NBDET)
                    {
                      /* le parametre est il un nombre */
                      if (!dv1_atoi (&buffer[i + 1], taille - i - 1, &val2))
                        {
                          flag_err = 1;
                        }
                      else
                        {
                          if (val2 < NB_CARTE * 16)
                            {
                              fini = FALSE;
                              j = 0;
                              while ((j < NB_ALARME) && (!fini))
                                {
                                  if (((vct_cf_alarme[j].type == ALARME_DET)
                                       && (vct_cf_alarme[j].num_log == val1))
                                      || (vct_cf_alarme[j].type ==
                                          ALARME_NULLE))
                                    fini = TRUE;
                                  else
                                    j++;
                                }
                              if (j < NB_ALARME)
                                {
                                  vct_cf_alarme[j].type = ALARME_DET;
                                  vct_cf_alarme[j].num_log = val1;
                                  vct_cf_alarme[j].num_tor = val2;
                                }
                            }
                          else
                            flag_err = 1;
                        }
                    }
                  else
                    flag_err = 1;
                }
            }
          else if (dv1_scmp (buffer, (STRING) "TRF", 3))
            {
              i = dv1_car (buffer, taille, '=');
              /* le parametre est il un nombre */
              if (!dv1_atoi (&buffer[3], i - 3, &val1))
                {
                  flag_err = 1;
                }
              else
                {
                  if (val1 < NBCANAL)
                    {
                      fini = FALSE;
                      j = 0;
                      while ((j < NB_ALARME) && (!fini))
                        {
                          if (((vct_cf_alarme[j].type == ALARME_TRAFIC) &&
                               (vct_cf_alarme[j].num_log == val1)) ||
                              (vct_cf_alarme[j].type == ALARME_NULLE))
                            fini = TRUE;
                          else
                            j++;
                        }
                      if (j < NB_ALARME)
                        {
                          vct_cf_alarme[j].type = ALARME_TRAFIC;
                          vct_cf_alarme[j].num_log = val1;
                          i++;
                          fini = FALSE;
                          l = 0;
                          /* tant que on est pas au bout du buffer, on continue le decodage */
                          while (!fini)
                            {
                              /* on cherche le caractere / qui indique la fin du parametre */
                              k = dv1_car (&buffer[i], taille - i, '/');
                              /* le parametre est il un nombre */
                              if (!dv1_atoi (&buffer[i], k, &val2))
                                {
                                  flag_err = 1;
                                }
                              else
                                {
                                  switch (l)
                                    {
                                    case 0:
                                      /* numero de sortie TOR */
                                      if (val2 >= NB_CARTE * 16)
                                        flag_err = 1;
                                      else
                                        vct_cf_alarme[j].num_tor = val2;

                                      l++;
                                      break;
                                    case 1:
                                      if (val2 > 250)
                                        flag_err = 1;
                                      else
                                        vct_cf_alarme[j].N_SAT = (UINT8) val2;
                                      l++;
                                      break;
                                    case 2:
                                      vct_cf_alarme[j].S_Q = val2;
                                      l++;
                                      break;
                                    case 3:
                                      if (val2 > 250)
                                        flag_err = 1;
                                      else
                                        vct_cf_alarme[j].N_Q =
                                          (UINT8) (val2 > 0 ? val2 : 1);
                                      l++;
                                      break;
                                    case 4:
                                      if (val2 > 99)
                                        flag_err = 1;
                                      else
                                        vct_cf_alarme[j].S_T = val2;
                                      l++;
                                      break;
                                    case 5:
                                      if (val2 > 250)
                                        flag_err = 1;
                                      else
                                        vct_cf_alarme[j].N_T =
                                          (UINT8) (val2 > 0 ? val2 : 1);
                                      l++;
                                      break;
                                    case 6:
                                      if (val2 > 250)
                                        flag_err = 1;
                                      else
                                        vct_cf_alarme[j].S_V = val2;
                                      l++;
                                      break;
                                    case 7:
                                      if (val2 > 250)
                                        flag_err = 1;
                                      else
                                        vct_cf_alarme[j].N_V =
                                          (UINT8) (val2 > 0 ? val2 : 1);
                                      fini = TRUE;
                                      break;
                                    }
                                }
                              i += k + 1;
                              /* on decale l'index de lecture */
                              if (i >= taille)
                                {
                                  if (fini == FALSE)
                                    flag_err = 1;
                                  fini = TRUE;
                                }
                            }
                        }
                    }
                  else
                    flag_err = 1;
                }
            }
        }
    }
  return (flag_err);
}
