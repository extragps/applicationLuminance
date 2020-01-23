/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_st                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 21/12/1996                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles              /
 /                           e la reinitialisation de la station               /
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
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "x01_str.h"
#include "x01_vcth.h"
#include "x01_var.h"
#include "lcr_init.h"
#include "lcr_st1.h"

#include "ecouteur.h"
#include "event.h"
#include "sequenceur.h"
#include "sramMessage.h"
#include "configuration.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

static VOID lcr_init_pip(VOID);

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_init_init                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 24/03/1999                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_init.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande init                            /
 /-------------------------------------------------------------------------DOC*/
VOID lcr_init_init(VOID)
{
	lcr_init_cmd();
	lcr_init_sv_act_pip();
	x01_cptr.mise_au_neutre = TRUE;
}

/*
 * lcr_init_sv_act_pip
 * ===================
 * OBJET DE LA FONCTION : procedure d'init de la structure de sauvegarde
 *                      : du module PIP
 * */
VOID lcr_init_sv_act_pip(VOID)
{
	INT i;
	INT j;
	for (i = 0; i < NB_CAISSON; i++)
	{
		T_pip_cf_caisson *conf_pt=&pip_cf_caisson[i];
		T_pip_sv_act_caisson act_caisson;
		T_pip_sv_act_caisson *caisson = &act_caisson;
		sramMessageLireCaisson(i,caisson);
		caisson->act = PIP_ACT_CAR;
		caisson->luminosite.type = TYPE_AUTOMATIQUE;
		/*caisson->cmd_lum = 'A';
		 caisson->cmd_intensite = 0;
		 */
		caisson->neutre = TRUE;

		caisson->extinction = (conf_pt->type==TYPE_ALPHA?FALSE:TRUE);
		caisson->forcage = FALSE;
		caisson->nbAlternance = 0;
		caisson->finDv=false;
		caisson->clig.type = PIP_AUCUN;
		caisson->validite = 0;
		caisson->validite_ori = 0;
		caisson->validite_dem = 0;
		for (j = 0; j < MAX_ALTERNANCE; j++)
		{
			caisson->action[j].type = 0;
		}
		sramMessageEcrireCaisson(i,caisson);
	}
}

/*
 * lcr_init_pip
 * ============
 * OBJET DE LA FONCTION : procedure d'init du module PIP
 * PARAMETRES           :
 * VALEURS RENDUES      :
 * FONCTIONS APPELEES   :
 * FONCTIONS APPELANTES :
 ***************************************************************************DOC*/
LOCAL VOID lcr_init_pip(VOID)
{

	INT i, j;
	for (i = 0; i < NB_MODULE; i++)
	{
		pip_act_module[i].activation = FALSE;
		pip_act_module[i].spc = pip_cf_module[i].spc;
		pip_act_module[i].hysteresis_lum = 0;
		pip_act_module[i].saborde = FALSE;
		pip_act_module[i].saborde_meca = FALSE;
		pip_act_module[i].def_alim = FALSE;
		pip_act_module[i].cellule = 'N';
		pip_act_module[i].val_cell = 0;
	}
	for (i = 0; i < NB_LT501; i++)
	{
		pip_act_cellule[i].conf = FALSE;
		pip_act_cellule[i].nv_etat = 0;
		/* pip_act_cellule[i].etat = 5000; */
		pip_act_cellule[i].num_seuil = 9;
		pip_act_cellule[i].seuilLum = '0';
		pip_act_cellule[i].hysteresis_lum = 10;
		pip_act_cellule[i].defChangement = FALSE;
		pip_act_cellule[i].defTrans = FALSE;
	}

	for (i = 0; i < NB_CAISSON; i++)
	{
		T_pip_act_caisson *caisson = &pip_act_caisson[i];
		caisson->etat_ok = TRUE;
		caisson->p_etat = P_NK;
		caisson->nbAlternance = 0;
		caisson->validite = 0;
		caisson->svNbAlternance = 0;
		caisson->extinction = FALSE;
		caisson->clig.type = PIP_AUCUN;
		caisson->forcage = FALSE;

		caisson->tens_diode = pip_cf_seuil_cellule[0].tens_diode;
		/* normal secours */
		caisson->mode_luminosite = 0;
		caisson->num_seuil = 0;

		caisson->alt.type = PIP_AUCUN;

		caisson->luminosite.type = TYPE_AUTOMATIQUE;
		caisson->cmd_test = 0;

		caisson->act_rythme = 0;
		caisson->relance_b = false;
		caisson->chg = 0;
//
//		caisson->etat_aff = 0;
//		caisson->sv_etat_aff = 0;

//		caisson->cpt_relance = pip_cf_tst_sc.cpt_relance;
//		caisson->cpt_attente_relance = pip_cf_tst_sc.cpt_attente_relance;

		caisson->tempo_def_min = 0;
		caisson->cpt_def_min = 0;

		caisson->carte_sabordee = FALSE;
		caisson->validite = 0;
		caisson->sab_def_trans = pip_cf_caisson[i].sab_def_trans;
	}

	for (i = 0; i < AFFICHEUR_NB_MAX; i++)
	{
		/* on doit activer le caractere */
		for (j = 0; j < MAX_ALTERNANCE; j++)
		{
			pip_act_car[i].cmd[j].car1 = 0;
			pip_act_car[i].cmd[j].car2 = 0;
		}
		pip_act_car[i].def_car = FALSE;
	}


	/* Il faut remettre toutes les sorties au bon niveau...  */

	x01_cptr.mise_au_neutre = FALSE; /* on raz la mise au neutre */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : cmd_set                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/12/1996                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_set.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande SET                            /
 /-------------------------------------------------------------------------DOC*/
VOID lcr_init_cmd(VOID)
{
	INT i;
	for (i = 0; i < NBPORT; i++)
	{
		/* pour la commande VT */
		xdg_vt[i].port = N_AFF;
		xdg_vt[i].tempo = 0;
		xdg_vt[i].trace = FALSE;
		x01_set[i].echo = FALSE;
		x01_set[i].cr = FALSE;
		x01_set[i].lpl = 0;
		x01_set[i].mtel = FALSE;

	}
	/* on initialise l'application PIP */
	lcr_init_pip();

	/* on incremente le compteur d'init */
	/* XG le 21/10/05. Le compteur d'initialisation n'est pas incremente ici
	 * normalement mais dans idf_reinit.!!! */
	/*  x01_status2.ini++;                                  */

	/* on signale une reinit */
	vct_ST_TR = 0x42;

	/* on signale l'evenement */
	lcr_st1_evt("INI");
	/* on incremente le compteur
	 * d'initialisation */
	configIncrIni();
	/* le type d'init */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_init_pmv                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 24/03/1999                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_init.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande init                            /
 /-------------------------------------------------------------------------DOC*/
VOID lcr_init_pmv(VOID)
{
	/* on initilise les zones de calcul */
	lcr_init_cmd();
	lcr_init_sv_act_pip();
	x01_cptr.mise_au_neutre = TRUE; /* on re-initialise les lampes */
	sequenceurSetReinit(true);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_init                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 20/12/1996                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_init                                             /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande INIT                          /
 /-------------------------------------------------------------------------DOC*/
/* XG : Fonction a supprimer au plus tot
 static VOID
 lcr_init (INT las,
 INT mode,
 INT lg_mess,
 STRING buffer,
 INT position,
 struct usr_ztf *pt_mess)
 {
 BYTE init;

 init = FALSE;

 if (tac_ctrlLocalDistant (las))
 {
 init = TRUE;
 }
 else
 x01_cptr.erreur = CPTRD_PROTOCOLE;

 if (x01_cptr.erreur == CPTRD_OK)
 {
 tedi_ctrl (las, mode, TRUE, 0);
 }
 else
 {
 tedi_erreur (las, mode);
 }

 if (init)
 {
 cmd_trc_mat (1, "RECEPTION CMD INIT LAS=%d", las + 1);
 lcr_init_pmv ();
 }
 }
 */
