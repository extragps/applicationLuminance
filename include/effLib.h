
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 26/07/01
 * Fichier	: effLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,26Jul01,xag  creation.
 * ************************************************************************	*/

#ifndef _EFF_LIB_H
#define _EFF_LIB_H

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define EFF_DEF				0
#define EFF_MIN_DEF   		10
#define EFF_SEQ_DEF   		'H'
#define EFF_VAL_DEF   		100

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* --------------------------------
 * T_effCanal
 * ==========
 * Indicateurs de donnees effrrantes.
 * --------------------------------	*/

typedef struct T_effCanal
{
  BOOL indic;
  unsigned short eff;
} T_effCanal;

/* --------------------------------
 * T_effCanalSeuil
 * ===============
 * Donnees necessaires au traitement 
 * des donnees effrrantes pour un 
 * canal.
 * --------------------------------	*/

typedef struct T_effCanalSeuil
{
  unsigned short seuil;
} T_effCanalSeuil;

/* --------------------------------
 * T_effStation
 * ============
 * Structure contenant toutes les 
 * informations utiles au traitement
 * des donnees effrrantes par la 
 * station.
 * --------------------------------	*/

typedef struct T_effStation
{
  unsigned short minimum;
  unsigned char seq;
  T_effCanalSeuil canal[NBCANAL];
} T_effStation;

/* ********************************
 * VARIABLES GLOBALES DU MODULE
 * ********************************	*/

extern T_effStation effStation;
extern T_effStation *effStationPt;
extern ULONG effNbMesCapteur[NBCAPTEUR];
extern ULONG effNbMesCanal[NBCANAL];

/* ********************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ********************************	*/

void effInit (T_effStation *);
void effClear (T_effStation *);
void effReset (T_effStation *);
void effTester (T_effStation *);
BOOL effSetSeuilCanal (T_effStation *, int, unsigned short);
unsigned short effGetSeuilCanal (T_effStation *, int);
BOOL effSetSeuil (T_effStation *, unsigned short);
BOOL effSetEffCanal (T_effStation *, int canal, unsigned short);
void effSetSeq (T_effStation *, unsigned char seq);
unsigned char effGetSeq (T_effStation *);
void effSetMin (T_effStation *, unsigned short min);
unsigned short effGetMin (T_effStation *);
void effSetPeriode (T_effStation *, ULONG);
ULONG effGetPeriode (T_effStation *);

/* ********************************
 * FIN DE effLib.h
 * ********************************	*/

#endif
