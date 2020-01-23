
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 18/09/01
 * Fichier	: oubLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,18Sep01,xag  creation.
 * ************************************************************************	*/

#ifndef _OUB_LIB_H
#define _OUB_LIB_H

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define OUB_SEUIL_DEF		0
#define OUB_TEMPO_DEF		900
#define OUB_SEQ_DEF   		'H'

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* --------------------------------
 * T_oubCapt
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

typedef struct T_oubCapt
{
  BOOL enCours;
  BOOL indic;
  int nbBloc;
  int nbBlocCour;
} T_oubCapt;

/* --------------------------------
 * T_oubCaptSeuil
 * ===============
 * Donnees necessaires au traitement 
 * des defauts. Il s'agit d'un seuil
 * en pourcentage qui est le rapport
 * entre le nombre de defauts et 
 * le nombre de detections.
 * --------------------------------	*/

typedef struct T_oubCaptSeuil
{
  int tempo;
  int seuil;
} T_oubCaptSeuil;

/* --------------------------------
 * T_oubStation
 * ============
 * Structure contenant toutes les 
 * informations utiles au traitement
 * des donnees oubrrantes par la 
 * station.
 * --------------------------------	*/

typedef struct T_oubStation
{
  unsigned char seq;
  T_oubCaptSeuil capt[NBCAPTEUR];
} T_oubStation;

/* ********************************
 * VARIABLES GLOBALES DU MODULE
 * ********************************	*/

extern T_oubStation oubStation;
extern T_oubStation *oubStationPt;

/* ********************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ********************************	*/

void oubInit (T_oubStation *);
void oubClear (T_oubStation *);
void oubReset (T_oubStation *);
void oubTester (T_oubStation *);
BOOL oubSetSeuilCapt (T_oubStation *, int, int);
int oubGetSeuilCapt (T_oubStation *, int);
BOOL oubSetEnCourCapt (T_oubStation *, int, BOOL);
BOOL oubGetEnCourCapt (T_oubStation *, int);
BOOL oubSetTempoCapt (T_oubStation *, int, int);
int oubGetTempoCapt (T_oubStation *, int);
BOOL oubGetIndicCapt (T_oubStation *, int);
void oubSetSeq (T_oubStation *, unsigned char seq);
unsigned char oubGetSeq (T_oubStation *);
void oubSetPeriode (T_oubStation *, ULONG);
ULONG oubGetPeriode (T_oubStation *);
void oubIncrNbBlocCapt (int);
int oubGetNbBlocCapt (int);

/* ********************************
 * FIN DE oubLib.h
 * ********************************	*/

#endif
