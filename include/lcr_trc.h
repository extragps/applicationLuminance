/****************************************************************************/
/* 		FICHIER: %M%      REL: %I% DATE: %G%                                */
/*DOC************************************************************************/
/*  					    					SIAT                		*/
/*								8, parc technologique de la Sainte Victoire */
/*													Le Cannet               */
/*											13590 Meyreuil - FRANCE         */
/*				  							 Tel. 04 42 58 63 71            */
/****************************************************************************/
/*  Fichier : Lcr_trc.h                                                     */
/*  MODULE  :                                                               */
/****************************************************************************/
/* Auteur :  						                                        */
/* Date de creation : 					                                    */
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

#ifndef LCR_TRC_H
#define LCR_TRC_H
/*******************************************************************/
#include <stdio.h>
#include <string.h>
#include "standard.h"

void cmd_trc_mat(INT trace, STRING format, ...);
VOID lcr_trace_tr(INT, INT, INT, STRING, INT, struct usr_ztf *);
VOID lcr_trace_m(INT, INT, INT, STRING, INT, struct usr_ztf *);
VOID cmd_trc_tr(char *, ...);
VOID cmd_trc_int108(INT, STRING, INT);
VOID lcr_trace_silhouette(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess);
VOID cmd_trc_silhouette(INT k);

#endif
