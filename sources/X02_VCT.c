/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES               **/
/**                                                                       **/
/***************************************************************************/
/*
 * X02_VCT.c
 * ===================
 *
 *  Created on: 1 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: X02_VCT.c,v $
 * Revision 1.2  2018/06/04 08:38:39  xg
 * Passage en version V122
 *
 * Revision 1.3  2010/08/05 14:38:18  xgaillard
 * *** empty log message ***
 *
 * Revision 1.2  2008/07/04 17:05:06  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include "standard.h"                  /* redefinition des types                         */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"
/***************************************************************************/
/**                                                                       **/
/*    SEGMENT : VARIABLE_SIREDO    CLASS : VARIABLE                        */
/**                                                                       **/
/***************************************************************************/

/******************************************************/
/* indicateur de reception d'un joker adresse         */
/******************************************************/
UINT8 flag_joker[NBPORT];

/******************************************************/
/* status temps reel de la station                    */
/******************************************************/
UINT8 vct_ST_TR;
UINT8 nv_vct_ST_TR;


/******************************************************/
/* indicateur d'echo active                           */
/******************************************************/
UINT8 vct_echo[NBPORT];

/******************************************************/
/* type de caractere echoe pour un CR recu            */
/******************************************************/
UINT8 vct_cr[NBPORT];           /* CR ou CR LF */

/***********************************************************************/
/* nombre de caracteres d'une reponse avant insertion d'un LF CR force */
/***********************************************************************/
UINT8 vct_lpl[NBPORT];


/************************************************/
/*  declaration des numeros de periodes         */
/************************************************/
ULONG vct_no_periode[MAXFIC];

ULONG vct_date[MAXFIC];

/************************************************/
/* declaration de la table d'autorisation alerte*/
/************************************************/
UINT8 autoriseAL[NBCAPTEUR];

/************************************************/
/* declaration flag et duree alerte vit indiv   */
/************************************************/
UINT8 vct_duree_al_mso[NBCANAL];
INT vct_mso[NB_CARTE];
INT vct_mgp102_in[NB_CARTE];
INT vct_mgp102_out[NB_CARTE];
INT vct_int108_in;
INT vct_int108_out;

/************************************************/
/*  flag d'emission vers la fifo debug          */
/************************************************/
UINT8 vct_emission_debug;

/********************************************************/
/* indicateur de validite de vt sur un las              */
/********************************************************/
UINT8 vct_vt[NBPORT];

/********************************************************/
/* indicateur de reception d'une cde d'interruption     */
/********************************************************/
UINT8 vct_IT[NBPORT];

UINT8 vct_flag_IT[NBPORT];

/*********************************************************************/
/* detection des conditions d'alerte                                 */
/*********************************************************************/
struct vct_alerte_tr vct_alerte_tr;

/*********************************************************************/
/* flag pour acces a la base                                         */
/*********************************************************************/
struct z_acces_bdd vct_acces_bdd[NBPORT];

/*******************************************************************/
/*  declaration de la structure de changement de date differee    */
/*******************************************************************/
struct z_horodate vct_horodate;

/*********************************************************************/
/* pour la recherche des cartes presentes                            */
/*********************************************************************/
UINT8 vct_debut_ram_carte;
UINT8 vct_type_init;
UINT8 vct_phase_lecture;
UINT8 vct_num_carte;
UINT8 vct_pres_com185[NBCOM185];
UINT8 vct_pres_mso[NB_CARTE];
UINT8 vct_pres_mgp102[NB_CARTE];
UINT8 vct_nb_mgp102;
UINT8 vct_nb_com185;
UINT8 vct_nb_int108;
UINT8 vct_nb_msrsimul;
UINT8 vct_nb_mso;
UINT8 vct_pres_int108;
UINT8 vct_pres_msr_simul;
UINT8 vct_fin_ram_carte;
INT vct_bcc_ram_carte;

UINT8 vct_phase_com185[NBCOM185];
UINT8 vct_phase_int108;
UINT8 vct_reinit_int108;


/* Compteur temporaire */
long biri_cpt;

#if STATION
/*#option sep_off*/
#endif


#if STATION
/*#option sep_on segment x02_vct_prom x02_vct_prom class x02_vct_prom x02_vct_prom*/
#endif

#if STATION
/*#option sep_off*/
#endif
