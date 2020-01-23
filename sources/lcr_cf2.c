/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cf2                                              /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 05/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la con- /
 / figuration des parametres de fonctionnemnt de la station.                   /
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
 * Modifications :
 * 02c,23Jul01,xag	Pour initialisation correcte des valeurs de mesures et
 * 					ajout d'une methode pour diviser par 2 les mesures si
 * 					les valeurs sont trop grandes (20000 mesures).
 * 02b,01Mar01,xag	La memoire libre est maintenant exprimee en KO. La variable
 * 					SZ_MEM est devenue bdSzMem
 * 02a,28Nov00,xag  Ajout des sequencements 10 secondes, 20 secondes et 1
 * 					minute. Suppression du mot clef "m=" pour le sequencement
 * 					variable.
 */

#include <string.h>
#include "standard.h"
#include "mon_inc.h"
#include "define.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_cf2h.h"
#include "lcr_cf3h.h"
#include "lcr_idfh.h"
#include "ted_prot.h"
#include "identLib.h"
#include "mon_debug.h"
#include "effLib.h"

#include "sramBdd.h"

/* *********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define CF2_MAX_CPT_MES	20000

//extern Sram *sram_pt;

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/
LOCAL UINT8 cmd_cff(STRING, INT, LONG *,  int *);

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : cmd_cff                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/12/1999                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cf2.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : Traitemen
 /-------------------------------------------------------------------------DOC*/

/* --------------------------------
 * cmd_cff
 * =======
 * Traitement des parameters de la
 * fonction CFF (taille des fichiers).
 * La fonction a ete modifiee par
 * X.GAILLARD pour prendre en
 * compte les fichiers de traces et
 * distinguer la modification de
 * la taille d'un fichier de mesure
 * de celle d'un fichier de trace.
 * Valeur de retour :
 * - 0 si pas d'erreur, le numero
 *   d'erreur qui va bien sinon.
 * --------------------------------	*/
LOCAL UINT8 cmd_cff(STRING buffer, INT nbcar, LONG val_fic[MAXFIC], int *ficTraChanged)
{
	UINT8 erreur = 0;
	STRING ptr_deb;
	STRING ptr_fin;
	LONG val_long;
	int fic;

	/* le dernier caractere est nul */
	buffer[nbcar] = 0;

	ptr_deb = buffer;
	ptr_fin = ptr_deb;
	*ficTraChanged = 0;

	while (((INT) (ptr_fin - buffer) <= nbcar) && (0 == erreur))
	{
		switch (*ptr_fin)
		{
		case 0:
		case ' ':
			*ptr_fin = 0;
			fic = -1;
			/* Configuration standard pour les
			 * tailles des fichiers.                        */
			if (*ptr_deb == 'S')
			{
				int indCour;
				printDebug("cmd_cff: configuration standard\n");
				for (indCour = 0; indCour < MAXFIC; indCour++)
				{
					if (val_fic[indCour] != ficInfo[indCour].standard)
					{
						val_fic[indCour] = ficInfo[indCour].standard;
							*ficTraChanged = 1;
					}
				}
			} else if (*ptr_deb == 'Z') /* Configuration minimale                       */
			{
				int indCour;
				printDebug("cmd_cff: configuration minimale\n");
				for (indCour = 0; indCour < MAXFIC; indCour++)
				{
					if (val_fic[indCour] != ficInfo[indCour].min)
					{
						val_fic[indCour] = ficInfo[indCour].min;
						*ficTraChanged = 1;
					}
				}
			} else
			{
				int notFin = 1;
				int indice = 0;
				/* Par defaut le nom du fichier est
				 * inconnu. */
				erreur = 2;
				while ((notFin) && (indice < MAXFIC))
				{
					if (0 == strncmp(ptr_deb, (char *) ficInfo[indice].motClef,
							strlen((char *) ficInfo[indice].motClef)))
					{
						ptr_deb += strlen(ficInfo[indice].motClef);
						fic = indice;
						notFin = 0;
						erreur = 0;
					}
					indice++;
				} /* endwhile((notFin)&&                          */

				if (0 == erreur)
				{
					printDebug("cmd_cff: configuration du fichier %d\n", fic);
					if (*ptr_deb == '*')
					{
						/* il faudra calculer */
						if (fic != -1)
						{
							val_fic[fic] = -1;
								*ficTraChanged = 1;
						} /* endif(fic!=-1                                        */
					} else if (dv1_str_atol(&ptr_deb, &val_long))
					{
						if (fic != -1)
						{
							if (val_fic[fic] != val_long)
							{
								val_fic[fic] = val_long;
							}
								*ficTraChanged = 1;
						}
					} else
					{
						/* Valeur incorrecte du parametre.      */
						erreur = 3;
					}
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
			ptr_fin++;
			break;
		} /* endswitch (*ptr_fin)                         */
	} /* endwhile (((INT) (ptr_fin-buffe      */

	/* --------------------------------
	 * FIN DE cmd_cff
	 * --------------------------------     */
	printDebug("cmd_cff: erreur retournee %d\n", erreur);
	return (erreur);
}
//
//static bool sramBddLireFichier(int indice, T_zdf *info)
//{
//	bool retour = false;
//	if ((indice >= 0) && (indice < MAXFIC))
//	{
//		retour = true;
//		printDebug("sramBddLireFichier : %#x\n",(unsigned int)& sram_bdd[indice]);
//		sramRead(sram_pt, (int32) & sram_bdd[indice], 1, sizeof(T_zdf), info);
//	}
//	return retour;
//}
//
//static bool sramBddEcrireFichier(int indice, T_zdf *info)
//{
//	bool retour = false;
//	if ((indice >= 0) && (indice < MAXFIC))
//	{
//		retour = true;
//		printDebug("sramBddEcrireFichier: %#x\n",(unsigned int)& sram_bdd[indice]);
//		sramWrite(sram_pt, (int32) & sram_bdd[indice], 1, sizeof(T_zdf), info);
//	}
//	return retour;
//}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : cf2_cff                                                /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 05/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cf2.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande CFF                           /
 /-------------------------------------------------------------------------DOC*/
INT cf2_cff(INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
	INT j;
	INT flag_err = 0;
	LONG tab_val[MAXFIC];
	LONG sav_seq[MAXFIC];
	INT sz_enr[MAXFIC];
	LONG MO;
	LONG ML;
	INT fic;
	INT init_fic = FALSE;
	INT config = FALSE;
	INT iPos;
	/* Les variables permettent de deter-
	 * miner si changee et si la taille
	 * des fichiers
	 * de trace a changee. On peut ainsi
	 * distinguer les deux et on evite d'effacer
	 * les traces si ce n'est pas indispensable. */
	int ficTraChanged = 0;

	/* on se cale sur le premier carac-
	 * tere */
	iPos = position + 3;
	/* on elimine les separateurs de
	 * parametres */
	iPos += dv1_separateur(&buffer[iPos], lg_mess - iPos);
	/* si la commande comporte des para-
	 * metres */

	if (iPos < lg_mess)
	{
		/* dans ce cas, on est en ecriture, il faut
		 * tester le mot de passe */
		printDebug("cf2_cff: Configuration\n");
		if (identLireAutorisationAvecNiveau(las, 0))
		{
			{
			int indice;
			printDebug("cf2_cff: Autorisation accordee\n");
			config = TRUE;
			/* on recopie la config courante dans la
			 * table de calcul */
			for (indice = 0; indice < MAXFIC; indice++)
			{
				T_zdf info;
				sramBddLireFichier(indice, &info);
				sav_seq[indice] = info.max_seq;
				tab_val[indice] = info.max_seq;
			}
			}

			flag_err = cmd_cff(&buffer[iPos], lg_mess - iPos, tab_val, &ficTraChanged);

			if (0 == flag_err)
			{
				int indice = 0;
				/* Le nombre de sequences ne peut etre
				 * inferieur au nombre minimum defini.
				 * Attention, dans le cas ou l'on
				 * trouve -1, il s'agit du cas ou l'on
				 * veut le nombre max pour la mesure.
				 */
				printDebug("cf2_cff: Syntaxe commande correcte\n");

				while (indice < MAXFIC)
				{
					if ((tab_val[indice] < ficInfo[indice].min) && (-1 != tab_val[indice]))
					{
						tab_val[indice] = ficInfo[indice].min;
					}
					indice++;
				}
				/* pour tous les fichiers */
				for (indice = 0; indice < MAXFIC; indice++)
				{
					if(tab_val[indice]!=sav_seq[indice])
					{
					T_zdf info;
					sramBddLireFichier(indice, &info);
					info.max_seq = tab_val[indice];
					sramBddEcrireFichier(indice, &info);
					}
				}
				MO = 0;
				fic = 99;

				/* pour chaque fichier */
				for (indice = 0; indice < MAXFIC; indice++)
				{
					T_zdf info;
					sramBddLireFichier(indice, &info);
					sz_enr[indice] = cf3_szenrfic(indice, FALSE);

					/* si le nombre de sequence est positif */
					if (info.max_seq != -1L)
					{
						/* on augmente la taille de la memoire
						 * occupee */
						MO += (info.max_seq * sz_enr[indice]);
						if (MO > sramBddTailleGet())
						{
							flag_err = 3;
						}
					} else
					{
						if (fic == 99)
						{
							/* Positionnement automatique de la
							 * taille du fichier. */
							fic = indice;
						} else
						{
							/* Le caractere * a ete utilise plus
							 * d'une fois. */
							flag_err = 3;
						}
					}
				} /* endfor(i=0;                                          */

				if (fic != 99)
				{
					/* on doit calculer la taille d'un fichier */
					if ((ML = sramBddTailleGet() - MO) >= 0)
					{
						T_zdf info;
						sramBddLireFichier(fic, &info);
						if (sz_enr[fic] > 0)
						{
							info.max_seq = ML / sz_enr[fic];
						} else
						{
							info.max_seq = 0;
						}
						for (indice = 0; (indice < MAXFIC) && (2 != flag_err); indice++)
						{
							if (tab_val[indice] < ficInfo[indice].min)
							{
								flag_err = 3;
							}
						}
						sramBddEcrireFichier(fic, &info);
					}
				}
				/* Positionnement des pointeurs des
				 * fichiers. */
				if (0 == flag_err)
				{

					STRING valPrec = (STRING) 0 ;
					valPrec = (STRING) ( sramBddTailleGet());
					for (indice = 0; indice < MAXFIC; indice++)
					{
						T_zdf info;
						sramBddLireFichier(indice, &info);
						info.pt_deb = valPrec - info.max_seq * info.sz_enr;
						info.pt_ecr = info.pt_deb;
						info.pt_prem = info.pt_deb;
						valPrec = info.pt_deb;
						sramBddEcrireFichier(indice, &info);
					}
					/* une reinitialisation des fichiers                    */
					init_fic = TRUE;
				} else
				{
					/* on restaure la config */
					for (indice = 0; indice < MAXFIC; indice++)
					{
						T_zdf info;
						sramBddLireFichier(indice, &info);
						info.max_seq = sav_seq[indice];
						sramBddEcrireFichier(indice, &info);
					}
				}
			} /* endif(0==flag_err)                           */
		} else
		{
			flag_err = 5;
		} /* endif (identLireAutorisationAvec     */
	} /* endif (i < lg_mess)                          */
	/* TRAITEMENT DE LA COMMANDE            */
	/* Le traitement doit etre realise
	 * avant l'envoi de la reponse pour
	 * eviter les problemes lies aux
	 * fichiers de traces. */
	if (init_fic)
	{
		/* on reinitialise */
		idf_reinit(FALSE);
		if (ficTraChanged)
		{
			cf3_init_fic_tra();
		}
	}
	/* RESULTAT DE LA COMMANDE                      */
	/* on retourne la configuration de
	 * la station */
	if (flag_err == 0)
	{
		int indice;
		/* on initialise la memoire occupee */
		MO = 0;
		/* on calcule la memoire dispo          */
		/* pour tous les fichiers */
		for (indice = 0; indice < MAXFIC; indice++)
		{
			/* on ajoute le nombre d'octets occupes par ce fichier */
			T_zdf info;
			sramBddLireFichier(indice, &info);
			MO += (info.max_seq * info.sz_enr);
		}

		/* on calcule la memoire libre */
		ML = sramBddTailleGet() - MO;
		/* on formate le buffer de reponse */
		j = dv1_scpy(buffer, (STRING) "CFF ", 4);
		/* Impression du nombre de sequences par
		 * type de fichier. Modification
		 * par X. GAILLARD pour introduire
		 * une boucle sur tous les fichiers. */
		for (indice = 0; indice < MAXFIC; indice++)
		{
			T_zdf info;
			sramBddLireFichier(indice, &info);
			j += sprintf(&buffer[j], "%s%ld ", ficInfo[indice].motClef, info.max_seq);
		}

		/* memoire libre dans la station */
		j += sprintf(&buffer[j], "ML=%ld", ML / 0x400);

		if (flg_fin == TRUE)
		{
			buffer[j++] = '\n';
			buffer[j++] = '\r';
			/* apres le dernier capteur on envoie la reponse */
			tedi_send_bloc(las, mode, buffer, j, *bloc, FALSE, pt_mess);
		} else
		{
			/* apres le dernier capteur on envoie la reponse */
			tedi_send_bloc(las, mode, buffer, j, *bloc, TRUE, pt_mess);
		}
		if (++(*bloc) >= 10)
			*bloc = 0;
	} else
	{
		x01_cptr.erreur = flag_err;
		tedi_ctrl_ext(las, mode, flag_err);
		config = FALSE;
	}

	return (config);
}
