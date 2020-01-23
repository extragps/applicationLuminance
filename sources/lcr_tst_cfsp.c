/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf0                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/05/93                                                 /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Commande de configuration globale                 /
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
#include "vxworks.h"
#include "standard.h"      /* redefinition des types                         */
#include "stdio.h"
#include "string.h"
#include "mon_inc.h"
#include "define.h"        /* definition des constantes                      */
#include "vct_str.h"       /* definition des structures globales             */
#include "x01_vcth.h"      /* definition des variables globales              */
#include "x01_var.h"     /* definition des variables globales               */
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_util.h"
#include "lcr_tst.h"
#include "ted_prot.h"
#include "identLib.h"

/* *******************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define LCR_TST_CFSP_ZGO "ZGO"


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cf0_cf                                                 /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 20/05/1993                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf0.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : mise a jour de la liste canal capteur                  /
/-------------------------------------------------------------------------DOC*/

INT lcr_tst_cfsp(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess,INT flg_fin, INT *bloc)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int nbCar = 0;
	int erreur = 0;
	INT config = FALSE;
	UINT8 zgo = 3;

	ptCour += strlen("TST CFSP");
	lgCour -= strlen("TST CFSP");
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);

	while ((lgCour > 0) && (0 == erreur)) {
		if (strncmp(ptCour, LCR_TST_CFSP_ZGO, strlen(LCR_TST_CFSP_ZGO)) == 0) {
			ptCour += strlen(LCR_TST_CFSP_ZGO);
			lgCour -= strlen(LCR_TST_CFSP_ZGO);
			ptCour = tst_passe_egale(ptCour, lgCour, &lgCour);

			/* Il faut ici parser le type.. */

			if ('3' == *ptCour) {

			} else {
				erreur = 3;
			}
			ptCour++;
			lgCour--;
		} else {
			erreur = 2;
		}
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	}
	if (0 == erreur) {
		tst_send_bloc(las, mode, buffer, &nbCar, bloc, FALSE, pt_mess,
				"TST CFSP %s=%d%s", LCR_TST_CFSP_ZGO, (int) zgo,(flg_fin==TRUE?"\n\r":""));
		tst_send_flush(las,mode,buffer,&nbCar,bloc,(flg_fin==TRUE?FALSE:TRUE),pt_mess);
	} else {
		tedi_ctrl_ext(las, mode, erreur);
		config = 0;
	}
	return (config);
}
