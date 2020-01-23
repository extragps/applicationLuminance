#include <stdio.h>
#include <string.h>
#include <kcommon.h>
#include <klog.h>
#include <tokenizerDirif.h>
#include "pip_def.h"
#include "configuration.h"
#include "configurationPriv.h"
#include "configurationSram.h"
#include "cpu432/sram.h"

#include "pip_str.h"
#include "pip_var.h"
#include "eriLib.h"

/* ************************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ************************************	*/

static void configInitDynamique(void);


/* ************************************
 * DECLARATION DES VARIABLES LOCALES
 * ************************************	*/

static ConfigStatique configStatique={0,0,2,50000,127,192,255,2,10,5,20,36};
static ConfigDynamique configDynamique={0,false,false,false,0,false};
static Kmutex *configMutex_pt=NULL;

/* ************************************
 * FONCTIONS DU MODULE
 * ************************************ */


void configInit()
{

	configMutex_pt=kmutexNew();
	/* Initialisation de la structure de gestion de la SRAM */
	/* TODO : Etudier ce que l'on fait avec le message courant */
//	configSetMessCour(NULL);
	/* Creation de la liste des parametres, la fonction est maintenant deportee. */
	configParamInit();

	configInitStatique();
	configInitDynamique();

	/* Chargement au besoin d'une configuration. */

	/* TODO : report de la fonctionnalite au niveau de la configuration globale... */
//	if(false==ioIsResetCfg())
//	{
//		configLoad();
//	}
//	else
//	{
//		configSave();
//	}
//	/* Au demarrage on force l'arret du rebouclage. */
//	configSetRebouclage(false);

}
void configTerm()
{
	configParamTerm();
	configSramTerm();
	kmutexDelete(configMutex_pt);
}

void configStatiqueSet(void *config_p)
{
	configStatique=*((ConfigStatique *)config_p);
}

void configStatiqueGet(void *config_p)
{
	*((ConfigStatique *)config_p)=configStatique;
}
/* _configSramInit
 * ===============
 * Dans le cas ou le checksum de la sram est defectueux, on
 * realise une reinitialisatio de la sram.
 * */

void configInitStatique(void)
{
ConfigStatique *conf_pt=&configStatique;
	int indice;

	/* On force a zero la configuration. */
	memset(conf_pt,0,sizeof(ConfigStatique));

	conf_pt->seuilJour_dw=100;
	conf_pt->seuilSurb_dw=5000;
	conf_pt->tempoAnim_dw=60;
	conf_pt->tempoTest_dw=10;
	conf_pt->periodeTest_dw=5;
	conf_pt->periodeTestPixel_dw=86400; // Un test par jour....
	conf_pt->periodeScrutAff_dw=40;
	conf_pt->periodeScrutThl_dw=30;
	conf_pt->periodeScrutEs_dw=10;
	conf_pt->paddingBefore_dw=1;
	conf_pt->paddingAfter_dw=1;
	conf_pt->seuilPixelMaxAff=2;
	conf_pt->seuilPixelMinAff=0;
	conf_pt->adresseEs=1;
	conf_pt->valNuit=50;
	conf_pt->valJour=100;
	conf_pt->valSurb=187;
	conf_pt->numPort_dw=2001;
	conf_pt->nbThls=2;
	conf_pt->seuilChauffage_dw=0;
	for(indice=0;indice<THL_NB_MAX;indice++)
	{
		conf_pt->adresseThl[indice]=indice+1;
	}
	conf_pt->modeThl[0]=0xC3;
	conf_pt->modeThl[1]=0xA8;
	conf_pt->nbAfficheurs=36;
	for(indice=0;indice<conf_pt->nbAfficheurs;indice++)
	{
		conf_pt->adresseAfficheur[indice]=indice+1;
		conf_pt->sortieAfficheur[indice]=-1;
		conf_pt->caissonAfficheur[indice]=0;
	}
	for(;indice<AFFICHEUR_NB_MAX;indice++)
	{
		conf_pt->adresseAfficheur[indice]=-1;
	}
}
static void configInitDynamique(void)
{
ConfigDynamique *dyn_pt=&configDynamique;
int indice;

	dyn_pt->numAffichage_dw=-1;
	dyn_pt->valCour_dw=0;
	dyn_pt->on_b=false;
	dyn_pt->lumCour_uc=0;
	dyn_pt->lumSeuil_uc=0;
	for(indice=0;indice<NB_CAISSON;indice++)
	{
	dyn_pt->lumChanged_b[indice]=false;
	dyn_pt->lumAuto_b[indice]=true;
	dyn_pt->lumVal_uc[indice]=0;
	dyn_pt->enCours_b[indice]=false;
	}
	for(indice=0;indice<AFFICHEUR_NB_MAX;indice++)
	{
		dyn_pt->etatAfficheur_dw[indice]=0;
	}
	dyn_pt->ok_b=false;
	dyn_pt->reprise_b=true;
	dyn_pt->chargementEnCours_b=false;
	dyn_pt->init_b=true;
	dyn_pt->tempoScrut_dw=0;
	dyn_pt->rebouclage_b=false;
}

int32 configGetAdresseAfficheur(int32 numero)
{
	int32 retour=-1;
	if((numero<0)||(numero>=AFFICHEUR_NB_MAX))
	{
		klogPut(NULL,LOG_INFO,"configGetAdresseAfficheur : numero incorrect %ld",numero);
	}
	else
	{
		retour=configStatique.adresseAfficheur[numero];
	}
	return retour;
}

bool configSetAdresseAfficheur(int32 numero,int32 valeur)
{
	bool retour_b=true;
	if((numero<0)||(numero>=AFFICHEUR_NB_MAX))
	{
		klogPut(NULL,LOG_INFO,"configSetAdresseAfficheur : numero incorrect %ld",numero);
		retour_b=false;
	}
	else
	{
		configStatique.adresseAfficheur[numero]=valeur;

	}
	return retour_b;

}

int32 configGetSortieAfficheur(int32 numero)
{
	int32 retour=-1;
	if((numero<0)||(numero>=AFFICHEUR_NB_MAX))
	{
		klogPut(NULL,LOG_INFO,"configGetSortieAfficheur : numero incorrect %ld",numero);
	}
	else
	{
		retour=configStatique.sortieAfficheur[numero];
	}
	return retour;
}

bool configSetSortieAfficheur(int32 numero,int32 valeur)
{
	bool retour_b=true;
	if((numero<0)||(numero>=AFFICHEUR_NB_MAX))
	{
		klogPut(NULL,LOG_INFO,"configSetSortieAfficheur : numero incorrect %ld",numero);
		retour_b=false;
	}
	else
	{
		configStatique.sortieAfficheur[numero]=valeur;

	}
	return retour_b;

}
int32 configGetCaissonAfficheur(int32 numero)
{
	int32 retour=-1;
	if((numero<0)||(numero>=AFFICHEUR_NB_MAX))
	{
		klogPut(NULL,LOG_INFO,"configGetCaissonAfficheur : numero incorrect %ld",numero);
	}
	else
	{
		retour=configStatique.caissonAfficheur[numero];
	}
	return retour;
}

bool configSetCaissonAfficheur(int32 numero,int32 valeur)
{
	bool retour_b=true;
	if((numero<0)||(numero>=AFFICHEUR_NB_MAX))
	{
		klogPut(NULL,LOG_INFO,"configSetCaissonAfficheur : numero incorrect %ld",numero);
		retour_b=false;
	}
	else
	{
		configStatique.caissonAfficheur[numero]=valeur;

	}
	return retour_b;

}

bool configSetNbAfficheurs(int32 nombre)
{
	bool retour_b=true;
	if((nombre<0)||(nombre>AFFICHEUR_NB_MAX))
	{
		klogPut(NULL,LOG_INFO,"configSetNbAfficheurs : nombre incorrect %ld",nombre);
		retour_b=false;
	}
	else
	{
		configStatique.nbAfficheurs=nombre;
		if((nombre+1<AFFICHEUR_NB_MAX))
		{
			configStatique.adresseAfficheur[nombre+1]=-1;
		}
	}
	return retour_b;
}
//
//bool configLoad()
//{
//	bool error_b = false;
//	FILE * f;
//	char line_ac[256];
//
//
//	f = fopen( CONFIG_FILE, "r" );
//	if( null != f ) {
//		while( !feof(f) && null != fgets( line_ac, 255, f) ) {
//			line_ac[255] = 0;
//			trim(line_ac);
//			if( line_ac[0] != '#' && line_ac[0] != 0 ) {
//				char param_ac[128];
//				char value_ac[128];
//				error_b = !configParseLigne(param_ac,value_ac,line_ac );
//				if( error_b ) {
//					char hexLine_ac[256];
//					fprintf(stderr, "erreur ligne : '%s'\n", kbuffer2Hex( hexLine_ac, 256, line_ac, strlen(line_ac), false) );
//				}
//				else
//				{
//					error_b=configParamTraiter(param_ac,value_ac);
//				}
//			}
//		}
//		fclose(f);
//	}
//
//	return error_b;
//}

//void configSave()
//{
//	bool error_b = false;
//	FILE * f;
//	char line_ac[256];
//
//
//	f = fopen( CONFIG_SAVE,"w" );
//	if( null != f ) {
//		ConfigParam *param_pt;
//		for(param_pt=klistFirst(configListeInfos_pt);NULL!=param_pt;param_pt=klistNext(configListeInfos_pt))
//		{
//			if(NULL!=param_pt->comment_pc)
//				fprintf(f,"# %s\n",param_pt->comment_pc);
//			switch(param_pt->type_e)
//			{
//			case CONFIG_BOOL:
//			{
//				bool (*info)()=(bool (*)())param_pt->getter;
//				fprintf(f,"%s : %s\n",param_pt->nomParam_pc,
//						(((info)()==false)?"0":"1"));
//			}
//			break;
//			case CONFIG_LONG:
//			{
//				int32 (*info)()=(int32 (*)())param_pt->getter;
//				fprintf(f,"%s : %ld\n",param_pt->nomParam_pc,info());
//			}
//			break;
//			case CONFIG_CHAINE:
//			{
//			uint8 chaine[256];
//				bool (*info)(uint8 *)=(bool (*)(uint8 *))param_pt->getter;
//				info(chaine);
//				fprintf(f,"%s : %s\n",param_pt->nomParam_pc,chaine);
//			}
//			break;
//			case CONFIG_LISTE_LONG:
//			{
//				int32 index_dw=0;
//				int32 valeur_dw;
//				int32 (*info)()=(int32 (*)())param_pt->getter;
//				while(-1!=(valeur_dw=info(index_dw)))
//				{
//					fprintf(f,"%s.%ld : %ld\n",param_pt->nomParam_pc,index_dw++,valeur_dw);
//				}
//
//
//			}
//			break;
//			default:
//				fprintf(f,"%s : ?????????????????\n",param_pt->nomParam_pc);
//				break;
//
//			}
//
//
//		}
//		while( !feof(f) && null != fgets( line_ac, 255, f) ) {
//			line_ac[255] = 0;
//			trim(line_ac);
//			if( line_ac[0] != '#' && line_ac[0] != 0 ) {
//				char param_ac[128];
//				char value_ac[128];
//				error_b = !configParseLigne(param_ac,value_ac,line_ac );
//				if( error_b ) {
//					char hexLine_ac[256];
//					fprintf(stderr, "erreur ligne : '%s'\n", kbuffer2Hex( hexLine_ac, 256, line_ac, strlen(line_ac), false) );
//				}
//				else
//				{
//					TokenizerDirif tkz_t;
//
//					tokenizeDirif( &tkz_t, param_ac, CONFIG_SEP_PARAM );
//					switch(tkzDirifGetSize(&tkz_t))
//					{
//					case 1:
//					{
//						ConfigParam *param_pt=_configParamGet(param_ac);
//						if(NULL!=param_pt)
//						{
//							if(false==_configParamSet(param_pt,value_ac))
//							{
//								klogPut(NULL,LOG_INFO,"Parametre trouve : %s valeur incorrecte : %s",param_ac,value_ac);
//							}
//						}
//						else
//						{
//							klogPut(NULL,LOG_INFO,"Parametre non trouve : %s param value : %s",param_ac,value_ac);
//						}
//					}
//					break;
//					case 2:
//						_configSetListe(&tkz_t,value_ac);
//						break;
//					default:
//						klogPut(NULL,LOG_INFO,"Syntaxe parametre incorrecte : %s param value : %s",param_ac,value_ac);
//						break;
//					}
//
//				}
//			}
//		}
//		fclose(f);
//	}
//	else
//	{
//		klogPut(NULL,LOG_ERROR,"configSave: impossible d'ouvrir %s",CONFIG_SAVE);
//	}
//
//	return ;
//}
//
//BscCmd *configGetMessCour()
//{
//	int32 tailleMessage_udw;
//	uint8 message_uc[CONFIG_TAILLE_MESSAGE];
//	BscCmd *reponse_pt=NULL;
//	kmutexLock(configMutex_pt);
//	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->tailleMessage_udw),sizeof(uint32),1,&tailleMessage_udw);
//	if(0!=tailleMessage_udw)
//	{
//		sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->message_ac),sizeof(uint8),MIN(CONFIG_TAILLE_MESSAGE,tailleMessage_udw),message_uc);
//		reponse_pt=bscCmdFabriqueGetMessage(0xFF,(char *)message_uc,tailleMessage_udw);
//	}
//	kmutexUnlock(configMutex_pt);
//	return reponse_pt;
//}

/* configIsMessCour
 * ================
 * Permet de savoir si un message courant est stocké,
 * dans le cas contraire, il n'y a pas de message en
 * cours (fin de durée de validité par exemple).
 * */
//
//bool configIsMessCour()
//{
//	int32 tailleMessage_udw;
//	kmutexLock(configMutex_pt);
//	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->tailleMessage_udw),sizeof(uint32),1,&tailleMessage_udw);
//	kmutexUnlock(configMutex_pt);
//	return (0!=tailleMessage_udw?true:false);
//}
//void configSetMessCour(BscCmd *mess_pt)
//{
//	int32 lgBuffer_dw=0;
//	char *buffer_pt=NULL;
//	kmutexLock(configMutex_pt);
//	if(NULL!=mess_pt)
//	{
//		buffer_pt=bscCmdGetDatas(mess_pt);
//		lgBuffer_dw=bscCmdGetNbDatas(mess_pt);
//	}
//	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->tailleMessage_udw),sizeof(uint32),1,&lgBuffer_dw);
//	if(NULL!=buffer_pt)
//	{
//		sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->message_ac),sizeof(uint8),MIN(lgBuffer_dw,CONFIG_TAILLE_MESSAGE),buffer_pt);
//		kmmFree(NULL,buffer_pt);
//	}
//	_configSramEcrireChecksum();
//	kmutexUnlock(configMutex_pt);
//}
//

int32 configGetValJour()
{
	return configStatique.valJour;
}

int32 configGetValNuit()
{
	return configStatique.valNuit;
}

int32 configGetValSurb()
{
	return configStatique.valSurb;
}


bool configSetSeuilJour(int32 val)
{
	configStatique.seuilJour_dw=val;
	return true;
}
bool configSetValJour(int32 val)
{
	configStatique.valJour=val;
	return true;
}
bool configSetValNuit(int32 val)
{
	configStatique.valNuit=val;
	return true;
}
bool configSetValSurb(int32 val)
{
	configStatique.valSurb=val;
	return true;
}

int32 configGetSeuilJour()
{
	return configStatique.seuilJour_dw;
}

static int _configGetLuminosite(bool forceSurb_b,bool forceNuit_b)
{
	int retour=configStatique.valNuit;
	if(false==forceNuit_b)
	{
	if(false==forceSurb_b)
	{

		if(configIsAuto())
		{
			if(configDynamique.valCour_dw>configStatique.seuilJour_dw)
			{
				if(configDynamique.valCour_dw>configStatique.seuilSurb_dw)
				{
					retour=configStatique.valSurb;
				}
				else
				{
					retour=configStatique.valJour;
				}
			}
		}
		else
		{
			if(configIsSurb())
			{
				retour=configStatique.valSurb;
			}
			else if(configIsJour())
			{
				retour=configStatique.valJour;
			}
		}
	}
	else
	{
		retour=configStatique.valSurb;
	}
	}
	return retour;
}

int32 configGetNbAfficheurs()
{
	return configStatique.nbAfficheurs;
}

/* configIsOn, configSetOn
 * =======================
 * Pour allumage et extinction du panneau.
 * */

bool configIsOn()
{
	return configDynamique.on_b;
}

bool configSetOn(bool val)
{
	bool retour_b=false;
	if(val!=configDynamique.on_b)
	{
		configDynamique.on_b=val;
		retour_b=val;
	}
	return retour_b;
}

static void _configSetLuminositeChanged(bool val)
{
int nbCaissons=configGetNbCaissons();
int indice;
	for(indice=0;indice<nbCaissons;indice++)
	{
		if(configDynamique.lumAuto_b[indice]||(false==val))
		{
			configDynamique.lumChanged_b[indice]=val;
		}
	}
}
uint8 configCalculerLuminositeCourante()
{
uint8 retour_uc=0;
int seuilCour=0;
int seuilAuto=0;
	/* Recuperer la consigne a partir de la luminosite courante */
	while((seuilCour<pip_nb_seuil_cellule)&&(configDynamique.valCour_dw>pip_seuil_cellule[seuilCour].lux))
	{
		retour_uc=pip_seuil_cellule[seuilCour].tens_diode;
		seuilAuto=seuilCour;
		seuilCour++;
	}
	configDynamique.lumSeuil_uc=seuilAuto;
	return retour_uc;
}

bool configSetLuminositeCourante(uint8 lum)
{
	bool retour=false;
	if(configDynamique.lumCour_uc!=lum)
	{
		retour=true;
		printf("Changement de luminosite %d pour %d\n",configDynamique.lumCour_uc,lum);
		configDynamique.lumCour_uc=lum;
		/* Propager le changement de luminosite a tous les caissons */
		_configSetLuminositeChanged(true);
	}
	return retour;
}

void configSetLuminositeChanged(bool val)
{
int nbCaissons=configGetNbCaissons();
int indice;
	for(indice=0;indice<nbCaissons;indice++)
	{
		configDynamique.lumChanged_b[indice]=val;
	}
}

bool configLuminositeIsChanged()
{
bool retour_b=false;
int32 nbCaissons_dw=configGetNbCaissons();
int32 indice_dw;
	for(indice_dw=0;(indice_dw<nbCaissons_dw)&&(false==retour_b);indice_dw++)
	{
		retour_b=configDynamique.lumChanged_b[indice_dw];
	}
	return retour_b;
}

bool configLumIsChangedCaisson(int32 numCaisson)
{
	return configDynamique.lumChanged_b[numCaisson];
}

void configLuminositeSetChangedCaisson(int32 numCaisson,bool val)
{
	configDynamique.lumChanged_b[numCaisson]=val;
}


void configLumSetChangedCaisson(int32 numCaisson,bool val)
{
	configDynamique.lumChanged_b[numCaisson]=val;
}


void configLumSetAutoCaisson(int32 numCaisson,bool val)
{
	configDynamique.lumAuto_b[numCaisson]=val;
	configDynamique.lumChanged_b[numCaisson]=true;
}

void configLumSetValCaisson(int32 numCaisson,uint8 val)
{
	if(configDynamique.lumVal_uc[numCaisson]!=val)
	{
		configDynamique.lumVal_uc[numCaisson]=val;
		configDynamique.lumChanged_b[numCaisson]=true;
	}
}

void configSetEnCoursCaisson(int32 numCaisson,bool val)
{
		configDynamique.enCours_b[numCaisson]=val;
}


bool configIsEnCoursCaisson(int32 numCaisson)
{
	return	configDynamique.enCours_b[numCaisson];
}



/* configCalculerLuminositeCourante
 * ================================
 * Fonction permettant de recalculer la luminosite a
 * appliquer au panneau. Sur reception des informations du
 * panneau, la luminosite va être réappliquée automatiquement
 * au besoin. */
uint8 configLumCalculer(int32 numCaisson)
{
uint8 retour_uc=configDynamique.lumCour_uc;
	if(!configDynamique.lumAuto_b[numCaisson])
	{
		retour_uc=pip_seuil_cellule[MIN(pip_nb_seuil_cellule-1,configDynamique.lumVal_uc[numCaisson])].tens_diode;
	}
	return retour_uc;
}
uint8 configLumLireSeuil(int32 numCaisson)
{
uint8 retour_uc=configDynamique.lumSeuil_uc;
	if(!configDynamique.lumAuto_b[numCaisson])
	{
		retour_uc=MIN(pip_nb_seuil_cellule-1,configDynamique.lumVal_uc[numCaisson]);
	}
	return retour_uc;
}

void configSetSurbDyn(bool val)
{
	if(val!=configDynamique.surb_b)
	{
		configDynamique.surb_b=val;
		configSetLuminositeChanged(true);
	}
}

void configSetNuitDyn(bool val)
{
	if(val!=configDynamique.nuit_b)
	{
		configDynamique.nuit_b=val;
		configSetLuminositeChanged(true);
	}
}

int32 configGetValCour()
{
	return configDynamique.valCour_dw;
}

bool configSetValCour(int32 valeur_dw)
{
	bool retour=false;
	if(valeur_dw!=configDynamique.valCour_dw)
	{
		retour=true;
		configDynamique.valCour_dw=valeur_dw;
	}
	return retour;
}
bool configSetOk(bool val)
{
	bool retour=false;
	if(val!=configDynamique.ok_b)
	{
		configDynamique.ok_b=val;
		retour=true;
		/* Ajouter ici une erreur majeure */
		if(val)
		{
			eriSupprimer(E_eriMajeure,ERI_DEF_PIP);
		}
		else
		{
			eriAjouter(E_eriMajeure,ERI_DEF_PIP);
		}
	}
	return retour;
}

bool configIsOk()
{
	return configDynamique.ok_b;
}
//
//void configResetTempoScrut()
//{
//	configDynamique.tempoScrut_dw=configStatique.scrutationPC_dw*1000;
//}
//
//uint32 configGetTempoScrut()
//{
//	return configDynamique.tempoScrut_dw;
//}
//
//void configSetTempoScrut(uint32 validite_udw)
//{
//	configDynamique.tempoScrut_dw=validite_udw;
//}
//
//bool configRemoveTempoScrut(uint32 delta_udw)
//{
//bool retour=false;
//	uint32 validite_udw=configGetTempoScrut();
//	if(delta_udw<validite_udw)
//	{
//		configSetTempoScrut(validite_udw-delta_udw);
//	}
//	else
//	{
//		retour=true;
//		configSetTempoScrut(0);
//	}
//	return retour;
//}
//

int32 configGetSeuilAffPixelMin(void)
{
	return configStatique.seuilPixelMinAff;
}
int32 configGetSeuilAffPixelMax(void)
{
	return configStatique.seuilPixelMaxAff;

}
int32 configGetSeuilModulePixelMin(void)
{
	return configStatique.seuilPixelMinMod;

}
int32 configGetSeuilModulePixelMax(void)
{
	return configStatique.seuilPixelMaxMod;
}
bool configSetSeuilAffPixelMin(int32 val)
{
	if(configStatique.seuilPixelMinAff!=val)
	{
		configStatique.seuilPixelMinAff=val;
	}
	return true;
}

bool configSetSeuilAffPixelMax(int32 val)
{
	if(configStatique.seuilPixelMaxAff!=val)
	{
		configStatique.seuilPixelMaxAff=val;
	}
	return true;
}
bool configSetSeuilModulePixelMin(int32 val)
{
	if(configStatique.seuilPixelMinMod!=val)
	{
		configStatique.seuilPixelMinMod=val;
	}
	return true;
}
bool configSetSeuilModulePixelMax(int32 val)
{
	if(configStatique.seuilPixelMaxMod!=val)
	{
		configStatique.seuilPixelMaxMod=val;
	}
	return true;
}

void configResetReprise()
{
	configDynamique.reprise_b=false;
}
bool configIsReprise()
{
	return configDynamique.reprise_b=false;
}


bool 	configSetSeuilSurb(int32 val)
{
	configStatique.seuilSurb_dw=val;
	return true;
}

int32 	configGetSeuilSurb(void)
{
	return configStatique.seuilSurb_dw;
}

bool 	configSetTempoTest(int32 val)
{
	configStatique.tempoTest_dw=val;
	return true;
}

int32 	configGetTempoTest(void)
{
	return configStatique.tempoTest_dw;
}

bool 	configSetPeriodeTest(int32 val)
{
	configStatique.periodeTest_dw=val;
	return true;
}

int32 	configGetPeriodeTest(void)
{
	return configStatique.periodeTest_dw;
}

bool 	configSetTemperatureChauffage(int32 val)
{
	configStatique.seuilChauffage_dw=val;
	return true;
}

int32 	configGetTemperatureChauffage(void)
{
	return configStatique.seuilChauffage_dw;
}


bool 	configSetPeriodeTestPixel(int32 val)
{
	configStatique.periodeTestPixel_dw=val;
	return true;
}

int32 	configGetPeriodeTestPixel(void)
{
	return configStatique.periodeTestPixel_dw;
}

bool 	configSetPeriodeAnimation(int32 val)
{
	configStatique.tempoAnim_dw=val;
	return true;
}

int32 	configGetPeriodeAnimation(void)
{
	return configStatique.tempoAnim_dw;
}

bool configSetPaddingBefore(int32 pad_dw)
{
bool retour=false;
	if(0<=pad_dw)
	{
		configStatique.paddingBefore_dw=pad_dw;
		retour=true;
	}
	return retour;
}

int32 configGetPaddingBefore(void)
{
	return MAX(0,configStatique.paddingBefore_dw);
}

bool configSetPaddingAfter(int32 pad_dw)
{
bool retour=false;
	if(0<=pad_dw)
	{
	configStatique.paddingAfter_dw=pad_dw;
		retour=true;
	}
	return retour;
}

int32 configGetPaddingAfter(void)
{
	return MAX(0,configStatique.paddingAfter_dw);
}

bool 	configSetPollingAff(int32 val)
{
	configStatique.periodeScrutAff_dw=val;
	return true;
}

int32 	configGetPollingAff(void)
{
	return configStatique.periodeScrutAff_dw;
}

bool 	configSetPollingSonde(int32 val)
{
	configStatique.periodeScrutThl_dw=val;
	return true;
}

int32 	configGetPollingSonde(void)
{
	return configStatique.periodeScrutThl_dw;
}

bool 	configSetPollingEs(int32 val)
{

	configStatique.periodeScrutEs_dw=val;
	return true;
}

int32 	configGetPollingEs(void)
{
	return configStatique.periodeScrutEs_dw;
}

bool 	configSetNbThls(int32 val)
{
	bool retour_b=false;
	if((val>=0)&&((val<THL_NB_MAX)))
	{
		configStatique.seuilSurb_dw=val;
		retour_b=true;
	}
	return retour_b;
}

int32 	configGetNbThls(void)
{
	return configStatique.nbThls;
}

int32 	configGetNbEss(void)
{
	/* Pour l'heure c'est fige, il y a une seule ES */
	return 1;
}

bool 	configSetAdresseThl(int32 val,int32 adresse)
{
	bool retour_b=false;
	if((val>=0&&(val<configGetNbThls())))
	{
		configStatique.adresseThl[val]=adresse;
	}
	return retour_b;
}

int32 	configGetAdresseThl(int32 val)
{
	int retour=-1;
	if((val>=0&&(val<configGetNbThls())))
	{
		retour=configStatique.adresseThl[val];

	}
	return retour;
}

bool 	configSetModeThl(int32 val,int32 mode)
{
	bool retour_b=false;
	if((val>=0&&(val<configGetNbThls())))
	{
		configStatique.modeThl[val]=mode;
	}
	return retour_b;
}

int32 	configGetModeThl(int32 val)
{
	int retour=-1;
	if((val>=0&&(val<configGetNbThls())))
	{
		retour=configStatique.modeThl[val];

	}
	return retour;
}

bool 	configSetAdresseEs(int32 val)
{
	configStatique.adresseEs=val;
	return true;
}
int32 configGetNumPortIp(void)
{
	return configStatique.numPort_dw;
}

void configSetNumPortIp(int32 val)
{
	configStatique.numPort_dw=val;
}

int32 	configGetAdresseEs(void)
{
	return configStatique.adresseEs;
}

bool configIsRebouclage()
{
	return configDynamique.rebouclage_b;
}

void configSetRebouclage(bool val)
{
	configDynamique.rebouclage_b=val;
}

bool configIsInit()
{
	return configDynamique.init_b;
}

void configSetInit(bool val)
{
	configDynamique.init_b=val;
}

/* Nouvelles fonctions pour le traitement de plusieurs caissons... */

int32 configGetNbCaissons(void)
{
	return pip_nb_caisson;
}

int32 configGetNbAfficheursCaisson(int32 numCaisson)
{
	int retour_dw=-1;
	if((0<=numCaisson)&&(numCaisson<pip_nb_caisson))
		{
		T_pip_cf_caisson *caisson = &pip_cf_caisson[numCaisson];
			retour_dw= caisson->nb_car;
		}
		return retour_dw;
}

int32 configGetPremAfficheurCaisson(int32 numCaisson)
{
int retour_dw=-1;
	if((0<=numCaisson)&&(numCaisson<pip_nb_caisson))
	{
	T_pip_cf_caisson *caisson = &pip_cf_caisson[numCaisson];
		retour_dw= caisson->num_car;
	}
	return retour_dw;
}

int32 configGetTypeCaisson(int32 numCaisson)
{
int retour_dw=N_AFF;
	if((0<=numCaisson)&&(numCaisson<pip_nb_caisson))
	{
	T_pip_cf_caisson *caisson = &pip_cf_caisson[numCaisson];
		retour_dw= caisson->type;
	}
	return retour_dw;
}
