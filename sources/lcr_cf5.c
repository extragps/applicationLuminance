/*
 * lcr_cf5.c
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_cf5.c,v $
 * Revision 1.2  2018/06/04 08:38:38  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include "string.h"
#include "standard.h"
#include "mon_inc.h"
#include "define.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "ted_prot.h"
#include "rec_util.h"
#include "lcr_cf5h.h"
#include "identLib.h"

 /**********************************************************/
 /*                                                        */
 /*            prototypage des fonctions                   */
 /*                                                        */
 /**********************************************************/

/* *************************************
 * PROTOTYPE DES FONCTIONS LOCALES
 * ************************************* */

static INT par_sy (STRING, INT);
static INT par_cfal_sy (STRING, INT);

/* *************************************
 * FONCTIONS DU MODULE
 * ************************************* */

static void raz_alerte(T_vct_critere *critere)
{
                  critere->nature = N_AFF;
                  critere->relation = N_AFF;
                  critere->seuil = (UINT) NIL;
                  critere->oplog = OP_OR;
}

static void raz_sy()
{
int j;
          vct_cf_al_sy.flag = FALSE;
          for (j = 0; j < MAX_CRT_SY; j++)
          {
             raz_alerte(&vct_cf_al_sy.critere[j]);
          }
}

/*DOC-------------------------------------------------------------------------/ / S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_sy                                                 /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 10/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf5.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des parametres alertes systeme            /
/-------------------------------------------------------------------------DOC*/

static INT par_sy (STRING buffer, INT taille)
{
  INT i = 0;                    /* variable de boucle */
  INT flag_err = 0;             /* flag d'erreur ds l'analyse de la question */
  INT j;
  UINT8 nature;
  LONG seuil = NIL;
  UINT8 relation = N_AFF;
  UINT8 oplog = N_AFF;
  UINT8 fini;

  if (taille < 3)
    {
      flag_err = 1;
    }
  else
    {

      nature = N_AFF;
      if (dv1_scmp (&buffer[i], (STRING) "CKS", 3))
        {
          nature = AL_CKS;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "EDF", 3))
        {
          nature = AL_EDF;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "GAR", 3))
        {
          nature = AL_GAR;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "RST", 3))
        {
          nature = AL_RST;
        }
      if (dv1_scmp (&buffer[i], (STRING) "INI", 3))
        {
          nature = AL_INI;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "TRM", 3))
        {
          nature = AL_TRM;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "ERR", 3))
        {
          nature = AL_ERR;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "ER1", 3))
        {
          nature = AL_ER1;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "ER2", 3))
        {
          nature = AL_ER2;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "ER3", 3))
        {
          nature = AL_ER3;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "BTR", 3))
        {
          nature = AL_BTR;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "BCL", 3))
        {
          nature = AL_BCL;
        }

      if (nature == N_AFF)
        flag_err = 1;

      if (flag_err == 0)
        {
          i += 3;
          switch (buffer[i])
            {
            case '>':
              if ((buffer[i + 1] == '>') && (i < taille - 1))
                {
                  relation = REL_SEUIL_SUP;
                  i += 2;
                }
              else
                {
                  relation = REL_SUP;
                  i++;
                }
              break;
            case '<':
              if ((buffer[i + 1] == '<') && (i < taille - 1))
                {
                  relation = REL_SEUIL_INF;
                  i += 2;
                }
              else
                {
                  relation = REL_INF;
                  i++;
                }
              break;
            case '=':
              if ((buffer[i + 1] == '>') && (i < taille - 1))
                {
                  relation = REL_DIFF;
                  i += 2;
                }
              else
                {
                  relation = REL_EGA;
                  i++;
                }
              break;
            }

          /* seuil */
          if (i < taille)
            {
              j = dv1_num (&buffer[i], taille - i);
              if (j != 0)
                {
                  if (REL_DIFF == relation)
                    {
                      /* Il y a erreur dans ce cas.   */
                      flag_err = 1;
                    }
                  else
                    {
                      dv1_atol (&buffer[i], j, &seuil);
                      i += j;
                    }
                }
              else
                {
                  if (relation != REL_DIFF)
                    {
                      /* Il y a erreur dans ce cas.   */
                      flag_err = 1;
                    }
                }
            }

          /* operateur logique */
          while (i < taille - 1)
            i++;

          if (i < taille)
            {
              if (buffer[i] == '&')
                {
                  oplog = OP_AND;
                }
			  else if (buffer[i] == '|')
                {
                  oplog = OP_OR;
                }
            }
        }
      if (flag_err == 0)
        {
          j = 0;
          fini = FALSE;
          while ((j < MAX_CRT_SY) && (!fini))
            {
              if ((vct_cf_al_sy.critere[j].nature == nature) ||
                  (vct_cf_al_sy.critere[j].nature == N_AFF))
                fini = TRUE;
              else
                j++;
            }

          vct_cf_al_sy.flag = TRUE;

          if (nature != N_AFF)
            {
              vct_cf_al_sy.critere[j].nature = nature;
            }
          /* si il y a une relation */
          if (relation != N_AFF)
            {
              vct_cf_al_sy.critere[j].relation = relation;
            }
          /* si il y a un seuil */
          if (seuil != NIL)
            {
              vct_cf_al_sy.critere[j].seuil = (UINT) seuil;
            }
          /* si il y a un operateur logique */
          if (oplog != N_AFF)
            {
              vct_cf_al_sy.critere[j].oplog = oplog;
            }

          /* on controle le critere */

          if ((vct_cf_al_sy.critere[j].nature == N_AFF) ||
              (vct_cf_al_sy.critere[j].relation == N_AFF) ||
              ((vct_cf_al_sy.critere[j].seuil == (UINT) NIL) &&
               (vct_cf_al_sy.critere[j].relation != REL_DIFF)))
            {
              flag_err = 1;
            }
          else
            {
              vct_cf_al_sy.critere[j].etat = REL_ETAT_IND;
            }
        }
    }

  return (flag_err);
}


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_cf_al_sy                                           /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 12/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf5.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : traitement des parametres de la commande lcr_cf_al     /
/-------------------------------------------------------------------------DOC*/
static INT par_cfal_sy (STRING buffer, INT lg_par)
{
  INT i = 0;                    /* indice de lecture de buffer               */
  INT flag_err = 0;             /* drapeau de detection d'erreur             */

  /* le premier caractere de buffer correspond au debut du 1er parametre */
  /* si on est pas au bout du buffer, on continue l'analyse */
  if((buffer [i]=='Z')||(buffer [i]=='S'))
  {
	raz_sy();
  }
  else if (i < lg_par)
    {
      INT fini = FALSE;

      while (!fini)
        {
          /* on recherche la fin du parametre et de l'operateur logique */
          INT taille = dv1_car (&buffer[i], lg_par - i, (INT) (' '));
          INT j = taille;
          while ((i + j < lg_par) && (!fini))
            {
              switch (buffer[i + j])
                {
                case ' ':
                  j++;
                  break;
                case '&':
				case '|':
                  taille = j + 1;
                  fini = TRUE;
                  break;
                default:
                  fini = TRUE;
                  break;
                }
            }

          flag_err = par_sy (&buffer[i], taille);

          fini = (flag_err != 0 ? TRUE : FALSE);

          /* on passe au parametre suivant */
          i += taille;

          /* on elimine les separateurs */
          if ((i += dv1_separateur (&buffer[i], lg_par - i)) >= lg_par)
            {
              fini = TRUE;
            }
        }                              /* endwhile (!fini) */
    }                                  /* endif (i < lg_par) */
  return (flag_err);
}

#ifdef RAD
/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_my                                                 /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 10/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf5.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des parametres alertes individuelles      /
/-------------------------------------------------------------------------DOC*/

static INT par_my (STRING buffer, INT taille)
{

  INT i;                        /* variable de boucle */
  INT flag_err;                 /* flag d'erreur ds l'analyse de la question */
  INT j;
  INT num_canal;
  INT nb_canal;
  UINT8 nature = 0;
  LONG seuil;
  UINT8 relation;
  UINT8 oplog;
  UINT8 fini;
  UINT8 trv;

  /* on suppose pas d'erreur */
  flag_err = 0;

  num_canal = N_AFF;
  seuil = NIL;
  relation = N_AFF;
  oplog = N_AFF;

  if (taille < 3)
    flag_err = 1;
  else
    {
      i = dv1_num (buffer, taille);
      if (i == 0)
        {
          if (buffer[0] == '*')
            {
              /* tous les canaux */
              num_canal = N_AFF;
              i = 1;
            }
          else
            {
              flag_err = 1;
            }
        }
      else
        {
          dv1_atoi (buffer, i, &num_canal);
          if (num_canal >= NBCANAL)
            flag_err = 1;
        }
      if (flag_err == 0)
        {
          nature = N_AFF;
          if (dv1_scmp (&buffer[i], (STRING) "QT", 2))
            {
              nature = N_QT;
            }
          else if (dv1_scmp (&buffer[i], (STRING) "TT", 2))
            {
              nature = N_TO;
            }
          else if (dv1_scmp (&buffer[i], (STRING) "VT", 2))
            {
              nature = N_VT;
            }
          else if (dv1_scmp (&buffer[i], (STRING) "QL", 2))
            {
              nature = N_QL;
            }

          if (nature == N_AFF)
            flag_err = 1;
        }
      if (flag_err == 0)
        {
          i += 2;
          switch (buffer[i])
            {
            case '>':
              if ((buffer[i + 1] == '>') && (i < taille - 1))
                {
                  relation = REL_SEUIL_SUP;
                  i += 2;
                }
              else
                {
                  relation = REL_SUP;
                  i++;
                }
              break;
            case '<':
              if ((buffer[i + 1] == '<') && (i < taille - 1))
                {
                  relation = REL_SEUIL_INF;
                  i += 2;
                }
              else
                {
                  relation = REL_INF;
                  i++;
                }
              break;
            case '=':
              if ((buffer[i + 1] == '>') && (i < taille - 1))
                {
                  relation = REL_DIFF;
                  i += 2;
                }
              else
                {
                  relation = REL_EGA;
                  i++;
                }
              break;
            }

          /* seuil */
          if (i < taille)
            {
              j = dv1_num (&buffer[i], taille - i);
              if (j != 0)
                {
                  if (REL_DIFF == relation)
                    {
                      /* Il y a erreur dans ce cas.   */
                      flag_err = 1;
                    }
                  else
                    {
                      dv1_atol (&buffer[i], j, &seuil);
                      i += j;
                    }
                }
              else
                {
                  if (relation != REL_DIFF)
                    {
                      /* Il y a erreur dans ce cas.   */
                      flag_err = 1;
                    }
                }
            }

          /* operateur logique */
          while (i < taille - 1)
            i++;

          if (i < taille)
            {
              if (buffer[i] == '&')
                {
                  oplog = OP_AND;
                }
            }
        }
      if (flag_err == 0)
        {
          if (num_canal == N_AFF)
            {
              num_canal = 0;
              nb_canal = NBCANAL;
            }
          else
            nb_canal = 1;

          trv = FALSE;

          for (i = num_canal; (i < num_canal + nb_canal) && (flag_err == 0);
               i++)
            {
              /* si le capteur est affecte */
              if (i < vct_nb_canal)
                {
                  /* il existe des canaux configures */
                  trv = TRUE;
                  /* on recherche le premier critere libre */
                  j = 0;
                  fini = FALSE;
                  while ((j < MAX_CRT_MY) && (!fini))
                    {
                      if ((vct_cf_al_my[i].critere[j].nature == nature) ||
                          (vct_cf_al_my[i].critere[j].nature == N_AFF))
                        fini = TRUE;
                      else
                        j++;
                    }

                  vct_cf_al_my[i].flag = TRUE;

                  if (nature != N_AFF)
                    {
                      vct_cf_al_my[i].critere[j].nature = nature;
                    }

                  /* si il y a une relation */
                  if (relation != N_AFF)
                    {
                      vct_cf_al_my[i].critere[j].relation = relation;
                    }
                  /* si il y a un seuil */
                  if (seuil != NIL)
                    {
                      vct_cf_al_my[i].critere[j].seuil = (UINT) seuil;
                    }
                  /* si il y a un operateur logique */
                  if (oplog != N_AFF)
                    {
                      vct_cf_al_my[i].critere[j].oplog = oplog;
                    }

                  /* on controle le critere */

                  if ((vct_cf_al_my[i].critere[j].nature == N_AFF) ||
                      (vct_cf_al_my[i].critere[j].relation == N_AFF) ||
                      (vct_cf_al_my[i].critere[j].seuil == (UINT) NIL))
                    {
                      flag_err = 1;
                    }
                  else
                    {
                      vct_cf_al_my[i].critere[j].etat = REL_ETAT_IND;
                    }
                }
            }
          if (trv == FALSE)
            flag_err = 2;
        }
    }

  return (flag_err);
}


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_cf_al_my                                           /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 12/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf5.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : traitement des parametres de la commande lcr_cf_al     /
/-------------------------------------------------------------------------DOC*/
static INT par_cfal_my (STRING buffer, INT lg_par)
{
  INT i;                        /* indice de lecture de buffer               */
  INT j;
  INT fini;                     /* variable de controle de boucle            */
  INT flag_err;                 /* drapeau de detection d'erreur             */
  INT taille;                   /* taille d'un parametre de detecteur        */
  /* on initialise i sur le premier caractere de buffer */
  i = 0;

  /* on suppose pas d'erreur */
  flag_err = 0;

  /* le premier caractere de buffer correspond au debut du 1er parametre */

  /* si on est pas au bout du buffer, on continue l'analyse */
  if (i < lg_par)
    {
      fini = FALSE;

      while (!fini)
        {
          /* on recherche la fin du parametre et de l'operateur logique */
          taille = dv1_car (&buffer[i], lg_par - i, (INT) (' '));
          j = taille;
          while ((i + j < lg_par) && (!fini))
            {
              switch (buffer[i + j])
                {
                case ' ':
                  j++;
                  break;
                case '&':
                  taille = j + 1;
                  fini = TRUE;
                  break;
                default:
                  fini = TRUE;
                  break;
                }
            }

          flag_err = par_my (&buffer[i], taille);

          /* si on a trouve une erreur, on termine le decodage */
          if (flag_err != 0)
            fini = TRUE;
          else
            fini = FALSE;

          /* on passe au parametre suivant */
          i += taille;

          /* on elimine les separateurs */
          if ((i += dv1_separateur (&buffer[i], lg_par - i)) >= lg_par)
            {
              fini = TRUE;
            }
        }
    }
  return (flag_err);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_ind                                                /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 10/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf5.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des parametres alertes individuelles      /
/-------------------------------------------------------------------------DOC*/

static INT par_ind (STRING buffer, INT taille)
{
  INT i;                        /* variable de boucle */
  INT flag_err;                 /* flag d'erreur ds l'analyse de la question */
  INT j;
  INT num_canal;
  INT nb_canal;
  UINT8 nature = 0;
  LONG seuil;
  UINT8 relation;
  UINT8 oplog;
  UINT8 fini;
  UINT8 trv;


  /* on suppose pas d'erreur */
  flag_err = 0;

  num_canal = N_AFF;
  seuil = NIL;
  relation = N_AFF;
  oplog = N_AFF;

  if (taille < 3)
    flag_err = 1;
  else
    {
      i = dv1_num (buffer, taille);
      if (i == 0)
        {
          if (buffer[0] == '*')
            {
              /* tous canaux */
              num_canal = N_AFF;
              i = 1;
            }
          else
            {
              flag_err = 1;
            }
        }
      else
        {
          dv1_atoi (buffer, i, &num_canal);
          if (num_canal >= NBCANAL)
            flag_err = 1;
        }
      if (flag_err == 0)
        {

          nature = N_AFF;
          if (dv1_scmp (&buffer[i], (STRING) "DI", 2))
            {
              nature = DI;
            }
          else if (dv1_scmp (&buffer[i], (STRING) "EI", 2))
            {
              nature = EI;
            }
          else if (dv1_scmp (&buffer[i], (STRING) "II", 2))
            {
              nature = II;
            }
          else if (dv1_scmp (&buffer[i], (STRING) "LI", 2))
            {
              nature = LI;
            }
          else if (dv1_scmp (&buffer[i], (STRING) "TI", 2))
            {
              nature = TI;
            }
          else if (dv1_scmp (&buffer[i], (STRING) "VI", 2))
            {
              nature = VI;
            }

          if (nature == N_AFF)
            flag_err = 1;
        }
      if (flag_err == 0)
        {
          i += 2;
          switch (buffer[i])
            {
            case '>':
              if ((buffer[i + 1] == '>') && (i < taille - 1))
                {
                  relation = REL_SEUIL_SUP;
                  i += 2;
                }
              else
                {
                  relation = REL_SUP;
                  i++;
                }
              break;
            case '<':
              if ((buffer[i + 1] == '<') && (i < taille - 1))
                {
                  relation = REL_SEUIL_INF;
                  i += 2;
                }
              else
                {
                  relation = REL_INF;
                  i++;
                }
              break;
            case '=':
              if ((buffer[i + 1] == '>') && (i < taille - 1))
                {
                  relation = REL_DIFF;
                  i += 2;
                }
              else
                {
                  relation = REL_EGA;
                  i++;
                }
              break;
            }

          /* seuil */

          if (i < taille)
            {
              j = dv1_num (&buffer[i], taille - i);
              if (j != 0)
                {
                  if (REL_DIFF == relation)
                    {
                      /* Il y a erreur dans ce cas.   */
                      flag_err = 1;
                    }
                  else
                    {
                      dv1_atol (&buffer[i], j, &seuil);
                      i += j;
                    }
                }
              else
                {
                  if (relation != REL_DIFF)
                    {
                      /* Il y a erreur dans ce cas.   */
                      flag_err = 1;
                    }
                }
            }
          /* operateur logique */
          while (i < taille - 1)
            i++;

          if (i < taille)
            {
              if (buffer[i] == '&')
                {
                  oplog = OP_AND;
                }
            }
        }
      if (flag_err == 0)
        {
          if (num_canal == N_AFF)
            {
              num_canal = 0;
              nb_canal = NBCANAL;
            }
          else
            nb_canal = 1;

          trv = FALSE;

          for (i = num_canal; (i < num_canal + nb_canal) && (flag_err == 0);
               i++)
            {
              /* si le canal est affecte */
              if (vct_canal[i].nb_capt != 0)
                {
                  /* il existe des capteurs configures */
                  trv = TRUE;
                  /* on recherche le premier critere libre */
                  j = 0;
                  fini = FALSE;
                  while ((j < MAX_CRT_IN) && (!fini))
                    {
                      if ((vct_cf_al_ind[i].critere[j].nature == nature) ||
                          (vct_cf_al_ind[i].critere[j].nature == N_AFF))
                        fini = TRUE;
                      else
                        j++;
                    }

                  vct_cf_al_ind[i].flag = TRUE;

                  if (nature != N_AFF)
                    {
                      vct_cf_al_ind[i].critere[j].nature = nature;
                    }
                  /* si il y a une relation */
                  if (relation != N_AFF)
                    {
                      vct_cf_al_ind[i].critere[j].relation = relation;
                    }
                  /* si il y a un seuil */
                  if (seuil != NIL)
                    {
                      vct_cf_al_ind[i].critere[j].seuil = (UINT) seuil;
                    }
                  /* si il y a un operateur logique */
                  if (oplog != N_AFF)
                    {
                      vct_cf_al_ind[i].critere[j].oplog = oplog;
                    }

                  /* on controle le critere */

                  if ((vct_cf_al_ind[i].critere[j].nature == N_AFF) ||
                      (vct_cf_al_ind[i].critere[j].relation == N_AFF) ||
                      (vct_cf_al_ind[i].critere[j].seuil == (UINT) NIL))
                    {
                      flag_err = 1;
                    }
                  else
                    {
                      vct_cf_al_ind[i].critere[j].etat = REL_ETAT_IND;
                    }
                }                      /* endif (vct_canal[i].nb_capt !=       */
            }                          /* endfor (i=num_canal;(i < num_c       */
          if (trv == FALSE)
            {
              flag_err = 2;
            }
        }
    }

  return (flag_err);
}


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_cf_al_ind                                          /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 12/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf5.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : traitement des parametres de la commande lcr_cf_al     /
/-------------------------------------------------------------------------DOC*/
static INT par_cfal_ind (STRING buffer, INT lg_par)
{
  INT i;                        /* indice de lecture de buffer               */
  INT j;
  INT fini;                     /* variable de controle de boucle            */
  INT flag_err;                 /* drapeau de detection d'erreur             */
  INT taille;                   /* taille d'un parametre de detecteur        */

  /* on initialise i sur le premier caractere de buffer */
  i = 0;

  /* on suppose pas d'erreur */
  flag_err = 0;

  /* le premier caractere de buffer correspond au debut du 1er parametre */

  /* si on est pas au bout du buffer, on continue l'analyse */
  if (i < lg_par)
    {
      fini = FALSE;

      while (!fini)
        {
          /* on recherche la fin du parametre et de l'operateur logique */
          taille = dv1_car (&buffer[i], lg_par - i, (INT) (' '));
          j = taille;
          while ((i + j < lg_par) && (!fini))
            {
              switch (buffer[i + j])
                {
                case ' ':
                  j++;
                  break;
                case '&':
                  taille = j + 1;
                  fini = TRUE;
                  break;
                default:
                  fini = TRUE;
                  break;
                }
            }

          flag_err = par_ind (&buffer[i], taille);

          /* si on a trouve une erreur, on termine le decodage */
          if (flag_err != 0)
            fini = TRUE;
          else
            fini = FALSE;

          /* on passe au parametre suivant */
          i += taille;

          /* on elimine les separateurs */
          if ((i += dv1_separateur (&buffer[i], lg_par - i)) >= lg_par)
            {
              fini = TRUE;
            }
        }
    }
  return (flag_err);
}

#endif

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_cf_al                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 10/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cfal.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande lcr_cfal                      /
/-------------------------------------------------------------------------DOC*/
INT cf5_cfal (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
              T_usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
  INT i, j, k;
  INT flag_err;
  INT fini;
#ifdef RAD
  struct vct_cf_al_ind sv_vct_cf_al_ind[NBCANAL];
  struct vct_cf_al_my sv_vct_cf_al_my[NBCANAL];
#endif
  struct vct_cf_al_sy sv_vct_cf_al_sy;
  INT config;

  /* on suppose qu'il n'y a pas d'erreur */
  flag_err = 0;
  config = FALSE;

  /* on se place apres le nom de la commande */
  i = position + 4;

  /* on elimine les separateurs */
  i += dv1_separateur (&buffer[i], lg_mess - i);

  /* si on est pas en fin de message, on continue l'analyse */
  if (i < lg_mess)
    {
      /* teste si l'operateur est habilite */
      if (identLireAutorisationAvecNiveau (las, 0))
        {
          config = TRUE;

          switch (buffer[i])
            {
#ifdef RAD
            case 'I':
              /* on sauvegarde la configuration des alertes individuelles */
              memcpy ((STRING) sv_vct_cf_al_ind, (STRING) vct_cf_al_ind,
                      NBCANAL * sizeof (struct vct_cf_al_ind));
              i++;
              /* alertes sur donnees individuelles */
              /* on elimine les separateurs */
              i += dv1_separateur (&buffer[i], lg_mess - i);
              flag_err = par_cfal_ind (&buffer[i], lg_mess - i);
              if (flag_err != 0)
                {
                  /* on restaure la configuration des alertes individuelles */
                  memcpy ((STRING) vct_cf_al_ind, (STRING) sv_vct_cf_al_ind,
                          NBCANAL * sizeof (struct vct_cf_al_ind));
                }
              break;

            case 'M':
              /* on sauvegarde la configuration des alertes moyennes */
              memcpy ((STRING) sv_vct_cf_al_my, (STRING) vct_cf_al_my,
                      NBCANAL * sizeof (struct vct_cf_al_my));
              i++;
              /* alertes sur donnees moyennes */
              /* on elimine les separateurs */
              i += dv1_separateur (&buffer[i], lg_mess - i);
              flag_err = par_cfal_my (&buffer[i], lg_mess - i);
              if (flag_err != 0)
                {
                  /* on restaure la configuration des alertes individuelles */
                  memcpy ((STRING) vct_cf_al_my, (STRING) sv_vct_cf_al_my,
                          NBCANAL * sizeof (struct vct_cf_al_my));
                }
              break;
#endif
            case 'Y':
              /* alertes sur donnees systeme      */
              /* on sauvegarde la configuration des alertes systeme */
              memcpy ((STRING) & sv_vct_cf_al_sy, (STRING) & vct_cf_al_sy,
                      sizeof (struct vct_cf_al_sy));
              i++;
              /* on elimine les separateurs */
              i += dv1_separateur (&buffer[i], lg_mess - i);
              flag_err = par_cfal_sy (&buffer[i], lg_mess - i);
              if (flag_err != 0)
                {
                  /* on restaure la configuration des alertes individuelles */
                  memcpy ((STRING) & vct_cf_al_sy, (STRING) & sv_vct_cf_al_sy,
                          sizeof (struct vct_cf_al_my));
                }
              break;

            case 'Z':
            case 'S':
#ifdef RAD
              /* suppression des conditions d'alerte */
              /* configuration des alertes */
              for (i = 0; i < NBCANAL; i++)
                {
                  vct_cf_al_ind[i].flag = FALSE;
                  for (j = 0; j < MAX_CRT_IN; j++)
                    {
                      vct_cf_al_ind[i].critere[j].nature = N_AFF;
                      vct_cf_al_ind[i].critere[j].relation = N_AFF;
                      vct_cf_al_ind[i].critere[j].seuil = (UINT) NIL;
                      vct_cf_al_ind[i].critere[j].oplog = OP_OR;
                    }
                }
              for (i = 0; i < NBCANAL; i++)
                {
                  vct_cf_al_my[i].flag = FALSE;
                  for (j = 0; j < MAX_CRT_MY; j++)
                    {
                      vct_cf_al_my[i].critere[j].nature = N_AFF;
                      vct_cf_al_my[i].critere[j].relation = N_AFF;
                      vct_cf_al_my[i].critere[j].seuil = (UINT) NIL;
                      vct_cf_al_my[i].critere[j].oplog = OP_OR;
                    }
                }
#endif
			  raz_sy();
              break;
            default:
              flag_err = 1;
              break;

            }
        }
      else
        flag_err = 3;
    }

  /* si on a pas trouve d'erreur */
  if (flag_err == 0)
    {
      /* on renvoie la configuration des alertes                    */
      j = 0;
      /* on initialise le numero de bloc de reponse */

      fini = FALSE;
#ifdef RAD
      /* configuration des alertes individuelles */
      memcpy (&buffer[j], (STRING) "CFAL I ", 7);
      j += 7;
      for (i = 0; (i < vct_nb_canal) && (!fini); i++)
        {
          k = 0;
          while ((vct_cf_al_ind[i].critere[k].nature != N_AFF)
                 && (k < MAX_CRT_IN) && (!fini))
            {
              char *chaine;
              if (j >= MAXLCR)
                {
                  tedi_send_bloc (las, mode, buffer, j, *bloc, FALSE,
                                  pt_mess);
                  if (++(*bloc) >= 10)
                    *bloc = 0;
                  j = 0;

                  if (vct_IT[las])
                    fini = TRUE;
                }
              /* le numero de capteur */
              j += dv1_itoa (&buffer[j], i, 0);

              /* la nature de mesure */
              chaine = rec_int_lire_type (vct_cf_al_ind[i].critere[k].nature);
              memcpy (&buffer[j], chaine, strlen (chaine));
              j += strlen (chaine);
              /* relation */
              chaine = rec_util_lire_relation
                (vct_cf_al_ind[i].critere[k].relation);
              memcpy (&buffer[j], chaine, strlen (chaine));
              j += strlen (chaine);
              if (REL_DIFF != vct_cf_al_ind[i].critere[k].relation)
                {
                  j += dv1_ltoa (&buffer[j],
                                 (LONG) (vct_cf_al_ind[i].critere[k].seuil),
                                 0);
                }

              /* si il y a un operateur logique */
              if ((k == MAX_CRT_IN - 1) ||
                  (vct_cf_al_ind[i].critere[k + 1].nature == N_AFF))
                vct_cf_al_ind[i].critere[k].oplog = OP_OR;

              if (vct_cf_al_ind[i].critere[k].oplog == OP_AND)
                {
                  buffer[j++] = ' ';
                  buffer[j++] = '&';
                }
              buffer[j++] = ' ';
              k++;
            }
        }
      j--;
      buffer[j++] = '\n';
      buffer[j++] = '\r';

      /* configuration des alertes moyennes  */
      memcpy (&buffer[j], (STRING) "CFAL M ", 7);
      j += 7;
      for (i = 0; i < vct_nb_canal; i++)
        {
          k = 0;

          while ((vct_cf_al_my[i].critere[k].nature != N_AFF) &&
                 (k < MAX_CRT_MY) && (fini == FALSE))
            {
              char *chaine;
              if (j >= MAXLCR)
                {
                  tedi_send_bloc (las, mode, buffer, j, *bloc, FALSE,
                                  pt_mess);
                  if (++(*bloc) >= 10)
                    *bloc = 0;
                  j = 0;

                  if (vct_IT[las])
                    fini = TRUE;
                }
              /* le numero de canal */
              j += dv1_itoa (&buffer[j], i, 0);

              /* la nature de mesure */
              chaine = rec_ing_lire_type (vct_cf_al_my[i].critere[k].nature);
              memcpy (&buffer[j], chaine, strlen (chaine));
              j += strlen (chaine);

              /* relation */
              chaine = rec_util_lire_relation
                (vct_cf_al_my[i].critere[k].relation);
              memcpy (&buffer[j], chaine, strlen (chaine));
              j += strlen (chaine);
              if (REL_DIFF != vct_cf_al_my[i].critere[k].relation)
                {
                  j += dv1_ltoa (&buffer[j],
                                 (LONG) (vct_cf_al_my[i].critere[k].seuil),
                                 0);
                }

              /* si il y a un operateur logique */
              if ((k == MAX_CRT_IN - 1) ||
                  (vct_cf_al_my[i].critere[k + 1].nature == N_AFF))
                vct_cf_al_my[i].critere[k].oplog = OP_OR;

              if (vct_cf_al_my[i].critere[k].oplog == OP_AND)
                {
                  buffer[j++] = ' ';
                  buffer[j++] = '&';
                }
              buffer[j++] = ' ';
              k++;
            }
        }
      j--;
      buffer[j++] = '\n';
      buffer[j++] = '\r';
#endif
      /* configuration des alertes systeme  */
      memcpy (&buffer[j], (STRING) "CFAL Y ", 7);
      j += 7;
      k = 0;

      while ((vct_cf_al_sy.critere[k].nature != N_AFF) &&
             (k < MAX_CRT_SY) && (fini == FALSE))
        {
          char *chaine;
          if (j >= MAXLCR)
            {
              tedi_send_bloc (las, mode, buffer, j, *bloc, FALSE, pt_mess);
              if (++(*bloc) >= 10)
                *bloc = 0;
              j = 0;

              if (vct_IT[las])
                fini = TRUE;
            }
          /* la nature de mesure */
          chaine = rec_util_lire_alerte (vct_cf_al_sy.critere[k].nature);
          memcpy (&buffer[j], chaine, strlen (chaine));
          j += strlen (chaine);
          /* relation */
          chaine = rec_util_lire_relation (vct_cf_al_sy.critere[k].relation);
          memcpy (&buffer[j], chaine, strlen (chaine));
          j += strlen (chaine);
          if (REL_DIFF != vct_cf_al_sy.critere[k].relation)
            {
              j +=
                dv1_ltoa (&buffer[j], (LONG) (vct_cf_al_sy.critere[k].seuil),
                          0);
            }
          /* si il y a un operateur logique */
          if ((k == MAX_CRT_IN - 1) ||
              (vct_cf_al_sy.critere[k + 1].nature == N_AFF))
            {
              vct_cf_al_sy.critere[k].oplog = OP_OR;
            }

          if (vct_cf_al_sy.critere[k].oplog == OP_AND)
            {
              buffer[j++] = ' ';
              buffer[j++] = '&';
            }
          buffer[j++] = ' ';
          k++;
        }
      j--;

      if (!vct_IT[las])
        {
          if (flg_fin == TRUE)
            {
              buffer[j++] = '\n';
              buffer[j++] = '\r';
              /* apres le dernier capteur on envoie
               * la reponse */
              tedi_send_bloc (las, mode, buffer, j, *bloc, FALSE, pt_mess);
            }
          else
            {
              /* apres le dernier capteur on envoie
               * la reponse */
              tedi_send_bloc (las, mode, buffer, j, *bloc, TRUE, pt_mess);
            }
          if (++(*bloc) >= 10)
            {
              *bloc = 0;
            }
        }
    }
  else
    {
      x01_cptr.erreur = (unsigned char) flag_err;
      tedi_erreur (las, mode);
      config = FALSE;
    }
  return (config);
}
