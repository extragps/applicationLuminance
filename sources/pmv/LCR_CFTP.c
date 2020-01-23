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
#include "x01_vcth.h"
#include "lcr_cftp.h"
#include "identLib.h"
#include "pip_util.h"
#include "configuration.h"

/* **********************************
 * DEFINITION DES MACROS
 * ********************************    */

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

static INT _lcr_cftp_cmd(STRING, INT);

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cftp_classe_caisson                                   /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 23/03/1999                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_p.c                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : construction de l'arborescence ses rampes                            /
 /-------------------------------------------------------------------------DOC*/

VOID lcr_cftp_arborescence(VOID)
{

	BYTE numPortique, numRampe, numPanneau, numCaisson;
	BYTE fini;
	BYTE i;

	if (pip_nb_caisson)
	{
		for (i = 0; i < pip_nb_caisson; i++)
		{
			pip_liste_caisson[i] = i;
		}

		/* on trie les caissons par ordre croissant */
		fini = FALSE;
		while (!fini)
		{
			fini = TRUE;
			for (i = 0; i < pip_nb_caisson - 1; i++)
			{
				if (pip_cf_caisson[pip_liste_caisson[i]].id_module > pip_cf_caisson[pip_liste_caisson[i + 1]].id_module)
				{
					BYTE mem;
					mem = pip_liste_caisson[i];
					pip_liste_caisson[i] = pip_liste_caisson[i + 1];
					pip_liste_caisson[i + 1] = mem;
					fini = FALSE;
				}
			}
		}

		numPortique = N_AFF;
		numRampe = N_AFF;
		numPanneau = N_AFF;
		numCaisson = N_AFF;

		pip_nb_panneau = 0;
		pip_nb_caisson_liste = 0;

		for (i = 0; i < NB_CAISSON; i++)
		{
			pip_panneau[i].numCaisson = 0;
			pip_panneau[i].nbCaisson = 0;
		}

		/* on cree l'arborescence portique,rampe,panneau et caisson */
		for (i = 0; i < pip_nb_caisson; i++)
		{
			/* si le numero de portique est different */
			if (numPanneau != (BYTE) (pip_cf_caisson[pip_liste_caisson[i]].id_module % 100))
			{
				/* on passe au panneau suivant */
				/* on memorise le numero de panneau */
				numPanneau = (BYTE) (pip_cf_caisson[pip_liste_caisson[i]].id_module % 100);

				/* on cree le panneau */
				pip_panneau[pip_nb_panneau].numCaisson = pip_nb_caisson_liste;
				pip_panneau[pip_nb_panneau].nbCaisson++;
				pip_panneau[pip_nb_panneau].idPanneau = numPanneau;
				pip_nb_panneau++;
				pip_nb_caisson_liste++;
			} else
			{
				/* on rajoute le caisson au panneau forcement */
				pip_panneau[pip_nb_panneau - 1].nbCaisson++;
				pip_nb_caisson_liste++;
			}
		}
	}
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : cmd_cftp                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/08/1993                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pmv0.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement des parametres de la commande TST CFTP      /
 /-------------------------------------------------------------------------DOC*/
static INT _lcr_cftp_cmd(STRING buffer, INT nbcar)
{
	INT config = FALSE;
	INT val;
	BYTE erreur;
	STRING ptr_deb;
	STRING ptr_fin;
	/* structure de reception de la commande de configuration */
	T_cmd_cftp cmd_cftp;

	lcr_cftp_cmd_init(&cmd_cftp);
	/* on initialise la structure de demande de configuration */

	ptr_deb = buffer;
	ptr_fin = buffer;

	/* le dernier caractere est nul */
	buffer[nbcar] = 0;

	while (((INT) (ptr_fin - buffer) <= nbcar) && (x01_cptr.erreur == CPTRD_OK))
	{
		config = TRUE;
		switch (*ptr_fin)
		{

		case 0:
		case ' ':
			/* fin du parametre */
			*ptr_fin = 0;
			/* si deux separateur ne se suivent pas */
			if (ptr_deb != ptr_fin)
			{
				/* on recherche le parametre */
				if (0 == strncmp(ptr_deb, LCR_CFTP_MODULE,strlen(LCR_CFTP_MODULE)))
				{
					/* si on a deja une commande, on a un nouveau module et il faut donc
					 * enregistrer les elements du module precedent. */

					if (cmd_cftp.flag)
					{
						/* on traite la commande predente*/
						lcr_cftp_cmd_exec(&cmd_cftp);
					}
					ptr_deb += strlen(LCR_CFTP_MODULE);

					/* On part en positionnant par defaut une erreur. */

					erreur = TRUE;

					/* Recherche du numero de module.... */
					if (dv1_str_atoi(&ptr_deb, &val))
					{
						/* le numero de module   */
						cmd_cftp.id_module = val;
						if (ptr_deb < ptr_fin)
						{
							if (dv1_str_atoi(&ptr_deb, &val))
							{
								/* le numero de module   */
								cmd_cftp.caisson = (BYTE) val;

								/* si c'est bien le dernier caractere */
								if (ptr_deb >= ptr_fin)
								{
									erreur = FALSE;
									/* on recoit une nouvelle commande */
									cmd_cftp.flag = TRUE;
								}
							}
						}
					}

					/* si il y a une erreur */
					if (erreur)
					{
						printDebug("Erreur dans le traitement du AM\n");
						x01_cptr.erreur = CPTRD_SYNTAXE;
					}
				} else if (0==strncmp(ptr_deb, LCR_CFTP_TYPE,strlen(LCR_CFTP_TYPE)))
				{
					ptr_deb += strlen(LCR_CFTP_TYPE);
					/* Selection du type de module. Pour l'heure ne
					 * sont definis que :
					 * - les modules alphanumeriques  : A
					 * - le module de delestage. : D
					 */
					switch(*ptr_deb++)
					{
					case 'A':
						cmd_cftp.type=TYPE_ALPHA;
						break;
					case 'D':
						cmd_cftp.type=TYPE_DELEST;
						break;
					default:
						/* Erreur de parametre. */
						x01_cptr.erreur = CPTRD_VALEUR_PARAM;
						break;
					}


				}
	              else if (0==strncmp (ptr_deb, LCR_CFTP_INFO, strlen(LCR_CFTP_INFO)))
				  {
				  int ok;
				  int lgCour=ptr_fin-ptr_deb;
	                  ptr_deb += strlen(LCR_CFTP_INFO);
					  lgCour-=strlen(LCR_CFTP_INFO);
					  ptr_deb = (STRING) tst_lit_param_cftp(ptr_deb,
									  lgCour, &lgCour, cmd_cftp.info, &ok);
					  if(FALSE==ok)
					  {
	                      x01_cptr.erreur = CPTRD_SYNTAXE;
					  }

				  }
	            else if (0==strncmp(ptr_deb, LCR_CFTP_CARTE,strlen(LCR_CFTP_CARTE)))
				{
					ptr_deb += strlen(LCR_CFTP_CARTE);
					/* Numeros des cartes associes au module, avec eventuellement un indicateur pour
					 * preciser le numero de la sortie a activer en cas de defaut de la carte.
					 */
					/* Le numero de carte est un entier. Il peut etre suivi du numero de sortie a
					 * activer en cas de defaut.
					 */
					if(cmd_cftp.type==TYPE_ALPHA)
					{
					bool erreur_b=false;
						/* Examin des numeros de cartes qu'il faut prevoir dans le module
						 * alphanumerique.
						 */
						 cmd_cftp.nb_es=0;
						 while((ptr_deb<ptr_fin)&&(false==erreur_b))
						 {
						 	if(dv1_str_atoi(&ptr_deb,&val))
						 	{
						 		cmd_cftp.es[cmd_cftp.nb_es++]=(BYTE)val;
						 	}
						 	else
						 	{
						 		erreur_b=true;
						 	}
						 }
						 if(erreur_b==true)
						 {
						 	x01_cptr.erreur=CPTRD_SYNTAXE;
						 }
					}
					else
					{
						/* Cas d'erreur, on doit entre en presence d'un module de type alpha. */
						x01_cptr.erreur = CPTRD_PARAM;
					}
				} else if (0==strncmp(ptr_deb, LCR_CFTP_LUMINOSITE,strlen(LCR_CFTP_LUMINOSITE)))
				{
					ptr_deb += strlen(LCR_CFTP_LUMINOSITE);
					/* Adresse des cartes de luminosite associees au module. */
					/* Plusieurs cartes de luminosite, separes par le caractere / peuvent
					 * etre associees a un module.
					 */
					if(cmd_cftp.type==TYPE_ALPHA)
					{
						/* Examen de la liste des cartes de luminosite associees au module.
						 * Par defaut, si il n'y a pas de carte, la luminosite sera forcee en
						 * nuit.
						 */
						 /* Faire comme avant une boucle pour tout traiter.... */

					}
					else
					{
						/* Cas d'erreur, on doit entre en presence d'un module de type alpha. */
						x01_cptr.erreur = CPTRD_PARAM;
					}
				} else if (0==strncmp(ptr_deb, LCR_CFTP_SABORDAGE,strlen(LCR_CFTP_SABORDAGE)))
				{
					ptr_deb += strlen(LCR_CFTP_SABORDAGE);
					/* Pour associer le module de sabordage au module. C'est dans le cas
					 * eventuel ou l'on aurait plusieurs alimentations.
					 */
					if(cmd_cftp.type==TYPE_ALPHA)
					{
						/* En cas d'erreur sur le module, indication de l'adresse du module de
						 * sabordage.
						 */

					}
					else
					{
						/* Cas d'erreur, on doit entre en presence d'un module de type alpha. */
						x01_cptr.erreur = CPTRD_PARAM;
					}
				} else if (0==strncmp(ptr_deb, LCR_CFTP_VALEUR,strlen(LCR_CFTP_VALEUR)))
				{
					ptr_deb += strlen(LCR_CFTP_VALEUR);
					/* Il s'agit des valeurs possibles pour le module de delestage. C'est
					 * pour faire comme avant avec le AF. */
					if(cmd_cftp.type==TYPE_DELEST)
					{
						/* Liste des commandes du module en fonction du numero de decor. */
					}
					else
					{
						/* Cas d'erreur, on doit entre en presence d'un module de type alpha. */
						x01_cptr.erreur = CPTRD_PARAM;
					}
				} else
				{
					printDebug("Autre erreur\n");
					x01_cptr.erreur = CPTRD_SYNTAXE;
				}
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
			if ('"' == *ptr_fin)
			{
				ptr_fin++;
				while (('"' != *ptr_fin) && (0 != *ptr_fin))
				{
					ptr_fin++;
				}
				if ('"' == *ptr_fin)
				{
					ptr_fin++;
				} else
				{
					x01_cptr.erreur = CPTRD_SYNTAXE;
				}
			} else
			{
				ptr_fin++;
			}
			break;
		}
	}
	/* si on a deja une commande */

	if ((x01_cptr.erreur == CPTRD_OK) && (cmd_cftp.flag))
	{
		/* on traite la commande */
		lcr_cftp_cmd_exec(&cmd_cftp);
		printDebug("Val1 indTopo pour caisson 2 %d\n", pip_cf_caisson[1].indexTopo);
	} else
	{
		config = FALSE;
	}
	printDebug("Val indTopo pour caisson 2 %d\n", pip_cf_caisson[1].indexTopo);
	return config;
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cftp_raz                                          /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 03/16/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cftp                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : reinitialisation de la configuration d'un panneau      /
 /-------------------------------------------------------------------------DOC*/

void lcr_cftp_raz(void)
{
	UINT i;
	pip_nb_module = 0; /* le nombre de module alphanumeriques configures */
	pip_nb_caisson = 0; /* le nombre de caisson alphanumeriques configures */
	pip_nb_topo = 0; /* le nombre d'equivalences topologiques configurees  */
	pip_nb_car_prepro = 0; /* le nombre de textes preprogrammes */
	configSetNbAfficheurs(0);

	/* on raz l'erreur */
	/* XG le 21/10/05  :  x01_status3.erreur_config = 0; */
	/* par defaut */
	for (i = 0; i < NB_MODULE; i++)
	{
		T_pip_cf_module *cf_module = &pip_cf_module[i];
		cf_module->num_cellule = 0;
		cf_module->num_sabordage = 0;
		cf_module->nb_caisson = 0;
		cf_module->num_caisson = 0;
	}

	for (i = 0; i < NB_CAISSON; i++)
	{
		T_pip_cf_caisson *cf_caisson = &pip_cf_caisson[i];
		cf_caisson->indexTopo = -1;
		cf_caisson->type = N_AFF;
		cf_caisson->caisson = 0;
		cf_caisson->nb_car = 0;
		cf_caisson->num_car = 0;
		/* forcage de la luminosite */
		cf_caisson->forceLum = '0';
		cf_caisson->info[0] = 0;

	}
	for (i = 0; i < NB_CAISSON; i++)
	{
		T_pip_cf_topo *topo = &pip_cf_topo[i];
		topo->indexCaisson = -1;
		topo->topoCaisson = -1;
		topo->topoModule = -1;
	}
//	pip_cf_caisson[0].id_module=9;
//	pip_cf_caisson[0].caisson=0;
//	pip_cf_caisson[0].type=TYPE_DELEST;
//	pip_cf_module[0].id_module=9;
//	pip_cf_module[0].nb_caisson=1;
//	pip_cf_module[0].num_caisson=0;
//	pip_nb_caisson=1;
//	pip_nb_module=1;
//	strcpy(pip_cf_caisson[0].info,"Delestage");

}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cftp_init                                          /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 03/16/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cftp                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : reinitialisation de la configuration d'un panneau      /
 /-------------------------------------------------------------------------DOC*/

void lcr_cftp_init(bool raz)
{
	lcr_cftp_raz();
	if(false==raz)
	{
		UINT i;
		int nbAfficheursParLigne=18;
	pip_nb_module=2;
	pip_cf_module[0].id_module=1;
	pip_cf_module[0].nb_caisson=2;
	pip_cf_module[0].num_caisson=0;
	pip_cf_module[1].id_module=9;
	pip_cf_module[1].nb_caisson=1;
	pip_cf_module[1].num_caisson=2;
	pip_nb_caisson=3;
	pip_cf_caisson[0].id_module=1;
	pip_cf_caisson[0].caisson=1;
	pip_cf_caisson[0].num_car=0;
	pip_cf_caisson[0].nb_car=nbAfficheursParLigne;
	pip_cf_caisson[0].clig.type=PIP_AUCUN;
	pip_cf_caisson[0].alt.type=PIP_AUCUN;
	pip_cf_caisson[0].validite=0;
	pip_cf_caisson[0].type=TYPE_ALPHA;
	strcpy(pip_cf_caisson[0].info,"18 caracteres luminance");

	pip_cf_caisson[1].id_module=1;
	pip_cf_caisson[1].caisson=2;
	pip_cf_caisson[1].num_car=pip_cf_caisson[0].nb_car;
	pip_cf_caisson[1].nb_car=nbAfficheursParLigne ;
	pip_cf_caisson[1].clig.type=PIP_AUCUN;
	pip_cf_caisson[1].alt.type=PIP_AUCUN;
	pip_cf_caisson[1].validite=0;
	pip_cf_caisson[1].type=TYPE_ALPHA;
	strcpy(pip_cf_caisson[1].info,"18 caracteres luminance");

	pip_cf_caisson[2].id_module=9;
	pip_cf_caisson[2].caisson=0;
	pip_cf_caisson[2].type=TYPE_DELEST;
	strcpy(pip_cf_caisson[2].info,"Delestage");

	configSetNbAfficheurs(0);
	for(i=0;i<2*nbAfficheursParLigne;i++)
	{
		afficheursInitUnitaire(configGetNbAfficheurs(),i+1,i/nbAfficheursParLigne);
		configSetNbAfficheurs(configGetNbAfficheurs()+1);
		configSetAdresseAfficheur(i,i+1);
		configSetCaissonAfficheur(i,i/nbAfficheursParLigne);
		configSetSortieAfficheur(i,(i%18)/3);
	}
	}


	/* Pour l'instant, deux caissons alpha et un caisson sabordage */



}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cftp                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 03/16/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cftp.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande CFTP                           /
 /-------------------------------------------------------------------------DOC*/
INT lcr_cftp(INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
	INT i, j, k, l;
	INT fini;
	BYTE flag = FALSE;
	INT config = FALSE;

	x01_cptr.erreur = CPTRD_OK;

	/* on se place apres le nom de la commande */
	i = position + 4;

	/* si on est pas en fin de message, on continue l'analyse */
	if (i < lg_mess)
	{
		x01_cptr.erreur = CPTRD_PROTOCOLE;
	}

	/* si on a pas trouve d'erreur */
	if (x01_cptr.erreur == CPTRD_OK)
	{
		printDebug("Val  trac indTopo pour caisson 2 %d\n", pip_cf_caisson[1].indexTopo);
		/* on renvoie la configuration */
		j = 0;
		if (!flag)
		{
			bool premier_b=true;
			/* on initialise le numero de bloc de reponse */
			fini = FALSE;

			/* tous les panneaux */
			i = 0;
			while ((!fini) && (i < pip_nb_module))
			{
				T_pip_cf_module *module = &pip_cf_module[i];
				k = module->num_caisson;
				while (k < module->num_caisson + module->nb_caisson)
				{
					j += sprintf(&buffer[j], (STRING) "%sCFTP %s%d.%d",(premier_b?"":"\n\r"), LCR_CFTP_MODULE, (INT) module->id_module,
							(INT) pip_cf_caisson[k].caisson);
					premier_b=false;
					switch (pip_cf_caisson[k].type)
					{
					case TYPE_ALPHA:
						/* le type de panneau */
						j += sprintf(&buffer[j], " PI=MUSA1 PS=1.%d.%d.1 BUS=COM485/0", 	(INT) pip_cf_caisson[k].caisson-1, (INT) pip_cf_caisson[k].nb_car);
						break;

					case TYPE_DELEST:
						/* le type de panneau */
						j += sprintf(&buffer[j], " PI=DEL%s","");
						break;
					}

					switch (pip_cf_caisson[k].type)
					{

					case TYPE_ALPHA:
						/* le type de panneau */
						/* pour les caracteres constituants le caisson */
						j += sprintf(&buffer[j], " %s", LCR_CFTP_CARTE);
						l = pip_cf_caisson[k].num_car;
						if (0 != pip_cf_caisson[k].nb_car)
						{
							while (l < pip_cf_caisson[k].num_car + pip_cf_caisson[k].nb_car)
							{
								if (j >= 150)
								{
									tedi_send_bloc(las, mode, buffer, j, *bloc, FALSE, pt_mess);
									*bloc = ((*bloc) + 1) % 10;
									j = 0;

									if (vct_IT[las])
										fini = TRUE;
								}

								j += sprintf(&buffer[j], "%ld",configGetAdresseAfficheur(l) );
								if (-1 != configGetSortieAfficheur(l))
								{
									j += sprintf(&buffer[j], ".%ld", configGetSortieAfficheur(l));
								}
								j += sprintf(&buffer[j], "/");
								/* le caractere suivant */
								l++;
							}
							/* On supprime le dernier caractere / */
							j--;
						}
						break;
					}

					/* le caisson suivant */
					k++;
				}
				/* le module suivant */
				i++;
			}
			if ((j >= 0) && (!fini))
			{
				if (TRUE == flg_fin)
				{
					buffer[j++] = '\n';
					buffer[j++] = '\r';
				}
				tedi_send_bloc(las, mode, buffer, j, *bloc, (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
				*bloc = ((*bloc) + 1) % 10;
				j = 0;

				if (vct_IT[las])
					fini = TRUE;
			}
		} else
		{
			j = sprintf(buffer, "");
			if (TRUE == flg_fin)
			{
				buffer[j++] = '\n';
				buffer[j++] = '\r';
			}

			tedi_send_bloc(las, mode, buffer, j, *bloc, (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
		}
		printDebug("Val fin indTopo pour caisson 2 %d\n", pip_cf_caisson[1].indexTopo);
	} else
	{
		config = FALSE;
		tedi_erreur(las, mode);
	}
	return config;
}

INT lcr_cftp_ori(INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
	INT i, j, k, l;
	INT fini;
	BYTE flag = FALSE;
	INT config = FALSE;

	x01_cptr.erreur = CPTRD_OK;

	/* on se place apres le nom de la commande */
	i = position + 5;

	/* si on est pas en fin de message, on continue l'analyse */
	if (i < lg_mess)
	{
		if (i < lg_mess)
		{
			char *ptCour = (char *) &buffer[i];
			int lgCour = lg_mess - i;
			ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
			/* on teste si l'operateur est habilite */
			if (identLireAutorisationAvecNiveau(las, 0))
			{
				flag = TRUE;
				if (dv1_scmp(ptCour, "Z",0))
				{
					/* on raz la config */
					config = TRUE;
					pip_init(true);
				} else
				if (dv1_scmp(ptCour, "S",0))
				{
					/* on raz la config */
					config = TRUE;
					pip_init(false);
				} else
				{
					/* on traite les parametres  */
					config = _lcr_cftp_cmd(ptCour, lgCour);
				}
				/* si il y a une erreur, relecture de la config */
				if (x01_cptr.erreur != CPTRD_OK)
				{
					printDebug("Erreur de configuration\n");
					tac_conf_cfg_lec_conf();
				} else
				{
					if (pip_nb_caisson)
						lcr_cftp_arborescence();
				}

				/* on signale une reconfiguration */
				x01_cptr.config = TEMPO_CONFIG;
			} else
			{
				printDebug("Probleme de protocole\n");
				x01_cptr.erreur = CPTRD_PROTOCOLE;
			}
		}
	}

	/* si on a pas trouve d'erreur */
	if (x01_cptr.erreur == CPTRD_OK)
	{
		printDebug("Val  trac indTopo pour caisson 2 %d\n", pip_cf_caisson[1].indexTopo);
		/* on renvoie la configuration */
		j = 0;
		if (!flag)
		{
			/* on initialise le numero de bloc de reponse */
			fini = FALSE;

			if (position == 0)
				j += sprintf(&buffer[j], "CFTP2");
			else
				j += sprintf(&buffer[j], "TST CFTP");

			/* tous les panneaux */
			i = 0;
			while ((!fini) && (i < pip_nb_module))
			{
				T_pip_cf_module *module = &pip_cf_module[i];
				k = module->num_caisson;
				while (k < module->num_caisson + module->nb_caisson)
				{
					printDebug("Val mod indTopo pour caisson 2 %d\n", pip_cf_caisson[1].indexTopo);
					j += sprintf(&buffer[j], (STRING) "\n\r\n\r%s%d.%d", LCR_CFTP_MODULE, (INT) module->id_module,
							(INT) pip_cf_caisson[k].caisson);

					switch (pip_cf_caisson[k].type)
					{
					case TYPE_ALPHA:
						/* le type de panneau */
						j += sprintf(&buffer[j], " %s%s", LCR_CFTP_TYPE, "A");
						break;

					case TYPE_DELEST:
						/* le type de panneau */
						j += sprintf(&buffer[j], " %s%s", LCR_CFTP_TYPE, "D");
						break;
					}

					switch (pip_cf_caisson[k].type)
					{

					case TYPE_ALPHA:
						/* le type de panneau */
						/* pour les caracteres constituants le caisson */
						j += sprintf(&buffer[j], "\n\r%s", LCR_CFTP_CARTE);
						l = pip_cf_caisson[k].num_car;
						if (0 != pip_cf_caisson[k].nb_car)
						{
							while (l < pip_cf_caisson[k].num_car + pip_cf_caisson[k].nb_car)
							{
								if (j >= 150)
								{
									tedi_send_bloc(las, mode, buffer, j, *bloc, FALSE, pt_mess);
									*bloc = ((*bloc) + 1) % 10;
									j = 0;

									if (vct_IT[las])
										fini = TRUE;
								}

								j += sprintf(&buffer[j], "%ld",configGetAdresseAfficheur(l) );
								if (-1 != configGetSortieAfficheur(l))
								{
									j += sprintf(&buffer[j], ".%ld", configGetSortieAfficheur(l));
								}
								j += sprintf(&buffer[j], "/");
								/* le caractere suivant */
								l++;
							}
							/* On supprime le dernier caractere / */
							j--;
						}
						break;
					}

					/* le caisson suivant */
					k++;
				}
				/* le module suivant */
				i++;
			}
			if ((j >= 0) && (!fini))
			{
				if (TRUE == flg_fin)
				{
					buffer[j++] = '\n';
					buffer[j++] = '\r';
				}
				tedi_send_bloc(las, mode, buffer, j, *bloc, (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
				*bloc = ((*bloc) + 1) % 10;
				j = 0;

				if (vct_IT[las])
					fini = TRUE;
			}
		} else
		{
			j = sprintf(buffer, "");
			if (TRUE == flg_fin)
			{
				buffer[j++] = '\n';
				buffer[j++] = '\r';
			}

			tedi_send_bloc(las, mode, buffer, j, *bloc, (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
		}
		printDebug("Val fin indTopo pour caisson 2 %d\n", pip_cf_caisson[1].indexTopo);
	} else
	{
		config = FALSE;
		tedi_erreur(las, mode);
	}
	return config;
}
