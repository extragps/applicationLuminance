/********************************************************************************/
/* 		FICHIER: %M%      REL: %I% DATE: %G%                            */
/*DOC****************************************************************************/
/*  					    SIAT                                */
/*			8, parc technologique de la Sainte Victoire             */
/*					Le Cannet                               */
/*				13590 Meyreuil - FRANCE                         */
/*				  Tel. 04 42 58 63 71                           */
/********************************************************************************/
/*  Fichier : WatchDog.h                                                */
/*  MODULE  : Systeme                                                          */
/********************************************************************************/
/* Auteur :   Herve ZEITTOUN                                                    */
/* Date de creation : 12 avril 2000                                             */
/********************************************************************************/
/* OBJET DU MODULE      :                                                       */
/*                                                                              */
/****************************************************************************DOC*/
/* Liste des fonctions du fichier :                                             */
/********************************************************************************/
/*                           Modifications                                      */
/*                      ***********************                                 */
/* Auteur :                                                                     */
/* Date de la modification :                                                    */
/* Description :                                                                */
/*                      ***********************                                 */
/********************************************************************************/

#ifndef TACHE_H
#define TACHE_H

#ifndef LINUX
#include "vxworks.h"
#include "tasklib.h"
#endif
#include "stdio.h"
#include "superviseur.h"


/*******************************************************************/
/*		Prototypes des fonctions du modules                */
/*******************************************************************/
//int SysTacheCreerBis (T_supDescTache *tache,int num,int port);
int tacheCreerBis(T_supDescTache *tache, FUNCPTR entree, char *nom, int port);
void tacheSetId(T_supDescTache *tache, TASK_ID tid);
unsigned long tacheGetId();
unsigned long taskGetId(T_supDescTache *tache);

TASK_ID tacheSpawn(char *name,int priority,int options,int stackSize,FUNCPTR entryPt,
		int numeroTache,int arg2,int arg3,int arg4,int arg5,
		int arg6,int arg7,int arg8,int arg9,int arg10);
STATUS tacheDelete(TASK_ID threadPt);


#endif
