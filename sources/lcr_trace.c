/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: lcr_trace.c
 * Objet	: Analyse et traitement de la commande TRACE pour l'affichage
 * 		des traces.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "portage.h"
#include "time.h"
#include "define.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "lcr_tst_sc.h"
#include "ted_prot.h"
#include "extLib.h"
#include "ficTrace.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define LCR_TRACE_NOMBRE	"N="
#define LCR_TRACE_NATURE	"U="
#define LCR_TRACE_TEMPS		"T="

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * lcr_trace
 * =========
 * Interpretation de la commande
 * LCR TRACE.
 * Les parametres sont identiques aux
 * fonctions de traitement des
 * caracteres.
 * --------------------------------	*/

int lcr_trace(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	char *ptSuiv;
	int lgSuiv;
	int nbCar = 0;
	int bloc = 0;
	int erreur = 0;
	struct tm dateDeb, dateFin;
	int traceNb = 1;
	int traceNbOk = 0;
	char traceNat = '*';
	int jourDebOk = 0;
	int jourFinOk = 0;
	int heureDebOk = 0;
	int heureFinOk = 0;
	time_t heureTmp = time(NULL);
	localtime_r(&heureTmp, &dateDeb);
	dateFin = dateDeb;

	ptCour += 5;
	lgCour -= 5;
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);

	while ((lgCour > 0) && (0 == erreur))
	{
		if (strncmp(ptCour, LCR_TRACE_NOMBRE, strlen(LCR_TRACE_NOMBRE)) == 0)
		{
			int nombre = -1;
			ptCour += strlen(LCR_TRACE_NOMBRE);
			lgCour -= strlen(LCR_TRACE_NOMBRE);
			ptCour = tst_lit_entier(ptCour, lgCour, &lgCour, &nombre);
			if (-1 != nombre)
			{
				traceNb = nombre;
				traceNbOk = 1;
				ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
			} else
			{
				erreur = 3;
			}
		} else if (strncmp(ptCour, LCR_TRACE_NATURE, strlen(LCR_TRACE_NATURE)) == 0)
		{
			ptCour += strlen(LCR_TRACE_NATURE);
			lgCour -= strlen(LCR_TRACE_NATURE);

			if ('*' == *ptCour)
			{
				traceNat = *ptCour;
				ptCour++;
				lgCour--;
				ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
			} else if ('&' == *ptCour)
			{
				ptCour++;
				traceNat = *ptCour;
				ptCour++;
				lgCour -= 2;
				ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
			} else
			{
				erreur = 3;
			}

		} else if (strncmp(ptCour, LCR_TRACE_TEMPS, strlen(LCR_TRACE_TEMPS)) == 0)
		{
			ptCour += strlen(LCR_TRACE_TEMPS);
			lgCour -= strlen(LCR_TRACE_TEMPS);
			ptSuiv = tst_lit_date(ptCour, lgCour, &lgSuiv, &dateDeb, &jourDebOk, &heureDebOk);
			if (ptCour != ptSuiv)
			{
				ptCour = ptSuiv;
				lgCour = lgSuiv;
				ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
				ptCour = tst_lit_date(ptCour, lgCour, &lgCour, &dateFin, &jourFinOk, &heureFinOk);
			}
			ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		} else
		{
			int nombre = -1;
			char *ptSuiv;
			int lgSuiv;
			int jourOk = 0;
			int heureOk = 0;
			struct tm dateLue;
			/* ----------------------------------
			 * LECTURE DEBUT ET FIN EVENTUELS
			 * --------------------------------     */
			ptSuiv = tst_lit_date(ptCour, lgCour, &lgSuiv, &dateLue, &jourOk, &heureOk);
			if ((0 != jourOk) || (0 != heureOk))
			{
				if ((0 != jourDebOk) || (0 != heureDebOk))
				{
					dateFin = dateLue;
					jourFinOk = jourOk;
					heureFinOk = heureOk;
				} else
				{
					dateDeb = dateLue;
					jourDebOk = jourOk;
					heureDebOk = heureOk;
				}
				ptCour = ptSuiv;
				lgCour = lgSuiv;
				ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
			} else
			{
				ptSuiv = tst_lit_entier(ptCour, lgCour, &lgSuiv, &nombre);
				if (-1 != nombre)
				{
					ptCour = ptSuiv;
					lgCour = lgSuiv;
					traceNb = nombre;
					traceNbOk = 1;
					ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
				} else
				{
					if ('*' == *ptCour)
					{
						traceNat = *ptCour;
						ptCour++;
						lgCour--;
						ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
					} else if ('&' == *ptCour)
					{
						ptCour++;
						traceNat = *ptCour;
						ptCour++;
						lgCour -= 2;
						ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
					} else
					{
						erreur = 2;
					}
				}
			} /* endif(0!=jourOk                                      */
		} /* endif(strncmp(ptCour,LCR_TRAC        */
	} /* endwhile                                                     */
	/* --------------------------------
	 * CONTROLE DES DATES AU BESOIN
	 * --------------------------------     */
	if ((0 != jourDebOk) || (0 != heureDebOk))
	{
		if (-1 == mktime(&dateDeb))
		{
			erreur = 3;
		} else
		{
			printDebug("L'heure de debut est %s\n", asctime(&dateDeb));
			if (0 == traceNbOk)
			{
				traceNb = 0;
			}
			if ((0 != jourFinOk) || (0 != heureFinOk))
			{
				if (-1 == mktime(&dateFin))
				{
					erreur = 3;
				} else
				{
					printDebug("L'heure de fin est %s\n", asctime(&dateFin));
				} /* endif(-1==mktime(&dateFin)) */
			} /* endif((0!=jourFinOk)||(0!=heureF */
		} /* endif(-1==mktime(&dateDeb)) */
	} /* endif((0!=jourDebOk)||(0!=heureD */
	/* --------------------------------
	 * TRAITEMENT DE LA COMMANDE
	 * -------------------------------- */
	if (0 == erreur)
	{
		ficTraceImprimer(las, mode, buffer, &nbCar, &bloc, FALSE, pt_mess, traceNb, traceNat, jourDebOk || heureDebOk,
				&dateDeb, jourFinOk || heureFinOk, &dateFin);
		tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess, "");
	} else
	{
		tedi_ctrl_ext(las, mode, erreur);
	}
	return (0);
}
