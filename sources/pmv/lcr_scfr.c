/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_scfr                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 19/11/2002                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Configuration execution de la commande TST SC FR
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

#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "tac_conf.h"
#include "lcr_sc.h"

 /**********************************************************/
 /*                                                        */
 /*            prototypage des fonctions                   */
 /*                                                        */
 /**********************************************************/

LOCAL VOID lcr_cmd_tst_sc_ft (STRING, INT);


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst_sc_ft_init                                        /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/03/2002                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_scfr.c                                             /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION :initialisation des parametres generaux                  /
/-------------------------------------------------------------------------DOC*/
VOID lcr_tst_sc_frc_init (VOID)
{
  BYTE i;
  for (i = 0; i < MAX_TS_FORCAGE; i++)
    {
      pip_cf_tst_sc_frc.nbCarDeb[i] = 0;
      pip_cf_tst_sc_frc.nbCarFin[i] = 0;
    }
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_tst_sc_frc                                         /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/03/2002                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_scfr.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande TST SC FT                     /
/-------------------------------------------------------------------------DOC*/
LOCAL VOID lcr_cmd_tst_sc_frc (STRING buffer, INT nbcar)
{
  UINT val;
  BYTE erreur;
  STRING ptr_deb;
  STRING ptr_fin;

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
              if (*ptr_deb == 'Z')
                {
                  if (++ptr_deb >= ptr_fin)
                    {
                      erreur = FALSE;
                      /* on reinitialise */
                      lcr_tst_sc_frc_init ();
                    }
                  else
                    erreur = TRUE;
                }
              else if (dv1_scmp (ptr_deb, "MD", 0))
                {
                  BYTE numMess;
                  /* fin du parametre */
                  *ptr_fin = 0;

                  /* on positionne le pointeur */
                  ptr_deb += 2;
                  numMess = *ptr_deb - '0';
                  if (numMess < MAX_TS_FORCAGE)
                    {
                      ptr_deb += 2;
                      /* on recherche la liste des parametres */
                      pip_cf_tst_sc_frc.nbCarDeb[numMess] =
                        dv1_scpy (pip_cf_tst_sc_frc.messageDeb[numMess],
                                  ptr_deb, 0);
                      pip_cf_tst_sc_frc.messageDeb[numMess][pip_cf_tst_sc_frc.
                                                            nbCarDeb[numMess]]
                        = 0;
                      ptr_deb += pip_cf_tst_sc_frc.nbCarDeb[numMess];
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
              else if (dv1_scmp (ptr_deb, "MF", 0))
                {
                  BYTE numMess;
                  /* fin du parametre */
                  *ptr_fin = 0;

                  /* on positionne le pointeur */
                  ptr_deb += 2;
                  numMess = *ptr_deb - '0';
                  if (numMess < MAX_TS_FORCAGE)
                    {
                      ptr_deb += 2;
                      /* on recherche la liste des parametres */
                      pip_cf_tst_sc_frc.nbCarFin[numMess] =
                        dv1_scpy (pip_cf_tst_sc_frc.messageFin[numMess],
                                  ptr_deb, 0);
                      pip_cf_tst_sc_frc.messageFin[numMess][pip_cf_tst_sc_frc.
                                                            nbCarFin[numMess]]
                        = 0;
                      ptr_deb += pip_cf_tst_sc_frc.nbCarFin[numMess];
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
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

  if (erreur)
    {
      x01_cptr.erreur = CPTRD_SYNTAXE;
      /* on restaure la config */
      tac_conf_cfg_lec_conf ();
    }
  else
    {
      /* on signale que la config a change */
      x01_cptr.config = TEMPO_CONFIG;
    }
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst_sc_ft                                          /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 08/03/2002                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_sc_frc                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande TST SC FRC                    /
/-------------------------------------------------------------------------DOC*/
VOID lcr_tst_sc_frc (INT las, INT mode, INT lg_mess, STRING buffer,
                     INT position, T_usr_ztf * pt_mess)
{
  INT j;
  INT bloc;
  INT flag;

  flag = FALSE;
  if (identLireAutorisationAvecNiveau (las, 0))
    {
      /* taille indique le nombre de caracteres du parametre */
      if (lg_mess > 8)
        {
          lcr_cmd_tst_sc_frc (&buffer[8], lg_mess - 8);
          flag = TRUE;
        }
    }
  else
    x01_cptr.erreur = CPTRD_PROTOCOLE;

  if (x01_cptr.erreur == CPTRD_OK)
    {
      BYTE i;
      INT bloc = 0;
      if (flag)
        {
          j = dv1_sprintf (buffer, "TST FRC", 0);
        }
      else
        {
          j = dv1_sprintf (buffer, "TST FRC\n\r", 0);
          for (i = 0; i < MAX_TS_FORCAGE; i++)
            {
              if (pip_cf_tst_sc_frc.nbCarDeb[i])
                {
                  if (j > 125)
                    {
                      tedi_send_bloc (las, mode, buffer, j, bloc, FALSE,
                                      pt_mess);
                      j = 0;
                      if (++bloc == 10)
                        bloc = 0;
                    }
                  j +=
                    dv1_sprintf (&buffer[j], "MD%d=%s\n\r", i,
                                 pip_cf_tst_sc_frc.messageDeb[i]);
                }
              if (pip_cf_tst_sc_frc.nbCarFin[i])
                {
                  if (j > 125)
                    {
                      tedi_send_bloc (las, mode, buffer, j, bloc, FALSE,
                                      pt_mess);
                      j = 0;
                      if (++bloc == 10)
                        bloc = 0;
                    }
                  j +=
                    dv1_sprintf (&buffer[j], "MF%d=%s\n\r", i,
                                 pip_cf_tst_sc_frc.messageFin[i]);
                }
            }
        }
      tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
    }
  else
    {
      tedi_erreur (las, mode);
    }
}
