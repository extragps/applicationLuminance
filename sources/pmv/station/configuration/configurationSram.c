#include <stdio.h>
#include <string.h>
#include <kcommon.h>
#include <klog.h>
#include <tokenizerDirif.h>
#include "pip_def.h"
#include "configuration.h"
#include "configurationSram.h"
#include "cpu432/sram.h"

#include "standard.h"
#include "eriLib.h"


/* ************************************
 * DEFINITION DES TYPES
 * ************************************	*/

/* Donnees stockées en SRAM */
typedef struct _ConfigSram
{
	int32 checksum_dw;
	bool deb_b;
	bool entretien_b;
	bool testEnCours_b;
	bool term_b;
	bool jour_b;
	bool surb_b;
	bool auto_b;
	uint8 indTest_uc;
	uint32 validite_udw[NB_CAISSON];
	uint32 tailleMessage_udw;
	Ktimestamp dateCoupure_t;
	uint8 message_ac[CONFIG_TAILLE_MESSAGE];
	uint32 diffSect_udw;
	uint32 cptUn_udw;
	uint32 cptDix_udw;
	uint32 cptPlus_udw;
	uint32 gar_udw;
	uint32 rst_udw;
	uint32 ovf_udw;
	uint32 ini_udw;
	uint32 spc_udw;
	uint32 reinit_udw;
	T_eriList eri_t;
	bool fin_b;
	Ktimestamp dateCour_t;
	Ktimestamp dateReprise_t;
} ConfigSram;

/* ************************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ************************************	*/

static void _configSramInit(void);
static int32 _configSramCalculerChecksum(void);
static void _configSramEcrireChecksum(void);
static bool _configSramControlerChecksum(void);
static void _configSetDateCoupure(void);
static void _configSetDateReprise(void);
static void _configResetDateCoupure(void);
static void _configSetCptUn(uint32);
static void _configSetCptDix(uint32);
static void _configSetCptPlus(uint32);
static void _configSetReinit(uint32);

/* ************************************
 * DECLARATION DES VARIABLES LOCALES
 * ************************************	*/

static Sram *configSram_pt=NULL;
//static Kmutex *configSramMutex_pt=NULL;

/* ************************************
 * FONCTIONS DU MODULE
 * ************************************ */


int32 configSramInit(void *adresseSram_p)
{
int32 taille=(sizeof(ConfigSram)/1024+1)*1024;
	configSram_pt=sramNew((void *)CONFIG_AD_SRAM,taille);
	if(false==_configSramControlerChecksum())
	{
		_configSramInit();
	}
	else
	{
		_configSetDateCoupure();
	}
	configSetTestEnCours(false);
	_configSetDateReprise();
	return (c4sramGetTotalSize(configSram_pt)/sizeof(long)+1)*sizeof(long);
}


void configSramTerm()
{
	sramDelete(configSram_pt);
}

/* _configSramInit
 * ===============
 * Dans le cas ou le checksum de la sram est defectueux, on
 * realise une reinitialisatio de la sram.
 * */

static void _configSramInit()
{
int indice;
	configSetJour(false);
	configSetSurb(false);
	configSetAuto(false);
//	configSetMessCour(NULL);
	for(indice=0;indice<NB_CAISSON;indice++)
	{
	configSetValidite(0,indice);
	}
	configSetTestEnCours(false);
	configSetNumTest(0);
	configSetTerm(false);
	configSetEntretien(false);
	_configResetDateCoupure();
	configSetDiffSect(0);
	_configSetCptUn(0);
	_configSetCptDix(0);
	_configSetCptPlus(0);
	configSetGar(0);
	configSetRst(0);
	configSetOvf(0);
	configSetIni(0);
	configSetSpc(0);
	_configSetReinit(0);
}
static int32 _configSramCalculerChecksum()
{
	ConfigSram config;
	int32 checksum=0;
	sramRead(configSram_pt,0,sizeof(ConfigSram),1,&config);
	{
		int offsetDeb=(int)(&((ConfigSram *)0)->deb_b);
		int offsetFin=(int)(&((ConfigSram *)0)->fin_b);
		int offset;
		for(offset=offsetDeb;offset<offsetFin;offset++)
		{
			checksum+=((uint8 *)&config)[offset];
		}
	}
	return checksum;
}

static void _configSramEcrireChecksum()
{
	int32 checksum=_configSramCalculerChecksum();
	/* Le checksum est situé en début de bloc. */
	sramWrite(configSram_pt,0,sizeof(int32),1,&checksum);
}

static bool _configSramControlerChecksum()
{
	int32 checksumCalc_dw=_configSramCalculerChecksum();
	int32 checksumEnreg_dw;
	bool retour_b=false;
	if(sramRead(configSram_pt,0,sizeof(int32),1,&checksumEnreg_dw)==sizeof(int32))
	{
		retour_b=(checksumCalc_dw==checksumEnreg_dw);
	}
	return retour_b;
}

//bool configIsMessCour()
//{
//	int32 tailleMessage_udw;
//	kmutexLock(configSramMutex_pt);
//	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->tailleMessage_udw),sizeof(uint32),1,&tailleMessage_udw);
//	kmutexUnlock(configSramMutex_pt);
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
bool configIsJour()
{
	bool retour_b=false;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->jour_b),sizeof(bool),1,&retour_b);
	return retour_b;
}

bool configIsSurb()
{
	bool retour_b=false;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->surb_b),sizeof(bool),1,&retour_b);
	return retour_b;
}

bool configIsAuto()
{
	bool retour_b=false;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->auto_b),sizeof(bool),1,&retour_b);
	return retour_b;
}

void configSetJour(bool val)
{
	configSetLuminositeChanged(true);
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->jour_b),sizeof(bool),1,&val);
	_configSramEcrireChecksum();
}

void configSetSurb(bool val)
{
	configSetLuminositeChanged(true);
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->surb_b),sizeof(bool),1,&val);
	_configSramEcrireChecksum();
}

void configSetAuto(bool val)
{
	configSetLuminositeChanged(true);
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->auto_b),sizeof(bool),1,&val);
	_configSramEcrireChecksum();
}

uint32 configGetValidite(int32 numCaisson)
{
	int32 retour_udw=0;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->validite_udw[numCaisson]),sizeof(uint32),1,&retour_udw);
	return retour_udw;
}


void configSetValidite(uint32 validite_udw,int32 numCaisson)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->validite_udw[numCaisson]),sizeof(uint32),1,&validite_udw);
	_configSramEcrireChecksum();
}

bool configRemoveValidite(uint32 delta_udw,int32 numCaisson)
{
	bool retour=false;
	uint32 validite_udw=configGetValidite(numCaisson);
	if(delta_udw<validite_udw)
	{
		configSetValidite(validite_udw-delta_udw,numCaisson);
	}
	else
	{
		retour=true;
		configSetValidite(0,numCaisson);
	}
	return retour;
}

bool configIsTestEnCours()
{
	bool retour_b=0;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->testEnCours_b),sizeof(bool),1,&retour_b);
	return retour_b;
}


void configSetTestEnCours(bool val)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->testEnCours_b),sizeof(bool),1,&val);
	_configSramEcrireChecksum();
}

void configSetNumTest(uint8 val)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->indTest_uc),sizeof(uint8),1,&val);
	_configSramEcrireChecksum();
}

uint8 configGetNumTest()
{
	uint8 retour_b=0;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->indTest_uc),sizeof(uint8),1,&retour_b);
	return retour_b;
}

bool configIsTerm()
{
	bool retour_b=0;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->term_b),sizeof(bool),1,&retour_b);
	return retour_b;
}

void configSetTerm(bool val)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->term_b),sizeof(bool),1,&val);
	_configSramEcrireChecksum();
}
bool configIsEntretien()
{
	bool retour_b=0;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->entretien_b),sizeof(bool),1,&retour_b);
	return retour_b;
}

static void _configSetDateCoupure()
{
	Ktimestamp dateCoupure_t;
	/* Au demarrage de l'application, la date de la derniere coupure connue est positionnee. */
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->dateCour_t),sizeof(Ktimestamp),1,&dateCoupure_t);
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->dateCoupure_t),sizeof(Ktimestamp),1,&dateCoupure_t);
	_configSramEcrireChecksum();
}

static void _configResetDateCoupure()
{
	Ktimestamp dateCoupure_t;
	ktsGetTime(&dateCoupure_t);
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->dateCoupure_t),sizeof(Ktimestamp),1,&dateCoupure_t);
	_configSramEcrireChecksum();
}

static void _configSetDateReprise()
{
	Ktimestamp dateReprise_t;
	ktsGetTime(&dateReprise_t);
	/* Pas de calcul de checksum pour la date de coupure...*/
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->dateReprise_t),sizeof(Ktimestamp),1,&dateReprise_t);
}

void configGetDateReprise(Ktimestamp *ts_pt)
{
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->dateReprise_t),sizeof(Ktimestamp),1,ts_pt);
}

void configGetDateCoupure(Ktimestamp *ts_pt)
{
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->dateCoupure_t),sizeof(Ktimestamp),1,ts_pt);
}
void configSetDateCour(Ktimestamp *dateCour_pt)
{
	/* Pas de calcul de checksum pour la date de coupure...*/
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->dateCour_t),sizeof(Ktimestamp),1,dateCour_pt);
}

void configSetEntretien(bool val)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->entretien_b),sizeof(bool),1,&val);
	_configSramEcrireChecksum();
}

void configSetDiffSect(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->diffSect_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetDiffSect(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->diffSect_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

static void _configSetCptUn(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->cptUn_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetCptUn(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->cptUn_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrCptUn(void)
{
uint32 val_udw=configGetCptUn();
	val_udw=(val_udw+1);
	_configSetCptUn(val_udw);
}

static void _configSetCptDix(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->cptDix_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetCptDix(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->cptDix_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrCptDix(void)
{
uint32 val_udw=configGetCptDix();
	val_udw=(val_udw+1);
	_configSetCptDix(val_udw);
}

static void _configSetCptPlus(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->cptPlus_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetCptPlus(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->cptPlus_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrCptPlus(void)
{
uint32 val_udw=configGetCptPlus();
	val_udw=(val_udw+1);
	_configSetCptPlus(val_udw);
}

void configSetGar(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->gar_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetGar(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->gar_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrGar(void)
{
uint32 val_udw=configGetGar();
	val_udw=(val_udw+1)%1000;
	configSetGar(val_udw);
}

void configSetRst(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->rst_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetRst(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->rst_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrRst(void)
{
uint32 val_udw=configGetRst();
	val_udw=(val_udw+1)%1000;
	configSetRst(val_udw);
}


void configSetOvf(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->ovf_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetOvf(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->ovf_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrOvf(void)
{
uint32 val_udw=configGetOvf();
	val_udw=(val_udw+1)%1000;
	configSetOvf(val_udw);
}

void configSetIni(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->ini_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetIni(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->ini_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrIni(void)
{
uint32 val_udw=configGetIni();
	val_udw=(val_udw+1)%1000;
	configSetIni(val_udw);
}

void configSetSpc(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->spc_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetSpc(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->spc_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrSpc(void)
{
uint32 val_udw=configGetSpc();
	val_udw=(val_udw+1)%1000;
	configSetSpc(val_udw);
}

static void _configSetReinit(uint32 val_udw)
{
	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->reinit_udw),sizeof(uint32),1,&val_udw);
	_configSramEcrireChecksum();
}
uint32 configGetReinit(void)
{
uint32 val_udw;
	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->reinit_udw),sizeof(uint32),1,&val_udw);
	return val_udw;
}

void configIncrReinit(void)
{
uint32 val_udw=configGetReinit();
	val_udw=(val_udw+1);
	_configSetReinit(val_udw);
}
//
//void configSramLireEri(void *eri_pt)
//{
//	sramRead(configSram_pt,(int32)&(((ConfigSram *)0)->eri_t),sizeof(T_eriList),1,eri_pt);
//}
//
//void configSramEcrireEri(void *eri_pt)
//{
//	sramWrite(configSram_pt,(int32)&(((ConfigSram *)0)->eri_t),sizeof(T_eriList),1,eri_pt);
//	_configSramEcrireChecksum();
//}
//
