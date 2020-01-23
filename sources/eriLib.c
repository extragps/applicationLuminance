/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 10 Mai 2001
 * Fichier	: eriLib.c
 * Objet	: Le fichier permet de traiter les erieurs ERI de la station.
 * 			Pour l'heure le nombre de ces erieurs est pour l'heure limite.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,10May01,xag  creation.
 * 01b,11Jun01,xag  les erreurs ne sont plus conservees sur coupure.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <stdio.h>
#include "portage.h"
#include "define.h"
#include "vct_str.h"
#include "mon_def.h"
#include "mon_debug.h"
#include "mon_str.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "eriLib.h"
#include "configurationSram.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static T_eriList eriList;
static T_eriList *eri = &eriList;

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

static BOOL eriChercherIndex(char, char, int *);
static BOOL eriChercherModuleIndex(char, char, unsigned char, unsigned char, int *);
static BOOL _eriChercherModuleIndexVal(char grav, char nature, unsigned char mod, unsigned char cais, int val,
		int *index);

/* --------------------------------
 * eriInit
 * =======
 * Initialisation du pointeur sur
 * la zone ou l'on stocke les
 * erreur ERI.
 * --------------------------------	*/

void eriInit(void)
{
	/* TODO : Charger les erreurs de la flash. */
	eri->nbData = 0;
	//	configSramLireEri(eri);
	/*  eri=adresse;
	 eri->nbData=0; */
	/* ---------------------------------
	 * FIN DE eriInit
	 * --------------------------------     */
}

/*
 * _eriSauver
 * ==========
 * Sauvegarde en memoire flash des erreurs ERI...
 */
//
//static void _eriSauver(void)
//{
//	/* Sauvegarde en flash des erreurs sur modification de la table... */
////	configSramEcrireEri(eri);
//}
/* --------------------------------
 * eriPrint
 * ========
 * --------------------------------	*/

static void eriPrint(int index)
{
	T_eriData *data = &eri->data[index];
	printDebug("Ajout de la donnee %d et grav %d et erreur %d", index, data->grav, data->nature);
	if (0 != data->module)
	{
		printDebug("/%d.%d", data->module, data->caisson);
		if (0xFF != data->valeur)
		{
			printDebug(".%d", data->valeur);
		}
	}
	printDebug("\n");
}

static void _eriSupprimerIndex(int eriCour)
{
	eri->nbData--;
	for (; eriCour < eri->nbData; eriCour++)
	{
		eri->data[eriCour] = eri->data[eriCour + 1];
	}
	//     if(force)
	//     {
	//    	 _eriSauver();
	//     }
}

/* --------------------------------
 * eriAjouterModule
 * ================
 * Ajout d'une nouvelle erreur avec
 * un numero de module et de caisson e la
 * liste des erreurs.
 *
 * Entree :
 * -	gravite de l'erieur,
 * -	nature de l'erieur.
 * - 	module, caisson et valeur
 * --------------------------------	*/

void eriAjouterModule(char grav, char nature, unsigned char mod, unsigned char cais, int val)
{
	eriSupprimerModuleVal(grav, nature, mod, cais,val);
	/* Ajout de la nouvelle erreur          */
	if (eri->nbData < ERI_MAX_DATA)
	{
		eri->data[eri->nbData].grav = grav;
		eri->data[eri->nbData].nature = nature;
		eri->data[eri->nbData].module = mod;
		eri->data[eri->nbData].caisson = cais;
		eri->data[eri->nbData].valeur = val;
		eriPrint(eri->nbData);
		eri->nbData++;
	}
	/* --------------------------------
	 * FIN DE eriAjouter
	 * --------------------------------     */
}

/* --------------------------------
 * eriSupprimerModule
 * ================
 * --------------------------------	*/

void eriSupprimerModuleVal(char grav, char nature, unsigned char mod, unsigned char cais, int val)
{
	int eriCour = eri->nbData;

	int trouve = _eriChercherModuleIndexVal(grav, nature, mod, cais, val, &eriCour);
	/* --------------------------------
	 * SUPPRESSION EVENTUELLE
	 * --------------------------------     */
	if (trouve)
	{
		_eriSupprimerIndex(eriCour);
	}
	/* --------------------------------
	 * FIN DE eriAjouter
	 * --------------------------------     */
}

void eriSupprimerModule(char grav, char nature, unsigned char mod, unsigned char cais)
{
	int eriCour = eri->nbData;

	int trouve = eriChercherModuleIndex(grav, nature, mod, cais, &eriCour);
	/* --------------------------------
	 * SUPPRESSION EVENTUELLE
	 * --------------------------------     */
	if (trouve)
	{
		_eriSupprimerIndex(eriCour);
	}
	/* --------------------------------
	 * FIN DE eriAjouter
	 * --------------------------------     */
}

/* --------------------------------
 * eriAjouter
 * ==========
 * Ajout d'une nouvelle erieur e la
 * liste des erieurs.
 * Entree :
 * -	gravite de l'erieur,
 * -	nature de l'erieur.
 * --------------------------------	*/

void eriAjouter(char grav, char nature)
{
	eriSupprimer(grav, nature);
	if (eri->nbData < ERI_MAX_DATA)
	{
		eri->data[eri->nbData].grav = grav;
		eri->data[eri->nbData].nature = nature;
		eri->data[eri->nbData].module = 0;
		eriPrint(eri->nbData);
		eri->nbData++;
	}
	//  _eriSauver();
}

/* --------------------------------
 * eriChercher
 * ==========
 * Recherche d'une erreur dans la liste
 * --------------------------------	*/

BOOL eriChercher(char grav, char nature)
{
	int eriCour;
	return eriChercherIndex(grav, nature, &eriCour);

}

/* --------------------------------
 * eriChercherIndex
 * ================
 * Recherche d'une erreur dans la liste
 * --------------------------------	*/

static BOOL eriChercherIndex(char grav, char nature, int *index)
{
	int eriCour;
	BOOL trouve = FALSE;

	for (eriCour = 0; eriCour < eri->nbData; eriCour++)
	{
		if ((grav == eri->data[eriCour].grav) && (nature == eri->data[eriCour].nature))
		{
			eriPrint(eriCour);
			*index = eriCour;
			trouve = TRUE;
			break;
		} /* endif((grav== */
	} /* endfor(eriCour */
	/* --------------------------------
	 * FIN DE eriChercher
	 * --------------------------------     */
	return trouve;

}

/* --------------------------------
 * eriChercherModuleIndex
 * ================
 * Recherche d'une erreur dans la liste
 * pour un module et un index donne.
 * --------------------------------	*/

static BOOL eriChercherModuleIndex(char grav, char nature, unsigned char mod, unsigned char cais, int *index)
{
	int eriCour;
	BOOL trouve = FALSE;

	for (eriCour = 0; eriCour < eri->nbData; eriCour++)
	{

		if ((grav == eri->data[eriCour].grav) && (nature == eri->data[eriCour].nature) && (mod
				== eri->data[eriCour].module) && (cais == eri->data[eriCour].caisson))
		{
			eriPrint(eriCour);
			*index = eriCour;
			trouve = TRUE;
			break;
		} /* endif((grav== */
	} /* endfor(eriCour */
	/* --------------------------------
	 * FIN DE eriChercher
	 * --------------------------------     */
	return trouve;

}

/* --------------------------------
 * eriChercherModuleIndex
 * ================
 * Recherche d'une erreur dans la liste
 * pour un module et un index donne.
 * --------------------------------	*/

static BOOL _eriChercherModuleIndexVal(char grav, char nature, unsigned char mod, unsigned char cais, int val,
		int *index)
{
	int eriCour;
	BOOL trouve = FALSE;

	for (eriCour = 0; eriCour < eri->nbData; eriCour++)
	{

		if ((grav == eri->data[eriCour].grav) && (nature == eri->data[eriCour].nature) && (mod
				== eri->data[eriCour].module) && (val == eri->data[eriCour].valeur) && (cais
				== eri->data[eriCour].caisson))
		{
			eriPrint(eriCour);
			*index = eriCour;
			trouve = TRUE;
			break;
		} /* endif((grav== */
	} /* endfor(eriCour */
	/* --------------------------------
	 * FIN DE eriChercher
	 * --------------------------------     */
	return trouve;

}

/* --------------------------------
 * eriSupprimer
 * ==========
 * suppression e la
 * liste des erreurs si elle existe.
 * Entree :
 * -	gravite de l'erieur,
 * -	nature de l'erieur.
 * --------------------------------	*/

void eriSupprimer(char grav, char nature)
{
	int eriCour;

	/* --------------------------------
	 * SUPPRESSION EVENTUELLE
	 * --------------------------------     */
	if (eriChercherIndex(grav, nature, &eriCour) == TRUE)
	{
		_eriSupprimerIndex(eriCour);
	}
	/* --------------------------------
	 * FIN DE eriSupprimer
	 * --------------------------------     */
}

/* --------------------------------
 * eriLireNbErreur
 * ===============
 * --------------------------------	*/

int eriLireNbErreur(void)
{
	return eri->nbData;
}

/* --------------------------------
 * eriLireErreur
 * =============
 * Lecture des donnees relatives a
 * une erreur.
 * Valeur de retour :
 * - la fonction retourne -1 si
 * 	 le nombre communique n'est
 * 	 pas correct.
 * --------------------------------	*/

int eriLireErreur(int numero, char *grav, char *nature)
{
	int retour = -1;
	if ((numero >= 0) && (numero < eri->nbData))
	{
		retour = 0;
		*grav = eri->data[numero].grav;
		*nature = eri->data[numero].nature;
	}
	return retour;
}

int eriLireErreurModule(int numero, char *mod, char *cais, int *val)
{
	int retour = -1;
	if ((numero >= 0) && (numero < eri->nbData))
	{
		retour = 0;
		*mod = eri->data[numero].module;
		*cais = eri->data[numero].caisson;
		*val = eri->data[numero].valeur;
	}
	return retour;

}

/* ********************************
 * FIN DE eriLib.c
 * ********************************	*/
