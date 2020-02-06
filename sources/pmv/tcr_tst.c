/*
 * tcr_tst.c
 * ===================
 *
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: TCR_TST.c,v $
 * Revision 1.2  2018/06/04 08:38:43  xg
 * Passage en version V122
 *
 * Revision 1.2  2008/07/04 17:05:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:30:14  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

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
#include "lcr_tst.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

/* *************************************
 * PROTOTYPES DES FONCTIONS LOCALES
 * ************************************* */

static VOID lcr_cmd_tst (STRING, INT);

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_tst_sc                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/01/97                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_sc.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande TST SC                        /
/-------------------------------------------------------------------------DOC*/
LOCAL VOID lcr_cmd_tst (buffer, nbcar)
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
                    }
                  else
                    erreur = TRUE;
                }
              else if (dv1_scmp (ptr_deb, "SYN=", 0))
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
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
              else
                erreur = TRUE;
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

  if (erreur)
    {
      x01_cptr.erreur = CPTRD_SYNTAXE;
    }
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst_sc                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/12/1996                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_init                                             /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande INIT                          /
/-------------------------------------------------------------------------DOC*/
VOID lcr_tst_sc (las, mode, lg_mess, buffer, position, pt_mess)
     INT las;                   /* numero du las sur lequel se fait le transfert */
     INT mode;                  /* mode de transmission                          */
     INT lg_mess;               /* nombre de caractere dans le buffer            */
     STRING buffer;             /* adresse du buffer                             */
     INT position;              /* position dans le buffer                       */
     struct usr_ztf *pt_mess;   /* pointeur vers un message de transmission      */

{
  INT i, j, k, bloc;
  BYTE fini;
  struct crt *carte;

/* taille indique le nombre de caracteres du parametre */
  if (lg_mess > 5)
    {
      if (dv1_scmp (&buffer[4], "MOD", 3))
        {
          j = 0;
          bloc = 0;
          fini = FALSE;
          for (i = 0; i < NB_LAS_PIP; i++)
            {
              for (k = 0; k < pip_ctrl_las[i].nb_matrice; k++)
                {
                  if ((j >= 0) && (!fini))
                    {
                      tedi_send_bloc (las, mode, buffer, j, bloc, TRUE,
                                      pt_mess);

                      if (++bloc >= 10)
                        bloc = 0;
                      j = 0;

                      if (vct_IT[las])
                        fini = TRUE;
                    }
                  carte = &pip_ctrl_carte[i].crt[j];
                  j +=
                    dv1_sprintf (&buffer[j],
                                 "LAS=%01d AD=%02X ETAT=%02X ETAT1=%02X LUM=%02X TYPE=%02X\n\r",
                                 i, k, carte->etat, carte->etat1, carte->lum,
                                 carte->type);
                }
            }
        }
    }
  else
    x01_cptr.erreur = CPTRD_SYNTAXE;

  if (x01_cptr.erreur == CPTRD_OK)
    {
      /* un aquit */
      j =
        dv1_sprintf (buffer, "TST SC ITM=%03d SVR=%01d SYN=%03d",
                     pip_cf_tst_sc.tp_intr_matrice, pip_cf_tst_sc.svr,
                     pip_cf_tst_sc.top_synchro);

      tedi_send_bloc (las, mode, buffer, j, 0, TRUE, pt_mess);
    }
  else
    {
      tedi_erreur (las, mode);
    }
}
