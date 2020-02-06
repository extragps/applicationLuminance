/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_dt                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 04/07/97                                                  /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE :                                                   /
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

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "standard.h"                  /* redefinition des types                         */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
#include "sir_dv1h.h"
#include "sir_dv2h.h"

#include "lcr_cf3h.h"
#include "ted_prot.h"
#include "lcr_idfh.h"
#include "lcr_util.h"
#include "ficTrace.h"
#include "lcr_trc.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/
/* ----------------------------
 * cmd_trc_mat
 * ==========
 * Trace de maintenance pour
 * garder la compatibilite avec
 * les traces PMV. Le premier
 * parametre n'est plus utilise,
 * il permettait de placer de
 * tracer des eventuels defauts.
 * ----------------------------	*/

void cmd_trc_mat(INT trace, STRING format, ...)
{
	va_list liste;
	struct timespec heure;
	T_ficTraceAdresse adresse;
	char txt[256];

	va_start(liste, format);
	clock_gettime(CLOCK_REALTIME, &heure);
	vsprintf(txt, format, liste);
	ficTraceAdresseSetStandard(&adresse, "I00000");
	ficTraceAjouterEnreg(F_IM, &heure, &adresse, strlen(txt), txt);
}

void cmd_trc_lon(STRING format, INT param[])
{
	/* Mise en commentaire des traces lon trop nombreuses!
	 cmd_trc_tr (format, param);
	 */
}

/* ----------------------------
 * cmd_trc_tr
 * ==========
 * Trace de maintenance.
 * ----------------------------	*/

void cmd_trc_tr(STRING format, ...)
{
	va_list liste;
	char txt[256];
	struct timespec heure;
	T_ficTraceAdresse adresse;
	va_start(liste, format);
	clock_gettime(CLOCK_REALTIME, &heure);
	vsprintf(txt, format, liste);
	ficTraceAdresseSetStandard(&adresse, "I00000");
	ficTraceAjouterEnreg(F_IM, &heure, &adresse, strlen(txt), txt);
}
