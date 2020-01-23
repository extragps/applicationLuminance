
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 26/07/01
 * Fichier	: picLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,26Jul01,xag  creation.
 * ************************************************************************	*/

#ifndef _PIC_LIB_H
#define _PIC_LIB_H

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define PIC_SEUIL_DEF		0
#define PIC_SEQ_DEF   		'B'

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* -------------------------------
 * E_picTypeDonnee
 * ===============
 * Enumeration des donnees disponibles
 * --------------------------------	*/

typedef enum E_picTypeDonnee
{
  E_picPic,
  E_picTrou,
  E_picHes,
  E_picReb,
  E_picTypeMax
} E_picTypeDonnee;

/* --------------------------------
 * T_picCapt
 * ==========
 * Differents compteurs de defauts.
 * On dispose pour chaque type de
 * defaut :
 * - d'un indicateur pour savoir si le
 *   seuil a ete depasse,
 * - d'un compteur de mesures,
 * - d'un compteur absolu de defautt-
 *   (il s'agit en fait du nombre 
 *   courant de defaut detectes),
 * - du nombre de defauts detectes 
 *   depuis la derniere interrogation.
 * --------------------------------	*/

typedef struct T_picCapt
{
  BOOL valDisp;
  unsigned short nbMes;
  BOOL indic[E_picTypeMax];
  USHORT valRel[E_picTypeMax];
  ULONG valAbs[E_picTypeMax];
} T_picCapt;

/* --------------------------------
 * T_picCaptSeuil
 * ===============
 * Donnees necessaires au traitement 
 * des defauts. Il s'agit d'un seuil
 * en pourcentage qui est le rapport
 * entre le nombre de defauts et 
 * le nombre de detections.
 * --------------------------------	*/

typedef struct T_picCaptSeuil
{
  unsigned short seuil[E_picTypeMax];
} T_picCaptSeuil;

/* --------------------------------
 * T_picStation
 * ============
 * Structure contenant toutes les 
 * informations utiles au traitement
 * des donnees picrrantes par la 
 * station.
 * --------------------------------	*/

typedef struct T_picStation
{
  unsigned char seq;
  T_picCaptSeuil capt[NBCANAL];
} T_picStation;

/* ********************************
 * VARIABLES GLOBALES DU MODULE
 * ********************************	*/

extern T_picStation picStation;
extern T_picStation *picStationPt;

/* ********************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ********************************	*/

void picInit (T_picStation *);
void picClear (T_picStation *);
void picReset (T_picStation *);
void picTester (T_picStation *);
BOOL picSetSeuilCapt (T_picStation *, int, E_picTypeDonnee, unsigned short);
unsigned short picGetSeuilCapt (T_picStation *, int, E_picTypeDonnee);
BOOL picSetSeuil (T_picStation *, E_picTypeDonnee, unsigned short);
BOOL picSetIndicCapt (T_picStation *, int capteur, ULONG, ULONG, ULONG,
                      ULONG);
void picSetSeq (T_picStation *, unsigned char seq);
unsigned char picGetSeq (T_picStation *);
void picSetHeureDeb (T_picStation *, unsigned char heure);
unsigned char picGetHeureDeb (T_picStation *);
void picSetHeureFin (T_picStation *, unsigned char heure);
unsigned char picGetHeureFin (T_picStation *);
void picSetPeriode (T_picStation *, ULONG);
ULONG picGetPeriode (T_picStation *);
ULONG picGetAbsCapt (T_picStation *, int, E_picTypeDonnee);
unsigned short picGetTauxCapt (T_picStation *, int, E_picTypeDonnee);
unsigned short picGetIndicCapt (T_picStation *, int, E_picTypeDonnee);
void picIncrNbMesCapt (T_picStation *, int);

/* ********************************
 * FIN DE picLib.h
 * ********************************	*/

#endif
