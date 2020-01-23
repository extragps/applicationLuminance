/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES               **/
/**                                                                       **/
/***************************************************************************/
#include "kcommon.h"
#include "cpu432/sram.h"
#include "standard.h"
#include "mon_def.h"
#include "define.h"
#include "vct_str.h"
#include "x01_vcth.h"

/***************************************************************************/
/**                                                                       **/
/*    SEGMENT : FICHIER_SIREDO      CLASS : FICHIER                        */
/**                                                                       **/
/***************************************************************************/

/***************************************************/
/*  declaration de la zone de stockage des donnees */
/***************************************************/

//UINT8 *vct_date_coupure;
Sram *sram_pt=NULL;

/********************************************************/
/*  definition de la table des descripteurs de fichiers */
/********************************************************/
/*****************************************************/
/* nombre de carte MSO DSRAM                         */
/*****************************************************/

T_zdf *sram_bdd;
T_x01_sram_var *sram_var;



UINT8 vct_debut_ramzp;
T_zdf sv_vct_bdd[MAXFIC];

UINT8 vct_num_scenario[6];
UINT8 vct_num_scenario_actif;
UINT vct_chk_scenario[6];

T_vct_usr_lcpi vct_usr_lcpi;
UINT8 vct_fin_ramzp;
DOUBLE vct_bcc_ramzp;
UINT vct_cks_prom;
UINT vct_cks_prom_prom;
UINT8 vct_conf;

/*****************************************************/
/* configuration des alarmes                         */
/*****************************************************/
struct vct_cf_alarme vct_cf_alarme[NB_ALARME];
UINT8 vct_liste_canal_alarme[NBCANAL];
UINT8 vct_liste_det_alarme[NBDET];
UINT8 vct_liste_edf_alarme;

struct vct_st_al vct_st_al;
struct vct_cf_al_sy vct_cf_al_sy;

/************************************************/
/*  declaration des droits d'acces e la station */
/************************************************/
struct zip vct_acces[NBID];
UINT8 vct_nb_acces;

/**********************************/
/* Compteur de scrutation PC	  */
/**********************************/

UINT vct_tempo_spc;

/*************************************************/
/*  definition des descripteurs d'enregistrement */
/*************************************************/

struct sz_enreg vct_enreg[MAXFIC];

/* ********************************
 * VARIABLES CONSTANTES
 * ********************************	*/

const T_ficInfo ficInfo[MAXFIC] = {

  {"IC=", 8, 16},
  {"IM=", 4, 8},
  {"IP=", 12, 20},
  {"IQ=", 4, 8},
  {"IR=", 4, 8},
  {"IA=", 4, 4}
};
