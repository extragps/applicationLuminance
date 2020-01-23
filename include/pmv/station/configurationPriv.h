#ifndef CONFIGURATION_PRIV_H_
#define CONFIGURATION_PRIV_H_
/* ************************************
 * DEFINITION DES TYPES LOCAUX
 * ************************************	*/

typedef struct _ConfigStatiqueConfigStatiqueConfigStatiqu
{
//	int32 numCaisson;
//	int32 numMsg1;
//	int32 numMsg2;
	int32 seuilJour_dw;
	int32 seuilSurb_dw;
	int32 tempoTest_dw;
	int32 periodeTest_dw;
	int32 tempoAnim_dw;
	int32 periodeScrutAff_dw;
	int32 periodeScrutThl_dw;
	int32 periodeScrutEs_dw;
	int32 valNuit;
	int32 valJour;
	int32 valSurb;
	int32 seuilPixelMinAff;
	int32 seuilPixelMaxAff;
	int32 seuilPixelMinMod;
	int32 seuilPixelMaxMod;
	int32 nbAfficheurs;
	int32 adresseAfficheur[AFFICHEUR_NB_MAX];
	int32 sortieAfficheur[AFFICHEUR_NB_MAX];
	int32 caissonAfficheur[AFFICHEUR_NB_MAX];
	int32 nbThls;
	int32 adresseThl[THL_NB_MAX];
	int32 modeThl[THL_NB_MAX];
	int32 adresseEs;
	int32 periodeTestPixel_dw;
//	uint8 *adresseBsc_pt;
//	bool demarrage_b;ConfigStatique
//	int32 vitesseLas1_dw;
//	int32 vitesseLas2_dw;
	int32 numPort_dw;
//	int32 scrutationPC_dw;
	int32 seuilChauffage_dw;
//	int32 scrutationMaint_dw;
//	int32 scrutationIp_dw;
	int32 paddingBefore_dw;
	int32 paddingAfter_dw;

} ConfigStatique;

typedef struct _ConfigDynamique
{
	int32 valCour_dw;	/* Valeur courante de la luminosite */
	bool on_b;
	bool ok_b;
	bool surb_b;
	bool nuit_b;
	uint8 lumCour_uc;
	uint8 lumSeuil_uc;
	bool  lumChanged_b[NB_CAISSON];
	bool  lumAuto_b[NB_CAISSON];
	uint8 lumVal_uc[NB_CAISSON];
	bool  enCours_b[NB_CAISSON];
	int32 etatAfficheur_dw[AFFICHEUR_NB_MAX];
	bool reprise_b;
	int32 numAffichage_dw;
	bool chargementEnCours_b;
	bool init_b;
	int32 tempoScrut_dw;
	bool rebouclage_b;
} ConfigDynamique;


#endif
