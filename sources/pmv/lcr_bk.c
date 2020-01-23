/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_bk												  /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/11/2005                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Configuration des equivalences topologiques.	  /
/                                                                             /
/-----------------------------------------------------------------------------/
/                                HISTORIQUE                                   /
/-----------------------------------------------------------------------------/
/ DATE   |   AUTEUR    | VERSION | No de fiche de modification                /
/-----------------------------------------------------------------------------/
/        |             |         |                                            /
/-----------------------------------------------------------------------------/
/        |             |         |                                            /
/-----------------------------------------------------------------------------/
/                    INTERFACES EXTERNES DU SOUS-MODULE                       /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/-----------------------------------------------------------------------------/
/ DONNEES EXPORTEES :                                                         /
/-----------------------------------------------------------------------------/
/ FONCTIONS IMPORTEES :                                                       /
/-----------------------------------------------------------------------------/
/ FONCTIONS EXPORTEES :                                                       /
/-----------------------------------------------------------------------------/
/               DONNEES ET FONCTIONS INTERNES DU SOUS-MODULE                  /
/-----------------------------------------------------------------------------/
/ DONNEES INTERNES :                                                          /
/-----------------------------------------------------------------------------/
/ FONCTIONS INTERNES :                                                        /
/-------------------------------------------------------------------------DOC*/

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include <string.h>
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "tac_conf.h"
#include "lcr_cfet.h"
#include "lcr_util.h"
#include "lcr_p.h"
#include "identLib.h"
#include "pip_util.h"
#include "lcr_bk.h"

#include "sramMessage.h"

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* *************************************
 * PROTOTYPE DES FONCTIONS LOCALES
 * ************************************* */

/* *************************************
 * FONCTIONS DU MODULE
 * ************************************* */

/* --------------------------------
 * lcr_bk
 * ========
 * Fonction de lecture et de confi-
 * guration des equivalences topolo-
 * giques.
 * --------------------------------	*/

INT lcr_bk (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
              T_usr_ztf * pt_mess)
{
  INT config = FALSE;
  int cmdBkS=FALSE;
  /* on se place apres le nom de la commande */
  char *ptCour = (char *) &buffer[position + strlen ("BK")];
  int lgCour = lg_mess - position - strlen ("BK");
  int flag_err = 0;
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);

  if (0 < lgCour)
    {
      /* on teste si l'operateur est habilite */
//      if (identLireAutorisationAvecNiveau (las, 0))
        {
          /* XG : TODO : traitement de la demande de configuration des
           * equivalences topologiques. Pour l'instant, il y a erreur*/
          if (strncmp (ptCour, "S", strlen ("S")) == 0)
            {
			  cmdBkS=TRUE;
              printDebug ("Traitement du S\n");
              ptCour++;
              lgCour--;
            }
          else
            {
              flag_err = 1;            /* Erreur parametre non reconnu. */
            }
          ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
          flag_err = (0 != lgCour) || flag_err;
        }
//      else
//        {
//          x01_cptr.erreur = CPTRD_PROTOCOLE;
//        }
    }

  /* si on a pas detecte d'erreur */
  if ((x01_cptr.erreur == CPTRD_OK) && (0 == flag_err))
    {
		if(TRUE==cmdBkS)
		{
		T_pip_cmd_p cmd_p;
			cmd_init_p(&cmd_p);
			cmd_p.id_module=NN_AFF;
			cmd_p.caisson=N_AFF;
			cmd_p.neutre=1;
			cmd_exec_p(&cmd_p);
			{
			int j;
			int deb,fin;
				deb=pip_util_ind_deb(0,0);
				fin=pip_util_ind_fin(NN_AFF,N_AFF);
				for (j = deb; j < fin; j++)
                {
				int indCaisson=pip_util_ind_caisson(j);
				T_pip_sv_act_caisson act_caisson;
				T_pip_sv_act_caisson *pip_sv_act_caisson=&act_caisson;
					sramMessageLireCaisson(indCaisson,pip_sv_act_caisson);
                      if (pip_sv_act_caisson->act != 0)
                      {
                          /* on active le caisson */
                          activation_caisson (indCaisson);
                      }
                }
			}

		}
      tedi_ctrl (las, mode, TRUE);
    }
  else
    {
      config = FALSE;
      tedi_ctrl_ext (las, mode, ((x01_cptr.erreur == CPTRD_PROTOCOLE) ? 5 : 2));
    }
  return config;
}
