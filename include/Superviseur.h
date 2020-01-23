/****************************************************************************/
/* 		FICHIER: %M%      REL: %I% DATE: %G%                            	*/
/*DOC************************************************************************/
/*  					    SIAT                                			*/
/*			8, parc technologique de la Sainte Victoire             		*/
/*					Le Cannet                               				*/
/*				13590 Meyreuil - FRANCE                         			*/
/*				  Tel. 04 42 58 63 71                           			*/
/****************************************************************************/
/*  Fichier : Superviseur.h                                                 */
/*  MODULE  : Superviseur                                                   */
/****************************************************************************/
/* Auteur :   Herve ZEITTOUN                                                */
/* Date de creation : 12 avril 2000                                         */
/****************************************************************************/
/* OBJET DU MODULE      :                                                   */
/*                                                                          */
/************************************************************************DOC*/
/* Liste des fonctions du fichier :                                         */
/****************************************************************************/
/*                           Modifications                                  */
/*                      ***********************                             */
/* Auteur :                                                                 */
/* Date de la modification :                                                */
/* Description :                                                            */
/*                      ***********************                             */
/****************************************************************************/

/* ********************************	*/
/* FICHIERS D'INCLUDE				*/
/* ********************************	*/

#include <sys/select.h>
#include "signal.h"
#include "unistd.h"
#include "standard.h"
#include "mon_ext.h"
#include "stdio.h"
#include "string.h"
#include "standard.h"                  /* redefinition des types                         */
#include "Tache.h"
#include "MQ.h"
#include "mon_ext.h"
#include "Semaphore.h"

#ifndef SUPERVISEUR_H
#define SUPERVISEUR_H

/*******************************************************************/
/*     Structure de gestion du logiciel                            */
/*******************************************************************/

/* ********************************	*/
/* DECLARATION DES CONSTANTES		*/
/* ********************************	*/

#define SUP_NOM_TACHE	30
#define SUP_MAX_TACHE	25
#define SUP_ETAT_INIT 7
#define SUP_ETAT_RUN 17
#define INIT			SUP_ETAT_INIT   /* Pour compatibilite                           */
#define RUN 			SUP_ETAT_RUN


/* ********************************	*/
/* DECLARATION DES TYPES			*/
/* ********************************	*/

/* --------------------------------	*/
/* T_supDescTache					*/
/* ==============					*/
/* Description d'une tache avec 	*/
/* son nom, la fonction appelee et  */
/* quelques autres informations dont*/
/* le semaphore utilise pour le 	*/
/* cadencement.						*/
/* --------------------------------	*/

typedef struct
{
  char nomTache[SUP_NOM_TACHE];
  FUNCPTR ptEntree;
  EVENT_ID tacVersMon;
  EVENT_ID monVersTac;
  TASK_ID tid;
  int bidon;
  int position;
  int sequence;
  int active;
  int numero;
  int32 lastTick_dw;
} T_supDescTache;

typedef T_supDescTache DescriptionTache;        /* Pour compatibilite                   */

/* --------------------------------	*/
/* T_supGestion						*/
/* ============						*/
/* Structure contenant les infos sur*/
/* les taches du systeme.			*/
/* --------------------------------	*/

typedef struct
{
  int appli;
  int nbTache;
  T_supDescTache Tache[SUP_MAX_TACHE];
} T_supGestion;

typedef T_supGestion SupGestion;        /* Pour compatibilite                   */



/* ********************************	*/
/* VARIABLES GLOBALES DU MODULE     */
/* ********************************	*/

extern T_supGestion supGestion;
extern SEM_ID etaSystSem;

/*******************************************************************/
/*		Prototypes des fonctions du modules                */
/*******************************************************************/

int supInit (T_supGestion * pGes, int,bool);
int supRun (T_supGestion * pGes, int);
void supInitTaskId(int numTache);
void supInitThreadId(int numTache, TASK_ID threadPt);


/* *****************************************************************
 * Fonction d'initialisation des taches à creer.
 * *****************************************************************/

void DialogueLuminanceInit(void);

void lumTraiteur_init(void);
void lumTraiteur_term(void);

void sequenceur_init(void);
void sequenceur_term(void);

/*******************************************************************/
/*		Prototypes des taches creer                        */
/*******************************************************************/

void *supInitGetConnLuminance(void);

VOID tac_las (int);
VOID rec_main (int);
VOID rec_acq (int);
FUNCPTR DialogueLCR (int,int);
FUNCPTR DialogueTPE (int,int);
FUNCPTR DialogueModem (int);
FUNCPTR DialogueDet (int);
FUNCPTR DialogueFip (int);
FUNCPTR DialoguePass (int);
FUNCPTR DialogueLuminance(int,int);

void sequenceur_main(int);
void lumTraiteur_main(int);

void etaSyst (T_supGestion * pGes);
void supIncrement (int);

#endif
