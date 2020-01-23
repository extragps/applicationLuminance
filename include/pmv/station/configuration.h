#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

/* ********************************	*
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define AFFICHEUR_NB_MAX	54			// Nombre d'afficheurs dans le caisson.

#define THL_NB_MAX	2			// Nombre de cartes luminosite/temperature
#define THL_TEMP_AD 0			// Carte temperature
#define THL_LUM_AD 1			// Carte luminosite

#define ES_ALIM 0
#define ES_ALIM_INPUT 0x07

#define STATION_DEF_PIXEL_NONE 0
#define STATION_DEF_PIXEL_MINEUR 2
#define STATION_DEF_PIXEL_MAJEUR 3


/* ************************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ************************************	*/

void configInit(void);
void configTerm(void);
void configParamInit(void);
void configParamTerm(void);
bool configLoad(void);
void configSave(void);
void configInitStatique(void);
void configStatiqueSet(void *config_p);
void configStatiqueGet(void *config_p);

int configGetLuminosite(bool force);
bool configSetLuminositeCourante(uint8 luminosite);

bool configSetSeuilJour(int32 val);
bool configSetValJour(int32 val);
bool configSetValNuit(int32 val);
bool configSetValSurb(int32 val);

bool configSetNbAfficheurs(int32 nombre);
int32 configGetAdresseAfficheur(int32 numero);
bool configSetAdresseAfficheur(int32 numero,int32 valeur);
int32 configGetSortieAfficheur(int32 numero);
bool configSetSortieAfficheur(int32 numero,int32 valeur);
int32 configGetCaissonAfficheur(int32 numero);
bool configSetCaissonAfficheur(int32 numero,int32 valeur);

uint8 configCalculerLuminosite();
uint8 configLumCalculer(int32 numCaisson);

void configLumSetChangedCaisson(int32 numCaisson,bool val);
void configLumSetAutoCaisson(int32 numCaisson,bool val);
void configLumSetValCaisson(int32 numCaisson,uint8 val);
bool configLuminositeIsChanged();
bool configLumIsChangedCaisson(int32 numCaisson);
void configLuminositeSetChanged(bool val);
void configSetEnCoursCaisson(int32 numCaisson,bool val);
bool configIsEnCoursCaisson(int32 numCaisson);

int32 configGetNbAfficheurs(void);
bool configSetNbAfficheurs(int32 nombre);



bool 	configSetSeuilSurb(int32);
int32 	configGetSeuilSurb(void);
bool 	configSetTempoTest(int32);
int32 	configGetTempoTest(void);
void 	configSetNumPortIp(int32);
int32 	configGetNumPortIp(void);
bool 	configSetPeriodeTest(int32);
int32 	configGetPeriodeTest(void);
bool 	configSetPeriodeTestPixel(int32);
int32 	configGetPeriodeTestPixel(void);
bool 	configSetPeriodeAnimation(int32);
int32 	configGetPeriodeAnimation(void);
bool 	configSetPollingAff(int32);
int32 	configGetPollingAff(void);
bool 	configSetPollingSonde(int32);
int32 	configGetPollingSonde(void);
bool 	configSetPollingEs(int32);
int32 	configGetPollingEs(void);
bool 	configSetNbThls(int32);
int32 	configGetNbThls(void);
int32 	configGetNbEss(void);
bool 	configSetAdresseThl(int32,int32);
int32 	configGetAdresseThl(int32);
int32 	configGetModeThl(int32);
bool 	configSetModeThl(int32,int32);
bool 	configSetAdresseEs(int32);
int32 	configGetAdresseEs(void);
bool 	configIsRebouclage(void);
void 	configSetRebouclage(bool val);
bool 	configSetTemperatureChauffage(int32 val);
int32 	configGetTemperatureChauffage(void);
uint32 	configGetTempoScrut(void);
bool 	configSetPaddingBefore(int32 pad_dw);
int32 	configGetPaddingBefore(void);
bool 	configSetPaddingAfter(int32 pad_dw);
int32 	configGetPaddingAfter(void);

void configSetSurbDyn(bool val);
void configSetNuitDyn(bool val);

int32 configGetSeuilAffPixelMin(void);
int32 configGetSeuilAffPixelMax(void);
int32 configGetSeuilModulePixelMin(void);
int32 configGetSeuilModulePixelMax(void);

bool configSetSeuilAffPixelMin(int32);
bool configSetSeuilAffPixelMax(int32);
bool configSetSeuilModulePixelMin(int32);
bool configSetSeuilModulePixelMax(int32);


/*
 * TODO : remplacer la notion de message courant telle qu'elle est
 * definie pour l'heure.
 *
BscCmd *configGetMessCour(void);
void configSetMessCour(BscCmd *mess_pt);
*/
bool configIsOn(void);
bool configSetOn(bool);

bool configSetValCour(int32 valeur_dw);

bool configSetOk(bool val);
bool configIsOk(void);

int32 configGetValCour(void);
int32 configGetSeuilJour(void);
int32 configGetValJour(void);
int32 configGetValNuit(void);
int32 configGetValSurb(void);

/* Nouvelles fonctions pour recuperer le nombres par caissons
 * configures.... */
int32 configGetNbAfficheursCaisson(int32 numCaisson);
int32 configGetPremAfficheurCaisson(int32 numCaisson);
int32 configGetNbCaissons(void);
int32 configGetTypeCaisson(int32 numCaisson);

#endif /*CONFIGURATION_H_*/
