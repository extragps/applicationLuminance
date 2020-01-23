/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_dt                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 21/12/1996                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la      /
/                           gestion de l'heure                                /
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
#endif
#include "standard.h"                  /* redefinition des types                         */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "xdg_str.h"                   /* definition des structures globales             */
#include "xdg_var.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "ted_prot.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/
LOCAL VOID cmd_vt (STRING, INT, INT);

VOID lcr_vt (INT, INT, INT, STRING, INT, struct usr_ztf *);

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_set                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 16/12/1996                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_set.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande SET                            /
/-------------------------------------------------------------------------DOC*/
LOCAL VOID cmd_vt (STRING buffer, INT nbcar, INT las)
{
  UINT8 erreur;
  STRING ptr_deb;
  STRING ptr_fin;
  INT i;
  INT port;
  INT duree;

  INT trace;

/* le dernier caractere est nul */
  buffer[nbcar] = 0;

  ptr_deb = buffer;
  ptr_fin = buffer;

  erreur = FALSE;

  i = 0;
  duree = 60;
  trace = FALSE;
  port = N_AFF;

  while (((INT) (ptr_fin - buffer) <= nbcar) && (!erreur))
    {
      switch (*ptr_fin)
        {

        case 0:
        case ' ':
          *ptr_fin = 0;

          /* si deux separateur ne se suivent pas */
          if (ptr_deb != ptr_fin)
            {

              if (i == 0)
                {
                  /* on recherche le numero du port */
                  if (!dv1_str_atoi (&ptr_deb, &port))
                    {
                      erreur = TRUE;
                    }
                  else
                    {
                      port--;
                      if ((port < 0) && (port > NBPORT))
                        erreur = TRUE;
                    }
                  i++;
                }
              else if (i == 1)
                {
                  /* on recherche la duree */
                  if (!dv1_str_atoi (&ptr_deb, &duree))
                    {
                      erreur = TRUE;
                    }
                  i++;
                }
              else if (i == 2)
                {
                  /* on recherche la duree */
                  if (dv1_scmp (ptr_deb, "TRACE", 0))
                    {
                      trace = TRUE;
                    }
                  else
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

  if ((las != port) && (!erreur))
    {
      xdg_vt[port].port = las;
      xdg_vt[port].tempo = duree;
      xdg_vt[port].trace = trace;
    }
  else
    erreur = TRUE;

  if (erreur)
    x01_cptr.erreur = CPTRD_SYNTAXE;
}



/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_fin                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/12/1996                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_set.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande SETU                          /
/-------------------------------------------------------------------------DOC*/
VOID lcr_vt (las, mode, lg_mess, buffer, position, pt_mess)
     INT las;                   /* numero du las sur lequel se fait le transfert */
     INT mode;                  /* mode de transmission                          */
     INT lg_mess;               /* nombre de caractere dans le buffer            */
     STRING buffer;             /* adresse du buffer                             */
     INT position;              /* position dans le buffer                       */
     struct usr_ztf *pt_mess;   /* pointeur vers un message de transmission      */


{

  INT i;                        /* variable de boucle lecture du buffer            */

/* on se place apres le nom de la commande */
  i = position + 2;

/* si on est pas en fin de message, on continue l'analyse */
  if (i < lg_mess)
    {
      /* taille indique le nombre de caracteres du parametre */
      cmd_vt (&buffer[i], lg_mess - i, las);
    }
  else
    x01_cptr.erreur = CPTRD_SYNTAXE;

  if (x01_cptr.erreur == CPTRD_OK)
    {
      tedi_send_bloc (las, mode, buffer, 0, 0, TRUE, pt_mess);
    }
  else
    {
      tedi_erreur (las, mode);
    }
}
