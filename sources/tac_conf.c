/*
 * tac_conf.c
 * ===================
 *
 *  Created on: 20 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: tac_conf.c,v $
 * Revision 1.2  2018/06/04 08:38:38  xg
 * Passage en version V122
 *
 * Revision 1.4  2008/10/03 16:21:45  xag
 * *** empty log message ***
 *
 * Revision 1.3  2008/09/29 07:58:51  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.2  2008/09/12 14:57:09  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.13  2008/06/20 07:24:10  xag
 * Pour tester l'entete de fichier
 *
 */
/* ********************************
 * LISTE DES INCLUDES
 * ********************************	*/

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "standard.h"
#include "mon_inc.h"
#include "define.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_idfh.h"
#include "lcr_cf3h.h"
#include "lcr_idfh.h"
#include "lcr_cfs.h"
#include "x01_trch.h"
#include "xversion.h"
#include "mon_debug.h"
#include "tac_conf.h"
#include "amd.h"
#include "extLib.h"
#include "seqLib.h"
#include "ypc_las.h"
#include "identLib.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "lcr_cftp.h"
#include "pip_util.h"
#include "lcr_sc.h"
#include "lcr_st_lcpi.h"

#include "configuration.h"
#include "configurationPriv.h"
#include "affichage/caractere.h"

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/
/* --------------------------------
 * T_tacConfEntete
 * ===============
 * Entete de configuration.
 * --------------------------------	*/

typedef struct T_tacConfEntete
{
	struct xdg_cf_las xdg_cf_las[16];
	struct x01_status1 x01_status1;
	LONG bcc1;
	UINT8 num_scenario[5];
	UINT8 num_scenario_actif;
	UINT8 bc;
} T_tacConfEntete;

/* --------------------------------
 * T_tacConf
 * =========
 * Contient l'ensemble des elements
 * de configuration. Sont maintenant
 * mis sous forme de structure afin
 * d'eviter tout probleme de calage.
 * --------------------------------	*/

typedef struct T_tacConf
{
	/* Pour l'identification utilisateurs/mots de passe] */
	UINT8 acces[sizeof(T_zip) * NBID];
	UINT8 nb_acces;
	struct vct_cf_al_sy cf_al_sy;
	struct vct_st_al st_al;
	struct vct_cf_alarme cf_alarme[NB_ALARME];
	/* Equivalences symboliques */
	T_pip_cf_module module[NB_MODULE];
	INT nb_module;
	T_pip_cf_caisson caisson[NB_CAISSON];
	INT nb_caisson;
	/* Equivalences topologiques */
	T_pip_cf_topo topo[NB_CAISSON];
	INT nb_topo;
	T_pip_cf_equiv equiv[NB_EQUIV];
	INT nb_equiv;
	T_pip_cf_litteral litteral[NB_LITTERAL];
	INT nb_litteral;
	/* Seuils des cellules */
	T_pip_seuil_cellule seuil_cellule[NB_SEUIL_CELLULE];
	INT nb_seuil_cellule;
	/* Divers */
	T_vct_usr_lcpi usr_lcpi;
	/* Parametres generaux de configuration */
	T_pip_cf_tst_sc	tst_sc;
	/* Les polices de caracteres */
	T_pip_police police_car[MAX_CAR_POLICE];

	/* La configuration du PMV luminance */
	ConfigStatique configLuminance_t;

	UINT8 liste_edf_alarme;
	UINT8 conf;
	/* Tempo de scrutation PC */
	UINT tempo_spc;
	UINT cks_prom;
	LONG bcc;
} T_tacConf;

/* ********************************
 * FONCTIONS DES CONSTANTES
 * ********************************	*/

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* Configuration propre aux stations RAD SIRIUS, a revoir pour
 * toute autre application. */

/* ********************************
 * VARIABLES LOCALES
 *********************************** */

//static int tac_conf_fic = 2;
#define TAC_CONF_TYPE_ENTETE  	0
#define TAC_CONF_TYPE_CFG  		1
#define TAC_CONF_FICHIER_CONFIG "config.cfg"
static void *_tac_conf_adresse_entete_pt = NULL;
static void *_tac_conf_adresse_pt = NULL;

/* ********************************
 * PROTOTYPE DES FONCTIONS DU MODULE
 * ********************************	*/

static void *_tac_conf_cfg_get_conf_entete_address();
static void *_tac_conf_cfg_get_conf_address();
static void *_tac_conf_cfg_get_address(int);
static void _tac_conf_cfg_flush_address(void);

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * tac_conf_cfg_ecr_entete
 * =======================
 * Lecture de l'entete de la
 * configuration.
 * --------------------------------	*/

VOID tac_conf_cfg_ecr_entete()
{
	T_tacConfEntete *entete;
	/* --------------------------------
	 * ALLOCATION DU TAMPON
	 * --------------------------------     */
	/* L'allocation d'un bloc s'avere
	 * necessaire, la taille de la pile
	 * etant limitee. */
	entete = (T_tacConfEntete *) kmmAlloc(NULL,sizeof(T_tacConfEntete));
	/* mise e zero du tampon pour permettre
	 * un calcul de checksum correcte. */
	if (entete != NULL)
	{
		int indice;
		char *ptr_deb, *ptr_fin;
		bzero((char *) entete, sizeof(T_tacConfEntete));
		for (indice = 0; indice < 16; indice++)
		{
			entete->xdg_cf_las[indice] = xdg_cf_las[indice];
		}
		entete->x01_status1 = x01_status1;
		/* Calcul du checksum   */
		ptr_deb = (char *) entete;
		ptr_fin = (char *) &entete->bcc1;
		entete->bcc1 = 0;
		while (ptr_deb < ptr_fin)
		{
			entete->bcc1 += (LONG) * ptr_deb++;
		}
		for (indice = 0; indice < 5; indice++)
		{
			entete->num_scenario[indice] = vct_num_scenario[indice];
		}
		entete->num_scenario_actif = vct_num_scenario_actif;
		/* Calcul du checksum   */
		ptr_deb = (char *) &entete->num_scenario[0];
		ptr_fin = (char *) &entete->bc;
		entete->bc = 0;
		while (ptr_deb < ptr_fin)
		{
			entete->bc += (LONG) * ptr_deb++;
		}
		/* --------------------------------
		 * COPIER DANS LA FLASH
		 * --------------------------------     */

		{
			void *adEntete = _tac_conf_cfg_get_address(TAC_CONF_TYPE_ENTETE );
			memcpy(adEntete, entete, sizeof(T_tacConfEntete));
			/* TODO : Realiser ici l'ecriture du fichier de configuration... */
			_tac_conf_cfg_flush_address();
		}
		/* --------------------------------
		 * LIBERER LA MEMOIRE
		 * --------------------------------     */
		kmmFree(NULL,(void *) entete);
	} else
	{
		printDebug("tac_conf_cfg_ecr_entete: Erreur d'ecriture de l'entete de la configuration\n");
	} /* endif(entete!=NULL                           */
}

/* ---------------------------------
 * tac_conf_cfg_raz_entete
 * =======================
 * Reset de la configuration
 * --------------------------------	*/

void tac_conf_cfg_raz_entete()
{
	/* seul le scenario 1 est actif */
	vct_num_scenario_actif = 1;
	vct_num_scenario[0] = 1;
	vct_num_scenario[1] = 0;
	vct_num_scenario[2] = 0;
	vct_num_scenario[3] = 0;
	vct_num_scenario[4] = 0;
	/* on initialise la liaison serie */
	tac_las_init_cfg();
	/* les parametres du status 1 */
	strncpy(x01_status1.ver,"201", 3);
	strncpy(x01_status1.adr,"rgs", 3);
	strncpy(x01_status1.cod,"frgdd.s", 7);
	strncpy(x01_status1.loc,"localisation  ", 14);
	strncpy(x01_status1.nst,"0000", 4);
	strncpy(x01_status1.bat,"000", 3);
	strncpy(x01_status1.eol,"033", 3);
	_tac_conf_cfg_flush_address();
}

/* ---------------------------------
 * tac_conf_cfg_set_conf
 * =====================
 * Ecriture de la config dans une
 * zone de memoire ce qui permet de
 * faire facilement une fonction
 * permettant de retourner le checksum
 * --------------------------------	*/

static T_tacConf *tac_conf_cfg_set_conf(void)
{
	T_tacConf *entete;
	/* --------------------------------
	 * ALLOCATION DU TAMPON
	 * --------------------------------     */
	/* L'allocation d'un bloc s'avere
	 * necessaire, la taille de la pile
	 * etant limitee. */
	entete = (T_tacConf *) kmmAlloc(NULL,sizeof(T_tacConf));
	bzero((char *) entete, sizeof(T_tacConf));

	/* mise e zero du tampon pour permettre
	 * un calcul de checksum correcte. */
	if (entete != NULL)
	{
		int indice;
		char *ptr_deb, *ptr_fin;
		LONG taille;
		bzero((char *) entete, sizeof(T_tacConf));
		memcpy(&entete->acces[0], &vct_acces[0], sizeof(T_zip) * NBID);
		entete->nb_acces = vct_nb_acces;
		entete->cf_al_sy = vct_cf_al_sy;
		entete->st_al = vct_st_al;
		for (indice = 0; indice < NB_ALARME; indice++)
		{
			entete->cf_alarme[indice] = vct_cf_alarme[indice];
		}
		entete->liste_edf_alarme = vct_liste_edf_alarme;
		entete->conf = vct_conf;
		for (indice = 0; indice < NB_MODULE; indice++)
		{
			entete->module[indice] = pip_cf_module[indice];
		}
		entete->nb_module = pip_nb_module;
		for (indice = 0; indice < NB_CAISSON; indice++)
		{
			entete->caisson[indice] = pip_cf_caisson[indice];
		}
		entete->nb_caisson = pip_nb_caisson;
		for (indice = 0; indice < NB_CAISSON; indice++)
		{
			entete->topo[indice] = pip_cf_topo[indice];
		}
		entete->nb_topo = pip_nb_topo;
		for (indice = 0; indice < NB_EQUIV; indice++)
		{
			entete->equiv[indice] = pip_cf_equiv[indice];
		}
		entete->nb_equiv = pip_nb_equiv;
		for (indice = 0; indice < NB_LITTERAL; indice++)
		{
			entete->litteral[indice] = pip_cf_litteral[indice];
		}
		entete->nb_litteral = pip_nb_litteral;
		for (indice = 0; indice < NB_SEUIL_CELLULE; indice++)
		{
			entete->seuil_cellule[indice] = pip_cf_seuil_cellule[indice];
		}
		entete->nb_seuil_cellule = pip_nb_seuil_cellule;
		for (indice = 0; indice < MAX_CAR_POLICE; indice++)
		{
		Caractere *car_pt=caractereTableRechercher(indice);
		int carCour;
			if(car_pt!=NULL)
			{
				for(carCour=0;carCour<5;carCour++)
				entete->police_car[indice].col[carCour]=(BYTE)car_pt->datas[carCour];
			}
			else
			{
				for(carCour=0;carCour<5;carCour++)
				entete->police_car[indice].col[carCour]=0;
			}
		}

		configStatiqueGet(&entete->configLuminance_t);

		entete->usr_lcpi = vct_usr_lcpi;
		entete->tst_sc = pip_cf_tst_sc;
		entete->tempo_spc = vct_tempo_spc;
		entete->cks_prom = vct_cks_prom;
		taille = (int) &entete->bcc - (int) entete;
		x01_status3.taille_config = taille;
		ptr_deb = (char *) entete;
		ptr_fin = (char *) &entete->bcc;
		entete->bcc = 0;
		while (ptr_deb < ptr_fin)
		{
			entete->bcc += *ptr_deb++;
		}
	}
	/* --------------------------------
	 * FIN DE tac_conf_cfg_set_conf
	 * --------------------------------     */
	return entete;
}

/* ---------------------------------
 * tac_conf_cfg_get_cnf
 * =====================
 * Pour la lecture du checksum de la
 * flash.
 * --------------------------------	*/

unsigned short tac_conf_cfg_get_cnf(void)
{
	unsigned short checksum = (unsigned short) 0xFFFF;
	T_tacConf *entete = tac_conf_cfg_set_conf();
	/* mise e zero du tampon pour permettre
	 * un calcul de checksum correcte. */
	if (entete != NULL)
	{
		checksum = (unsigned short) entete->bcc;
		kmmFree(NULL,(void *) entete);
	} /* endif(entete!=NULL                           */
	/* --------------------------------
	 * FIN DE tac_conf_cfg_get_cnf
	 * --------------------------------     */
	return checksum;
}

/***************************************************************************/
/* 22/02/96   *  non de la procedure : VOID tac_clv_validation             */
/***************************************************************************/
/* But de la procedure :     ecriture de la configuration                  */
/***************************************************************************/
/* evolutions * RAM => FLASH                                               */
/*            *                                                            */
/***************************************************************************/
VOID tac_conf_cfg_ecr_conf(void)
{
	T_tacConf *entete;
	/* --------------------------------
	 * ALLOCATION DU TAMPON
	 * --------------------------------     */
	/* L'allocation d'un bloc s'avere
	 * necessaire, la taille de la pile
	 * etant limitee. */
	entete = tac_conf_cfg_set_conf();
	/* mise e zero du tampon pour permettre
	 * un calcul de checksum correcte. */
	if (entete != NULL)
	{
	T_tacConf *adEntete = _tac_conf_cfg_get_address(TAC_CONF_TYPE_CFG);
		/* Recopie du contenu de l'entete */
		*adEntete=*entete;
		/* Sauvegarde du fichier de configuration */
		_tac_conf_cfg_flush_address();
		/* --------------------------------
		 * LIBERER LA MEMOIRE
		 * --------------------------------     */
		kmmFree(NULL,(void *) entete);
	} else
	{
		printDebug("tac_conf_cfg_ecr_conf: Probleme d'allocation memoire pour la config\n");
	}
	printDebug("tac_conf_cfg_ecr_conf: Application sans flash\n");
}

/* --------------------------------
 * tac_conf_cfg_raz_conf
 * =========================
 * Reinitialisation de la config.
 * --------------------------------	*/

void tac_conf_cfg_raz_conf()
{
	int j;
	vct_conf = 1;
	vct_liste_edf_alarme = N_AFF;
	vct_cf_al_sy.flag = FALSE;
	for (j = 0; j < MAX_CRT_SY; j++)
	{
		vct_cf_al_sy.critere[j].nature = N_AFF;
		vct_cf_al_sy.critere[j].relation = N_AFF;
		vct_cf_al_sy.critere[j].seuil = NIL;
		vct_cf_al_sy.critere[j].oplog = OP_OR;
	}
	vct_st_al.act = FALSE;
	vct_st_al.neut = 400;
	vct_st_al.rep = 2;
	vct_st_al.seq = 'B';
	/* Modification de la valeur par
	 * defaut definie pour le port. */
	vct_st_al.port = 3;
	/* raz de la configuration pour tous
	 * les identifiants */
	identStandard();

	configInitStatique();
	/* configuration de la memoire */
	cf3_conf_memoire();

	pip_init(false);
	x01_bat = 0; /* on raz la config */
	lcr_tst_sc_init(); /* les message forces */
	lcr_st_lcpi_init();
	vct_tempo_spc = 0; /* Pas de scrutation PC par defaut   */
}

/***************************************************************************/
/* 22/02/96   *  non de la procedure : VOID tac_clv_validation             */
/***************************************************************************/
/* But de la procedure :  lecture de la configuration                      */
/***************************************************************************/
/* evolutions *                                                            */
/*            *                                                            */
/***************************************************************************/
UINT8 tac_conf_cfg_lec_entete()
{
	UINT8 erreur = 0;
	char *ptr_deb;
	char *ptr_fin;
	LONG bcc1;
	int indice;
	T_tacConfEntete *entete = _tac_conf_cfg_get_address(TAC_CONF_TYPE_ENTETE);

	printDebug("L'adresse de l'entete de la config est %#x", (unsigned int) entete);

	for (indice = 0; indice < 16; indice++)
	{
		xdg_cf_las[indice] = entete->xdg_cf_las[indice];
	}
	x01_status1 = entete->x01_status1;
	/* Calcul du checksum                           */
	ptr_deb = (char *) entete;
	ptr_fin = (char *) &entete->bcc1;
	bcc1 = 0;
	while (ptr_deb < ptr_fin)
	{
		bcc1 += (LONG) * ptr_deb++;
	}
	/* Controle du checksum                         */
	if (bcc1 == 0)
	{
		printDebug("Probleme, le BCC est nul\n");
		erreur = 1;
	} else if (bcc1 != entete->bcc1)
	{
		printDebug("Probleme, le BCC est incorrect : %#lx pour %#lx, adresse %#lx\n", entete->bcc1, bcc1,
				(unsigned long) &entete->bcc1);
		erreur = 1;
	}

	if (!erreur)
	{
		UINT8 bc = 0;
		for (indice = 0; indice < 5; indice++)
		{
			vct_num_scenario[indice] = entete->num_scenario[indice];
		}
		vct_num_scenario_actif = entete->num_scenario_actif;
		/* Calcul du checksum                           */
		ptr_deb = (char *) &entete->num_scenario[0];
		ptr_fin = (char *) &entete->bc;
		while (ptr_deb < ptr_fin)
		{
			bc += (LONG) * ptr_deb++;
		}
		if ((bc != entete->bc) || (bc == 0))
		{
			/* il faudra sauvegarder la config
			 * mais pas reinitialiser la trans */
			erreur = 2;
		}
		if (vct_num_scenario_actif > 5)
		{
			erreur = 2;
		}
		if (vct_num_scenario[vct_num_scenario_actif - 1] != vct_num_scenario_actif)
		{
			erreur = 2;
		}
	} else
	{
		/* on raz les scenario actif */
		vct_num_scenario[0] = 1;
		vct_num_scenario[1] = 0;
		vct_num_scenario[2] = 0;
		vct_num_scenario[3] = 0;
		vct_num_scenario[4] = 0;

		/* le numero de scenario actif */
		vct_num_scenario_actif = 1;

	}

	return (erreur);
}

/* --------------------------------
 * tac_conf_cfg_lec_def_conf
 * =========================
 * Lecture en flash de la variable
 * signalant la config usine.
 * --------------------------------	*/

int tac_conf_cfg_lec_def_conf(void)
{
	int retour = 0;
	T_tacConf *entete = (T_tacConf *) _tac_conf_cfg_get_address(TAC_CONF_TYPE_CFG);
	if (entete != NULL)
	{
		retour = entete->conf;
	}
	return retour;
}

UINT8 tac_conf_cfg_lec_conf(void)
{
	UINT8 erreur = 0;
	T_tacConf *entete = (T_tacConf *) _tac_conf_cfg_get_address(TAC_CONF_TYPE_CFG);
	/* --------------------------------
	 * ALLOCATION DU TAMPON
	 * --------------------------------     */
	/* L'allocation d'un bloc s'avere
	 * necessaire, la taille de la pile
	 * etant limitee. */
	/* mise e zero du tampon pour permettre
	 * un calcul de checksum correcte. */
	if (entete != NULL)
	{
		LONG bcc;
		int indice;
		char *ptr_deb, *ptr_fin;

		/* Identifiants equipement */
		memcpy(&vct_acces[0], &entete->acces[0], sizeof(T_zip) * NBID);
		vct_nb_acces = entete->nb_acces;
		vct_cf_al_sy = entete->cf_al_sy;
		vct_st_al = entete->st_al;
		for (indice = 0; indice < NB_ALARME; indice++)
		{
			vct_cf_alarme[indice] = entete->cf_alarme[indice];
		}
		vct_liste_edf_alarme = entete->liste_edf_alarme;

		for (indice = 0; indice < NB_MODULE; indice++)
		{
			pip_cf_module[indice] = entete->module[indice];
		}
		pip_nb_module = entete->nb_module;
		for (indice = 0; indice < NB_CAISSON; indice++)
		{
			pip_cf_caisson[indice] = entete->caisson[indice];
		}
		pip_nb_caisson = entete->nb_caisson;
		for (indice = 0; indice < NB_CAISSON; indice++)
		{
			pip_cf_topo[indice] = entete->topo[indice];
		}
		pip_nb_topo = entete->nb_topo;
		for (indice = 0; indice < NB_EQUIV; indice++)
		{
			pip_cf_equiv[indice] = entete->equiv[indice];
		}
		pip_nb_equiv = entete->nb_equiv;
		for (indice = 0; indice < NB_LITTERAL; indice++)
		{
			pip_cf_litteral[indice] = entete->litteral[indice];
		}
		pip_nb_litteral = entete->nb_litteral;
		for (indice = 0; indice < NB_SEUIL_CELLULE; indice++)
		{
			pip_cf_seuil_cellule[indice] = entete->seuil_cellule[indice];
		}
		pip_nb_seuil_cellule = entete->nb_seuil_cellule;
		vct_usr_lcpi = entete->usr_lcpi;
		/* Parametres generaux. */
		pip_cf_tst_sc=entete->tst_sc;
		for (indice = 0; indice < MAX_CAR_POLICE; indice++)
		{
		Caractere *car_pt=caractereTableRechercher(indice);
			if(NULL==car_pt)
			{
				car_pt=caractereNewFromData(indice,entete->police_car[indice].col);
				caractereAdd(car_pt);
			}
			else
			{
			int carCour;
				for(carCour=0;carCour<5;carCour++)
				{
					car_pt->datas[carCour] = entete->police_car[indice].col[carCour];
				}
			}
		}

		configStatiqueSet(&entete->configLuminance_t);

		vct_tempo_spc = entete->tempo_spc;
		vct_cks_prom = entete->cks_prom;
		ptr_deb = (char *) entete;
		ptr_fin = (char *) &entete->bcc;
		bcc = 0;
		while (ptr_deb < ptr_fin)
		{
			bcc += *ptr_deb++;
		}
		if (bcc != entete->bcc)
		{
			erreur = 1;
			vct_chk_scenario[vct_num_scenario_actif - 1] = 0;
		} else
		{
			vct_chk_scenario[vct_num_scenario_actif - 1] = (UINT) bcc;
		}
	}
	/* Je ne comprend pas tres bien pourquoi on retourne la negation
	 * de l'erreur. Si je ne le fait pas, il y a reconfiguration.
	 */
	return ((UINT8) (!erreur));
}

static void *_tac_conf_cfg_get_address(int val)
{
	void *address = NULL;
	switch (val)
	{
	case 0:
		address = (void *) _tac_conf_cfg_get_conf_entete_address();
		break;
	case 1:
		address = (void *) _tac_conf_cfg_get_conf_address();
		break;
	}
	return address;
}
/*
 * _tac_conf_cfg_get_conf_entete_address
 * ==============================
 * La fonction retourne l'adresse de la structure de configuration.
 * Si l'adresse est nulle, lecture du fichier de configuration.
 */

static void *_tac_conf_cfg_get_conf_entete_address(void)
{
	if (NULL == _tac_conf_adresse_entete_pt)
	{
		FILE *fichierConfig_pt;
		_tac_conf_adresse_entete_pt = kmmAlloc(NULL,sizeof(T_tacConfEntete));
		/* Ouverture du fichier de configuration et lecture de l'entete.... */
		fichierConfig_pt = fopen(TAC_CONF_FICHIER_CONFIG, "r");
		if (NULL != fichierConfig_pt)
		{
			/* Attention, il n'est pas nécessaire de se positionner dans le fichier car
			 * les informations sont au début du fichier. */
			int nbInfoLus = fread(_tac_conf_adresse_entete_pt, sizeof(T_tacConfEntete), 1, fichierConfig_pt);
			if (-1 == nbInfoLus)
			{
				/* TODO: Mettre un message d'erreur */
				printDebug("_tac_conf_cfg_get_conf_entete_adresse : erreur de lecture de %s, erreur %d", TAC_CONF_FICHIER_CONFIG,
						errno);
			} else
			{
				if (1 == nbInfoLus)
				{
					/* Tout va bien, on a lu la totalite du fichier de configuration. */
				}
			}
			if (EOF == fclose(fichierConfig_pt))
			{
				/* TODO: Mettre un message signalant le probleme */
				printDebug("_tac_conf_cfg_get_conf_entete_adresse : erreur de fermeture de %s, erreur %d", TAC_CONF_FICHIER_CONFIG,
						errno);
			}

		} else
		{
			/* TODO: Mettre un message indiquant que le fichier de configuration n'a
			 * pas pu être ouvert.
			 */
			printDebug("_tac_conf_cfg_get_conf_entete_adresse : erreur d'ouverture de %s, erreur %d", TAC_CONF_FICHIER_CONFIG, errno);
		}
	}
	return _tac_conf_adresse_entete_pt;
}

/*
 * _tac_conf_cfg_get_conf_address
 * ==============================
 * La fonction retourne l'adresse de la structure de configuration.
 * Si l'adresse est nulle, lecture du fichier de configuration.
 */
static void *_tac_conf_cfg_get_conf_address(void)
{
	if (NULL == _tac_conf_adresse_pt)
	{
		FILE *fichierConfig_pt;
		_tac_conf_adresse_pt = kmmAlloc(NULL,sizeof(T_tacConf));
		fichierConfig_pt = fopen(TAC_CONF_FICHIER_CONFIG, "r");
		if (NULL != fichierConfig_pt)
		{
			if (-1 == fseek(fichierConfig_pt, sizeof(T_tacConfEntete), SEEK_SET))
			{
				printDebug("_tac_conf_cfg_get_conf_adresse : erreur de deplacement dans %s, erreur %d", TAC_CONF_FICHIER_CONFIG, errno);

			} else
			{
				int nbInfoLus;
				nbInfoLus = fread(_tac_conf_adresse_pt, sizeof(T_tacConf), 1, fichierConfig_pt);
				if (-1 == nbInfoLus)
				{
					/* TODO: Mettre un message d'erreur */
					printDebug("_tac_conf_cfg_get_conf_adresse : erreur de lecture de %s, erreur %d", TAC_CONF_FICHIER_CONFIG, errno);
				} else
				{
					if (1 == nbInfoLus)
					{
						/* Tout va bien, on a lu la totalite du fichier de configuration. */
						printDebug("_tac_conf_cfg_get_conf_adresse: lecture correcte de la configuration\n");
					}
				}
			}
			if (EOF == fclose(fichierConfig_pt))
			{
				/* TODO: Mettre un message signalant le probleme */
				printDebug("_tac_conf_cfg_get_conf_adresse : erreur de fermeture de %s, erreur %d", TAC_CONF_FICHIER_CONFIG, errno);
			}

		} else
		{
			/* TODO: Mettre un message indiquant que le fichier de configuration n'a
			 * pas pu être ouvert.
			 */
			printDebug("_tac_conf_cfg_get_conf_adresse : erreur d'ouverture de %s, erreur %d", TAC_CONF_FICHIER_CONFIG, errno);
		}
	}
	return _tac_conf_adresse_pt;
}

/*
 * _tac_conf_cfg_flush_address
 * ===========================
 * La fonction a pour objet d'ecrire la configuration.
 */
static void _tac_conf_cfg_flush_address(void)
{
FILE *fichierConfig_pt;
void *entete_pt=_tac_conf_cfg_get_conf_entete_address();
void *config_pt=_tac_conf_cfg_get_conf_address();
	fichierConfig_pt = fopen(TAC_CONF_FICHIER_CONFIG, "w");
	if(NULL!=fichierConfig_pt)
	{
	int retour;
		retour=fwrite(entete_pt,sizeof(T_tacConfEntete),1,fichierConfig_pt);
		retour=fwrite(config_pt,sizeof(T_tacConf),1,fichierConfig_pt);
		fclose(fichierConfig_pt);
		printDebug("_tac_conf_cfg_flush_address: ecriture configuration\n");

	} else
	{
		/* TODO: Mettre un message indiquant que le fichier de configuration n'a
		 * pas pu être ouvert.
		 */
		printDebug("_tac_conf_cfg_flush_adresse : erreur d'ouverture de %s, erreur %d", TAC_CONF_FICHIER_CONFIG, errno);
	}

}
/* *********************************
 * FIN DE tac_conf.c
 * ********************************* */
