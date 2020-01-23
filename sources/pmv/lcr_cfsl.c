/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_pmv10                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/02/1991                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la con- /
/ figuration et l'activation des panneaux                                     /
/                                                                             /
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
#include "standard.h"                  /* redefinition des types                         */

#include "mon_inc.h"
#include "xdg_def.h"                   /* definition des structures globales             */
#include "xdg_str.h"                   /* definition des variables globales              */
#include "xdg_var.h"                   /* definition des structures globales             */
#include "ted_prot.h"

#include "sir_dv1h.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "tac_conf.h"

#include "sir_dv1h.h"
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales              */
#include "lcr_cfsl.h"

#define CFSL_NB_INFO_CELLULE 3

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

LOCAL BYTE cmd_exec_cfs (struct cmd_cfs *);
LOCAL VOID lcr_cmd_cfs (STRING, INT, INT);

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_cfsl_init                                           /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 16/08/1999                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cfsl.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : initialisation des seuils de luminosite                /
/-------------------------------------------------------------------------DOC*/
VOID lcr_cfsl_init ()
{
  INT j;
  pip_nb_seuil_cellule = NB_SEUIL_CELLULE;
  for (j = 0; j < NB_SEUIL_CELLULE; j++)
    {
      pip_cf_seuil_cellule[j].lux = pip_seuil_cellule[j].lux;
      pip_cf_seuil_cellule[j].tens_diode = pip_seuil_cellule[j].tens_diode;
      pip_cf_seuil_cellule[j].cell = pip_seuil_cellule[j].cell;
    }
/* on determine les seuils de nuit jours et surbrillance */
  for (j = pip_nb_seuil_cellule - 1; j >= 0; j--)
    {
      switch (pip_cf_seuil_cellule[j].cell)
        {
        case 2:
          pip_cf_tst_sc.num_seuil_surb = j;
          break;

        case 1:
          pip_cf_tst_sc.num_seuil_jour = j;
          break;

        case 0:
          pip_cf_tst_sc.num_seuil_nuit = j;
          break;
        }
    }
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_exec_cfs                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 16/03/1999                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_pmv0.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande CFS                     /
/-------------------------------------------------------------------------DOC*/
static BYTE cmd_exec_cfs (struct cmd_cfs *cmd_cfs)
{
  INT j;
  BYTE erreur;

  erreur = FALSE;


/* on raz le nombre de seuils configure */
  if (cmd_cfs->nb_cel % CFSL_NB_INFO_CELLULE == 0)
    {
      for (j = 0; (j < cmd_cfs->nb_cel / CFSL_NB_INFO_CELLULE) && (!erreur); j++)
        {
          pip_cf_seuil_cellule[pip_nb_seuil_cellule].lux =
            cmd_cfs->cellule[j * 6];
          pip_cf_seuil_cellule[pip_nb_seuil_cellule].tens_diode =
            (BYTE) cmd_cfs->cellule[j * CFSL_NB_INFO_CELLULE + 1];
          pip_cf_seuil_cellule[pip_nb_seuil_cellule].cell =
            (BYTE) cmd_cfs->cellule[j * CFSL_NB_INFO_CELLULE + 2];

          /* on controle la valeur du seuil avec le precedent */
          if (pip_nb_seuil_cellule > 0)
            {
              if (pip_cf_seuil_cellule[pip_nb_seuil_cellule].lux <=
                  pip_cf_seuil_cellule[pip_nb_seuil_cellule - 1].lux)
                erreur = TRUE;
            }
          /* il y a un seuil supplementaire */
          pip_nb_seuil_cellule++;
          if (pip_nb_seuil_cellule > NB_SEUIL_CELLULE)
            erreur = TRUE;
        }
      /* on determine les seuils de nuit jours et surbrillance */
      for (j = pip_nb_seuil_cellule - 1; j >= 0; j--)
        {
          switch (pip_cf_seuil_cellule[j].cell)
            {
            case 2:
              pip_cf_tst_sc.num_seuil_surb = j;
              break;

            case 1:
              pip_cf_tst_sc.num_seuil_jour = j;
              break;

            case 0:
              pip_cf_tst_sc.num_seuil_nuit = j;
              break;
            }
        }
    }
  return (erreur);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_cfs                                                /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 16/03/1999                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cfsl.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande CFS configuration des         /
/                      seuils de luminosite                                   /
/-------------------------------------------------------------------------DOC*/
LOCAL VOID lcr_cmd_cfs (STRING buffer, INT nbcar, INT las)
{

  INT val;
  BYTE erreur;
  STRING ptr_deb;
  STRING ptr_fin;
  struct cmd_cfs cmd_cfs;

/* on initialise la structure de demande de configuration */
  cmd_cfs.flag = FALSE;
  cmd_cfs.nb_cel = 0;

/* le dernier caractere est nul */
  buffer[nbcar] = 0;

  ptr_deb = buffer;
  ptr_fin = buffer;

  erreur = FALSE;

  while (((INT) (ptr_fin - buffer) <= nbcar) && (!erreur))
    {
      switch (*ptr_fin)
        {

        case 0:
        case ' ':
          /* si deux separateur ne se suivent pas */
          if (ptr_deb != ptr_fin)
            {
              if (*ptr_deb == 'S')
                {
                  if (++ptr_deb >= ptr_fin)
                    {
                      erreur = FALSE;
                      lcr_cfsl_init ();
                    }
                  else
                    erreur = TRUE;
                }
              else if (*ptr_deb == 'Z')
                {
                  if (++ptr_deb >= ptr_fin)
                    {
                      erreur = FALSE;
                      pip_nb_seuil_cellule = 0;
                    }
                  else
                    erreur = TRUE;
                }
              else if (dv1_scmp (ptr_deb, "LU=", 0))
                {
                  cmd_cfs.flag = TRUE;
                  cmd_cfs.nb_cel = 0;
                  ptr_deb += 3;
                  /* a priori il n'y a pas d'erreur */
                  erreur = FALSE;
                  while (ptr_deb < ptr_fin)
                    {
                      /* on recherche la liste des parametres */
                      if (dv1_str_atoi (&ptr_deb, &val))
                        {
                          /* le numero d'entree/sortie */
                          cmd_cfs.cellule[cmd_cfs.nb_cel++] = (UINT) val;
                        }
                      else
                        erreur = TRUE;

                      if (cmd_cfs.nb_cel > NB_SEUIL_CELLULE * CFSL_NB_INFO_CELLULE)
                        erreur = TRUE;
                    }

                  /* si il y a une erreur */
                  if (erreur)
                    x01_cptr.erreur = CPTRD_SYNTAXE;
                }

            }
          /* on passe au parametre suivant */
          ptr_fin++;

          /* on supprime tous les blancs */
          while (*ptr_fin == ' ')
            ptr_fin++;

          /* sur le parametre suivant */
          ptr_deb = ptr_fin;
          break;

        default:
          ptr_fin++;
          break;
        }
    }
  if (erreur)
    x01_cptr.erreur = CPTRD_SYNTAXE;

/* si on a deja une commande */
  if ((x01_cptr.erreur == CPTRD_OK) && (cmd_cfs.flag))
    {
      /* on traite la commande */
      erreur = cmd_exec_cfs (&cmd_cfs);
    }

  if (erreur)
    {
      x01_cptr.erreur = CPTRD_SYNTAXE;

      /* on restaure la config */
      tac_conf_cfg_lec_conf ();

    }

/* on signale que la config a change */
  x01_cptr.config = TEMPO_CONFIG;

}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_p                                                  /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/12/96                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_p.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : commande de positionnement d'un panneau                /
/-------------------------------------------------------------------------DOC*/

INT lcr_cfsl (INT las, INT mode, INT lg_mess, STRING buffer,
              INT position, T_usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
  INT i, j, k;                  /* variable de boucle lecture du buffer                  */
  INT config = FALSE;
/* on se place apres le nom de la commande */
  i = position + 4;

  x01_cptr.erreur = CPTRD_OK;

/* si on est pas en fin de message, on continue l'analyse */
  if (i < lg_mess)
    {
      /* on traite la commande */
      lcr_cmd_cfs (&buffer[i], lg_mess - i, las);
      if (x01_cptr.erreur == CPTRD_OK)
        {
          config = TRUE;
          j = 0;

          /* on initialise le numero de bloc de reponse */
          j = dv1_scpy (&buffer[j], "CFSL OK", 0);
          if (TRUE == flg_fin)
            {
              buffer[j++] = '\n';
              buffer[j++] = '\r';
            }
          tedi_send_bloc (las, mode, buffer, j, *bloc,
                          (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
        }
      else
        tedi_erreur (las, mode);
    }
  else
    {
      /* on traite l'etat des panneaux */
      i = 0;
      j = 0;

      /* on initialise le numero de bloc de reponse */

      j += dv1_scpy (&buffer[j], "CFSL\n\r", 0);

      for (k = 0; k < pip_nb_seuil_cellule; k++)
        {
          j +=
            sprintf (&buffer[j], (STRING) "%05d %d-%d\n\r",
                         (UINT) pip_cf_seuil_cellule[k].lux,
                         (UINT) pip_cf_seuil_cellule[k].tens_diode,
                         pip_cf_seuil_cellule[k].cell);
        }
      if (TRUE == flg_fin)
        {
          buffer[j++] = '\n';
          buffer[j++] = '\r';
        }
      tedi_send_bloc (las, mode, buffer, j, *bloc,
                      (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
    }
  return config;
}
