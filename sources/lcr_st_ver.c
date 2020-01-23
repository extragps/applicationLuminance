/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 11/06/01
 * Fichier	: lcr_st_ver.c
 * Objet	: Commande de lecture de la version logicielle;
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,11Jun01,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "mon_str.h"
#include "mon_debug.h"
#include "portage.h"
#include "lcr_util.h"
#include "standard.h"
#include "define.h"
#include "x01_str.h"
#include "vct_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_st_divers.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define ST_VER_CMD				"ST VER"
#define TST_VER_CMD_FULL                        "TST VERSION"

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * lcr_st_ver
 * =============
 * Analyse de la commande "ST VER"
 * --------------------------------	*/
int lcr_st_btr(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin,
		INT * bloc)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int modifConfig = 0;
	int nbCar = 0;

	ptCour += strlen(ST_VER_CMD);
	lgCour -= strlen(ST_VER_CMD);
	tst_send_bloc(las, mode, buffer, &nbCar, bloc, TRUE, pt_mess, "STATUS BTR=%s", ((esIsDefautAlim()||essDefautAlim())?"1":"0"));
	/* ------------------------------------
	 * FIN DE lcr_st_ver
	 * ------------------------------------ */
	return modifConfig;
}

int lcr_st_edf(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin,
		INT * bloc)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int modifConfig = 0;
	int nbCar = 0;

	ptCour += strlen(ST_VER_CMD);
	lgCour -= strlen(ST_VER_CMD);
	tst_send_bloc(las, mode, buffer, &nbCar, bloc, TRUE, pt_mess, "STATUS EDF=%1c", x01_status3.edf);
	/* ------------------------------------
	 * FIN DE lcr_st_ver
	 * ------------------------------------ */
	return modifConfig;
}

int lcr_st_ver(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin,
		INT * bloc)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int modifConfig = 0;
	int nbCar = 0;

	ptCour += strlen(ST_VER_CMD);
	lgCour -= strlen(ST_VER_CMD);
	tst_send_bloc(las, mode, buffer, &nbCar, bloc, TRUE, pt_mess, "STATUS VER=%d.%d", versionLireMajeure("Application"),
			versionLireMineure("Application"));
	/* ------------------------------------
	 * FIN DE lcr_st_ver
	 * ------------------------------------ */
	return modifConfig;
}

int lcr_tst_version(INT las,INT mode,INT lg_mess,STRING buffer,INT position, struct usr_ztf *pt_mess,INT flg_fin,INT *bloc)
{
char *ptCour=(char *)&buffer[position];
int   lgCour=lg_mess-position;
int nbCar=0;
int flag_err=0;

	/* Une petite evolution pour prendre en compte le TST VERSION */

	if(0==strncmp(buffer,TST_VER_CMD_FULL,strlen(TST_VER_CMD_FULL)))
	{
		ptCour+=strlen(TST_VER_CMD_FULL);
		lgCour-=strlen(TST_VER_CMD_FULL);
	} else {
		flag_err=1;
	}
	ptCour=tst_passe_blanc(ptCour,lgCour,&lgCour);

	if(0==flag_err) {

	tst_send_bloc(las,mode,buffer,&nbCar,bloc,TRUE,pt_mess,
			"TST VERSION APP=%s VER=%s build=%s",
			dirifGetAppName(), dirifGetVersionString(), dirifGetVersionBuild());
	} else {
		x01_cptr.erreur = flag_err;
		tedi_erreur (las,mode);
	}
									/* ------------------------------------
									 * FIN DE lcr_st_ver
									 * ------------------------------------	*/
	return 0;
}
