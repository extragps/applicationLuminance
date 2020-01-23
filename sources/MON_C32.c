/**********************************************************/
/*                                                        */
/*                MONITEUR CPU32                          */
/*                                                        */
/* 14/05/91 creation                                      */
/**********************************************************/
#include "standard.h"
#include "mon_def.h"
#include "mon_str.h"
#include "mon_ext.h"
#include "mon_pro.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "hdl_crt.h"
#include "ypc_las.h"
#include "rec_main.h"
#include "identLib.h"

#if STATION
#include "varpile.h"
#endif

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/
GLOBALREF INT crt_type_init (VOID);

/*
VOID rqcbal (struct usr_anc *);
VOID rqemes (struct usr_anc *, struct mon_mes *);
VOID rqfin (struct mon_anc *, struct mon_mes *);
VOID rqini (VOID);
VOID rqithr (VOID);
struct mon_mes *rqmbx (LONG, struct usr_anc *, struct mon_mes *);
VOID rqrel (VOID);
VOID rqsusp (INT, struct usr_anc *);
VOID rquartsusp (INT, INT, struct usr_anc *, struct mon_mes *);
struct usr_ztf *rqzmes (struct mon_anc *, STRING);
*/

/* ---------------------------------
 * mnSetTempo
 * ==========
 * --------------------------------	*/
void mnSetTempo(int cpt,int val)
{
	rqtemp[cpt]=val;
}

int  mnGetTempo(int cpt)
{
	return rqtemp[cpt];
}

/**********************************************************/
/*                                                        */
/* monTraitPeriodique : traitement de l'interruption periodique       */
/*                                                        */
/**********************************************************/
VOID mnTraitPeriodique (VOID)
{
  INT i;
  c32_rqtemp = TRUE;
  for (i = 0; i < TP_MAX; i++)
    {
      if ((rqtemp[i] != TP_HS) && (rqtemp[i] != TP_FINI))
        {
          if (--rqtemp[i] == TP_FINI)
            {
              if ((i >= ID_0) && (i < (ID_0 + NBPORT)))
                {
                  /* Arret de l'autorisation en mode
                   * direct sur la liaison.       */
                  identInterdire (i - ID_0, TRUE);
                }                      /* endif((i>=ID_0)&&(i<(ID_0+NBPORT     */
            }                          /* endif(--rqtemp [i] ==TP_FINI)        */
        }                              /* if ((rqtemp [i] != TP_HS) &&         */
    }

  /* on controle les transmissions */
  tac_las_ctrl ();
}
