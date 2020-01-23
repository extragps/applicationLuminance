
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 26/07/01
 * Fichier	: abeLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,26Jul01,xag  creation.
 * ************************************************************************	*/

#ifndef _ABE_LIB_H
#define _ABE_LIB_H

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define ABE_S_QT_MIN_DEF	0
#define ABE_S_QT_MAX_DEF	9999
#define ABE_S_TT_MIN_DEF	0
#define ABE_S_VT_MOY_DEF	255
#define ABE_H_DEB_DEF   	7
#define ABE_H_FIN_DEF   	22
#define ABE_SEQ_DEF   		'B'

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* -------------------------------
 * E_abeTypeDonnee
 * ===============
 * Enumeration des donnees disponibles
 * --------------------------------	*/

typedef enum E_abeTypeDonnee
{
  E_abeQTmin,
  E_abeQTmax,
  E_abeTTmin,
  E_abeVTmoy,
  E_abeTypeMax
} E_abeTypeDonnee;

/* --------------------------------
 * T_abeCanal
 * ==========
 * Indicateurs de donnees aberrantes.
 * --------------------------------	*/

typedef struct T_abeCanal
{
  BOOL indic[E_abeTypeMax];
} T_abeCanal;

/* --------------------------------
 * T_abeCanalSeuil
 * ===============
 * Donnees necessaires au traitement 
 * des donnees aberrantes pour un 
 * canal.
 * --------------------------------	*/

typedef struct T_abeCanalSeuil
{
  unsigned short seuil[E_abeTypeMax];
} T_abeCanalSeuil;

/* --------------------------------
 * T_abeStation
 * ============
 * Structure contenant toutes les 
 * informations utiles au traitement
 * des donnees aberrantes par la 
 * station.
 * --------------------------------	*/

typedef struct T_abeStation
{
  unsigned char heureDeb;
  unsigned char heureFin;
  unsigned char seq;
  T_abeCanalSeuil canal[NBCANAL];
} T_abeStation;

/* ********************************
 * VARIABLES GLOBALES DU MODULE
 * ********************************	*/

extern T_abeStation abeStation;
extern T_abeStation *abeStationPt;

/* ********************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ********************************	*/

void abeInit (T_abeStation *);
void abeClear (T_abeStation *);
void abeReset (T_abeStation *);
void abeTester (T_abeStation *);
BOOL abeSetSeuilCanal (T_abeStation *, int, E_abeTypeDonnee, unsigned short);
unsigned short abeGetSeuilCanal (T_abeStation *, int, E_abeTypeDonnee);
BOOL abeSetSeuil (T_abeStation *, E_abeTypeDonnee, unsigned short);
BOOL abeSetIndicCanal (T_abeStation *, int canal,
                       unsigned short, unsigned short, unsigned short);
void abeSetSeq (T_abeStation *, unsigned char seq);
unsigned char abeGetSeq (T_abeStation *);
void abeSetHeureDeb (T_abeStation *, unsigned char heure);
unsigned char abeGetHeureDeb (T_abeStation *);
void abeSetHeureFin (T_abeStation *, unsigned char heure);
unsigned char abeGetHeureFin (T_abeStation *);
void abeSetPeriode (T_abeStation *, ULONG);
ULONG abeGetPeriode (T_abeStation *);
BOOL abeGetValide (T_abeStation *, ULONG, ULONG);

/* ********************************
 * FIN DE abeLib.h
 * ********************************	*/

#endif
