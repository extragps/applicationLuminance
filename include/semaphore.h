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
#ifdef VXWORKS
#include "vxworks.h"
#include "stdio.h"
#include "tasklib.h"
#include "wdLib.h"
#include "semLib.h"
#else
#include "portage.h"
#endif

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

/*******************************************************************/
/*		Prototypes des fonctions du modules                */
/*******************************************************************/
extern SEM_ID SysSemCreer ();
extern void SysSemDetruire (SEM_ID semId);
extern int SysSemPrendre (SEM_ID semId, int timeout);
extern int SysSemDonner (SEM_ID semId);


#endif
