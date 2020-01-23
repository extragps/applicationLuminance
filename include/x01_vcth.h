/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES                */
/**                                                                       **/
/***************************************************************************/

/***************************************************************************/
/**                                                                       **/
/***************************************************/
#include "define.h"
#ifdef LINUX
#include "kcommon.h"
#include "cpu432/sram.h"
#endif
#include "x01_str.h"


#ifndef X01_VCT_H
#define X01_VCT_H

                                                                                /* Pointeurs sur des zones de
                                                                                 * memoire sauvegardee.                         */
//extern UINT8 *vct_date_coupure;
//#ifdef LINUX
//extern Sram *sram_pt;
//extern T_x01_sram_var *sram_var;
//#endif
//extern UINT8 *sram_mem;
//extern long vct_mem_size;

/***************************************************************************/
/**                                                                       **/
/*    SEGMENT : PARAMETRE_SIREDO    CLASS : RAMZP                          */
/**                                                                       **/
/***************************************************************************/

extern UINT8 vct_debut_ramzp;

extern UINT8 vct_num_scenario[];
extern UINT vct_chk_scenario[];
extern UINT8 vct_num_scenario_actif;
extern UINT8 vct_nb_mso;
extern UINT8 vct_nb_com185;
/* L'initialisation du pointeur
 * est realisee dans rec_main_init      */
//extern T_zdf *sram_bdd;

/********************************************************/
/*  definition de la table des descripteurs de fichiers */
/********************************************************/

/************************************************/
/*      periode de lecture des compteurs        */
/************************************************/
extern LONG vct_perco;
extern LONG vct_perco_ms;

/************************************************/
/*  configuration de l'option poids             */
/************************************************/
extern UINT8 vct_poids;

extern UINT8 vct_algo_sens;     /* rejection de sens sur longueur */

/************************************************/
/*  declaration des droits d'acces e la station */
/************************************************/
extern struct zip vct_acces[NBID];
extern UINT8 vct_nb_acces;



/*************************************************/
/*  definition des descripteurs d'enregistrement */
/*************************************************/
extern struct sz_enreg vct_enreg[MAXFIC];
extern struct vct_hmvl vct_hmvl;

/********************************************************/
/* valeur du sequencement variable configure            */
/********************************************************/
extern ULONG vct_Vseq;

/********************************************************/
/* defini la periode utilisateur                        */
/********************************************************/
extern UINT8 vct_per_util;


/*****************************************************/
/* configuration des alertes                         */
/*****************************************************/
extern struct vct_cf_al_my vct_cf_al_my[NBCANAL];
extern struct vct_cf_al_sy vct_cf_al_sy;
extern struct vct_st_al vct_st_al;

extern struct vct_alerte_tr vct_alerte_tr;


/*****************************************************/
/* nombre de carte MDB185 COM185                     */
/*****************************************************/
extern UINT8 vct_nb_mdb185;
extern UINT8 vct_nb_com185_conf;

extern UINT8 vct_nb_com185;

/*****************************************************/
/* configuration des alarmes                         */
/*****************************************************/
extern struct vct_cf_alarme vct_cf_alarme[NB_ALARME];
extern UINT8 vct_liste_canal_alarme[NBCANAL];
extern UINT8 vct_liste_det_alarme[NBDET];
extern UINT8 vct_liste_edf_alarme;

/* ********************************
 * COMPTEUR DE SCRUTATION PC
 * ********************************	*/

extern UINT vct_tempo_spc;

/*****************************************************/
/* VIP                                               */
/*****************************************************/

extern UINT8 vct_fin_ramzp;
extern DOUBLE vct_bcc_ramzp;
extern UINT vct_cks_prom;
extern UINT vct_cks_prom_prom;
extern UINT8 vct_conf;


/***************************************************************************/
/**                                                                       **/
/*    SEGMENT : VARIABLE_SIREDO    CLASS : VARIABLE                        */
/**                                                                       **/
/***************************************************************************/


/******************************************************/
/* indicateur de reception d'un joker adresse         */
/******************************************************/
extern UINT8 flag_joker[NBPORT];

/******************************************************/
/* status temps reel de la station                    */
/******************************************************/
extern UINT8 vct_ST_TR;
extern UINT8 nv_vct_ST_TR;

/******************************************************/
/* indicateur d'echo active                           */
/******************************************************/
extern UINT8 vct_echo[NBPORT];

/***********************************************************************/
/* indicateur d'adaptation de l'equipement a un minitel en videotext   */
/***********************************************************************/
extern UINT8 vct_mtel[NBPORT];

/******************************************************/
/* type de caractere echoe pour un CR recu            */
/******************************************************/
extern UINT8 vct_cr[NBPORT];    /* CR ou CR LF */

/***********************************************************************/
/* nombre de caracteres d'une reponse avant insertion d'un LF CR force */
/***********************************************************************/
extern UINT8 vct_lpl[NBPORT];


/************************************************/
/*  declaration des numeros de periodes         */
/************************************************/
extern ULONG vct_no_periode[MAXFIC];

extern ULONG vct_date[MAXFIC];

/************************************************/
/*  declaration de la fifo vilt alerte          */
/************************************************/
extern struct vct_fifo_vilt vct_fifo_alerte;
/************************************************/
/* declaration de la table d'autorisation alerte*/
/************************************************/
extern UINT8 autoriseAL[NBCAPTEUR];
/************************************************/
/* declaration flag et duree alerte vit indiv   */
/************************************************/
extern UINT8 vct_duree_al_mso[NBCANAL];
extern INT vct_mso[NB_CARTE];
extern INT vct_mgp102_in[NB_CARTE];
extern INT vct_mgp102_out[NB_CARTE];
extern INT vct_int108_in;
extern INT vct_int108_out;


/************************************************/
/*  declaration des variables intermediaires    */
/************************************************/
extern struct qtv qtv;

/************************************************/
/*  flag d'emission vers la fifo debug          */
/************************************************/
extern UINT8 vct_emission_debug;

/********************************************************/
/* indicateur de validite de vt sur un las              */
/********************************************************/
extern UINT8 vct_vt[NBPORT];

/********************************************************/
/* indicateur de reception d'une cde d'interruption     */
/********************************************************/
extern UINT8 vct_IT[NBPORT];
extern UINT8 vct_flag_IT[NBPORT];

/***************************************************/
/* definition de l'etat d'une commande RD          */
/***************************************************/
extern struct zed vct_rd;

/*********************************************************************/
/* detection des conditions d'alerte                                 */
/*********************************************************************/
extern struct vct_det_alerte vct_det_alerte;

/*********************************************************************/
/* flag pour acces a la base                                         */
/*********************************************************************/
extern struct z_acces_bdd vct_acces_bdd[NBPORT];

/*******************************************************************/
/*  declaration de la structure de changement de dater differee    */
/*******************************************************************/
extern struct z_horodate vct_horodate;

/*********************************************************************/
/* pour la recherche des cartes presentes                            */
/*********************************************************************/

extern UINT8 vct_debut_ram_carte;
extern UINT8 vct_type_init;
extern UINT8 vct_phase_lecture;
extern UINT8 vct_num_carte;
extern UINT8 vct_pres_dsram[NB_CARTE];
extern UINT8 vct_pres_com185[NBCOM185];
extern UINT8 vct_pres_mso[NB_CARTE];
extern UINT8 vct_pres_mgp102[NB_CARTE];
extern UINT8 vct_nb_mgp102;
extern UINT8 vct_pres_msr_simul;
extern UINT8 vct_pres_int108;
extern UINT8 vct_fin_ram_carte;
extern INT vct_bcc_ram_carte;

/**********************************/
/* trace silhouette               */
/**********************************/
extern struct vct_trace_silhouette vct_trace_silhouette;

/* Compteur temporaire */
extern long biri_cpt;
extern struct vct_trace_maint vct_trace_maint;
extern struct vct_trace_ai vct_trace_ai;

extern UINT8 vct_phase_com185[NBCOM185];

extern T_vct_usr_lcpi vct_usr_lcpi;

#endif
