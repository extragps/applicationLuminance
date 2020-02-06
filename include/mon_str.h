
#ifndef MON_STR_H
#define MON_STR_H

#ifdef VXWORKS
#include "vxworks.h"
#include "mq.h"
#endif

#ifdef LINUX
#include "mq.h"
#endif

#include "standard.h"
#include "define.h"
#include "mon_def.h"

/************************************************************************

                 STRUCTURES GLOBALES

************************************************************************/

/************************************************/
/* structure des messages du  moniteur          */
/************************************************/
struct mon_mes
{
  struct mon_mes *pt_amont;
  struct mon_mes *pt_aval;
  mqd_t MqId;
  LONG co;
};

#if !STATION
/***************************************************************/
/* structure d'information pour le debug debordement des piles */
/***************************************************************/
struct bug_sp
{
  INT *sp;                      /* sommet de la pile            */
  INT *bp_sp;                   /* fin de la pile               */
  INT fr;                       /* format de pile               */
};


/***************************************************/
/* structure des descripteurs dynamiques de taches */
/***************************************************/
struct des_dyn
{
  struct mon_mes entete;        /* entete de type message     */
  STRING *pc;                   /* le pointeur courant        */
  INT *sp;                      /* le pointeur de pile        */
  INT sr;                       /* registre sr                */
  INT id;                       /* identificateur de la tache */
  INT fr;                       /* format de pile             */
  UINT8 *top_sp;                /* sur le sommet de la pile   */
  UINT8 *bot_sp;                /* sur le fond de la pile     */
  UINT tp_max;                  /* le temps d'occupation max de la tache    */
  UINT tp_min;                  /* le temp d'occupation min de la tache     */
  ULONG csec;                   /* la duree d'activation de la tache        */
  LONG tp_occ;                  /* le temps d'occupation total de la tache  */
};
/***************************************************/
/* structure des descripteurs statiques de taches */
/***************************************************/
struct des_sta
{
  LONG proc;
  LONG tl_sp;
};
#else
/***************************************************************/
/* structure d'information pour le debug debordement des piles */
/***************************************************************/
struct bug_sp
{
  UINT8 *sp;                    /* sommet de la pile            */
  UINT8 *bp_sp;                 /* fin de la pile               */
  INT fr;                       /* format de pile               */
};

/***************************************************/
/* structure des descripteurs dynamiques de taches */
/***************************************************/
struct des_dyn
{
  struct mon_mes entete;        /* entete de type message     */
  INT *sp;                      /* registre usp               */
  LONG pc;                      /* registre pc                */
  INT sr;                       /* registre sr                */
  INT id;                       /* identificateur de la tache */
  INT fr;                       /* format de pile             */
};

/**************************************************/
/* structure des descripteurs statiques de taches */
/**************************************************/
struct des_sta
{
  LONG pc;                      /* adresse de demarrage de la tache */
  LONG lp;                      /* longueur de la pile              */
};

#endif

/*****************************************/
/* structure d'information pour le debug */
/*****************************************/
struct bug_inf
{
  INT *sp;                      /* registre usp                 */
  LONG pc;                      /* registre pc                  */
  INT sr;                       /* registre sr                  */
  INT id;                       /* identificateur de la tache   */
  INT fr;                       /* format de pile               */
  struct des_dyn *ad;           /* adresse du descripteur       */
};

/*****************************************
 * structure d'information pour le controle des temps d'occupation
 *****************************************/
struct mon_tac
{
  INT tp;
  INT tp_min;
  INT tp_max;
  INT tp_moy;
};
/*************************************************/
/* structure de controle des transmissions QUART */
/*************************************************/
struct mon_ctl
{
  POINTEUR adr_io;              /* adresse de base de l'UART               */
  UINT8 rts;                    /* demande le RTS                          */
  UINT8 attente;                /* demande pour attente courte             */
  UINT8 ctud_485[2];            /* temps d'attente pour 485                */
  UINT8 ctuf_485[2];
  UINT8 mode;                   /* mode de transmission                    */
  UINT8 echo;
  UINT8 vt;
  UINT8 vt_pt_in;
  UINT8 vt_pt_out;
  UINT8 presence;
  INT *co_mx;                   /* compteur maximal                        */
  INT co_em;                    /* compteur d'emission                     */
  STRING buf_em;                /* buffer d'emission                       */
};
/************************************************/
/* structure d'initialisation des ports         */
/************************************************/
struct mon_iuart
{
  UINT8 presence;
  UINT8 cr;
  UINT8 mr1;
  UINT8 mr2;
  UINT vitesse;
  UINT8 protocole;
  UINT8 t_full;
  UINT8 t_485;
  UINT8 t_rts;
  UINT8 rts;
  UINT8 rtr;
  UINT8 dtr;
  UINT8 dcd;
  UINT8 cts;
  UINT8 n_pad;
  UINT8 npad_485;
  INT time_out;
};
/************************************************/
/* structure de traitement des caracteres recus*/
/************************************************/
struct mon_car
{
  UINT8 eve;
  UINT8 err;
  UINT8 nb_car;
  UINT8 car[8];
  INT port;
  struct mon_ctl *pt_ctrl;
};

/************************************************/
/* structure des ancrages du moniteur           */
/************************************************/
struct mon_anc
{
  struct mon_mes *pt_tete;
  struct mon_mes *pt_queue;
};

/******************************************/
/* structure d'acces aux registres du SCI */
/******************************************/
struct mon_sci
{
  INT *pt_status;               /* adresse du registre de status  */
  INT *pt_donnee;               /* adresse du registre  de donnee */
  INT *pt_control;              /* adresse du registre  de control */
};

/******************************************************************/
/* structure de traitement du dialogue operateur avec le moniteur */
/******************************************************************/
struct mon_ope
{
  INT co_in;                    /* compteur de donnees recues    */
  INT co_in_max;
  INT co_out;                   /* compteur de donnees emises    */
  INT co_max;                   /* compteur de donnees a emettre */
  UINT8 buf_in[MAX_OPE];        /* buffer de reception           */
  UINT8 buf_out[MAX_OPE];       /* buffer d'emission             */
  UINT8 *pt_mem;                /* pointeur sur une zone memoire */
  UINT8 phase;
  UINT8 code;
  UINT8 fin_em;
};

/*******************************************/
/* structure d'acces aux registres du QSPI */
/*******************************************/
struct mon_qspi
{
  INT *pt_enable;               /* adresse du registre  de validation  du QSPI */
  INT *pt_control;              /* adresse du registre  de control             */
  INT *pt_status;               /* adresse du registre de status               */
  INT *pt_reception;            /* adresse de la ram en reception              */
  INT *pt_emission;             /* adresse de la ram en emission               */
  POINTEUR pt_commande;         /* adresse de la ram des commandes             */
};

/************************************************/
/* structure des messages de question reponse   */
/************************************************/
typedef struct usr_ztf
{
  struct mon_mes entete;        /* definition des entetes du moniteur   */
  STRING pt_buff;               /* adresse du buffer du message         */
  INT nbcar_rec;                /* nombre de caractere recu             */
  INT nbcar_max;                /* nombre de caractere max du buffer de
                                   reception */
  UINT8 adresse[5];             /* adresse du message                   */
  UINT8 no_las;
  UINT8 mode;                   /* mode du protocole TEDI utilise       */
  UINT8 reponse;                /* si l'application de configuration doi */
  /* transmettre la reponse               */
  UINT8 no_application;
  UINT8 no_carte;
  UINT8 compte_rendu;
  UINT8 libre;
  UINT8 bin;
  char buffer[SZ_BLK * 2];
} T_usr_ztf;

/************************************************/
/* structure des ancrages des utilisateurs      */
/************************************************/
struct usr_anc
{
  struct mon_anc mes;
  struct mon_anc tac;
};



/************************************************/
/* pour gestion du calendrier                   */
/************************************************/

struct mon_cal
{
  UINT8 sec;
  UINT8 min;
  UINT8 heure;
  UINT8 jr_s;
  UINT8 jr_ms;
  UINT8 mois;
  UINT8 annees;
};

#endif
