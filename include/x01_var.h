#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"

#ifndef X01_VAR_H
#define X01_VAR_H

/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES                */
/**                                                                       **/
/***************************************************************************/
/* variables utilisees par l'application */
/* ancrage des liaison en reception pour le protocole TEDI esclave */

extern mqd_t x01_ancrage_TEDI_esclave_em[NBPORT];
extern mqd_t x01_ancrage_TEDI_esclave_rec[NBPORT];
#ifdef RAD
extern mqd_t x01_ancrage_TEDI_abt[NBPORT];
extern mqd_t x01_ancrage_com185;
extern mqd_t x01_ancrage_int108;
extern mqd_t x01_ancrage_acq;
extern mqd_t x01_ancrage_detecteur[NBPORT];
extern mqd_t x01_ancrage_master[NBPORT];
extern mqd_t ancrage_abt[NBPORT];
extern mqd_t ancrage_tac_vip_emission_reception;
extern mqd_t ancrage_tac_vip_em_rec;
#endif
extern mqd_t ancrage_pcp_lecmess[NBPORT];
extern INT MqDialLas[NBPORT];
extern INT MqLasDial[NBPORT];
extern INT MqAbtDial[NBPORT];
extern mqd_t x01_ancrage_interne;
extern mqd_t ancrageTedi_ctrl;  /* definition de l'ancrage de retour          */

/* flag de reception sur un port */
extern UINT8 x01_las_rec[NBPORT];

#ifdef PMV
/* tempo d'occupation des liaison en maitre */
extern INT x01_liaison_maitre[NBPORT];
extern BYTE x01_application_PIP;
mqd_t x01_ancrage_module_aff[NBPORT];
#endif
/************************************************/
/*  declaration des droits d'acces e la station */

/************************************************/
/*  pour le type d'init                         */
/************************************************/
extern UINT8 x01_reinit;

/************************************************/
/*  si reception d'une adresse jocker           */
/************************************************/
extern UINT8 x01_flag_jocker[NBPORT];

/************************************************/
/*  declaration des droits d'acces  CFID        */
/************************************************/
extern struct x01_cfid x01_cfid[NBPORT];
extern INT x01_nb_cfid;

/***********************************************************************/
/* configuration des ports logiques   SET                              */
/***********************************************************************/
extern struct x01_set x01_set[NBPORT];

/***************************************************/
/*   definition du status general de la station ST */
/***************************************************/
extern struct x01_status1 x01_status1;
extern struct x01_status3 x01_status3;
/* extern T_x01_status2 *x01_status2; */
/*extern T_x01_reprise_secteur *x01_reprise_secteur;*/

/************************************************/
/*   definition de l'erreur de transmission     */
/************************************************/

extern struct x01_cptr x01_cptr;

/*****************************************************/
/* nombre d'heure d'autonomie avant mode economiseur */
/*****************************************************/

extern UINT8 x01_bat;
extern char x01_date[];
extern UINT x01_ctrl_timer;
extern LONG x01_bcc_prom;
CommCnx *cnxLiaison[NBPORT];

#endif
