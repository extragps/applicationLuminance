/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pcp.c                                            /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 08/01/1991                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE : Lit les messages de commandes dans la boite aux   /
 / lettres LSi. Determine leur validite et demande leur execution par la       /
 / fonction adequate. Envoie les reponse dans la boites aux lettres LSi.       /
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
 / DONNEES EXPORTEES : AUCUNE                                                  /
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
#include "define.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "ted_prot.h"
#include "ypc_las.h"
#include "rec_main.h"
#include "tac_vip.h"
#include "lcr_cf0h.h"
#include "lcr_cf1h.h"
#include "lcr_cf2h.h"
#include "lcr_cf3h.h"
#include "lcr_cf4h.h"
#include "lcr_cf5h.h"
#include "lcr_cf6h.h"
#include "lcr_cf7h.h"
#include "lcr_idfh.h"
#include "lcr_regh.h"
#include "lcr_st1.h"
#include "lcr_seth.h"
#include "lcr_vs1h.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_tsth.h"
#include "lcr_vt.h"
#include "lcr_trc.h"
#include "lcr_cfs.h"
#include "tst_vip.h"
#include "Superviseur.h"
#include "mon_debug.h"
#include "tst_simu.h"
#include "lcr_bk.h"
#include "lcr_tst_sc.h"
#include "lcr_cfsl.h"
#include "lcr_cfpl.h"
#include "lcr_cfes.h"
#include "lcr_cfet.h"
#include "lcr_cftp.h"
#include "lcr_cfpp.h"
#include "lcr_st_lcpi.h"
#include "lcr_ident.h"
#include "lcr_tc_e.h"
#include "lcr_sc.h"
#include "lcr_temp.h"
#include "lcr_p.h"
#include "lcr_st_divers.h"
#include "lcr_tst.h"
#include "lcr_pcp.h"
#include "lcr_tst_def.h"
#include "lcr_cfid.h"
#include "identLib.h"
#include "stAlLib.h"
#include "etaSyst.h"

/* **********************************
 * DEFINITION DES MACROS
 * ********************************	*/

#define DEBUG 0
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* **********************************
 * FONCTIONS LOCALES
 * ******************************** */

static VOID analyse_syntaxique(INT, INT, INT, STRING, struct usr_ztf *);
static VOID pcp_lecmess(INT, INT);
static VOID terminal(INT, struct usr_ztf *);
static VOID test(INT, struct usr_ztf *);
static VOID protege(INT, struct usr_ztf *);

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : analyse_syntaxique                                     /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 01/03/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pcp                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : analyse la syntaxe des commandes reeues et demande l'  /
 / execution de celle qui sont repertoriees dans le LCR.                       /
 /-----------------------------------------------------------------------------/
 / DONNEES IMPORTEES : las  : numero du las de reception du message.           /
 /                     mode : mode du protocole TEDI utilise                   /
 /                     lg_mess : nombre de caracteres du message recu          /
 /                     buffer  : adresse du buffer de la commande recue        /
 /-------------------------------------------------------------------------DOC*/
static VOID analyse_syntaxique(INT las, INT mode, INT nbcar, STRING buffer, struct usr_ztf *pt_mess)
{
	INT i; /* indice de lecture du buffer  */
	UINT8 config = 0;
	INT bloc = 0;

	printDebug("analyse_syntaxique : buffer %s\n", buffer);
	x01_cptr.erreur = CPTRD_OK;
	x01_cptr.flag = TRUE;

	/* on elimine les separateurs d'avant la commande */

	i = dv1_separateur(&buffer[0], nbcar);

	/* --------------------------------
	 * IDENTIFICATION !!!!!
	 * -------------------------------- */
	/* Recherche dans le buffer d'une indentification en mode direct ou en
	 * mode parametre. */
	if (0==strncmp(&buffer[i],  "ID", 2))
	{
		/* Identification en mode direct */
		i += lcr_ident(las, mode, nbcar, buffer, i, pt_mess);
	} else
	{
		/* identification en mode parametre */
		/* La fonction retourne le nombre   */
		/* de caracteres restant e traiter  */
		/* eventuellement. */
		nbcar = lcr_ident_param(las, mode, nbcar, buffer, i, pt_mess);
	}
	/* --------------------------------
	 * TEMPORISATION
	 * -------------------------------- */
	if (rqtemp[TR_0 + las] != TP_HS)
	{
		/* on relance la tempo              */
		rqtemp[TR_0 + las] = TEMP_TERM;
	} /* endif(rqtemp                     */

	/* --------------------------------
	 * TRAITEMENT DES COMMANDES
	 * -------------------------------- */
	printDebug("Nombre de caracteres %d position %d\n", nbcar, i);
	if (i == nbcar)
	{
		/* Il ne reste plus de caracteres dans le buffer, pas la peine
		 * d'appeler une commande de decodage.  */
		printDebug("Tous les caracteres ont ete traites\n");
	} else if (0==strcmp(&buffer[i],  "TERMINAL=FALSE"))
	{
		x01_status3.tempo_mode_terminal[las] = 3600;
	} else if (0==strcmp(&buffer[i], "TERMINAL=TRUE"))
	{
		x01_status3.tempo_mode_terminal[las] = 0;
	} else if (0==strncmp(&buffer[i], "TC E", 4))
	{
		config = lcr_tc_e(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "ST LCPI", strlen("ST LCPI")))
	{
		config = lcr_st_lcpi(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "BK", strlen("BK")))
	{
		config = lcr_bk(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "CFET", 4))
	{
		config = lcr_cfet(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "CFES", 4))
	{
		config = lcr_cfes(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "CFTP", 4))
	{
		config = lcr_cftp(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "CFPL", 4))
	{
		config = lcr_cfpl(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "CFSL", 4))
	{
		config = lcr_cfsl(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i], (STRING) "TST ETH", strlen("TST_ETH")))
	{
		config = lcr_tst_eth(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i], "TST TEMP", 8))
	{
		lcr_temp(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "TST SCC", 7))
	{
		config = lcr_tst_sc_pmv(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i], "PE",2) || 0==strncmp(&buffer[i], "PS", 2))
	{

		lcr_pe(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i], "PA", 2) || 0==strncmp(&buffer[i], "P1", 2))
	{
		lcr_pa(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i], "P", 1))
	{

		lcr_p(las, mode, nbcar, buffer, i, pt_mess);
	}
	/* XG : Ajout de la commande CF* pour la liste des commandes de configuration
	 * du PMV. */
	else if (0==strncmp(&buffer[i],  "CFPP", 4))
	{
		config = lcr_cfpp(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "CF*", 3))
	{
		config = cf0_cf(las, mode, nbcar, buffer, i, pt_mess);
	}
	/* XG : Ajout de la commande CFAL pour la configuration des
	 * alarmes systemes dans le cas de l'application PMV */
	else if (0==strncmp(&buffer[i],  "CFAL", strlen("CFAL")))
	{
		config = cf5_cfal(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
		vct_ST_TR &= ~STATUS_TR_ALERTE;
	} else if (0==strncmp(&buffer[i],  "CFF", 3))
	{
		config = cf2_cff(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i], "TRACE", 5))
	{
		lcr_trace(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "TST SC", 6))
	{
		config = lcr_tst_sc(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "TST DEBUG", strlen("TST_DEBUG")))
	{
		config = lcr_tst_debug(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "TST LUM", strlen("TST_LUM")))
	{
		lcr_tst_lum(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "TST DEF", strlen("TST_DEF")))
	{
		lcr_tst_def(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "TST CNF", strlen("TST_CNF")))
	{
		lcr_tst_cnf(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "TST ", strlen("TST_")))
	{
		/* cette commande est protegee */
		if (identLireAutorisationAvecNiveau(las, 0))
		{
			lcr_tst_tst(las, mode, nbcar, buffer, i, pt_mess);
		} else
		{
			x01_cptr.erreur = 3;
			tedi_erreur(las, mode);
		}
	} else if (0==strncmp(&buffer[i],  "CFID", 4))
	{
		config = lcr_cfid(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "FIN", 3))
	{
		idf_fin(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "INIT", 4))
	{
		idf_init(las, mode, nbcar, buffer, i, pt_mess);
		vct_ST_TR &= ~STATUS_TR_ALERTE;
	} else if (0==strncmp(&buffer[i],  "DATE", 4) || 0==strncmp(&buffer[i],  "DT", 2))
	{
		idf_date(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "ST AL", 5))
	{
		config = cf6_st_al(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
		vct_ST_TR &= ~STATUS_TR_ALERTE;
	} else if (0==strncmp(&buffer[i],  "ST ERI", 6))
	{
		lcr_st_eri(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "ST BTR", 6))
	{
		config = lcr_st_btr(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "ST EDF", 6))
	{
		config = lcr_st_edf(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "ST VER", 6))
	{
		config = lcr_st_ver(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "ST OVF", 6))
	{
		/* Commande permettant de savoir si il y a eu debordement dans
		 * la file de reception WorldFip */
		config = lcr_st_ovf(las, mode, nbcar, buffer, i, pt_mess, FALSE, &bloc);
	} else if (0==strncmp(&buffer[i],  "ST LCOM", 7))
	{
		lcr_st_lcom(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "ST", 2))
	{
		config = st1_st(las, mode, nbcar, buffer, i, pt_mess);
		vct_ST_TR &= ~STATUS_TR_ALERTE;
	} else if (0==strncmp(&buffer[i],  "SETU", 4))
	{
		config = lcr_setu(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "SET", 3))
	{
		config = lcr_set(las, mode, nbcar, buffer, i, pt_mess);
	} else if (0==strncmp(&buffer[i],  "VT", 2))
	{
		lcr_vt(las, mode, nbcar, buffer, i, pt_mess);
	} else
	{
		/* Commande non reconnue.           */
		tedi_ctrl_ext(las, mode, 1);

		if (mode == MODE_TERMINAL)
		{
			/* on autorise la reception */
			vct_flag_IT[las] = 0;
		}
	}
	/* on reinitialise le bit b4 du     */
	/* status temps reel                */

	if (config)
	{

		mnSetTempo(TP_VALID_CONFIG, 160);
	}
	/* Suppression de l'identification
	 * temporaire si il y en a eu une.  */
	identInterdire(las, FALSE);
	/* --------------------------------
	 * FIN DE analyse_syntaxique
	 * -------------------------------- */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : protege                                                /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 01/03/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pcp                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement du message recu en mode protege             /
 /-----------------------------------------------------------------------------/
 / DONNEES IMPORTEES : las  : numero du las de reception du message.           /
 /                     pt_mess : pointeur vers la structure de message         /
 /-------------------------------------------------------------------------DOC*/

static VOID protege(INT las, struct usr_ztf *pt_mess)
{
	INT bcc; /* contient le bcc du message               */
	/* on elimine les acquitements et les interruptions */
	if (pt_mess->nbcar_max > 7)
	{
		/* on calcule le bcc du message recu */
		bcc = tedi_bcc(pt_mess->pt_buff, pt_mess->nbcar_rec - 1);

		/* le bcc calcule doit etre egal au bcc recu */
		if (bcc == pt_mess->pt_buff[pt_mess->nbcar_rec - 1])
		{

			/* on remplace les jokers */
			if (pt_mess->pt_buff[1] == '0')
			{
				pt_mess->pt_buff[1] = x01_status1.adr[0];
				x01_flag_jocker[las] = 1;
			}

			if (pt_mess->pt_buff[2] == '0')
			{
				pt_mess->pt_buff[2] = x01_status1.adr[1];
				x01_flag_jocker[las] = 1;

			}

			if (pt_mess->pt_buff[3] == '0')
			{
				pt_mess->pt_buff[3] = x01_status1.adr[2];
				x01_flag_jocker[las] = 1;

			}

			/* on teste l'adresse passe dans le message */
			if (0==strncmp(&(pt_mess->pt_buff[1]),  x01_status1.adr, 3))
			{

				/* l'adresse est correcte, on valide le mode terminal */
				rqtemp[TR_0 + las] = TEMP_TERM;

				/* si il s'agit d'une question, on passe a l'analyse syntaxique */
				if (pt_mess->pt_buff[0] == ENQ)
				{
					analyse_syntaxique(las, MODE_PROTEGE, pt_mess->nbcar_rec - 7, &(pt_mess->pt_buff[5]), pt_mess);
				}
			}
		} else
		{
			/* on signale une erreur BCC */
			switch (las)
			{
			case 0:
				x01_status3.er1 = 10 + x01_status3.er1 % 10;
				break;
			case 1:
				x01_status3.er2 = 10 + x01_status3.er2 % 10;
				break;
			}
			trt_alt_tester_alerte();
		}
	}
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : test                                                   /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 04/03/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pcp                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement du message recu en mode test                /
 /-----------------------------------------------------------------------------/
 / DONNEES IMPORTEES : las  : numero du las de reception du message.           /
 /                     pt_mess : pointeur vers la structure de message         /
 /-------------------------------------------------------------------------DOC*/

static VOID test(INT las, struct usr_ztf *pt_mess)
{
	/* on elimine les acquitements et les interruptions */
	if (pt_mess->nbcar_max > 6)
	{

		/* on remplace les jokers */
		if (pt_mess->pt_buff[1] == '0')
		{
			pt_mess->pt_buff[1] = x01_status1.adr[0];
			x01_flag_jocker[las] = 1;

		}

		if (pt_mess->pt_buff[2] == '0')
		{
			pt_mess->pt_buff[2] = x01_status1.adr[1];
			x01_flag_jocker[las] = 1;

		}

		if (pt_mess->pt_buff[3] == '0')
		{
			pt_mess->pt_buff[3] = x01_status1.adr[2];
			x01_flag_jocker[las] = 1;

		}

		/* on teste l'adresse passe dans le message */
		if (0==strncmp(&(pt_mess->pt_buff[1]),  x01_status1.adr, 3))
		{

			/* l'adresse est correcte, on valide le mode terminal */
			rqtemp[TR_0 + las] = TEMP_TERM;

			/* si il s'agit d'une question, on passe a l'analyse syntaxique */
			if (pt_mess->pt_buff[pt_mess->nbcar_rec - 1] == CR)
			{
				analyse_syntaxique(las, MODE_TEST, pt_mess->nbcar_rec - 6, &(pt_mess->pt_buff[5]), pt_mess);
			}
		}
	}
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : terminal                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 04/03/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pcp                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement du message recu en mode terminal            /
 /-----------------------------------------------------------------------------/
 / DONNEES IMPORTEES : las  : numero du las de reception du message.           /
 /                     pt_mess : pointeur vers la structure de message         /
 /-------------------------------------------------------------------------DOC*/

static VOID terminal(INT las, struct usr_ztf *pt_mess)
{
	/* on teste la validite du mode
	 * terminal */
	if (((rqtemp[TR_0 + las] > TP_FINI) || (!xdg_cf_las[las].protect)) && (x01_status3.tempo_mode_terminal[las] == 0))
	{
		printDebug("terminal : pt_mess->nbcar_rec %d et message %s\n", pt_mess->nbcar_rec, pt_mess->pt_buff);
		/* on elimine les acquitements et
		 * les IT */
		if (pt_mess->nbcar_rec > 1)
		{
			/* si il s'agit d'une question, on
			 * passe a l'analyse syntaxique */
			if (pt_mess->pt_buff[pt_mess->nbcar_rec - 1] == CR)
			{
				/* analyse_syntaxique(las,MODE_TERMINAL,
				 pt_mess->nbcar_rec-1,pt_mess->pt_buff,pt_mess); */
				analyse_syntaxique(las, MODE_TERMINAL, pt_mess->nbcar_rec - 1, pt_mess->pt_buff, pt_mess);
			}
		} else
		{
			/* on renvoie ! sur CR */
			if (pt_mess->pt_buff[0] == CR)
			{
				printDebug("terminal : on renvoie ! sur CR \n");
				tedi_ctrl(las, MODE_TERMINAL, TRUE);
			}
		} /* endif(pt_mess->nbcar             */
	} /* endif (((rqtemp [TR_0+las] >     */
	/* --------------------------------
	 * FIN DE terminal
	 * -------------------------------- */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : pcp_lecmess                                            /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 08/01/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_pcp                                              /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : fonction de reception des messages du langage de cde   /
 / routier. Reconnait les commandes et demande leur execution.                 /
 /-------------------------------------------------------------------------DOC*/

static VOID pcp_lecmess(INT numero, INT las)
{
	/* -------------------------------- */
	/* DECLARATION DES VARIABLES        */
	/* -------------------------------- */
	struct usr_ztf message;
	struct usr_ztf *pt_message = &message;
	int retour;
	char Buffer[MSG_SIZE];
	/* --------------------------------
	 * SYNCHRO
	 * -------------------------------- */
	mnsuspIniPrendre();
	/* Peut etre faudrait il faire un petit mnsusp!!!! */
	/* --------------------------------
	 * BOUCLE DE TRAITEMENT
	 * -------------------------------- */
	do
	{
		/* Initialisation de la structure message et mise en attente    */
		pt_message = monMessInit(ancrage_pcp_lecmess[las], pt_message);
		monMessSend(x01_ancrage_TEDI_esclave_rec[las], &pt_message, sizeof(pt_message));
		/* on se met en attente sur l'ancrage de retour */
		retour = monMessRec(ancrage_pcp_lecmess[las], 500, (char *) Buffer);

		if (2 != retour)
		{
			/* Traitement d'un message reeu */
			memcpy(&pt_message, Buffer, sizeof(struct usr_ztf *));
			x01_cptr.erreur = CPTRD_OK;
			if (vct_IT[las])
			{
				vct_IT[las] = FALSE;
				x01_flag_jocker[las] = 0;
				if (pt_message->compte_rendu == CR_OK)
				{
					switch (pt_message->mode)
					{
					case MODE_PROTEGE:
						protege(las, pt_message);
						break;
					case MODE_TEST:
						test(las, pt_message);
						break;
					case MODE_TERMINAL:
						terminal(las, pt_message);
						break;
					} /* endswitch(pt_message->compte_    */
					if (x01_cptr.erreur == CPTRD_OK)
					{
						/* Faire ici les eventuels traitements d'erreur */
					}
				}
				else
				{
				} /* endif(pt_message                 */
			} /* endif (vct_IT [las])             */
		} /* endif(2!=retour)     */
		/* Increment du compteur..... */
		etaSystIncrCompteur(numero);
	} while (TRUE);
	/* ----------------------------
	 * FIN DE pcp_lecmess
	 * ---------------------------- */
}

/* --------------------------------
 * pcp_main
 * ========
 * Fonciton permettant de lancer
 * une tache d'analyse pour un port
 * donne si celui ci est connu comme
 * etre un port TEDI_PROT.
 * Entree :
 * -    le numero de la liaison.
 * -------------------------------- */

void pcp_main(int numero, int liaison)
{
	if (xdg_cf_las[liaison].protocole == TEDI_PROT)
	{
		pcp_lecmess(numero, liaison);
	} else
	{
		printDebug("_pcp_main %d: protocole incorrect %d\n", liaison, xdg_cf_las[liaison].protocole);
	}
}

/* --------------------------------
 * pcp_interne
 * ============
 * Traitement des message internes.
 * A priori, il s'agit surtout de
 * traiter les messages d'affichage
 * PMV.
 * -------------------------------- */

VOID pcp_interne(int numero)
{
	/* -------------------------------- */
	/* DECLARATION DES VARIABLES        */
	/* -------------------------------- */
	struct usr_ztf *pt_message = NULL;
	int retour;
	char Buffer[MSG_SIZE];
	/* --------------------------------
	 * SYNCHRO
	 * -------------------------------- */
	mnsuspIniPrendre();
	/* --------------------------------
	 * BOUCLE DE TRAITEMENT
	 * -------------------------------- */
	while (TRUE)
	{
		/* on se met en attente sur l'ancrage de retour. Le fonctionnement est un
		 * peu different de pcp_lecmess car les structures de donnees (message)
		 * sont allouees dans une autre tache. */
		retour = monMessRec(x01_ancrage_interne, 500, (char *) Buffer);
		if (2 != retour)
		{
			memcpy(&pt_message, Buffer, sizeof(struct usr_ztf *));
			x01_cptr.erreur = CPTRD_OK;
			printDebug("Dans pcpInterne : nbCar %d , message : %s\n", pt_message->nbcar_rec, pt_message->pt_buff);
			ksleep(500);
			analyse_syntaxique(-1, MODE_TERMINAL, pt_message->nbcar_rec, pt_message->pt_buff, pt_message);
			if (x01_cptr.erreur != CPTRD_OK)
			{
				pt_message->nbcar_max = 0;
			}
		}
		if (NULL != pt_message)
		{
			monMessSend(pt_message->entete.MqId, &pt_message, sizeof(pt_message));
		}
		etaSystIncrCompteur(numero);
	}
	/* ----------------------------
	 * FIN DE pcp_interne
	 * ---------------------------- */

}

/* ****************************
 * FIN DU FICHIER lcr_pcp.c
 * **************************** */
