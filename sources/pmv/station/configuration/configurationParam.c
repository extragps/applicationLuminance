#include <stdio.h>
#include <string.h>
#include <kcommon.h>
#include <klog.h>
#include <tokenizerDirif.h>
#include "configuration.h"
#include "configurationParam.h"
#include "cpu432/sram.h"


/* ************************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ************************************	*/

static char * _configParamGetSimple(ConfigParam *nomParam_pt);
static char* _configParamGetListe(TokenizerDirif *tkz_t);
static void _configParamRead(ConfigParam *param_pt,char *tableau,int longueur);

/* ************************************
 * DECLARATION DES VARIABLES LOCALES
 * ************************************	*/

static Klist *configListeInfos_pt=NULL;

/* ************************************
 * FONCTIONS DU MODULE
 * ************************************ */

/* ------ POUR LES PARAMETRES ------ */

static ConfigParam *configParamNew(char *info_pc,ConfigType type,bool (*adresse)(),void (*getter)(),char *comment_pt)
{
	ConfigParam *param_pt=(ConfigParam *)kmmAlloc(NULL,sizeof(ConfigParam));
	if(NULL!=param_pt)
	{
		param_pt->nomParam_pc=(char *)kmmAlloc(NULL,strlen(info_pc)+1);
		param_pt->comment_pc=NULL;
		if(NULL==param_pt->nomParam_pc)
		{
			kmmFree(NULL,param_pt);
			param_pt=NULL;
		}
		else
		{
			if(NULL!=comment_pt)
			{
				param_pt->comment_pc=(char *)kmmAlloc(NULL,strlen(comment_pt)+1);
				if(NULL!=param_pt->comment_pc)
				{
					strcpy(param_pt->comment_pc,comment_pt);
				}

			}
			strcpy(param_pt->nomParam_pc,info_pc);
			param_pt->type_e=type;
			param_pt->setter=adresse;
			param_pt->getter=getter;
		}

	}
	return param_pt;
}

static void _configParamDelete(void *param_p)
{
	if(NULL!=param_p)
	{
		ConfigParam *param_pt=(ConfigParam *)param_p;
		if(NULL!=param_pt->nomParam_pc)
		{
			kmmFree(NULL,param_pt->nomParam_pc);
		}
		if(NULL!=param_pt->comment_pc)
		{
			kmmFree(NULL,param_pt->comment_pc);
		}
		kmmFree(NULL,param_pt);
	}
}

static ConfigParam *_configParamGet(char *nomParam_pt)
{
	ConfigParam *tmp_pt=NULL;
	for(tmp_pt=klistFirst(configListeInfos_pt);tmp_pt!=NULL;tmp_pt=klistNext(configListeInfos_pt))
	{
		if(strcmp(nomParam_pt,tmp_pt->nomParam_pc)==0)
		{
			break;
		}
	}
	return tmp_pt;
}

static char * _configParamGetSimple(ConfigParam *nomParam_pt)
{
	char *retour_pt=NULL;
	switch(nomParam_pt->type_e)
	{
	case CONFIG_BOOL:
		retour_pt=kmmAlloc(NULL,5);
		if((((bool (*)(void))(nomParam_pt->getter))())==true)
		{
			strcpy(retour_pt,"vrai");
		}
		else
		{
			strcpy(retour_pt,"faux");
		}
		break;
	case CONFIG_LONG:
	{
		char codage[100];
		int32 valeur_dw=((int32 (*)(void))(nomParam_pt->getter))();
		sprintf(codage,"%ld",valeur_dw);
		retour_pt=kmmAlloc(NULL,strlen(codage)+1);
		strcpy(retour_pt,codage);
	}
	break;
	case CONFIG_CHAINE:
	{
		uint8 info[256];
		if(((bool (*)(uint8 *))(nomParam_pt->getter))(info))
		{
			retour_pt=kmmAlloc(NULL,strlen(info)+1);
			strcpy(retour_pt,info);
		}
	}
	break;
	default:
		break;
	}
	return retour_pt;
}

static bool _configParamSet(ConfigParam *nomParam_pt,char *value_ac)
{
	bool retour_b=false;
	switch(nomParam_pt->type_e)
	{
	case CONFIG_BOOL:
		if(
				(strcmp(value_ac,"true")==0)||
				(strcmp(value_ac,"1")==0)||
				(strcmp(value_ac,"vrai")==0)||
				(strcmp(value_ac,"oui")==0))
		{
			retour_b=true;
			if((((bool (*)(void))(nomParam_pt->getter))())!=true)
			{
				nomParam_pt->setter(true);
			}
		}
		else
			if(
					(strcmp(value_ac,"false")==0)||
					(strcmp(value_ac,"0")==0)||
					(strcmp(value_ac,"non")==0)||
					(strcmp(value_ac,"faux")==0))
			{
				retour_b=true;
				if((((bool (*)(void))(nomParam_pt->getter))())!=false)
				{
					nomParam_pt->setter(false);
				}
			}
		break;
	case CONFIG_LONG:
	{
		int32 value_dw;
		retour_b=str2int(&value_dw,value_ac);
		if(true==retour_b)
		{

			if(((int32 (*)(void))(nomParam_pt->getter))()!=value_dw)
			{
				retour_b=nomParam_pt->setter(value_dw);
			}
		}
	}
	break;
	case CONFIG_CHAINE:
	{
		uint8 info[256];
		retour_b=((bool (*)(uint8 *))(nomParam_pt->setter))(value_ac);
	}
	break;
	default:
		break;
	}
	return retour_b;
}

/* configInit
 * ==========
 * Fonction d'initialisation de la configuration.
 *  */
void configParamInit()
{
	/* Creation de la liste des parametres */
	configListeInfos_pt=klistNew(0,0,NULL,_configParamDelete);
//	klistAdd(configListeInfos_pt,configParamNew("adresseBsc",CONFIG_CHAINE,configSetAdresseBsc,(void (*)())configGetAdresseBsc,"Adresse bsc sur trois caracteres alphanumeriques"));
//	klistAdd(configListeInfos_pt,configParamNew("numCaisson",CONFIG_LONG,configSetNumCaisson,(void (*)())configGetNumCaisson,"Numero du caisson"));
//	klistAdd(configListeInfos_pt,configParamNew("demarrage",CONFIG_BOOL,configSetDemarrage,(void (*)())configGetDemarrage,"Allumage du panneau au demarrage"));
//	klistAdd(configListeInfos_pt,configParamNew("numMsg1",CONFIG_LONG,configSetMsg1Neutre,(void (*)())configGetMsg1Neutre,"Numero preprogramme du premier message neutre"));
//	klistAdd(configListeInfos_pt,configParamNew("numMsg2",CONFIG_LONG,configSetMsg2Neutre,(void (*)())configGetMsg2Neutre,"Numero preprogramme du second message neutre"));
	klistAdd(configListeInfos_pt,configParamNew("valJour",CONFIG_LONG,configSetValJour,(void (*)())configGetValJour,"Consigne de luminosite jour"));
	klistAdd(configListeInfos_pt,configParamNew("valNuit",CONFIG_LONG,configSetValNuit,(void (*)())configGetValNuit,"Consigne de luminosite nuit"));
	klistAdd(configListeInfos_pt,configParamNew("valSurb",CONFIG_LONG,configSetValSurb,(void (*)())configGetValSurb,"Consigne de luminosite surbrillance"));
	klistAdd(configListeInfos_pt,configParamNew("seuilJour",CONFIG_LONG,configSetSeuilJour,(void (*)())configGetSeuilJour,"Seuil de luminosite jour en lux"));

	klistAdd(configListeInfos_pt,configParamNew("seuilSurb",CONFIG_LONG,configSetSeuilSurb,(void (*)())configGetSeuilSurb,"Seuil de luminosite surbrillance en lux"));
	klistAdd(configListeInfos_pt,configParamNew("temporisationTest",CONFIG_LONG,configSetTempoTest,(void (*)())configGetTempoTest,"Temporisation avant demarrage sequence de test"));
	klistAdd(configListeInfos_pt,configParamNew("periodeTest",CONFIG_LONG,configSetPeriodeTest,(void (*)())configGetPeriodeTest,"Temps en secondes entre deux messages de test"));

	klistAdd(configListeInfos_pt,configParamNew("periodeAnimation",CONFIG_LONG,configSetPeriodeAnimation,(void (*)())configGetPeriodeAnimation,"Periode en seconde pour synchro d'animation"));

	klistAdd(configListeInfos_pt,configParamNew("periodePollingAfficheur",CONFIG_LONG,configSetPollingAff,(void (*)())configGetPollingAff,"Periode en seconde pour le polling des cartesu afficheur"));
	klistAdd(configListeInfos_pt,configParamNew("periodePollingThl",CONFIG_LONG,configSetPollingSonde,(void (*)())configGetPollingSonde,"Periode en seconde pour le polling des cartes thl"));
	klistAdd(configListeInfos_pt,configParamNew("periodePollingEs",CONFIG_LONG,configSetPollingEs,(void (*)())configGetPollingEs,"Periode en seconde pour le polling de la carte es"));

	klistAdd(configListeInfos_pt,configParamNew("periodeTestPixel",CONFIG_LONG,configSetPeriodeTestPixel,(void (*)())configGetPeriodeTestPixel,"Temps en secondes entre deux tests pixels (entretien actif)"));

	klistAdd(configListeInfos_pt,configParamNew("nombreThls",CONFIG_LONG,configSetNbThls,(void (*)())configGetNbThls,"Nombre de cartes thl"));
	klistAdd(configListeInfos_pt,configParamNew("adresseThl",CONFIG_LISTE_LONG,configSetAdresseThl,(void (*)())configGetAdresseThl,"Adresse des cartes thl"));
	klistAdd(configListeInfos_pt,configParamNew("modeThl",CONFIG_LISTE_LONG,configSetModeThl,(void (*)())configGetModeThl,"Mode de fonctionnement de la carte thl, depend de son equipement"));

	klistAdd(configListeInfos_pt,configParamNew("adresseEs",CONFIG_LONG,configSetAdresseEs,(void (*)())configGetAdresseEs,"Adresse de la carte es"));







	klistAdd(configListeInfos_pt,configParamNew("pixelHsMineurAfficheur",CONFIG_LONG,configSetSeuilAffPixelMin,(void (*)())configGetSeuilAffPixelMin,"Seuil default pixel mineur afficheur"));
	klistAdd(configListeInfos_pt,configParamNew("pixelHsMajeurAfficheur",CONFIG_LONG,configSetSeuilAffPixelMax,(void (*)())configGetSeuilAffPixelMax,"Seuil default pixel majeur afficheur"));
	klistAdd(configListeInfos_pt,configParamNew("pixelHsMineurModule",CONFIG_LONG,configSetSeuilModulePixelMin,(void (*)())configGetSeuilModulePixelMin,"Seuil default pixel mineur module"));
	klistAdd(configListeInfos_pt,configParamNew("pixelHsMajeurModule",CONFIG_LONG,configSetSeuilModulePixelMax,(void (*)())configGetSeuilModulePixelMax,"Seuil default pixel majeur module"));
	klistAdd(configListeInfos_pt,configParamNew("nombreAfficheurs",CONFIG_LONG,configSetNbAfficheurs,(void (*)())configGetNbAfficheurs,"Nombre d'afficheurs"));
	klistAdd(configListeInfos_pt,configParamNew("adresseAfficheur",CONFIG_LISTE_LONG,configSetAdresseAfficheur,(void (*)())configGetAdresseAfficheur,"Adresse des cartes afficheurs"));
	klistAdd(configListeInfos_pt,configParamNew("rebouclage",CONFIG_BOOL,configSetRebouclage,(void (*)())configIsRebouclage,"Configuration du mode rebouclage"));
//	klistAdd(configListeInfos_pt,configParamNew("vitesseLas1",CONFIG_LONG,configSetVitesseLas1,(void (*)())configGetVitesseLas1,"Vitesse de transmission sur le port serie LAS1"));
//	klistAdd(configListeInfos_pt,configParamNew("vitesseLas2",CONFIG_LONG,configSetVitesseLas2,(void (*)())configGetVitesseLas2,"Vitesse de transmission sur le port serie LAS2"));
//	klistAdd(configListeInfos_pt,configParamNew("numPortBsc",CONFIG_LONG,configSetNumPortIpBsc,(void (*)())configGetNumPortIpBsc,"Numero de port IP pour la connexion BSC par ethernet"));
//	klistAdd(configListeInfos_pt,configParamNew("defCommPc",CONFIG_LONG,configSetScrutationPc,(void (*)())configGetScrutationPc,"Delai de scrutation par l'UG"));
//	klistAdd(configListeInfos_pt,configParamNew("defCommMaint",CONFIG_LONG,configSetScrutationMaint,(void (*)())configGetScrutationMaint,"Delai de scrutation sur le port de maintenance"));
//	klistAdd(configListeInfos_pt,configParamNew("defCommIp",CONFIG_LONG,configSetScrutationIp,(void (*)())configGetScrutationIp,"Delai de scrutation sur le port BSC Ip"));
	klistAdd(configListeInfos_pt,configParamNew("seuilChauffage",CONFIG_LONG,configSetTemperatureChauffage,(void (*)())configGetTemperatureChauffage,"Temperature de declenchement du chauffage"));


}
void configParamTerm()
{
	if(NULL!=configListeInfos_pt)
	{
	klistDelete(configListeInfos_pt);
	configListeInfos_pt=NULL;
	}
}


/**--------------------------------------------------------------------------------------------------------------------
 * Traitement d'une ligne du fichier d'initialisation des ports
 * format ligne : module_pc.id_pc.param_pc=value_pc
 * @param module_pc : &char[20]
 * @param id_pc : &char[20]
 * @param param_pc : &char[20]
 * @param value_pc : &char[80]
 */

bool configParseLigne(char *param_pc,char *value_pc, const char * ligne_pc )
{
	bool success_b = false;
	TokenizerDirif tkz_t;

	tokenizeExtended( &tkz_t, ligne_pc, CONFIG_SEPARATORS );
	if( tkzDirifGetSize( &tkz_t ) == 2 && tkzDirifGetFirst( &tkz_t, param_pc, 80) && tkzDirifGetNext( &tkz_t, value_pc, 80 ) ) {
		trim(param_pc);
		trim(value_pc);
		success_b=true;
	}

	return success_b;
}

bool configParseLigneParam(char *param_pc,const char * ligne_pc )
{
	bool success_b = false;
	TokenizerDirif tkz_t;

	tokenizeExtended( &tkz_t, ligne_pc, CONFIG_SEPARATORS );
	if( (tkzDirifGetSize( &tkz_t ) == 1 ) && tkzDirifGetFirst( &tkz_t, param_pc, 80)  ) {
		trim(param_pc);
		success_b=true;
	}

	return success_b;
}
static char*	_configParamGetListe(TokenizerDirif *tkz_t)
{
	char paramName_ac[80];
	char numero_ac[80];
	int32 numero_dw;
	char *retour_pt=NULL;
	/* Recuperation du parametre et du numero... */
	tkzDirifGetFirst(tkz_t,paramName_ac,80);
	tkzDirifGetNext(tkz_t,numero_ac,80);
	/* Parsing du numero */
	if(str2int(&numero_dw,numero_ac))
	{
		/* Recherche du parametre de configuration */
		ConfigParam *param_pt=_configParamGet(paramName_ac);
		/* Positionnement du parametre de configuration */
		if(NULL!=param_pt)
		{
			switch(param_pt->type_e)
			{
			case CONFIG_LISTE_LONG:
			{
				char codage[100];
				int32 valeur_dw=((int32 (*)(int32))(param_pt->getter))(numero_dw);
				sprintf(codage,"%ld",valeur_dw);
				retour_pt=kmmAlloc(NULL,strlen(codage)+1);
				strcpy(retour_pt,codage);
			}
			break;
			default:
				break;
			}
		}
	}
	return retour_pt;
}

static bool	_configSetListe(TokenizerDirif *tkz_t,char *value_ac)
{
	char paramName_ac[80];
	char numero_ac[80];
	int32 numero_dw;
	bool retour_b=false;
	/* Recuperation du parametre et du numero... */
	tkzDirifGetFirst(tkz_t,paramName_ac,80);
	tkzDirifGetNext(tkz_t,numero_ac,80);
	/* Parsing du numero */
	if(str2int(&numero_dw,numero_ac))
	{
		/* Recherche du parametre de configuration */
		ConfigParam *param_pt=_configParamGet(paramName_ac);
		/* Positionnement du parametre de configuration */
		if(NULL!=param_pt)
		{
			switch(param_pt->type_e)
			{
			case CONFIG_LISTE_LONG:
			{
				int32 value_dw;
				if(str2int(&value_dw,value_ac))
				{

					if(((int32)((int32 (*)(int32))(param_pt->getter))(numero_dw))!=value_dw)
					{
						retour_b=param_pt->setter(numero_dw,value_dw);
					}
					else
					{
						retour_b=true;
					}
				}
			}

			break;
			default:
				break;
			}
		}
	}
	return retour_b;
}

char *configParamGet(char *param_ac)
{
	char *retour_pt=NULL;
	TokenizerDirif tkz_t;

	tokenizeDirif( &tkz_t, param_ac, CONFIG_SEP_PARAM );
	switch(tkzDirifGetSize(&tkz_t))
	{
	case 1:
	{
		ConfigParam *param_pt=_configParamGet(param_ac);
		if(NULL!=param_pt)
		{
			retour_pt=_configParamGetSimple(param_pt);
			if(NULL==retour_pt)
			{
				klogPut(NULL,LOG_INFO,"Parametre non trouve  %s",param_ac);
			}
		}
		else
		{
			klogPut(NULL,LOG_INFO,"Parametre non trouve : %s ",param_ac);
		}
	}
	break;
	case 2:
		{
		retour_pt=_configParamGetListe(&tkz_t);
		}
		break;
	default:
		klogPut(NULL,LOG_INFO,"Parametre non trouve : %s",param_ac);
		break;
	}
	return retour_pt;
}
bool configParamTraiter(char *param_ac,char *value_ac)
{
	bool retour_b=false	;
	TokenizerDirif tkz_t;

	tokenizeDirif( &tkz_t, param_ac, CONFIG_SEP_PARAM );
	switch(tkzDirifGetSize(&tkz_t))
	{
	case 1:
	{
		ConfigParam *param_pt=_configParamGet(param_ac);
		if(NULL!=param_pt)
		{
			retour_b=_configParamSet(param_pt,value_ac);
			if(false==retour_b)
			{
				klogPut(NULL,LOG_INFO,"Parametre trouve : %s valeur incorrecte : %s",param_ac,value_ac);
			}
		}
		else
		{
			klogPut(NULL,LOG_INFO,"Parametre non trouve : %s param value : %s",param_ac,value_ac);
		}
	}
	break;
	case 2:
		retour_b=_configSetListe(&tkz_t,value_ac);
		break;
	default:
		klogPut(NULL,LOG_INFO,"Syntaxe parametre incorrecte : %s param value : %s",param_ac,value_ac);
		break;
	}
	return retour_b;
}

void configParamFirst(char *tableau,int longueur)
{
	KlistIterator it_t;
	ConfigParam *param_pt=klistFirst(configListeInfos_pt);
	if(NULL!=param_pt)
	{
		_configParamRead(param_pt,tableau,longueur);
	}
}
void configParamNext(char *tableau,int longueur)
{
	ConfigParam *param_pt=klistNext(configListeInfos_pt);
	if(NULL==param_pt)
	{
		param_pt=klistFirst(configListeInfos_pt);
	}
	if(NULL!=param_pt)
	{
		_configParamRead(param_pt,tableau,longueur);
	}
}

static void _configParamRead(ConfigParam *param_pt,char *tableau,int longueur)
{
	switch(param_pt->type_e)
	{
	case CONFIG_BOOL:
	{
		bool (*info)()=(bool (*)())param_pt->getter;
		snprintf(tableau,longueur,"%s: %s",param_pt->nomParam_pc,
				(((info)()==false)?"0":"1"));
	}
	break;
	case CONFIG_LONG:
	{
		int32 (*info)()=(int32 (*)())param_pt->getter;
		snprintf(tableau,longueur,"%s: %ld",param_pt->nomParam_pc,info());
	}
	break;
	case CONFIG_CHAINE:
	{
		uint8 chaine[256];
		bool (*info)(uint8 *)=(bool (*)(uint8 *))param_pt->getter;
		info(chaine);
		snprintf(tableau,longueur,"%s: %s",param_pt->nomParam_pc,chaine);
	}
	break;
	case CONFIG_LISTE_LONG:
	{
		int32 index_dw=0;
		int32 valeur_dw;
		int32 lgCour=0;
		int32 (*info)()=(int32 (*)())param_pt->getter;
		snprintf(tableau,longueur,"%s[]: ",param_pt->nomParam_pc);
		lgCour=strlen(tableau);
		while((-1!=(valeur_dw=info(index_dw)))&&(lgCour<longueur))
		{
			snprintf(&tableau[lgCour],longueur-lgCour,"%ld,",valeur_dw);
			lgCour=strlen(tableau);
			index_dw++;
		}
		tableau[strlen(tableau)-1]=0;
	}
	break;
	default:
		snprintf(tableau,longueur,"%s: ?????????????????",param_pt->nomParam_pc);
		break;

	}
}
