/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES               **/
/**                                                                       **/
/***************************************************************************/
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "pip_def.h"
#include "pip_str.h"
#include "x01_str.h"
#include "x01_var.h"
#include "pip_var.h"
#include "configuration.h"

/* configuration des modules alphanumeriques  */
UINT pip_nb_module;
T_pip_cf_module pip_cf_module[NB_MODULE];

/* configuration des caissons alphanumeriques */
UINT pip_nb_caisson;
T_pip_cf_caisson pip_cf_caisson[NB_CAISSON];

/* configuration des equivalences symboliques */
UINT pip_nb_litteral;
T_pip_cf_litteral pip_cf_litteral[NB_LITTERAL];

/* configuration des equivalences symboliques */
UINT pip_nb_equiv;
T_pip_cf_equiv pip_cf_equiv[NB_EQUIV];

/* configuration des equivalences topologiques */
UINT pip_nb_topo;
T_pip_cf_topo pip_cf_topo[NB_CAISSON];

/* configuration des cellules */
UINT pip_nb_seuil_cellule;
T_pip_seuil_cellule pip_cf_seuil_cellule[NB_SEUIL_CELLULE];

///* configuration des caracteres alpha */
//UINT pip_nb_car;
//T_pip_cf_car pip_cf_car[NB_CAR_ALPHA];

/* configuration des textes preprogrammes */
UINT pip_nb_car_prepro;
UINT pip_cf_car_prepro[NB_CAR_PREPRO];



/* configuration des parametres generaux */
T_pip_cf_tst_sc pip_cf_tst_sc;
//
///* configuration des polices de caracteres */
//
//T_pip_police pip_police[MAX_CAR_POLICE];

/***************************************************************************/
/* parametres de controle                                                  */
/***************************************************************************/

T_pip_act_module pip_act_module[NB_MODULE];

T_pip_act_cellule pip_act_cellule[NB_LT501];

T_pip_act_caisson pip_act_caisson[NB_CAISSON];

//T_pip_sv_act_caisson sv_act_caisson[NB_CAISSON];

T_pip_act_car pip_act_car[AFFICHEUR_NB_MAX];

T_pip_type_aff pip_type_aff[NB_CAISSON];

T_pip_sv_act_caisson pip_sv_sv_act_caisson[NB_CAISSON];

//T_pip_sv_act_caisson *pip_sv_act_caisson;
//ULONG *pip_sv_act_caisson_bcc;

BYTE pip_nb_panneau;
BYTE pip_nb_caisson_liste;
T_pip_panneau pip_panneau[NB_PANNEAU];
BYTE pip_liste_caisson[NB_CAISSON];
