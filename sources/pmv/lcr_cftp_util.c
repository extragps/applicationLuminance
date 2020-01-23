/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cftp                                              /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 29/09/1998                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la con- /
 / figuration  des panneaux                                                    /
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
#include "tac_conf.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "lcr_cftp.h"

#include "configuration.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

/* --------------------------------
 * caisChercher
 * ============
 * Chercher l'index d'un caisson a
 * partir d'un numero de module et
 * d'un numero de caisson
 * --------------------------------	*/

int caisChercher(int numModule, int numCaisson)
{
	int index = -1;
	int indModule;
	indModule = modChercherIndex(numModule);
	if (-1 != indModule)
	{
		index = caisChercherIndex(indModule, numCaisson);
	}
	return index;
}

/* ------------------------------
 * caisChercherIndex
 * =================
 * Recherche d'un numero de caisson
 * dans la liste des caissons d'un
 * module.
 * --------------------------------	*/

int caisChercherIndex(int indexModule, int numCaisson)
{
	int index = -1;
	int indCour = pip_cf_module[indexModule].num_caisson;
	int indMax = indCour + pip_cf_module[indexModule].nb_caisson;
	for (; (indCour < indMax) && (-1 == index); indCour++)
	{
		if (numCaisson == pip_cf_caisson[indCour].caisson)
		{
			index = indCour;
		}
	}
	return index;
}

/* ------------------------------
 * caisGetIndex
 * ============
 * Allocation d'un nouveau caisson
 * pour un module.
 * Attention, l'allocation reclame
 * le decalage de tous les caissons
 * suivants.
 * ----------------------------	*/

int caisGetIndex(int numModule, int numCaisson)
{
	int index = -1;
	int indCour = pip_cf_module[numModule].num_caisson;
	int indMax = indCour + pip_cf_module[numModule].nb_caisson;
	if (pip_nb_caisson < NB_CAISSON)
	{
		int m;
		for (index = indCour; (pip_cf_caisson[index].caisson < numCaisson) && (index < indMax); index++)
			;
		/* Decalage des caissons suivant */
		for (m = pip_nb_caisson; m > index; m--)
		{
		int numCour;
			pip_cf_caisson[m] = pip_cf_caisson[m - 1];
			/* Changer ici aussi tous les index de caisson des caracteres */
			for(numCour=0;numCour<pip_cf_caisson[m].nb_car;numCour++)
			{
				configSetCaissonAfficheur(pip_cf_caisson[m].num_car+numCour,m);
			}

		}
		/* Increment du numero de premier caisson
		 * des modules suivants */
		for (m = numModule + 1; m < pip_nb_module; m++)
		{
			pip_cf_module[m].num_caisson++;
		}
		/* Positionnement du numero de caisson */
		pip_cf_caisson[index].caisson = numCaisson;
		/* Increment du nombre de caissons du module */
		pip_cf_module[numModule].nb_caisson++;
		/* Increment du nombre de caissons utilises */
		pip_nb_caisson++;
	}
	return index;
}

/* ---------------------------------
 * modChercherIndex
 * ================
 * Recherche de l'index d'un numero
 * de module dans une liste de
 * modules.
 * --------------------------------	*/

int modChercherIndex(int numModule)
{
	int index = -1;
	int indCour = 0;
	for (indCour = 0; (indCour < pip_nb_module) && (-1 == index); indCour++)
	{
		if (numModule == pip_cf_module[indCour].id_module)
		{
			index = indCour;
		}
	}
	return index;
}

/* ---------------------------------
 * modGetIndex
 * ===========
 * Allocation d'un nouveau module.
 * --------------------------------	*/

int modGetIndex(int numModule)
{
	int index = -1;
	if (pip_nb_module < NB_MODULE)
	{
		int index2;
		for (index = 0; (index < pip_nb_module) && (pip_cf_module[index].id_module < numModule); index++)
			;
		for (index2 = pip_nb_module; index2 > index; index2--)
		{
			pip_cf_module[index2] = pip_cf_module[index2 - 1];
		}
		pip_cf_module[index].id_module = numModule;
		pip_cf_module[index].nb_caisson = 0;
		if (index == pip_nb_module)
		{
			pip_cf_module[index].num_caisson = pip_nb_caisson;
		} else
		{
			pip_cf_module[index].num_caisson = pip_cf_module[index + 1].num_caisson;
		}
		pip_nb_module++;
	}
	return index;
}

/* ---------------------------------
 * cmd_init_cftp
 * =============
 * Initialisation de la structure
 * de commande.
 * --------------------------------	*/
void  lcr_cftp_cmd_init(T_cmd_cftp * cmd_cftp)
{
	int i;
	cmd_cftp->flag = FALSE;
	cmd_cftp->id_module = NN_AFF;
	cmd_cftp->caisson = N_AFF;
	cmd_cftp->info[0] = 0;
	cmd_cftp->type = N_AFF;
	cmd_cftp->forceLum = 0;
	cmd_cftp->nb_afficheur = 0;
	cmd_cftp->nb_es = 0;
	cmd_cftp->nb_ad = 0;
}


/*---------------------------------
 * conf_alpha_cftp
 * =================
 * Configuration d'un caisson de
 * type alpha_d
 * TODO: Arranger la chose pour faire en sorte
 * que ca fonctionne sur luminance.
 * --------------------------------	*/

static int conf_alpha_cftp(T_cmd_cftp *cmd_cftp, int numCaisson)
{
	int erreur = 0;
	T_pip_cf_caisson *caisson = &pip_cf_caisson[numCaisson];

	/* les caracteres doivent etre multiple de 2 */
	if ((cmd_cftp->nb_es % 2 == 0) && cmd_cftp->nb_es)
	{
		int l;
		int nb_car=0;
		int k = caisson->num_car;
		/* pour tous les caracteres recus de la commande */
		for (l = 0; (l < cmd_cftp->nb_es) && (!erreur); l += 2)
		{
			/* Ajout d'un nouvel afficheur. */
			/* Positionnement de l'adresse et de la sortie... */
			if(configGetNbAfficheurs()<AFFICHEUR_NB_MAX)
			{
				afficheursInitUnitaire(configGetNbAfficheurs(),cmd_cftp->es[l],numCaisson);
			configSetNbAfficheurs(configGetNbAfficheurs()+1);
			configSetAdresseAfficheur(k,cmd_cftp->es[l]);
			configSetSortieAfficheur(k,cmd_cftp->es[l+1]);
			configSetCaissonAfficheur(k,numCaisson);
			}
			else

				{
					erreur = 12;
				}

				nb_car++;
				k++;
			}
			if(0==erreur)
			{
				caisson->nb_car=nb_car;
			}

	} else
	{
		erreur = 15;
	}
	return erreur;
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : cpd_exec_cftp                                          /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/08/1993                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cftp.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement de la configuration                         /
 /-------------------------------------------------------------------------DOC*/
VOID lcr_cftp_cmd_exec(struct cmd_cftp * cmd_cftp)
{
	INT i, j;
	BYTE trv;
	BYTE erreur;

	erreur = FALSE;

	if (cmd_cftp->id_module != NN_AFF)
	{
		/* on recherche le panneau dans les modules alphanumeriques */
		i = modChercherIndex(cmd_cftp->id_module);
		if (i < 0)
		{
			trv = TRUE;
			i = modGetIndex(cmd_cftp->id_module);
			if (-1 != i)
			{
				/* si le module est un module e feux tricolore */
				switch (cmd_cftp->type)
				{
				case TYPE_ALPHA:
					/* le type de panneau */
					pip_cf_module[i].s_type = TYPE_ALPHA;
					break;
				case TYPE_DELEST:
					pip_cf_module[i].s_type = TYPE_ALPHA;
					break;

				default:
					erreur = 6;
					break;
				} /* endswitch (cmd_cftp->type) */
			} else
			{
				erreur = 6;
			}
		}

		if (!erreur)
		{
			/* si le numero de module n'est pas affecte */
			if ((pip_cf_module[i].id_module != N_AFF) && (cmd_cftp->caisson != N_AFF))
			{
				/* on recherche les caissons */
				j = caisChercherIndex(i, cmd_cftp->caisson);
				if (j < 0)
				{
					j = caisGetIndex(i, cmd_cftp->caisson);
				}
				if (j >= 0)
				{
					T_pip_cf_caisson *caisson = &pip_cf_caisson[j];
					/* on cree un nouveau caisson */
					printf("Index caisson %d topo %d et 2 %d\n", j, caisson->indexTopo, pip_cf_caisson[1].indexTopo);
					caisson->caisson = cmd_cftp->caisson;
					caisson->type = cmd_cftp->type;
					caisson->id_module = pip_cf_module[i].id_module;
					if (0 != strlen(cmd_cftp->info))
					{
						strncpy(caisson->info, cmd_cftp->info, MAX_USR_LCPI);
						caisson->info[MAX_USR_LCPI] = 0;
					}
					/* le numero de carte lon */
					{
						/* si la luminosite est forcee sur ce caisson */
						caisson->num_car = configGetNbAfficheurs();
						caisson->nb_car = 0;
						caisson->type = cmd_cftp->type;
						/* Initialisation suivant le type de caisson */
						switch (caisson->type)
						{
						case TYPE_ALPHA:
							erreur = conf_alpha_cftp(cmd_cftp, j);
							break;
						case TYPE_DELEST:
							break;
						default:
							erreur = 31; /* Erreur de type de caisson                     */
							break;

						} /* endswitch (pip_cf_caisson[j].type) */
					}
					printf("INDEX caisson %d topo %d et le second %d\n", j, caisson->indexTopo,
							pip_cf_caisson[1].indexTopo);
				} else
				{
					erreur = 10; /* Erreur d'allocation de caisson       */
				}

			} else
			{
				erreur = 10; /* Num module ou caisson non affecte    */
			}
		}
	} else
	{
		erreur = 4;
	} /* endif (cmd_cftp->id_module == NN_AFF)        */

	/* si on a detecte une erreur */
	if (erreur)
	{
		printf("Erreur de configuratoin cftp %d\n", erreur);
		x01_cptr.erreur = CPTRD_PARAM;
		x01_status3.erreur_config = erreur;
	}
	/* on initialise la structure de demande de configuration */
	lcr_cftp_cmd_init(cmd_cftp);

}
/* *********************************
 * FIN DE lcr_cftp_util.c
 * ********************************* */
