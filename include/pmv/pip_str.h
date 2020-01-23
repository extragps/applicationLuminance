
#ifndef INCLUDE_STR_PIP
#define INCLUDE_STR_PIP
#include "ficTrace.h"

/* T_pip_clig
 * ==========
 * Definition du clignotement. */

typedef struct pip_clig
{
	UINT8 type;
	UINT val[PIP_NB_VAL_CLIG];
}	T_pip_clig;

/* T_pip_alt
 * =========
 * Definition des parametres d'alternat. */

typedef struct pip_alt
{
	UINT8 type;
	UINT val[PIP_NB_VAL_ALT];
}	T_pip_alt;

/*DOC-------------------------------------------------------------------------
/       /
/ DEFINITION DES STRUCTURES COMMUNES A PLUSIEURS TACHES  /
/       /
-------------------------------------------------------------------------DOC*/
/* typedef structure de description des decors graphiques */
typedef struct pip_cf_dec_graph
{
  BYTE id_dec;                  /* identifiant du decor    */
  UINT nb_carte;                /* nombre de carte constituant le decor  */
  UINT num_carte;               /* numero de la premiere carte  */
} T_pip_cf_dec_graph;

typedef struct pip_cf_carte_graph
{
  BYTE id_carte;
  struct
  {
    LONG pix_coul1;             /* pixel couleur 1 */
    LONG pix_coul2;             /* pixel couleur 2 */
    LONG pix_coul3;             /* pixel couleur 3 */
  } branche[8];
} T_pip_cf_carte_graph;

/****************************************************************************/
/* typedef structures de definition des panneaux alphanumeriques   */
/****************************************************************************/

typedef struct pip_cf_module
{
  UINT id_module;               /* numero de module   */
  BYTE s_type;                  /* TYPE_ALPHA = 1 */
  /* TYPE_PICTO = 2 */
  BYTE type_car;                /* 0 :160 -> 5 : 320 */
  BYTE nb_caisson;              /* nombre de caisson dans le module   */
  BYTE num_caisson;             /* numero du premier caisson du module   */
  BYTE num_sabordage;           /* numero du relais de sabordage  */
  BYTE num_cellule;             /* numero de la cellule    */
  INT spc;
  INT hysteresis_lum;
} T_pip_cf_module;

/****************************************************************************/
/* typedef structures de definition des panneaux alphanumeriques   */
/****************************************************************************/
typedef struct pip_seuil_cellule
{
  UINT lux;                     /* luminosite  */
//  BYTE tens_lampe;              /* tension d'alim lampe  */
  BYTE tens_diode;        /* tension d'alim diode couleur1 */
//  BYTE tens_diode_coul2;        /* tension d'alim diode couleur2 */
//  BYTE tens_diode_coul3;        /* tension d'alim diode couleur3 */
  BYTE cell;                    /* Nuit, Jour, Surbrillance */
} T_pip_seuil_cellule;

/* -------------------------------
 * T_pip_equiv
 * ===========
 * Definition de la valeur d'une
 * equivalence
 * --------------------------------	*/

typedef struct pip_equiv
{
	BYTE typeEquiv;		/* Eclairage ou affichage */
	BYTE typeSymb;		/* Litteral ou numerique */
	UINT valeur;
} T_pip_equiv;

/* -------------------------------
 * T_pip_cf_equiv
 * ==============
 * Table des equivalences symboliques
 * nouvelle version.
 * --------------------------------	*/

typedef struct pip_cf_equiv
{
	UINT module;
	UINT caisson;
	char symbole[LG_SYMB];
	T_pip_equiv equiv;
}	T_pip_cf_equiv;

/* --------------------------------
 * T_pip_cf_litteral
 * =================
 * Litteral associe a un symbole.
 * Pour l'heure, il y a seulement
 * un buffer. Un premier caractere e
 * 0x7F indique que le buffer n'est
 * plus utilise (dans le cas de la
 * suppression d'une equivalence).
 * --------------------------------	*/

typedef struct pip_cf_litteral
{
	char texte[BUFF_MESS_P];
}	T_pip_cf_litteral;

/* -------------------------------
 * T_pip_cf_topo
 * =============
 * Structure ajoutee par XG pour
 * la gestion des equivalences
 * topologiques.
 * --------------------------------	*/

typedef struct pip_cf_topo
{
  INT topoModule;
  INT topoCaisson;
  INT indexCaisson;
} T_pip_cf_topo;

typedef struct pip_cf_caisson
{
  INT indexTopo;                /* Ajoute par XG. index du caisson dans la
								   liste des equivalences topologiques. -1
								   si pas d'equivalence.      */
  UINT id_module;               /* numero de panneau */
  BYTE type;
  BYTE caisson;                 /* numero du caisson   */
  BYTE nb_car;                  /* nombre de caractere du caisson  */
  BYTE num_car;                 /* numero du premier caractere   */
  T_pip_clig clig;
  T_pip_alt  alt;
  BYTE forceLum;                /* pour le forcage de la luminosite */
  UINT sab_def_trans;           /* sabordage en cas de non reponse */
  LONG validite;                /* la duree de validite    */
  char info[MAX_USR_LCPI+1];
} T_pip_cf_caisson;


/* typedef structure de description des caracteres alphanumeriques */
typedef struct pip_cf_car
{
//  BYTE adresse;                 /* adresse du module d'affichage contenant le caractere */
//  BYTE num_caisson;
//  char adresse_sortie;
} T_pip_cf_car;

/*typedef structure de description des textes preprogrammees */
typedef struct pip_cf_texte_prepro
{
  UINT num_car;
  BYTE nb_car;
} T_pip_cf_texte_prepro;

/* typedef structure de description des pictogrammes */
typedef struct pip_cf_bitmap
{
  BYTE colonne[5];              /* matrice   */
} T_pip_cf_bitmap;


/****************************************************************************/
/* typedef structures de definition des panneaux generaux   */
/****************************************************************************/
typedef struct pip_cf_tst_sc
{
  BYTE num_seuil_nuit;          /* numero de seuil nuit   */
  BYTE num_seuil_jour;          /* numero de seuil jour   */
  BYTE num_seuil_surb;          /* numero de seuil jour   */
  UINT diff_sect;               /* tempo max de duree d'une coupure secteur */
  BYTE entretient;
  BYTE tempo_test;
//  BYTE ts_local;                /* numero de la TS pour passage en local */
//  BYTE ts_distant;              /* numero de la TS pour passage en distant */
  struct
  {
	int tempo;
	int seuil;
	int actif;
	} temp;
  UINT autotests;
  INT ts_bp_test;
  INT tc_bp_led;
} T_pip_cf_tst_sc;

/****************************************************************************/
/* typedef structures de description des activation module   */
/****************************************************************************/
typedef struct pip_act_module
{
  BYTE activation;
  INT spc;
  INT hysteresis_lum;
  BYTE cellule;
  BYTE saborde;
  BYTE def_alim;
  BYTE saborde_meca;
  UINT val_cell;
} T_pip_act_module;

typedef struct pip_act_cellule
{
  UINT temp;
  UINT nv_temp;
  UINT nv_etat;
  /* UINT etat; variable d'utilite nulle.*/
  BYTE conf;                    /* si la cellule est utilisee */
  BYTE defChangement;
  BYTE defTrans;
  BYTE num_seuil;               /*0->9 */
  BYTE seuilLum;                /*0,D,J,N,S */
  INT hysteresis_lum;
  UINT periodeControle;
} T_pip_act_cellule;

/****************************************************************************/
/* typedef structures de description des activation caisson   */
/****************************************************************************/
typedef struct pip_lum
{
	int type;
	union {
		char symbole[LG_SYMB];
		BYTE graph;
	}	valeur;
} T_pip_lum;

typedef struct pip_act_caisson
{
  BYTE etat_ok;
  BYTE p_etat;
  BYTE nbAlternance;
  BYTE svNbAlternance;
  INT tp_alt[MAX_ALTERNANCE];
  BYTE extinction;
  BYTE nb_alt;
  T_pip_clig clig;
  T_pip_alt alt;
  T_pip_lum luminosite;
  BYTE cmd_test;
  BYTE tens_diode;        /* intensite de luminosite */
  /* TODO : Supprimer le mode de luminosite. */
  BYTE mode_luminosite;         /* mode de commande lampe 0 = N/S, 1 = 2 lampes com */
  BYTE act_car;
  BYTE act_lum;
  BYTE act_rythme;
  BYTE act_test;
  bool relance_b;
  BYTE demande;
  BYTE mise_au_noir;
  BYTE chg;                     /* signale un changement d'etat caisson */
  BYTE forcage;
  BYTE cpt_relance;
  UINT cpt_attente_relance;
//  UINT etat_aff;
//  UINT sv_etat_aff;
  INT pixel_def;
  BYTE tempo_def_min;
  BYTE cpt_def_min;
  BYTE tempo_def_coupure;
  BYTE carte_sabordee;
  BYTE num_seuil;
  UINT sab_def_trans;           /* sabordage en cas de non reponse */
  LONG validite;				/* Duree de validite courante		*/
  LONG validite_ori;			/* Duree de validite programmee 	*/
  T_ficTraceAdresse ad;
} T_pip_act_caisson;

/****************************************************************************/
/* typedef structures de description des sauvegardes activation caisson   */
/****************************************************************************/
typedef struct pip_action
{
	int type;
	union {
		struct
		{
		int nbcar_txt;
		char txt[BUFF_MESS_P];
		} litteral;
		char symbole[LG_SYMB];
		char graph;
	}	valeur;
} T_pip_action;

typedef struct pip_sv_act_caisson
{
  BYTE act;                     /* pour le type d'activation */
  BYTE cmd_test;
  BYTE nbAlternance;
  BYTE conf ;                    /* pour le type d'activation */
  bool finDv;
  T_pip_action action[MAX_ALTERNANCE];
  T_pip_lum	   luminosite;
  BYTE neutre;
  BYTE extinction;
  T_pip_clig clig;
  T_pip_alt	 alt;
  BYTE forcage;
  INT type_predef[MAX_ALTERNANCE];
  LONG validite;
  LONG validite_ori;
  LONG validite_dem;	/* Pour la relance */
  UINT validite_fin;
  T_ficTraceAdresse ad;
} T_pip_sv_act_caisson;

/* ---------------------------------
 * T_pip_car
 * =========
 * Un caractere affiche peut etre
 * le compose de deux caracteres
 * de la police.
 * Au cas ou le second caractere n'est
 * pas utilise, il doit etre positionne e 0
 * --------------------------------	*/
typedef struct pip_car
{
	BYTE car1;
	BYTE car2;
}	T_pip_car;

typedef struct pip_act_car
{
  T_pip_car cmd[MAX_ALTERNANCE];     /* la commande */
  BYTE def_car;                 /* pour signaler un defaut */
} T_pip_act_car;

typedef struct pip_police
{
  BYTE col[5];
} T_pip_police;

typedef struct pip_type_aff
{
  BYTE num_groupe;
  BYTE type_caisson;
  BYTE type;
  BYTE rang_caisson;
  BYTE nb_alt;
  BYTE tp_on[5];
  BYTE tp_off[5];
  BYTE tens_lampe;
  BYTE tens_diode_coul1;
  BYTE tens_diode_coul2;
  BYTE tens_diode_coul3;
  BYTE mode_luminosite;
  BYTE num_seuil;
  BYTE flagTest;
} T_pip_type_aff;

typedef struct pip_panneau
{
  BYTE rgCaisson;
  BYTE numCaisson;
  BYTE nbCaisson;
  BYTE idPanneau;
} T_pip_panneau;

typedef struct pip_cmd_p
{
  UINT8 flag;
  UINT8 type;
  UINT id_module;               /* le numero de panneau                   */
  int caisson;                /* le numero de caisson                   */
  UINT8 flag_id;
  /* UINT8 id[LG_ID + 1];
  UINT8 pwd[LG_ID + 1]; */
  UINT8 extinction;
  UINT8 neutre;
  UINT8 nb_alt;
  T_pip_action action[MAX_ALTERNANCE];
  T_pip_lum	luminosite;
  UINT8 type_mess_predef[MAX_ALTERNANCE];
  UINT8 type_mess_rang[MAX_ALTERNANCE];
  /* UINT8 decor[MAX_ALTERNANCE];
  UINT8 message[MAX_ALTERNANCE][BUFF_MESS_P]; */

  T_pip_clig clig;
  T_pip_alt	 alt;
  /* Utilisation maintenant de la struture luminosite
   *  UINT8 lum;
  UINT8 lum_int; */
  UINT8 forcage; /* Non utilise pour SIRIUS */
  UINT8 test; /* Non utilise pour SIRIUS */
  UINT8 macro; /* Non utilise pour SIRIUS */
  UINT8 entretient; /* Non utilise pour SIRIUS */
  INT no_las;
  LONG validite;
} T_pip_cmd_p;


/****************************************************************************/
/* structures de reception des messages cfpp                                */
/****************************************************************************/
typedef struct pip_cmd_cfpp
{
  UINT8 flag;
  UINT id_module;               /* le numero de panneau                   */
  UINT8 caisson;                /* le numero de caisson                   */
  UINT8 flag_id;
  INT hysteresis_lum;
  INT sab_def_trans;
  INT spc;
  T_pip_clig clig;
  T_pip_alt	 alt;
  UINT8 princ;
  UINT8 no_las;
  UINT8 forceLum;
  LONG validite;
} T_pip_cmd_cfpp;

#endif
