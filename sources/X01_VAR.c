#ifdef VXWORKS
#include "vxworks.h"
#include "MQ.h"
#endif
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "x01_str.h"


/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES                */
/**                                                                       **/
/***************************************************************************/
#if STATION
/*#option sep_on segment x01_var_ram x01_var_ram class ram_x01 ram_x01*/
#endif

/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES                */
/**                                                                       **/
/***************************************************************************/
                                                                                /* variables utilisees par l'appli-
                                                                                 * cation */
                                                                                /* ancrage des liaison en reception
                                                                                 * pour le protocole TEDI esclave */
#ifdef RAD
mqd_t x01_ancrage_TEDI_abt[NBPORT];
mqd_t ancrage_abt[NBPORT];
mqd_t x01_ancrage_com185;
mqd_t x01_ancrage_int108;
mqd_t x01_ancrage_acq;
mqd_t x01_ancrage_detecteur[NBPORT];
INT MqAbtDial[NBPORT];
#endif
#ifdef PMV
UINT8 x01_application_PIP;
mqd_t x01_ancrage_module_aff[NBPORT];
#endif
mqd_t x01_ancrage_TEDI_esclave_em[NBPORT];
mqd_t x01_ancrage_TEDI_esclave_rec[NBPORT];
mqd_t x01_ancrage_master[NBPORT];
mqd_t ancrage_pcp_lecmess[NBPORT];

/* ancrage interne */
mqd_t x01_ancrage_interne;
                                                /*  mq tedi_ctrl */
mqd_t ancrageTedi_ctrl;         /* definition de l'ancrage de retour          */

/*mqd_t MqDialLas    [NBPORT];*/
INT MqDialLas[NBPORT];
INT MqLasDial[NBPORT];

/********************************************************/
/* indicateur de reception d'une cde d'interruption     */
/********************************************************/
#ifdef RAD
UINT8 x01_las_rec[NBPORT];
#endif

/************************************************/
/*  pour le type d'init                         */
/************************************************/
UINT8 x01_reinit;

/************************************************/
/*  si reception d'une adresse jocker           */
/************************************************/
UINT8 x01_flag_jocker[NBPORT];

#ifdef PMV

/***********************************************************/
/*  declaration des cfmp macro commande de positionnement  */
/***********************************************************/
struct x01_cfmp x01_cfmp[NB_CFMP];
INT x01_nb_cfmp;


#endif

/***********************************************************************/
/* configuration des ports logiques   SET                              */
/***********************************************************************/
struct x01_set x01_set[NBPORT];

/***************************************************/
/*   definition du status general de la station ST */
/***************************************************/
struct x01_status1 x01_status1;
struct x01_status3 x01_status3;

/************************************************/
/*   definition de l'erreur de transmission     */
/************************************************/
struct x01_cptr x01_cptr;
/*****************************************************/
/* nombre d'heure d'autonomie avant mode economiseur */
/*****************************************************/
UINT8 x01_bat;
char x01_date[SZ_DATE];
UINT x01_ctrl_timer;
LONG x01_bcc_prom;

/* Ajoute pour vxWorks */

CommCnx *cnxLiaison[NBPORT];
