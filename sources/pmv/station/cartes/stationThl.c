
#include <stdio.h>
#include <string.h>
#include "kcommon.h"
#include "klog.h"
#include "affichage/zone.h"
#include "affichage/page.h"
#include "affichage/caractere.h"
#include "affichage/affichage.h"
#include "infos/lumInfos.h"
#include "lumAdresse.h"
#include "lumMsg.h"
#include "luminance/lumMsgCreer.h"
#include "affichage/afficheur.h"
#include "es/es.h"
#include "thl/thl.h"
#include "event.h"
#include "cartes.h"
#include "cartesAfficheur.h"
#include "configuration.h"

static Thl *thlTable[THL_NB_MAX];


void thlsInit(void)
{
	int indice;
	/* TODO : Initialisation des cartes temperature et luminosite.
	 * Par la meme occasion on peut ajuster leur configuration. */
	for(indice=0;indice<THL_NB_MAX;indice++)
	{
		thlTable[indice]=thlNew(configGetAdresseThl(indice));
		thlSetMode(thlTable[indice],configGetModeThl(indice));
		thlSetPadding(thlTable[indice],configGetPaddingBefore(),configGetPaddingAfter());
	}
}

void thlsTerm(void)
{
	int indice;
	/* Liberation des ressources allouees. */
	for(indice=0;indice<THL_NB_MAX;indice++)
	{
		printf("thlsTerm: terminsaison thl %d\n",indice);
		thlDelete(thlTable[indice]);
	}
}

void thlsReset()
{
	int indice;
	/* Liberation des ressources allouees. */
	for(indice=0;indice<THL_NB_MAX;indice++)
	{
	Thl *thl_pt=thlTable[indice];
		thl_pt->conf_b=false;
	}
}

bool thlsIsDefautNumero(int numero)
{
bool retour=false;
	if((0<=numero)&&(configGetNbThls()>numero))
	{
	Thl *thl_pt=thlTable[numero];
		retour =thlIsDefautCom(thl_pt);
	}
	return retour;
}


bool thlsIsOk()
{
bool retour=true;
	int indice;
	/* Liberation des ressources allouees. */
	for(indice=0;indice<configGetNbThls();indice++)
	{
	Thl *thl_pt=thlTable[indice];
		retour &=((!thlIsDefautCom(thl_pt))&&thl_pt->conf_b&&thl_pt->vers_b);
	}
	return retour;

}
Event * thlsTraiterMessage(LumMsg *msg_pt)
{
	/* Récupérer l'adresse de la carte. */
	int adresse=lumAdresseGetLumAdresse(lumMsgGetSource(msg_pt));
	Thl *thl_pt=thlsChercherParAdresse(adresse);
	Event *evt_pt=NULL;
	if(NULL!=thl_pt)
	{
	int tempoReprog;
		thlTraiterMessage(thl_pt,msg_pt);
		tempoReprog=thlTesterReprogrammation(thl_pt);
		if(0!=tempoReprog)
		{
			Ktimestamp ts_t;
			Event *evt_pt;
			ktsGetTime(&ts_t);
			kAddTime(&ts_t,0,tempoReprog);
			evt_pt=eventNew(&ts_t,EVENT_THL,thl_pt,NULL);
		}
		{
		int32 luminosite=thlGetLuminosite(thl_pt);
			if(-1!=luminosite)
			{
				if(configSetValCour(luminosite))
				{
					configSetLuminositeCourante(configCalculerLuminositeCourante());
				}
			}
		}
	}
	else
	{
		klogPut(NULL,LOG_INFO,"thlsTraiterMessage: probleme d'adresse %d",adresse);
	}
	return evt_pt;

}
int32 thlsGetTemperature()
{
int32 luminosite_dw=-1000;
int indice;
		for(indice=0;indice<configGetNbThls();indice++)
		{
		Thl *thl_pt=thlTable[indice];

			if(0xFFFFFFFF!=thlGetTemp(thl_pt))
			{
				luminosite_dw=MAX(thlGetTemp(thl_pt),luminosite_dw);
			}
		}
	return luminosite_dw;
}

int32 thlsGetLuminosite()
{
int luminosite_dw=-1;
int indice;
		for(indice=0;indice<configGetNbThls();indice++)
		{
		Thl *thl_pt=thlTable[indice];
			luminosite_dw=MAX(thlGetLuminosite(thl_pt),luminosite_dw);
		}
	return luminosite_dw;
}

void thlsGetInfos(char *infos,int longueur)
{
	if(thlsIsOk())
	{
		infos[0]=0;
	}
	else
	{
	int indice=0;
	int lgCour=0;
		snprintf(&infos[lgCour],longueur-lgCour," T=");
		lgCour=strlen(infos);
		for(indice=0;(indice<configGetNbThls())&&(lgCour<longueur);indice++)
		{
		Thl *thl_pt=thlTable[indice];
			if(thlIsDefautCom(thl_pt)||(!thl_pt->conf_b)||(!thl_pt->vers_b))
			{
				snprintf(&infos[lgCour],longueur-lgCour,"%02d,",indice);
				lgCour=strlen(infos);
			}
		}
		infos[strlen(infos)-1]=0;
	}
}


void thlsTraiterDefautCom(LumMsg *msg_pt)
{
	int adresse=lumAdresseGetLumAdresse(lumMsgGetDest(msg_pt));
	Thl *aff_pt=thlsChercherParAdresse(adresse);
	if(NULL!=aff_pt)
	{
		thlTraiterDefautCom(aff_pt,msg_pt);
	}
	else
	{
		klogPut(NULL,LOG_INFO,"thlsTraiterMessage: probleme d'adresse %d",adresse);
	}
}

Thl *thlsChercherParIndex(int index)
{
	Thl *thl_pt=NULL;
	if((index>=0)&&(index<configGetNbThls()))
	{
		thl_pt=thlTable[index];
	}
	return thl_pt;
}

Thl *thlsChercherParAdresse(int adresse)
{
	Thl *thl_pt=NULL;
	int indice;
	for(indice=0;indice<THL_NB_MAX;indice++)
	{
		if(NULL!=thlTable[indice])
		{
			if(thlGetNumber(thlTable[indice])==adresse)
			{
				thl_pt=thlTable[indice];
				break;
			}
		}
	}
	return thl_pt;
}
