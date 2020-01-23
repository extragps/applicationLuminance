
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: ?
 * Fichier	: vct_str.h
 * Objet	: Definition de differentes structures.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01b,03Jui01,xag  ajout pour l'efficacite des mesures.
 * ************************************************************************	*/


#ifndef VCT_STR_H
#define VCT_STR_H

#include "mon_def.h"


/************************************************/
/*   structure des identifiants et mots de passe*/
/************************************************/
typedef struct zip
{

  char num_util;
  char identifiant[9];         /* nom de l'identifiant                            */
  char password[9];            /* mot de passe lie e l'identifiant                */
  char format;
} T_zip;



/*************************************************/
/* definition des parametres de status general   */
/*************************************************/

/***************************************************************************/
/*  definition des structures de configuration des alertes individuelles   */
/***************************************************************************/
typedef struct vct_critere
{
  UINT8 nature;                 /* nature de la mesure:DI distance inter-vehiculaire */
  /*                     EI poids d'un essieu          */
  /*                     II temps intervehiculaire     */
  /*                     LI longueur d'un vehicule     */
  /*                     TI temps de presence          */
  /*                     VI vitesse instantannee       */
  /*                     N_AFF pour non affecte        */
  UINT8 relation;               /* REL_INF pour inferieur au seuil                   */
  /* REL_SUP pour superieur au seuil                   */
  /* REL_DIFF pour different du seuil                  */
  /* REL_EGAL pour egal au seuil                       */
  UINT8 etat;
  UINT8 oplog;
  UINT prec;
  UINT seuil;
} T_vct_critere;


/***************************************************************************/
/*  definition des structures de configuration des alertes grandeur moyenne*/
/***************************************************************************/

struct vct_cf_al_my
{
  UINT8 flag;                   /* presence d'une alerte sur le canal                */
  T_vct_critere critere[MAX_CRT_MY];
};

/***************************************************************************/
/*  definition des structures de configuration des alertes systeme         */
/***************************************************************************/

typedef struct vct_cf_al_sy
{
  UINT8 flag;                   /* presence d'une alerte systeme                     */
  T_vct_critere critere[MAX_CRT_SY];
} T_vct_cf_al_sy;

/***************************************************************************/
/*  definition des structures de status des alertes                        */
/***************************************************************************/

typedef struct vct_st_al_mess
{
  UINT8 flag;
  UINT8 prot;
  UINT8 adresse[3];
  unsigned long adFip;
  unsigned long idFip;
  UINT8 nb_em;
  UINT8 txt_em[MAX_CAR_AL];
  UINT8 nb_rep;
  UINT8 txt_rep[MAX_CAR_AL];
  INT tp_em;
  INT tp_rep;
} T_vct_st_al_mess;

typedef struct vct_st_al
{
  UINT8 act;                    /* FALSE alerte inactive                                */
  /* TRUE  alerte active                                  */
  UINT neut;                    /* temps de neutralisation entre 2 alertes              */
  UINT8 rep;                    /* nombre de repetition                                 */
  UINT8 seq;                    /* sequencement pour CFAL M m,V,B,H                     */
  UINT8 port;                   /* port sur lequel sera genere l'alerte                 */
  UINT8 nb_mess;                /* nombre de message                                    */
  T_vct_st_al_mess mess[MAX_MESS_AL];
} T_vct_st_al;
/********************************************************/
/*  definition des structures de detection des alertes  */
/********************************************************/
struct vct_det_alerte
{
  UINT8 init;                   /* flag d'initialisation                           */
  UINT8 test;                   /* generation d'un test d'alerte                   */
  UINT8 ind[NBCANAL];           /* detection d'une condition d'alerte individuelle */
  UINT8 my[NBCANAL];            /* detection d'une condition d'alerte moyenne      */
  UINT8 sys[MAX_CRT_SY];        /* detection d'une condition d'alerte systeme      */
};
/************************************************/
/*  pour les alertes temps reel                */
/************************************************/
struct vct_alerte_tr
{
  UINT8 alerte;
  UINT8 canal_surcharge;
  UINT8 canal_contresens;
  UINT8 flag_alerte[NBCAPTEUR];
};


/************************************************************/
/*  definition des structures de configuration des alarmes  */
/************************************************************/
struct vct_cf_alarme
{
  UINT8 type;                   /* le type d'alarme 0 non affectee        */
  /*                  1 EDF                 */
  /*                  2 BCL                 */
  /*                  3 TRAFIC              */
  UINT8 num_tor;                /* le numero de la sortie TOR             */
  UINT8 num_log;                /* le numero de la boucle ou numero canal */
  UINT8 N_SAT;                  /* nombre de sequence avant saturation    */
  INT S_Q;                      /* seuil QT                               */
  UINT8 N_Q;                    /* nombre de sequence QT                  */
  INT S_T;                      /* seuil TT                               */
  UINT8 N_T;                    /* nombre de sequence TT                  */
  INT S_V;                      /* seuil VT                               */
  UINT8 N_V;                    /* nombre de sequence VT                  */
};

//************************************************/
/*  definition des descripteurs de fichiers     */
/************************************************/

typedef struct zdf
{
  STRING pt_deb;                /* adresse de depart de la zone reservee au fichier      */
  LONG sz_enr;                  /* taille d'un enregistrement                            */
  LONG max_seq;                 /* nombre maxi de sequences e stocker                    */

  LONG nb_ecr;                  /* nombre de sequences disponibles                       */
  LONG num_ecr;                 /* numero de donnees courant                             */
  STRING pt_ecr;                /* adresse de l'enregistrement courant                   */
  STRING pt_prem;               /* adresse du premier enregistrement  (pour les
                                 * fichiers de traces. */

  LONG nb_dispo[NBPORT];        /* nombre de sequences disponibles               */
  LONG sav_num_ecr[NBPORT];     /* sauvegarde du numero de donnees courant       */
  STRING sav_pt_ecr[NBPORT];    /* adresse de l'enregistrement courant           */
  UINT8 ty_seq;                 /* type de sequencement du fichier                       */
  UINT8 pad;                    /* type de sequencement du fichier                       */
} T_zdf;

/************************************************/
/*  definition du descripteur d'acces a la base  */
/************************************************/
struct z_acces_bdd
{
  UINT8 ecriture;
  UINT8 lecture;
};
/*****************************************************/
/*  definition du descripteur de changement d'heure  */
/*****************************************************/
struct z_horodate
{
  UINT8 flag;
  INT las;
  LONG diff_sec;
};
/************************************************/
/*  definition du descripteur d'enregistrement  */
/************************************************/

struct sz_enreg
{
  INT sz_rec;                   /* taille d'un enregistrement complet  */
  INT sz_tete;                  /* taille de l'entete                  */
  UINT8 nb_donn[MAXDATA];       /* nombre de donnees par ligne        */
  struct
  {
    UINT8 sz_donn_rec[MAXDATA]; /* taille des donnes stockees */
    UINT8 sz_donn_em[MAXDATA];  /* taille des donnes stockees */
    UINT8 sz_canal;             /* taille des donnes canal    */
  } canal[NBCANAL];
};

/**********************************************************************/
/*    pour l'emission sur la liaison de debug                         */
/**********************************************************************/

struct deb_brt
{
  UINT8 nbcar;
  UINT8 code;
  UINT8 capteur;
  ULONG date;
  ULONG mls;
  UINT8 pres;
  ULONG tp;
  UINT va;
  UINT vam;
};


/************************************************/
/*  pour lidentification de la version          */
/************************************************/
struct vct_date_version
{
  UINT8 jour;
  UINT8 mois;
  UINT8 annee;
};


struct vct_trace_ai
{
  UINT8 las[NBPORT];
  UINT8 flag_trace;
  UINT8 detecteur;
  INT pt_in;
  INT pt_out;
  INT pt_lec;
  struct
  {
    UINT8 date[SZ_DATE];
    UINT8 txt[LG_TRACE_AI];
  } trc[MAX_TRACE_AI];
};

struct vct_trace_maint
{
  INT pt_in;
  INT pt_out;
  INT pt_lec;
  INT pt_max;
  ULONG bcc;
  struct
  {
    UINT8 date[SZ_DATE];
    UINT8 txt[LG_TRACE_AI];
  } trc[MAX_TRACE_MAINT];
};

/************************************************/
/*   structure de gestion des boucles profondes */
/************************************************/

struct x01_bclprf
{
  DOUBLE sl;                    /* somme des longueurs                             */
  DOUBLE nv;                    /* nombre de vehicules                             */
  DOUBLE lm;                    /* longueur moyenne                                */
  DOUBLE lmv;                   /* longueur moyenne validee                        */
  DOUBLE coef;                  /* coefficient                                     */
};

/* structure de gestion de la trace silhouette */
struct vct_trace_silhouette
{
  UINT8 pt_in;
  UINT8 pt_out;
  UINT8 pt_max;
  UINT8 txt[MAX_TRC_SILHOUETTE][100];
};

typedef struct vct_usr_lcpi
{
	char infos[MAX_INFO_LCPI];
}	T_vct_usr_lcpi;
#endif
