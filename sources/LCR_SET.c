/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_set                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 21/02/1991                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la con- /
/ figuration des communication de la station avec l'exterieur.                /
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

#include <stdio.h>
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
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "ypc_las.h"
#include "identLib.h"
#include "lcr_seth.h"
#include "mon_debug.h"

/* *********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* *********************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ********************************	*/

static VOID cmd_set (STRING, INT, INT);
static int cmd_setu (STRING, INT);

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
static VOID cmd_set (STRING buffer, INT nbcar, INT las)
{
  INT val;
  UINT8 erreur;
  STRING ptr_deb;
  STRING ptr_fin;
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
              /* a priori il y a une erreur */
              erreur = TRUE;
              if ((*ptr_deb == 'Z') || (*ptr_deb == 'S'))
                {
                  x01_set[las].echo = FALSE;
                  x01_set[las].cr = FALSE;
                  x01_set[las].lpl = 0;
                  x01_set[las].mtel = FALSE;
                  xdg_cf_las[las].vitesse = 1200;
				  if(xdg_cf_las[las].type_e==LIAISON_PASS) {
						xdg_cf_las[las].numPortIp = 21355  + las-PORT_PASS;
				  } else {
						/* Pour avoir le numero de port IP compatible avec
						 * le serveur WEB. */
					  xdg_cf_las[las].numPortIp = 33999 + las-PORT_IP_LCR;
				  }
				  ptr_deb++;
                  if (ptr_deb >= ptr_fin) {
                	  erreur = FALSE;
                  }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "ECHO", 0))
                {
                  /* echo */
                  ptr_deb += 5;
                  switch (*ptr_deb)
                    {
                    case 'O':
                      x01_set[las].echo = TRUE;
                      ptr_deb++;
                      mnecho ((UINT8) las, TRUE);
                      if (ptr_deb >= ptr_fin)
                        erreur = FALSE;
                      break;
                    case 'N':
                      x01_set[las].echo = FALSE;
                      ptr_deb++;
                      mnecho ((UINT8) las, FALSE);
                      if (ptr_deb >= ptr_fin)
                        erreur = FALSE;
                      break;
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "CR", 0))
                {
                  /* echo */
                  ptr_deb += 3;
                  switch (*ptr_deb)
                    {
                    case 'O':
                      x01_set[las].cr = TRUE;
                      ptr_deb++;
                      if (ptr_deb >= ptr_fin)
                        erreur = FALSE;
                      break;
                    case 'N':
                      x01_set[las].cr = FALSE;
                      ptr_deb++;
                      if (ptr_deb >= ptr_fin)
                        erreur = FALSE;
                      break;
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "LPL", 0))
                {
                  ptr_deb += 4;
                  /* on recherche la liste des parametres */
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      if (val <= 255)
                        {
                          x01_set[las].lpl = (UINT8) val;
                          erreur = FALSE;
                        }
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "MTEL", 0))
                {
                  /* mode minitel video-text */
                  ptr_deb += 5;
                  switch (*ptr_deb)
                    {
                    case 'O':
                      x01_set[las].mtel = TRUE;
                      ptr_deb++;
                      if (ptr_deb >= ptr_fin)
                        erreur = FALSE;
                      break;
                    case 'N':
                      x01_set[las].mtel = TRUE;
                      ptr_deb++;
                      if (ptr_deb >= ptr_fin)
                        erreur = FALSE;
                      break;
                    }
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
    {
      x01_cptr.erreur = CPTRD_SYNTAXE;
    }
  else
    {
      /* on re configure la liaison  */
      mncfuart ((LONG) las,
                (LONG) xdg_cf_las[las].vitesse,
                (LONG) xdg_cf_las[las].data,
                (LONG) xdg_cf_las[las].parite,
                (LONG) xdg_cf_las[las].stop, (LONG) xdg_cf_las[las].media);

    }                                  /* endif(erreur                                         */
  /* --------------------------------
   * FIN DE cmd_set
   * --------------------------------     */
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_setu                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 16/12/1996                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_set.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande SET                           /
/-------------------------------------------------------------------------DOC*/

/* --------------------------------
 * cmd_setu
 * ========
 * Traitement des parametres de la
 * La commande a ete modifiee pour
 * les besoins de SIRIUS. Seuls les
 * parametres pour le port serie
 * sont utilises.
 * Valeur de retour :
 * - la fonction retourne 0 si il
 *   n'y a pas d'erreur de parame-
 *   tre, 2 si le parametre est
 *   incorrect et 3 si la valeur du
 *   parametre est incorrecte.
 * --------------------------------	*/

static int cmd_setu (STRING buffer, INT nbcar)
{
  INT las;
  UINT8 erreur = 0;
  STRING ptr_deb = buffer;
  STRING ptr_fin = buffer;

  buffer[nbcar] = 0;
  while (((INT) (ptr_fin - buffer) <= nbcar) && (!erreur))
    {
      switch (*ptr_fin)
        {
        case 0:
        case ' ':
          /* si deux separateur ne se suivent
           * pas */
          if (ptr_deb != ptr_fin)
            {
              /* a priori il y a une erreur sur la valeur d'un parametre */
              erreur = 3;
              if ((*ptr_deb == 'Z') || (*ptr_deb == 'S'))
                {
                  ptr_deb++;
                  if (ptr_deb >= ptr_fin)
                    {
                      tac_las_init_cfg ();
                      erreur = FALSE;
                      printDebug ("cmd_setu : erreur  = FALSE\n");
                    }
                  else
                    {
                      /* parite */
                      las = *ptr_deb - '1';
                      if ((NB_PORT_TEDI>las)&&(las>=0))
                        {
                          if (++ptr_deb >= ptr_fin)
                            {
                              tac_las_init_cfg_las (las);
                              erreur = FALSE;
                              printDebug ("cmd_setu : erreur  = FALSE\n");
                            }
                        }
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "BD", 0))
                {
                  *ptr_fin = 0;
                  /* vitesse de transmission */
                  las = ptr_deb[2] - '1';
                  ptr_deb += 4;
                      if ((NB_PORT_TEDI>las)&&(las>=0))
                    {
                      LONG val_long;
                      /* on recherche la liste des parametres */
                      if (dv1_str_atol (&ptr_deb, &val_long))
                        {
                          switch (val_long)
                            {
                            case 300:
                            case 600:
                            case 1200:
                            case 2400:
                            case 4800:
                            case 9600:
                            case 19200:
                            case 38400:
                            case 115200:
                              xdg_cf_las[las].vitesse =
                                (UINT) (val_long);
                              if (ptr_deb >= ptr_fin)
                                erreur = FALSE;
                              printDebug ("cmd_setu : erreur  = FALSE\n");
                              break;
                            }          /* endswitch(val_long                           */
                        }
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "PA", 0))
                {
                  *ptr_fin = 0;
                  /* parite */
                  las = ptr_deb[2] - '1';
                  ptr_deb += 4;
                      if ((NB_PORT_TEDI>las)&&(las>=0))
                    {
                      switch (*ptr_deb)
                        {
                        case 'P':
                          /* parite paire */
                          xdg_cf_las[las].parite = PAIRE;
                          ptr_deb++;
                          if (ptr_deb >= ptr_fin)
                            erreur = FALSE;
                          printDebug ("cmd_setu : erreur  = FALSE\n");
                          break;
                        case 'I':
                          /* parite impaire  */
                          xdg_cf_las[las].parite = IMPAIRE;
                          ptr_deb++;
                          if (ptr_deb >= ptr_fin)
                            erreur = FALSE;
                          printDebug ("cmd_setu : erreur  = FALSE\n");
                          break;
                        case 'N':
                          /* sans parite */
                          xdg_cf_las[las].parite = SANSPARITE;
                          ptr_deb++;
                          if (ptr_deb >= ptr_fin)
                            erreur = FALSE;
                          printDebug ("cmd_setu : erreur  = FALSE\n");
                          break;
                        }
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "ST", 0))
                {
                  *ptr_fin = 0;
                  /* nombre de bits de stop */
                  las = ptr_deb[2] - '1';
                  ptr_deb += 4;
                      if ((NB_PORT_TEDI>las)&&(las>=0))
                    {
                      INT val_int;
                      /* on recherche la liste des parametres */
                      if (dv1_str_atoi (&ptr_deb, &val_int))
                        {
                          /* si la valeur est correcte */
                          if (val_int < 3)
                            {
                              xdg_cf_las[las].stop = (UINT8) val_int;
                              if (ptr_deb >= ptr_fin)
                                erreur = FALSE;
                              printDebug ("cmd_setu : erreur  = FALSE\n");
                            }
                        }
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "LG", 0))
                {
                  *ptr_fin = 0;
                  /* nombre de bits de data */
                  las = ptr_deb[2] - '1';
                  ptr_deb += 4;
                      if ((NB_PORT_TEDI>las)&&(las>=0))
                    {
                      INT val_int;
                      /* on recherche la liste des parametres */
                      if (dv1_str_atoi (&ptr_deb, &val_int))
                        {
                          /* si la valeur est correcte */
                          if ((val_int < 9) && (val_int > 5))
                            {
                              xdg_cf_las[las].data = (UINT8) val_int;
                              if (ptr_deb >= ptr_fin)
                                erreur = FALSE;
                              printDebug ("cmd_setu : erreur  = FALSE\n");
                            }
                        }
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "PR", 0))
                {
                  *ptr_fin = 0;
                  /* mode protege */
                  las = ptr_deb[2] - '1';
                  ptr_deb += 4;
                      if ((NB_PORT_TEDI>las)&&(las>=0))
                    {
                      switch (*ptr_deb)
                        {
                        case 'O':
                          /* mode protege */
                          xdg_cf_las[las].protect = TRUE;
                          ptr_deb++;
                          if (ptr_deb >= ptr_fin)
                            erreur = FALSE;
                          printDebug ("cmd_setu : erreur  = FALSE\n");
                          break;
                        case 'N':
                          /* mode protege */
                          xdg_cf_las[las].protect = FALSE;
                          ptr_deb++;
                          if (ptr_deb >= ptr_fin)
                            erreur = FALSE;
                          printDebug ("cmd_setu : erreur  = FALSE\n");
                          break;
                        }
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "TAL", 0))
                {
                  *ptr_fin = 0;
                  /* nombre de caractere de suffixe */
                  las = ptr_deb[3] - '1';
                  ptr_deb += 5;
                      if ((NB_PORT_TEDI>las)&&(las>=0))
                    {
                      INT val_int;
                      /* on recherche la liste des parametres */
                      if (dv1_str_atoi (&ptr_deb, &val_int))
                        {
                          /* si la valeur est correcte */
                          if (val_int <= 999)
                            {
                              xdg_cf_las[las].suffixe =
                                (UINT8) val_int;
                              if (ptr_deb >= ptr_fin)
                                erreur = FALSE;
                              printDebug ("cmd_setu : erreur  = FALSE\n");
                            }
                        }
                    }
                }
              else if (dv1_scmp (ptr_deb, (STRING) "XMT", 0))
                {
                  *ptr_fin = 0;
                  /* nature du media/nombre de car de prefixe */
                  las = ptr_deb[3] - '1';
                  ptr_deb += 5;
                      if ((NB_PORT_TEDI>las)&&(las>=0))
                    {
                      INT val_int;
                      switch (*ptr_deb)
                        {
                        case 'R':
                          /* radio */
                        case 'C':
                          /* RTC */
                        case 'P':
                          /* ligne privee */
                        case 'L':
                          /* local */
                          /* mode protege */
                          xdg_cf_las[las].media = *ptr_deb;
                          ptr_deb++;
                          if (dv1_str_atoi (&ptr_deb, &val_int))
                            {
                              /* si la valeur est correcte */
                              if (val_int <= 999)
                                {
                                  xdg_cf_las[las].prefixe =
                                    (UINT8) val_int;
                                  if (ptr_deb >= ptr_fin)
                                    erreur = FALSE;
                                }
                            }
                          break;
                        }              /* endswitch (*ptr_deb)                         */
                    }
                }
              else
                {
                  erreur = 2;
                }
            }                          /* endif (ptr_deb != ptr_fin)           */
          /* on passe au parametre suivant */
          ptr_fin++;
          /* on supprime tous les blancs */
          while (*ptr_fin == ' ')
            {
              ptr_fin++;
            }
          /* sur le parametre suivant */
          ptr_deb = ptr_fin;
          break;
        default:
          printDebug ("cmd_setu : dans default : *ptr_fin %d %c \n",
                      *ptr_fin, *ptr_fin);
          ptr_fin++;
          break;
        }                              /* endswitch (*ptr_fin)                         */
    }                                  /* endwhile (((INT) (ptr_fin-buff       */
  if (erreur)
    {
      x01_cptr.erreur = CPTRD_SYNTAXE;
      printDebug ("cmd_setu :erreur\n");
    }
  return (erreur);
}


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : set_port_log                                           /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 21/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_set.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande SET                           /
/-------------------------------------------------------------------------DOC*/
INT lcr_set (las, mode, lg_mess, buffer, position, pt_mess)
     INT las;                   /* numero du las sur lequel se fait le transfert */
     INT mode;                  /* mode de transmission                          */
     INT lg_mess;               /* nombre de caractere dans le buffer            */
     STRING buffer;             /* adresse du buffer                             */
     INT position;              /* position dans le buffer                       */
     struct usr_ztf *pt_mess;   /* pointeur vers un message de transmission      */


{                                      /* debut de set_port_log */

  INT i, j;                     /* variable de boucle lecture du buffer            */

/* on se place apres le nom de la commande */
  i = position + 3;


#if DEBUG > 0
  printDebug ("lcr_set : au debut\n");
#endif


/* si on est pas en fin de message, on continue l'analyse */
  if (i < lg_mess)
    {
      /* on teste si l'operateur est habilite */
      if (identLireAutorisationAvecNiveau (las, 0))
        {
          /* taille indique le nombre de caracteres du parametre */
          cmd_set (&buffer[i], lg_mess - i, las);
        }
      else
        x01_cptr.erreur = CPTRD_PROTOCOLE;
    }

/* si on a pas detecte d'erreur */
  if (x01_cptr.erreur == CPTRD_OK)
    {
      /* pour l'echo */
      j = dv1_scpy (buffer, "SET ECHO=", 0);
      if (x01_set[las].echo)
        buffer[j++] = 'O';
      else
        buffer[j++] = 'N';

      /* ajout de LF devant CR en echo */
      j += dv1_scpy (&buffer[j], " CR=", 0);
      if (x01_set[las].cr)
        buffer[j++] = 'O';
      else
        buffer[j++] = 'N';

      /* nombre de replis de ligne */
      j += sprintf (&buffer[j], " LPL=%d", x01_set[las].lpl);

      /* videotext */
      j += dv1_scpy (&buffer[j], " MTEL=", 0);
      if (x01_set[las].mtel)
        buffer[j++] = 'O';
      else
        buffer[j++] = 'N';

      /* on envoie le bloc de la transmission */
      tedi_send_bloc (las, mode, buffer, j, 0, TRUE, pt_mess);

    }
  else
    {

      tedi_erreur (las, mode);

    }
  return (TRUE);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : set_uart                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 21/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_set.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande SETU                          /
/-------------------------------------------------------------------------DOC*/
INT lcr_setu (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
              T_usr_ztf * pt_mess)
{
  INT i;                        /* variable de boucle lecture du buffer            */
  INT j;                        /* variable de boucle ecriture du buffer           */
  INT init = FALSE;
  int erreur = 0;
  LONG vitesseLiaison = xdg_cf_las[las].vitesse;

  /* on se place apres le nom de la
   * commande */
  i = position + 4;
  /* si on est pas en fin de message,
   * on continue l'analyse */
  if (i < lg_mess)
    {
      /* on teste si l'operateur est
       * habilite */
      if (identLireAutorisationAvecNiveau (las, 0))
        {
          /* pour la configuration des
           * liaisons */
          dv1_scpy ((STRING) xdg_sv_cf_las, (STRING) xdg_cf_las,
                    NBPORT * sizeof (struct xdg_cf_las));
          /* taille indique le nombre de
           * caracteres du parametre */
          erreur = cmd_setu (&buffer[i], lg_mess - i);
          /* si la commande SETU s'est bien
           * passee, on modifie la config des
           * uarts         */
          if (x01_cptr.erreur == CPTRD_OK)
            {
              init = TRUE;
            }
          else
            {
              dv1_scpy ((STRING) xdg_cf_las, (STRING) xdg_sv_cf_las,
                        NBPORT * sizeof (struct xdg_cf_las));
            }
        }
      else
        {
          x01_cptr.erreur = CPTRD_PROTOCOLE;
          erreur = 5;
        }
    }

  /* si on a pas detecte d'erreur */
  if (x01_cptr.erreur == CPTRD_OK)
    {
      printDebug ("lcr_setu : si on a pas detecte d'erreur NBPORT %d\n",
                  NBPORT);
      /* on initialise l'indice de
       * remplissage du buffer */
      /* Pour le port serie.                  */
      j = 0;
      i = PORT_SERIE;
      /* si on est en TEDI */
      for(i=0;i<NB_PORT_TEDI;i++){

      if (xdg_cf_las[i].protocole == TEDI_PROT)
        {
          j += sprintf (&buffer[j],  "SETU %d PROT=T XMT=",i+1);
          buffer[j++] = xdg_cf_las[i].media;
          j += sprintf (&buffer[j], "%d", xdg_cf_las[i].prefixe);
          j += sprintf (&buffer[j], " BD=%ld PA=",
                             xdg_cf_las[i].vitesse);
          switch (xdg_cf_las[i].parite)
            {
            case PAIRE:
              buffer[j++] = 'P';
              break;

            case IMPAIRE:
              buffer[j++] = 'I';
              break;

            case SANSPARITE:
              buffer[j++] = 'N';
              break;
            }
          /* nombre de bits de stop longueur */
          /* vitesse de transmission */
          j += sprintf (&buffer[j], " ST=%1d LG=%d PR=",
                            xdg_cf_las[i].stop, xdg_cf_las[i].data);

          /* protection en mode terminal */
          if (xdg_cf_las[i].protect)
            {
              buffer[j++] = 'O';
            }
          else
            {
              buffer[j++] = 'N';
            }
          /* nombre de caracteres de suffixe */
          j += sprintf (&buffer[j], " TAL=%d\n\r", xdg_cf_las[i].suffixe);
        }
      }
      /* Pour faciliter la comprehension, le numero de port est 4 maintenant. */
      j += sprintf (&buffer[j],
                        "SETU %d PROT=50170 XMT=P0 BD=31250 PA=N ST=1 LG=8 TAL=0",i+1+1);
      /* on envoie le bloc de la transmission */
      tedi_send_bloc (las, mode, buffer, j, 0, TRUE, pt_mess);
    }
  else
    {
      tedi_ctrl_ext (las, mode, erreur);
    }
  /* si il faut initialiser les ports */
  if (init)
    {
      /* on attends une seconde */
      switch (vitesseLiaison)
        {
        case 300:
          mnsusp (256);
          break;
        case 600:
          mnsusp (128);
          break;
        case 1200:
          mnsusp (64);
          break;
        case 2400:
          mnsusp (32);
          break;
        case 4800:
        case 9600:
        case 19200:
        case 38400:
        case 115200:
        default:
          mnsusp (16);
          break;
        }
      j = PORT_SERIE;
      if (xdg_cf_las[j].protocole == TEDI_PROT)
        {
          mncfuart (j, xdg_cf_las[j].vitesse, xdg_cf_las[j].data,
                    xdg_cf_las[j].parite, xdg_cf_las[j].stop,
                    xdg_cf_las[j].media);
        }                              /* endif(j=0;j<NB                                       */
    }                                  /* endif(init                                           */
  /* --------------------------------
   * FIN DE lcr_setu
   * --------------------------------     */
  return (init);
}
