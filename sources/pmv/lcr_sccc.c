/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_sccc                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/02/2002                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Configuration execution de la commande TST SC CC
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
#include "pip_def.h"                   /* definition des constantes                       */
#include "pip_str.h"                   /* definition des constantes                       */
#include "pip_var.h"                   /* definition des constantes                       */

#include "sir_dv1h.h"
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales              */

#include "tac_conf.h"                  /* definition des variables globales              */
#include "lcr_sc.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

LOCAL VOID lcr_cmd_tst_sc_cc (STRING, INT);


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_tst_sc                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/03/2002                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_scft.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande TST SC FT                     /
/-------------------------------------------------------------------------DOC*/
LOCAL VOID lcr_cmd_tst_sc_cc (buffer, nbcar)
     STRING buffer;             /* buffer contenant les nouveaux parametres de panneau   */
     INT nbcar;                 /* longueur du buffer de parametres                       */

{

  INT val;
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
                      lcr_tst_sc_cc_init ();
                    }
                  else
                    erreur = TRUE;
                }
              else if (dv1_scmp (ptr_deb, "CA=", 0))
                {
                  /* fin du parametre */
                  *ptr_fin = 0;

                  /* on positionne le pointeur */
                  ptr_deb += 3;

                  /* a priori il n'y a pas d'erreur */
                  erreur = FALSE;
                  /* on recherche la liste des parametres */
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* le code feux tricolore */
                      if ((val > 0) && (val <= 99))
                        pip_cf_tst_sc_cc.code_CA = (BYTE) val;
                      else
                        erreur = TRUE;
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
              else if (dv1_scmp (ptr_deb, "NRP=", 0))
                {
                  /* fin du parametre */
                  *ptr_fin = 0;

                  /* on positionne le pointeur */
                  ptr_deb += 4;

                  /* a priori il n'y a pas d'erreur */
                  erreur = FALSE;

                  /* on recherche la liste des parametres */
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* numero de rampe */
                      if ((val == 1000) || (val == 100) || (val == 10)
                          || (val == 1))
                        pip_cf_tst_sc_cc.numRampe = val;
                      else
                        erreur = TRUE;
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
/ NOM DE LA FONCTION : lcr_tst_sc_ft                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 08/03/2002                                              /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_sccc                                             /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande TST SC CC                          /
/-------------------------------------------------------------------------DOC*/
VOID lcr_tst_sc_cc (las, mode, lg_mess, buffer, position, pt_mess)
     INT las;                   /* numero du las sur lequel se fait le transfert */
     INT mode;                  /* mode de transmission                          */
     INT lg_mess;               /* nombre de caractere dans le buffer            */
     STRING buffer;             /* adresse du buffer                             */
     INT position;              /* position dans le buffer                       */
     struct usr_ztf *pt_mess;   /* pointeur vers un message de transmission      */

{
  INT j;

  if (identLireAutorisationAvecNiveau (las, 0))
    {
      /* taille indique le nombre de caracteres du parametre */
      if (lg_mess > 9)
        {
          lcr_cmd_tst_sc_cc (&buffer[6], lg_mess - 6);
        }
    }
  else
    x01_cptr.erreur = CPTRD_PROTOCOLE;

  if (x01_cptr.erreur == CPTRD_OK)
    {
      /* on retourne les parametres */
      j = dv1_sprintf (buffer, "TST SC CC CA=%02d FR=%02d",
                       pip_cf_tst_sc_cc.code_CA, pip_cf_tst_sc_cc.code_FR);
      j += dv1_sprintf (&buffer[j], " NRP=%04d", pip_cf_tst_sc_cc.numRampe);

      tedi_send_bloc (las, mode, buffer, j, 0, TRUE, pt_mess);
    }
  else
    {
      tedi_erreur (las, mode);
    }
}
