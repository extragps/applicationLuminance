
#include <stdio.h>
#include <string.h>
#include "kcommon.h"
#include "klog.h"
#include "pip_def.h"

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
#include "configuration.h"
#include "sequenceur.h"
#include "cartes.h"
#include "mgpLib.h"

Afficheur *afficheurTable[AFFICHEUR_NB_MAX];
uint16 afficheursDefauts=0;

/* **********************************************
 * FONCTIONS COMMUNES
 * Les fonctions qui suivent s'appliquent a tous
 * les afficheurs gérés par la station.
 * TODO : Faire en sorte de ne plus utiliser de
 * variable statique pour la gestion des afficheurs,
 * il faut penser à la suite et notament la possibité
 * d'utiliser plusieurs canaux de communication.
 * ***********************************************	*/

uint16 afficheursGetSortieDefauts(void)
{
	return afficheursDefauts;
}

void afficheursSetSortieDefauts(uint16 defauts)
{
	if(afficheursDefauts!=defauts)
	{
	int indice=0;
		afficheursDefauts=defauts;
		for(indice=0;indice<6;indice++)
		{
			if((defauts&(1<<indice))!=0)
			{
				mgpPositionnerSortie(indice,true);
			}
			else
			{
				mgpPositionnerSortie(indice,false);
			}
		}

	}
}
uint16 afficheursGetMasqueDefauts()
{
uint16 retour_uw=0;
int indice;
	for(indice=0;indice<configGetNbAfficheurs();indice++)
	{
	Afficheur *aff_pt;
	uint8 nbHs_uc;
		aff_pt=afficheurTable[indice];
		nbHs_uc=afficheurGetDefautPixel(aff_pt);
		if(nbHs_uc>configGetSeuilAffPixelMin())
		{
		int32 sortie_dw=configGetSortieAfficheur(indice);
			if(-1!=sortie_dw)
			{
				retour_uw|=(1<<sortie_dw);
			}
		}
	}
	return retour_uw;
}
void afficheursInit(void)
{
	int indice;
	/* Initialisation de la table des afficheurs. */
	for(indice=0;indice<configGetNbAfficheurs();indice++)
	{
	Afficheur *aff_pt=afficheurNew(configGetAdresseAfficheur(indice));
		/* Configuration du polling et de la tempo d'animation. */
		afficheurSetPolling(aff_pt,configGetPollingAff()*2);
		afficheurSetGroupe(aff_pt,configGetCaissonAfficheur(indice));
		afficheurSetTempoAnimation(aff_pt,configGetPeriodeAnimation());
		afficheurSetPadding(aff_pt,configGetPaddingBefore(),configGetPaddingAfter());
		afficheurTable[indice]=aff_pt;
	}
	for(;indice<AFFICHEUR_NB_MAX;indice++)
	{
		afficheurTable[indice]=NULL;
	}

}
void afficheursInitUnitaire(int numAfficheur,int adresse,int caisson)
{
	if(numAfficheur<AFFICHEUR_NB_MAX)
	{
		if(NULL==afficheurTable[numAfficheur])
		{
	Afficheur *aff_pt=afficheurNew(adresse);
		/* Configuration du polling et de la tempo d'animation. */
		afficheurSetPolling(aff_pt,configGetPollingAff());
		afficheurSetGroupe(aff_pt,caisson);
		afficheurSetTempoAnimation(aff_pt,configGetPeriodeAnimation());
		afficheurSetPadding(aff_pt,configGetPaddingBefore(),configGetPaddingAfter());
		afficheurTable[numAfficheur]=aff_pt;
		}
		else
		{
		Afficheur *aff_pt=afficheurTable[numAfficheur];
			afficheurSetNumber(aff_pt,adresse);
			afficheurSetGroupe(aff_pt,caisson);
		}
	}
}

void afficheursTerm(void)
{
	int indice;
	/* Initialisation de la table des afficheurs. */
	for(indice=0;indice<AFFICHEUR_NB_MAX;indice++)
	{
		if(NULL!=afficheurTable[indice])
		{
		afficheurDelete(afficheurTable[indice]);
		afficheurTable[indice]=NULL;
		}
	}
}

void afficheursReset()
{
	int indice;
	/* Liberation des ressources allouees. */
	for(indice=0;indice<configGetNbAfficheurs();indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		aff_pt->conf_b=false;
		afficheurResetDefautCom(aff_pt);
		afficheurResetDefautPixel(aff_pt);
		afficheurResetStatus(aff_pt);
	}
}

bool afficheursIsOkNumAff(int *num)
{
bool retour=true;
	int indice;
	/* Liberation des ressources allouees. */
	for(indice=0;indice<configGetNbAfficheurs();indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		if(false==((aff_pt->conf_b&&aff_pt->vers_b&&!afficheurIsDefautCom(aff_pt))
			&&(!afficheurIsDefautCarte(aff_pt)&&(!afficheurIsDefautTsd(aff_pt)))))
		{
			retour=false;
			*num=indice;
			break;
		}
	}
	if(retour)
	{
	retour&=(STATION_DEF_PIXEL_MAJEUR!=afficheursGetDefautPixel()?true:false);
	*num=255;
	}
	return retour;

}

bool afficheursIsOk()
{
bool retour=true;
	int indice;
	/* Liberation des ressources allouees. */
	for(indice=0;indice<configGetNbAfficheurs();indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		if(false==((aff_pt->conf_b&&aff_pt->vers_b&&!afficheurIsDefautCom(aff_pt))
			&&((!afficheurIsDefautCarte(aff_pt))&&(!afficheurIsDefautTsd(aff_pt)))))
		{
			retour=false;
			break;
		}
	}
	retour&=(STATION_DEF_PIXEL_MAJEUR!=afficheursGetDefautPixel()?true:false);
	return retour;

}

bool afficheursIsToutOk()
{
bool retour=true;
	int indice;
	/* Liberation des ressources allouees. */
	for(indice=0;indice<configGetNbAfficheurs();indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		retour &=afficheurIsToutOk(aff_pt);
	}
	return retour;

}

bool afficheursIsDefautCom()
{
bool retour=false;
int indice;
	for(indice=0;(indice<configGetNbAfficheurs())&&(false==retour);indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		retour|=afficheurIsDefautCom(aff_pt);
	}
	return retour;
}

bool afficheursIsDefautAnim()
{
bool retour=false;
int indice;
	for(indice=0;(indice<configGetNbAfficheurs())&&(false==retour);indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		retour|=afficheurIsDefautAnim(aff_pt);
	}
	return retour;
}

bool afficheursIsDefautActivation()
{
bool retour=false;
int indice;
	for(indice=0;(indice<configGetNbAfficheurs())&&(false==retour);indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		retour|=afficheurIsDefautActivation(aff_pt);
	}
	return retour;
}

bool afficheursIsDefautSinOut()
{
bool retour=false;
int indice;
	for(indice=0;(indice<configGetNbAfficheurs())&&(false==retour);indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		retour|=afficheurIsDefautCarte(aff_pt);
	}
	return retour;
}

bool afficheursIsDefautTsd()
{
bool retour=false;
int indice;
	for(indice=0;(indice<configGetNbAfficheurs())&&(false==retour);indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		retour|=afficheurIsDefautTsd(aff_pt);
	}
	return retour;
}

Afficheur *afficheursChercherParIndex(int index)
{
Afficheur *aff_pt=NULL;
	if((0<=index)&&(index<configGetNbAfficheurs()))
	{
		aff_pt=afficheurTable[index];
	}
	return aff_pt;
}
Afficheur *afficheursChercherParAdresse(int adresse)
{
	Afficheur *aff_pt=NULL;
	int indice=0;
	for(indice=0;indice<configGetNbAfficheurs();indice++)
	{
		if(adresse==afficheurGetNumber(afficheurTable[indice]))
		{
			aff_pt=afficheurTable[indice];
			break;
		}
	}
	return aff_pt;
}

static bool _afficheursControlerChargementCaisson(int numCaisson)
{
bool retour_b=true;
int32 premCar=configGetPremAfficheurCaisson(numCaisson);
int32 nbCarCaisson=configGetNbAfficheursCaisson(numCaisson);
int32 indice=0;
	for(indice=0;(indice<nbCarCaisson)&&(true==retour_b);indice++)
	{
	Afficheur *aff_pt=afficheurTable[premCar+indice];
		retour_b&=afficheurIsLoaded(aff_pt)||
			((afficheurIsDefautCom(aff_pt)||afficheurIsDefautCarte(aff_pt)
			||afficheurIsDefautTsd(aff_pt))&&configIsRebouclage());
	}

	return retour_b;
}

bool afficheursControlerChargement()
{
bool retour_b=true;
int nbCaisson=configGetNbAfficheurs();
int indice=0;
	for(indice=0;(indice<nbCaisson)&&(true==retour_b);indice++)
	{
	Afficheur *aff_pt=afficheurTable[indice];
		retour_b&=afficheurIsLoaded(aff_pt)||
			((afficheurIsDefautCom(aff_pt)||afficheurIsDefautCarte(aff_pt)
			||afficheurIsDefautTsd(aff_pt))&&configIsRebouclage());
	}

	return retour_b;
}

bool afficheursSetAffichage(int adresse,Affichage *affichage_pt)
{
bool retour=false;
	Afficheur *aff_pt=afficheursChercherParIndex(adresse);
	if(NULL!=aff_pt)
	{
		retour=true;
		afficheurSetAffichage(aff_pt,affichage_pt);
	}
	else
	{
		klogPut(NULL,LOG_INFO,"afficheursSetAffichage: probleme d'adresse %d",adresse);
	}
	return retour;
}
void afficheursTraiterDefautCom(LumMsg *msg_pt)
{
	int adresse=lumAdresseGetLumAdresse(lumMsgGetDest(msg_pt));
	Afficheur *aff_pt=afficheursChercherParAdresse(adresse);
	if(NULL!=aff_pt)
	{
		afficheurTraiterDefautCom(aff_pt,msg_pt);
	}
	else
	{
		klogPut(NULL,LOG_INFO,"afficheursTraiterMessage: probleme d'adresse %d",adresse);
	}
}
Event *afficheursTraiterMessage(LumMsg *msg_pt,LumMsg *ori_pt)
{
	/* Récupérer l'adresse de la carte. */
		Event *evt_pt=NULL;
	int adresse=lumAdresseGetLumAdresse(lumMsgGetSource(msg_pt));
	Afficheur *aff_pt=afficheursChercherParAdresse(adresse);
	if(NULL!=aff_pt)
	{
	uint16 tempoReprog;
		afficheurTraiterMessage(aff_pt,msg_pt,ori_pt);
		tempoReprog=afficheurTesterReprogrammation(aff_pt);
		if(0!=tempoReprog)
		{
		Ktimestamp ts_t;
			klogPut(NULL,LOG_INFO,"afficheursTraiterMessage: reprogrammation evenement %d",adresse);
			ktsGetTime(&ts_t);
			kAddTime(&ts_t,0,tempoReprog);
			evt_pt=eventNew(&ts_t,EVENT_AFFICHEUR,aff_pt,NULL);
		}
	}
	else
	{
		klogPut(NULL,LOG_INFO,"afficheursTraiterMessage: probleme d'adresse %d",adresse);
	}
	return evt_pt;
}

void afficheursGetInfos(char *infos,int longueur)
{
	if(afficheursIsToutOk())
	{
		infos[0]=0;
	}
	else
	{
	int indice=0;
	int lgCour=0;
		snprintf(&infos[lgCour],longueur-lgCour," A=");
		lgCour=strlen(infos);
		for(indice=0;(indice<configGetNbAfficheurs())&&(lgCour<longueur);indice++)
		{
		Afficheur *aff_pt=afficheurTable[indice];
			if(!afficheurIsToutOk(aff_pt))
			{
				snprintf(&infos[lgCour],longueur-lgCour,"%02d,",indice);
				lgCour=strlen(infos);
			}
		}
		infos[strlen(infos)-1]=0;
	}
}

void afficheursGetEtatPixels(int numAff,uint8* etatPixels)
{
Afficheur *aff_pt=afficheursChercherParIndex(numAff);
	if(NULL!=aff_pt)
	{
		afficheurGetEtatPixels(aff_pt,etatPixels);
	}
}

uint8 afficheursGetDefautPixelNumero(int indice)
{
uint8 retour_uc=STATION_DEF_PIXEL_NONE;
	if((0<=indice)&&(configGetNbAfficheurs()>indice))
	{
	Afficheur *aff_pt=NULL;
	uint8 nbHs_uc;
		aff_pt=afficheurTable[indice];
		nbHs_uc=afficheurGetDefautPixel(aff_pt);
		if(nbHs_uc>configGetSeuilAffPixelMax())
		{
			retour_uc=STATION_DEF_PIXEL_MAJEUR;
		}
		else
		{
			if(nbHs_uc>configGetSeuilAffPixelMin())
			{
				retour_uc=STATION_DEF_PIXEL_MINEUR;
			}
		}
	}
	return retour_uc;
}

uint8 afficheursGetDefautPixel()
{
	uint8 retour_uc=STATION_DEF_PIXEL_NONE;
	Afficheur *aff_pt=NULL;
	int indice=0;
	int32 nbHsTotal_dw=0;
	for(indice=0;(indice<configGetNbAfficheurs())&&(retour_uc!=STATION_DEF_PIXEL_MAJEUR);indice++)
	{
	uint8 nbHs_uc;
		aff_pt=afficheurTable[indice];
		nbHs_uc=afficheurGetDefautPixel(aff_pt);
		if(nbHs_uc>configGetSeuilAffPixelMax())
		{
			retour_uc=STATION_DEF_PIXEL_MAJEUR;
		}
		else
		{
			if(nbHs_uc>configGetSeuilAffPixelMin())
			{
				retour_uc=STATION_DEF_PIXEL_MINEUR;
			}
			nbHsTotal_dw+=nbHs_uc;
		}
	}
	return retour_uc;
}

