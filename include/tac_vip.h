/********************************************************************************/
/* 		FICHIER: %M%      REL: %I% DATE: %G%                                   */
/*DOC****************************************************************************/
/*  					    					SIAT                                		  */
/*								8, parc technologique de la Sainte Victoire             */
/*													Le Cannet                                */
/*											13590 Meyreuil - FRANCE                        */
/*				  							 Tel. 04 42 58 63 71                           */
/********************************************************************************/
/*  Fichier : Tac_Vip.h                                                         */
/*  MODULE  :                                                                   */
/********************************************************************************/
/* Auteur :  						                                                  */
/* Date de creation : 					                                            */
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
#endif

#include "stdio.h"
#include "string.h"
#include "standard.h"



#ifndef TAC_VIP_H
#define TAC_VIP_H
/*******************************************************************/

GLOBALREF VOID tac_vip (VOID);
GLOBALREF VOID tac_vip_init (VOID);
GLOBALREF VOID tac_vip_init_cfg (VOID);
GLOBALREF UINT8 tac_vip_alerte_video (STRING);
GLOBALREF VOID tac_vip_transfert_donnees (UINT);
GLOBALREF INT tac_vip_dial (INT las, INT mode, INT lg_mess,
                            STRING buffer, INT position,
                            struct usr_ztf *pt_mess
                            /*, INT flg_fin,INT *bloc */ );


#endif
