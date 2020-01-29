/*
 * lcr_tst_def
 * ===================
 * Objet	: Traitement de la commande TST DEF qui permet en theorie
 * 			d'activer ou de desactiver des traces sur un module.
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_tst_def.c,v $
 * Revision 1.2  2018/06/04 08:38:44  xg
 * Passage en version V122
 *
 * Revision 1.5  2008/11/05 09:00:34  xag
 * Ajout du defaut sabordage...
 *
 * Revision 1.4  2008/10/03 16:21:47  xag
 * *** empty log message ***
 *
 * Revision 1.3  2008/09/29 07:58:54  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.2  2008/09/22 07:53:41  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.1  2008/09/12 14:59:04  xag
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
#include "configuration.h"
#include "configurationPriv.h"
#include "configurationSram.h"
#include "cpu432/sram.h"
#include "cartes.h"
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



/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define TST_DEF_CMD			"TST DEF"
#define TST_DEF_AFF			"A="
#define TST_DEF_THL			"T="
#define TST_DEF_ES 			"E="

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
 * lcr_tst_def
 * =============
 * Analyse de la commande "TST DEF".
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

int lcr_tst_def(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int nbCar = 0;
	int bloc = 0;
	int flag_err = 0;

	ptCour += strlen(TST_DEF_CMD);
	lgCour -= strlen(TST_DEF_CMD);
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if ((0 !=lgCour) && (0 == flag_err))
	{
	int val =-1;
		/* On regarde de quel parametre il s'agit... */
		if (strncmp(ptCour, TST_DEF_AFF, strlen(TST_DEF_AFF)) == 0)
		{
			/* Recuperation du numero d'afficheur... */
			ptCour += strlen(TST_DEF_AFF);
			lgCour -= strlen(TST_DEF_AFF);
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
					char detail[256]="";
						if(0!=afficheurGetEtat(aff_pt))
						{
						int etat=afficheurGetEtat(aff_pt);
							snprintf(detail,256,"(%s,%s,%s,%s,%s,%s) ",
								(((AFFICHEUR_DEFAUT_PIXEL&etat)!=0)?"PIXEL":""),
								(((AFFICHEUR_DEFAUT_CARTE&etat)!=0)?"SINOUT":""),
								(((AFFICHEUR_DEFAUT_TSD&etat)!=0)?"TSD":""),
								(((AFFICHEUR_DEFAUT_TEST&etat)!=0)?"TEST":""),
								(((AFFICHEUR_DEFAUT_LOAD&etat)!=0)?"LOAD":""),
								(((AFFICHEUR_DEFAUT_ANIM&etat)!=0)?"ANIM":""));
						}
						/* Composer le buffer de reponse.... */
						tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess,
								"TST DEF %s%02d ad:%02ld etat:%02X %scom:%s(%d/%d) pixHs:%d",
								TST_DEF_AFF,
								val,configGetAdresseAfficheur(val),
								(unsigned int)afficheurGetEtat(aff_pt),
								detail,
								(afficheurIsDefautCom(aff_pt)?"NK":
									(afficheurIsConf(aff_pt)?"OK":"??")),
								afficheurGetNbDefCom(aff_pt),
								afficheurGetNbDefComTot(aff_pt),
								(unsigned int)afficheurGetDefautPixel(aff_pt));

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
		if (strncmp(ptCour, TST_DEF_THL, strlen(TST_DEF_THL)) == 0)
		{
			ptCour += strlen(TST_DEF_THL);
			lgCour -= strlen(TST_DEF_THL);
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
						int32 temp_dw=thlGetTemp(thl_pt);
						int32 lum_dw=thlGetLuminosite(thl_pt);
						char infoTemp[20];
						char infoLum[20];
						if(0xFFFFFFFF==temp_dw)
						{
							infoTemp[0]=0;
						}
						else
						{
							snprintf(infoTemp,20," temp=%ld.%ld",temp_dw/10,kmodulo(temp_dw,10));
						}
						if(-1==lum_dw)
						{
							infoLum[0]=0;
						}
						else
						{
							snprintf(infoLum,20," lum=%ld",lum_dw);
						}
						tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess,
								"TST DEF %s%02d ad:%02ld etat:%02X mode:%02X com:%s(%d/%d)%s%s",
								TST_DEF_THL,
								val,configGetAdresseThl(val),
								thlGetEtat(thl_pt),
								thlGetMode(thl_pt),
								(thlIsDefautCom(thl_pt)?"NK":(thlIsConf(thl_pt)?"OK":"??")),
								thlGetNbDefCom(thl_pt),
								thlGetNbDefComTot(thl_pt),
								infoTemp,infoLum);
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
		if (strncmp(ptCour, TST_DEF_ES, strlen(TST_DEF_ES)) == 0)
		{
			ptCour += strlen(TST_DEF_ES);
			lgCour -= strlen(TST_DEF_ES);
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
						tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess, "TST DEF %s%02d ad:%02ld etat:%02X %s%scom:%s(%d/%d)",
								TST_DEF_ES,
								0,configGetAdresseEs(),
								esGetEtat(es_pt),
								(essDefautAlim()?"Pb alim ":
									((esGetDefautAlim(es_pt)!=0)?"Alim HS ":"")),
								(esIsSabordage()?" Sabordage ":""),
								(esIsDefautCom(es_pt)?"NK":
									((esIsConf(es_pt)?"OK":"??"))),
								esGetNbDefCom(es_pt),
								esGetNbDefComTot(es_pt));

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
		/* Rapport d'etat global sur le panneau (a l'image de ce que l'on pouvait faire sur le PMV BSC */
		char infoEs[256];
		char infoThl[256];
		char infoAff[256];

			essGetInfos(infoEs,256);
			thlsGetInfos(infoThl,256);
			afficheursGetInfos(infoAff,256);
		tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess, "TST DEF%s%s%s",
				infoEs,infoThl,infoAff);
	}
	/* ------------------------------------
	 * FIN DE tst_simu_dial
	 * ------------------------------------ */
	return 0;
}
