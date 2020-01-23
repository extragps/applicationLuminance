/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_reg                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 11/02/1991                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions de reglages et de   /
/ mises au point de la station et de ses accessoires.                         /
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
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_ge1h.h"
#include "lcr_ge3h.h"
#include "ted_prot.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

VOID reg_rd (INT, INT, INT, STRING, INT, struct usr_ztf *);
LOCAL INT rep_rd (INT, INT, STRING);

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : rep_RD                                                 /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 11/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_reg.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : formattage de la reponse a la commande RD              /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : val    : valeur de la distance dynamique                /
/                     capteur: numero du capteur pair servant au calcul de dd /
/-----------------------------------------------------------------------------/
/ DONNEES EXPORTEES : buffer : chaine contenant la reponse a RD               /
/-----------------------------------------------------------------------------/
/ VALEURS RENDUES : longueur du buffer de reponse                             /
/-----------------------------------------------------------------------------/
/ CODE ERREUR :                                                               /
/-----------------------------------------------------------------------------/
/ FONCTIONS APPELEES :                                                        /
/-----------------------------------------------------------------------------/
/ FONCTIONS APPELANTES : reg_RD                                               /
/-----------------------------------------------------------------------------/
/ DESCRIPTION ALGORITHMIQUE DES TRAITEMENTS :                                 /
/-----------------------------------------------------------------------------/
/-------------------------------------------------------------------------DOC*/
LOCAL INT rep_rd (val, capteur, buffer)
     INT val;                   /* valeur de la distance dynamique calculee                 */
     INT capteur;               /* numero du capteur pair servant au calcul de la dist. dyn. */
     STRING buffer;             /* buffer a formatter pour elaborer la reponse e RD         */

{                                      /* debut de rep_RD */

  INT i;                        /* indice de remplissage de buffer                     */
  INT nbcar;                    /* nombre de caracteres de codage du capteur           */

/* on initialise le buffer de reponse a RD */
  dv1_scpy (buffer, (STRING) "RD ", 3);

/* on initialise l'indice de remplissage de buffer a 3 */
  i = 3;

/* on transforme en chaine le numero de capteur pair */
  nbcar = dv1_itoa (&buffer[i], capteur, 0);

  i += nbcar;                          /* on deplace l'indice de remplissage de buffer */

  buffer[i] = ':';

  i++;

/* on transforme en chaine le numero de capteur impair */
  nbcar = dv1_itoa (&buffer[i], capteur + 1, 0);

  i += nbcar;                          /* on deplace l'indice de remplissage de buffer */

  buffer[i] = '=';
  i++;

/* on transforme en chaine la valeur de la dist dynamique */
  dv1_itoa (&buffer[i], val, 3);

  i += 3;                              /* on deplace l'indice de remplissage de buffer */

/* on termine la ligne avec LF/CR */
  buffer[i] = '\n';
  i++;

  buffer[i] = '\r';
  i++;

  return (i);

}                                      /* fin de rep_RD */

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : reg_RD                                                 /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 11/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_reg.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : reglage de la distance  dynamique inter capteur dans   /
/                      une paire.                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : las    : numero du las ou doit se faire la reponse      /
/                     mode   : mode du protocole TEDI utilise                 /
/                     lg_mess: nombre de caractere de buffer                  /
/                     buffer : chaine contenant la commande RD                /
/-----------------------------------------------------------------------------/
/ DONNEES EXPORTEES : aucune                                                  /
/-----------------------------------------------------------------------------/
/ VALEURS RENDUES : aucune                                                    /
/-----------------------------------------------------------------------------/
/ CODE ERREUR :                                                               /
/-----------------------------------------------------------------------------/
/ FONCTIONS APPELEES :                                                        /
/-----------------------------------------------------------------------------/
/ FONCTIONS APPELANTES : lcr_pcp                                              /
/-----------------------------------------------------------------------------/
/ DESCRIPTION ALGORITHMIQUE DES TRAITEMENTS :                                 /
/-----------------------------------------------------------------------------/
/-------------------------------------------------------------------------DOC*/
VOID reg_rd (las, mode, lg_mess, buffer, position, pt_mess)
     INT las;                   /* numero du las sur lequel se fait le transfert */
     INT mode;                  /* mode de transmission                          */
     INT lg_mess;               /* nombre de caractere dans le buffer            */
     STRING buffer;             /* adresse du buffer                             */
     INT position;              /* position dans le buffer                       */
     struct usr_ztf *pt_mess;   /* pointeur vers un message de transmission      */


{                                      /* debut de reg_RD */

  INT flag_err;                 /* indicateur d'erreur dans la commande recue     */
  INT i;                        /* indice de lecture de buffer                    */
  INT taille;                   /* taille des differents parametres de la cde     */
  INT fini;                     /* variable de controle de boucle                 */
  INT capteur;                  /* numero du capteur servant pour la cde RD       */
  INT pod;                      /* indice de lecture de la fifo des dist dyn.     */
  INT no_bloc;                  /* numero du bloc a envoyer                       */
  INT nbcar;                    /* nombre de caracteres du bloc a envoyer         */

  INT vitesse;                  /* valeur de la vitesse du vehicule temoin        */


/* on suppose qu'il n'y a pas d'erreur dans le message recu */
  flag_err = 0;

/* on pointe sur le debut du buffer */
  i = position + 2;

/* valeur par defaut de la vitesse */
  vitesse = 60;

/* on recherche le debut de la zone parametre */
  i += dv1_separateur (&buffer[i], lg_mess - i);

/* si on est pas en fin de la commande, on continue l'analyse */
/* en cherchant le numero du capteur                          */
  if (i < lg_mess)
    {                                  /* IF1 */

      /* on cherche la fin du parametre donnant le no de capteur */
      taille = dv1_cherchefin (&buffer[i], lg_mess - i);

      /* la taille de codage du no de capteur est 1 ou 2 */
      if (taille > 2)
        {                              /* IF2 */

          /* on genere une erreur de syntaxe */
          flag_err = 1;

        }                              /* FIF2 */
      else
        {                              /* EIF2 */

          /* on calcule la valeur du capteur */
          if (!dv1_atoi (&buffer[i], taille, &capteur))
            {                          /* IF3 */

              /* on genere une erreur de syntaxe */
              flag_err = 1;

            }                          /* FIF3 */
          else
            {                          /* EIF3 */

              /* le capteur indique doit etre de rang pair */
              if (((capteur % 2) != 0) || (vct_capteur1[capteur].type != 2))
                {                      /* IF4 */

                  /* erreur */
                  flag_err = 2;

                }                      /* FIF4 */
              else
                {                      /* EIF4 */

                  i += taille;

                  /* on cherche le debut du parametre de vitesse */
                  i += dv1_separateur (&buffer[i], lg_mess - i);

                  /* si on est pas en fin du message, on decode la vitesse */
                  if (i < lg_mess)
                    {                  /* IF5 */

                      /* on compte le nbre de caracteres de la vitesse */
                      taille = dv1_cherchefin (&buffer[i], lg_mess - i);

                      /* la taille de la vitesse doit etre <= 3 */
                      if (taille > 3)
                        {              /* IF6 */

                          /* erreur de syntaxe */
                          flag_err = 1;

                        }              /* FIF6 */
                      else
                        {              /* EIF6 */

                          /* on calcule la valeur de la vitesse */
                          if (!dv1_atoi (&buffer[i], taille, &vitesse))
                            {          /* IF7 */

                              /* erreur */
                              flag_err = 1;

                            }          /* FIF7 */
                          else
                            {
                              if (vitesse == 0)
                                flag_err = 2;
                            }

                        }              /* FEIF6 */

                    }                  /* FIF5 */

                }                      /* FEIF4 */

            }                          /* FEIF3 */

        }                              /* FEIF2 */

    }                                  /* FIF1 */
  else
    {                                  /* EIF1 */

      /* erreur de syntaxe le capteur doit etre donne */
      flag_err = 1;

    }                                  /* FEIF1 */

/* si on a pas detecte d'erreur, on execute la commande */
  if (flag_err == 0)
    {                                  /* IF1 */

      /* on lance la tempo de validite de RD */
      rqtemp[RD_0 + las] = 60L * TOP;


      fini = FALSE;

      /* initialisation des variables d'execution de RD */
      /* on indique la vit. du veh. temoin */
      vct_rd.vitesse = vitesse;

      /* on demande l'elaboration des distances dynamiques pour ce capteur */
      vct_rd.capteur = capteur;
      vct_rd.flag = TRUE;

      /* on rejoint pointeur d'ecriture et de lecture de la FIFO */
      /* des distances dynamiques                                */
      pod = vct_rd_fifo.pt_in;

      /* on envoie un acquis a la question */
      tedi_ctrl (las, mode, TRUE);

      /* on reinitialise le numero de bloc en cours */
      no_bloc = 0;

      /* tant que la commande n'est pas terminee */
      while (!fini)
        {                              /* W1 */

          /* si une distance dynamique a ete stockee dans la fifo */
          if (vct_rd_fifo.pt_in != pod)
            {                          /* IF2 */

              /* on teste si le capteur qui a genere la distance */
              /* dynamique lue est interessant                   */
              if (capteur == vct_rd_fifo.dd[pod].capteur)
                {                      /* IF3 */

                  /* on appelle la fonction de preparation du message RD */
                  nbcar = rep_rd (vct_rd_fifo.dd[pod].val, capteur, buffer);

                  /* on envoie le vilt */
                  tedi_send_bloc (las, mode, buffer, nbcar, no_bloc, FALSE,
                                  pt_mess);

                  /* on passe au bloc suivant */
                  no_bloc = (no_bloc + 1) % 10;

                }                      /* FIF3 */

              /* on passe a la distance dynamique suivante */
              pod++;

              /* si on a parcouru toute la fifo, on repart au debut */
              if (pod == MAXDD)
                pod = 0;

            }                          /* FIF2 */

          if ((vct_IT[las]) || (rqtemp[RD_0 + las] <= TP_FINI))
            fini = TRUE;

          /* on relance le moniteur */
          mnsusp (0);

        }                              /* FW1 */

      /* on arrete le calcul de la distance dynamique */
      vct_rd.flag = FALSE;

      vct_IT[las] = FALSE;

      /* on termine la commande AI */

      tedi_ctrl (las, mode, TRUE);


    }                                  /* FIF1 */
  else
    {
      x01_cptr.erreur = flag_err;
      tedi_erreur (las, mode);
    }
}                                      /* fin de reg_RD */
