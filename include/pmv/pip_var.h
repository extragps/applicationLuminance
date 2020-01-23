#ifndef _PIP_VAR_H
#define _PIP_VAR_H
/* configuration des modules alphanumeriques  */
extern UINT pip_nb_module;
extern struct pip_cf_module pip_cf_module[];


/* configuration des caissons alphanumeriques */
extern UINT pip_nb_caisson;
extern struct pip_cf_caisson pip_cf_caisson[];

extern UINT pip_nb_equiv;
extern T_pip_cf_equiv pip_cf_equiv[];

extern UINT pip_nb_litteral;
extern T_pip_cf_litteral pip_cf_litteral[];

extern UINT pip_nb_topo;
extern T_pip_cf_topo pip_cf_topo[];

/* configuration des cellules */
extern UINT pip_nb_seuil_cellule;
extern struct pip_seuil_cellule pip_cf_seuil_cellule[];
//
///* configuration des caracteres alpha */
//extern UINT pip_nb_car;
//extern T_pip_cf_car pip_cf_car[];

/* configuration des textes preprogrammes */
extern UINT pip_nb_car_prepro;
extern UINT pip_cf_car_prepro[];

extern UINT pip_nb_bitmap;
extern struct pip_cf_bitmap pip_cf_bitmap[];

/* configuration des parametres generaux */
extern struct pip_cf_tst_sc pip_cf_tst_sc;

/* configuration des feux tricolore */
extern struct pip_cf_tst_sc_ft pip_cf_tst_sc_ft;

/* configuration des feux tricolore */
extern struct pip_cf_tst_sc_cc pip_cf_tst_sc_cc;

extern struct pip_cf_tst_sc_frc pip_cf_tst_sc_frc;

/* configuration des polices de caracteres */
extern T_pip_police pip_police[256];

/* configuration des seuil de luminosite par defaut */
extern struct pip_seuil_cellule pip_seuil_cellule[];
//
///* configuration des decors graphiques */
//extern UINT pip_nb_dec_graph;
//extern struct pip_cf_dec_graph pip_cf_dec_graph[];
//
///* configuration des cartes graphiques (par decor) */
//extern UINT pip_nb_carte_graph;
//extern struct pip_cf_carte_graph pip_cf_carte_graph[];



extern struct pip_act_module pip_act_module[];
extern struct pip_act_cellule pip_act_cellule[];
extern struct pip_act_caisson pip_act_caisson[];
extern struct pip_sv_act_caisson sv_act_caisson[];

extern struct pip_act_car pip_act_car[];
//extern struct pip_type_aff pip_type_aff[];
//extern BYTE pip_decor_dispo[];
//extern UINT pip_prisme_activation[];
//extern UINT pip_prisme_activation_erreur[];


extern struct pip_sv_act_caisson pip_sv_sv_act_caisson[];

extern BYTE pip_nb_panneau;
extern BYTE pip_nb_caisson_liste;
extern struct pip_panneau pip_panneau[];
extern BYTE pip_liste_caisson[];

#endif
