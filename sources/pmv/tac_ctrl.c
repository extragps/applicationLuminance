/*
 * TAC_CTRL.c
 * ===================
 *
 * Objet : Contient la tache principale assurant les traitements
 * periodiques realises par l'application.
 * --------------------------------------------------------
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: TAC_CTRL.c,v $
 * Revision 1.3  2019/10/10 14:12:17  xg
 * Supppression des references au tpe.
 *
 * Revision 1.2  2018/06/04 08:38:43  xg
 * Passage en version V122
 *
 * Revision 1.9  2008/11/05 09:01:05  xag
 * Ajout du reset des traces lors du forcage de la configuration.
 *
 * Revision 1.8  2008/10/13 08:18:17  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.7  2008/10/03 16:21:47  xag
 * *** empty log message ***
 *
 * Revision 1.6  2008/09/29 07:58:54  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.5  2008/09/22 13:40:02  xag
 * Test de la  base de donnees au demarrage.
 *
 * Revision 1.4  2008/09/22 07:53:41  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.3  2008/09/12 14:59:04  xag
 * Archivage de printemps
 *
 * Revision 1.2  2008/07/04 17:05:06  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:30:15  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include "standard.h"
#include "portage.h"
#include "config.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "ted_prot.h"
#include "ypc_las.h"
#include "lcr_cfpp.h"
#include "lcr_init.h"
#include "lcr_sc.h"
#include "tac_conf.h"
#include "lcr_p.h"
#include "tac_ctrl.h"
#include "x01_vcth.h"
#include "Tache.h"
#include "mon_debug.h"
#include "tac_conf.h"
#include "ficTrace.h"
#include "etaSyst.h"
#include "lcr_cf3h.h"
#include "lcr_trc.h"
#include "stAlLib.h"
#include "lcr_cftp.h"
#include "lcr_cfpl.h"
#include "bpTestLib.h"
#include "tempLib.h"
#include "pipTest.h"

/* Nouvelles declation pour la gestion de la configuration */
#include "configuration.h"
#include "configurationSram.h"
#include "sramMessage.h"
#include "io/iolib.h"
#include "cpu432/cpu432.h"
#include "affichage/caractere.h"

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* *************************************
 * PROTOTYPES DES FONCTIONS staticES
 * ************************************* */

static int tac_ctrl_reactivation = FALSE;

/* *************************************
 * FONCTIONS DU MODULE
 * ************************************* */

//static void tac_ctrl_bp_appui(voi)
//{
//	setEtatCour(1);
//	taskDelay(250);
//	setEtatCour(0);
//}
//
//void testAppuiBp()
//{
//	tacheSpawn("bpTest", 90, 0, 100, (FUNCPTR) tac_ctrl_bp_appui, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//}
static VOID tac_ctrl_aff_horo_date(VOID);
static VOID tac_ctrl_relance(INT, INT);
static VOID tac_ctrl_reactiver(VOID);
static VOID tac_ctrl_relance_secteur(VOID);

/*
 *  tac_ctrl_coupure_secteur
 *  ========================
 *  Pour la mise a jour periodique de l'heure afin de pouvoir
 *  gerer la duree des coupures secteur.
 */

VOID tac_ctrl_coupure_secteur() {
	Ktimestamp date_t;
	ktsGetTime(&date_t);

	configSetDateCour(&date_t);
}
/*DOC***************************************************************************
 *  SIAT.  *   FICHE DE  FONCTION:  tac_ctrl_init
 ********************************************************************************
 * OBJET DE LA FONCTION : module d'init de la tache de controle
 * PARAMETRES           :
 * VALEURS RENDUES      :
 * FONCTIONS APPELEES   :
 * FONCTIONS APPELANTES :
 ***************************************************************************DOC*/
VOID tac_ctrl_init(VOID) {
	INT i;
	BYTE reinit;
	UINT val;
	INT preinit = FALSE;
	UINT8 sauvegarde_config = FALSE;
	/* TODO : Remplacer par un controle correct de la base */

	printDebug("rec_main_init\n");
	/* Initialisation par defaut de la configuration */
	configInit();
	caracteresInitTable();

	/* Coupure systematique de l'ali
	 * mentation des detecteurs deportes */
	/* ioPortEcrireAlimDetecteur (FALSE); */
	/* on positionne le flag de pre-ini
	 * tialisation */
	/* on initialise l'adresse et la
	 * taille de la memoire fichier */

	/* a priori il ne faudra pas sauve-
	 * garder la config */
	if (cpu432SW1IsActive( IO_SWITCH_CFG )) {
		eriAjouter(E_eriMineure, ERI_DEF_FORCAGE_CONFIG);
		val = 1;
		printDebug("tac_ctrl_init, Forcage de la configuration %#0x\n", val);
	} else {
		val = 0;
	}

	/* Controle de l'entete */
	switch (tac_conf_cfg_lec_entete()) {
	case 0:
		eriSupprimer(E_eriMajeure, ERI_DEF_CONFIG);
		/* la config est bonne le numero de scenario egalement */
		break;

	case 2:
		eriSupprimer(E_eriMajeure, ERI_DEF_CONFIG);
		/* la config est bonne mais pas le num de scenario  */
		sauvegarde_config = TRUE;
		break;

	default:
		printf("La configuration n'est pas bonne \n");
		/* la config n'est pas bonne */
		eriAjouter(E_eriMajeure, ERI_DEF_CONFIG);
		val = 1;
		break;
	}

	if (val & 1) {
		printDebug("rec_main_init : l'entete n'est meme pas bonne"
				"ou il y a un forcage\n");
		ksleep(50);
		/* l'entete n'est meme pas bonne ou
		 * il y a un forcage */
		preinit = TRUE;
		/* seul le scenario 1 est actif */
		tac_conf_cfg_raz_entete();
		/* il faut sauvegarder la config */
		sauvegarde_config = TRUE;
		/* Reinitialisation des fichiers
		 * de traces et de mesures.     */
	} else {
		/* on controle la zone de config
		 * application */
		if (!tac_conf_cfg_lec_conf()) {
			preinit = TRUE;
			/* il faut sauvegarder la config */
			sauvegarde_config = TRUE;
			eriAjouter(E_eriMajeure, ERI_DEF_CONFIG);
		}
	}

	/* Initialisation de la sram.... */
	/* TODO : Faire eventuellement un controle de la configuration */

	/* Traiter en cas de sram correcte... */

	if (1) {
		Ktimestamp dateFin_t;
		Ktimestamp dateDebut_t;
		float64 diff_sect = 0;

		configGetDateCoupure(&dateDebut_t);
		configGetDateReprise(&dateFin_t);

		diff_sect = kGetDeltaTimestamp(&dateDebut_t, &dateFin_t);

		configSetDiffSect((uint32) diff_sect);

		/* Positionnement du delta */
		/* Increment du compteur qui va bien */
		if (diff_sect <= 1.) {
			/* Increment du compteur secondes... */
			configIncrCptUn();
		} else if (diff_sect <= 1.) {
			configIncrCptDix();
		} else {
			/* Increment du compteur global... */
			configIncrCptPlus();
		}
	} else {
		/* réaliser une initialisation des fichiers de trace... */
	}

	/* Calcul du checksum de l'appli-
	 * cation.      */
	{
		unsigned long cks_prom = tac_conf_cfg_calculer_checksum_appli();
		/* Il faut controler le checksum de l'application.... */
		vct_cks_prom_prom = vct_cks_prom;
		if (cks_prom != vct_cks_prom) {
			/* Est ce une mise a jour de l'application??? 			 */
			/* Sinon, il y a un probleme de checksum.... */
			vct_cks_prom = cks_prom;
			sauvegarde_config = TRUE;
			eriAjouter(E_eriMajeure, ERI_DEF_CKS_PROGRAMME);
		}
	}
	/* Test de l'integrite et a defaut
	 * on realise une reinitalisation des
	 * fichiers.    */
	/* on reinitialise la table des
	 * defauts capteur */
	/* si une pre_initialisation est
	 * necessaire... */
	/* Increment des compteurs de restart
	 * watchDog ou Restart */
	{
		bool gar_b;

		ioGetGar(&gar_b);
		if (true == gar_b) {
			configIncrGar();
		}
	}
	configIncrRst();

	/* Preinitialisation eventuelle */
	if (preinit) {
		tac_conf_cfg_raz_conf();
		sauvegarde_config = TRUE;
	}
	/* endif(preinit                                        */
	if (ioIsResetCfg()) {
		cf3_conf_memoire();
		cf3_init_fic_tra();
		/* Ajout de la premiere trace.... */
		cmd_trc_tr("FORCAGE CONFIGURATION", NULL);
	} else if (-1 == ficTraceTester()) {
		/* A faire surrement dans le cas du PMV */
		cf3_conf_memoire();
		cf3_init_fic_tra();

		cmd_trc_tr("DEFAUT INTEGRITE BASE DETECTE", NULL);
	}
	/* si on doit sauvegarde la config */
	if (sauvegarde_config) {
		tac_conf_cfg_ecr_entete();
		tac_conf_cfg_ecr_conf();
		vct_conf = 0;
	} else {
		trt_alt_tester_alerte();
	}

	/* l'arborescence */
	lcr_cftp_arborescence();

	/* on met a l'heure */
	mnlcal(x01_date);
	/* Comme la carte n'a pas de pile, il peut etre interessant de
	 * forcer la sauvegarde de l'heure lorsque l'on detecte le
	 * probleme. */
	{
		if (mnlcalSec() < 60) {
			mnecal(x01_date);
		}
		if (!dv1_test_date(x01_date)) {
			dv1_init_date();
			/* on relie l'heure */
			mnlcal(x01_date);
		}
	}

	/* si c'est une reprise secteur */
	reinit = TRUE;
	/* on calcul le bcc */

	/* Recuperation des donnees sauvegardees pour les caissons... */

	//  La reinitialisation des caissons intervient eventuellement en amont lors de l'initialisation.
	//
	//	{
	//
	//		ULONG bcc_pip;
	//		ULONG pip_sv_act_caisson_bcc;
	//		sramRead(sram_pt, (int32) & sram_var->pip_sv_act.caisson, NB_CAISSON, sizeof(T_pip_sv_act_caisson),
	//				pip_sv_sv_act_caisson);
	//		bcc_pip = dv1_chk((STRING) & pip_sv_sv_act_caisson[0], (STRING) & pip_sv_sv_act_caisson[NB_CAISSON]);
	//		sramRead(sram_pt, (int32) & sram_var->pip_sv_act.bcc, 1, sizeof(ULONG), &pip_sv_act_caisson_bcc);
	//		if (pip_sv_act_caisson_bcc != bcc_pip)
	//		{
	//			printDebug("Valeur calc %#0x stockee %#0x\n", bcc_pip, pip_sv_act_caisson_bcc);
	//			pip_sv_act_caisson_bcc = 0xFFFFFFFF;
	//			sramWrite(sram_pt, (int32) & sram_var->pip_sv_act.bcc, 1, sizeof(ULONG), &pip_sv_act_caisson_bcc);
	//		}
	//
	//	}
	/* si on doit reinitialise */
	if (reinit) {
		/* c'est une reinit */
		configIncrReinit();
	}

	x01_status3.rebouclage = 0;

	for (i = 0; i < NBPORT; i++) {
		x01_status3.tempo_mode_terminal[i] = 0;
	}

	x01_status3.eri = 0;
	x01_status3.eri_rebouclage = 0;
	dv1_scpy(x01_status3.mod_rebouclage, (STRING) "00  1.1 1.2 3.4", 0);
	x01_status3.edf = '0';
	x01_status3.grp = '0';
	x01_status3.jn = 'N';
	x01_status3.srb = '0';
	x01_status3.trm = '0';
	x01_status3.verrouillage = FALSE;
	x01_status3.err = 0;
	x01_status3.er1 = 0;
	x01_status3.er2 = 0;
	x01_status3.er3 = 0;

	x01_bat = 0;

	if (pip_nb_module == 0) {
		/* il n'y a pas de configuration chargee */
		x01_cptr.configuration = FALSE;
	} else {
		/* il n'y a une configuration */
		x01_cptr.configuration = TRUE;
	}

	/* on raz les tempo d'attente config */

	/* il n'y a pas de configuration en cours */
	x01_cptr.config = 0;

	/* il n'y a pas de relance */
	x01_cptr.relance = 0;

	/* on est en distant */
	x01_status3.distant = TRUE;

	/* les TS de forcage ne sont pas actives */
	/* la duree de la coupure secteur */
	/* Initialisation pour le bouton de test */
	bpTestInit();
	/* Temporisation pour les autotests */
	pipLonInitTest();

}

static void tac_ctrl_autotests_diodes(void) {
	if (mnGetTempo(TP_TEST_DIODES) == TP_FINI) {
		pipLonInitTest();
		pipLonSetTest(1);
	}
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tac_ctrl_emission                                       /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/97                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tac_ctrl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : emission reception vers l'application                  /
 /-------------------------------------------------------------------------DOC*/
INT tac_ctrl_emission(STRING buffer, INT nbcar) {
	struct usr_ztf message;
	T_usr_ztf *pt_message = &message;
	mqd_t ancrage;

	/* l'ancrage de retour */
	ancrage = SysMQCreer();
	/* on indique l'adresse de retour */
	pt_message = monMessInit(ancrage, &message);
	message.nbcar_rec = nbcar;
	strncpy((void*) message.buffer, (void*) buffer, min(nbcar, SZ_BLK * 2));
	message.nbcar_max = 30;
	/* on emet le message sur l'ancrage interne */
	monMessSend(x01_ancrage_interne, &pt_message, sizeof(pt_message));
	/* on se met en attente sur l'ancrage de retour */
	if (0 != monMessRec(ancrage, 5000, NULL)) {
	}
	return (message.nbcar_max);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tac_ctrl_aff_hoho_date                                 /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/97                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tac_ctrl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : affichage de l'heure ou de la temperature sur le panneau/                                /
 /-------------------------------------------------------------------------DOC*/
static VOID tac_ctrl_aff_horo_date() {

	BYTE trv;
	int k, j, i;
	j = 0;
	for (i = 0; i < pip_nb_module; i++) {
		T_pip_cf_module *module = &pip_cf_module[i];
		/* si module alpha */
		switch (pip_cf_caisson[i].type) {
		case TYPE_ALPHA:
			for (j = module->num_caisson;
					j < module->num_caisson + module->nb_caisson; j++) {
				T_pip_sv_act_caisson caisson;
				T_pip_sv_act_caisson *pip_sv_act_caisson = &caisson;
				sramMessageLireCaisson(j, pip_sv_act_caisson);
				trv = FALSE;

				/* on recherche si il y a un message predefini */
				for (k = 0; k < pip_sv_act_caisson->nbAlternance; k++) {
					if (pip_sv_act_caisson->type_predef[k] == TYPE_HR) {
						trv = TRUE;
					} else if (pip_sv_act_caisson->type_predef[k] == TYPE_DT0) {
						trv = TRUE;
					} else if (pip_sv_act_caisson->type_predef[k] == TYPE_DT1) {
						trv = TRUE;
					}
				}

				/* si il y a un message predefini on genere la commande */
				if (trv) {
					if (pip_sv_act_caisson->validite != 0) {
						pip_sv_act_caisson->validite_dem =
								pip_sv_act_caisson->validite;
					}
					sramMessageEcrireCaisson(j, pip_sv_act_caisson);
					activation_caisson(j);
				}
			}
			break;
		}
	}
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tac_ctrl_seconde                                       /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/97                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tac_ctrl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : gestion des tempos seconde                             /
 /-------------------------------------------------------------------------DOC*/
static int tac_ctrl_seconde() {
	INT i, j;
	char buffer[100];
	INT nbcar;
	INT relance = FALSE;

	/* les durees de validite pour la sauvegarde */
	for (i = 0; i < NB_CAISSON; i++) {
		T_pip_sv_act_caisson caisson;
		T_pip_sv_act_caisson *pip_sv_act_caisson = &caisson;
		sramMessageLireCaisson(i, pip_sv_act_caisson);
		if (pip_sv_act_caisson->validite > 1) {
			pip_sv_act_caisson->validite--;
			sramMessageEcrireCaisson(i, pip_sv_act_caisson);
		}
	}

	/* pour tous les modules */
	for (i = 0; i < pip_nb_module; i++) {
		T_pip_cf_module *module = &pip_cf_module[i];
		/* surveillance du PC */
		if (pip_act_module[i].spc != 0) {
			pip_act_module[i].spc--;
			if (pip_act_module[i].spc == 0) {
				/* on met le panneau au neutre */
				cmd_trc_mat(1, "EXT DEFAUT PC AM=%d ", module->id_module);

				nbcar = sprintf(buffer, "PN AM=%d AF=0", module->id_module);

				/* on genere la commande */
				tac_ctrl_emission(buffer, nbcar);
			}
		}
		/* les duree de validite */
		j = module->num_caisson;
		while (j < module->num_caisson + module->nb_caisson) {
			/* validite */
			if (pip_act_caisson[j].validite != 0L) {
				pip_act_caisson[j].validite--;
				if (pip_act_caisson[j].validite == 0L) {
					T_pip_sv_act_caisson caisson;
					T_pip_sv_act_caisson *pip_sv_act_caisson = &caisson;
					sramMessageLireCaisson(j, pip_sv_act_caisson);

					pip_sv_act_caisson->finDv = 1;
					pip_act_caisson->relance_b = true;
					relance = TRUE;

					sramMessageEcrireCaisson(j, pip_sv_act_caisson);

				}
			}
			/* on passe au caisson suivant */
			j++;
		}
		/* on passe au module suivant */
	}
	/* pour l'interdiction du mode terminal */
	for (i = 0; i < NBPORT; i++) {
		/* on decremente la tempo terminal */
		if (x01_status3.tempo_mode_terminal[i])
			x01_status3.tempo_mode_terminal[i]--;
	}
	return relance;
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tac_ctrl_relance                                       /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/97                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tac_ctrl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : relance de l'affichage apres coupure secteur           /
 /-------------------------------------------------------------------------DOC*/
static VOID tac_ctrl_relance(INT indCaisson, INT diff_sect) {
	T_pip_sv_act_caisson caisson;
	T_pip_sv_act_caisson *action = &caisson;
	sramMessageLireCaisson(indCaisson, action);

	if (action->nbAlternance && (!action->neutre)) {
		/* si la duree de validite n'est pas infinie */
		if (action->validite != 0) {
			/* Interpretation non parfaite de la commande car on
			 * transforme la duree de validite d'origine.*/
			printDebug("Test validite %d pour %d\n", diff_sect,
					action->validite);
			if (diff_sect < action->validite) {
				printDebug("Validite demandee %d\n", action->validite_dem);
				action->validite_dem = action->validite - (LONG) (diff_sect);
			} else {
				action->validite = 0L;
				action->nbAlternance = 0;
				action->neutre = TRUE;
			}
			sramMessageEcrireCaisson(indCaisson, action);
		}
	}
	activation_caisson(indCaisson);
}
//
//static VOID tac_ctrl_fin_dv(INT indCaisson)
//{
//	T_pip_sv_act_caisson caisson;
//	T_pip_sv_act_caisson *action = &caisson;
//	sramMessageLireCaisson(indCaisson, action);
//
//	if (action->nbAlternance && (!action->neutre))
//	{
//		action->validite_fin = 1;
//		action->validite = 0L;
//		action->nbAlternance = 0;
//		action->neutre = TRUE;
//		sramMessageEcrireCaisson(indCaisson, action);
//	}
//	activation_caisson(indCaisson);
//}
//
/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tac_ctrl_relance_secteur                                 /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/97                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tac_ctrl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : relance de l'affichage apres coupure secteur           /
 /-------------------------------------------------------------------------DOC*/
VOID tac_ctrl_relance_secteur() {
	INT i;
	for (i = 0; i < pip_nb_module; i++) {
		int j;
		T_pip_cf_module *module = &pip_cf_module[i];
		int num_caisson = module->num_caisson;
		int nb_caisson = module->nb_caisson;

		for (j = num_caisson; j < num_caisson + nb_caisson; j++) {
			tac_ctrl_relance(j, configGetDiffSect());
		}
	}
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tac_ctrl_relance                                       /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/97                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tac_ctrl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : relance de l'affichage apres defaut                    /
 /-------------------------------------------------------------------------DOC*/

void tac_ctrl_set_force(int val) {
	tac_ctrl_reactivation = val;
}

int tac_ctrl_get_force() {
	return tac_ctrl_reactivation;
}

/*
 * tac_ctrl_reactiver_force
 * ========================
 * Attention, la fonction est maintenant utilisee
 * dans le sequenceur. Il faudrait prevoire de la déplacer,
 * elle et ses soeurs.
 * */

VOID tac_ctrl_reactiver_force() {
	int i = 0;
	printDebug("tac_ctrl_reactiver_force\n");
	/* pour tous les modules */
	tac_ctrl_set_force(FALSE);
	for (i = 0; i < pip_nb_caisson; i++) {
		pip_act_caisson[i].relance_b = true;
	}
	tac_ctrl_reactiver();
}

static VOID tac_ctrl_reactiver() {
	INT i;

	for (i = 0; i < pip_nb_module; i++) {
		int j;
		T_pip_cf_module *module = &pip_cf_module[i];
		int num_caisson = module->num_caisson;
		int nb_caisson = module->nb_caisson;

		for (j = num_caisson; j < num_caisson + nb_caisson; j++) {
			if (pip_act_caisson[j].relance_b) {
				pip_act_caisson[j].relance_b = false;
				tac_ctrl_relance(j, 1);
			}
		}
	}

}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tac_ctrl_vt                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/97                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tac_ctrl_vt.c                                       /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : relance de l'affichage apres defaut                    /
 /-------------------------------------------------------------------------DOC*/
static VOID tac_ctrl_vt() {
	BYTE i;
	for (i = 0; i < NBPORT; i++) {
		if (xdg_vt[i].tempo != 0) {
			if (--xdg_vt[i].tempo == 0) {
				xdg_vt[i].port = N_AFF;
				xdg_vt[i].pt_in = 0;
				xdg_vt[i].pt_out = 0;
			}
		}
	}
}

/* --------------------------------
 * tac_ctrl_valid_config
 * =====================
 * Ecriture de la configuration en
 * memoire FLASH
 * --------------------------------	*/

VOID tac_ctrl_valid_config() {
	printDebug("DEBUT ECRITURE CONFIGURATION %d \n\r", 0);
	/* on annule la tempo de validation
	 * de la configuration */
	mnSetTempo(TP_VALID_CONFIG, TP_HS);
	/* on sauvegarde l'entete */
	tac_conf_cfg_ecr_entete();
	printDebug("DEBUT ECRITURE CONFIGURATION %d \n\r", vct_num_scenario_actif);
	/* on sauvegarde la zone de donnee */
	tac_conf_cfg_ecr_conf();
	printDebug("FIN ECRITURE CONFIGURATION %d \n\r", 0);
	mnsusp(1);
}

/*
 * tac_ctrl
 * ========
 * OBJET DE LA FONCTION : tache de controle du module PIP
 * PARAMETRES           :
 * VALEURS RENDUES      :
 * FONCTIONS APPELEES   :
 * FONCTIONS APPELANTES :
 * */
VOID tac_ctrl_main(int numero) {
	BYTE i;
	BYTE cptSec;
	ULONG sec = 0;
	uint32 diffSect_udw;

	/* -------------------------
	 * SYNCHRONISATION
	 * ------------------------ */
	mnsuspIniPrendre();
	/* on met a l'heure */
	mnlcal(x01_date);
	if (!dv1_test_date(x01_date)) {
		dv1_init_date();
		/* on relie l'heure */
		mnlcal(x01_date);
	}

	/* on initialise l'application PIP  */
	lcr_init_cmd();

	x01_cptr.activation_secteur = FALSE;

	/* Controle du checksum de la zone de configuration des
	 * caissons. */

	{
//		ULONG pip_sv_act_caisson_bcc;
//		sramRead(sram_pt, (int32) & sram_var->pip_sv_act.bcc, 1, sizeof(ULONG), &pip_sv_act_caisson_bcc);
//
//		if (pip_sv_act_caisson_bcc != 0xFFFFFFFF)
		{
			INT nbActivation = 0;
			for (i = 0; i < NB_CAISSON; i++) {
				T_pip_sv_act_caisson cais;
				sramMessageLireCaisson(i, &cais);
				if (cais.nbAlternance) {
					x01_cptr.activation_secteur = TRUE;
					nbActivation++;
				}
			}
			cmd_trc_mat(1, "BCC ACTIVATION OK NB CAIS.=%d", nbActivation);

		}
//		else
//		{
//			cmd_trc_mat(1, "BCC ACTIVATION INCORRECT %d", 0);
//			eriAjouter(E_eriMajeure, ERI_DEF_CKS_PIP);
//		}
	}

	diffSect_udw = configGetDiffSect();
	/* selon la duree de la coupure secteur */
	if ((pip_cf_tst_sc.diff_sect > 0)
			&& (pip_cf_tst_sc.diff_sect < diffSect_udw)) {
		printf("Param %d diff_sect %d\n", pip_cf_tst_sc.diff_sect,
				diffSect_udw);
		x01_cptr.activation_secteur = FALSE;
	} else {
		printDebug("Pas d'activation secteur\n");
	}

	if (!x01_cptr.activation_secteur) {
		/* Reinitialisation de la structure des actions!!! */
		lcr_init_sv_act_pip();
		x01_cptr.activation_secteur = TRUE;
		//		configSetReactivationSurCoupure(false);
		printf("RELANCE NK DUREE COUPURE %06lu sec\n", diffSect_udw);
		cmd_trc_mat(1, "RELANCE NK DUREE COUPURE %06lu sec", diffSect_udw);
	} else {
		/* la sauvegarde est bonne */
		//		configSetReactivationSurCoupure(true);
		/* on trace la relance */
		cmd_trc_mat(1, "RELANCE OK DUREE COUPURE %06lu sec", diffSect_udw);
		printf("RELANCE OK DUREE COUPURE %06lu sec\n", diffSect_udw);
	}

	/* on initialise la tempo de reprise secteur */
	/* a priori le secteur est present */
	rqtemp[TP_VALID_CONFIG] = TP_HS;
	rqtemp[TP_SECT] = TEMPO_SECTEUR;

	/* Voir avec Nicolas la signification des straps */
	x01_status3.strapRebouclage = FALSE;
	x01_status3.strapForcageInput = FALSE;

	/* on initialise le compteur de seconde */
	/* on demarre */

	cptSec = 0;
	while (1) {
		int changeSec = FALSE;
		int relance = FALSE;
		/* TODO : Il faut proceder autremement pour la boucle. */
		if (x01_status3.strapRebouclage) {
			/* on force le rebouclage  */
			x01_status3.rebouclage = 1;
		}

		/* on lit l'heure courante */
		mnlcal(x01_date);

		/* retour au moniteur */
		mnsusp(0);
		/* si on doit passer en mode BOOT */
		/*   N'existe plus.
		 *
		 *    if (rqtemp[TP_MODE_BOOT] == TP_FINI)
		 {
		 rqtemp[TP_MODE_BOOT] = TP_HS;
		 printDebug ("Avant mnrestart\n");
		 mnrestart ();
		 printDebug ("Apres mnrestart\n");
		 } */
		if (mnGetTempo(TP_VALID_CONFIG) == TP_FINI) {
			printDebug("Avant rec_main_alid\n");
			tac_ctrl_valid_config();
			printDebug("Apres rec_main_valid\n");
		}
		/* TEST DU BOUTON POUSSOIR */

		/* if (DEMARRAGE_OK == x01_cptr.demarrage) */
		{
			if (bpTest()) {
				tac_ctrl_set_force(TRUE);
			}
		}

		/* Pour detection du changement de seconde */
		{
			ULONG secCour = mnlcalSec();
			if (secCour != sec) {
				sec = secCour;
				changeSec = TRUE;
				rqflc2 = TRUE; /* Pour temporisation de la tache lon */
			}
		}
		if (changeSec) {
//			tac_ctrl_io();
//			tac_ctrl_status_temps_reel(); /* on traite le status temps reel */
//			tac_ctrl_autotests_diodes();
			/* si le flag de seconde est actif et si le secteur est present */
			//			if (lon_status.secteur == TEMPO_ATTENTE_SECTEUR)
			{
				/* on decremente les tempo de surveillance de la transmission */
				for (i = 0; i < pip_nb_caisson; i++) {
					/* si la tempo n'est pas nulle on la decremente */
					if (pip_act_caisson[i].cpt_attente_relance)
						pip_act_caisson[i].cpt_attente_relance--;

				}
				/* on decremente la tempo de surveillance du lon */

				//				for (i = 0; i < NB_CRT_LON100 + NB_PSEUDO_LON; i++)
				//				{
				//					if ((lon_status.tp_lon[i]) && (lon_status.presenceCarteLon[i]))
				//					{
				//						if (--lon_status.tp_lon[i] == 0)
				//						{ /* absence de com sur le lon */
				//							lon_status.lon_ok[i] = FALSE;
				//							/* on signale une deconnexion du LON */
				//							cmd_trc_mat(1, "DECONNEXION DU RESEAU LON %d", i);
				//						}
				//					}
				//				}
				/* pour la commande VT */
				tac_ctrl_vt();

				/* on attend que l'application ait completement demarre */
				//				switch (x01_cptr.demarrage)
				//				{
				//				case DEMARRAGE_OK:
				//					/* si la premiere surveillance est effectue */
				//					if (x01_cptr.surveillance > 2)
				//					{
				//						/* si une mise au neutre est demandee */
				//						if (x01_cptr.mise_au_neutre)
				//						{
				//							/* TODO : Pour l'heure, on ne fait pas de traitement
				//							 * dans ce cas precis. */
				//							x01_cptr.mise_au_neutre = FALSE;
				//						}
				//						tac_ctrl_reactiver(); /* si on doit reactiver  */
				//						/* si on doit reactiver suite e une relance */
				//						if (x01_cptr.activation_secteur)
				//						{
				//							x01_cptr.activation_secteur = FALSE;
				//							tac_ctrl_relance_secteur();
				//						} else
				//						{
				//							if (!tac_ctrl_test())
				//							{
				//								relance = tac_ctrl_seconde();
				//								if (++cptSec >= 60) /* 5 sec si !station */
				//								{
				//									cptSec = 0; /* on controle le changement
				//									 d'etat des cellules */
				//									tac_ctrl_minute();
				//								}
				//
				//							}
				//
				//							if (tac_ctrl_cellule()) /* traitement des cellules */
				//							{
				//								tac_ctrl_set_force(TRUE);
				//							}
				//							/* si la seconde est a 0 on change l'affichage de
				//							 * l'heure */
				//							if (x01_date[H_SEC] < 5)
				//							{
				//								if (!flagSec)
				//								{
				//									tac_ctrl_aff_horo_date();
				//									flagSec = TRUE;
				//								}
				//							} else
				//							{
				//								flagSec = FALSE;
				//							}
				//						} /* endif (x01_cptr.activation_secteur) */
				//					} /* endif (x01_cptr.activation_secteur) */
				//					break;
				//				} /* endswitch (x01_cptr.demarrage) */
			} /* endif (lon_status.secteur == TEMPO_A */
			changeSec = FALSE;
			/* Toute les secondes, on ecrit la nouvelle date dans la sram. */
			tac_ctrl_coupure_secteur();
		} /* endif (changeSec) */
		if (relance) {
			tac_ctrl_reactiver();
		}
		etaSystIncrCompteur(numero);
	} /* endwhile (1) */
}
