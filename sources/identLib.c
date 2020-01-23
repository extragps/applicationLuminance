/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/12/00
 * Fichier	: identLib.c
 * Objet	: Gestion des identifiants
 * 			Alors que les identifiants etaient geres jusqu'a present au fil
 * 			du code, le traitement de ceux ci est maitenant realise dans ce
 * 			fichier.
 * 			Attention, pour les besoins de Sirius, la configuration standard
 * 			des identifiants ne correspond pas e une mise e zero.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * 01b,26Oct05,xag  ajout d'un test sur la liaison -1 qui est l'ancrage interne
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#include "standard.h"
#include "mon_debug.h"
#include "define.h"
#include "extLib.h"
#include "vct_str.h"                   /* Pour la manipulation des donnees */
#include "mon_def.h"
#include "mon_str.h"
#include "mon_ext.h"
#include "x01_vcth.h"
#include "etaSyst.h"

#include "io/iolib.h"
/* brute.                                                       */

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif
/* Pour l'heure, la simulation est
 * limitee e 200 vehicules sur dix
 * voies....    */
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
 * identMettreAZero
 * ================
 * Mise e zero des identifiants.
 * --------------------------------	*/

void identMettreAZero(void)
{
	vct_nb_acces = 0;
}

/* -----------------------------
 * identLireIdent
 * ==============
 * Recherche dans la liste des
 * identificateurs celui dont
 * le mot de passe correspond
 * au parametre et retourne
 * un pointeur sur l'idendificateur
 * Entree :
 * - le mot de passe e rechercher,
 * Valeur de retour :
 * - l'identificaterur ou NULL
 *   si celui ci n'a pas ete
 *   trouve.
 * ----------------------------	*/

char *identLireIdent(char *password)
{
	char *ident = NULL;
	int indice = 0;
	while (indice < vct_nb_acces)
	{
		if (strcmp(password, (char *) vct_acces[indice].password) == 0)
		{
			ident = (char *) &vct_acces[indice].identifiant[0];
			break;
		}
		indice++;
	} /* endwhile(indice<0)                                   */
	/* ------------------------------------
	 * FIN DE identTesterPassword
	 * ------------------------------------ */
	return ident;
}

/* -----------------------------
 * identTesterPassword
 * ===================
 * Verifie dans la liste des passwords
 * la presence du password passe en
 * parametre.
 * ----------------------------	*/

int identTesterPassword(char *password)
{
	int retour = -1;
	int indice = 0;
	while ((indice < vct_nb_acces) && (retour == -1))
	{
		if (strcmp(password, (char *) vct_acces[indice].password) == 0)
		{
			retour = indice;
		}
		indice++;
	} /* endwhile(indice<0)                                   */
	/* ------------------------------------
	 * FIN DE identTesterPassword
	 * ------------------------------------ */
	return retour;
}

/* -----------------------------
 * identTesterIdentEtPassword
 * ===================
 * Verifie dans la liste des passwords
 * la presence du password passe en
 * parametre.
 * ----------------------------	*/

int identTesterIdentEtPassword(char *ident, char *password)
{
	int retour = -1;
	int indice = 0;
	printDebug("Test de ident : %s et passwd : %s...\n", ident, password);
	while ((indice < vct_nb_acces) && (retour == -1))
	{
		printDebug("---%d de ident : %s et passwd : %s...\n", indice + 1, vct_acces[indice].identifiant,
				vct_acces[indice].password);
		if ((strcmp(password, vct_acces[indice].password) == 0) && (strcmp(ident, vct_acces[indice].identifiant) == 0))
		{
			retour = indice;
		}
		indice++;
	} /* endwhile(indice<0)                                   */
	/* Traces pour le debug.                                */
	if (retour != -1)
	{
		printDebug("C'est trouve\n");
	}
	/* ------------------------------------
	 * FIN DE identTesterPassword
	 * ------------------------------------ */
	return retour;
}

/* --------------------------------
 * identAjouter
 * ============
 * Ajout d'un nouvel identifiant.
 * La fonction retourne FALSE si
 * il n'y a plus d'identification
 * libre ou si une erreur est inter
 * venue.
 * Si on trouve un couple d'identi-
 * fication avec deje le numero
 * d'identification, on l'utilise.
 * --------------------------------	*/

BOOL identAjouter(char *ident, char *passwd, int numUtil)
{
	BOOL retour = FALSE;
	int indice = 0;
	/* Recherche d'un couple ayant deje
	 * le numero d'utilisateur. */
	while ((indice < vct_nb_acces) && (retour == FALSE))
	{
		if (vct_acces[indice].num_util == numUtil)
		{
			if ((strlen(ident) < 9) && (strlen(passwd) < 9))
			{
				strcpy(vct_acces[indice].identifiant, ident);
				strcpy(vct_acces[indice].password, passwd);
				vct_acces[indice].format = FALSE;
				retour = TRUE;
			}
		}

		indice++;
	}
	/* Si le numero d'identifiant
	 * n'a pas ete trouve, on alloue
	 * un nouvel element.   */
	if (retour == FALSE)
	{
		if ((vct_nb_acces < NBID) || (strlen(ident) < 9) || (strlen(passwd) < 9))
		{
			strcpy(vct_acces[vct_nb_acces].identifiant, ident);
			strcpy(vct_acces[vct_nb_acces].password, passwd);
			vct_acces[vct_nb_acces].num_util = numUtil;
			vct_acces[vct_nb_acces].format = FALSE;
			vct_nb_acces++;
			retour = TRUE;
		}
	}
	/* ---------------------------------
	 * FIN DE identAjouter
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * identStandard
 * =============
 * Configuration standard des iden-
 * tifiants pour SIRIUS.
 * --------------------------------	*/

void identStandard(void)
{
	identMettreAZero();
	identAjouter("CS", "CS", 1);
	identAjouter("CIGT", "CIGT", 2);
	identAjouter("MAINT", "MAINT", 3);
}

/* --------------------------------
 * identAutoriser
 * ==============
 * Autorisation .
 * --------------------------------	*/

void identAutoriser(int las, int ident, BOOL direct)
{
	if (-1 != las)
	{
		if (TRUE == direct)
		{
			printDebug("Autorisation en mode direct sur la liaison %d\n", las);
			rqtemp[ID_0 + las] = TEMP_ID;
			idDirectNum[las] = ident;
		} else
		{
			printDebug("Autorisation en mode datagramme sur la liaison %d\n", las);
			idtemp[las] = 1;
			idTempNum[las] = ident;
		}
	}
}

/* --------------------------------
 * identInterdire
 * ==============
 * Lever des autorisation sur la
 * liaison. N'a pas de signification
 * en mode datagramme.
 * --------------------------------	*/

void identInterdire(int las, BOOL direct)
{
	if (-1 != las)
	{
		if (TRUE == direct)
		{
			printDebug("Interdiction en mode direct sur la liaison %d\n", las);
			rqtemp[ID_0 + las] = TP_FINI;
			idDirectNum[las] = -1;
		} else
		{
			idtemp[las] = 0;
			idTempNum[las] = -1;
			printDebug("Interdiction en mode datagramme sur la liaison %d\n", las);
		}
	}
}

/* ---------------------------------
 * identLireUtilisateur
 * ====================
 * Lecture de l'utilisateur autorise
 * --------------------------------	*/

int identLireUtilisateur(int las)
{
	int utilisateur = -1;
	/* Identification normale               */
	if (1 == idtemp[las])
	{
		utilisateur = idTempNum[las];
	} else if (rqtemp[ID_0 + las] > TP_FINI)
	{
		utilisateur = idDirectNum[las];
	}
	/* --------------------------------
	 * FIN DE identLireUtilisateur
	 * --------------------------------     */
	return utilisateur;
}

/* --------------------------------
 * identLireAutorisation
 * ======================
 * Lecture de l'autorisation.
 * --------------------------------	*/

BOOL identLireAutorisation(int las)
{
	BOOL retour = FALSE;
	/* Identification temporaire    */
	if(ioIsUnlocked()==true)
	{
		retour = TRUE;
	} else if (1 == idtemp[las])
	{
		retour = TRUE;
	}
	/* Identification normale               */
	else if (rqtemp[ID_0 + las] > TP_FINI)
	{
		retour = TRUE;
	}
	/* ---------------------------------
	 * FIN DE identLireAutorisation
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * identLireAutorisationAvecNiveau
 * ===============================
 * Lecture de l'autorisation.
 * --------------------------------	*/

BOOL identLireAutorisationAvecNiveau(int las, int niveau)
{
	/* ---------------------------------
	 * FIN DE identLireAutorisationAvecNi
	 * --------------------------------     */
	return identLireAutorisation(las);
}
