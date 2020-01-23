/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pmv10                                              /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la con- /
 / figuration et l'activation des panneaux                                     /
 /                                                                             /
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
#include "pip_str.h"
#include "pip_var.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_util.h"
#include "lcr_p.h"
#include "lcr_cfes.h"
#include "lcr_trc.h"
#include "ficTrace.h"
#include "mon_debug.h"
#include "pip_util.h"
#include "bpTestLib.h"
#include "identLib.h"

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define PIP_NB_MODULES 2

/* --------------------------------
 * DEFINITION DES TYPES LOCAUX
 * --------------------------------	*/

/* --------------------------------
 * T_pip_etat_chaine
 * =================
 * Definition de la chaine de
 * caractere associee a un etat pour les
 * premiers modules.
 * --------------------------------	*/

typedef struct pip_etat_chaine
{
	int etat;
	int lum;
	char * chaine[PIP_NB_MODULES];
} T_pip_etat_chaine;

/* --------------------------------
 * DEFINITION DES VARIABLES LOCALES
 * --------------------------------	*/

static T_pip_etat_chaine pip_etat_chaine[] =
{
{ PIP_DELESTAGE, 10,
{ "\031G", "" } },
{ PIP_NEUTRE, 10,
{ "*", "*" } },
{ PIP_TEMPERATURE, 10,
{ "!", "" } },
{ PIP_VEILLE_SANS_COM, 10,
{ "\031&", "" } },
{ PIP_VEILLE_AVEC_COM, 10,
{ "&", "" } },
{ PIP_TEST_DIODES, 0,
{ "\031P\031P\031P\031P\031P\031P\031P\031P\031P"
	"\031P\031P\031P\031P\031P\031P\031P\031P\031P", "\031P\031P\031P\031P\031P\031P\031P\031P\031P"
	"\031P\031P\031P\031P\031P\031P\031P\031P\031P" } },
{ PIP_TEST_BP1, -1,
{ " PANNEAU EN TEST  ", "\031a\031a\031a\031a\031a\031a\031a\031a\031a"
	"\031a\031a\031a\031a\031a\031a\031a\031a\031a" } },
{ PIP_TEST_BP2, -1,
{ " PANNEAU EN TEST  ", "\031b\031b\031b\031b\031b\031b\031b\031b\031b"
	"\031b\031b\031b\031b\031b\031b\031b\031b\031b" } },
{ PIP_TEST_BP3, -1,
{ "\031a\031a\031a\031a\031a\031a\031a\031a\031a"
	"\031a\031a\031a\031a\031a\031a\031a\031a\031a", " PANNEAU EN TEST  " } },
{ PIP_TEST_BP4, -1,
{ "\031b\031b\031b\031b\031b\031b\031b\031b\031b"
	"\031b\031b\031b\031b\031b\031b\031b\031b\031b", " PANNEAU EN TEST  " } },
{ PIP_TEST_TPE1, -1,
{ "ABCDEFGHIJKLMNOPQR", "STUVWXYZ0123456789" } },
{ PIP_TEST_TPE2, -1,
{ "abcdefghijklmnopqr", "stuvwxyz0123456789" } },
{ PIP_TEST_TPE3, -1,
{ " #.:!$*=+-><\031.\031)\031*\031Q\031P ", " #.:!$*=+-><\031.\031)\031*\031Q\031P " } },
{ PIP_TEST_TPE4, 10,
{ " TEST SEUIL NUIT  ", " TEST SEUIL NUIT  " } },
{ PIP_TEST_TPE5, 11,
{ " TEST SEUIL JOUR  ", " TEST SEUIL JOUR  " } },
{ PIP_TEST_TPE6, 12,
{ " TEST SEUIL SURB  ", " TEST SEUIL SURB  " } },
{ -1, 0,
{ NULL, NULL } } };

static int pip_etat = PIP_AFFICHAGE;

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

VOID lcr_p(INT, INT, INT, STRING, INT, struct usr_ztf *);

static BYTE lcr_cmd_p(STRING, INT, INT);
static int cmd_exec_p_caisson(int j, T_pip_cmd_p * cmd_p);
static BYTE message_predefini(STRING, STRING, INT *);
static VOID lcr_p_caisson_neutre(T_pip_cf_caisson *, T_pip_act_caisson *);
static int activation_caisson_reelle(INT, T_pip_sv_act_caisson *);
static void pip_aff_etat(int, T_pip_sv_act_caisson *, int);
static int pip_aff_trace(int indCaisson);

/* --------------------------------
 * pipGetEtat()
 * ============
 * Recuperation de l'etat courant
 * --------------------------------	*/

int pipGetEtat()
{
	return pip_etat;
}

void pipSetEtat(int etat)
{
	pip_etat = etat;
}

/* ----------------------------------
 * lcr_p_aff
 * =========
 * Fonction qui interprete la chaine
 * de caracteres pour trouver les
 * caracteres dans la police.
 * --------------------------------	*/

int pip_car_is_null(T_pip_car *car)
{
	return (car->car1 == 0);
}
static void pip_car_reset(T_pip_car *car)
{
	car->car1 = 0;
	car->car2 = 0;
}
int pip_car_dec(T_pip_car *car, STRING buffer)
{
	int nbCarDec = 0;
	if (0 != car->car1)
	{
		if (car->car1 >= 0x80)
		{
			buffer[nbCarDec++] = 0x19;
			buffer[nbCarDec++] = car->car1 & 0x7F;
			if ((car->car1 & 0xF0) == 0xC0)
			{
				if (0 != car->car2)
				{
					buffer[nbCarDec++] = car->car2;
				}
			}
		} else
		{
			buffer[nbCarDec++] = car->car1;
		}
	}
	return nbCarDec;
}
static int pip_car_aff(T_pip_car *car, STRING buffer)
{
	int nbCarUtil = 0;
	pip_car_reset(car);
	if (0 != buffer[nbCarUtil])
	{
		if (0x19 == buffer[nbCarUtil])
		{
			nbCarUtil++;
			if (0 != buffer[nbCarUtil])
			{
				car->car1 = 0x80 + buffer[nbCarUtil];
				{
					nbCarUtil++;
				}
			}

		} else
		{
			car->car1 = buffer[nbCarUtil++];
		}
	}
	return nbCarUtil;
}
/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_p_caisson_neutre                                   /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/08/1993                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_p.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : extinction d'un caisson                                /
 /-------------------------------------------------------------------------DOC*/
LOCAL VOID lcr_p_caisson_neutre(T_pip_cf_caisson *caisson, T_pip_act_caisson *action)
{
	INT k, l;
	/* le caisson n'est pas commande */
	action->validite = 0L;
	action->validite_ori = 0L;

	action->nbAlternance = 0;
	action->svNbAlternance = 0;

	action->alt.type = PIP_AUCUN;
	action->clig.type = PIP_AUCUN;

	/* on doit activer le caisson */

	action->act_car = TRUE;
	action->clig.type = PIP_AUCUN;

	for (k = caisson->num_car; k < caisson->num_car + caisson->nb_car; k++)
	{
		/* on raz les zones de stockage caractere */
		for (l = 0; l < MAX_ALTERNANCE; l++)
		{
			pip_car_reset(&pip_act_car[k].cmd[l]);
		}
	}
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : cmd_p                                                  /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/08/1993                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pmv0.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande P de positionnement           /
 / 							mise e jour de pip_sv_act_caisson            		      /
 /-------------------------------------------------------------------------DOC*/

VOID cmd_init_p(T_pip_cmd_p * cmd_p)
{
	int i;
	cmd_p->flag = FALSE;
	cmd_p->id_module = NN_AFF;
	cmd_p->caisson = N_AFF;
	cmd_p->neutre = FALSE;
	cmd_p->extinction = FALSE;
	cmd_p->nb_alt = 0;
	cmd_p->clig.type = PIP_DEFAUT;
	cmd_p->alt.type = PIP_DEFAUT;
	cmd_p->luminosite.type = TYPE_AUTOMATIQUE; /* par defaut */
	cmd_p->forcage = N_AFF;
	cmd_p->test = N_AFF;
	cmd_p->entretient = N_AFF;
	cmd_p->validite = -1L;

	for (i = 0; i < MAX_ALTERNANCE; i++)
	{
		cmd_p->action[i].type = 0;
	}
}

void cmd_exec_p(T_pip_cmd_p * cmd_p)
{
	int erreur = FALSE;
	int deb = pip_util_ind_deb(cmd_p->id_module, cmd_p->caisson);
	int fin = pip_util_ind_fin(cmd_p->id_module, cmd_p->caisson);
	int caisson;
	for (caisson = deb; (caisson < fin) && (!erreur); caisson++)
	{
		erreur = cmd_exec_p_caisson(pip_util_ind_caisson(caisson), cmd_p);
	}

	if (erreur)
		x01_cptr.erreur = CPTRD_PARAM;
	/* on initialise la structure de demande de configuration */
	cmd_init_p(cmd_p);
}

static int cmd_exec_p_caisson(int j, T_pip_cmd_p * cmd_p)
{
	int erreur = 0;
	BYTE cmd_p_neutre;
	BYTE cmd_p_nb_alt;
	int l;
	T_pip_cf_caisson *caisson = &pip_cf_caisson[j];
	T_pip_sv_act_caisson infoCaisson;
	T_pip_sv_act_caisson *sv_action = &infoCaisson;

	sramMessageLireCaisson(j,sv_action);
	cmd_p_neutre = cmd_p->neutre;
	cmd_p_nb_alt = cmd_p->nb_alt;

	sv_action->conf = 1;
	sv_action->finDv = false;
	sv_action->act=0;

	/* si il y a une commande de positionnement */
	switch (caisson->type)
	{
	case TYPE_ALPHA:
		printDebug("cmd_exec_p_caisson, alpha, nb_alt %d ext %d neu %d\n", cmd_p_nb_alt, cmd_p->extinction,
				cmd_p_neutre);
		sv_action->extinction = cmd_p->extinction;
		if ((cmd_p_nb_alt != 0) || (cmd_p_neutre))
		{
			int m;
			/* on signale une activation caractere */
			sv_action->act |= PIP_ACT_CAR;
			sv_action->neutre = cmd_p_neutre;
			/* le nombre d'alternance du caisson */
			sv_action->nbAlternance = cmd_p_nb_alt;
			/* on met e jour l'affichage  pour toutes les alternances */
			for (m = 0; m < cmd_p_nb_alt; m++)
			{
				//				int fini = FALSE;
				l = 0;
				/* on raz la sauvegarde */
				sv_action->action[m] = cmd_p->action[m];

				switch (sv_action->action[m].type)
				{
				case TYPE_LITTERAL:
				{
				int cptCar=0;
				int indCour=0;
				int nbcar_txt=sv_action->action[m].valeur.litteral.nbcar_txt;
				char *txt_pt=sv_action->action[m].valeur.litteral.txt;
				
					printDebug("cmd_exec_p_caisson, Dans le cas litteral\n");
					for(indCour=0;indCour<nbcar_txt;indCour++)
					{
						if(25==txt_pt[indCour])
						{
							indCour++;
						}
						cptCar++;
					}
					
					if (cptCar > caisson->nb_car)
					{
						printDebug("Trop de caracteres : %d pour %d\n",
								sv_action->action[m].valeur.litteral. nbcar_txt, caisson->nb_car);

						erreur = TRUE;
					}
				}
					break;

				case TYPE_SYMBOLIQUE:
				{
					char buffer_mess[BUFF_MESS_P];
					int nbcar;
					printDebug("cmd_exec_p_caisson, Dans le cas symbolique\n");
					if ((nbcar = message_predefini(buffer_mess, sv_action->action[m].valeur. litteral.txt,
							&sv_action->type_predef[m])) == 0)
					{
						/* il n'y a pas de message predefini */
						sv_action->type_predef[m] = N_AFF;
					} else if (nbcar > caisson->nb_car)
					{
						/* la longueur du message
						 * depasse le nb car du caisson */
						erreur = TRUE;
					}
				}
					break;
				default:
					printDebug("cmd_exec_p_caisson, autre type %d\n", sv_action->action[m].type);
					break;
				}
			}
		}
		break;
	case TYPE_DELEST:
		sv_action->act |= PIP_ACT_CAR;
		sv_action->extinction = cmd_p->extinction;
//		if(0<cmd_p_nb_alt)
//		{
//				switch (sv_action->action[0].type)
//				{
//				case TYPE_SYMBOLIQUE:
//					sv_action->valeur.graph = sv_action->action[0].valeur.graph;
//					break;
//				}
//		}

		break;
	}
	/* si il y a une commande de neutre */
	if (cmd_p_neutre)
	{
		int m;
		printDebug("cmd_exec_p_caisson, cas du neutre\n");
		/* on signale une activation caractere */
		sv_action->act |= PIP_ACT_CAR;
		sv_action->neutre = cmd_p_neutre;

		/* le nombre d'alternance du caisson */
		sv_action->nbAlternance = 1;

		/* on met e jour l'affichage  pour toutes les alternances */
		for (m = 0; m < MAX_ALTERNANCE; m++)
		{
			l = 0;
			/* on raz la sauvegarde */

			sv_action->action[m].type = TYPE_LITTERAL;
			sv_action->action[m].valeur.litteral.nbcar_txt = caisson->nb_car;

			/* pour la fin de chaine */
			while (l < caisson->nb_car)
			{
				sv_action->action[m].valeur.litteral.txt[l++] = ' ';
			}
			sv_action->action[m].valeur.litteral.txt[l] = 0;
		}
	}

	/* si c'est une mise au neutre */
	if (cmd_p_neutre)
	{
		/* on signale une activation validite */
		sv_action->act |= PIP_ACT_VAL;
		sv_action->validite = 0L;
		sv_action->validite_ori = 0L;
		sv_action->validite_dem = 0L;
		sv_action->validite_fin = 0L;
		configSetValidite((-1 == sv_action->validite ? 0 : sv_action->validite * 1000), j);
	} else if (cmd_p_nb_alt)
	{

		/* on signale une activation validite */
		sv_action->act |= PIP_ACT_VAL;
		sv_action->validite_fin = 0L;
		/* il y a une demande de modification de la duree de validite */
		if (cmd_p->validite != -1L)
		{
			/* on sauvegarde la duree de validite */
			sv_action->validite = cmd_p->validite;
			sv_action->validite_ori = cmd_p->validite;
		} else
		{
			/* on sauvegarde la duree de validite par defaut */
			sv_action->validite = caisson->validite;
			sv_action->validite_ori = caisson->validite;
		}
		/* Positionner ici la validite du caisson... */
		configSetValidite((-1 == sv_action->validite ? 0 : sv_action->validite * 1000), j);

	}

	/* si il y a une commande de luminosite */
	if (cmd_p->luminosite.type != N_AFF)
	{
		/* on signale une activation luminosite et on force le reaffichage */
		sv_action->act |= PIP_ACT_LUM|PIP_ACT_CAR;

		/* on sauvegarde la commande de luminosite */
		sv_action->luminosite = cmd_p->luminosite;
	}

	/* on raz la commande de test */
	sv_action->cmd_test = 0;

	/* si il y a une demande d'entretient */
	if (cmd_p->entretient == 1)
	{
		/* on signale une activation de test,entretient  */
		sv_action->act |= PIP_ACT_TEST;
		/* on sauvegarde la commande de test */
		sv_action->cmd_test = 10;
	} else if (cmd_p->entretient == 0)
	{
		sv_action->cmd_test = 0;
		sv_action->act |= PIP_ACT_CAR;
	} else
	{
		/* si il y a une commande de test */
		if (cmd_p->test != N_AFF)
		{
			if (cmd_p->test > '0')
			{
				/* on signale une activation de test */
				sv_action->act |= PIP_ACT_TEST;

				/* on sauvegarde la commande de test */
				sv_action->cmd_test = cmd_p->test - '0';
			} else
			{
				/* on reactive */
				sv_action->cmd_test = 0;
				sv_action->act |= PIP_ACT_CAR;

			}
		}
	}
	/* si il y a une commande de forcage */
	if (cmd_p->forcage != N_AFF)
	{
		/* on signale une activation luminosite */
		if (cmd_p->forcage == '1')
			sv_action->forcage = TRUE;
		else
			sv_action->forcage = FALSE;
	}

	/* si il y a du clignotement */
	if (0 == cmd_p_nb_alt)
	{
		sv_action->clig.type = PIP_AUCUN;
	} else
	{
		switch (cmd_p->clig.type)
		{
		case PIP_DEFAUT:
			/* Recherche dans les parametres par defaut
			 * si la valeur est clignotante... */
			if (caisson->clig.type == PIP_ACTIF)
			{
				sv_action->act |= PIP_ACT_CLIG;
			}
			break;
		case PIP_ACTIF:
			sv_action->act |= PIP_ACT_CLIG;
			break;
		}
		sv_action->clig = cmd_p->clig;
	}
	if (cmd_p_nb_alt > 1)
	{
		switch (cmd_p->alt.type)
		{
		case PIP_DEFAUT:
			/* Recherche dans les parametres par defaut
			 * si la valeur est clignotante... */
			if (caisson->alt.type == PIP_AUCUN)
			{
				erreur = 3;
			}
			break;
		case PIP_AUCUN:
			erreur = 3;
			break;
		}
		sv_action->alt = cmd_p->alt;
	}
	/* on passe au caisson suivant */
	printDebug("Sortie avec erreur %d\n", erreur);
	sramMessageEcrireCaisson(j,sv_action);
	return erreur;
}

void activation_caisson_lire_action(INT indCaisson,T_pip_sv_act_caisson *sv_action)
{
	int etat = pipGetEtat();
	if (pip_cf_caisson[indCaisson].type == TYPE_DELEST)
	{
	sramMessageLireCaisson(indCaisson,sv_action);
	}
	else if ((etat & PIP_VEILLE) == PIP_VEILLE)
	{
		if ((etat & PIP_COM) == PIP_COM)
		{
			pip_aff_etat(indCaisson, sv_action, PIP_VEILLE_AVEC_COM);
		} else
		{
			pip_aff_etat(indCaisson, sv_action, PIP_VEILLE_SANS_COM);
		}
	} else
	{
		if ((etat & PIP_TEMP) == PIP_TEMP)
		{
			pip_aff_etat(indCaisson, sv_action, PIP_TEMPERATURE);
		} else if ((etat & PIP_DELESTE) == PIP_DELESTE)
		{
			pip_aff_etat(indCaisson, sv_action, PIP_DELESTAGE);
		} else if ((etat & PIP_TEST_BP) == PIP_TEST_BP)
		{
			/* Calculer ici la mire e afficher */
			int message = bpTestGetMessage();
			pip_aff_etat(indCaisson, sv_action, message);

		} else
		{
			sramMessageLireCaisson(indCaisson,sv_action);
			if (sv_action->finDv||(sv_action->neutre&&(sv_action->extinction!=TRUE)))
			{
			bool finVal=true;
				if(sv_action->neutre&&(sv_action->extinction!=TRUE))
				{
					finVal=false;
				}
				/* Test de l'expiration de la duree de validite ou passage au neutre */
				pip_aff_etat(indCaisson, sv_action, PIP_NEUTRE);
				if(true==finVal)
				{
				sv_action->validite_fin=1;
				}
			}
		}
	}
}

/* ----------------------------------
 * activation_caisson
 * ==================
 * Nouvelle fonction
 * --------------------------------	*/

int activation_caisson(INT indCaisson)
{
	int retour=0;
	T_pip_sv_act_caisson action;
	T_pip_sv_act_caisson *sv_action = &action;
	/* Lecture de l'action */
	activation_caisson_lire_action(indCaisson,sv_action);
	/* Activation. */

	retour = activation_caisson_reelle(indCaisson, sv_action);
	return retour;
}

/* ----------------------------------
 * activation_reelle_caisson
 * =========================
 * La commande permet de realiser
 * l'activation effective de l'unite
 * d'affichage en fonction de l'etat
 * dans lequel on se trouve!!!
 * --------------------------------	*/

static int activation_caisson_reelle(INT indCaisson, T_pip_sv_act_caisson *sv_action)
{
	/* Pour sauvegarder l'action courante */
	T_pip_act_caisson sauvegarde_action;
	T_pip_cf_caisson *caisson = &pip_cf_caisson[indCaisson];
	T_pip_act_caisson *action = &pip_act_caisson[indCaisson];
	INT l, m;
	INT nbcar;
	INT num_car;
	STRING ptr_buffer;
	int retour = 0;

	if (0 == sv_action->validite_fin)
	{
		eriSupprimerModule(E_eriMineure, ERI_DEF_FIN_VALIDITE, caisson->id_module, caisson->caisson);
	} else
	{
		eriAjouterModule(E_eriMineure, ERI_DEF_FIN_VALIDITE, caisson->id_module, caisson->caisson, 255);
	}

	/* Recuperation de l'action courante. */

	sauvegarde_action = pip_act_caisson[indCaisson];

	/* Adresse de la commande           . */

	action->ad = sv_action->ad;

	/* on execute la commande de positionnement pour ce caisson */
	/* si il y a une commande de positionnement */
	if (sv_action->act & PIP_ACT_CAR)
	{
		if ((sv_action->nbAlternance != 0) || sv_action->neutre)
		{
			lcr_p_caisson_neutre(caisson, action);
			action->extinction = sv_action->extinction;
			action->nbAlternance = sv_action->nbAlternance;
			if (sv_action->neutre)
			{
				action->nbAlternance = 0;
			}
			/* si caisson ALPHANUMERIQUE */
			switch (caisson->type)
			{
			case TYPE_ALPHA:

				/* on met e jour l'affichage  pour toutes les alternances */
				for (m = 0; m < action->nbAlternance; m++)
				{
					T_pip_action *alt = &sv_action->action[m];
					char buffer_mess[BUFF_MESS_P];

					nbcar = 0;
					switch (alt->type)
					{
					/* Affichage symbolique, cela veut dire que l'on n'a pas trouve de
					 * guillemet au debut de la commande. On doit alors recuperer le
					 * message prédéfini.
					 */
					case TYPE_SYMBOLIQUE:
						/* si il y a un message predefini */
						if (sv_action->type_predef[m] != N_AFF)
						{
							/* on refait une recherche du message predefini */
							nbcar = message_predefini(buffer_mess, sv_action->action[m].valeur. litteral.txt,
									&sv_action->type_predef[m]);
							ptr_buffer = buffer_mess;
						} else
						{
							/* Si il y a des symboles, c'est topologique! */
							int indTopo = caisson->indexTopo;
							T_pip_cf_topo *topo = &pip_cf_topo[indTopo];
							/* Il s'agit d'un symbole, on va chercher
							 * la valeur dans la table des symboles. */
							int index = cfes_chercher_type(topo->topoModule, topo->topoCaisson, alt->valeur.symbole,
									PIP_EQUIV_AFF);
							ptr_buffer = pip_cf_litteral[pip_cf_equiv[index].equiv.valeur]. texte;
							nbcar = min (strlen (ptr_buffer), BUFF_MESS_P);

						}
						break;
					case TYPE_LITTERAL:
						if (!sv_action->neutre)
						{
							/* on prend le massage archive */
							nbcar = sv_action->action[m].valeur.litteral.nbcar_txt;
							ptr_buffer = sv_action->action[m].valeur.litteral.txt;
						}
						break;
					}

					/* A la sortie du switch, on a un pointeur sur la liste des caracteres a
					 * afficher. Il est d'alleurs bizarre que l'on ne fasse pas de test sur
					 * le pointeur et le nombre de caracteres.
					 */

					/* les caracteres qui constituent le caisson */
					num_car = caisson->num_car;
					/* on charges les caracteres dans la table d'activation */
					l = num_car;
					/* On positionne suivant le nombre de caracteres trouves. */
					for (l = num_car; l < num_car + nbcar; l++)
					{

						ptr_buffer += pip_car_aff(&pip_act_car[l].cmd[m], ptr_buffer);
					}
					/* On complete la ligne par des espaces. */
					for (; l < num_car + caisson->nb_car; l++)
					{
						pip_car_reset(&pip_act_car[l].cmd[m]);
					}
				}
				break;



			case TYPE_DELEST:
			{
				T_pip_action *alt = &sv_action->action[0];
				switch (alt->type)
				{
				case TYPE_SYMBOLIQUE:
				{
					/* Si il y a des symboles, c'est topologique! */
					int indTopo = caisson->indexTopo;
					T_pip_cf_topo *topo = &pip_cf_topo[indTopo];
					/* Il s'agit d'un symbole, on va chercher
					 * la valeur dans la table des symboles. */
					int index = cfes_chercher_type(topo->topoModule, topo->topoCaisson, alt->valeur.symbole,
							PIP_EQUIV_AFF);
				}
					break;
				default:
					/* on met e jour l'affichage  pour toutes les alternances */
					break;
				}

			}
				break;
			}
		}
	}

	/* si il y a une commande de luminosite */
	if (sv_action->act & PIP_ACT_LUM)
	{
		action->act_lum = TRUE;
		action->luminosite = sv_action->luminosite;
	}

	/* pour le forcage */
	action->forcage = sv_action->forcage;

	/* si il y a une commande de test */
	if (sv_action->act & PIP_ACT_TEST)
	{
		/* on active le test */
		action->act_test = TRUE;
		action->cmd_test = sv_action->cmd_test;
		sv_action->cmd_test = 0;
	} else
	{
		/*on raz la commande de test */
		action->act_test = FALSE;
		action->cmd_test = 0;

		/* il y a une demande de modification de la duree de validite */
		if (sv_action->act & PIP_ACT_VAL)
		{
			/* Il s'agit de l'astuce au redemarrage pour programmer la
			 * duree de validite correcte. */
			printDebug("Activation validite dem %d ori %d val  %d\n", sv_action->validite_dem, sv_action->validite_ori,
					sv_action->validite);
			if (sv_action->validite_dem)
			{
				action->validite_ori = sv_action->validite_ori;
				action->validite = sv_action->validite_dem;
				sv_action->validite_dem = 0;
				/* Sauvegarde pour la remise a zero de la duree de validite
				 * de demarrage. */
				sramMessageEcrireCaisson (indCaisson,sv_action);
			} else
			{
				action->validite_ori = sv_action->validite;
				action->validite = sv_action->validite;
			}
		}
		/* si il y a du clignotement */
		if (sv_action->act & PIP_ACT_CLIG)
		{
			action->act_rythme = TRUE;
			action->clig.type = PIP_ACTIF;
		} else
		{
			action->clig.type = PIP_AUCUN;
		}

		/* si il y a une activation caractere */
		if (sv_action->act & PIP_ACT_CAR)
		{
			/* si il y a de l'alternance */
			/* il y a une modification du rythme d'affichage */
			if (action->nbAlternance > 1)
			{
				action->act_rythme = TRUE;
				switch (sv_action->alt.type)
				{
				case PIP_DEFAUT:
					switch (caisson->alt.type)
					{
					case PIP_ACTIF:
						action->alt = caisson->alt;
						break;
					case PIP_AUCUN:
					default:
						retour = -1;
						printDebug("Il y a un probleme de valeur par defaut!\n");
						break;
					}
					break;
				case PIP_ACTIF:
					action->alt = sv_action->alt;
					break;
				default:
					printDebug("Il y a un probleme alt %d!!!!\n", sv_action->alt.type);
					break;
				}
			}

			{
				switch (sv_action->clig.type)
				{
				case PIP_DEFAUT:
					switch (caisson->clig.type)
					{
					case PIP_ACTIF:
					case PIP_AUCUN:
						action->clig = caisson->clig;
						break;
					default:
						printDebug("Clig : Il y a un probleme de valeur par defaut! %d\n", caisson->clig.type);
						break;
					}
					break;
				case PIP_AUCUN:
				case PIP_ACTIF:
					action->clig = sv_action->clig;
					break;
				default:
					printDebug("Il y a un probleme clig %d!!!!\n", sv_action->clig.type);
					break;
				}
				/* si il y a du pseudo clignotement */

			}
		}
	}

	if (-1 != retour)
	{
		/* On compare l'ancier buffer et le nouveau. Si il y a du
		 * changement, on trace */
		if (0 != memcmp(action, &sauvegarde_action, sizeof(T_pip_act_caisson)))
		{
			pip_aff_trace(indCaisson);
		}
		/* on active la luminosite */
		/* TODO : Remplacer la fonction d'activation des caissons.... */
		//		pip_main_act_lum(indCaisson);
	} else
	{
		/* Il y a une erreur, on recupere l'ancienne configuration */
		pip_act_caisson[indCaisson] = sauvegarde_action;
	}
	return retour;

}


static int pip_aff_trace(int indCaisson)
{
	int indTopo = pip_cf_caisson[indCaisson].indexTopo;
	int mod, cais;
	char buffer[MAXLCR];
	int lgCour = 0;
	if ((-1 != indTopo) && (indTopo <= pip_nb_topo))
	{
		mod = pip_cf_topo[indTopo].topoModule;
		cais = pip_cf_topo[indTopo].topoCaisson;
	} else
	{
		mod = pip_cf_caisson[indCaisson].id_module;
		cais = pip_cf_caisson[indCaisson].caisson;
	}
	sprintf(buffer, "PE ");
	lgCour = strlen(buffer);
	lcr_pe_param(mod, cais, indCaisson, PE_TOUS_PARAM, buffer, lgCour, 0);
	printDebug("Trace position : %s\n", buffer);
	ficTraceEnregistrerPosition(&pip_act_caisson[indCaisson].ad, buffer);
	return indTopo;
}

/* --------------------------
 * pip_aff_init
 * ============
 * Initialisation de la structure
 * d'affichage avec une commande
 * d'extinction.
 * --------------------------------	*/

static void pip_aff_init(T_pip_sv_act_caisson *action)
{
	action->act = PIP_ACT_CAR | PIP_ACT_LUM;
	action->validite = 0;
	action->validite_ori = 0;
	action->forcage = FALSE;
	action->clig.type = PIP_AUCUN;
	action->alt.type = PIP_AUCUN;
	action->neutre = 0;
	action->extinction = 1;
	action->validite_fin = 0L;
	action->validite = 0L;
	/* Pour la luminosite, utiliser la luminosite minimale */
	action->luminosite.type = TYPE_AUTOMATIQUE;
	action->luminosite.valeur.graph = 0;
	action->cmd_test = 0;
	action->nbAlternance = 1;
	action->action[0].type = TYPE_ETEINT;
	ficTraceAdresseSetStandard(&action->ad, "I00000");
}

/* ----------------------------------
 * pip_aff_etat
 * ============
 * Calcul de la commande d'activation
 * en fonction de l'etat d'affichage
 * demande et du numero de caisson.
 * L'affichage de l'etat se fait que
 * sur le premier caisson. Les autres
 * caissons sont eteints.
 * --------------------------------	*/

static void pip_aff_etat(int numCaisson, T_pip_sv_act_caisson *action, int etat)
{
	pip_aff_init(action);
	printDebug("pip_aff_etat, %d, %d\n", numCaisson, etat);
	if (numCaisson < PIP_NB_MODULES)
	{
		int indice = 0;
		int trouve = 0;
		while ((pip_etat_chaine[indice].etat != -1) && (0 == trouve))
		{
			if (pip_etat_chaine[indice].etat == etat)
			{
				T_pip_etat_chaine *etatChaine = &pip_etat_chaine[indice];
				STRING chaine = etatChaine->chaine[numCaisson];
				trouve = 1;
				pip_aff_init(action);
				/* Luminosite demandee */
				if (etatChaine->lum != -1)
				{
					action->act |= PIP_ACT_LUM;
					action->luminosite.type = TYPE_NUMERIQUE;
					switch (etatChaine->lum)
					{
					case 10:
						action->luminosite.valeur.graph = pip_cf_tst_sc.num_seuil_nuit;
						break;
					case 11:
						action->luminosite.valeur.graph = pip_cf_tst_sc.num_seuil_jour;
						break;
					case 12:
						action->luminosite.valeur.graph = pip_cf_tst_sc.num_seuil_surb;
						break;
					default:
						action->luminosite.valeur.graph = etatChaine->lum;
						break;
					}
				}
				if (NULL == chaine)
				{
					printf("Extinction improbable\n");
					action->extinction = TRUE;
					action->nbAlternance = 0;
				} else
				{
					int lg = strlen(chaine);
					action->extinction = FALSE;
					action->nbAlternance = 1;

					action->action[0].type = TYPE_LITTERAL;
					action->action[0].valeur.litteral.nbcar_txt = lg;
					strncpy(action->action[0].valeur.litteral.txt, chaine, lg);
					action->action[0].valeur.litteral.txt[lg] = 0;
				}
			}
			indice++;
		} /* while((pip_etat_chaine[indice].eta */
	} /*endif(numCaisson<PIP_NB_MODULES) */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : message_predefini                                      /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 15/01/97                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_p.c                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : recherche des affichage par defaut                     /
 /-------------------------------------------------------------------------DOC*/
LOCAL BYTE message_predefini(STRING d, STRING s, INT * type_mess)
{
	BYTE j, k, l;
	BYTE trv;
	BYTE fini;
	char date[SZ_DATE];
	char mess[5][5];
	/* = {"HR0","HR1","DT0","DT1","TP0"}; */
	char *ptr_mess;
	char *ptr_cmd;

	/* on lit la date */
	mnlcal(date);

	strcpy(mess[0], "HR0");
	strcpy(mess[1], "HR1");
	strcpy(mess[2], "DT0");
	strcpy(mess[3], "DT1");
	strcpy(mess[4], "TP0");
	mess[4][3] = 0;
	/* on raz l'etat de la predefinition */
	trv = FALSE;
	fini = FALSE;
	k = 0;
	while (!fini)
	{
		j = 0;
		/* il y a cinq messages predefinis */
		while ((j < 5) && (!fini))
		{
			/* pointe sur les messages predefinis */
			ptr_mess = mess[j];
			/* pointe sur le kieme caractere du message recu */
			ptr_cmd = &s[k];

			/* on va jusqu'au bout de la chaine recue */
			if ((*ptr_cmd == CR) || (*ptr_cmd == LF) || (*ptr_cmd == 0))
			{
				/* on est en bout de chaine */
				fini = TRUE;
			}

			while ((*ptr_mess) && (*ptr_mess == *ptr_cmd))
			{
				/* on a trouve des caracteres semblables */
				ptr_mess++;
				ptr_cmd++;
			}
			if (*ptr_mess == 0)
			{
				/* on est en bout de chaine on a trouve */
				trv = TRUE;
				fini = TRUE;
			} else
			{
				/* on passe au message suivant */
				j++;
			}
		}

		if (!trv)
		{
			/* on passe au caractere suivant de la chaine recu */
			k++;
		}
	}

	if (trv)
	{
		/* on a trouve la chaine complete */
		/* on remplace dans la chaine */
		l = 0;
		if (k > 0)
			l += snprintf(d, k, s);

		/* selon le type de message trouve on remplace dans la chaine */
		switch (j)
		{
		case 0:
		case 1:
			/* message d'heure */
			l += dv1_format_date(&d[l], date, TYPE_HR);
			/* on sauvegarde le type de message et le rang */
			*type_mess = TYPE_HR;
			k += 3;
			break;

		case 2:
			/* message de date */
			l += dv1_format_date(&d[l], date, TYPE_DT0);
			/* on sauvegarde le type de message et le rang */
			*type_mess = TYPE_DT0;
			k += 3;
			break;

		case 3:
			/* message de date */
			l += dv1_format_date(&d[l], date, TYPE_DT1);
			/* on sauvegarde le type de message et le rang */
			*type_mess = TYPE_DT1;
			k += 3;
			break;

		case 4:
			/* message de temperature */
			l += sprintf(&d[l], " 00 C");
			*type_mess = TYPE_TP;
			k += 3;
			break;
		}
		/* on copie le reste du buffer */
		fini = FALSE;
		while (!fini)
		{
			d[l] = s[l];
			if ((s[l] == CR) || (s[l] == LF) || (s[l] == 0))
				fini = TRUE;
			else
			{
				l++;
				k++;
			}
		}
	} else
		l = 0;
	/* on retourne le nombre de caractere de la chaine destination */
	return (l);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cmd_p                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/08/1993                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_p.c                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande P de positionnement           /
 /-------------------------------------------------------------------------DOC*/
static BYTE lcr_cmd_p(STRING buffer, INT nbcar, INT las)
{

	INT val;
	BYTE erreur=FALSE;
	STRING ptr_deb;
	STRING ptr_fin;
	STRING mem_ptr;
	BYTE fini;
	BYTE trv;
	INT i;
	char *ptrExtinction;

	/* structure de reception de la commande de configuration */
	T_pip_cmd_p cmd_p;

	/* on initialise le pointeur */
	ptrExtinction = 0;
//
//	/* on sauvegarde l'activation en cours */
//	for(i=0;i<NB_CAISSON;i++)
//	{
//		sramMessageLireCaisson(i,&sv_act_caisson[i]);
//	}

	/* on initialise la structure de demande d'activation */
	cmd_init_p(&cmd_p);

	/* le dernier caractere est nul */
	buffer[nbcar] = 0;

	switch (*buffer)
	{
	case 'N':
	case ' ':
		cmd_p.type = TYPE_NUMERIQUE;
		break;

	case '0':
	case 'S':
		cmd_p.type = TYPE_SYMBOLIQUE;
		break;

	case 'L':
		cmd_p.type = TYPE_LITTERAL;
		break;

	case 'M':
		cmd_p.type = TYPE_MACRO;
		break;

	case '1':
		cmd_p.type = TYPE_STATUS;
		break;
	}

	/* le numero de la liaison */
	cmd_p.no_las = las;

	buffer++;
	ptr_deb = buffer;
	ptr_fin = buffer;

	printDebug("Dans lcr_cmd_p\n");

	while (((INT) (ptr_fin - buffer) <= nbcar) && (!erreur))
	{
		switch (*ptr_fin)
		{
		case 0:
		case ' ':
			/* si deux separateur ne se suivent pas */
			if (ptr_deb != ptr_fin)
			{

				/* a priori il n'y a pas d'erreur */
				erreur = FALSE;

				/* on recherche le parametre */
				if (0 == strcmp(ptr_deb, "AM="))
				{
					/* fin du parametre */
					*ptr_fin = 0;
					/* si on a deja une commande */
					if (cmd_p.flag)
					{
						/* on traite la commande */
						cmd_exec_p(&cmd_p);
					}

					/* on recoit une nouvelle commande */
					cmd_p.flag = TRUE;

					ptr_deb += 3;

					/* on recherche le parametre */
					if (*ptr_deb == '*')
					{
						/* niveau module */
						cmd_p.id_module = NN_AFF;
						ptr_deb += 2;

						if (ptr_deb < ptr_fin)
						{
							/* niveau caisson */
							if (*ptr_deb == '*')
							{
								/* niveau caisson */
								cmd_p.caisson = N_AFF;
								ptr_deb++;
							} else
							{
								if (dv1_str_symb(&ptr_deb, &val))
								{
									/* le numero de caisson   */
									cmd_p.caisson = (BYTE) val;
								} else
									erreur = TRUE;
							}
						}
						/* si le parametre n'est pas fini */
						if (ptr_deb < ptr_fin)
							erreur = TRUE;
					} else
					{
						/* si on est en numerique */
						switch (cmd_p.type)
						{
						case TYPE_NUMERIQUE:
						case TYPE_SYMBOLIQUE:
						case TYPE_LITTERAL:
							/* on recherche la liste des parametres */
							if (dv1_str_symb(&ptr_deb, &val))
							{
								/* le numero de module   */
								cmd_p.id_module = val;

								if (ptr_deb < ptr_fin)
								{
									/* si il y a un jocker */
									if (*ptr_deb == '*')
									{
										/* le numero de caisson   */
										cmd_p.caisson = N_AFF;
										/* le caractere suivant */
										ptr_deb++;
									} else if (dv1_str_symb(&ptr_deb, &val))
									{
										/* le numero de caisson   */
										cmd_p.caisson = (BYTE) val;
									} else
										erreur = TRUE;

									/* si c'est bien le dernier caractere */
									if (ptr_deb >= ptr_fin)
									{
										erreur = FALSE;
										/* on recoit une nouvelle commande */
										cmd_p.flag = TRUE;
									}
								}
							}
							if (!erreur)
							{
								/* on ajuste le type de caisson */
								if (cmd_p.id_module != NN_AFF)
								{
									for (i = 0; i < pip_nb_module; i++)
									{
										if (pip_cf_module[i].id_module == cmd_p.id_module)
										{
											switch (pip_cf_module[i].s_type)
											{
											case TYPE_ALPHA:
												cmd_p.type = TYPE_LITTERAL;
												break;
											}
										}
									}
								}
							}
							break;

						case TYPE_MACRO:
							break;
						}
					}
					/* si il y a une erreur */
					if (erreur)
						x01_cptr.erreur = CPTRD_SYNTAXE;
				} else if (0 == strcmp(ptr_deb, "AF="))
				{
					printDebug("Traitement de l'affichage\n");
					/* on positionne le pointeur */
					ptr_deb += 3;
					/* on recoit une nouvelle commande */
					cmd_p.flag = TRUE;

					/* on recherche le cas ou on affiche des blancs */
					trv = TRUE;

					/* c'est peut etre une extinction */
					if (*ptr_deb == '"')
					{

						trv = FALSE;

						if (cmd_p.type == TYPE_LITTERAL)
						{
							/* on positionne la fin du parametre */
							fini = 0;
							ptr_fin = ptr_deb;
							/* retrouver la chaine                */
							/* "mess1  "/"mess2   "/"mess3   "    */
							while (((INT) (ptr_fin - buffer) <= nbcar) && (fini != 2))
							{
								if (*ptr_fin == '"')
								{
									switch (fini)
									{
									case 0:
										fini = 1;
										ptr_fin++;
										break;
									case 1:
										ptr_fin++;
										if (*ptr_fin == '"')
										{
											ptr_fin++;
											fini = 1;
										} else if (*ptr_fin == '/')
										{
											ptr_fin++;
											fini = 0;
										} else
										{
											fini = 2;
										}
										break;
									}
								} else
								{
									if (ptrExtinction == 0)
									{
										ptrExtinction = ptr_fin;
									}
									if (*ptr_fin != ' ')
									{

										trv = TRUE;
									}
									ptr_fin++;
								}
							}
						}
					}
					/* si on a trouve que des blancs */
					if (!trv)
					{
						/* c'est une commande de neutre */
						ptr_deb = ptr_fin - 1;
						*ptr_deb = '0';
					} else if (ptrExtinction != 0)
					{
						/* on regarde si on a une "EXTINCTION" */
						if (0 == strncmp(ptrExtinction, "EXTINCTION", 10))
						{
							/* c'est une commande de neutre */
							cmd_p.extinction = TRUE;
							cmd_p.neutre = TRUE;
							ptr_deb = ptr_fin - 1;
							*ptr_deb = '0';

						}
					}

					/* a priori il y a une erreur */
					erreur = TRUE;

					/* pour la fin de chaine */
					*ptr_fin = 0;

					mem_ptr = ptr_deb;

					if ((*ptr_deb == '0') && (ptr_deb + 1 == ptr_fin))
					{
						printDebug("Neutralite\n");
						ptr_deb++;
						erreur = FALSE;
						cmd_p.neutre = TRUE;
					} else if (cmd_p.type != TYPE_LITTERAL)
					{
						printDebug("On se retrouve dans le cas bizarre\n");
						if (dv1_str_symb(&ptr_deb, &val))
						{
							if (val == DECOR_SOMBRE)
							{
								erreur = FALSE;
								cmd_p.neutre = TRUE;
								cmd_p.nb_alt = 1;
							}
						}
						printDebug("On se retrouve dans le cas bizarre %s\n", val);
					}

					if (!cmd_p.neutre)
					{
						printDebug("Traitement type de commande  %d\n", cmd_p.type);
						ptr_deb = mem_ptr;
						/* si on est en numerique */
						switch (cmd_p.type)
						{
						case TYPE_NUMERIQUE:
						case TYPE_SYMBOLIQUE:
							printDebug("Traitement du cas numerique\n");
							/* fin du parametre */
							*ptr_fin = 0;
							/* il n'y a pas d'alternance */
							cmd_p.nb_alt = 0;

							/* a priori il y a une erreur */
							erreur = FALSE;
							while ((ptr_deb < ptr_fin) && (cmd_p.nb_alt < MAX_ALTERNANCE) && (!erreur))
							{
								/* on recherche la liste des parametres */
								if (dv1_str_symb(&ptr_deb, &val))
								{
									/* le numero d'entree/sortie */
									cmd_p.action[cmd_p.nb_alt++].valeur.graph = (BYTE) val;
								} else
									erreur = TRUE;
							}

							if (ptr_deb < ptr_fin)
								erreur = TRUE;
							break;

						case TYPE_MACRO:
							erreur = TRUE;
							break;

						case TYPE_LITTERAL:
							printDebug("Traitement du cas litteral, fini =%d\n", fini);

							/* il n'y a pas d'alternance */
							cmd_p.nb_alt = 0;
							/* a priori il n'y a pas d'erreur */
							if (fini != 2)
								erreur = TRUE;
							else
								erreur = FALSE;

							while ((ptr_deb < ptr_fin) && (cmd_p.nb_alt < MAX_ALTERNANCE) && (!erreur))
							{
								/* on recherche le message */
								if (!dv1_str_litt(&ptr_deb, cmd_p.action[cmd_p.nb_alt].valeur.litteral. txt,
										BUFF_MESS_P))
								{
									erreur = TRUE;
								} else
								{
									printDebug("Texte %d val %s\n", cmd_p.nb_alt,
											cmd_p.action[cmd_p.nb_alt]. valeur.litteral.txt);
								}
								if (*ptr_deb == '/')
									ptr_deb++;
								cmd_p.nb_alt++;
							}

							if (ptr_deb < ptr_fin)
								erreur = TRUE;
							break;
						}
					} else
					{
						printDebug("C'est une commande neutre\n");
					}
				} else if (0 == strcmp(ptr_deb, "CL="))
				{
					int nb_clig = 0;
					/* on recoit une nouvelle commande */
					cmd_p.flag = TRUE;
					*ptr_fin = 0;
					ptr_deb += 3;
					cmd_p.clig.type = PIP_AUCUN;
					erreur = FALSE;
					while ((ptr_deb < ptr_fin) && (nb_clig < MAX_ALTERNANCE))
					{
						cmd_p.clig.type = PIP_ACTIF;
						if (dv1_str_atoi(&ptr_deb, &val))
						{
							cmd_p.clig.val[nb_clig++] = (BYTE) (val);
						} else
							erreur = TRUE;
					}
					if ((cmd_p.clig.type == PIP_ACTIF) && (cmd_p.clig.val[0] == 0))
					{
						cmd_p.clig.type = PIP_AUCUN;
					}

					if (ptr_deb < ptr_fin)
					{
						erreur = TRUE;
					}
				} else if (0 == strcmp(ptr_deb, "DV="))
				{
					/* on recoit une nouvelle commande */
					cmd_p.flag = TRUE;
					/* fin du parametre */
					*ptr_fin = 0;
					/* on positionne le pointeur */
					ptr_deb += 3;

					/* a priori il y a une erreur */
					erreur = TRUE;
					if (ptr_deb + 1 >= ptr_fin)
					{
						if (*ptr_deb == '0')
						{
							erreur = FALSE;
							cmd_p.validite = 0;
						} else
							erreur = TRUE;
						/* le caractere suivant */
						ptr_deb++;
					} else
					{
						/* pour etre comprehensible par dv1_str_atoi */
						ptr_deb[2] = '.';
						if (dv1_str_atoi(&ptr_deb, &val))
						{
							/* la duree en seconde */
							cmd_p.validite = (LONG) val * 3600;
							if (ptr_deb < ptr_fin)
							{
								/* pour etre comprehensible par dv1_str_atoi */
								ptr_deb[2] = '.';
								if (dv1_str_atoi(&ptr_deb, &val))
								{
									/* la duree en seconde */
									cmd_p.validite += (LONG) val * 60;
									if (ptr_deb < ptr_fin)
									{
										/* pour etre comprehensible par dv1_str_atoi */
										ptr_deb[2] = '.';
										if (dv1_str_atoi(&ptr_deb, &val))
										{
											/* la duree en seconde */
											cmd_p.validite += (LONG) val;
											erreur = FALSE;
										}
									} else
										erreur = FALSE;
								}
							} else
								erreur = FALSE;
						}
					}
				} else if (0 == strcmp(ptr_deb, "FM="))
				{
					/* fin du parametre */
					*ptr_fin = 0;
					/* on positionne le pointeur */
					ptr_deb += 3;
					cmd_p.forcage = *ptr_deb;
					if ((cmd_p.forcage != '0') && (cmd_p.forcage != '1'))
					{
						erreur = TRUE;
					}

					ptr_deb++;
					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (0 == strcmp(ptr_deb, "EC="))
				{
					/* Plus utilise...
					 cmd_p.flag = TRUE;
					 *ptr_fin = 0;
					 ptr_deb += 3;
					 switch (*ptr_deb)
					 {
					 case 'A':
					 cmd_p.lum = 'A';
					 break;
					 case 'J':
					 cmd_p.lum = 'J';
					 break;
					 case 'N':
					 cmd_p.lum = 'N';
					 break;
					 case 'S':
					 cmd_p.lum = 'S';
					 break;

					 default:
					 if ((*ptr_deb >= '0') && (*ptr_deb <= '9'))
					 cmd_p.lum = *ptr_deb;
					 else
					 erreur = TRUE;
					 break;
					 }
					 ptr_deb++;

					 if (ptr_deb < ptr_fin)
					 {
					 if ((*ptr_deb >= '0') && (*ptr_deb <= '9'))
					 {
					 cmd_p.lum_int = *ptr_deb - '0';
					 ptr_deb++;
					 }
					 else
					 {
					 erreur = TRUE;
					 }
					 }
					 if (ptr_deb < ptr_fin)
					 erreur = TRUE;
					 */
				} else if (0 == strcmp(ptr_deb, "TEST="))
				{
					/* on recoit une nouvelle commande */
					cmd_p.flag = TRUE;

					/* fin du parametre */
					*ptr_fin = 0;
					/* on positionne le pointeur */
					ptr_deb += 5;
					if ((*ptr_deb >= '0') && (*ptr_deb <= '8'))
					{
						cmd_p.test = *ptr_deb;
					} else
						erreur = TRUE;

					ptr_deb++;

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (0 == strcmp(ptr_deb, "ENT=1"))
				{
					/* on recoit une nouvelle commande */
					cmd_p.flag = TRUE;
					cmd_p.entretient = TRUE;
				} else if (0 == strcmp(ptr_deb, "ENT=0"))
				{
					/* on recoit une nouvelle commande */
					cmd_p.flag = TRUE;
					cmd_p.entretient = FALSE;
				} else
					erreur = TRUE;
			}

			/* on passe au parametre suivant */
			ptr_fin++;

			/* on supprime tous les blancs */
			while (*ptr_fin == ' ')
				ptr_fin++;

			/* sur le parametre suivant */
			ptr_deb = ptr_fin;
			break;

		default:
			ptr_fin++;
			break;
		}
	}
	if (erreur)
		x01_cptr.erreur = CPTRD_SYNTAXE;

	/* si on a deja une commande */
	if ((x01_cptr.erreur == CPTRD_OK) && (cmd_p.flag))
	{
		/* si c'est une demande d'entretient */

		if (cmd_p.entretient == 0)
		{
			/* on arrete l'entretient */
			//			lon_status.entretient = 3;
		} else if (cmd_p.entretient == 1)
		{
			/* on demarre l'entretient */
			//			if (lon_status.entretient == 0)
			//				lon_status.entretient = 1;
		}
		/* on traite la commande */
		cmd_exec_p(&cmd_p);
	}
	return erreur;
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_p                                                  /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 03/12/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_p.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : commande de positionnement d'un panneau                /
 /-------------------------------------------------------------------------DOC*/

VOID lcr_p(INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess)
{
	INT i, j, k, l, m; /* variable de boucle lecture du buffer                  */
	INT bloc=0; /* numero de bloc a envoyer                              */
	INT fini;
	BYTE qualif;
	BYTE trv;

	x01_cptr.erreur = CPTRD_OK;

	/* on se place apres le nom de la commande */
	i = position + 1;
	if ((lg_mess - i) >= 1)
	{
		switch (buffer[1])
		{
		case 'N':
		case 'L':
		case ' ':
			qualif = TYPE_NUMERIQUE;
			break;

		case '0':
			qualif = TYPE_NREP;
			break;

		case 'S':
		case 'M':
			qualif = TYPE_SYMBOLIQUE;
			break;

		case '1':
			qualif = TYPE_STATUS;
			break;
		default:
			x01_cptr.erreur = CPTRD_SYNTAXE;
			break;
		}
	} else
		qualif = TYPE_STATUS;

	/* si on est pas en fin de message, on continue l'analyse */
	if (i + 1 < lg_mess)
	{
		if (identLireAutorisationAvecNiveau(las, 0))
		{
			/* on raz la table des activation */
			for (j = 0; j < NB_CAISSON; j++)
			{
				T_pip_sv_act_caisson cais;
				T_pip_sv_act_caisson *pip_sv_act_caisson = &cais;
				sramMessageLireCaisson(j,pip_sv_act_caisson);
				pip_sv_act_caisson->act = 0;
				sramMessageEcrireCaisson(j,pip_sv_act_caisson);

			}
			/* on sauvegarde la table d'activation */
			{
				sramMessageLireTout(pip_sv_sv_act_caisson);

				/* on traite la commande, en cas d'erreur, on restaure la sauvegarde... */
				if(TRUE==lcr_cmd_p(&buffer[i], lg_mess - i, las))
				{
					sramMessageEcrireTout(pip_sv_sv_act_caisson);
				}

			}
			if (x01_cptr.erreur == CPTRD_OK)
			{
				if(!esIsSabordage())
				{
				/* on execute la commande */
				int deb = pip_util_ind_deb(0, 0);
				int fin = pip_util_ind_fin(NN_AFF, N_AFF);

				/* Si on est en mode affichage, on realise celui ci
				 * uniquement si l'on est en mode affichage. */
				if (pipGetEtat() == PIP_AFFICHAGE)
				{
					for (j = deb; j < fin; j++)
					{
					T_pip_sv_act_caisson caisson;
					T_pip_sv_act_caisson *pip_sv_act_caisson=&caisson;
						int indCaisson = pip_util_ind_caisson(j);
						sramMessageLireCaisson(indCaisson,pip_sv_act_caisson);

						if (pip_sv_act_caisson->act != 0)
						{
							/* on active le caisson */
							activation_caisson(indCaisson);
						}
					}
				}
				if (qualif != TYPE_NREP)
					tedi_ctrl(las, mode, TRUE);
				}
				else
				{
					tedi_ctrl_ext(las, mode,4);
				}
					
			}
			/* sinon, on signale l'erreur */
			else
			{
				if (qualif != TYPE_NREP)
					tedi_erreur(las, mode);
			}
		} else
		{
			x01_cptr.erreur = CPTRD_PROTOCOLE;

		}
	} else if (qualif != TYPE_STATUS)
	{
		/* on traite l'etat des panneaux */
		i = 0;
		j = 0;

		/* on initialise le numero de bloc de reponse */
		bloc = 0;

		fini = FALSE;
		while ((i < pip_nb_module) && (!fini))
		{
			k = pip_cf_module[i].num_caisson;
			while (k < pip_cf_module[i].num_caisson + pip_cf_module[i].nb_caisson)
			{
				if (j >= 150)
				{
					tedi_send_bloc(las, mode, buffer, j, bloc, FALSE, pt_mess);

					if (++bloc >= 10)
						bloc = 0;
					j = 0;

					if (vct_IT[las])
						fini = TRUE;
				}

				/* si on est en rebouclage */
				if (x01_status3.rebouclage)
				{
					/* le caisson est OK - peut etre defaut mineur */
					j += sprintf(&buffer[j], "OK ");
				} else
				{
					/* si le secteur est absent */
					//					if (!lon_status.secteur)
					//					{
					//						/* on force l'etat du caisson */
					//						pip_act_caisson[k].p_etat = P_NK;
					//					}
					switch (pip_act_caisson[k].p_etat)
					{
					case P_OK:
						/* le caisson est HS defaut majeur */
						j += sprintf(&buffer[j], "OK ");
						break;
					case P_NK:
						/* le caisson est HS defaut majeur */
						j += sprintf(&buffer[j], "NK ");
						break;
					case P_NL:
						{
							/* le caisson a des decors en defaut */
							j += sprintf(&buffer[j], "NL ");
						}
						break;
					}
				}

				j += sprintf(&buffer[j], "AM=%d.%d", (INT) pip_cf_module[i].id_module, (INT) pip_cf_caisson[k].caisson);
				trv = FALSE;

				/* si on est en test */
				if (pip_act_caisson[k].cmd_test)
				{
					j += sprintf(&buffer[j], " TEST=%d", (INT) pip_act_caisson[k].cmd_test);
				} else
				{
					j += sprintf(&buffer[j], " AF=");

					switch (pip_cf_caisson[k].type)
					{

					case TYPE_ALPHA:
						/* si le caisson est au neutre  */
						if (pip_act_caisson[k].nbAlternance == 1)
						{
							m = pip_cf_caisson[k].num_car;
							while (m < pip_cf_caisson[k].num_car + pip_cf_caisson[k].nb_car)
							{
								if (!pip_car_is_null(&pip_act_car[m].cmd[0]))
								{
									trv = TRUE;
								}
								m++;
							}
						} else if (pip_act_caisson[k].nbAlternance > 1)
						{
							trv = TRUE;
						}

						/* si le caisson est actif */
						if ((pip_act_caisson[k].nbAlternance) && (trv))
						{
							for (l = 0; l < pip_act_caisson[k].nbAlternance; l++)
							{
								buffer[j++] = '"';
								m = pip_cf_caisson[k].num_car;
								while (m < pip_cf_caisson[k].num_car + pip_cf_caisson[k].nb_car)
								{
									if ((!pip_act_car[m].def_car) || (x01_status3.rebouclage))
									{
										j += pip_car_dec(&pip_act_car[m].cmd[l], &buffer[j]);
									} else
									{
										buffer[j++] = '_';
									}
									m++;
								}
								buffer[j++] = '"';
								buffer[j++] = '/';
							}
							j--;
						} else
						{
							buffer[j++] = '0';
						}
						break;
					}
					/* le clignotement */
					j += sprintf(&buffer[j], " CL=");
					if (pip_act_caisson[k].nbAlternance > 1)
					{
						for (l = 0; l < pip_act_caisson[k].nbAlternance; l++)
						{
							j += sprintf(&buffer[j], "%d/", pip_act_caisson[k].tp_alt[l] * 10);
						}
						j--;
						buffer[j++] = ' ';

					} else if (pip_act_caisson[k].nbAlternance == 1)
					{
						/* si il y a du clignotement */
						if (pip_act_caisson[k].clig.type == PIP_ACTIF)
						{
							j += sprintf(&buffer[j], "%d/%d ", pip_act_caisson[k].clig.val[0],
									pip_act_caisson[k].clig.val[1]);
						} else
						{
							buffer[j++] = '0';
							buffer[j++] = ' ';
						}
					} else
					{
						buffer[j++] = '0';
						buffer[j++] = ' ';
					}

					/* l'eclairage */
					j += sprintf(&buffer[j], "EC=");
					switch (pip_act_caisson[k].luminosite.type)
					{
					case TYPE_AUTOMATIQUE:
						j += sprintf(&buffer[j], "AU");
						break;
					case TYPE_NUMERIQUE:
						j += sprintf(&buffer[j], "%d", pip_act_caisson[k].luminosite.valeur.graph);
						break;
					case TYPE_SYMBOLIQUE:
						j += sprintf(&buffer[j], "%s", pip_act_caisson[k].luminosite.valeur.symbole);
						break;
					}

					/* la duree de validite */
					if (pip_act_caisson[k].validite != 0L)
					{
						j += sprintf(&buffer[j], " DV=%02d:%02d:%02d", (INT) (pip_act_caisson[k].validite / 3600),
								(INT) ((pip_act_caisson[k].validite % 3600) / 60), (INT) ((pip_act_caisson[k].validite
										% 3600) % 60));
					} else
					{
						j += sprintf(&buffer[j], " DV=0");
					}

					/* si on est en forcage */
					if (pip_act_caisson[k].forcage)
					{
						j += sprintf(&buffer[j], " FM=1");
					}
					/* si il y a une condition de verrouillage */
					if (x01_status3.verrouillage)
					{
						j += sprintf(&buffer[j], " VR=1");
					}
				}
				j += sprintf(&buffer[j], "\n\r");

				/* on passe au caisson suivant */
				k++;
			}
			/* on passe au module suivant */
			i++;
		}
		/* on supprime le LF/CR */
		j -= 2;

		/* si on est en rebouclage */
		if (x01_status3.rebouclage)
		{
			buffer[j++] = x01_status1.x01_ST_TR;
		} else
		{
			buffer[j++] = vct_ST_TR;
		}

		tedi_send_bloc(las, mode, buffer, j, bloc, TRUE, pt_mess);

	} else
	{
		j = 0;
		/* si on est en rebouclage */
		if (x01_status3.rebouclage)
		{
			buffer[j++] = x01_status1.x01_ST_TR;
		} else
		{
			buffer[j++] = vct_ST_TR;
		}

		tedi_send_bloc(las, mode, buffer, j, bloc, TRUE, pt_mess);

	}
	/* *****************************
	 * FIN DE lcr_p
	 * *****************************/
}

/* *********************************
 * FIN DE LCR_P.c
 * ********************************* */

