/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_st                                              /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 21/12/1996                                               /
 /-----------------------------------------------------------------------------/
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

/*
 * LCR_CFES.c
 * ===================
 * FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la con-
 * figuration des communication de la station avec l'exterieur.
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: LCR_CFES.c,v $
 * Revision 1.2  2018/06/04 08:38:43  xg
 * Passage en version V122
 *
 * Revision 1.5  2008/10/24 15:55:16  xag
 * Suppression du debug.
 * Ajout de l'identification obligatoire sur configuration.
 *
 * Revision 1.4  2008/10/16 08:20:01  xag
 * Corrections.
 *
 * Revision 1.3  2008/10/13 08:18:17  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.2  2008/07/04 17:05:05  xag
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
#include <string.h>
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_str.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "tac_conf.h"
#include "lcr_cfes.h"
#include "pip_str.h"
#include "pip_var.h"
#include "lcr_util.h"
#include "pip_util.h"

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* *************************************
 * PROTOTYPE DES FONCTIONS LOCALES
 * ************************************* */

static int cfes_ajouter(int, int, STRING, T_pip_equiv);
static int cfes_supprimer_type(int, int, int);

/* -------------------------------
 * cfes_imprimer
 * =============
 * Impression d'un index de la
 * table des equivalences.
 * ----------------------------	*/

void cfes_imprimer(int index)
{
	if (index < pip_nb_equiv)
	{
		T_pip_cf_equiv *equiv = &pip_cf_equiv[index];
		printDebug("Module %d.%d, equiv %s type %d ", equiv->module, equiv->caisson, equiv->symbole,
				equiv->equiv.typeEquiv);
		if (equiv->equiv.typeSymb == PIP_SYMB_LITTERAL)
		{
			printDebug("valeur %s\n", pip_cf_litteral[equiv->equiv.valeur].texte);
		} else
		{
			printDebug("valeur %d\n", equiv->equiv.valeur);
		}

	}
}
void cfes_imprimer_tout(void)
{
	int indice;
	for (indice = 0; indice < pip_nb_equiv; indice++)
	{
		printDebug("Equiv %02d ", indice);
		cfes_imprimer(indice);
	}
}

/* ------------------------------
 * lcr_cfes_init
 * =============
 * Reinitialisation de la table
 * des equivalences symboliques.
 * ----------------------------	*/

int lcr_cfes_init(void)
{
	if (0 != pip_nb_equiv)
	{
		pip_nb_equiv = 0;
		pip_nb_litteral = 0;
	}
	return TRUE;
}

/* Il faut creer les fonctions suivantes :
 * - ajout d'une equivalence,
 * - suppression d'une equivalence,
 * - remise e zero des equivalences pour un
 *   module,
 * - remise e zero des equivalences pour un
 *   caisson. */

/* --------------------------------
 * cfes_index.
 * ==========
 * Recherche de l'index d'une
 * equivalence pour un couple module,
 * caisson donne.
 * --------------------------------	*/

static int cfes_index(int module, int caisson, STRING equiv)
{
	int index = 0;
	int trouve = 0;
	if ((0 < pip_nb_equiv) && (0 == trouve))
	{
		printDebug("Recherche du module %d index %d\n", module, index);
		while ((pip_cf_equiv[index].module < module) && (index < pip_nb_equiv))
		{
			index++;
		}
		printDebug("Recherche du caisson %d index %d\n", caisson, index);
		while ((pip_cf_equiv[index].module == module) && ((pip_cf_equiv[index].caisson) < caisson) && (index
				< pip_nb_equiv))
		{
			index++;
		}
		printDebug("Recherche du symbole %s index %d\n", equiv, index);
		while ((pip_cf_equiv[index].module == module) && ((pip_cf_equiv[index].caisson) == caisson) && (strncmp(
				pip_cf_equiv[index].symbole, equiv, LG_SYMB) < 0) && (index < pip_nb_equiv))
		{
			index++;
		}
	}
	return index;
}

/* -------------------------
 * cfes_chercher_type
 * ==================
 * Recherche d'une equivalence
 * pour un type d'equivalence
 * donne.
 * ----------------------------	*/

int cfes_chercher_type(int module, int caisson, STRING equiv, int type)
{
	int index = cfes_chercher(module, caisson, equiv);
	printDebug("cfes_chercher_type : module %d.%d equiv %s type %d\n", module, caisson, equiv, type);
	if (-1 != index)
	{
		if (pip_cf_equiv[index].equiv.typeEquiv != type)
		{
			index = -1;
		}
	}
	return index;
}

/* ----------------------------
 * cfes_chercher
 * =============
 * Recherche dans la liste des
 * equivalence si l'equivalence
 * est presente.
 * ---------------------------- */

int cfes_chercher(int module, int caisson, STRING equiv)
{
	int index = 0;
	int trouve = 0;
	printDebug("cfes_chercher\n");
	if ((0 < pip_nb_equiv) && (0 == trouve))
	{
		index = cfes_index(module, caisson, equiv);
		if ((pip_cf_equiv[index].module == module) && ((pip_cf_equiv[index].caisson) == caisson) && (strncmp(
				pip_cf_equiv[index].symbole, equiv, LG_SYMB) == 0) && (index < pip_nb_equiv))
		{
			trouve = 1;
		}
	}
	return (trouve ? index : -1);
}

/* --------------------------------
 * cfes_chercher_valeur
 * ====================
 * Il s'agit de chercher dans la
 * liste des symbole dont le type
 * correspond, si la valeur est
 * deje utilisee. */

int cfes_chercher_valeur(int module, int caisson, BYTE typeSymb, BYTE typeEquiv, UINT valeur)
{
	int index = -1;
	if (0 < pip_nb_equiv)
	{
		int indice = cfes_index(module, caisson, "");
		printDebug("cfes_chercher_valeur type %d equiv %d val %d de %d max %d\n", typeSymb, typeEquiv, valeur, indice,
				pip_nb_equiv);
		while ((indice < pip_nb_equiv) && (pip_cf_equiv[indice].module == module) && ((pip_cf_equiv[indice].caisson)
				== caisson) && (-1 == index))
		{
			T_pip_cf_equiv *equiv = &pip_cf_equiv[indice];
			printDebug("-- Recherche pour %s type %d equiv %d val %d\n", equiv->symbole, equiv->equiv.typeSymb,
					equiv->equiv.typeEquiv, equiv->equiv.valeur);
			if ((equiv->equiv.typeSymb == typeSymb) && (equiv->equiv.typeEquiv == typeEquiv) && (equiv->equiv.valeur
					== valeur))
			{
				printDebug("Trouve %d", index);
				index = indice;
			} else
			{
				indice++;
			}
		}
	}
	return index;
}

/* ----------------------------
 * cfes_trouver
 * =============
 * Recherche dans la liste des
 * equivalence si l'equivalence
 * est presente. On recherche ici
 * eventuellement les groupes.
 * ---------------------------- */
//static int cfes_trouver (int module, int caisson, STRING equiv)
//{
//  int index = -1;
//  /* Recherche normale */
//  index = cfes_chercher (module, caisson, equiv);
//  if (-1 == index)
//    {
//      /* Recherche  si il n'y a pas une regle pour le module */
//      index = cfes_chercher (module, N_AFF, equiv);
//      if (-1 == index)
//        {
//          /* Recherche  si il n'y a pas une regle pour tous les modules */
//          index = cfes_chercher (NN_AFF, N_AFF, equiv);
//        }
//    }
//  return index;
//}

/* ----------------------------
 * cfes_trouver_type
 * =================
 * Recherche dans la liste des
 * equivalence si l'equivalence
 * est presente. On recherche ici
 * eventuellement les groupes.
 * ---------------------------- */
int cfes_trouver_type(int module, int caisson, STRING equiv, int type)
{
	int index = -1;
	/* Recherche normale */
	index = cfes_chercher_type(module, caisson, equiv, type);
	if (-1 == index)
	{
		/* Recherche  si il n'y a pas une regle pour le module */
		index = cfes_chercher_type(module, N_AFF, equiv, type);
		if (-1 == index)
		{
			/* Recherche  si il n'y a pas une regle pour tous les modules */
			index = cfes_chercher_type(NN_AFF, N_AFF, equiv, type);
		}
	}
	return index;
}

static void cfes_standard_type_const(int index, int type)
{
	T_pip_cf_caisson *caisson = &pip_cf_caisson[index];

	switch (type)
	{
	case PIP_EQUIV_AFF:
		switch(caisson->type)
		{
			case TYPE_DELEST:
			{
				int mod=pip_cf_topo[caisson->indexTopo].topoModule;
				int cais=pip_cf_topo[caisson->indexTopo].topoCaisson;
				T_pip_equiv val;
					val.typeEquiv=PIP_EQUIV_AFF;
					val.typeSymb=PIP_SYMB_NUMERIQUE;
					val.valeur=1;
					if(cfes_tester_picto(mod,cais,val.valeur))
					{
						cfes_ajouter(mod,cais,"ALM1",val);
					}
					val.valeur=0;
					if(cfes_tester_picto(mod,cais,val.valeur))
					{
						cfes_ajouter(mod,cais,"ALM0",val);
					}

			}
			break;
		}

		break;
	case PIP_EQUIV_EC:
		/* Positionnement des equivalences standard!!! */
	{
		int mod = pip_cf_topo[caisson->indexTopo].topoModule;
		int cais = pip_cf_topo[caisson->indexTopo].topoCaisson;
		int indCour;
		T_pip_equiv val;
		val.typeEquiv = PIP_EQUIV_EC;
		val.typeSymb = PIP_SYMB_NUMERIQUE;
		/* Seuil Nuit */
		for (indCour = 0; indCour < NB_SEUIL_CELLULE; indCour++)
		{
			if (0 == pip_seuil_cellule[indCour].cell)
			{
				val.valeur = indCour;
				cfes_ajouter(mod, cais, "NU", val);
				break;
			}
		}
		/* Seuil Jour */
		for (indCour = 0; indCour < NB_SEUIL_CELLULE; indCour++)
		{
			if (1 == pip_seuil_cellule[indCour].cell)
			{
				val.valeur = indCour;
				cfes_ajouter(mod, cais, "JO", val);
				break;
			}
		}
		/* Seuil Surbrillance */
		for (indCour = 0; indCour < NB_SEUIL_CELLULE; indCour++)
		{
			if (2 == pip_seuil_cellule[indCour].cell)
			{
				val.valeur = indCour;
				cfes_ajouter(mod, cais, "SU", val);
				break;
			}
		}
	}
		break;
	}
}
/* ----------------------------
 * cfes_standard_type
 * ===================
 * Equivalences standard pour le
 * modules designes.
 * ----------------------------	*/

static int cfes_standard_type(int module, int caisson, int type)
{
	int config = FALSE;
	/* Recherche du module */
	config = cfes_supprimer_type(module, caisson, type);
	if (0 < pip_nb_topo)
	{
		int deb = pip_util_ind_deb_topo(module, caisson);
		int fin = pip_util_ind_fin_topo(module, caisson);
		if (deb < fin)
		{
			int indCour;
			config = TRUE;
			for (indCour = deb; indCour < fin; indCour++)
			{
				cfes_standard_type_const(pip_cf_topo[indCour].indexCaisson, type);
			}
		}
	}
	return config;
}

/* ------------------------------
 * cfes_tester_alpha
 * =====================
 * Controle qu'une liste de caisson
 * est bien alphanumerique.
 * ----------------------------	*/
static int cfes_tester_alpha(int mod, int cais)
{
	int valOk = 0;
	if (0 < pip_nb_topo)
	{
		int deb = pip_util_ind_deb_topo(mod, cais);
		int fin = pip_util_ind_fin_topo(mod, cais);
		if (deb < fin)
		{
			int indCour;
			valOk = 1;
			for (indCour = deb; (indCour < fin) && valOk; indCour++)
			{
				int indCaisson = pip_cf_topo[indCour].indexCaisson;
				if ((pip_cf_caisson[indCaisson].type & TYPE_ALPHA) == 0)
				{
					printDebug("cfes_tester_alpha, %#x\n", pip_cf_caisson[indCaisson].type);

					valOk = 0;
				}
			}
		}
	}
	return valOk;
}
int cfes_tester_picto(int mod, int cais,int val)
{
	int valOk = 0;
	if (0 < pip_nb_topo)
	{
		int deb = pip_util_ind_deb_topo(mod, cais);
		int fin = pip_util_ind_fin_topo(mod, cais);
		if (deb < fin)
		{
			int indCour;
			valOk = 1;
			for (indCour = deb; (indCour < fin) && valOk; indCour++)
			{
				int indCaisson = pip_cf_topo[indCour].indexCaisson;
				switch(pip_cf_caisson[indCaisson].type)
				{
					case TYPE_DELEST:
					{
						if((val==0)||(val==1))
						{
							valOk=1;
						}
					}
					break;
					default:
						valOk=0;
						break;
				}
			}
		}
	}
	return valOk;
}

static int cfes_supprimer_index(int index)
{
	int indice;
	int config = FALSE;
	if (index < (pip_nb_equiv - 1))
	{
		printDebug("cfes_supprimer_index %d\n", index);
		for (indice = index; indice < (pip_nb_equiv - 1); indice++)
		{
			pip_cf_equiv[indice] = pip_cf_equiv[indice + 1];
		}

		pip_nb_equiv--;
		config = TRUE;
	}
	return config;
}

/* ----------------------------
 * cfes_ajouter
 * ============
 * Ajout d'une equivalence
 * symbolique (ou remplacement)
 * ----------------------------	*/

static int cfes_ajouter(int mod, int cais, STRING equiv, T_pip_equiv val)
{
	int index = -1;
	printDebug("cfes_ajouter\n");
	/* TODO : Il faut distinguer l'equivalence numerique de l'equivalence
	 * symbolique. */
	if (0 != pip_nb_topo)
	{
		int indice;
		int deb = pip_util_ind_deb_topo(mod, cais);
		int fin = pip_util_ind_fin_topo(mod, cais);
		for (indice = deb; indice < fin; indice++)
		{
			int module = pip_cf_topo[indice].topoModule;
			int caisson = pip_cf_topo[indice].topoCaisson;
			if (val.typeSymb == PIP_SYMB_NUMERIQUE)
			{
				printDebug("Recherche de la valeur eventuelle \n");
				index = cfes_chercher_valeur(module, caisson, val.typeSymb, val.typeEquiv, val.valeur);
				if (-1 != index)
				{
					cfes_supprimer_index(index);
				}
			}
			/* Rechercher eventuellement l'equivalence pour la remplacer. */
			index = cfes_chercher(module, caisson, equiv);
			/* Si l'index n'existe pas, rechercher l'emplacement de l'insertion */
			if (-1 == index)
			{
				index = cfes_index(module, caisson, equiv);
				/* Controler qu'il reste des places */
				if ((index < NB_EQUIV) && (pip_nb_equiv < NB_EQUIV))
				{
					/* Deplacer eventuellement les autres objets */
					int indice = pip_nb_equiv;
					for (indice = pip_nb_equiv; indice > index; indice--)
					{
						pip_cf_equiv[indice] = pip_cf_equiv[indice - 1];
					}
				} else
				{
					index = -1;
				}
			} else
			{
				printDebug("cfes_ajouter, index trouve %d\n", index);
				/* Si il s'agit d'une equivalence numerique, il faut supprimer
				 * l'equivalence puis la recreer eventuellement */
			}
			/* L'element est trouve, on l'initialise */
			if (-1 != index)
			{
				T_pip_cf_equiv *element = &pip_cf_equiv[index];
				printDebug("Ajout d'une equivalence a l'emplacement %d\n", index);
				element->module = module;
				element->caisson = caisson;
				strncpy(element->symbole, equiv, LG_SYMB);
				element->equiv = val;
				pip_nb_equiv++;
			}
		} /* endfor( */
	}
	/* On retourne l'index de l'element et -1 si il n'est pas trouve */
	return index;
}
static int cfes_supprimer_type(int module, int caisson, int type)
{
	int config = FALSE;
	int deb, fin;
	if (NN_AFF == module)
	{
		deb = 0;
		fin = pip_nb_equiv;
	} else
	{
		if (N_AFF == caisson)
		{
			deb = cfes_index(module, 0, (STRING) "");
			fin = cfes_index(module + 1, 0, (STRING) "");
		} else
		{
			/* Recherche de toutes les equivalences pour un caisson donne */
			deb = cfes_index(module, caisson, (STRING) "");
			fin = cfes_index(module, caisson + 1, (STRING) "");
		}

	}
	if (deb != fin)
	{
		int indCour = 0;
		while ((deb + indCour) < fin)
		{
			if (pip_cf_equiv[deb + indCour].equiv.typeEquiv == type)
			{
				int cour = deb + indCour;
				int val = pip_nb_equiv - 1;
				fin -= 1;
				pip_nb_equiv -= 1;
				for (; cour < val; cour++)
				{
					pip_cf_equiv[cour] = pip_cf_equiv[cour + 1];
				}
				config = TRUE;
				cfes_imprimer_tout();
			} else
			{
				indCour++;
			}
		}
	}
	return config;
}

static int cfes_ajouter_litteral(char *litteral)
{
	int index = -1;
	int indice;
	for (indice = 0; (indice < pip_nb_litteral) && (-1 != index); indice++)
	{
		if (0x7F == pip_cf_litteral[indice].texte[0])
		{
			index = indice;
		}
	}
	if (-1 == index)
	{
		if (pip_nb_litteral < NB_LITTERAL)
		{
			index = pip_nb_litteral;
			pip_nb_litteral++;
		}
	}
	if (-1 != index)
	{
		int nbCar = min (strlen (litteral) - 2, BUFF_MESS_P);
		strncpy(pip_cf_litteral[index].texte, &litteral[1], nbCar);
		if (nbCar < BUFF_MESS_P)
		{
			pip_cf_litteral[index].texte[nbCar] = 0;
		}
	}
	return index;
}

/* ------------------------------
 * lcr_cfes_af
 * ===========
 * Configuration des equivalences
 * d'affichage.
 * --------------------------------	*/

static int lcr_cfes_af(STRING ptCour, INT lgCour, INT * lgSuiv, INT * config, INT *amMod, INT *amCais)
{
	int flag_err = 0;
	printDebug("lcr_cfes_af...\n");
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if (dv1_scmp_and_skip(ptCour, lgCour, "AM=", 0, &ptCour, &lgCour))
	{
		int mod, cais, amOk;
		ptCour = tst_lit_am_bis(ptCour, lgCour, &lgCour, &mod, &cais, &amOk);
		if (amOk)
		{
			*amMod = mod;
			*amCais = cais;
			ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
			if ((1 == lgCour) && ('Z' == *ptCour))
			{
				lgCour--;
				ptCour++;
				/* Reinit des modules selectionnes */
				*config = cfes_supprimer_type(mod, cais, PIP_EQUIV_AFF);
			} else if ((1 == lgCour) && ('S' == *ptCour))
			{
				lgCour--;
				ptCour++;
				if (TRUE == cfes_standard_type(mod, cais, PIP_EQUIV_AFF))
				{
					*config = TRUE;
				}
			} else if (0 != lgCour)
			{
				char symbole[MAXLCR];
				/* Il faut ici analyser le nom de l'equivalence et ensuite
				 * analyse ce qui suit. */
				printDebug("lcr_cfes_af, am : %d.%d\n", mod, cais);
				ptCour = tst_lit_param_symb(ptCour, lgCour, &lgCour, symbole);
				if (pip_util_test_nom_symbole(symbole))
				{
					printDebug("lcr_cfes_af, symbole : %s\n", symbole);
					if ('=' == *ptCour)
					{
						char param[MAXLCR];
						ptCour = tst_lit_param_af(ptCour + 1, lgCour - 1, &lgCour, param);
						if (pip_util_test_litteral(mod, cais, param))
						{
							T_pip_equiv equiv;
							printDebug("lcr_cfes_af, litteral : %s\n", param);
							/* Nous sommes dans le cas d'une equivalence
							 * symbolique pour des caissons de type alpha.
							 * On verifie que c'est bien le cas et on
							 * cree l'equivalence. */
							equiv.typeEquiv = PIP_EQUIV_AFF;
							equiv.typeSymb = PIP_SYMB_LITTERAL;
							if (cfes_tester_alpha(mod, cais))
							{
								/* Il faut trouver une case libre dans les
								 * litteraux!!!! */
								equiv.valeur = cfes_ajouter_litteral(param);
								if ((-1 != cfes_ajouter(mod, cais, symbole, equiv)) && (-1 != equiv.valeur))
								{
									*config = TRUE;
								}
							} else
							{
								printDebug("Les caisson ne sont pas alpha\n");
								flag_err = 2;
							}

						} else if (pip_util_test_numerique(mod, cais, param))
						{
							T_pip_equiv equiv;
							int valOk=1;
							dv1_atoi(param,strlen(param),&equiv.valeur);
							valOk=cfes_tester_picto(mod,cais,equiv.valeur);
							if(valOk)
							{
								equiv.typeEquiv=PIP_EQUIV_AFF;
								equiv.typeSymb=PIP_SYMB_NUMERIQUE;
								if(-1!=cfes_ajouter(mod,cais,symbole,equiv))
								{
									*config=TRUE;
								}
							}
							else
							{
								flag_err=3;
							}
						} else
						{
							printDebug("Ni num ni litt #%s#\n", param);
							/* Erreur de valeur de parametre.       */
							flag_err = 3;
						}

					} else
					{
						flag_err = 2; /* probleme de parametre */
					}
				} else
				{
					printDebug("Le symbole est incorrect %s\n", symbole);
					flag_err = 2;
				}

			}
		} else
		{
			printDebug("Probleme de lecture de l'AM\n");
			flag_err = 2;
		}
	} else if (dv1_scmp_and_skip(ptCour, lgCour, "AM", 0, &ptCour, &lgCour))
	{
		/* Il faut controler qu'il n'y a plus rien apres */
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		if (0 != lgCour)
		{

			printDebug("Erreur de parametre, rien ne doit suivre le AM\n");
			flag_err = 2;
		}

	} else
	{

		if (0 != lgCour)
		{
			printDebug("Erreur de parametre, le AM est obligatoire\n");
			flag_err = 2;
		}
	}
	return flag_err;
}

/* ------------------------------
 * lcr_cfes_ec
 * ===========
 * Configuration des equivalences
 * d'eclairage.
 * --------------------------------	*/

static int lcr_cfes_ec(STRING ptCour, INT lgCour, INT * lgSuiv, INT * config, INT *amMod, INT *amCais)
{
	int flag_err = 0;
	printDebug("lcr_cfes_ec\n");
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if (dv1_scmp_and_skip(ptCour, lgCour, "AM=", 0, &ptCour, &lgCour))
	{
		int mod, cais, amOk;
		ptCour = tst_lit_am_bis(ptCour, lgCour, &lgCour, &mod, &cais, &amOk);
		if (amOk)
		{
			*amMod = mod;
			*amCais = cais;
			ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
			printDebug("lcr_cfes_ec, am %d.%d\n", mod, cais);
			if ((1 == lgCour) && ('Z' == *ptCour))
			{
				lgCour--;
				ptCour++;
				/* Reinit des modules selectionnes */
				*config = cfes_supprimer_type(mod, cais, PIP_EQUIV_EC);
			} else if ((1 == lgCour) && ('S' == *ptCour))
			{
				lgCour--;
				ptCour++;
				if (TRUE == cfes_standard_type(mod, cais, PIP_EQUIV_EC))
				{
					*config = TRUE;
				}
			} else if (0 != lgCour)
			{
				char symbole[MAXLCR];
				/* Il faut ici analyser le nom de l'equivalence et ensuite
				 * analyse ce qui suit. */
				ptCour = tst_lit_param_symb(ptCour, lgCour, &lgCour, symbole);
				if (pip_util_test_nom_symbole(symbole))
				{
					if ('=' == *ptCour)
					{
						char param[MAXLCR];
						ptCour = tst_lit_param(ptCour + 1, lgCour - 1, &lgCour, param);
						if (pip_util_test_numerique(mod, cais, param))
						{
							T_pip_equiv equiv;
							/* Dans le cas d'une equivalence symbolique
							 * d'eclairage, il est imperatif que le
							 * symbole soit un numerique. */
							printDebug("lcr_cfes_ec, c'est numerique\n");
							dv1_atoi(param, strlen(param), (INT *) &equiv.valeur);
							/* Controle de la validite du numero d'eclairge */
							if (equiv.valeur < NB_SEUIL_CELLULE)
							{
								equiv.typeEquiv = PIP_EQUIV_EC;
								equiv.typeSymb = PIP_SYMB_NUMERIQUE;
								if (-1 != cfes_ajouter(mod, cais, symbole, equiv))
								{
									*config = TRUE;
								}
							} else
							{
								/* Erreur de valeur de parametre.       */
								printDebug("lcr_cfes_ec numerique non valide %d!\n", equiv.valeur);
								flag_err = 3;
							}

						} else
						{
							/* Erreur de valeur de parametre.       */
							printDebug("lcr_cfes_ec et non numerique !\n");
							flag_err = 3;
						}

					} else
					{
						printDebug("lcr_cfes_ec pas de =!\n");
						flag_err = 2; /* probleme de parametre */
					}
				} else
				{
					printDebug("lcr_cfes_ec symbole incorrect #%s# \n", symbole);
					flag_err = 2; /* probleme de parametre */
				}

			}
		} else
		{
			printDebug("Probleme de lecture de l'AM\n");
			flag_err = 2;
		}

	} else if (dv1_scmp_and_skip(ptCour, lgCour, "AM", 0, &ptCour, &lgCour))
	{
		/* Il faut controler qu'il n'y a plus rien apres */
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		if (0 != lgCour)
		{

			printDebug("Erreur de parametre, rien ne doit suivre le AM\n");
			flag_err = 2;
		}

	} else
	{
		if (0 != lgCour)
		{
			printDebug("Erreur de parametre, le AM est obligatoire\n");
			flag_err = 2;
		}
	}
	return flag_err;
}

static int cfes_trier_equiv(int deb, int fin, int type, int *tabEquiv, int nbEquiv)
{
	int equiCour;
	for (equiCour = deb; equiCour < fin; equiCour++)
	{
		if (type == pip_cf_equiv[equiCour].equiv.typeEquiv)
		{
			int index = 0;
			int trv = FALSE;
			int indCour;
			while ((index < nbEquiv) && (FALSE == trv))
			{
				if (strncmp(pip_cf_equiv[tabEquiv[index]].symbole, pip_cf_equiv[equiCour].symbole, LG_SYMB) >= 0)
				{
					trv = TRUE;
				} else
				{
					index++;
				}
			}
			for (indCour = nbEquiv; indCour > index; indCour--)
			{
				tabEquiv[indCour] = tabEquiv[indCour - 1];
			}
			tabEquiv[index] = equiCour;
			nbEquiv++;
		} /* endif(type==pip_cf_equiv[equiCour].typeEquiv) */
	} /* endfor(equiCour=deb;equiCour<fin;equiCour++) */
	return nbEquiv;
}

/* --------------------------------
 * lcr_cfes_liste_am
 * =================
 * Liste des equivalences topolo-
 * giques pour une am donnees. La
 * fonction prend en compte les *
 * --------------------------------	*/

INT lcr_cfes_liste_am(INT las, INT mode, INT lg_mess, STRING buffer, INT j, T_usr_ztf * pt_mess, INT type, INT * bloc,
		INT amMod, INT amCais)
{
	int indice;
	/* Boucle sur la liste des equivalences topologiques */
	for (indice = 0; indice < pip_nb_topo; indice++)
	{
		T_pip_cf_topo *topo = &pip_cf_topo[indice];
		int mod = topo->topoModule;
		int cais = topo->topoCaisson;
		if ((amMod == NN_AFF) || ((amMod == mod) && (amCais == N_AFF)) || ((amMod == mod) && (amCais == cais)))
		{
			int tabEquiv[NB_EQUIV];
			int nbEquiv = 0;
			int equiCour;
			j += sprintf(&buffer[j], "AM=%d", mod);
			if (-1 != cais)
			{
				j += sprintf(&buffer[j], ".%d", cais);
			}
			j += sprintf(&buffer[j], "\n\r");
			/* Traitement suiva le type demande... */
			if (PIP_EQUIV_AFF == type)
			{
				int indCaisson = pip_cf_topo[indice].indexCaisson;
				/* Cas du module alpha */
				if (pip_cf_caisson[indCaisson].type & TYPE_ALPHA)
				{
					if (N_AFF != cais)
					{
						nbEquiv = cfes_trier_equiv(cfes_index(mod, cais, (STRING) ""), cfes_index(mod, cais + 1,
								(STRING) ""), type, tabEquiv, nbEquiv);
					}
					if (NN_AFF != mod)
					{
						nbEquiv = cfes_trier_equiv(cfes_index(mod, N_AFF, (STRING) ""), cfes_index(mod + 1, 0,
								(STRING) ""), type, tabEquiv, nbEquiv);
					}
					nbEquiv = cfes_trier_equiv(cfes_index(NN_AFF, 0, (STRING) ""), pip_nb_equiv, type, tabEquiv,
							nbEquiv);
					/* Rechercher les equivalences du module */
					/* Recherche des equivalences globlales */
					for (equiCour = 0; equiCour < nbEquiv; equiCour++)
					{
						T_pip_cf_equiv *inf = &pip_cf_equiv[tabEquiv[equiCour]];
						j += dv1_scpy(&buffer[j],inf->symbole,min(strlen(inf->symbole),  LG_SYMB));
						switch (inf->equiv.typeEquiv)
						{
						case PIP_SYMB_LITTERAL:
						{
							char *texte = pip_cf_litteral[inf->equiv.valeur].texte;
							buffer[j++] = '=';
							buffer[j++] = '"';
							j += dv1_scpy(&buffer[j],texte,min(strlen(texte),BUFF_MESS_P));
							buffer[j++] = '"';
						}
							break;
						case PIP_SYMB_NUMERIQUE:
							/* Il y a erreur!!! */
							j += sprintf(&buffer[j], "=%d", inf->equiv.valeur);
							break;
						}
						j += sprintf(&buffer[j], "\n\r");
						/* Balancer eventuellement  */
						tedi_test_bloc(las, mode, buffer, &j, bloc, pt_mess);
					} /* endfor(equiCour=0;equiCour<nbEquiv;equiCour++) */
				}
				else if (pip_cf_caisson[indCaisson].type == TYPE_DELEST)
				{
				int val=0;
					for(val=0;val<=1;val++)
					{
					int index=cfes_chercher_valeur(mod,cais,PIP_SYMB_NUMERIQUE,PIP_EQUIV_AFF,val);
						if(-1!=index)
						{
						T_pip_cf_equiv *inf=&pip_cf_equiv[index];
							j+=dv1_scpy(&buffer[j],inf->symbole,min(strlen(inf->symbole),LG_SYMB));
						}
						j+=dv1_sprintf(&buffer[j],"=%d\n\r",val);
						tedi_test_bloc(las,mode,buffer,&j,bloc,pt_mess);

					}
				}
				/* Cas du module numerique (picto) */
			} else if (PIP_EQUIV_EC == type) /* Equivalences d'affichage. */
			{
				/* Pour l'eclairage, on realise une boucle sur les valeurs
				 * admises pour l'eclarage et on regarde si une equivalence
				 * existe. Dans l'affirmative, on imprime cette equivalence.
				 * Dans le cas contraire, on se contente d'imprimer la valeur
				 * precedee d'un =. */
				int numSeuil;
				for (numSeuil = 0; numSeuil < NB_SEUIL_CELLULE; numSeuil++)
				{
					int index = cfes_chercher_valeur(mod, cais, PIP_SYMB_NUMERIQUE, PIP_EQUIV_EC, numSeuil);
					if (-1 != index)
					{
						T_pip_cf_equiv *inf = &pip_cf_equiv[index];
						j += dv1_scpy(&buffer[j],inf->symbole, min (strlen (inf->symbole), LG_SYMB));
					}
					j += sprintf(&buffer[j], "=%d\n\r", numSeuil);
					tedi_test_bloc(las, mode, buffer, &j, bloc, pt_mess);

				}
			}
		}
	} /* endfor (indice=0;indice<pip_nb_topo;indice++) */
	return j;
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cfes
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 21/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cfes.c
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : Commande de configuration des equivalences symboliques
 /-------------------------------------------------------------------------DOC*/
INT lcr_cfes(INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
	char *ptCour = (char *) &buffer[position + strlen("CFES")];
	int lgCour = lg_mess - position - strlen("CFES");
	int flag_err = 0;
	int config = FALSE;
	int listeAf = TRUE; /* Lister les equivalences d'affichage */
	int listeEc = TRUE; /* Liste les equivalences d'eclairage */
	int amMod = NN_AFF;
	int amCais = N_AFF;
	int standard =0;

	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if (0 < lgCour)
	{
	      if (identLireAutorisationAvecNiveau (las, 0))
	        {

		/* On test le caractere... */
		if (dv1_scmp_and_skip(ptCour, lgCour, "Z", 0, &ptCour, &lgCour))
		{
			/* Interdit.... */
			flag_err=5;
		} else if (dv1_scmp_and_skip(ptCour, lgCour, "S", 0, &ptCour, &lgCour))
		{
			/* Equivalences standard */
			config = lcr_cfes_init();
			cfes_standard_type(NN_AFF, N_AFF, PIP_EQUIV_AFF);
			cfes_standard_type(NN_AFF,N_AFF, PIP_EQUIV_EC);
			standard=1;
		} else if (dv1_scmp_and_skip(ptCour, lgCour, "AF", 0, &ptCour, &lgCour))
		{
			/* Equivalences d'affichage */
			printDebug("lcr_cfes : affichage\n");
			listeEc = FALSE;
			flag_err = lcr_cfes_af(ptCour, lgCour, &lgCour, &config, &amMod, &amCais);
		} else if (dv1_scmp_and_skip(ptCour, lgCour, "EC", 0, &ptCour, &lgCour))
		{
			/* Equivalences d'eclairage */
			printDebug("lcr_cfes : eclairage\n");
			listeAf = FALSE;
			flag_err = lcr_cfes_ec(ptCour, lgCour, &lgCour, &config, &amMod, &amCais);
		} else
		{
			/* Erreur */
			flag_err = 2;
		}
	        }
	      else
	        {
	          x01_cptr.erreur = CPTRD_PROTOCOLE;
	        }
	}
	/* si on a pas detecte d'erreur */
	if ((x01_cptr.erreur == CPTRD_OK) && (0 == flag_err) && ((0 == config)||(1==standard)))
	{
		if ((0 == config)||(1==standard))
		{
			int j = 0;
			/* On liste d'abord les equivalences d'affichage */
			if (TRUE == listeAf)
			{
				j += sprintf(&buffer[j], "CFES AF ");
				j = lcr_cfes_liste_am(las, mode, lg_mess - j, buffer, j, pt_mess, PIP_EQUIV_AFF, bloc, amMod, amCais);
			}
			/* On liste ensuite les equivalences d'eclairage */
			if (TRUE == listeEc)
			{
				j += sprintf(&buffer[j], "CFES EC ");
				j = lcr_cfes_liste_am(las, mode, lg_mess - j, buffer, j, pt_mess, PIP_EQUIV_EC, bloc, amMod, amCais);
			}

//			if (TRUE == flg_fin)
//			{
//				buffer[j++] = '\n';
//				buffer[j++] = '\r';
//			}
			tedi_send_bloc(las, mode, buffer, j, *bloc, (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
		} else
		{
			tedi_send_bloc(las, mode, "", 0, *bloc, (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
		}
	} else
	{
		if (flag_err)
		{
			config = FALSE;
		}
		tedi_ctrl_ext(las, mode, ((x01_cptr.erreur == CPTRD_PROTOCOLE) ? 5 : flag_err));
	}
	return config;
}
