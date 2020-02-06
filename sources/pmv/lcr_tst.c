/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_tst                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 08/04/97                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE : Commande TST diverses                             /                                                                             /
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
#include "portage.h"
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
#include "lcr_tst.h"
#include "mgpLib.h"
#include "configurationSram.h"

/* *********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define MODULE  1                      /* demande module */
#define CRTRACK 2                      /* des cartes presentes dans le rack */
#define DUMP    3                      /* dump                              */
#define TRACE   4                      /* trace                             */
#define TSTPILE 5                      /* pile                              */
#define SECTEUR 6                      /* reprise secteur                    */
#define VERSION 7
#define TC_TS   8
#define TC_TC   9
#define FICINV     10

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

LOCAL INT lcr_cmd_tst(STRING, INT, struct cmd_tst *);

/* TODO : XG le 10/11/05. Il y a visiblement un probleme avec la variable
 * dont je trouve pas l'initialisation. */
char *dateVersion = "du " __TIME__ " " __DATE__ ;


void lcr_cmd_tst_print_version(void)
{
	printf("VERSION %d.%d %s\n", versionLireMajeure ("Application"),
                            versionLireMineure ("Application"), dateVersion,0);
}
/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : cmd_tst_sc                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/97                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_sc.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande TST SC                        /
 /-------------------------------------------------------------------------DOC*/
LOCAL INT lcr_cmd_tst(STRING buffer, INT nbcar, struct cmd_tst *cmd_tst)
{

	INT val;
	LONG val_lg;
	BYTE erreur;
	STRING ptr_deb;
	STRING ptr_fin;
	STRING ptr;
	BYTE k;
	/* le dernier caractere est nul */
	buffer[nbcar] = 0;

	ptr_deb = buffer;
	ptr_fin = buffer;

	erreur = FALSE;

	while (((INT) (ptr_fin - buffer) <= nbcar) && (!erreur))
	{
		switch (*ptr_fin)
		{

		case 0:
		case ' ':
			/* si deux separateur ne se suivent pas */
			if (ptr_deb != ptr_fin)
			{
				if (*ptr_deb == 'Z')
				{
					if (++ptr_deb >= ptr_fin)
					{
						erreur = FALSE;
					} else
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "MOD", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* les module */
					cmd_tst->cmd = MODULE;

					/* on positionne le pointeur */
					ptr_deb += 3;
					if (*ptr_deb == '=')
						ptr_deb++;

					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;

					while ((ptr_deb < ptr_fin) && (cmd_tst->nb_par < MAX_PAR_TST) && (!erreur))
					{
						/* on recherche la liste des parametres */
						if (dv1_str_symb(&ptr_deb, &val))
						{
							/* le parametre */
							cmd_tst->par[cmd_tst->nb_par++] = (BYTE) val;
						} else
							erreur = TRUE;
					}

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "CRT", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* les module */
					cmd_tst->cmd = CRTRACK;

					/* on positionne le pointeur */
					ptr_deb += 3;

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
					else
						erreur = FALSE;
				} else if (dv1_scmp(ptr_deb, "INV", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* les module */
					cmd_tst->cmd = FICINV;

					/* on positionne le pointeur */
					ptr_deb += 3;

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
					else
						erreur = FALSE;
				} else if (dv1_scmp(ptr_deb, "SECT", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* les module */
					cmd_tst->cmd = SECTEUR;

					/* on positionne le pointeur */
					ptr_deb += 4;
					if (*ptr_deb == '=')
						ptr_deb++;

					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;

					while ((ptr_deb < ptr_fin) && (cmd_tst->nb_par < MAX_PAR_TST) && (!erreur))
					{
						/* on recherche la liste des parametres */
						if (dv1_str_symb(&ptr_deb, &val))
						{
							/* le parametre */
							cmd_tst->par[cmd_tst->nb_par++] = (BYTE) val;
						} else
							erreur = TRUE;
					}

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "MEM=", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* les module */
					cmd_tst->cmd = DUMP;

					/* on positionne le pointeur */
					ptr_deb += 4;

					/* on suprime les separateurs pour la commande dv1_htol */
					ptr = ptr_deb;
					while (ptr < ptr_fin)
					{
						switch (*ptr)
						{
						case ',':
						case ':':
						case '.':
							*ptr = 0;
							break;
						}
						ptr++;
					}
					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;

					/* a priori il y a une erreur */
					erreur = FALSE;
					while ((ptr_deb < ptr_fin) && (cmd_tst->nb_par < MAX_PAR_TST) && (!erreur))
					{
						/* on recherche la liste des parametres */
						if (dv1_htol(ptr_deb, 0, &val_lg))
						{
							/* le parametre */
							cmd_tst->par_lg[cmd_tst->nb_par++] = val_lg;
						} else
							erreur = TRUE;
						while ((ptr_deb < ptr_fin) && (*ptr_deb))
						{
							ptr_deb++;
						}
						ptr_deb++;
					}

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "DEF", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* on positionne le pointeur */
					ptr_deb += 3;

					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;

					/* les module qui on un defaut */
					cmd_tst->def = TRUE;

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "TRC", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* on positionne le pointeur */
					ptr_deb += 3;

					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;

					/* les module qui on un defaut */
					cmd_tst->cmd = TRACE;

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "IO", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* on positionne le pointeur */
					ptr_deb += 2;

					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;

					/* les module qui on un defaut */
					cmd_tst->cmd = TC_TS;

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "TC=", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* on positionne le pointeur */
					ptr_deb += 3;

					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;

					/* les module qui on un defaut */
					cmd_tst->cmd = TC_TC;

					/* on controle la longueur */
					if ((INT) (ptr_fin - ptr_deb) == 6)
					{
						/* on controle le buffer */
						for (k = 0; k < 6; k++)
						{
							if ((ptr_deb[k] != '0') && (ptr_deb[k] != '1') && (ptr_deb[k] != '*'))
								erreur = TRUE;
						}
						if (!erreur)
						{
							for (k = 0; k <  6; k++)
							{
								if (ptr_deb[k] == '0')
								{
									mgpPositionnerSortie(k,0);
								}
								else
								{
									mgpPositionnerSortie(k,1);
								}
							}
						}
						ptr_deb = ptr_fin;
					} else
						erreur = TRUE;
					if (ptr_deb < ptr_fin)
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "VERSION", 0))
				{
					/* fin du parametre */
					*ptr_fin = 0;

					/* on positionne le pointeur */
					ptr_deb += 7;

					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;

					/* les module qui on un defaut */
					cmd_tst->cmd = VERSION;

					if (ptr_deb < ptr_fin)
						erreur = TRUE;
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
	{
		x01_cptr.erreur = CPTRD_SYNTAXE;
	}
	return erreur;
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_tst_sc                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 20/12/1996                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_init                                             /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande INIT                          /
 /-------------------------------------------------------------------------DOC*/
void lcr_tst_tst(INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess)
{
	INT i, indice;
	BYTE erreur = FALSE;
	struct cmd_tst cmd_tst;
	STRING ptr_byte;

	cmd_tst.nb_par = 0;
	cmd_tst.def = FALSE;
	cmd_tst.cmd = N_AFF;

	for (indice = 0; indice < MAX_PAR_TST; indice++)
	{
		cmd_tst.par[indice] = 0;
	}

	i = position + 4;

	if (lg_mess > i)
	{
		erreur = lcr_cmd_tst(&buffer[i], lg_mess - i, &cmd_tst);
	}

	if (x01_cptr.erreur == CPTRD_OK)
	{
		int k;
		int j = 0;
		int bloc = 0;
		int fini = FALSE;
//		T_x01_reprise_secteur reprise_secteur;
//		T_x01_reprise_secteur *x01_reprise_secteur = &reprise_secteur;
//		sramRead(sram_pt, (int32) & sram_var->reprise_secteur, 1, sizeof(T_x01_reprise_secteur), x01_reprise_secteur);

		switch (cmd_tst.cmd)
		{
		case N_AFF:
			break;

		case MODULE:
			break;
		case CRTRACK:
			/* les cartes presentes dans le rack */
			j += sprintf(&buffer[j], "TST CRT MGP102=%03d",(mgpInit()?1:0));
			break;
		case DUMP:
			if (cmd_tst.nb_par == 1)
			{
				cmd_tst.par_lg[1] = 16L;
			}
			if (cmd_tst.par_lg[1] > 128)
			{
				cmd_tst.par_lg[1] = 128L;
			}
			if ((cmd_tst.nb_par == 2) && ((cmd_tst.par_lg[0] >= 0) && (cmd_tst.par_lg[0] < AD_MAX_CPU)))
			{
				ptr_byte = (STRING) cmd_tst.par_lg[0];
				j += dv1_scpy(&buffer[j], "TST MEM", 0);
				for (k = 0; k < cmd_tst.par_lg[1]; k++)
				{
					if (j >= 200)
					{
						/* on envoie le buffer courant */
						tedi_send_bloc(las, mode, buffer, j, bloc, FALSE, pt_mess);

						/* on enregistre le no de bloc */
						bloc = (bloc + 1) % 10;

						/* on remet a zero l'indice de remplissage de buffer */
						j = 0;
					}
					if (k % 16 == 0)
					{
						j += sprintf(&buffer[j], "\n\r%08X : ", (unsigned int) ptr_byte);
					}
					j += sprintf(&buffer[j], " %02X", (unsigned int) *ptr_byte);
					ptr_byte++;
				}
			}
			break;

		case SECTEUR:
			j += sprintf(&buffer[j], "TST SECT CPT=%03lu CPT1=%03lu CPT1_10=%03lu CPT10=%03lu \n\r",
					configGetCptUn()+configGetCptDix()+configGetCptPlus(),
					configGetCptUn(),configGetCptDix(),configGetCptPlus() );
			/* Remplacer ici par les nouvelles informations. */
			{
				Ktimestamp date_t;
				configGetDateCoupure(&date_t);
				j += sprintf(&buffer[j], "DEBUT=%02ld/%02ld/%02ld %02ld:%02ld:%02ld", date_t.day_dw, date_t.month_dw,
						(date_t.year_dw) % 100, date_t.hour_dw, date_t.min_dw, date_t.sec_dw);
				configGetDateReprise(&date_t);
				j += sprintf(&buffer[j], "FIN=%02ld/%02ld/%02ld %02ld:%02ld:%02ld", date_t.day_dw, date_t.month_dw,
						(date_t.year_dw) % 100, date_t.hour_dw, date_t.min_dw, date_t.sec_dw);
			}

			j += sprintf(&buffer[j], " DIFF=%04lu ACT=%01d\n\r", configGetDiffSect(),1);
			j += sprintf(&buffer[j], " T CONF= %ld %d ", (ULONG) (x01_status3.taille_config),
					(UINT) (x01_status3.taille_config));
			j += sprintf(&buffer[j], " RK DISTANT TRAME= %ld ERREUR TRAME = %ld ", (ULONG) xdg_ctrl_rack.nbTrame,
					(ULONG) xdg_ctrl_rack.nbErreurTrame);

			break;

		case VERSION:
			/* les traces */
			j += sprintf(&buffer[j], "TST VERSION %d.%d %s", versionLireMajeure ("Application"),
                            versionLireMineure ("Application"), dateVersion,0);
			break;
		case TC_TC:
		case TC_TS:
		{
			int l;
			j += sprintf(&buffer[j], "TST IO TS=");
			for(l=0;l<10;l++)
			{
			int entree=mgpLireEntree(l);
				switch(entree)
				{
				case -1:
					buffer[j++] = '?';
					break;
				case 0:
					buffer[j++] = '0';
					break;
				case 1:
					buffer[j++] = '1';
					break;
				default:
					buffer[j++] = '.';
					break;
				}
			}
			j += sprintf(&buffer[j], " TC=");
			for (l = 0; l < 6; l++)
			{
			int commande=mgpLireSortie(l);
			switch(commande)
				{
				case -1:
					buffer[j++] = '?';
					break;
							case 0:
								buffer[j++] = '0';
								break;
							case 1:
								buffer[j++] = '1';
								break;
							default:
								buffer[j++] = '.';
								break;
							}
			}

		}
			break;
		}
		if ((j >= 0) && (!fini))
		{
			tedi_send_bloc(las, mode, buffer, j, bloc, TRUE, pt_mess);

			if (++bloc >= 10)
				bloc = 0;
			j = 0;

			if (vct_IT[las])
				fini = TRUE;
		}
	} else
	{
		tedi_erreur(las, mode);
	}
}
/* *********************************
 * FIN DE LCR_TST.c
 * ********************************* */
