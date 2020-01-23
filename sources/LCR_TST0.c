/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst                                              /
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

#include "lcr_idfh.h"
#include "lcr_cf3h.h"
#include "lcr_tst1.h"
#include "lcr_tst2.h"
#include "rec_main.h"
#include "tac_conf.h"
#include "x01_trch.h"
#include "xversion.h"
#include "ted_prot.h"
#include "tac_vip.h"
#include "identLib.h"


/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : tst_tst                                                /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 01/03/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_tst.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande TST                           /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : las   = numero du port sur lequel est parvenue la cde   /
/                     mode  = mode de TEDI ds lequel doit se faire la reponse /
/                     lg_mess = longueur du message contenant la commande     /
/                     buffer  = chaine de caractere contenant la commande     /
/-------------------------------------------------------------------------DOC*/
INT
tst_tst (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
         struct usr_ztf *pt_mess)
{
  INT i, j;               /* variable de boucle lecture du buffer            */
  INT taille;                   /* taille d'une chaine de caracteres               */
  INT flag_err;                 /* indicateur d'erreur dans la ligne de cde ST     */
  INT bloc;                     /* numero de bloc a envoyer                        */
  INT config;
  UINT8 flag_init;
  flag_err = 0;
  config = FALSE;
  bloc = 0;
  flag_init = FALSE;
  i = position + 3;
  /* on elimine les separateurs */
  i += dv1_separateur (&buffer[i], lg_mess - i);
  /* si on est pas en fin de message,
   * on continue l'analyse */
  if (i < lg_mess)
    {
      /* on cherche la fin du parametre */
      taille = dv1_cherchefin (&buffer[i], lg_mess - i);
      /* commande TST D  : configuration
       * des detecteurs */
#ifdef RAD
      if ((buffer[i] == 'D') && (taille <= 3))
        {
          /* Determination du numero de detecteur */
          if (taille > 1)
            {
              if (!dv1_atoi (&buffer[i + 1], taille - 1, &detecteur))
                {
                  flag_err = 1;
                }
              else
                {
                  if (detecteur >= NBDET)
                    {
                      flag_err = 2;
                    }
                }
            }
          else
            {
              detecteur = 99;
            }
          /* Les autres parametres...             */

          if (flag_err == 0)
            {
              j = i + taille;
              j += dv1_separateur (&buffer[j], lg_mess - j);
              if (j < lg_mess)         /* apres V, il y a des parametres */
                {
                  if (identLireAutorisationAvecNiveau (las, 0))
                    {
                      int parSeuil = 0;
                      config = TRUE;
                      flag_err =
                        tst_detecteur (detecteur, &buffer[j], lg_mess - j,
                                       &parSeuil);
                      if (flag_err == 0)
                        {
                          vct_controle.reinit = TRUE;
                          if (parSeuil)
                            {
                              vct_controle.init_seuil = TRUE;
                            }
                        }

                    }
                  else
                    flag_err = 3;
                }
              /* si on a pas trouve d'erreur */
              if (flag_err == 0)
                {
                  i = 0;
                  j = 0;
                  bloc = 0;
                  fini = FALSE;
                  while ((!fini) && (i < NBDET))
                    {
                      if ((detecteur == 99) || (i == detecteur))
                        {
                          j += dv1_sprintf (&buffer[j], "TST D");
                          j += dv1_itoa (&buffer[j], i, 0);
                          j += dv1_sprintf (&buffer[j], " TYP=");
                          j +=
                            dv1_itoa (&buffer[j], vct_detecteur[i].type, 0);
                          j += dv1_sprintf (&buffer[j], " ETAT=");
                          if (TRUE == rec_acq_detecteur_mdb (i * 4))
                            {
                              dv1_itoh (&buffer[j],
                                        rec_acq_get_etat_detecteur_mdb (i *
                                                                        4),
                                        2);
                              j += 2;
                            }
                          else
                            {
                              j += dv1_sprintf (&buffer[j], "  ");
                            }
                          j += dv1_sprintf (&buffer[j], (STRING) "\n\r", 2);
                          for (k = 0; k < 4; k++)
                            {
                              j += dv1_sprintf (&buffer[j], "    V");
                              j += dv1_itoa (&buffer[j], k, 0);
                              j += dv1_sprintf (&buffer[j], "  BCL=");
                              j +=
                                dv1_itoa (&buffer[j],
                                          vct_detecteur[i].boucle[k], 0);
                              j += dv1_sprintf (&buffer[j], " FRQ=");
                              j +=
                                dv1_itoa (&buffer[j],
                                          vct_detecteur[i].frequence[k], 0);
                              j += dv1_sprintf (&buffer[j], " SBL=");
                              j +=
                                dv1_itoa (&buffer[j],
                                          vct_detecteur[i].sensibilite[k], 0);
                              j += dv1_sprintf (&buffer[j], " OBL=");
                              j +=
                                dv1_itoa (&buffer[j],
                                          vct_detecteur[i].oubli[k], 0);
                              j += dv1_sprintf (&buffer[j], " FLT=");
                              j +=
                                dv1_itoa (&buffer[j],
                                          vct_detecteur[i].filtre[k], 0);
                              j += dv1_sprintf (&buffer[j], " PZO=");
                              j += dv1_itoa (&buffer[j], 0, 0);
                              j += dv1_sprintf (&buffer[j], " EXT=");
                              j +=
                                dv1_itoa (&buffer[j],
                                          vct_detecteur[i].seuil[k], 0);
                              j +=
                                dv1_sprintf (&buffer[j], (STRING) "\n\r", 2);
                            }
                          if ((i == detecteur)
                              || ((i == (NBDET - 1)) && (detecteur == 99)))
                            {
                              tedi_send_bloc (las, mode, buffer, j, bloc,
                                              TRUE, pt_mess);
                            }
                          else
                            {
                              if (j > 0)
                                {
                                  tedi_send_bloc (las, mode, buffer, j, bloc,
                                                  FALSE, pt_mess);
                                  bloc = (bloc + 1) % 10;
                                  j = 0;
                                  if (vct_IT[las])
                                    fini = TRUE;
                                }
                            }
                        }
                      i++;
                    }
                }
            }

          if (flag_err != 0)
            {
              x01_cptr.erreur = flag_err;
              tedi_erreur (las, mode);
            }
        }
      else if ((buffer[i] == 'S') && (taille <= 3))
        {
          /* on lit le numero du detecteur demande, si il existe */
          if (taille > 1)
            {
              /* on calcule le numero de detecteur */
              if (!dv1_atoi (&buffer[i + 1], taille - 1, &detecteur))
                {
                  /* erreur de syntaxe */
                  flag_err = 1;
                }
              else
                {
                  /* on teste la validite du numero de detecteur */
                  if (detecteur >= NBDET)
                    {
                      flag_err = 2;
                    }
                }
            }
          else
            {
              /* tous les detecteurs sont a prendre en compte */
              detecteur = 99;
            }

          /* si on a pas trouve d'erreur */
          if (flag_err == 0)
            {
              /* on teste si on est en lecture ou en ecriture */
              /* en lecture la commande n'a pas de parametre  */
              j = i + taille;

              j += dv1_separateur (&buffer[j], lg_mess - j);

              if (j < lg_mess)         /* apres V, il y a des parametres */
                {
                  /* teste si l'operateur est habilite */
                  if (identLireAutorisationAvecNiveau (las, 0))
                    {
                      int parSeuil = 0;
                      config = TRUE;

                      /* la commande a des parametres */
                      /* on traite les parametres detecteur */
                      flag_err = tst_detecteur_silh (detecteur, &buffer[j],
                                                     lg_mess - j, &parSeuil);

                      /* si on a pas trouve d'erreur */
                      if (flag_err == 0)
                        {
                          /* on demande l'initialisation des detecteurs */
                          vct_controle.reinit = TRUE;
                          if (parSeuil)
                            {
                              vct_controle.init_seuil = TRUE;
                            }
                        }

                    }
                  else
                    flag_err = 3;
                }

              /* si on a pas trouve d'erreur */
              if (flag_err == 0)
                {
                  /* on renvoie le status des canaux */
                  /* pour tous les canaux configurees */
                  i = 0;
                  j = 0;

                  /* on initialise le numero de bloc de reponse */
                  bloc = 0;

                  fini = FALSE;

                  while ((!fini) && (i < NBDET))
                    {
                      if ((detecteur == 99) || (i == detecteur))
                        {
                          dv1_scpy (&buffer[j], (STRING) "TST S", 5);
                          j += 5;

                          taille = dv1_itoa (&buffer[j], i, 0);
                          j += taille;

                          dv1_scpy (&buffer[j], (STRING) " DDN=", 5);
                          j += 5;
                          taille =
                            dv1_itoa (&buffer[j], vct_det_silh[i].dd, 0);
                          j += taille;

                          dv1_scpy (&buffer[j], (STRING) " LDN=", 5);
                          j += 5;
                          taille =
                            dv1_itoa (&buffer[j], vct_det_silh[i].ld, 0);
                          j += taille;

                          dv1_scpy (&buffer[j], (STRING) " DDS=", 5);
                          j += 5;
                          taille =
                            dv1_itoa (&buffer[j], vct_det_silh[i].dd_silh, 0);
                          j += taille;

                          dv1_scpy (&buffer[j], (STRING) " LDS=", 5);
                          j += 5;
                          taille =
                            dv1_itoa (&buffer[j], vct_det_silh[i].ld_silh, 0);
                          j += taille;

                          dv1_scpy (&buffer[j], (STRING) " VA1=", 5);
                          j += 5;
                          taille =
                            dv1_itoa (&buffer[j], vct_det_silh[i].vam1, 0);
                          j += taille;

                          dv1_scpy (&buffer[j], (STRING) " VA2=", 5);
                          j += 5;
                          taille =
                            dv1_itoa (&buffer[j], vct_det_silh[i].vam2, 0);
                          j += taille;

                          /* fin de ligne */
                          dv1_scpy (&buffer[j], (STRING) "\n\r", 2);
                          j += 2;

                          if ((i == detecteur)
                              || ((i == (NBDET - 1)) && (detecteur == 99)))
                            {
                              /* on envoie le buffer courant */
                              tedi_send_bloc (las, mode, buffer, j, bloc,
                                              TRUE, pt_mess);
                            }
                          else
                            {
                              if (j > 0)
                                {
                                  /* on envoie le buffer courant */
                                  tedi_send_bloc (las, mode, buffer, j, bloc,
                                                  FALSE, pt_mess);

                                  /* on enregistre le no de bloc */
                                  bloc = (bloc + 1) % 10;

                                  /* on remet a zero l'indice de remplissage de buffer */
                                  j = 0;

                                  if (vct_IT[las])
                                    fini = TRUE;
                                }
                            }
                        }

                      /* on passe au detecteur suivant */
                      i++;
                    }
                }
            }

          if (flag_err != 0)
            {
              x01_cptr.erreur = flag_err;
              tedi_erreur (las, mode);
            }
        }
      else if (dv1_scmp (&buffer[i], (STRING) "VCMT", 4) && (taille <= 6))
        {
          /* Determination du numero de detecteur */
          if (taille > 4)
            {
              if (!dv1_atoi (&buffer[i + 4], taille - 4, &detecteur))
                {
                  flag_err = 1;
                }
              else
                {
                  if (detecteur >= NBDET_VCMT)
                    {
                      flag_err = 2;
                    }
                }
            }
          else
            {
              detecteur = 99;
            }
          /* Les autres parametres...             */

          if (flag_err == 0)
            {
              j = i + taille;
              j += dv1_separateur (&buffer[j], lg_mess - j);
              if (j < lg_mess)         /* apres V, il y a des parametres */
                {
                  if (identLireAutorisationAvecNiveau (las, 0))
                    {
                      int parSeuil = 0;
                      config = TRUE;
                      flag_err = tst_detecteur_vcmt (detecteur,
                                                     &buffer[j], lg_mess - j,
                                                     &parSeuil);
                      if (flag_err == 0)
                        {
                          vct_controle.reinit = TRUE;
                          if (parSeuil)
                            {
                              vct_controle.init_seuil = TRUE;
                            }
                        }

                    }
                  else
                    flag_err = 3;
                }
              /* si on a pas trouve d'erreur */
              if (flag_err == 0)
                {
                  i = 0;
                  j = 0;
                  bloc = 0;
                  fini = FALSE;
                  while ((!fini) && (i < NBDET_VCMT))
                    {
                      if ((detecteur == 99) || (i == detecteur))
                        {
                          j += dv1_sprintf (&buffer[j], "TST VCMT");
                          j += dv1_itoa (&buffer[j], i, 0);
                          j += dv1_sprintf (&buffer[j], " ETAT=");
                          if (TRUE == rec_acq_detecteur_vcmt (i))
                            {
                              dv1_itoh (&buffer[j],
                                        rec_acq_get_etat_detecteur_vcmt (i),
                                        2);
                              j += 2;
                            }
                          else
                            {
                              j += dv1_sprintf (&buffer[j], "  ");
                            }
                          j += dv1_sprintf (&buffer[j], " SEUIL=");
                          j +=
                            dv1_itoa (&buffer[j], vct_detecteur_vcmt[i].seuil,
                                      0);
                          j += dv1_sprintf (&buffer[j], " GAIN=");
                          j +=
                            dv1_itoa (&buffer[j], vct_detecteur_vcmt[i].gain,
                                      0);
                          j += dv1_sprintf (&buffer[j], " DELAI_ON=");
                          j +=
                            dv1_itoa (&buffer[j],
                                      vct_detecteur_vcmt[i].delai_on, 0);
                          j += dv1_sprintf (&buffer[j], " DELAI_OFF=");
                          j +=
                            dv1_itoa (&buffer[j],
                                      vct_detecteur_vcmt[i].delai_off, 0);
                          j += dv1_sprintf (&buffer[j], " OUBLI=");
                          j +=
                            dv1_itoa (&buffer[j], vct_detecteur_vcmt[i].oubli,
                                      0);
                          j += dv1_sprintf (&buffer[j], (STRING) "\n\r", 2);
                          if ((i == detecteur) ||
                              ((i == (NBDET_VCMT - 1)) && (detecteur == 99)))
                            {
                              tedi_send_bloc (las, mode, buffer, j, bloc,
                                              TRUE, pt_mess);
                            }
                          else
                            {
                              if (j > 0)
                                {
                                  tedi_send_bloc (las, mode, buffer, j,
                                                  bloc, FALSE, pt_mess);
                                  bloc = (bloc + 1) % 10;
                                  j = 0;
                                  if (vct_IT[las])
                                    fini = TRUE;
                                }
                            }
                        }
                      i++;
                    }
                }
              else if (2 == flag_err)
                {
                  j = 0;
                  bloc = 0;
                  j = dv1_sprintf (&buffer[j], "TST VCMT LECT");
                  tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
                  flag_err = 0;
                }
            }

          if (flag_err != 0)
            {
              x01_cptr.erreur = flag_err;
              tedi_erreur (las, mode);
            }
        }
      else
        /* TST BCL  : etat des boucles  */
      if (dv1_scmp (&buffer[i], (STRING) "BCL", 3))
        {
          j = 0;
          /* on initialise le numero de bloc de reponse */
          bloc = 0;
          j += dv1_scpy (&buffer[j], (STRING) "TST BCL\n\r", 9);
          j += dv1_scpy (&buffer[j],
                         (STRING) "BCL  DEF  DUREE    CL0    CL1    CL2\n\r",
                         38);
          for (i = 0; i < NBCAPTEUR; i++)
            {
              /* si le capteur est configure */
              if (vct_capteur1[i].type != 0)
                {
                  if (j >= MAXLCR)
                    {
                      tedi_send_bloc (las, mode, buffer, j, bloc, FALSE,
                                      pt_mess);
                      if (++bloc >= 10)
                        bloc = 0;
                      j = 0;
                      if (vct_IT[las])
                        fini = TRUE;
                    }
                  memset (&buffer[j], ' ', 34);
                  dv1_itoa (&buffer[j], i, 2);
                  buffer[j + 6] = z_etat_capteur[i].etat;
                  dv1_itoa (&buffer[j + 10], z_etat_capteur[i].cpt_defaut, 0);
                  dv1_itoa (&buffer[j + 19], z_etat_capteur[i].class[0], 0);
                  dv1_itoa (&buffer[j + 26], z_etat_capteur[i].class[1], 0);
                  taille = dv1_itoa (&buffer[j + 33],
                                     z_etat_capteur[i].class[2], 0);
                  buffer[j + 33 + taille] = '\n';
                  buffer[j + 33 + taille + 1] = '\r';
                  j += 33 + taille + 2;
                }
            }
          /* on envoie le buffer courant */
          tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
        }
      else
#endif
      if (dv1_scmp (&buffer[i], "CONF", 3))
        {
          /* les cartes presentes dans le rack */
          j = sprintf (&buffer[j], "TST CONF SZ=%04X\n\r",
                           x01_status3.taille_config);
          /* on envoie le buffer courant */
          tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
        }
    }
  if (flag_err != 0)
    {
      config = FALSE;
    }
  return (config);
}
