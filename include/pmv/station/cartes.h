#ifndef CARTES_H_
#define CARTES_H_

#include "lumAdresse.h"
#include "lumMsg.h"
#include "event.h"

/* ********************************	*
 * PROTOTYPAGE DES FONCTIONS DU MODULE
 * ********************************	*/
void stationInit(void);
void stationTerm(void);
bool stationIsOk(void);
void stationReset(void);
bool stationIsDefautCom(void);


bool afficheursIsOk(void);
void afficheursReset(void);
void afficheursInit(void);
void afficheursTerm(void);
Event * afficheursTraiterMessage(LumMsg *,LumMsg *);
void afficheursTraiterDefautCom(LumMsg *);
uint8 afficheursGetDefautPixel(void);
void afficheursGetEtatPixels(int numAff,uint8* etatPixels);
void afficheursGetInfos(char *,int);
bool afficheursIsDefautCom(void);
bool afficheursIsDefautAnim(void);
bool afficheursIsDefautActivation(void);
bool afficheursControlerChargement(void);
uint16 afficheursGetSortieDefauts(void);
uint16 afficheursGetMasqueDefauts(void);
void afficheursSetSortieDefauts(uint16 defauts);
void afficheursInitUnitaire(int numAfficheur,int adresse,int caisson);
uint8 afficheursGetDefautPixelNumero(int indice);
bool afficheursIsOkNumAff(int *num);

bool essIsOk(void);
void essReset(void);
void essInit(void);
void essTerm(void);
Event * essTraiterMessage(LumMsg *);
void essTraiterDefautCom(LumMsg *);
bool essIsDefautCom(void);
bool essIsDefaut(void);
void essGetInfos(char *,int);
bool essDefautAlim(void);
int essGetAlim(void);
void essSetConsigne(bool consigne_b);
bool essIsConsigne(void);
bool esIsSabordage(void);
bool esIsDefautAlim(void);
bool esIsDefautBis(void);
void esSetSabordage(void);
void esResetSabordage(void);

bool thlsIsOk(void);
void thlsReset(void);
void thlsInit(void);
void thlsTerm(void);
Event *thlsTraiterMessage(LumMsg *);
void thlsTraiterDefautCom(LumMsg *);
void thlsGetInfos(char *,int);
int32 thlsGetLuminosite(void);
int32 thlsGetTemperature(void);
bool thlsIsDefautNumero(int numero);

#endif /*CARTES_H_*/
