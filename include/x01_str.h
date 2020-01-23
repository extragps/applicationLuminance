/****************************************************************************/
/*									    */
/* DEFINITION DES STRUCTURES UTILISEES PAR PLUSIEURS TACHES                 */
/*									    */
/****************************************************************************/

#ifndef X01_STR_H
#define X01_STR_H

#include "xdg_def.h"
#include "eriLib.h"
#ifdef PMV
#include "pip_def.h"
#include "pip_str.h"
#endif

/* ********************************
 * DEFINITIONS
 * ********************************	*/

#define CFTP_NB_AD	10

/**************************************************/
/*   structure de configuration des ports logiques*/
/**************************************************/
struct x01_set
{

  UINT8 echo;                   /* si echo                         */
  UINT8 cr;                     /* si cr                           */
  UINT8 lpl;                    /* si replie                       */
  UINT8 mtel;                   /* si minitel                      */
};

/*************************************************/
/* definition des parametres de status general   */
/*************************************************/

struct x01_status1
{

  char adr[4];                 /* adresse de la station                           */
  char cod[8];                 /* code equipement                                 */
  char loc[15];                /* localisation de la station                      */
  char nst[4];                 /* numero de serie de la station                   */
  char ver[4];                 /* version de la station                           */
  char gen[8];                 /* generation de l'equipement                      */
  char bat[3];                 /* nombre d'heures definies avant mode protection  */
  char eol[4];                 /* car de fin de reponse pour cde X du LCR         */
  UINT8 local;
  UINT8 x01_ST_TR;
  UINT8 err[2];
};
struct x01_status3
{
  UINT8 edf;                    /* presence secteur 220 V                          */
  UINT8 grp;                    /* presence groupe                                 */
  UINT8 jn;                     /* jour/nuit                                       */
  UINT8 srb;                    /* surbrillance                                    */
  UINT8 trm;                    /* presence d'un terminal portable sur I2          */
  UINT8 err;                    /* code interne d'erreurs                          */
  UINT8 er1;                    /* code erreur port 1                              */
  UINT8 er2;                    /* code erreur port 2                              */
  UINT8 er3;                    /* code erreur port 3                              */
  struct
  {

    char date[SZ_DATE];        /* date du dernier evenement                       */
    char nom[3];               /* nom du dernier evenement s'etant produit        */

  } evt;
  INT tempo_mode_terminal[NBPORT];
  UINT taille_config;
  BYTE erreur_config;
  BYTE rebouclage;
  BYTE strapRebouclage;
  BYTE strapForcageInput;
  BYTE cmdLum;                  /* pour commande du relais jour/nuit (TUMA) */
  BYTE etatCmdJourNuit;         /* pour l'etat du relais jour/nuit  (TUMA) */
  BYTE tr[2];
  BYTE eri;
  BYTE eri_rebouclage;
  BYTE distant;
  char mod_rebouclage[50];
  BYTE verrouillage;
  BYTE ver[4];                  /* version de la station                           */
  BYTE gen[8];                  /* generation de l'equipement                      */
};

/************************************************/
/*  pour les erreurs                            */
/************************************************/
struct x01_cptr
{
  UINT8 erreur;
  UINT8 num_par;
  UINT8 flag;
  UINT8 demarrage;
  UINT8 configuration;
  UINT8 config;
  UINT8 suspension;
  UINT8 mise_au_neutre;
  INT tempo_horodate;
  UINT8 config_dec;
#ifdef PMV
  BYTE activation_secteur;
  BYTE surveillance;
  INT relance;
#endif
};
/************************************************/
/*   structure des identifiants et mots de passe*/
/************************************************/
struct x01_cfid
{
  INT num_id;
  UINT8 idf[LG_ID];             /* nom de l'identifiant                            */
  UINT8 pwd[LG_ID];             /* mot de passe lie e l'identifiant                */
};

#ifdef PMV


/**************************************************/
/*  configuration des macro commande et scenarios */
/**************************************************/
typedef struct x01_cfmp
{
  UINT8 nom[LG_SYMB + 1];       /* nom de la macro             */
  UINT8 ligne;                  /* numero de ligne             */
  UINT8 module;                 /* le numero de panneau        */
  UINT8 caisson;                /* le numero de caisson        */
  UINT8 nb_alt;
  UINT8 decor[MAX_ALTERNANCE];
  UINT8 message[MAX_ALTERNANCE][BUFF_MESS_P];
  UINT8 nb_clig;
  UINT8 clig[MAX_ALTERNANCE];
  UINT8 lum;
  UINT8 lum_int;
  UINT8 forcage;
  LONG validite;
  INT tp;                       /* temporisation de l'action de P */
  UINT8 dg_macro[LG_SYMB + 1];  /* mode degrade vers la macro m   */
  UINT8 dg_ligne[LG_SYMB + 1];  /* mode degrade vers la ligne l   */
  struct
  {
    UINT8 x;
    UINT8 y;
    UINT8 p;
  } ck;                         /* verification condition p en entree x,y */
} T_x01_cfmp;

/****************************************************************************/
/* structures de reception des messages cftp                                */
/****************************************************************************/
typedef struct cmd_cftp
{
  UINT8 flag;
  UINT id_module;               /* le numero de panneau                   */
  UINT8 caisson;                /* le numero de caisson                   */
  char info [MAX_USR_LCPI+1];  /* Information sur le caisson.	*/
  UINT8 type;  /* Ligne alphanumerique ou module de delestage */
  UINT8 type_car;
  UINT8 nb_afficheur;	/* Nombre de cartes dans le module. */
  UINT8 nb_es;
  UINT8 es[100];
  UINT8 nb_ad;
  UINT8 ad[CFTP_NB_AD];         /* pour les panneaux alphanumerique
								 * 0 : le numero du relais de sabordage
								 * 1 : le numero de la cellule jour/nuit
								 * 2 : le numero de la cellule de surbrillance
								 * 4 : le num du relais de sabordage statique */
  UINT8 forceLum;               /* pour le forcage de la luminosite */
} T_cmd_cftp;


/****************************************************************************/
/* structures de reception des messages cfs                                */
/****************************************************************************/
typedef struct cmd_cfs
{
  UINT8 flag;
  UINT8 nb_cel;
  UINT cellule[NB_SEUIL_CELLULE * 10];
} T_cmd_cfs;

/****************************************************************************/
/* structures de reception des messages cfpl                                */
/****************************************************************************/
struct cmd_cfpl
{
  UINT8 flag;
  UINT8 nb_car;
  uint8 car[50];
};

/****************************************************************************/
/* structures de reception des messages tst                                 */
/****************************************************************************/
struct cmd_tst
{
  UINT8 cmd;
  UINT8 def;
  INT nb_par;
  INT par[MAX_PAR_TST];
  LONG par_lg[MAX_PAR_TST];
};
#endif


/***************************************************************************/
/*                  Pour les coupures secteur                              */
/***************************************************************************/

typedef struct x01_sram_var
{
//	T_x01_status2 			status2;
#ifdef PMV
//	T_x01_reprise_secteur 	reprise_secteur;
	struct
	{
		T_pip_sv_act_caisson caisson[NB_CAISSON];
		ULONG 				bcc;
	}	pip_sv_act;
#endif
} T_x01_sram_var;

/*
struct x01_trace_tr
{
	UINT8 las[NBPORT];
	UINT8 flag_trace;
	INT pt_in;
	INT pt_out;
	struct
	{
		UINT8 date [SZ_DATE];
		UINT8 txt  [LG_TRACE_AI];
	}trc[MAX_TRACE_AI];
};
struct x01_trace_maint
{
	INT pt_in;
	INT pt_out;
	INT pt_lec;
	INT pt_max;
	UINT bcc;
	struct
	{
		UINT8 date [SZ_DATE];
		UINT8 txt  [LG_TRACE_AI];
	}trc[MAX_TRACE_MAINT];
};

struct x01_trace_defaut
{
	INT pt_in;
	INT pt_out;
	INT pt_lec;
	INT pt_max;
	UINT bcc;
	struct
	{
		UINT8 date [SZ_DATE];
		UINT8 txt  [LG_TRACE_AI+10];
	}trc[MAX_TRACE_DEFAUT];
};

struct x01_trace_cmd
{
	INT pt_in;
	INT pt_out;
	INT pt_lec;
	INT pt_max;
	UINT bcc;
	struct
	{
		UINT8 date [SZ_DATE];
		UINT8 txt  [LG_TRACE_AI];
	}trc[MAX_TRACE_CMD];
};
*/
#endif
