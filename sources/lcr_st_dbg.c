
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 02/05/01
 * Fichier	: lcr_st_dbg.c
 * Objet	: Traitement de la commande d'initialisation du compteur
 * 			anti-chevauchement.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,02May01,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "standard.h"
#include "define.h"
#include "x01_str.h"
#include "vct_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_st_divers.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_cftp.h"
#include "identLib.h"
#include "pip_util.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif


/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

#define ST_DBG_CMD				"ST DBG"

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * lcr_st_dbg
 * =============
 * Analyse de la commande "ST ERI"
 * --------------------------------	*/

int lcr_st_dbg (INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
  char *ptCour = (char *) &buffer[position];
  int lgCour = lg_mess - position;
  int modifConfig = 0;
  int bloc=0;
  int j;
  int fini=0;
  int i=0;


  ptCour += strlen(ST_DBG_CMD);
	lgCour -= strlen(ST_DBG_CMD);
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);

	j = sprintf(buffer, "ST DBG\n\r");
	while ((!fini) && (i < pip_nb_module)) {
		T_pip_cf_module *module = &pip_cf_module[i];
		int k = module->num_caisson;
		bool premier_b = true;
		while (k < module->num_caisson + module->nb_caisson) {
			if (pip_cf_caisson[k].type == TYPE_ALPHA) {
				if (premier_b) {
					j += sprintf(&buffer[j], (STRING) " BUS=COM485/0 CNX=OK");

				}
				premier_b = false;
				j += sprintf(&buffer[j], (STRING) "\n\r  AM=%d.%d ST=OK",
						(INT) module->id_module,
						(INT) pip_cf_caisson[k].caisson);
			}
			k++;
		}
		/* le module suivant */
		i++;
	}

  	tedi_send_bloc (las, mode, buffer, j, bloc,TRUE, pt_mess);
  /* ------------------------------------
   * FIN DE lcr_st_dbg
   * ------------------------------------ */
  return modifConfig;
}
