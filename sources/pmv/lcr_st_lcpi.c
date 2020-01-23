
/*
 * lcr_st_lcpi.c
 * ===================
 *
 *  Created on: 3 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_st_lcpi.c,v $
 * Revision 1.2  2018/06/04 08:38:43  xg
 * Passage en version V122
 *
 * Revision 1.2  2008/07/04 17:05:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include <string.h>
#include "standard.h"
#include "string.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_str.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "tac_conf.h"
#include "pip_str.h"
#include "pip_var.h"
#include "lcr_cfet.h"
#include "lcr_util.h"
#include "lcr_st_lcpi.h"
#include "identLib.h"


#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* --------------------------------
 * lcr_st_lcpi_init
 * ================
 * Initialisation des donnees utilisateurs
 * de ST LCPI.
 * --------------------------------	*/

void lcr_st_lcpi_init()
{
	vct_usr_lcpi.infos[0]=0;
}

/* --------------------------------
 * lcr_st_lcpi_modif
 * =================
 * Modification du commentaire
 * utilisateur.
 * --------------------------------	*/

int lcr_st_lcpi_modif(char *usr1,char *usr2)
{
int ok=TRUE;
	printDebug("lcr_st_lcpi_modif\n");
	if(strcmp(usr2,"Z")==0)
	{
			/* Supprimer dans le commentaire la premiere chaine qui
			 * commence par usr1 */
			/* Probleme si la chaine n'est pas trouvee...*/
	char *ptCour=vct_usr_lcpi.infos;
		printDebug("lcr_st_lcpi_modif, effacement\n");
		while(*ptCour!=0)
		{
			if(strncmp(ptCour,usr1,strlen(usr1))==0)
			{
			char *ptAux=strstr(ptCour,"\n");
				if(NULL==ptAux)
				{
					*ptCour=0;
				}
				else
				{
					ptAux++;
					while(0!=*ptAux)
					{
						*ptCour++=*ptAux++;
					}
					*ptCour=0;
					break;
				}
			}
			else
			{
				ptCour=strstr(ptCour,"\n");
				if(NULL==ptCour)
				{
					ok=FALSE;
					break;
				}
				else
				{
					ptCour+=strlen("\n");
				}
			}
		}
	}
	else
	{
		if(0!=usr2[0])
		{
			/* Rechercher la premiere chaine qui debute par usr2 et
			 * la remplacer par usr1 */
			/* Probleme si la chaine n'est pas trouvee...*/
		char *ptCour=vct_usr_lcpi.infos;
			printDebug("lcr_st_lcpi_modif, remplacement\n");
			while(*ptCour!=0)
			{
				if(strncmp(ptCour,usr2,strlen(usr2))==0)
				{
				char *ptAux=strstr(ptCour,"\n");
				int lgUsr2=(ptAux-ptCour);
					if(lgUsr2>strlen(usr1))
					{
						/* Remplacement de la chaine et decalage de la fin */
						strncpy(ptCour,usr1,strlen(usr1));
						ptCour+=strlen(usr1);
						while(0!=*ptAux)
						{
							*ptCour++=*ptAux++;
						}
						*ptCour=0;
					}
					else
					{
						if((strlen(vct_usr_lcpi.infos)+strlen(usr1)-lgUsr2)<
										MAX_INFO_LCPI)
						{
						int lgFin=strlen(ptAux);
						int diff=strlen(usr1)-lgUsr2;
							ptAux[lgFin+diff]=0;
							while(lgFin--)
							{
								ptAux[lgFin+diff]=ptAux[lgFin];
							}
							strncpy(ptCour,usr1,strlen(usr1));
							break;
						}
						else
						{
							ok=FALSE;
						}
						/* Controle de la longueur, decalage de la fin et
						 * remplacement de la chaine. */
						break;
					}
				}
				else
				{
					ptCour=strstr(ptCour,"\n");
					if(NULL==ptCour)
					{
						ok=FALSE;
						break;
					}
					else
					{
						ptCour+=strlen("\n");
					}
				}
			}
		}
		else
		{
			/* Ajouter la chaine e la fin du buffer!! */
		int lgCour=strlen(vct_usr_lcpi.infos);
		int lgNouv=strlen(usr1);
			printDebug("lcr_st_lcpi_modif, ajour pos %d lg %d\n",lgCour,lgNouv);
		if((lgCour+lgNouv+2)<MAX_INFO_LCPI)
		{
			if(0!=strlen(vct_usr_lcpi.infos))
			{
				strcat(vct_usr_lcpi.infos,"\n");
			}
			strcat(vct_usr_lcpi.infos,usr1);
		}
		else
		{
				ok=FALSE;
			}
		}
	}

	return ok;
}


/* --------------------------------
 * lcr_st_lcpi
 * ========
 * --------------------------------	*/

INT lcr_st_lcpi (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
              T_usr_ztf * pt_mess)
{
  INT config = FALSE;
  INT usrSeul=FALSE;	/* Par defaut, toute la configuration est listee. */
  T_vct_usr_lcpi sv_vct_usr_lcpi;

  /* on se place apres le nom de la commande */
  char *ptCour = (char *) &buffer[position + strlen ("ST LCPI")];
  int lgCour = lg_mess - position - strlen ("ST LCPI");
  int flag_err = 0;
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);

  sv_vct_usr_lcpi=vct_usr_lcpi;
  if (0 < lgCour)
    {
          if (dv1_scmp_and_skip (ptCour,lgCour,"USR=Z",strlen("USR=Z"),
								  (STRING *)&ptCour,&lgCour))
        {

			lcr_st_lcpi_init();
			config=TRUE;
			usrSeul=TRUE;
		}
		  else if (dv1_scmp_and_skip (ptCour,lgCour,"USR=",strlen("USR="),
								  (STRING *)&ptCour,&lgCour))
        {
          /* XG : TODO : traitement de la demande de configuration des
           * equivalences topologiques. Pour l'instant, il y a erreur*/
      /* on teste si l'operateur est habilite */
      if (identLireAutorisationAvecNiveau (las, 0))
            {
			char usr1[MAX_USR_LCPI+1];
			char usr2[MAX_USR_LCPI+1];
			int  ok;
				/* Traitement d'une ligne de commentaires.	*/
				/* Recuperation de la nouvelle ligne et eventuellement de
				 * la ligne e supprimer. 	*/
				ptCour=tst_lit_param_usr(ptCour,lgCour,&lgCour,
								usr1,usr2,&ok);
				if(TRUE==ok)
				{
					printf("Tout va bien ajout %s suppr #%d#\n",usr1,(int)usr2);
					if(FALSE==lcr_st_lcpi_modif(usr1,usr2))
					{
						flag_err=3;
					}
					config=TRUE;
					usrSeul=TRUE;
				}
				else
				{
					flag_err=2;
				}


        }
      else
        {
          x01_cptr.erreur = CPTRD_PROTOCOLE;
        }
            }
          else
          if (dv1_scmp_and_skip (ptCour,lgCour,"USR",0,(STRING *)&ptCour,&lgCour))
		  {
				printDebug("lcr_st_lcpi demande des infos utilistaur %d\n",lgCour);
				usrSeul=TRUE;
		  }
		  else
            {
              flag_err = 1;            /* Erreur parametre non reconnu. */
            }
          ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
          flag_err = (0 !=  lgCour) || flag_err;
    }

  /* si on a pas detecte d'erreur */
  if ((x01_cptr.erreur == CPTRD_OK) && (0 == flag_err))
    {
    int j = 0;
	int bloc=0;
		/* Lister ici la configuration */
		if(FALSE==usrSeul)
		{
		int numCaisson;
            j = dv1_scpy (buffer, "ST LCPI SIT=EXT CTL=CLE TBL=AM\n\r", 0);
			for(numCaisson=0;numCaisson<pip_nb_caisson;numCaisson++)
			{
			T_pip_cf_caisson *caisson=&pip_cf_caisson[numCaisson];

				j+=sprintf(&buffer[j],"%d.%d=%s\n\r",caisson->id_module,
								caisson->caisson,caisson->info);
				printDebug("Les infos %s\n",buffer);
          		tedi_test_bloc (las, mode, buffer, &j, &bloc, pt_mess);
			}
			/* Trace des informations module. */
		}
        j += dv1_scpy (&buffer[j], "ST LCPI USR\n\r", 0);
		printDebug("Les infos %s\n",buffer);
		/* Impression de toutes les lignes d'info utilisateur. */
		{
		char *ptCour=vct_usr_lcpi.infos;
			while(0!=*ptCour)
			{
			char *ptAux=strstr(ptCour,"\n");
			printDebug("Valeur de ptAux %#0x pour ptCour %#0x\n",
							(unsigned int)ptAux,(unsigned int)ptCour);
				if(NULL==ptAux)
				{
					while(0!=*ptCour)
					{
						printDebug("Ajout de %c\n",*ptCour);
						buffer[j++]=*ptCour++;
					}
					buffer[j++]='\n';
					buffer[j++]='\r';
				}
				else
				{
					while(ptCour!=ptAux)
					{
						printDebug("Ajout de %c\n",*ptCour);
						buffer[j++]=*ptCour++;
					}
					buffer[j++]='\n';
					buffer[j++]='\r';
					ptCour++;
				}
          		tedi_test_bloc (las, mode, buffer, &j, &bloc, pt_mess);
			}
		}
		/* Listing ici des donnees utilisateur. */
	    tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
    }
  else
    {
		if(TRUE==config)
		{
			vct_usr_lcpi=sv_vct_usr_lcpi;
      		config = FALSE;
		}
      tedi_ctrl_ext (las, mode, ((x01_cptr.erreur == CPTRD_PROTOCOLE) ? 5 : 2));
    }
  return config;
}

/* *********************************
 * FIN DE lcr_st_lcpi.c
 * ********************************* */
