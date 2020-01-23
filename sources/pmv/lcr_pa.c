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
#include "pip_util.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_util.h"
#include "lcr_p.h"
#include "lcr_trc.h"
#include "lcr_cfes.h"
#include "mon_debug.h"

#include "sramMessage.h"

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

static int lcr_pa_cmd(STRING buffer, INT nbcar, INT las);
static STRING lcr_pa_af(STRING, INT, INT *, T_pip_cmd_p *, int *);
static STRING lcr_pa_at(STRING, INT, INT *, T_pip_cmd_p *, int *);
static STRING lcr_pa_ec(STRING, INT, INT *, T_pip_cmd_p *, int *);
static STRING lcr_pa_cl(STRING, INT, INT *, T_pip_cmd_p *, int *);
static STRING lcr_pa_dv(STRING, INT, INT *, T_pip_cmd_p *, int *);

/* ---------------------------------
 * lcr_pa_analyse_ec
 * =================
 * --------------------------------	*/

static int lcr_pa_analyse_ec(char *param, T_pip_cmd_p *cmd_p)
{
	int flag_err = 0;
	/* Est on en presence d'un symbole :
	 * -taille inferieure e 8 caracteres,
	 * - caracteres du jeu j??? */
	/* On controle si on est en presence effective d'une
	 * equivalence symbolique */
	printDebug("lcr_pa_analyse_ec %s\n", param);
	if (pip_util_test_symbole_ec(cmd_p->id_module, cmd_p->caisson, param))
	{
		T_pip_lum *lum = &cmd_p->luminosite;
		/* Il faudrait recuperer la valeur de la luminosite.! */
		printDebug("Eclairage, equivalence symbolique %s\n", param);
		lum->type = TYPE_SYMBOLIQUE;
		strncpy(lum->valeur.symbole, param, LG_SYMB);
	} else if (pip_util_test_numerique(cmd_p->id_module, cmd_p->caisson, param))
	{
		int val;
		int lgCour;
		printDebug("Eclairage, numerique %s\n", param);
		tst_lit_entier(param, strlen(param), &lgCour, &val);
		if ((val >= 0) && (val < NB_SEUIL_CELLULE))
		{
			T_pip_lum *lum = &cmd_p->luminosite;
			printDebug("Eclairage val numerique : type %d val %d\n", TYPE_NUMERIQUE, val);
			lum->type = TYPE_NUMERIQUE;
			lum->valeur.graph = val;
		} else
		{
			printDebug("Erreur 3\n", param);
			flag_err = 3;
		}
		/* Est on en presence d'un numerique... */
	} else
	{
		printDebug("Il y a une erreur d'eclairage\n");
		flag_err = 3;
	}

	printDebug("lcr_pa_analyse_ec %s retour \n", param, flag_err);
	return flag_err;
}

/* ---------------------------------
 * lcr_pa_analyse_cl
 * =================
 * --------------------------------	*/

static int lcr_pa_analyse_cl(char *param, T_pip_cmd_p *cmd_p)
{
	int flag_err = 0;
	int t1, t2;
	T_pip_clig *clig = &cmd_p->clig;
	/* Les formes possibles pour at sont :
	 * - t,
	 * - t1/t2
	 * - /t2/t1     */
	printDebug("Dans lcr_pa_analyse_cl : #%s#\n", param);
	if ('/' == *param)
	{
		char *ptCour = param;
		int lgCour = strlen(param);
		ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
		ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t2);
		ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
		ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t1);
		if ((0 != lgCour) || (-1 == t1) || (-1 == t2))
		{
			printDebug("Erreru detectee %d %d %d\n", lgCour, t1, t2);
			flag_err = 2;
		} else
		{
			printDebug("Clignotement Cas 3 : %d/%d\n", t1, t2);
		}
	} else
	{
		char *ptCour = param;
		int lgCour = strlen(param);
		ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t1);
		if (0 == lgCour)
		{
			/* Il n'y a qu'un entier, il faut faire ce qui va bien */
			printDebug("Clignotement Cas 1 : %d/%d\n", t1, t1);
			t2 = t1;
		} else
		{
			printDebug("Clignotement analyse t2 : %d et ptCour %c\n", t1, *ptCour);
			if ('/' == *ptCour)
			{
				ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
				ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t2);
				if ((0 != lgCour) || (-1 == t1) || (-1 == t2))
				{
					printDebug("Erreru detectee %d %d %d\n", lgCour, t1, t2);
					flag_err = 2;
				} else
				{
					printDebug("Clignotement Cas 2 : %d/%d\n", t1, t2);
				}
			} else
			{
				/* Presence d'une erreur */
				printDebug("Erreur analyse du clignotement : %d/%d\n", t1, t2);
				flag_err = 2;
			}
		}

	}
	if (0 == flag_err)
	{
		pip_clig_set(clig, PIP_ACTIF, t1, t2);
		cmd_p->flag = 1;
	}
	return flag_err;

}

/* ---------------------------------
 * lcr_pa_analyse_at
 * =================
 * --------------------------------	*/

static int lcr_pa_analyse_at(char *param, T_pip_cmd_p *cmd_p)
{
	T_pip_alt *alt = &cmd_p->alt;
	int flag_err = 0;
	int t1, t2, t3, t4;
	/* Les formes possibles pour at sont :
	 * - t,
	 * - t1/t2/t3/t4
	 * - /t2/t1/t4/t3       */
	if ('/' == *param)
	{
		char *ptCour = param;
		int lgCour = strlen(param);
		ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
		ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t2);
		ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
		ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t1);
		ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
		ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t4);
		ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
		ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t3);
		if ((0 != lgCour) || (-1 == t1) || (-1 == t2) || (-1 == t3) || (-1 == t4))
		{
			flag_err = 2;
		} else
		{
			printDebug("Alternat Cas 3 : %d/%d/%d/%d\n", t1, t2, t3, t4);
		}
	} else
	{
		char *ptCour = param;
		int lgCour = strlen(param);
		ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t1);
		if (0 == lgCour)
		{
			/* Il n'y a qu'un entier, il faut faire ce qui va bien */
			printDebug("Alternat Cas 1 : %d/0/%d/0\n", t1, t1);
			t3 = t1;
			t2 = 0;
			t4 = 0;
		} else
		{
			if ('/' == *ptCour)
			{
				ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
				ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t2);
				ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
				ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t3);
				ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
				ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &t4);
				if ((0 != lgCour) || (-1 == t1) || (-1 == t2) || (-1 == t3) || (-1 == t4))
				{
					flag_err = 2;
				} else
				{
					printDebug("Alternat Cas 2 : %d/%d/%d/%d\n", t1, t2, t3, t4);
				}
			} else
			{
				/* Presence d'une erreur */
				flag_err = 2;
			}
		}

	}
	if (0 == flag_err)
	{
		pip_alt_set(alt, PIP_ACTIF, t1, t2, t3, t4);
		cmd_p->flag = 1;
	}
	return flag_err;

}

/* ---------------------------------
 * lcr_pa_analyse_af
 * =================
 * La methode permet de controler que
 * le parametre de la commande d'affichage
 * est effectivement correct.
 * --------------------------------	*/

static int lcr_pa_analyse_af(char *param, T_pip_cmd_p *cmd_p)
{
	int flag_err = 0;
	if (pip_util_test_symbole(cmd_p->id_module, cmd_p->caisson, param))
	{
		int index = cfes_chercher_type(cmd_p->id_module, cmd_p->caisson, param, PIP_EQUIV_AFF);

		T_pip_action *action = &cmd_p->action[cmd_p->nb_alt];
		printDebug("L'index du symbole est %d\n", index);
		cfes_imprimer(index);
		/* Tout va bien.... le type est symbolique et
		 * la valeur est correcte. */

		action->type = TYPE_SYMBOLIQUE;
		if(-1!=index)
		{
		if(pip_cf_equiv[index].equiv.typeEquiv==TYPE_NUMERIQUE)
		{
			action->valeur.graph = pip_cf_equiv[index].equiv.valeur;
			if(0==action->valeur.graph)
			{
				cmd_p->action[0].type = TYPE_ETEINT;
				cmd_p->extinction = TRUE;
				cmd_p->neutre = TRUE;
			}
		}
		}
		strncpy(action->valeur.symbole, param, LG_SYMB);
		cmd_p->nb_alt++;
		cmd_p->flag = 1;
	} else if (pip_util_test_numerique(cmd_p->id_module, cmd_p->caisson, param))
	{
		T_pip_action *action = &cmd_p->action[cmd_p->nb_alt];
		INT valeur;
		/* Tout va bien.... le type est numerique, on
		 * est en presence d'un ou plusieur caissons
		 * de type picto et la valeur est correcte. */
		/* Tout va bien.... le type est symbolique et
		 * la valeur est correcte. */
		action->type = TYPE_NUMERIQUE;
		dv1_atoi(param, strlen(param), (INT *) &valeur);
		action->valeur.graph = (BYTE) valeur;
		cmd_p->extinction = FALSE;
		cmd_p->nb_alt++;
		cmd_p->flag = 1;
		printDebug("L'index du symbole est %d\n", action->valeur.graph);
	} else if (pip_util_test_litteral(cmd_p->id_module, cmd_p->caisson, param))
	{
		T_pip_action *action = &cmd_p->action[cmd_p->nb_alt];
		int nbCar = min(strlen(param)-2,BUFF_MESS_P-1);
		printDebug("On est sauves, c'est du litteral %s!!!\n", param);
		/* Tout va bien.... le type est symbolique et
		 * la valeur est correcte. */
		action->type = TYPE_LITTERAL;
		action->valeur.litteral.nbcar_txt = nbCar;
		strncpy(action->valeur.litteral.txt, &param[1], nbCar);
		action->valeur.litteral.txt[nbCar] = 0;
		cmd_p->nb_alt++;
		cmd_p->flag = 1;
	} else
	{
		printDebug("Erreur lors de l'analyse de la valeur d'affichage\n");
		flag_err = 2;
	}
	return flag_err;
}

/* ----------------------------------
 * lcr_pa_af
 * =========
 * Analyse de l'affichage demande
 * --------------------------------	*/

static STRING lcr_pa_af(STRING ptCour, INT lgCour, INT * lgSuiv, T_pip_cmd_p *cmd_p, int *flag_err)
{
	int erreur = 0;
	if ('0' == *ptCour)
	{
		printDebug("Dans lcr_pa_af : extinction\n");
		ptCour++;
		lgCour--;
		cmd_p->action[0].type = TYPE_ETEINT;
		cmd_p->flag = 1;
		cmd_p->extinction = TRUE;
		cmd_p->neutre = TRUE;
		/* On controle que l'on n'est pas en presence d'un message
		 * alterne. */
	} else if ('N' == *ptCour)
	{
		printDebug("Dans lcr_pa_af neutre\n");
		/* Cas special du neutre. */
		ptCour++;
		lgCour--;
		cmd_p->action[0].type = TYPE_NEUTRE;
		cmd_p->flag = 1;
		cmd_p->neutre = TRUE;
	} else
	{
		/* Analyse de l'alternat de la forme : t, t1/t2/t3/t4 ou /t2/t1/t4/t3 */
		/* cas : t, deux fois la meme duree */
		/* cas 2 : t2 et t4 doivent etre e 0 */
		/* cas 3 : t2 et t4 doivent etre e 0 */
		char param[MAXLCR];
		ptCour = tst_lit_param_af(ptCour, lgCour, &lgCour, param);
		/* Test du parametre : est on en presence d'un symbole, d'un texte
		 * ou d'un numerique??? */
		printDebug("Dans lcr_pa_af affichage Premier parametre %s\n", param);
		erreur = lcr_pa_analyse_af(param, cmd_p);
		if ('/' == *ptCour)
		{
			ptCour++;
			lgCour--;
			ptCour = tst_lit_param_af(ptCour, lgCour, &lgCour, param);
			printDebug("Second parametre %s\n", param);
			erreur = lcr_pa_analyse_af(param, cmd_p);
		}
	}
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	*lgSuiv = lgCour;
	*flag_err = erreur;
	return ptCour;
}

/* ----------------------------------
 * lcr_pa_at
 * =========
 * Analyse de la duree de validite.
 * --------------------------------	*/

static STRING lcr_pa_at(STRING ptCour, INT lgCour, INT * lgSuiv, T_pip_cmd_p *cmd_p, int *flag_err)
{
	int erreur = 0;
	if ('0' == *ptCour)
	{
		ptCour++;
		lgCour--;
		cmd_p->alt.type = PIP_AUCUN;
		cmd_p->flag = 1;
		/* On controle que l'on n'est pas en presence d'un message
		 * alterne. */

	} else if ('D' == *ptCour)
	{
		ptCour++;
		lgCour--;
		/* Lecture du parametre par defaut pour l'AM */
		/* On controle la compatibilite avec le type d'affichage demande. */
		cmd_p->alt.type = PIP_DEFAUT;
		cmd_p->flag = 1;
	} else
	{
		/* Analyse de l'alternat de la forme : t, t1/t2/t3/t4 ou /t2/t1/t4/t3 */

		/* cas : t, deux fois la meme duree */
		/* cas 2 : t2 et t4 doivent etre e 0 */
		/* cas 3 : t2 et t4 doivent etre e 0 */
		char param[MAXLCR];
		ptCour = tst_lit_param_at(ptCour, lgCour, &lgCour, param);
		erreur = lcr_pa_analyse_at(param, cmd_p);
	}
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	*lgSuiv = lgCour;
	*flag_err = erreur;
	return ptCour;
}

/* ----------------------------------
 * lcr_pa_cl
 * =========
 * Analyse de la duree de validite.
 * --------------------------------	*/

static STRING lcr_pa_cl(STRING ptCour, INT lgCour, INT * lgSuiv, T_pip_cmd_p *cmd_p, int *flag_err)
{
	int erreur = 0;
	printDebug("Dans lcr_pa_cl\n");
	if ('0' == *ptCour)
	{
		ptCour++;
		lgCour--;
		cmd_p->clig.type = PIP_AUCUN; /* Pas de clignotement */
		cmd_p->flag = 1;
	} else if ('D' == *ptCour)
	{
		ptCour++;
		lgCour--;
		cmd_p->clig.type = PIP_DEFAUT;
		cmd_p->flag = 1;
		/* Lecture du parametre par defaut pour l'AM */
	} else
	{
		/* Analyse du clignotement : t, t1/t2 ou /t1/t1....     */
		char param[MAXLCR];
		printDebug("Analyse des parametres de clignotement\n");
		ptCour = tst_lit_param_at(ptCour, lgCour, &lgCour, param);
		erreur = lcr_pa_analyse_cl(param, cmd_p);

	}
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	*lgSuiv = lgCour;
	*flag_err = erreur;
	return ptCour;
}

/* ----------------------------------
 * lcr_pa_ec
 * =========
 * Analyse de la duree de validite.
 * --------------------------------	*/

static STRING lcr_pa_ec(STRING ptCour, INT lgCour, INT * lgSuiv, T_pip_cmd_p *cmd_p, int *flag_err)
{
	int erreur = 0;
	if (dv1_scmp_and_skip(ptCour, lgCour, "AU", 0, &ptCour, &lgCour))
	{
		/* Calcul automatique de l'eclairage. */
		T_pip_lum *lum = &cmd_p->luminosite;
		printDebug("lcr_pa_ec module automatique\n");
		lum->type = TYPE_AUTOMATIQUE;
	} else
	{
		/* Analyse de l'eclairage : symbolique ou numerique     */
		char param[MAXLCR];
		ptCour = tst_lit_param(ptCour, lgCour, &lgCour, param);
		/* Pour le parametre d'eclairage, on peut avoir un
		 * parametre symbolique....     */
		printDebug("lcr_pa_ec parametre %s\n", param);
		erreur = lcr_pa_analyse_ec(param, cmd_p);

	}
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	*lgSuiv = lgCour;
	*flag_err = erreur;
	return ptCour;
}

/* ----------------------------------
 * lcr_pa_dv
 * =========
 * Analyse de la duree de validite.
 * --------------------------------	*/

static STRING lcr_pa_dv(STRING ptCour, INT lgCour, INT * lgSuiv, T_pip_cmd_p *cmd_p, int *flag_err)
{
	int erreur = 0;
	if (('0' == *ptCour) && (!((lgCour > 1) && (((ptCour[1] >= '0') && (ptCour[1] <= '9')) || (ptCour[1] == ':')))))
	{
		ptCour++;
		lgCour--;
		cmd_p->validite = 0;
		cmd_p->flag = 1;
	} else if ('D' == *ptCour)
	{
		/* Lecture du parametre par defaut pour l'AM */
		ptCour++;
		lgCour--;
		/* Par convention, on va supposer qu'une validite e -1 est
		 * utilise pour designer la valeur par defaut. */
		cmd_p->validite = -1;
		cmd_p->flag = 1;
	} else
	{
		/* Analyse de l'eclairage : symbolique ou numerique     */
		long validite;
		int heureOk;
		ptCour = tst_lit_dv(ptCour, lgCour, &lgCour, &validite, &heureOk);
		if (heureOk)
		{
			printDebug("La duree de validite est correcte %d\n", validite);
			cmd_p->validite = validite;
			cmd_p->flag = 1;
		} else
		{
			erreur = 3;
		}
	}

	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	*lgSuiv = lgCour;
	*flag_err = erreur;
	return ptCour;
}

/* ---------------------------------
 * lcr_pa_am
 * =========
 * Analyse des parametres associes a
 * un am.
 * --------------------------------	*/

static STRING lcr_pa_am(STRING ptCour, INT lgCour, INT *lgSuiv, T_pip_cmd_p *cmd_p, int *flag_err)
{
	int erreur = 0;
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if (0 < lgCour)
	{
		if (dv1_scmp_and_skip(ptCour, lgCour, "AF=", 0, &ptCour, &lgCour))
		{
			printDebug("lcr_pa_am, AF trouve \n");
			ptCour = lcr_pa_af(ptCour, lgCour, &lgCour, cmd_p, &erreur);
			/* Placer ici la lecture des l'affichage. */
			if (0 == erreur)
			{
				if (dv1_scmp_and_skip(ptCour, lgCour, "AT=", 0, &ptCour, &lgCour))
				{
					printDebug("lcr_pa_am, AT trouve \n");
					/* Analyse du parametre alternat et controle de coherence */
					ptCour = lcr_pa_at(ptCour, lgCour, &lgCour, cmd_p, &erreur);
				}
				if (dv1_scmp_and_skip(ptCour, lgCour, "CL=", 0, &ptCour, &lgCour))
				{
					printDebug("lcr_pa_am, CL trouve \n");
					/* Analyse du parametre clignotement */
					ptCour = lcr_pa_cl(ptCour, lgCour, &lgCour, cmd_p, &erreur);
				}
				if (dv1_scmp_and_skip(ptCour, lgCour, "EC=", 0, &ptCour, &lgCour))
				{
					printDebug("lcr_pa_am, EC trouve \n");

					/* Analyse du parametre d'eclairage */
					ptCour = lcr_pa_ec(ptCour, lgCour, &lgCour, cmd_p, &erreur);
				}
				if (dv1_scmp_and_skip(ptCour, lgCour, "DV=", 0, &ptCour, &lgCour))
				{
					printDebug("lcr_pa_am, DV trouve \n");
					/* Analyse de la duree de validite */
					ptCour = lcr_pa_dv(ptCour, lgCour, &lgCour, cmd_p, &erreur);
				}
			}
		} else if (dv1_scmp_and_skip(ptCour, lgCour, "EC=", 0, &ptCour, &lgCour))
		{
			/* Analyse du parametre d'eclairage */
			printDebug("lcr_pa_am, EC trouve \n");

			/* Analyse du parametre d'eclairage */
			ptCour = lcr_pa_ec(ptCour, lgCour, &lgCour, cmd_p, &erreur);
		} else
		{
			printDebug("Probleme de parametrage \n");
			erreur = 2; /* Erreur de parametre. Seuls EC et AF
			 sont autorises */
		}
	} else
	{
		erreur = 2; /* Erreur de parametre. Seuls EC et AF */
	} /* endif(0<lgCour) */
	*flag_err = erreur;
	*lgSuiv = lgCour;
	return ptCour;
}

/* ---------------------------------
 * lcr_pa_cmd
 * ==========
 * Traitement de la commande d'affi-
 * chage nouvelle generation. Celle
 * ci se veut conforme e la norme
 * LCR classe 3.
 * --------------------------------	*/
static int lcr_pa_cmd(STRING buffer, INT nbcar, INT las)
{
	int flag_err = 0;
	char *ptCour = &buffer[1];
	int lgCour = nbcar - 1;
	T_pip_cmd_p cmd_p;

	printDebug("Dans lcr_pa_cmd\n");
	cmd_init_p(&cmd_p);
	/* Initialisation de la structure de gestion de commande */
	/* Passage des premiers blancs */
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	/* Tant qu'il y a des caracteres et pas d'erreur */
	while ((0 < lgCour) && (0 == flag_err))
	{

		printDebug("Dans lcr_pa_cmd, en cours de traitement %d\n", lgCour);
		if (dv1_scmp(ptCour, "AM=", 0))
		{
			int mod, cais, amOk;
			printDebug("Dans lcr_pa_cmd, AM trouve %d\n", lgCour);
			ptCour += strlen("AM=");
			lgCour -= strlen("AM=");
			ptCour = tst_lit_am_bis(ptCour, lgCour, &lgCour, &mod, &cais, &amOk);
			if (amOk)
			{
				/* Controle de l'existance effective du ou des modules
				 * designes. */
				if (pip_util_ind_ctrl(mod, cais))
				{
					printDebug("Dans lcr_pa_cmd, AM correct %d\n", lgCour);

					cmd_p.id_module = mod;
					cmd_p.caisson = cais;
					ptCour = lcr_pa_am(ptCour, lgCour, &lgCour, &cmd_p, &flag_err);
					/* Si tout va bien, on execute la commande */
					if (0 == flag_err)
					{
						cmd_exec_p(&cmd_p);
					}
				} else
				{
					printDebug("Dans lcr_pa_cmd, AM incorrect %d.%d\n", mod, cais);
					flag_err = 2; /* Couple module/caisson incorrect */
				}
			} else
			{
				printDebug("Dans lcr_pa_cmd, AM illisible %d.%d\n", mod, cais);
				flag_err = 2; /* Erreur d'analyse du numero de module */
			} /* endif(amOk */
		} else
		{
			flag_err = 2; /* Erreur, on doit obligatoirement avoir un AM          */
		} /* endif(dv1_scmp("AM=" */
	} /* endwhile((0<lgCour) */
	return flag_err;
}

VOID lcr_pa(INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess)
{
	INT i, j;
	INT bloc = 0;
	INT afficheStr = 0;

	int flag_err = 0;
	x01_cptr.erreur = CPTRD_OK;

	/* on se place apres le nom de la commande */
	i = position + 1;
	if ((lg_mess - i) >= 1)
	{
		switch (buffer[1])
		{
		case 'A':
			break;
		case '1':
			afficheStr = 1;
			break;
		default:
			flag_err = 2;
			break;
		}
	} else
	{
		flag_err = 2;
	}

	/* si on est pas en fin de message, on continue l'analyse */
	if ((0 == flag_err) && (i + 1 < lg_mess))
	{
#if 0
		if (identLireAutorisationAvecNiveau (las, 0))
		{
#endif
		/* on raz la table des activation */
		for (j = 0; j < NB_CAISSON; j++)
		{
			T_pip_sv_act_caisson act_caisson;
			T_pip_sv_act_caisson *pip_sv_act_caisson = &act_caisson;
			sramMessageLireCaisson(j, pip_sv_act_caisson);
			pip_sv_act_caisson->conf = 0;
			sramMessageEcrireCaisson(j, pip_sv_act_caisson);
		}
		/* on sauvegarde la table d'activation */
		{
			sramMessageLireTout(pip_sv_sv_act_caisson);
			/* on raz la condition de verrouillage */
			flag_err = lcr_pa_cmd(&buffer[i], lg_mess - i, las);

			/* si il y a verrouillage ou une erreur de commande*/
			if (0 != flag_err)
			{
				sramMessageEcrireTout(pip_sv_sv_act_caisson);
			}

		}

	}
	/* Activation des caissons : on peut eventuellement retourner une
	 * erreur de parametrage!!!! */

	if (0 == flag_err)
	{
		if(!esIsSabordage())
		{
		int deb, fin;
		int retour = 0;
		deb = pip_util_ind_deb(0, 0);
		fin = pip_util_ind_fin(NN_AFF, N_AFF);

		for (j = deb; (j < fin) && (-1 != retour); j++)
		{
			T_pip_sv_act_caisson caisson;
			T_pip_sv_act_caisson *pip_sv_act_caisson = &caisson;
			int indCaisson = pip_util_ind_caisson(j);
			sramMessageLireCaisson(indCaisson, pip_sv_act_caisson);
			/* Uniquement sur les caissons parametres pendant la
			 * commande. */
			if (pip_sv_act_caisson->conf != 0)
			{
				/* Il faut sauvegarder l'adresse du port utilise... */
				ficTraceGetAdressePort(las, &pip_sv_act_caisson->ad);
				sramMessageEcrireCaisson(indCaisson, pip_sv_act_caisson);
				retour = activation_caisson(indCaisson);
			}
		}
		if (-1 == retour)
		{
			flag_err = 3;
		}
		}
		else
		{
			flag_err=4;
		}
	}

	/* Retour de la commande */
	if (0 == flag_err)
	{

		j = 0;
		/* si on est en rebouclage */
		if (afficheStr)
		{
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
			tedi_ctrl(las, mode, TRUE);
		}
	} else
	{
		printDebug("Probleme %d\n", flag_err);
		tedi_ctrl_ext(las, mode, flag_err);
	}
}
