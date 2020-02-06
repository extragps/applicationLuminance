/*
 * LCR_SC.c
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: LCR_SC.c,v $
 * Revision 1.2  2018/06/04 08:38:43  xg
 * Passage en version V122
 *
 * Revision 1.9  2009/01/23 09:41:00  xag
 * Adapatation pour pouvoir configurer un padding nul.
 *
 * Revision 1.8  2008/11/26 09:07:17  xag
 * Reintegration de la programmation du padding.
 *
 * Revision 1.7  2008/11/19 16:26:59  xag
 * Passage de la temporisation defaut secteur de 600 à 1 pour redemarrer automatiquement en mode neutre.
 *
 * Revision 1.6  2008/10/13 08:18:17  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.5  2008/10/03 16:21:47  xag
 * *** empty log message ***
 *
 * Revision 1.4  2008/09/29 07:58:54  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.3  2008/09/12 14:59:04  xag
 * Archivage de printemps
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
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "tac_conf.h"
#include "lcr_sc.h"
#include "identLib.h"
#include "configuration.h"
#include "lcr_pcp_cmd.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

LOCAL VOID lcr_cmd_tst_sc (STRING, INT);

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_tst_sc_init                                        /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/01/97                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_sc.c                                             /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION :initialisation des parametres generaux                  /
/-------------------------------------------------------------------------DOC*/
VOID lcr_tst_sc_init (VOID)
{
  /* ITM la periode d'interrogation des matrices */
										/* Changement de la valeur de diff_sect
										 * Par defaut, on redemarre en mode neutre.
										 */
  pip_cf_tst_sc.diff_sect = 1;        /* SCT reactivation sur defaut secteur */
  pip_cf_tst_sc.entretient = TRUE;

  /* Donnees ajoutees pour le PMV A12 par X.Gaillard */
  pip_cf_tst_sc.temp.tempo = 600;	/* TTE Tempo sur retombee temp.	*/
  pip_cf_tst_sc.temp.seuil = 70;	/* TSE Val du seuil de temperature	*/
  pip_cf_tst_sc.temp.actif = 0;		/* TAC Controle temperature.			*/
  pip_cf_tst_sc.autotests = 0; /* Autotests des afficheurs diode */
  pip_cf_tst_sc.ts_bp_test = 0 ; /* Relais utilise pour le bouton poussoir */
  pip_cf_tst_sc.tc_bp_led  = -1; /* Relais utilise pour la led BP */
}


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
LOCAL VOID lcr_cmd_tst_sc (STRING buffer,INT nbcar)
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
                      lcr_tst_sc_init ();
                    }
                  else
                    erreur = TRUE;
                }


                /* Tempo de scrutation cartes entree sortie *  (en secondes) */
               else if (dv1_scmp (ptr_deb, "SCE=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* pixel en defaut seuil 1 */
                      if (val > 0)
                      	configSetPollingEs((int32)val);
                      else
                        erreur = TRUE;
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
                /* Tempo de scrutation cartes temperature et luminosite
                 *  (en secondes) */
               else if (dv1_scmp (ptr_deb, "SCT=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* pixel en defaut seuil 1 */
                      if (val > 0)
                      	configSetPollingSonde((int32)val);
                      else
                        erreur = TRUE;
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
                /* Tempo de scrutation afficheur (en secondes) */
               else if (dv1_scmp (ptr_deb, "SCA=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* pixel en defaut seuil 1 */
                      if (val > 0)
                      	configSetPollingAff((int32)val);
                      else
                        erreur = TRUE;
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
               else if (dv1_scmp (ptr_deb, "SCA=", 0))
                 {
                   *ptr_fin = 0;
                   ptr_deb += 4;
                   erreur = FALSE;
                   if (dv1_str_atoi (&ptr_deb, &val))
                     {
                       /* pixel en defaut seuil 1 */
                       if (val > 0)
                       	configSetPollingAff((int32)val);
                       else
                         erreur = TRUE;
                     }
                   else
                     erreur = TRUE;

                   if (ptr_deb < ptr_fin)
                     erreur = TRUE;
                 }
               else if (dv1_scmp (ptr_deb, "PAB=", 0))
                 {
                   *ptr_fin = 0;
                   ptr_deb += 4;
                   erreur = FALSE;
                   if (dv1_str_atoi (&ptr_deb, &val))
                     {
                       /* pixel en defaut seuil 1 */
                       if (val >=0)
                       	configSetPaddingBefore((int32)val);
                       else
                         erreur = TRUE;
                     }
                   else
                     erreur = TRUE;

                   if (ptr_deb < ptr_fin)
                     erreur = TRUE;
                 }
              /* Valeur du seuil de luminosite jour */
                /* Valeur du seuil de luminosite jour */
               else if (dv1_scmp (ptr_deb, "PAA=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* pixel en defaut seuil 1 */
                      if (val >= 0)
                      	configSetPaddingAfter((int32)val);
                      else
                        erreur = TRUE;
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
                /* Valeur du seuil de luminosite surbrillance */
                 else if (dv1_scmp (ptr_deb, "SLS=", 0))
                {
                  /* fin du parametre */
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* pixel en defaut seuil 1 */
                      if (val > 0)
                      	configSetSeuilSurb((int32)val);
                      else
                        erreur = TRUE;
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
                else if (dv1_scmp (ptr_deb, "DP1=", 0))
                {
                  /* fin du parametre */
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* pixel en defaut seuil 1 */
                      if (val >= 0)
                      	configSetSeuilAffPixelMin((int32)val);
                      else
                        erreur = TRUE;
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
              else if (dv1_scmp (ptr_deb, "DP2=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* pixel en defaut seuil 2 */
                      if (val >=0 )
                      	configSetSeuilAffPixelMax((int32)val);
                      else
                        erreur = TRUE;
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
               else if (dv1_scmp (ptr_deb, "REB=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      /* pixel en defaut seuil 2 */
                      switch(val)
                      {
                      case 0:
                      configSetRebouclage(false);
                      break;
                      case 1:
                      configSetRebouclage(true);
                      break;
                      default:
                      erreur=TRUE;
                      break;
                      }
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
                              else if (dv1_scmp (ptr_deb, "SEC=", 0))
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
                      /* la periode de syncro */
                      pip_cf_tst_sc.diff_sect = (UINT) val;
                    }
                  else
                    erreur = TRUE;

                  if (ptr_deb < ptr_fin)
                    erreur = TRUE;
                }
                else if (dv1_scmp (ptr_deb, "TSE=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      pip_cf_tst_sc.temp.seuil=val;
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
              else if (dv1_scmp (ptr_deb, "TAC=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      pip_cf_tst_sc.temp.actif=val;
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
              else if (dv1_scmp (ptr_deb, "RBP=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
  						pip_cf_tst_sc.ts_bp_test = val;
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
              else if (dv1_scmp (ptr_deb, "LBP=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
  						pip_cf_tst_sc.tc_bp_led = val;
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
               else if (dv1_scmp (ptr_deb, "TCH=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
  						configSetTemperatureChauffage( val);
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
                   else if (dv1_scmp (ptr_deb, "IPP=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
  						configSetNumPortIp(val);
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
                else if (dv1_scmp (ptr_deb, "AUT=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                    	/* La valeur 0 pour supprimer le test. */
                    	if((val>=120)||val==0)
                    	{
                    	configSetPeriodeTestPixel(val);
                    	}
                    	else
                    	{
	                    erreur = TRUE;
                    	}
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
                }
              else if (dv1_scmp (ptr_deb, "TTE=", 0))
                {
                  *ptr_fin = 0;
                  ptr_deb += 4;
                  erreur = FALSE;
                  if (dv1_str_atoi (&ptr_deb, &val))
                    {
                      pip_cf_tst_sc.temp.tempo=val;
                    }
                  else
				  {
                    erreur = TRUE;
				  }
                  if (ptr_deb < ptr_fin)
				  {
                    erreur = TRUE;
				  }
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
/ NOM DE LA FONCTION : lcr_tst_sc                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/12/1996                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_init                                             /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande INIT                          /
/-------------------------------------------------------------------------DOC*/
static int _lcr_tst_cfsc (INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess,STRING commande_ac)
{
  INT  j;
  int config=FALSE;
  int lgCommande=strlen(commande_ac);

      /* taille indique le nombre de caracteres du parametre */
	if (lg_mess > lgCommande) {
		if (identLireAutorisationAvecNiveau(las, 0)) {
			config = TRUE;
			lcr_cmd_tst_sc(&buffer[lgCommande], lg_mess - lgCommande);
		} else {
			x01_cptr.erreur = CPTRD_PROTOCOLE;
		}
	}

  if (x01_cptr.erreur == CPTRD_OK)
    {
	int bloc=0;
      /* on retourne les parametres */
      j=0;
      j +=
        sprintf (&buffer[j], "%s DP1=%ld DP2=%ld ",
        		commande_ac,
                    configGetSeuilAffPixelMin(),
                    configGetSeuilAffPixelMax());

      j +=
        sprintf (&buffer[j], " SLJ=%ld SLS=%ld",
        	configGetSeuilJour(),
        	configGetSeuilSurb());

      j +=
        sprintf (&buffer[j], " SCE=%ld SCT=%ld SCA=%ld",
        	configGetPollingEs(),
        	configGetPollingSonde(),
        	configGetPollingAff());
      j +=
        sprintf (&buffer[j], " PAB=%ld PAA=%ld",
        	configGetPaddingBefore(),
        	configGetPaddingAfter());

      tedi_send_bloc (las, mode, buffer, j, bloc, FALSE, pt_mess);
	  bloc++;
      j = 0;
      j += sprintf (&buffer[j], " SPC=%d",vct_tempo_spc);
      j += sprintf (&buffer[j], " SEC=%d", pip_cf_tst_sc.diff_sect);
      j += sprintf (&buffer[j], " IPP=%ld", configGetNumPortIp());
      j += sprintf (&buffer[j], " REB=%ld", (configIsRebouclage()?1:0));

//      j += dv1_sprintf (&buffer[j], " MOD=%d", pip_cf_tst_sc.mode);

      /* periode de controle des cellules */
      j += sprintf (&buffer[j], " TAC=%d", pip_cf_tst_sc.temp.actif);
      j += sprintf (&buffer[j], " TSE=%d", pip_cf_tst_sc.temp.seuil);
      j += sprintf (&buffer[j], " TCH=%ld", configGetTemperatureChauffage());
      j += sprintf (&buffer[j], " TTE=%d", pip_cf_tst_sc.temp.tempo);
      j += sprintf (&buffer[j], " AUT=%d", configGetPeriodeTestPixel());
	  if(-1!=pip_cf_tst_sc.ts_bp_test)
	  {
      j += sprintf (&buffer[j], " RBP=%d", pip_cf_tst_sc.ts_bp_test);
	  }
	  else
	  {
      	j += sprintf (&buffer[j], " RBP=?");
	  }

      tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
    }
  else
    {
	  config=FALSE;
      tedi_erreur (las, mode);
    }
   return config;
}
int lcr_tst_sc_pmv (INT las, INT mode, INT lg_mess, STRING buffer,
                     INT position, T_usr_ztf * pt_mess)
{
  INT  j;
  int config=FALSE;

      /* taille indique le nombre de caracteres du parametre */
   if (lg_mess > 7)
   {
      if (identLireAutorisationAvecNiveau (las, 0))
      {
		  config=TRUE;
          lcr_cmd_tst_sc (&buffer[7], lg_mess - 7);
      }
      else
      {
         x01_cptr.erreur = CPTRD_PROTOCOLE;
      }
    }

  if (x01_cptr.erreur == CPTRD_OK)
    {
	int bloc=0;
      /* on retourne les parametres */
      j=0;
      j +=
        sprintf (&buffer[j], "TST SCC DP1=%ld DP2=%ld ",
                    configGetSeuilAffPixelMin(),
                    configGetSeuilAffPixelMax());

      j +=
        sprintf (&buffer[j], " SLJ=%ld SLS=%ld",
        	configGetSeuilJour(),
        	configGetSeuilSurb());

      j +=
        sprintf (&buffer[j], " SCE=%ld SCT=%ld SCA=%ld",
        	configGetPollingEs(),
        	configGetPollingSonde(),
        	configGetPollingAff());
      j +=
        sprintf (&buffer[j], " PAB=%ld PAA=%ld",
        	configGetPaddingBefore(),
        	configGetPaddingAfter());

      tedi_send_bloc (las, mode, buffer, j, bloc, FALSE, pt_mess);
	  bloc++;
      j = 0;

      j += sprintf (&buffer[j], " SEC=%d", pip_cf_tst_sc.diff_sect);
      j += sprintf (&buffer[j], " IPP=%ld", configGetNumPortIp());
      j += sprintf (&buffer[j], " REB=%ld", (configIsRebouclage()?1:0));

//      j += dv1_sprintf (&buffer[j], " MOD=%d", pip_cf_tst_sc.mode);

      /* periode de controle des cellules */
      j += sprintf (&buffer[j], " TAC=%d", pip_cf_tst_sc.temp.actif);
      j += sprintf (&buffer[j], " TSE=%d", pip_cf_tst_sc.temp.seuil);
      j += sprintf (&buffer[j], " TCH=%ld", configGetTemperatureChauffage());
      j += sprintf (&buffer[j], " TTE=%d", pip_cf_tst_sc.temp.tempo);
      j += sprintf (&buffer[j], " AUT=%d", configGetPeriodeTestPixel());
	  if(-1!=pip_cf_tst_sc.ts_bp_test)
	  {
      j += sprintf (&buffer[j], " RBP=%d", pip_cf_tst_sc.ts_bp_test);
	  }
	  else
	  {
      	j += sprintf (&buffer[j], " RBP=?");
	  }

      tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
    }
  else
    {
	  config=FALSE;
      tedi_erreur (las, mode);
    }
   return config;
}

int lcr_tst_scc (INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess)
{
	return _lcr_tst_cfsc(las,mode,lg_mess,buffer,position,pt_mess,LCR_PCP_TST_SCC);
}
int lcr_tst_cfsc (INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess)
{
	return _lcr_tst_cfsc(las,mode,lg_mess,buffer,position,pt_mess,LCR_PCP_TST_CFSC);
}
