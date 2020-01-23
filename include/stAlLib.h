
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 14 Juillet 2001
 * Fichier	: stAlLib.h
 * Objet	: Structures utilise dans le cadre des mecanismes d'alerte.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,14Jui01, xag Creation du fichier.
 * ************************************************************************	*/

#ifndef _ST_AL_LIB_H
#define _ST_AL_LIB_H

/* ********************************
 * PROTOTYPE DES TYPES
 * ********************************	*/

/* --------------------------------
 * E_stAlEtat
 * ==========
 * Differents etats de l'automate du
 * ST AL.
 * -------------------------------- */

typedef enum E_stAlEtat
{
  E_stAlLibre,
  E_stAlLancer,
  E_stAlAttente,
  E_stAlAcquit
} E_stAlEtat;

/* --------------------------------
 * E_stAlNeutre
 * ==========
 * Neutralisation du mecanisme d'alerte
 * -------------------------------- */

typedef enum E_stAlNeutre
{
  E_stAlNeutralise,
  E_stAlDisponible
} E_stAlNeutre;

/* --------------------------------
 * T_stAlData
 * ==========
 * structure devant contenir les info
 * pour le traitement des alertes.
 * Les champs sont :
 * - etat courant du traitement d'alerte
 * - numero du message lance ou e lancer,
 * - compteur d'attente avant lancement,
 * - compteur d'attente d'acquitement,:q
 * - compteur de repetition,
 * - etat de neutralisation,
 * - compteur de neutralisation,
 * - aboutissement, si a vrai en
 *   fin de sequence, on positionne
 *   le bit du status temps reel.
 * --------------------------------	*/

typedef struct T_stAlData
{
  int port;
  E_stAlNeutre neutre;
  int cptNeutre;
  E_stAlEtat etat;
  int numMess;
  int cptAttente;
  int cptAcquit;
  int numRepet;
  int abouti;
  int test;
  int cptTest;
  int testEnCours;
} T_stAlData;

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

void stAlInit (void);
void stAlMessageSuivant (T_stAlData *);
void stAlTester (int);
int stAlLancer (void);
int stAlForcer (void);
int stAlAcquitFip (int, char *, int);
void stAlDemarrerTest (void);
void stAlDecompter (void);

/* Ajout des prototypes de trt_alt.c pour eviter de faire un nouveau fichier */
void trt_alt (void);
int trt_alt_tester_alerte (void);

/* ********************************
 * FIN DE stAlLib.h
 * ********************************	*/
#endif
