/*
 * lcr_tst_lum
 * ===================
 * Objet	: Traitement de la commande TST LUM qui permet en theorie
 * 			d'activer ou de desactiver des traces sur un module.
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_tst_lum.c,v $
 * Revision 1.2  2018/06/04 08:38:43  xg
 * Passage en version V122
 *
 * Revision 1.4  2009/04/16 13:39:17  xag
 * Suppression de warning de compilation.
 *
 * Revision 1.3  2008/10/03 16:21:47  xag
 * *** empty log message ***
 *
 * Revision 1.2  2008/09/29 07:58:53  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.1  2008/09/12 14:59:05  xag
 * Archivage de printemps
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

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "portage.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "rdtComm.h"
#include "pip_def.h"
#include "lcr_util.h"
#include "ted_prot.h"
#include "lcr_tst.h"
#include "ecouteur.h"
#include "event.h"
#include "configuration.h"
#include "sequenceur.h"
#include "sequenceurTraiter.h"
#include "configurationPriv.h"
#include "configurationSram.h"
#include "cpu432/sram.h"
#include "lumAdresse.h"
#include "lumMsg.h"
#include "luminance/lumMsgCreer.h"
#include "lumTraiter.h"
#include "infos/lumInfos.h"
#include "affichage/zone.h"
#include "affichage/page.h"
#include "affichage/caractere.h"
#include "affichage/affichage.h"
#include "affichage/afficheur.h"
#include "thl/thl.h"
#include "es/es.h"
#include  "station/cartesAfficheur.h"

#include "configuration.h"
#include "lcr_tst_def.h"



/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define TST_LUM_CMD			"TST LUM"
#define TST_LUM_AFF			"A="
#define TST_LUM_THL			"T="
#define TST_LUM_ES 			"E="

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * lcr_tst_lum
 * =============
 * Analyse de la commande "TST LUM".
 * Entree :
 * - numero de la liaison sur lequel
 *   se fait le transfert.
 * - mode : mode de transmission,
 * - nombre de caracteres dans le
 *   buffer,
 * - position dans le buffer,
 * - pointeur vers un message de
 *   transmission
 * --------------------------------	*/

int lcr_tst_lum(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int nbCar = 0;
	int bloc = 0;
	int flag_err = 0;

	ptCour += strlen(TST_LUM_CMD);
	lgCour -= strlen(TST_LUM_CMD);
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if ((0 !=lgCour) && (0 == flag_err))
	{
	int val =-1;
		/* On regarde de quel parametre il s'agit... */
		if (strncmp(ptCour, TST_LUM_AFF, strlen(TST_LUM_AFF)) == 0)
		{
			/* Recuperation du numero d'afficheur... */
			ptCour += strlen(TST_LUM_AFF);
			lgCour -= strlen(TST_LUM_AFF);
			ptCour = tst_lit_entier(ptCour, lgCour, &lgCour,&val);
			if(-1==val)
			{
				flag_err=3;
			}
			else
			{
				/* On regarde que l'on soit bien a la fin de la commande.... */
				ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
				if(lgCour==0)
				{
					if((0<=val)&&(val<configGetNbAfficheurs()))
					{
					Afficheur *aff_pt=afficheursChercherParIndex(val);
					int longueur=256;
					uint8 chaine[256];
					afficheurGetVersion(aff_pt,(char *)chaine,longueur);
					/* Composer le buffer de reponse.... */
					tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess,
													"TST LUM %s%02d %s", TST_LUM_AFF, val,chaine);

					}
					else
					{
						/* Il y a une erreur de syntaxe... */
						flag_err=2;
					}
				}
				else
				{
					flag_err=2;
				}
			}
		}
		else
		if (strncmp(ptCour, TST_LUM_THL, strlen(TST_LUM_THL)) == 0)
		{
			ptCour += strlen(TST_LUM_THL);
			lgCour -= strlen(TST_LUM_THL);
			ptCour = tst_lit_entier(ptCour, lgCour, &lgCour,&val);
			if(-1==val)
			{
				flag_err=3;
			}
			else
			{
				/* On regarde que l'on soit bien a la fin de la commande.... */
				ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
				if(lgCour==0)
				{
					if((0<=val)&&(val<configGetNbThls()))
					{
						Thl *thl_pt=thlsChercherParIndex(val);
						int longueur=256;
						uint8 chaine[256];
						thlGetVersion(thl_pt,(char *)chaine,longueur);
						/* Composer le buffer de reponse.... */
						tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess,
														"TST LUM %s%02d %s", TST_LUM_THL, val,chaine);
					}
					else
					{
						/* Il y a une erreur de syntaxe... */
						flag_err=2;
					}
				}
				else
				{
					flag_err=2;
				}
			}

		}
		else
		if (strncmp(ptCour, TST_LUM_ES, strlen(TST_LUM_ES)) == 0)
		{
			ptCour += strlen(TST_LUM_ES);
			lgCour -= strlen(TST_LUM_ES);
			ptCour = tst_lit_entier(ptCour, lgCour, &lgCour,&val);
			if(-1==val)
			{
				flag_err=3;
			}
			else
			{
				/* On regarde que l'on soit bien a la fin de la commande.... */
				ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
				if(lgCour==0)
				{
					if((0<=val)&&(val<configGetNbEss()))
					{
						Es *es_pt=essChercherParIndex(val);
						int longueur=256;
						uint8 chaine[256];
						esGetVersion(es_pt,(char *)chaine,longueur);
						/* Composer le buffer de reponse.... */
						tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess,
														"TST LUM %s%02d %s", TST_LUM_ES, val,chaine);

					}
					else
					{
						/* Il y a une erreur de syntaxe... */
						flag_err=2;
					}
				}
				else
				{
					flag_err=2;
				}
			}


		}
		else
		{
			/* Erreur sur le nom du parametre.... */
			flag_err=2;
		}
		if(0!=flag_err)
		{
		tedi_ctrl_ext(las, mode, flag_err);
		}
	}
	else
	{
		flag_err=2;
		tedi_ctrl_ext(las, mode, flag_err);
	}
	/* ------------------------------------
	 * FIN DE tst_simu_dial
	 * ------------------------------------ */
	return 0;
}
