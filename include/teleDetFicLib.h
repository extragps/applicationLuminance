
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 09/08/01
 * Fichier	: teleDetFicLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,02May01,xag  creation.
 * ************************************************************************	*/

#ifndef _TELE_DET_FIC_LIB_H
#define _TELE_DET_FIC_LIB_H



/* ********************************	
 * DEFINITION DES TYPES
 * ********************************	*/

/* -------------------------------
 * E_teleDetFicErreur
 * ================
 * --------------------------------	*/

typedef enum E_teleDetFicErreur
{
  E_teleDetFicErrOk,
  E_teleDetFicErrAbs,
  E_teleDetFicErrEff,
  E_teleDetFicErrProg,
  E_teleDetFicErrVal,
  E_teleDetFicErrChk,
  E_teleDetFicErrChk2,
  E_teleDetFicErrChk3,
  E_teleDetFicErrId,
  E_teleDetFicMaxErreur
} E_teleDetFicErreur;


/* -------------------------------
 * E_teleDetFicEtat
 * ================
 * Liste des etats possibles pour
 * le telechargement des fichiers
 * detecteur.
 * --------------------------------	*/

typedef enum E_teleDetFicEtat
{
  E_teleDetFicInitialise,
  E_teleDetFicChargementEnCours,
  E_teleDetFicChargementTermine,
  E_teleDetFicLancer,
  E_teleDetFicVersion,
  E_teleDetFicLectureVersion,
  E_teleDetFicIdentification,
  E_teleDetFicEffacement,
  E_teleDetFicTelechargement,
  E_teleDetFicVerification,
  E_teleDetFicValidation,
  E_teleDetFicOk,
  E_teleDetFicInconnu,
  E_teleDetFicNok,
  E_teleDetFicMaxEtat
} E_teleDetFicEtat;


/* ----------------------------
 * T_teleDetFic
 * ==========
 * Le fichier contient les infos
 * necessaire au telechargement d'un
 * fichier.
 * --------------------------------	*/

typedef struct T_teleDetFic
{
  int blocCour;
  int nbBlocs;
  int nbRetry;
  int ficSel;
  char **enreg;
  int nbEnregMax;
  E_teleDetFicEtat etat;
  E_teleDetFicErreur erreur;
  int numDet;
  int mineure;
  int majeure;
  int force;
} T_teleDetFic;

/* ********************************	
 * VARIABLES GLOBALES DU MODULE
 * ********************************	*/

extern T_teleDetFic *teleDetFicPt;

/* ********************************	
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ********************************	*/

T_teleDetFic *teleDetFicCreer (void);
T_teleDetFic *teleDetFicGet (void);
void teleDetFicDetruire (T_teleDetFic *);
void teleDetFicSetNbBlocs (T_teleDetFic *, int);
int teleDetFicGetNbBlocs (T_teleDetFic *);
int teleDetFicGetBlocCour (T_teleDetFic *);
int teleDetFicSetBlocCour (T_teleDetFic *, int);
int teleDetFicSetEnreg (T_teleDetFic *, int, char *);
char *teleDetFicGetEnreg (T_teleDetFic *, int);
E_teleDetFicEtat teleDetFicGetEtat (T_teleDetFic *);
void teleDetFicSetEtat (T_teleDetFic *, E_teleDetFicEtat);
E_teleDetFicErreur teleDetFicGetErreur (T_teleDetFic *);
void teleDetFicSetErreur (T_teleDetFic *, E_teleDetFicErreur);
int teleDetFicGetNumDet (T_teleDetFic *);
void teleDetFicSetNumDet (T_teleDetFic *, int);
int teleDetFicGetMineure (T_teleDetFic *);
void teleDetFicSetMineure (T_teleDetFic *, int);
int teleDetFicGetMajeure (T_teleDetFic *);
void teleDetFicSetMajeure (T_teleDetFic *, int);

#endif
