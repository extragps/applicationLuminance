/**********************************************************/
/*                                                        */
/*         definition des variables globales              */
/*                                                        */
/**********************************************************/

/*
 * Modifications :
 * 01a,22Mar01,xag	suppression des variables rqflc et rqflc1
 */

/* ********************************
 * INCLUSION DES FICHIERS
 * ********************************	*/

#ifdef VXWORKS
#include "vxworks.h"
#endif
#include "standard.h"
#include "mon_def.h"
#include "mon_str.h"
#include "define.h"
#include "xdg_def.h"


/* la tache de gestion des LAS */

VOID tac_clav (VOID);
VOID pcp_main0 (VOID);
VOID pcp_main1 (VOID);
VOID pcp_interne (VOID);
VOID rec_main_init (VOID);
VOID rec_main (VOID);
VOID rec_acq (VOID);
VOID trt_alt (VOID);
VOID hdl_dep (VOID);
VOID hdl_dep_ini (VOID);
VOID cal_ini (VOID);
VOID quart_ini (VOID);

/**********************************************************/
/*                                                        */
/*         definition des constantes globales             */
/*                                                        */
/**********************************************************/
#define  MAX_TACHE     9
#define  MAX_ANCRAGE   3
#define  LG_PILE0      600L
#define  LG_PILE1      1000L
#define  LG_PILE2      2000L
#define  LG_PILE3      10000L
#define  LG_PILE5      2000L

/**********************************************************/
/*                                                        */
/*         definition des variables globales              */
/*                                                        */
/**********************************************************/


/*#option sep_on segment u_data u_data  class mon_ram1 mon_ram1*/



//INT rqdgtb[MAX_TACHE];          /* table des chiens de garde           */
//INT rqtop[MAX_TACHE];           /* nombre de TOP pendant l'activite de la
//								   tache    */
//INT rqmxtop[MAX_TACHE];         /* nombre max de TOP pour une tache    */
//
//struct des_dyn rqdesc[MAX_TACHE];       /* table des descripteurs dynamiques */
//struct des_dyn *rqatv;          /* pointeur sur la tache active        */
//struct mon_anc rqata;           /* ancrage des taches actives          */
//struct mon_anc rqats;           /* ancrage des taches suspendues       */
//struct mon_anc rqatsig;         /* ancrage des taches SIG              */
//struct bug_sp rqbug_sp[MAX_TACHE];      /* information pour le debug   */
//struct bug_inf rqbug;           /* information pour le debug           */
INT rqtemp[NB_TP_MAX];          /* table des cases tempo               */
UINT8 idtemp[NBPORT];
char idTempNum[NBPORT];
char idDirectNum[NBPORT];

struct mon_ctl rqctl[NBPORT];   /* table de controle des uarts         */
struct mon_ope c32_operateur;

UINT8 c32_rqtemp;
INT c32_trace_dem[MAX_TRACE_DEM];

UINT8 cal_fini;
UINT8 cal_commande;
INT c32_top;



/*#option sep_off

#option sep_on segment parametre parametre class mon_prom mon_prom*/

/*
* table d'acces aux registres du QSPI
*/
LONG rqacqspi[6] = { SPCR1, SPCR2, SPCR3, SPREC, SPTRA, SPCDE };

/*
* table d'acces aux registres du TPU
*/
LONG tpu_reg = TPU_R;
LONG ch_cont = CH_CONT;
LONG pin_level = PIN_LEVEL;

/*
* table de debug
*/
LONG debug = TRUE;              /* indicateur de debug */

INT rqnbt = MAX_TACHE;          /* nombre de taches configurees        */
INT rqnba = MAX_ANCRAGE;        /* nombre d'ancrages configures        */
INT rqnbp = TP_MAX;             /* nombre de cases tempo configurees   */
INT rqnbi = MAX_PROC;
UINT8 rqflc2;					/* Detection du changement de seconde, utilise
								   pour sequencer des taches periodiques.
								   des taches */
