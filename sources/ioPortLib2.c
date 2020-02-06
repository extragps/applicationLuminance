
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 17/01/06
 * Fichier	: ioPortLib2.c
 * Objet	:
 * 		Librairie qui a pour objet de lire :
 * 		- les interruptions (pour discriminer les cartes sur le bus),
 * 		- les entrees (les switchs de la carte),
 * 		- les autres entrees : BP0, BP1, alim detecteur,
 * 		- ?? L'adresse de la carte WorldFip (sur la carte worldFip). Cette
 * 		fonction sera peut etre deportee ailleurs.
 * 		On peut aussi ecrire les infos suivantes :
 * 		- la led en face avant,
 * 		- l'alimentation des detecteurs deportes,
 * 		- .....
 * 		Les informations sont ecrites et lues sur le MACH Lattice en 16 bits.
 * 		La demarche est la suivante :
 * 		- ecriture du registre de sortie,
 * 		- lecture des interruptions,
 * 		- lecture des switchs,
 * 		- lecture des autres infos.
 *		En resume, une ecriture suivie de quatre lectures.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,13Dec00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/
 
#include <fcntl.h>
#include <unistd.h>
#include "standard.h"
#include "define.h"
#include "portage.h"
#include "config.h"
#include "stdio.h"
#include "ioPortLib2.h"
#include "mon_debug.h"
#ifdef CPU432
#include "config.h"
#endif


/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif


UINT16 ioPortSorties=0;
UINT16 ioPortIts;
UINT16 ioPortSwitchs;
UINT16 ioPortInfos;
volatile UINT16 *ioPortAdresse=(UINT16 *)LATTICE_BASE;

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static UINT16 ioPortBits[IO_NB_BITS]={ IO_BIT0, IO_BIT1, IO_BIT2, IO_BIT3,
		IO_BIT4, IO_BIT5, IO_BIT6, IO_BIT7};

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * ioPortLecture
 * =============
 * Lecture de tous les registres.
 * --------------------------------	*/

void ioPortLecture(void)
{
	*ioPortAdresse=ioPortSorties;
	ioPortIts=*ioPortAdresse;
	ioPortInfos=*ioPortAdresse;
	ioPortSwitchs=*ioPortAdresse;
}


/* --------------------------------
 * ioPortResetDetecteur
 * ===================
 * Reset du detecteur.
 * --------------------------------	*/

void ioPortResetDetecteur (void)
{
  printDebug ("Reset du detecteur\n");
  /* ---------------------------------
   * FIN DE ioPortResetDetecteur
   * --------------------------------     */
}

/* --------------------------------
 * ioPortLireAdresse
 * ===================
 * Lecture de l'info adresse WorldFip sur
 * le port d'entree sortie du micro.
 * Valeur de retour :
 * -	TRUE si le bit est leve.
 * --------------------------------	*/

/* --------------------------------
 * ioPortLireAdresse
 * ===================
 * Lecture de l'info adresse WorldFip sur
 * le port d'entree sortie du micro.
 * Valeur de retour :
 * -	TRUE si le bit est leve.
 * --------------------------------	*/

int ioPortLireAdresse( char *adresse )
{
	int retour = 0;
#ifdef CPU432
	int desc = open( "/dev/fip", O_RDWR );
	if( -1 != desc ) {
		*adresse = ~( *( (char*) ( FIP_BASE + 0x20 ) ) );
		printf( "ioPortLireAdresse, adresse de la carte %d\n", (int) ( *adresse ) );
		retour = 1;
		close( desc );
	}
#endif
	return retour;
}

/* --------------------------------
 * ioPortLireParafoudre
 * ===================
 * Lecture de l'info parafoudre sur
 * le port d'entree sortie du micro.
 * Valeur de retour :
 * -	TRUE si le bit est leve.
 * --------------------------------	*/

BOOL ioPortLireParafoudre (void)
{
  BOOL parafoudre = TRUE;
  return (parafoudre);
}

/* --------------------------------
 * ioPortLireAlimDetecteur
 * =======================
 * Lecture de l'info alimentation des
 * detecteurs deportes.
 * Valeur de retour :
 * -	TRUE si le bit est leve.
 * --------------------------------	*/

BOOL ioPortLireAlimDetecteur (void)
{
  BOOL alimDetecteur = FALSE;
  return (alimDetecteur);
}

/* --------------------------------
 * ioPortLireTerminal
 * ==================
 * -------------------------------- */

BOOL ioPortLireTerminal (void)
{
  BOOL terminalConnecte = FALSE;
  return (terminalConnecte);
}



/* --------------------------------
 * ioPortEcrirAlimDetecteur
 * =======================
 * Activation ou arret de l'alimentation
 * des detecteurs.
 * Entree :
 * -	TRUE si l'alim doit etre positionnee.
 * --------------------------------	*/

void ioPortEcrireAlimDetecteur (BOOL alimenter)
{
  printDebug ("Ecrire alim detecteur %d\n", alimenter);
}

/* -----------------------------------
 * ioPortLireFlags
 * ===============
 * Lecture des flags suivant l'ancienne
 * methode.
 * --------------------------------	*/

UINT8 ioPortLireFlags(BOOL refresh)
{
int indice;
UINT8 switchs=0;
	if(refresh)
	{
		ioPortLecture();
	}
	for(indice=0;indice<IO_NB_BITS;indice++)
	{
		if(0==(ioPortBits[indice]&ioPortSwitchs))
		{
			switchs|=(1<<indice);
		}
	}
	return switchs;
}


