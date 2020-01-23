/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cfid                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 18/10/2005                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE : Fonction de configuration des identifiants		  /
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
 /-------------------------------------------------------------------------DOC*/
#ifdef VXWORKS
#include "vxworks.h"
#include "string.h"
#else
#include <memory.h>
#endif

#include "standard.h"                  /* redefinition des types                         */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
#include "sir_dv1h.h"
#include "sir_dv2h.h"

#include "ted_prot.h"
#include "identLib.h"
#include "lcr_cfid.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cfid                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 12/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cfid.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande CFID                          /
 /-------------------------------------------------------------------------DOC*/
INT lcr_cfid(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin,
		INT * bloc)
{

	INT i, k; /* indice de lecture des caracteres du buffer question */
	INT j = 0;
	INT fini; /* variable de controle de boucle                      */
	INT flag_err = 0; /* drapeau indicateur d'erreur dans la commande        */
	INT taille; /* taille d'un bloc de configuration de voie           */
	INT num_util; /* numero de l'utilisateur                             */
	struct zip sav_acces[NBID]; /* sauvegarde des couples idf/pwd          */
	UINT8 sav_nb_acces;
	UINT8 trv;
	INT config = FALSE;
	/* cette commande n'est permise que
	 * si elle a ete deverouilee au
	 * niveau materiel dans la station -
	 * strap en position OFF       */
	if (tedi_securite())
	{
		i = position + 4;
		/* on elimine les separateurs de
		 * parametres */
		i += dv1_separateur(&buffer[i], lg_mess - i);
		/* si la commande comporte des para-
		 * metres */
		if (i < lg_mess)
		{
			/* on sauvegarde la config courante */
			dv1_scpy((STRING) sav_acces, (STRING) vct_acces, NBID * sizeof(struct zip));
			sav_nb_acces = vct_nb_acces;
			config = TRUE;
			/* on se cale sur le premier
			 * caractere */
			i = position + 4;
			/* on initialise la variable de
			 * controle de la boucle while */
			fini = FALSE;
			/* tant que l'on n'a pas atteint la
			 * fin du message */
			while (!fini)
			{
				/* on elimine les separateurs de para-
				 * metres */
				i += dv1_separateur(&buffer[i], lg_mess - i);
				/* on recherche le debut du prochain
				 * bloc de parametrage de fichier */
				if ((i < lg_mess) && ((buffer[i] == 'Z') || (buffer[i] == 'S') || ((buffer[i] >= '1') && (buffer[i]
						<= ('0' + NBID)))))
				{
					/* si on a trouve un Z, on arrette
					 * le decodage et on ne retient que
					 * ce parametre */
					if (buffer[i] == 'Z')
					{
						/* on arrete prematurement le
						 * decodage */
						if ((i + 1 < lg_mess) && ((buffer[i + 1] != ' ') && (buffer[i + 1] != ',')))
						{
							/* on genere une erreur de syntaxe */
							flag_err = 1;
						} else
						{
							/* raz de la configuration pour tous
							 * les identifiants */
							identMettreAZero();
#ifdef RAD
							/* on raz les formats */
							vct_nb_cfmf = 0;
#endif
						}
						/* on arrete le decodage */
						fini = TRUE;
					} else if (buffer[i] == 'S')
					{
						/* on arrete prematurement le
						 * decodage */
						if ((i + 1 < lg_mess) && ((buffer[i + 1] != ' ') && (buffer[i + 1] != ',')))
						{
							/* on genere une erreur de syntaxe */
							flag_err = 1;
						} else
						{
							/* raz de la configuration pour tous
							 * les identifiants */
							identStandard();
							/* on raz les formats */
#ifdef RAD
							vct_nb_cfmf = 0;
#endif
						}
						/* on arrete le decodage */
						fini = TRUE;
					} else
					{
						/* on calcule la taille du bloc d'info
						 * d'un identifiant */
						taille = dv1_cherchefin(&buffer[i], lg_mess - i);
						/* le 2ieme caractere du bloc doit
						 * etre un '=' */
						if (buffer[i + 1] != '=')
						{
							/* erreur de syntaxe */
							flag_err = 1;
						} else
						{
							/* on lit le numero de l'utilisateur
							 * qui permet  */
							/* de stocker les infos dans la bonne
							 * case de la */
							/* table des utilisateurs       */
							num_util = buffer[i] - '0';
							if ((num_util < 0) || (num_util >= NBID))
							{
								flag_err = 1;
							} else
							{
								/* on deplace l'indice de lecture */
								if (i + 2 >= lg_mess)
								{
									/* erreur de syntaxe */
									flag_err = 1;
								} else
								{
									i += 2;
									/* on cherche la fin du nom de
									 * l'identifiant */
									j = dv1_car(&buffer[i], lg_mess - i, '/');
									/* la taille de l'identifiant doit
									 * etre <= 8 */
									if (j > 8)
									{
										/* erreur de syntaxe */
										flag_err = 1;
									} else
									{
										/* on efface l'identifiant courant */
										trv = FALSE;
										k = 0;
										while ((!trv) && (k < vct_nb_acces))
										{
											if (vct_acces[k].num_util == num_util)
											{
												trv = TRUE;
											} else
											{
												k++;
											}
										} /* endwhile((!trv                               */
										if ((!trv) && (k < NBID - 1))
										{
											vct_nb_acces++;
											trv = TRUE;
										}
										if (trv)
										{
											char identifiant[8];
											char password[8];
											vct_acces[k].num_util = (unsigned char) num_util;
											/* on sauvegarde le nom de l'identi-
											 * fiant */
											dv1_scpy((STRING) identifiant, (STRING) & buffer[i], j);
											identifiant[j] = 0;
											/* on deplace l'indice de lecture */
											if (i + j + 1 >= lg_mess)
											{
												/* erreur de syntaxe */
												flag_err = 1;
											} else
											{
												i += (j + 1);
												/* on calcule la taille du password */
												j = taille - j - 3;
												/* la taille de l'identifiant doit
												 * etre <= 8 */
												if (j > 8)
												{
													/* erreur de syntaxe */
													flag_err = 1;
												} else
												{
													/* on sauvegarde le passwd
													 * de l'identifiant */
													dv1_scpy((STRING) password, (STRING) & buffer[i], j);
													password[j] = 0;
													identAjouter(identifiant, password, num_util);
												}
											}
										} else
										{
											flag_err = 1;
										}
									}
								}
							}
						} /* endif (buffer[i+1] != '=')           */
						/* on deplace l'index de lecture du
						 * buffer */
						i += j;
					} /* endif (buffer[i] == 'Z')                     */
				} else
				{
					if (i < lg_mess)
					{
						/* on genere une erreur */
						flag_err = 1;
					}
				} /* endif ((i < lg_mess) &&                      */
				/* si on a passe en revue tout le
				 * buffer */
				if ((i >= lg_mess) || (0 != flag_err))
				{
					fini = TRUE;
				}
			} /* endwhile (! fini)                            */
			/* si on a pas detecte d'erreur */
			if (flag_err != 0)
			{
				/* on restaure l'ancienne confi */
				memcpy(vct_acces, sav_acces, NBID * sizeof(struct zip));
				vct_nb_acces = sav_nb_acces;
			}
		} /* endif (i < lg_mess)                          */
		/* on retourne la configuration de
		 * la station */
		if (flag_err == 0)
		{
			/* on formate le buffer de reponse */
			j = dv1_scpy(buffer, (STRING) "CFID", 0);
			/* pour chaque paire idf/pwd */
			for (i = 0; i < vct_nb_acces; i++)
			{
				buffer[j++] = ' ';
				/* ecriture numero de l'utilisateur */
				buffer[j++] = (unsigned char) ('0' + vct_acces[i].num_util);
				buffer[j++] = '=';
				/* on copie dans le buffer de reponse
				 * l'identifiant */
				j += dv1_scpy(&buffer[j], vct_acces[i].identifiant, 0);
				/* l'identifiant est separe de son
				 * password par un / */
				buffer[j++] = '/';
				/* on copie dans le buffer de reponse
				 * le password */
				j += dv1_scpy(&buffer[j], vct_acces[i].password, 0);
			} /* endfor (i=0;i < vct_nb_acces;i       */
			if (flg_fin == TRUE)
			{
				buffer[j++] = '\n';
				buffer[j++] = '\r';
				tedi_send_bloc(las, mode, buffer, j, *bloc, FALSE, pt_mess);
			} else
			{
				tedi_send_bloc(las, mode, buffer, j, *bloc, TRUE, pt_mess);
			}
			if (++(*bloc) >= 10)
			{
				*bloc = 0;
			}
		} else
		{
			x01_cptr.erreur = (unsigned char) flag_err;
			tedi_erreur(las, mode);
			config = FALSE;
		}
	} else
	{
		tedi_ctrl_ext(las, mode, 5);
	}
	/* --------------------------------
	 * FIN DE cf4_cfid
	 * --------------------------------     */
	return (config);
}
