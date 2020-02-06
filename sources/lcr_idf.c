/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf                                              /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE : Regroupe les fonctions de restrictions d'acces a  /
 / la station.                                                                 /
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
 * 01a,02Mar01,xag	Ajout de l'initialisation des tables utilisateurs par
 * 					liaison.
 *	*/

#include "string.h"
#include "time.h"
#include "standard.h"                  /* redefinition des types                         */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
#include "xdg_str.h"                   /* definition des variables globales               */
#include "xdg_var.h"                   /* definition des variables globales               */
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_cf3h.h"
#include "ted_prot.h"
#include "rec_main.h"
#include "tac_vip.h"
#include "identLib.h"
#include "seqLib.h"
#include "mon_debug.h"
#include "lcr_util.h"
#ifdef PMV
#include "lcr_init.h"
#endif

/* Nouvelles declation pour la gestion de la configuration */
#include "configuration.h"
#include "configurationSram.h"

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

static int lcr_idf_debug = 0;

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

VOID idf_id(INT, INT, INT, STRING, INT, struct usr_ztf *);
VOID idf_fin(INT, INT, INT, STRING, INT, struct usr_ztf *);
VOID idf_init(INT, INT, INT, STRING, INT, struct usr_ztf *);
VOID idf_date(INT, INT, INT, STRING, INT, struct usr_ztf *);
VOID idf_reinit(INT);
INT idf_reinit_date(STRING, INT);
VOID idf_datebis(ULONG, STRING);
ULONG idf_sec(STRING);
INT test_date(STRING);
LOCAL VOID ini_calcul(INT);

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : ini_calcul                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : initialise les variables de calcul                     /
 /-------------------------------------------------------------------------DOC*/
/* Ajout de l'initialisation de la totalite du tableau vct_TV	*/

LOCAL VOID ini_calcul(flag)
	INT flag;
{
#ifdef RAD
	INT i, j;
	UINT8 date[SZ_DATE];
	ULONG sec;
	ULONG date_sec;
	ULONG periode;
	UINT8 num_capt;
	UINT8 nb_capt;

	if (flag)
	{
		/* on lit la date et l'heure courante */
		mnlcal(date);

		/* on la convertie en secondes */
		date_sec = idf_sec(date);

		/* pour tous les fichiers de mesures faire... */
		for (i = 0; i < MAXFICMES; i++)
		{
			/* on la date convertie en numero de periodes pour chaque type de donnees */
			periode = seqGetPeriode(vct_bdd[i].ty_seq);
			if (!periode)
			{
				periode = 1;
			}
			sec = (date_sec / periode) * periode;
			vct_date[i] = sec;
			vct_no_periode[i] = sec / periode;
			/* mise a l'heure des detecteurs */
			vct_controle.init_co = TRUE;
		}
	}

	/* pour les conditions d'alerte  */
	for (i = 0; i < NBCANAL; i++)
	{
		vct_det_alerte.ind[i] = FALSE;
	}
	for (i = 0; i < NBCANAL; i++)
	{
		vct_det_alerte.my[i] = FALSE;
	}

	for (i = 0; i < MAX_CRT_SY; i++)
	{
		vct_det_alerte.sys[i] = FALSE;
	}
	/* les alerte INT108 */
	for (i = 0; i < 4; i++)
	{
		vct_ctrl_alerte_int108.duree[i] = 0;
	}

	vct_int108_in = 0;
	vct_int108_out = 0;

	/* on raz la fifo de stockage des silhouettes */
	for (i = 0; i < MAX_DONNEE_SILHOUETTE; i++)
	{
		vct_donnee_silhouette[i].flag = FALSE;
	}
	/* communication sur alertes */
	/* suppression du test des alertes */
	vct_det_alerte.test = FALSE;

	/* on raz les alertes temps reel */
	vct_alerte_tr.alerte = FALSE;
	vct_alerte_tr.canal_surcharge = 0xFF;
	vct_alerte_tr.canal_contresens = 0xFF;

	for (i = 0; i < NB_ALARME; i++)
	{
		switch (vct_cf_alarme[i].type)
		{
		case ALARME_NULLE:
			i = NB_ALARME;
			break;
		case ALARME_EDF:
			vct_dyn_alarme[i].N_SAT = FALSE;
			break;
		case ALARME_DET:
			vct_dyn_alarme[i].N_SAT = FALSE;
			break;
		case ALARME_TRAFIC:
			/* on initialise les variables dynamiques */
			vct_dyn_alarme[i].N_SAT = vct_cf_alarme[i].N_SAT; /* nombre de sequence QT                  */
			vct_dyn_alarme[i].N_Q = vct_cf_alarme[i].N_Q; /* nombre de sequence QT                  */
			vct_dyn_alarme[i].N_T = vct_cf_alarme[i].N_T; /* nombre de sequence TT                  */
			vct_dyn_alarme[i].N_V = vct_cf_alarme[i].N_V; /* nombre de sequence VT                  */
			break;
		}
	}

	/* on traite les alarmes */
	rec_alarme();

	/* Boucles profondes */
	x01_bclprf_auto_calibrage = TRUE;
	x01_bclprf_g.sl = 0.0;
	x01_bclprf_g.nv = 0.0;
	x01_bclprf_g.lm = 0.0;
	x01_bclprf_g.lmv = 0.0;
	x01_bclprf_g.coef = 1.0;

	/* mesure en cours d'elaboration */
	for (i = 0; i < NBCAPTEUR; i++)
	{
		int indTv;
		T_vct_detection *det = &vct_detection[i];
		/* Boucles profondes */
		x01_bclprf_c[i].sl = 0.0;
		x01_bclprf_c[i].nv = 0.0;
		x01_bclprf_c[i].lm = 0.0;
		x01_bclprf_c[i].lmv = 0.0;
		x01_bclprf_c[i].coef = 1.0;

		for (indTv = 0; indTv < NB_CPT_TV; indTv++)
		{
			vct_TV[indTv][i].Q = 0;
			vct_TV[indTv][i].T = 0L;
			vct_TV[indTv][i].QV = 0;
			vct_TV[indTv][i].QL = 0L;
			vct_TV[indTv][i].TIV = 0L;
			vct_TV[indTv][i].UnsurV = 0.0;
			vct_TV[indTv][i].QVL = 0;
			vct_TV[indTv][i].UnsurVL = 0.0;
			vct_TV[indTv][i].dtp = 0L;
		}
		vct_Qhor[i] = 0;

		vct_Qjour[i] = 0L;

		for (j = 0; j < NBLCLASS; j++)
		{
			vct_LC[i][j] = 0L;
		}

		for (j = 0; j < NBVCLASS; j++)
		{
			vct_VC[i][j] = 0L;
		}

		for (j = 0; j < NBKCLASS; j++)
		{
			vct_KC[i][j] = 0L;
		}

		for (j = 0; j < NBPCLASS; j++)
		{
			vct_PC[i][j] = 0L;
		}

		for (j = 0; j < NBECLASS; j++)
		{
			vct_EC[i][j] = 0L;
		}

		for (j = 0; j < NBTCLASS; j++)
		{
			vct_TC[i][j] = 0L;
		}

		for (j = 0; j < NBKSLASS; j++)
		{
			vct_KS[i][j] = 0L;
		}

		for (j = 0; j < NBPMLASS; j++)
		{
			vct_PM[i][j] = 0L;
			vct_QPM[i][j] = 0L;
		}

		for (j = 0; j < NBPTLASS; j++)
		{
			vct_PT[i][j] = 0L;
		}

		det->fc = FALSE;
		det->sens = SENS_NAFF;
		det->sens_canal = SENS_NAFF;
		det->normal = 0;
		det->inverse = 0;
		det->nb_det = 0;

		det->pr[0].depart[0] = (DOUBLE) (LONG) date_sec * 1000.0;
		det->pr[1].depart[0] = (DOUBLE) (LONG) date_sec * 1000.0;
		det->pr[0].depart[1] = (DOUBLE) (LONG) date_sec * 1000.0;
		det->pr[1].depart[1] = (DOUBLE) (LONG) date_sec * 1000.0;
		det->pr[0].vitesse[0] = 0;
		det->pr[1].vitesse[0] = 0;
		det->pr[0].vitesse[1] = 0;
		det->pr[1].vitesse[1] = 0;

		vct_tempo_contre_sens[i] = 0;
	}

	for (i = 0; i < NBCAPTEURCANAL; i++)
	{
		vct_canal_capteur[i] = 0xFF;
	}

	/* on determine l'appartenance des capteurs video aux canaux LCR */
	/* pour tous les canaux configures */
	for (i = 0; i < vct_nb_canal; i++)
	{
		/* pour tous les capteurs du canal */
		num_capt = vct_canal[i].num_capt;
		nb_capt = vct_canal[i].nb_capt;

		for (j = num_capt; j < (num_capt + nb_capt); j++)
		{
			vct_canal_capteur[j] = i;
		}
	}
#endif

}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : idf_datebis_hms                                         /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : calcul une date a partir d'un nombre de secondes       /
 /-------------------------------------------------------------------------DOC*/

VOID idf_datebis_hms(seconde, pt_date)
	ULONG seconde;STRING pt_date; /* pointeur sur la date et l'heure */

{

	pt_date[H_SEC] = seconde % 60L; /* seconde courante */
	pt_date[H_MIN] = (seconde / 60L) % 60L; /* minute courante  */
	pt_date[H_HOU] = (seconde / 3600L) % 24L; /* heure courante   */

}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : idf_datebis                                            /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : calcul une date a partir d'un nombre de secondes       /
 /-------------------------------------------------------------------------DOC*/

VOID idf_datebis(seconde, pt_date)
	ULONG seconde;STRING pt_date; /* pointeur sur la date et l'heure */

{
	INT co_mois; /* compteur de mois                                       */
	ULONG nb_jours; /* nombre de jours ecoules depuis la date de reference    */
	ULONG annee; /* l'annee complete dans le siecle                        */
	INT fini;

	pt_date[H_SEC] = seconde % 60L; /* seconde courante */
	pt_date[H_MIN] = (seconde / 60L) % 60L; /* minute courante  */
	pt_date[H_HOU] = (seconde / 3600L) % 24L; /* heure courante   */

	nb_jours = (seconde / 86400L) + 1L; /* nombre de jours */

	annee = (nb_jours * 4L + 1970L * 1461L - 2) / 1461L;

	/* on positionne l'annee dans le siecle courant */
	pt_date[H_YEA] = (annee >= 2000L ? annee - 2000L : annee - 1900L);

	/* on initialise le nombre de jours avec les annees ecoulees */
	nb_jours -= (365L * (annee - 1970L) + /* nombre de jour dans les annees */
	((annee - 1970L) + 1L) / 4L); /* nombre de jours bissextiles    */

	/* pour tous les mois ecoulees depuis le debut de l'annee */

	fini = FALSE;

	co_mois = 1;

	while (!fini)
	{
		/* on enleve le nombre de jours correspondant au mois */
		switch (co_mois)
		{
		case 1: /* janvier  */
		case 3: /* mars     */
		case 5: /* mai      */
		case 7: /* juillet  */
		case 8: /* aout     */
		case 10: /* octobre  */
		case 12: /* decembre */
			if (nb_jours > 31L)
			{
				nb_jours -= 31L;
			} else
			{
				fini = TRUE;
			}
			break;

		case 4: /* avril     */
		case 6: /* juin      */
		case 9: /* septembre */
		case 11: /* novembre  */
			if (nb_jours > 30L)
			{
				nb_jours -= 30L;
			} else
			{
				fini = TRUE;
			}
			break;

		case 2: /* fevrier */
			if ((pt_date[H_YEA] % 4) == 0)
			{
				if (nb_jours > 29L)
				{
					nb_jours -= 29L;
				} else
				{
					fini = TRUE;
				}
			} else
			{
				if (nb_jours > 28L)
				{
					nb_jours -= 28L;
				} else
				{
					fini = TRUE;
				}
			}
			break;
		default:
			fini = TRUE;
			break;
		}
		if (!fini)
			co_mois++;
	}

	pt_date[H_DAT] = nb_jours;
	pt_date[H_MON] = co_mois;
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : calcul_sec                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : calcul une date en seconde depuis 01/01/70             /
 /-------------------------------------------------------------------------DOC*/

ULONG idf_sec(STRING pt_date)
{
	ULONG seconde;
	struct tm dateCour;

	dateCour.tm_year = (pt_date[H_YEA] < 90 ? (int) pt_date[H_YEA] + 100L : (int) pt_date[H_YEA]);
	dateCour.tm_mon = pt_date[H_MON] - 1;
	dateCour.tm_mday = pt_date[H_DAT];
	dateCour.tm_hour = pt_date[H_HOU];
	dateCour.tm_min = pt_date[H_MIN];
	dateCour.tm_sec = pt_date[H_SEC];
	seconde = mktime(&dateCour);
	/* on retourne les secondes */
	if (lcr_idf_debug)
	{
		printf("Les secondes calculees sont %ld\n", seconde);
	}
	return (seconde);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : test_date                                              /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : test de la date                                        /
 /-------------------------------------------------------------------------DOC*/

INT test_date(pt_date)
	STRING pt_date; /* pointeur sur la date et l'heure */

{
	INT flag_err;

	flag_err = 0;

	switch (pt_date[H_MON])
	{
	case 1: /* janvier  */
	case 3: /* mars     */
	case 5: /* mai      */
	case 7: /* juillet  */
	case 8: /* aout     */
	case 10: /* octobre  */
	case 12: /* decembre */
		if ((pt_date[H_DAT] < 1) || (pt_date[H_DAT] > 31))
		{
			flag_err = 2;
		}
		break;

	case 4: /* avril     */
	case 6: /* juin      */
	case 9: /* septembre */
	case 11: /* novembre  */
		if ((pt_date[H_DAT] < 1) || (pt_date[H_DAT] > 30))
		{
			flag_err = 2;
		}
		break;

	case 2: /* fevrier */
		if ((pt_date[H_YEA] % 4) == 0) /* annee bissextile     */
		{
			if ((pt_date[H_DAT] < 1) || (pt_date[H_DAT] > 29))
			{
				flag_err = 2;
			}
		} else /* annee non bissextile */
		{
			if ((pt_date[H_DAT] < 1) || (pt_date[H_DAT] > 28))
			{
				flag_err = 2;
			}
		}
		break;

	default:
		flag_err = 2;
		break;
	}

	if ((pt_date[H_YEA] > 99) || (pt_date[H_HOU] > 23) || (pt_date[H_MIN] > 59) || (pt_date[H_SEC] > 59))
	{
		flag_err = 2;
	}

	return (flag_err);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : idf_reinit_date                                        /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : commande DATE                                          /
 /-------------------------------------------------------------------------DOC*/

INT idf_reinit_date(STRING date, INT las)
{
	INT i;
	char date_old[SZ_DATE];
	ULONG sec;
	ULONG sec_old;
	LONG d;
	INT init_fic = FALSE;
	vct_horodate.flag = FALSE;
	/* on occupe la base en lecture */
	/* on garde l'ancienne date */
	mnlcal(date_old);
	/* on change la date */
	date[H_DAY] = 1;
	mnecal(date);
	/* on calcule la difference */
	sec_old = idf_sec(date_old);
	sec = idf_sec(date);
	d = sec - sec_old;
	if ((d >= 3960L) || (d <= -3960L))
	{
		/* on reinitialise */
		idf_reinit(FALSE);
		init_fic = TRUE;
	} else
	{

	} /* endif ((d >= 3960L) || (d <= -3      */
	/* on libere l'acces a la base en
	 * lecture et en ecriture */
	for (i = 0; i < NBPORT; i++)
	{
		/* si on est en TEDI */
		if (xdg_cf_las[i].protocole == TEDI_PROT)
		{
			vct_acces_bdd[i].ecriture = TRUE;
			vct_acces_bdd[i].lecture = TRUE;
		} /* endif (xdg_cf_las[i].protocole       */
	} /* endif(i=0                                            */
	/* --------------------------------
	 * FIN DE idf_reinit_date
	 * --------------------------------     */
	return (init_fic);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : idf_date                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : commande DATE                                          /
 /-------------------------------------------------------------------------DOC*/

VOID idf_date(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf * pt_mess)
{

	INT i; /* indice de lecture des caracteres du buffer question */
	INT flag_err = 0; /* drapeau indicateur d'erreur dans la commande        */
	INT taille; /* taille d'un bloc de configuration de voie           */
	INT val;
	char date[SZ_DATE];
	INT init_fic = FALSE;

	printDebug("idf_date : buffer <%s>\n", buffer);
	/* on suppose qu'il n'y a pas d'
	 * erreur */
	/* on se cale sur le premier car.       */
	if (buffer[position + 1] == 'T')
	{
		i = position + 2;
	} else
	{
		i = position + 4;
	} /* endif(buffer[position+1]                     */
	/* on elimine les separateurs de
	 * parametres */
	i += dv1_separateur(&buffer[i], lg_mess - i);
	/* si la commande comporte des
	 * parametres */
	if (i < lg_mess)
	{
		/*
		 *
		 * Visiblement il ne faut plus d'autorisation pour la mise a l'heure.
		 *
		 * 		if (identLireAutorisationAvecNiveau(las, 0))
 		 *
 		 */
		{
			/* on calcule la taille du parametre */
			taille = dv1_cherchefin(&buffer[i], lg_mess - i);

			if ((taille == 8) && (buffer[i + 2] == '/') && (buffer[i + 5] == '/'))
			{
				/* on calcule le jour */
				if (!dv1_atoi(&buffer[i], 2, &val))
				{
					flag_err = 1;
				} else
				{
					date[H_DAT] = val;
				}
				/* on calcule le mois */
				if (!dv1_atoi(&buffer[i + 3], 2, &val))
				{
					flag_err = 1;
				} else
				{
					date[H_MON] = val;
				} /* endif(!dv1_atoi                                      */
				/* on calcule l'annee */
				if (!dv1_atoi(&buffer[i + 6], 2, &val))
				{
					flag_err = 1;
				} else
				{
					date[H_YEA] = val;
				}
			} else if ((taille == 10) && (buffer[i + 2] == '/') && (buffer[i + 5] == '/'))
			{
				/* on calcule le jour */
				if (!dv1_atoi(&buffer[i], 2, &val))
				{
					flag_err = 1;
				} else
				{
					date[H_DAT] = val;
				}
				/* on calcule le mois */
				if (!dv1_atoi(&buffer[i + 3], 2, &val))
				{
					flag_err = 1;
				} else
				{
					date[H_MON] = val;
				} /* endif(!dv1_atoi                                      */
				/* on calcule l'annee */
				if (!dv1_atoi(&buffer[i + 6], 4, &val))
				{
					flag_err = 1;
				} else
				{
					/* Il faut soustraire 2000 a la
					 * date pour obtenir la date reelle     */
					date[H_YEA] = val - 2000;
				}
			} else
			{
				flag_err = 1;
			}
			/* on elimine les separateurs de
			 * parametres */
			i += taille;
			i += dv1_separateur(&buffer[i], lg_mess - i);
			/* on calcule la taille du parametre */
			taille = dv1_cherchefin(&buffer[i], lg_mess - i);

			if ((taille == 8) && (buffer[i + 2] == ':') && (buffer[i + 5] == ':'))
			{
				/* on calcule l'heure */
				if (!dv1_atoi(&buffer[i], 2, &val))
				{
					flag_err = 1;
				} else
				{
					date[H_HOU] = val;
				}

				/* on calcule les minutes */
				if (!dv1_atoi(&buffer[i + 3], 2, &val))
				{
					flag_err = 1;
				} else
				{
					date[H_MIN] = val;
				}

				/* on calcule les secondes */
				if (!dv1_atoi(&buffer[i + 6], 2, &val))
				{
					flag_err = 1;
				} else
				{
					date[H_SEC] = val;
				}
			} else
			{
				flag_err = 1;
			}
		}
		/*
		 * Plus d'autorisation pour la mise a l'heure.
		else
		{
			flag_err = 3;
		}
		*/
		if (flag_err == 0)
		{
			flag_err = test_date(date);
		}
		/* si la base est libre en lecture
		 * /ecriture  */
		if (flag_err == 0)
		{
			init_fic = idf_reinit_date(date, las);
			/* Il faut effectuer un reveil de la tache sequenceur!!! */
			sequenceurSignal();

		}
	} else
	{
		printDebug("idf_date : Commande sans parametre buffer %s\n", buffer);
		mnlcal(date);
	}

	if (flag_err == 4)
	{
		printDebug("idf_date : erreur 4");
		flag_err = 0;
	}

	if (flag_err == 0)
	{
		sprintf(buffer, "%02d/%02d/%02d %02d:%02d:%02d", date[H_DAT], date[H_MON], date[H_YEA], date[H_HOU],
				date[H_MIN], date[H_SEC]);
		tedi_send_bloc(las, mode, buffer, 17, 0, TRUE, pt_mess);
	} else
	{
		x01_cptr.erreur = flag_err;
		tedi_erreur(las, mode);
	}

	/* on reinitialise les fichiers de
	 * mesures. */
#ifdef RAD
	if (init_fic)
	{
		cf3_init_fic_mes(TRUE);
	}
#endif
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : idf_reinit                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : re-initialisation                                      /
 /-------------------------------------------------------------------------DOC*/

VOID idf_reinit(INT flag)
{
	INT i;
	/* si le flag est positionne... */
	if (flag)
	{
		/* emission vers la fifo de debug */
		vct_emission_debug = FALSE;
		/* status temp reel */
		vct_ST_TR = STATUS_TR_OK;
		nv_vct_ST_TR = STATUS_TR_OK;
		/* temporisation diverse, mode echo,
		 * commande VT */
		for (i = 0; i < NBPORT; i++)
		{
			rqtemp[AI_0 + i] = TP_FINI;
			rqtemp[RD_0 + i] = TP_FINI;
			rqtemp[ID_0 + i] = TP_FINI;
			rqtemp[TR_0 + i] = TP_FINI;
			rqtemp[VT_0 + i] = TP_FINI;
			idtemp[i] = 0;
			/* Aucun utilisateur par defaut */
			idTempNum[i] = -1;
			idDirectNum[i] = -1;

			vct_echo[i] = FALSE;
			mnecho((UINT8) i, FALSE);
		} /* endfor (i=0; i<NBPORT; i++)       */
		/* temporisation de l'acquisition       */
		for (i = 0; i < NBCAPTEUR; i++)
		{
			rqtemp[AC_0 + i] = TP_HS;
			rqtemp[TV_0 + i] = TP_HS;
		} /* endfor (i=0; i<NBCAPTEUR; i++)    */
		rqtemp[AC_ACQ] = 1;
	} /* endif (flag)                                         */
	/* on initialise les variables de
	 * calcul */
	ini_calcul(TRUE);
	/* On initialise le compteur
	 * antichevauchement */
#ifdef RAD
	x01_status3.cac = 0;
#endif
	vct_type_init = TYPE_INIT_INIT;
	/* on signale l'initialisation */
	vct_ST_TR |= STATUS_TR_INIT;

	/* -------------------------------
	 * FIN DE idf_reinit
	 * --------------------------------     */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : idf_init                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande INIT                          /
 /-------------------------------------------------------------------------DOC*/

VOID idf_init(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
bool reboot_b=false;
	/* on reinitialise */
	if (identLireAutorisationAvecNiveau(las, 0))
	{
		char *ptCour = (char *) &buffer[position];
		int lgCour = lg_mess - position;
		int flag_err = 0;
		ptCour += 4;
		lgCour -= 4;
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		if (strncmp(ptCour, "ZF", strlen("ZF")) == 0)
		{
			reboot_b=true;
			cf3_init_fic_tra();
			flag_err = 0;
		} else if (strncmp(ptCour, "CF", strlen("CF")) == 0)
		{
			flag_err = 2;
		} else if (strncmp(ptCour, "XF", strlen("XF")) == 0)
		{
			flag_err = 2;
		} else if (0 != lgCour)
		{
			flag_err = 2;
		}
		if (0 == flag_err)
		{
			idf_reinit(TRUE);
			lcr_init_pmv();
			tedi_ctrl(las, mode, TRUE);
			if(reboot_b)
			{
				printf("Attention reboot application");
				ksleep(3000);
#ifdef CPU432
				sysToMonitor(2);
#endif
			}
		} else
		{
			x01_cptr.erreur = flag_err;
			tedi_erreur(las, mode);
		}
	} else
	{
		x01_cptr.erreur = 3;
		tedi_erreur(las, mode);
	}
	/* ---------------------------------
	 * FIN DE idf_init
	 * --------------------------------     */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : idf_fin                                                /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 28/02/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_idf.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande FIN                           /
 /                                                                             /
 /-----------------------------------------------------------------------------/
 / DONNEES IMPORTEES : las   = numero du port sur lequel est parvenue la cde   /
 /                     mode  = mode de TEDI ds lequel doit se faire la reponse /
 /                     lg_mess = longueur du message contenant la commande     /
 /                     buffer  = chaine de caractere contenant la commande     /
 /-------------------------------------------------------------------------DOC*/
VOID idf_fin(las, mode, lg_mess, buffer, position, pt_mess)
	INT las; /* numero du las sur lequel se fait le transfert */
	INT mode; /* mode de transmission                          */
	INT lg_mess; /* nombre de caractere dans le buffer            */
	STRING buffer; /* adresse du buffer                             */
	INT position; /* position dans le buffer                       */
	struct usr_ztf *pt_mess; /* pointeur vers un message de transmission      */
{
	char *ptCour = (char *) &buffer[position + strlen("FIN")];

	int lgCour = lg_mess - position - strlen("FIN");

	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if (0 == lgCour)
	{
	identInterdire(las, TRUE);
	tedi_ctrl(las, mode, TRUE);
	}
	else
	{
	tedi_ctrl_ext(las, mode, 2);
	}

}
