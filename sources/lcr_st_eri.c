/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 02/05/01
 * Fichier	: lcr_st_eri.c
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
#include "eriLib.h"
#include "lcr_pcp_cmd.h"
#include "configuration.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define ST_ERI_CMD				LCR_PCP_ST_ERI
#define LCR_TST_INFO_ERI_EXT	"EXT"
#define LCR_ENDLINE "\n\r"

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

const struct _eriLibelle {
	int32 gravite_dw;
	int32 numero_dw;
	char *libelle_pt;
} _eriLibelle [] = {
	{E_eriMajeure,ERI_DEF_COMM_PIP,"Def communication interface"},
	{E_eriMajeure,ERI_DEF_CKS_PROGRAMME,"Defaut checksum"},
	{E_eriMajeure,ERI_DEF_PIP,"Defaut panneau"},
	{E_eriMajeure,ERI_DEF_SCRUTATION_PC,"Defaut scrutation PC"},
	{E_eriMajeure,ERI_DEF_PIXEL,"Defaut pixel"},
	{E_eriMajeure,ERI_DEF_ES,"Defaut carte d'entrees/sorties"},
	{E_eriMajeure,ERI_DEF_SABORDAGE_TRANS,"Sabordage"},
	{E_eriMajeure,ERI_DEF_TRANS,"Def communication afficheur"},
	{E_eriMajeure,ERI_DEF_TEMPERATURE,"Def temperature"},
	{E_eriMajeure,ERI_DEF_TEMP_AFF,"Def temperature interne"},
	{E_eriMajeure,ERI_DEF_CONF_AFF,"Def configuration afficheur"},
	{E_eriMajeure,ERI_DEF_SERIALISATION,"Defaut AR HS"},
	{E_eriMineure,ERI_DEF_FIN_VALIDITE,"Fin de validite message"},
	{E_eriMineure,ERI_DEF_CELL_LUMINOSITE,"Def cellule luminosite"},
	{E_eriMineure,ERI_INF_CHAUFFAGE,"Chauffage active"},
	{E_eriMineure,ERI_DEF_PARASURTENSEUR,"Defaut parasurtenseur"},
	{E_eriTest,ERI_TST_TEST_EN_COURS,"Test en cours"},
	{E_eriMajeure,ERI_DEF_ALIM_AFF,"Defaut alimentation"},

	{-1,-1,null},
} ;


/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

static const char *_stEriLibelleGet(int32 niveau_dw,int32 numero_dw )
{
	int32 cour_dw=0;
	const char *libelle_pt="";
	while(_eriLibelle[cour_dw].gravite_dw!=-1) {
		if((_eriLibelle[cour_dw].gravite_dw==niveau_dw)&&(_eriLibelle[cour_dw].numero_dw==numero_dw)) {
			libelle_pt=_eriLibelle[cour_dw].libelle_pt;
		}
		cour_dw++;
	}
	return libelle_pt;

}


static int _stEriBuildError(char *chaine,int maxLen,int32 niveau_dw,int32 numero_dw,const char *info_pt,const char *infoExt_pt,bool ext_b)
{
	uint16 chaineLen_uw=0;
	chaine[0]=0;
	if( ext_b ) {
		char *gravite_pt = " ";
		const char *libelle_pt = " ";
		switch( niveau_dw ) {
			case E_eriMineure:
				gravite_pt = "w";
				break;
			case E_eriMajeure:
				gravite_pt = "E";
				break;
			case E_eriTest:
				gravite_pt = "t";
				break;
		}
		libelle_pt = _stEriLibelleGet( niveau_dw, numero_dw );
		chaineLen_uw=snprintf(chaine,maxLen,"%s%s 10%01d%02d %20s", LCR_ENDLINE,gravite_pt,niveau_dw,numero_dw, libelle_pt );
		if( null != info_pt ) {
			if( 0 != strlen( info_pt ) ) {
				chaineLen_uw+=snprintf(&chaine[chaineLen_uw],maxLen-chaineLen_uw, " %s", info_pt );
				if( null != infoExt_pt ) {
					if( 0 != strlen( infoExt_pt ) ) {
						chaineLen_uw+=snprintf(&chaine[chaineLen_uw],maxLen-chaineLen_uw, ".%s", infoExt_pt );
					}
				}
			}
		}
	} else {
		chaineLen_uw=snprintf(chaine,maxLen," %01d%02d", niveau_dw,numero_dw );
		if( null != info_pt ) {
			if( 0 != strlen( info_pt ) ) {
				chaineLen_uw+=snprintf(&chaine[chaineLen_uw],maxLen-chaineLen_uw, "/%s", info_pt );
				if( null != infoExt_pt ) {
					if( 0 != strlen( infoExt_pt ) ) {
						chaineLen_uw+=snprintf(&chaine[chaineLen_uw],maxLen-chaineLen_uw, ".%s", infoExt_pt );
					}
				}
			}
		}
	}
	return chaineLen_uw;
}


/* --------------------------------
 * lcr_st_eri
 * =============
 * Analyse de la commande "ST ERI"
 * --------------------------------	*/

int lcr_st_eri(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess) {
	char *ptCour = (char*) &buffer[position];
	int lgCour = lg_mess - position;
	int modifConfig = 0;
	int nbErreur;
	int bloc = 0;
	bool ext_b=false;
	int erreur=0;

	ptCour += strlen(ST_ERI_CMD);
	lgCour -= strlen(ST_ERI_CMD);
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	while ((lgCour > 0) && (0 == erreur)) {
		if (strncmp(ptCour, LCR_TST_INFO_ERI_EXT, strlen(LCR_TST_INFO_ERI_EXT))
				== 0) {
			ext_b=true;
		} else {
			erreur = 3;
		}
		ptCour++;
		lgCour--;
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	}
	nbErreur = eriLireNbErreur();
	if ((0 == nbErreur) || (2 == configGetModeDegrade())) {
		int j = 0;
		j = sprintf(buffer, "%s 0","STATUS_ERI");
		tedi_send_bloc(las, mode, buffer, j, bloc, TRUE, pt_mess);
	} else {
		int j = 0;
		int indCour;
		j = sprintf(buffer,"%s","STATUS_ERI");
		for (indCour = nbErreur - 1; indCour >= 0; indCour--) {
			char grav, nature;
			char temp[MAX_MESSAGE_LCR];
			char modStr[20]="";
			char caisStr[20]="";
			char mod, cais;
			int val;
			eriLireErreur(indCour, &grav, &nature);

			eriLireErreurModule(indCour, &mod, &cais, &val);
			if (0 != mod) {
				snprintf(modStr,sizeof(modStr), "/%d", mod);
				if (((char) N_AFF) != cais) {
					int caisLen=snprintf(caisStr,sizeof(caisStr), ".%d", cais);
					if (255 != val) {
						caisLen+=snprintf(&caisStr[caisLen],sizeof(caisStr)-caisLen, ".%d", val);
					}
				}
			}

			_stEriBuildError(temp,sizeof(temp),grav,nature,modStr,caisStr,ext_b);

			j += sprintf(&buffer[j], "%s", temp);
			tedi_test_bloc(las, mode, buffer, &j, &bloc, pt_mess);


		}
		tedi_send_bloc(las, mode, buffer, j, bloc, TRUE, pt_mess);
	}
	/* ------------------------------------
	 * FIN DE lcr_st_eri
	 * ------------------------------------ */
	return modifConfig;
}
int lcr_tst_info_eri(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess) {
	char *ptCour = (char*) &buffer[position];
	int lgCour = lg_mess - position;
	int modifConfig = 0;
	int nbErreur;
	int bloc = 0;
	int erreur=0;
	bool ext_b=true;


	ptCour += strlen(LCR_PCP_TST_INFO_ERI);
	lgCour -= strlen(LCR_PCP_TST_INFO_ERI);
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	while ((lgCour > 0) && (0 == erreur)) {
		if (strncmp(ptCour, LCR_TST_INFO_ERI_EXT, strlen(LCR_TST_INFO_ERI_EXT))
				== 0) {
			ext_b=true;
		} else {
			erreur = 3;
		}
		ptCour++;
		lgCour--;
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	}
	nbErreur = eriLireNbErreur();
	if ((0 == nbErreur) || (2 == configGetModeDegrade())) {
		int j = 0;
		j = sprintf(buffer, "%s 0",LCR_PCP_TST_INFO_ERI);
		tedi_send_bloc(las, mode, buffer, j, bloc, TRUE, pt_mess);
	} else {
		int j = 0;
		int indCour;
		j = sprintf(buffer, "%s",LCR_PCP_TST_INFO_ERI);
		for (indCour = nbErreur - 1; indCour >= 0; indCour--) {
			char grav, nature;
			char temp[MAX_MESSAGE_LCR];
			char modStr[20]="";
			char caisStr[20]="";
			char mod, cais;
			int val;
			eriLireErreur(indCour, &grav, &nature);

			eriLireErreurModule(indCour, &mod, &cais, &val);
			if (0 != mod) {
				snprintf(modStr,sizeof(modStr), "/%d", mod);
				if (((char) N_AFF) != cais) {
					int caisLen=snprintf(caisStr,sizeof(caisStr), ".%d", cais);
					if (255 != val) {
						caisLen+=snprintf(&caisStr[caisLen],sizeof(caisStr)-caisLen, ".%d", val);
					}
				}
			}

			_stEriBuildError(temp,sizeof(temp),grav,nature,modStr,caisStr,ext_b);

			j += sprintf(&buffer[j], "%s", temp);
			tedi_test_bloc(las, mode, buffer, &j, &bloc, pt_mess);

		}
		tedi_send_bloc(las, mode, buffer, j, bloc, TRUE, pt_mess);
	}
	/* ------------------------------------
	 * FIN DE lcr_st_eri
	 * ------------------------------------ */
	return modifConfig;
}
