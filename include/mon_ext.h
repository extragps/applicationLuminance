
/*
 * 01a,02Mar01,xag	Ajout des tables pour stocker les numeros des
 * 					utilisateurs temporaires et en mode direct sur chaque
 * 					liaison.
 *	*/


#ifndef MON_EXT_H
#define MON_EXT_H

#include "define.h"

/**********************************************************/
/*                                                        */
/*         definition des constantes externes             */
/*                                                        */
/**********************************************************/
GLOBALREF POINTEUR rqst1;
GLOBALREF POINTEUR rqst2;
GLOBALREF POINTEUR rqst1com1;
GLOBALREF POINTEUR rqst2com1;
GLOBALREF POINTEUR rqipcr1;
GLOBALREF POINTEUR rqipcr2;
GLOBALREF POINTEUR rqipcr1com1;
GLOBALREF POINTEUR rqipcr2com1;
GLOBALREF POINTEUR rqacquart[];
GLOBALREF struct mon_qspi rqacqspi;
GLOBALREF UINT *tpu_reg;
GLOBALREF UINT *ch_cont;
GLOBALREF UINT *pin_level;      /* pointeur sur les parametres pin_level            */
GLOBALREF UINT table_vecteur;

GLOBALREF INT rqnbt;            /* nombre de taches configurees        */
GLOBALREF INT rqnba;            /* nombre d'ancrages configures        */
GLOBALREF INT rqnbp;            /* nombre de cases tempo configurees   */
GLOBALREF INT rqnbi;            /* nombre de procedure d'INIT      */

GLOBALREF struct des_sta *rqcrtb;      /* table de creation des taches        */

GLOBALREF struct
{
  void (*proc) (void);
} rqsys_proc[MAX_PROC];
GLOBALREF struct usr_anc *rqcran[];     /* table de creation des ancrages      */


GLOBALREF struct mon_iuart *rqiuart;   /* configuration des uarts             */


/**********************************************************/
/*                                                        */
/*         definition des variables externes              */
/*                                                        */
/**********************************************************/




GLOBALREF INT rqdgtb[];         /* table des chiens de garde           */
GLOBALREF INT rqtop[];          /* table des chiens de garde           */
GLOBALREF INT rqmxtop[];        /* table des chiens de garde           */
GLOBALREF struct des_dyn *rqdesc;      /* table des descripteurs dynamiques   */
GLOBALREF struct des_dyn *rqatv;        /* pointeur sur la tache active        */
GLOBALREF struct mon_anc rqata; /* ancrage des taches actives          */
GLOBALREF struct mon_anc rqats; /* ancrage des taches suspendues       */
GLOBALREF struct mon_anc rqatsec;       /* ancrage des taches suspendues  seconde */
GLOBALREF struct mon_anc rqatsig;       /* ancrage des taches suspendues sig   */
GLOBALREF struct bug_sp *rqbug_sp;     /* information pour le debug           */
GLOBALREF struct bug_inf rqbug; /* information pour le debug           */
GLOBALREF struct mon_tac *mon_tac;


/**********************************************************/
/*                                                        */
/*         definition des variables globales              */
/*                                                        */
/**********************************************************/


GLOBALREF INT rqtemp[];         /* table des cases tempo               */
GLOBALREF UINT8 idtemp[];       /* Pour les identifications temporaires */
                                                                        /* Pour stocker les numeros des utilisa-
                                                                         * teurs autorises, en mode direct ou
                                                                         * temporaire sur une liaison.  */
GLOBALREF char idTempNum[];
GLOBALREF char idDirectNum[];

GLOBALREF struct mon_ctl *rqctl;       /* table de controle des uarts         */
GLOBALREF UINT8 rqflc2;         /* fin de lecture du calendrier        */

GLOBALREF UINT8 rqrsr;          /* reset status register               */

GLOBALREF struct mon_ope c32_operateur;


GLOBALREF ULONG c32_co;
GLOBALREF ULONG c32_co_sav;
GLOBALREF ULONG c32_co_max;
GLOBALREF ULONG c32_co_min;
GLOBALREF UINT c32_co_sec;
GLOBALREF UINT8 c32_rqtemp;
GLOBALREF UINT c32_co_ms;

GLOBALREF UINT8 c32_phase_pile;
GLOBALREF INT c32_trace_dem[];
GLOBALREF INT c32_nbcar_printf;
GLOBALREF UINT8 c32_mem_printf[];
GLOBALREF UINT8 c32_fifo_printf[];
GLOBALREF INT c32_fifo_printf_in;
GLOBALREF INT c32_fifo_printf_out;
GLOBALREF UINT *c32_ptr_zpower;

GLOBALREF UINT8 cal_fini;
GLOBALREF UINT8 cal_commande;
GLOBALREF UINT8 cal_calendrier[];
GLOBALREF INT c32_top;
GLOBALREF BYTE c32_flag_ctrl_trans;



GLOBALREF INT trace_init[];


#endif
