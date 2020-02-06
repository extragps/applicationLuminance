/*
 * mon_time.c
 *
 *  Created on: 13 oct. 2010
 *      Author: xavier
 */
#include <pipe.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "standard.h"                  /* redefinition des types                         */
#include "semaphore.h"
#include "etaSyst.h"

#include "xdg_str.h"
#include "xdg_var.h"
#include "mon_inc.h"
#include "define.h"        /* definition des constantes                      */
#include "xdg_def.h"       /* definition des structures globales             */
#include "x01_str.h"       /* definition des structures globales             */
#include "x01_var.h"     /* definition des variables globales               */
#include "vct_str.h"       /* definition des structures globales             */
#include "x01_vcth.h"     /* definition des variables globales               */

#include "x01_vcth.h"

#include "mon_str.h"
#include "standard.h"      /* redefinition des types */
#include "define.h"
#include "mon_def.h"
#include "mon_ext.h"
#include "mon_pro.h"
#include"lcr_idfh.h"
#include "ypc_las.h"
#include "tac_ctrl.h"

#include "mon_debug.h"
#include "mon_def.h"
#include "semLib.h"
#include "rec_main.h"

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/**********************************************************/
/*                                                        */
/* MNLCAL : lecture du calendrier                         */
/*                                                        */
/**********************************************************/

#ifdef CPU432

#include <stdlib.h>

#define timegm my_timegm

static time_t my_timegm(struct tm *tm)
{
	time_t ret;
	char *tz;

	tz = getenv("TZ");
	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	if (tz)
		setenv("TZ", tz, 1);
	else
		unsetenv("TZ");
	tzset();
	return ret;
}
#endif

static long mncalDelta = 0;

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

VOID mon_datebis(ULONG seconde, STRING pt_date)
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
			}
			else
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
			}
			else
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
				}
				else
				{
					fini = TRUE;
				}
			}
			else
			{
				if (nb_jours > 28L)
				{
					nb_jours -= 28L;
				}
				else
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

ULONG mncalMktime(struct tm *dateCour)
{
	return timegm(dateCour);
}

VOID mnlcal(STRING date)
{
	struct timeval heure;
	gettimeofday(&heure, NULL);
	heure.tv_sec += mncalDelta;

	mon_datebis(heure.tv_sec, date);
	return;
}

ULONG mnlcalSec(void)
{
	struct timeval heure;
	gettimeofday(&heure, NULL);
	return heure.tv_sec + mncalDelta;
}

int mnlcalTime(struct timespec *tp)
{
	struct timeval heure;
	int retour = -1;
	if (NULL != tp)
	{
		retour = gettimeofday(&heure, NULL);
		if (-1 != retour)
		{
			tp->tv_sec = heure.tv_sec + mncalDelta;
			tp->tv_nsec = heure.tv_usec * 1000;
		}
	}
	return retour;
}

ULONG mon_sec(STRING pt_date)
{
	ULONG seconde;
	struct tm dateCour;

	dateCour.tm_year = (pt_date[H_YEA] < 90 ? (int) pt_date[H_YEA] + 100L : (int) pt_date[H_YEA]);
	dateCour.tm_mon = pt_date[H_MON] - 1;
	dateCour.tm_mday = pt_date[H_DAT];
	dateCour.tm_hour = pt_date[H_HOU];
	dateCour.tm_min = pt_date[H_MIN];
	dateCour.tm_sec = pt_date[H_SEC];
	seconde = mncalMktime(&dateCour);
	/* on retourne les secondes */
	return (seconde);
}

/******************************************************************************
 *
 * MNECAL : ecriture du calendrier
 *                     (acces utilisateur)
 *
 * appel  : mnecal (pt_date)
 *
 * avec   : pt_date = adresse de la structure date
 *
 *******************************************************************************/

VOID mnecal(STRING st1)
{
	struct timeval hBis;
	int retour = -1;

	/* On recupere le nombre de millis */
	retour = gettimeofday(&hBis, NULL);
	if (-1 != retour)
	{
		/* Calcul du nombre de secondes.	*/
#ifdef CPU432
		hBis.tv_sec = mon_sec(st1);
		if (-1 == settimeofday(&hBis, NULL))
		{
			printf("Probleme de mise a l'heure %s\n", strerror(errno));
		}
#else
		{
			unsigned long nouvelleHeure=mon_sec(st1);
			if(nouvelleHeure>hBis.tv_sec)
			{
				mncalDelta=nouvelleHeure-hBis.tv_sec;
			}
			else
			{
				mncalDelta=-(hBis.tv_sec-nouvelleHeure);
			}
		}

#endif
	}
	else
	{
		printf("Probleme de lecture de l'heure %s\n", strerror(errno));

	}

	/* --------------------------------
	 * FIN DE mnecal
	 * --------------------------------	*/
	return;
}
