/*
 * LCR_ST1.c
 * ===================
 * FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la con-
 * figuration des parametres de status de la station.
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: LCR_ST1.c,v $
 * Revision 1.2  2018/06/04 08:38:38  xg
 * Passage en version V122
 *
 * Revision 1.9  2010/09/15 15:56:03  xgaillard
 * Suppression de quelques warnings.
 *
 * Revision 1.8  2009/08/24 08:25:32  xgaillard
 * renomage de moduleLire par versionLire
 *
 * Revision 1.7  2009/04/16 13:19:45  xag
 * Suppression warning de compilation.
 *
 * Revision 1.6  2008/10/24 15:50:08  xag
 * Ajout de la commande ST STR
 *
 * Revision 1.5  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.4  2008/10/03 16:21:46  xag
 * *** empty log message ***
 *
 * Revision 1.3  2008/09/12 14:57:09  xag
 * *** empty log message ***
 *
 * Revision 1.2  2008/07/04 17:05:07  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include <string.h>
#include "standard.h"                  /* redefinition des types                         */
#include "portage.h"
#include "mon_inc.h"
#include "define.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "pip_def.h"                   /* definition des constantes                       */
#include "pip_str.h"                   /* definition des constantes                       */
#include "pip_var.h"                   /* definition des constantes                       */
#include "tac_conf.h"                  /* definition des constantes                       */
#include "lcr_init.h"                  /* definition des constantes                       */

#include "sir_dv1h.h"
#include "xdg_var.h"
#include "xversion.h"
#include "ted_prot.h"
#include "identLib.h"
#include "eriLib.h"
#include "lcr_st1.h"
#include "lcr_trc.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "ioPortLib.h"
#include "mgpLib.h"


/* Nouvelles declation pour la gestion de la configuration */
#include "configuration.h"
#include "configurationSram.h"

/* ****************************
 * DEFINITION DES CONSTANTES
 * ****************************	*/

#define TP_3600 3600
#define ST_REPONSE "STATUS"
#define ST_MAX_PHASE 31



/* ****************************
 * DEFINITION DES MACROS
 * ****************************	*/

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ****************************
 * PROTOTYPES DES FONCTIONS LOCALES
 * ****************************	*/

VOID raz_erreur (VOID);
INT par_canal (STRING, INT, INT);
INT par_general (STRING, INT);
INT st_canal (INT, STRING, INT);
static INT st_general (STRING, INT);
static void format_st_general (INT, INT,char *, struct usr_ztf *);

static int st_nbModHs ();
static int st_nbCarHs ();
static int st_nbPixHs ();

/* *************************************
 * FONCTIONS DU MODULE
 * ************************************* */
/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_st_evt                                           /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 05/12/98                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_set.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : positionnement d'un evenement                        /
/-------------------------------------------------------------------------DOC*/
VOID lcr_st1_evt (char * evt)
{
  mnlcal (x01_status3.evt.date);
  strncpy ((char *)x01_status3.evt.nom, evt, 3);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : raz_erreur                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_st1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : remise a zero des indicateur d'erreur sur les ports    /
/-------------------------------------------------------------------------DOC*/
VOID raz_erreur (VOID)
{

  /* on raz les erreurs des ports de communication */
  x01_status3.er1 = 0;
  x01_status3.er2 = 0;
  x01_status3.er3 = 0;
  /* on raz l'indication d'INIT du status temps reel */
  vct_ST_TR &= ~STATUS_TR_INIT;
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_general                                            /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_st1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification d'un parametre de status general de la    /
/                      station                                                /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/                     buffer = buffer comprenant le parametre a traiter       /
/                     taille = taille du buffer                               /
/-------------------------------------------------------------------------DOC*/
INT par_general (STRING buffer, INT taille)
{
  INT i;                        /* variable de boucle */
  INT val;                      /* valeur entiere pour la conversion chaine --> entier */

  INT flag_err;                 /* flag d'erreur ds l'analyse de la question */


/* on suppose pas d'erreur */
  flag_err = 0;

  i = dv1_car (buffer, taille, '=');

  if ((i == taille) || (i != 3))
    flag_err = 1;
  else
    {                                  /* EIF1 */

      /* code de l'equipement */
      if (0==strncmp (buffer, "COD", 3))
        {                              /* IF2 */

          /* le parametre est-il valide ? */
          if (taille == 11)
            {                          /* IF3 */
               char *adr = x01_status1.adr;

              /* on enregistre le parametre */
              strncpy (x01_status1.cod, &buffer[(int) i + 1], 7);

              /* on lit les caracteres de l'adresse de la station */
              adr[(int) 0] = buffer[(int) i + 2];
              adr[(int) 1] = buffer[(int) i + 3];
              adr[(int) 2] = buffer[(int) i + 7];


            }                          /* FIF3 */
          else
            flag_err = 1;

        }                              /* FIF2 */
      else
        {                              /* EIF2 */

          /* localisation */
          if (0==strncmp (buffer, (STRING) "LOC", 3))
            {                          /* IF3 */

              /* le parametre est-il valide ? */
              if ((taille > 4) && (taille <= 18))
                {                      /* IF4 */

                  /* on enregistre le parametre */
                  strncpy (x01_status1.loc, (STRING) "              ", 14);
                  strncpy (x01_status1.loc, &buffer[(int) i + 1],
                            taille - 4);

                }                      /* FIF4 */
              else
                flag_err = 1;

            }                          /* FIF3 */
          else
            {                          /* EIF3 */

              if (0==strncmp (buffer, (STRING) "NST", 3))
                {                      /* IF4 */

                  /* le parametre est-il valide ? */
                  if ((taille > 4) && (taille <= 8))
                    {                  /* IF5 */

                      /* on enregistre le parametre */
                      strncpy (x01_status1.nst, (STRING) "    ", 4);
                      strncpy (x01_status1.nst, &buffer[(int) i + 1],
                                taille - 4);

                    }                  /* FIF5 */
                  else
                    flag_err = 1;

                }                      /* FIF4 */
              else
                {                      /* EIF4 */

                  /* caractere EOL */
                  if (0==strncmp (buffer, (STRING) "EOL", 3))
                    {                  /* IF5 */

                      /* le parametre est-il valide ? */
                      if ((taille > 4) && (taille <= 7))
                        {              /* IF6 */

                          /* le parametre est il un nombre */
                          if (!dv1_atoi
                              (&buffer[(int) i + 1], taille - 4, &val))
                            flag_err = 1;
                          else
                            {          /* EIF7 */

                              dv1_itoa (x01_status1.eol, val, 3);
                              x01_status1.eol[3]=0;

                            }          /* FEIF7 */

                        }              /* FIF6 */
                      else
                        flag_err = 1;

                    }                  /* FIF5 */
                  else
                    {                  /* EIF5 */

                      /* BAT */
                      if (0==strncmp (buffer, (STRING) "BAT", 3))
                        {              /* IF6 */

                          /* le parametre est-il valide ? */
                          if ((taille > 4) && (taille <= 7))
                            {          /* IF7 */

                              /* le parametre est il un nombre */
                              if (!dv1_atoi
                                  (&buffer[(int) i + 1], taille - 4, &val))
                                flag_err = 1;
                              else
                                {      /* EIF8 */

                                  dv1_itoa (x01_status1.bat, val, 3);

                                }      /* FEIF8 */

                            }          /* FIF7 */

                        }              /* FIF6 */
                      else
                        {              /* EIF6 */

                          flag_err = 1;

                        }              /* FEIF6 */

                    }                  /* FEIF5 */

                }                      /* FEIF4 */

            }                          /* FEIF3 */

        }                              /* FEIF2 */

    }                                  /* FEIF1 */

  return (flag_err);

}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : st_general                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_st1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : traitement des parametres generaux de la cde  ST       /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/                     buffer = buffer de question comprenant les param canaux /
/                     lg_par = taille du buffer                               /
/-------------------------------------------------------------------------DOC*/
static INT st_general (STRING buffer, INT lg_par)
{
  INT i = 0;                    /* indice de lecture de buffer               */
  INT fini = FALSE;             /* variable de controle de boucle            */
  INT flag_err = 0;             /* drapeau de detection d'erreur             */
  INT taille;                   /* taille d'un parametre de canal            */
  /* si on est pas au bout du buffer, on continue l'analyse */
  if (i < lg_par)
    {                                  /* IF1 */
      while (!fini)
        {                              /* W1 */
          /* on recherche la fin du parametre */
          taille = dv1_cherchefin (&buffer[(int) i], lg_par - i);
          flag_err = par_general (&buffer[(int) i], taille);
          /* si on a trouve une erreur, on termine le decodage */
          if (flag_err != 0)
            {
              fini = TRUE;
            }
          /* on passe au parametre suivant */
          i += taille;
          /* on elimine les separateurs */
          if ((i += dv1_separateur (&buffer[(int) i], lg_par - i)) >= lg_par)
            {
              fini = TRUE;
            }
        }                              /* FW1 */
    }                                  /* FIF1 */
  return (flag_err);
}                                      /* fin de st_general   */

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : format_st_general                                      /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_st1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : formattage de la reponse a une cde ST general          /
/-------------------------------------------------------------------------DOC*/
static void format_st_general (INT las, INT mode, char * buffer,
                        struct usr_ztf * pt_mess)
{
  UINT8 bloc;
  UINT8 fini;
  UINT8 phase;
  INT j;
  fini = FALSE;
  phase = 0;
  j = 0;
  bloc = 0;

  while (fini == FALSE)
    {
      switch (phase)
        {
        case 0:
          /* ADRESSE */
          j+=sprintf (&buffer[(int) j], "STATUS ADR=");
          j+=snprintf (&buffer[(int) j],4,"%s", x01_status1.adr);
          break;

        case 1:
          /* COD */
        	j+=sprintf (&buffer[(int) j], " COD=");
          j+=snprintf(&buffer[(int) j],8,"%s", x01_status1.cod);
          break;

        case 2:
          /* LOCALISATION */
          j+=sprintf (&buffer[(int) j], " LOC=");
          j+=snprintf (&buffer[(int) j],15,"%s", x01_status1.loc);
          break;

        case 3:
          /* VERSION  */
          j += sprintf (&buffer[ j],  " VER=%d.%d",
                            versionLireMajeure ("Application"),
                            versionLireMineure ("Application"));
          break;

        case 4:
          /* GENERATION  */
          j += sprintf (&buffer[(int) j],  " GEN=SIA.C01");
          break;

        case 5:
          /* EDF  */
          j += sprintf (&buffer[(int) j],
                            (STRING) " EDF=%1c", x01_status3.edf);
          break;


        case 6:
          /* CHIEN DE GARDE  */
          j += sprintf (&buffer[(int) j],
                            (STRING) " GAR=%03lu", configGetGar());
          break;

        case 7:
          /* RESTART  */
          j += sprintf (&buffer[(int) j],
                            (STRING) " RST=%03lu", configGetRst());
          break;

        case 8:
          /* INITIALISATION  */
          j += sprintf (&buffer[(int) j],
                            (STRING) " INI=%03lu", configGetIni());
          break;

        case 9:
          /* SCRUTATION PC  */
          j += sprintf (&buffer[(int) j],
                            (STRING) " SPC=%03lu",configGetSpc());
          break;

        case 10:
          /* TERMINAL  */
          j += sprintf (&buffer[(int) j],
                            (STRING) " TRM=%03d", x01_status3.trm);
          break;

        case 11:
          /* ERR  */

          {
            int err = 0;

            if (x01_status3.rebouclage)
              {
                /* si on est en rebouclage */
                err = 1;
              }
            else if (pip_nb_module == 0)
              {
                /* absence de configuration */
                err = 2;
              }
            else
              {

                if (mgpInit()==false)
                  {
                    /* absence carte MGP102 */
                    err = 6;
                  }
               }
            j += sprintf (&buffer[j], " ERR=%02X", err);
          }
          break;

        case 12:
          /* ER1  */
          j += sprintf (&buffer[j],
                            (STRING) " ER1=%02d", x01_status3.er1);
          break;

        case 13:
          /* ER2  */
          j += sprintf (&buffer[j],
                            (STRING) " ER2=%02d", x01_status3.er2);
          break;

        case 14:
          /* ER3  */
          j += sprintf (&buffer[j],
                            (STRING) " ER3=%02d", x01_status3.er3);
          break;

        case 15:
          /* ERI  */
          {
            int nbErreur = eriLireNbErreur ();
            if (0 == nbErreur)
              {
                strncpy (&buffer[(int) j], (STRING) " ERI=0", 6);
                j += 6;
              }
            else
              {
                char grav, nature;
                eriLireErreur (nbErreur - 1, &grav, &nature);
                j += sprintf (&buffer[j],
                                  (STRING) " ERI=10%01d%02d",
                                  grav, nature);
              }
          }
          break;

        case 16:
          /* NST  */
          strncpy (&buffer[(int) j], (STRING) " NST=", 5);
          j += 5;
          strncpy (&buffer[(int) j], x01_status1.nst, 4);
          j += 4;
          break;

        case 17:
          /* BAT  */
          strncpy (&buffer[(int) j], (STRING) " BAT=", 5);
          j += 5;
          strncpy (&buffer[(int) j], x01_status1.bat, 3);
          j += 3;
          break;

        case 18:
          /* BRT  */
          j+=sprintf (&buffer[(int) j], (STRING) " BTR=%s",(esIsDefautAlim()?"1":"0"));
          break;

        case 19:
          /* CHEKSUM  */
          j += sprintf (&buffer[(int) j], " CKS=%04X", vct_cks_prom);
          break;

        case 20:
          /* EOL  */
          j += sprintf (&buffer[(int) j]," EOL=");
          j += snprintf (&buffer[(int) j],4, x01_status1.eol);
          break;

        case 21:
          break;
        case 22:
          break;
        case 23:
	  {
	  char adresse;
		if(ioPortLireAdresse(&adresse))
		{
          	j += sprintf (&buffer[(int) j], " ADT=%06X",adresse);
		}
		else
		{
          	j += sprintf (&buffer[(int) j], " ADT=?");
		}
	  }
          break;

        case 24:
          /* EVT  */
          j += sprintf (&buffer[(int) j], " EVT=");
          j += snprintf (&buffer[(int) j],4,"%s", x01_status3.evt.nom);

          buffer[(int) j++] = ':';
          /* on formatte l'horodate de l'evenement */
          /* pour tous les champs de l'horodate */

          j += dv1_itoa (&buffer[(int) j], x01_status3.evt.date[(int) 4], 2);
          buffer[(int) j++] = '/';
          j += dv1_itoa (&buffer[(int) j], x01_status3.evt.date[(int) 5], 2);
          buffer[(int) j++] = '/';
          j += dv1_itoa (&buffer[(int) j], x01_status3.evt.date[(int) 6], 2);
          buffer[(int) j++] = ' ';
          j += dv1_itoa (&buffer[(int) j], x01_status3.evt.date[(int) 2], 2);
          buffer[(int) j++] = ':';
          j += dv1_itoa (&buffer[(int) j], x01_status3.evt.date[(int) 1], 2);
          buffer[(int) j++] = ':';
          j += dv1_itoa (&buffer[(int) j], x01_status3.evt.date[(int) 0], 2);
          break;
#ifdef PMV
        case 25:
          j += sprintf (&buffer[(int) j], " MOD=%d", st_nbModHs ());
          break;
        case 26:
          j += sprintf (&buffer[(int) j], " LHS=%d", 0/* st_nbLhs ()*/);
          break;
        case 27:
          j += sprintf (&buffer[(int) j], " CAR=%d", st_nbCarHs ());
          break;
        case 28:
          j += sprintf (&buffer[(int) j], " OCC=%d", 0/*st_nbOccHs ()*/);
          break;
        case 29:
          j += sprintf (&buffer[(int) j], " TRS=%d", 0/* st_nbTrsHs ()*/);
          break;
        case 30:
          j += sprintf (&buffer[(int) j], " PIX=%d", st_nbPixHs ());
          break;
#endif

        default:
          fini = TRUE;
          break;
        }
      if ((++phase) < ST_MAX_PHASE)
        {
          if (j >= MAXLCR)
            {
              tedi_send_bloc (las, mode, buffer, j, bloc, FALSE, pt_mess);

              if (++bloc >= 10)
                bloc = 0;

              j = 0;

              if (vct_IT[(int) las])
                {
                  printDebug ("Il y a IT sur la ligne\n");
                  fini = TRUE;
                }
            }
        }
      else
        {
          /* apres le dernier champ  on envoie la reponse */
          fini = TRUE;
          tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
        }
    }
  /* on reinitialise les erreurs */
  raz_erreur ();

};

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : st1_general                                            /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_st1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande ST                            /
/-------------------------------------------------------------------------DOC*/
INT st1_st (INT las, INT mode, INT lg_mess, char * buffer, INT position,
            T_usr_ztf * pt_mess)
{
  INT i;
  INT j;
  INT taille;
  INT flag_err = 0;
  INT bloc = 0;
  INT config = FALSE;

  /* on se place apres le nom de la commande */
  i = position + 2;

  /* on elimine les separateurs */
  i += dv1_separateur (&buffer[(int) i], lg_mess - i);

  /* si on est pas en fin de message, on continue l'analyse */
  if (i < lg_mess)
    {
      /* on cherche la fin du parametre */
      taille = dv1_cherchefin (&buffer[(int) i], lg_mess - i);

	    if (0==strncmp (&buffer[(int) i], "MOD", 3))
        {
          int erreur = FALSE;

          if (0==strncmp (&buffer[(int) i], "MOD=", 4))
            {
              if (x01_status3.rebouclage)
                {
                  int indice = 0;
                  i += 4;
                  strcpy (x01_status3.mod_rebouclage, &buffer[i]);
                  /* on supprime les separateurs */
                  while (x01_status3.mod_rebouclage[indice])
                    {
                      if (x01_status3.mod_rebouclage[indice] == '-')
                        x01_status3.mod_rebouclage[indice] = ' ';
                      indice++;
                    }
                }
              else
                erreur = TRUE;
            }
          if (FALSE == erreur)
            {
              j = sprintf (buffer, "%s MOD", ST_REPONSE);
              if (!x01_status3.rebouclage)
                {
                  int k;
                  /* les modules.caissons HS */
                  for (k = 0; k < pip_nb_module; k++)
                    {
                      int l;
                      for (l = pip_cf_module[k].num_caisson;
                           l <
                           pip_cf_module[k].num_caisson +
                           pip_cf_module[k].nb_caisson; l++)
                        {
                         	uint16 etatAff_uw=configEtatCaisson(l);
                          /* si le caisson est en defaut */
                          if (etatAff_uw & ERREUR_MAJEURE)
                            {
                              j +=
                                sprintf (&buffer[j], " %d.%d",
                                             pip_cf_module[k].id_module,
                                             pip_cf_caisson[l].caisson);
                            }
                        }
                    }
                }
              /* on envoie le bloc de la transmission */
              tedi_send_bloc (las, mode, (STRING)buffer, j, 0, TRUE, pt_mess);
            }
        }
      else if (dv1_scmp (&buffer[(int) i], "PIX", 3))
        {
          j = dv1_sprintf (buffer, "%s PIX", ST_REPONSE);
          if (!x01_status3.rebouclage)
            {
        	  /* Faire une bouche sur toutes les cartes.... */
        	  /* On retourne le numero de la carte et son adresse puis le nombre
        	   * de pixels si il y des pixels en defaut... */
        	  int nbAff=configGetNbAfficheurs();

              for (i = 0; i < nbAff; i++)
                {
            	int32 adresse_dw=configGetAdresseAfficheur(i);
            	int32 nbPixelsHs_dw=configEtatAffGetNbDefautsPixel(i);
            	int32 numCaisson=configGetCaissonAfficheur(i);
            	int32 indModule=pip_cf_caisson[numCaisson].id_module;
            	int32 indCaisson=pip_cf_caisson[numCaisson].caisson;
					if(0!=nbPixelsHs_dw)
					{

                      if (j > 200)
                        {
                          /* on envoie le bloc de la transmission */
                          tedi_send_bloc (las, mode, buffer, j, bloc, FALSE,
                                          pt_mess);
                          j = 0;
                          if (++bloc == 10)
                            bloc = 0;
                        }
                      j +=
                        dv1_sprintf (&buffer[j], " %d.%d.%d=%d",
									indModule,indCaisson,i,nbPixelsHs_dw);
					}
                }
            }
          /* on envoie le bloc de la transmission */
          tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
        }

      else if (0==strncmp (&buffer[(int) i], "STR", 3))
        {
            int nbErreur = eriLireNbErreur ();
          j =
            sprintf (&buffer[0], "%s STR BTR=0 EDF=%1c",ST_REPONSE, x01_status3.edf);
            if (0 == nbErreur)
            {
            	strncpy (&buffer[(int) j], (STRING) " ERI=0", 6);
            	j+=6;
            }
            else
            {
               char grav, nature;
                eriLireErreur (nbErreur - 1, &grav, &nature);
            	j+=sprintf(&buffer[(int) j], (STRING) " ERI=%01d%02d",
                                  grav, nature);
            }
            j += sprintf (&buffer[(int) j], " EVT=");
            j += snprintf (&buffer[(int) j],4,"%s", x01_status3.evt.nom);
            j+=sprintf(&buffer[j],
            	":%02d/%02d/%02d %02d:%02d:%02d GAR=%d INI=%d RST=%d TRM=%ld",
                         x01_status3.evt.date[(int) 4],
                         x01_status3.evt.date[(int) 5],
                         x01_status3.evt.date[(int) 6],
                         x01_status3.evt.date[(int) 2],
                         x01_status3.evt.date[(int) 1],
                         x01_status3.evt.date[(int) 0],
                         configGetGar(), configGetIni(), configGetRst(),
                         x01_status3.trm);
          tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
        }

      else if (0==strncmp (&buffer[(int) i], "CFG", 3))
        {
          j =
            sprintf (buffer, "%s CFG=%d", ST_REPONSE,
                         x01_status3.erreur_config);
          tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
        }
      else if (0==strncmp (&buffer[(int) i], "REB=0", 5))
        {
          if (x01_status3.rebouclage)
            {
              lcr_init_init ();
            }
          x01_status3.rebouclage = 0;
          format_st_general (las, mode, buffer, pt_mess);
        }
      else if (0==strncmp (&buffer[(int) i], "REB=1", 5))
        {
          if (!x01_status3.rebouclage)
            {
              lcr_init_init ();
            }
          x01_status3.rebouclage = 1;
          format_st_general (las, mode, buffer, pt_mess);
        }
      else if (0==strncmp (&buffer[(int) i], "REB=2", 5))
        {
          if (!x01_status3.rebouclage)
            {
              lcr_init_init ();
            }
          x01_status3.rebouclage = 2;
          format_st_general (las, mode, buffer, pt_mess);
        }
      else if (0==strncmp (&buffer[(int) i], "_INIT", 5))
        {
    	  configSetGar(0);
    	  configSetRst(0);
    	  configSetIni(0);
    	  configSetOvf(0);

          cmd_trc_tr ("Reinitialisatin WDG,RST,INIT", 0);
          format_st_general (las, mode, buffer, pt_mess);
        }

      else if (0==strncmp (&buffer[ i], "CKS", 3))
        {
          /* on remonte les checksum station */
          j = sprintf (buffer, "ST CKS %04X/%04X",
                           vct_cks_prom, vct_cks_prom_prom);
          tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
        }
      else
        {
          /* status general de la station */

          /* on teste si on est en lecture ou en ecriture */
          /* en lecture,la commande n'a pas de parametre  */
          j = i;

          j += dv1_separateur (&buffer[(int) j], lg_mess - j);

          if (j < lg_mess)             /* apres ST, il y a des parametres */
            {
              /* teste si l'operateur est habilite */
              if (identLireAutorisationAvecNiveau (las, 0))
                {
                  /* la commande a des parametres */
                  /* on traite les parametres generaux */
                  flag_err = st_general (&buffer[(int) j], lg_mess - j);
                  config = TRUE;

                }
              else
                flag_err = 3;
            }
          /* si on a pas trouve d'erreur */
          if (flag_err == 0)
            {
              /* on renvoie le status general de la station */
              format_st_general (las, mode, buffer, pt_mess);
            }
          else
            {
              x01_cptr.erreur = flag_err;
              tedi_erreur (las, mode);
            }
        }
    }
  else
    {

      /* lecture du status general de la station */
      format_st_general (las, mode, buffer, pt_mess);
    }
  return (config);
}


/* Fonctions ajoutees par XG pour comptabiliser le nombre
 * d'erreurs sur les differents composants du panneau. */

static int st_nbModHs ()
{
  int nbHs = 0;
  if (0 == x01_status3.rebouclage)
    {
      int k;
      /* les modules.caissons HS */
      for (k = 0; k < pip_nb_module; k++)
        {
          int l;
          for (l = pip_cf_module[k].num_caisson;
               l < pip_cf_module[k].num_caisson + pip_cf_module[k].nb_caisson;
               l++)
            {
           	uint16 etatAff_uw=configEtatCaisson(l);
            /* si le caisson est en defaut */
            if (etatAff_uw & ERREUR_MAJEURE)
                {
                  nbHs++;
                }
            }
        }
    }
  return nbHs;
}

static int st_nbCarHs ()
{
  int nbHs = 0;
//  if (0 == x01_status3.rebouclage)
//    {
//      int i;
//      for (i = 0; i < lon_nb_cd501; i++)
//        {
//          if (lon_cd501[i].mem_etat_aff & ERREUR_HS_DIODES2)
//            {
//              nbHs++;
//            }
//        }
//    }
  return nbHs;
}

static int st_nbPixHs ()
{
  int nbHs = 0;
//  if (0 == x01_status3.rebouclage)
//    {
//      int i;
//      for (i = 0; i < lon_nb_cd501; i++)
//        {
//          int k;
//          for (k = 0; k < 3; k++)
//            {
//              nbHs += lon_cd501[i].nb_pixel_def[k];
//            }
//        }
//    }
  return nbHs;
}

