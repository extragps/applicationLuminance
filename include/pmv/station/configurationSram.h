#ifndef CONFIGURATION_SRAM_H_
#define CONFIGURATION_SRAM_H_

/* ********************************	*
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define CONFIG_TAILLE_SRAM 0xFF00
#define CONFIG_AD_SRAM 0x90000100
#define CONFIG_TAILLE_MESSAGE 256

/* ************************************
 * DEFINITION DES TYPES LOCAUX
 * ************************************	*/

/* ************************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ************************************	*/


int32 configSramInit(void *);
void configSramTerm(void);

bool configIsJour(void);
bool configIsSurb(void);
bool configIsAuto(void);
void configSetJour(bool val);
void configSetSurb(bool val);
void configSetAuto(bool val);
uint32 configGetValidite();
void configSetValidite(uint32 validite_udw,int32 numCaisson);
bool configRemoveValidite(uint32 delta_udw,int32 numCaisson);
bool configIsTestEnCours(void);
void configSetTestEnCours(bool val);
void configSetNumTest(uint8 val);
uint8 configGetNumTest(void);
bool configIsTerm(void);
void configSetTerm(bool val);
bool configIsEntretien(void);
void configGetDateReprise(Ktimestamp *ts_pt);
void configGetDateCoupure(Ktimestamp *ts_pt);
void configSetDateCour(Ktimestamp *dateCour_pt);
void configSetEntretien(bool val);

void configSetDiffSect(uint32);
void configSetGar(uint32);
void configSetRst(uint32);
void configSetIni(uint32);
void configSetOvf(uint32);
void configSetSpc(uint32);
void configIncrCptUn(void);
void configIncrCptDix(void);
void configIncrCptPlus(void);
void configIncrGar(void);
void configIncrRst(void);
void configIncrOvf(void);
void configIncrIni(void);
void configIncrSpc(void);
void configIncrReinit(void);

uint32 configGetDiffSect(void);
uint32 configGetCptUn(void);
uint32 configGetCptDix(void);
uint32 configGetCptPlus(void);
uint32 configGetGar(void);
uint32 configGetRst(void);
uint32 configGetOvf(void);
uint32 configGetIni(void);
uint32 configGetSpc(void);
uint32 configGetReinit(void);
//
//void configSramLireEri(void *eri_pt);
//void configSramEcrireEri(void *eri_pt);

#endif /*CONFIGURATION_H_*/
