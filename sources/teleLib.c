/*
 * teleLib.c
 * ===================
 * Objet	: Telechargement d'un fichier contenant systeme et application
 * 		sur la carte e partir des donnees envoyees sur WorldFip (ou un
 * 		autre media pour l'instant).
 *
 *  Created on: 20 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: teleLib.c,v $
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.6  2010/08/05 14:38:18  xgaillard
 * *** empty log message ***
 *
 * Revision 1.5  2009/08/24 08:24:52  xgaillard
 * renomage de moduleLire par versionLire
 *
 * Revision 1.4  2008/09/29 07:58:52  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.3  2008/09/22 07:53:43  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/09/12 14:57:10  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <standard.h>
#include <stdio.h>
#include <time.h>
#include "amd.h"
#include "standard.h"
#include "xdg_str.h"
#include "lcr_util.h"
#include "anaSys.h"
#include "teleLib.h"
#include "mon_debug.h"
#include "tac_conf.h"
#ifdef RAD
#include "teleDetFicLib.h"
#endif
#include "etaSyst.h"
#include "portage.h"
#include "supRun.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define TELE_MODETEL	"MODETEL"
#define TELE_TESTFIP	"TESTFIP"
#define TELE_TELFIC		"TELFIC"
#define TELE_MODEEXP	"MODEEXP"
#define TELE_LISTEVERS	"LISTEVERS"
#define TELE_SYSTEME	"SYSTEME"
#define TELE_VERSION	"VERSION"
#define TELE_FIC		"FIC="
#define TELE_NUMERO		"NUM="
#define TELE_NUM_BLOC	"NO="
#define TELE_NB_BLOCS	"NB="
#define TELE_TAILLE		"ND="
#define TELE_DATA		"DATA="
#define TELE_DETTEL		"DETTEL"
#define TELE_DETFIC		"DETFIC"
#define TELE_DETFLASH		"DETFLASH"
#define TELE_DETPROG 	"DETPROG"
#define TELE_DETETAT 	"DETETAT"
#define TELE_DETVERS 	"DETVERS"
#define TELE_DETEXP  	"DETEXP"
#define TELE_DETFORCE  	"FORCE"

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ----------------------------
 * T_teleFic
 * ==========
 * Le fichier contient les infos
 * necessaire au telechargement d'un
 * fichier.
 * --------------------------------	*/

typedef struct T_teleFic
{
	char nom[256];
	int blocCour;
	int nbBlocs;
	int bin;
	int tailleZone;
	int tailleTampon;
	int adCour;
	FILE *fd_pt;
	int sectCouri;
	char *zone;
	char *tampon;
	int nbCarTampon;
	int nbRetry;
	int ficSel;
} T_teleFic;

/* ********************************
 * VARIABLES LOCALES
 * ********************************	*/

struct T_teleFic *teleFichier = NULL;

/* ********************************
 * PROTOTYPE DES FONCTIONS LOCALES
 * ********************************	*/

static int teleLireBloc(int, char *, int, T_lcr_util_buffer *);
static int teleChangerModeTel(int, char *, int, T_lcr_util_buffer *);
static int teleChangerModeExp(int, char *, int, T_lcr_util_buffer *);
static char *teleLireValeurShort(char *, int, int *, unsigned short *);
static char *teleLireValeur(char *, int, int *, int *);
static int teleFicCalculerChecksum(T_teleFic *);
int teleDecoderBloc(T_teleFic * fichier, char *bloc, int taille, int *notFin);
static int teleFicFlasherZone(T_teleFic * fichier);
#ifdef RAD
static int teleDetFic (int liaison, char *bloc, int taille,
		T_lcr_util_buffer *reponse,T_teleDetFic *desc);
static int teleDetFicLireFlash(char *bloc,int taille,
		T_lcr_util_buffer * reponse, T_teleDetFic * desc);
#endif
static int teleFicEffacerZone(T_teleFic * fichier);

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * teleFicCreer
 * ============
 * Creation de la structure utilisee
 * pour le telechargement de fichier
 * Entree :
 * - adresse de la zone dans laquelle
 *   doivent etre stockees les donnees
 *   ou NULL si la zone doit etre allouee
 * - taille de la zone dans laquelle le
 *   programme doit etre recopie,
 * - taille en octet du tampon dans lequel
 *   est place le reste e decoder (entre
 *   deux appels de la fonction de decodage).
 * Retour :
 * - la structure contenant l'ensemble des
 *   infos utilisees pour le telechargement.
 * --------------------------------	*/

T_teleFic *teleFicCreer(int tailleTampon)
{
	T_teleFic *fichier = (T_teleFic *) kmmAlloc(NULL,sizeof(T_teleFic));
	/* --------------------------------
	 * SI ALLOCATION OK ALORS ON CONTINUE
	 * --------------------------------     */
	if (fichier != NULL)
	{
		fichier->nom[0] = 0;
		fichier->bin = 1; /* Par defaut mode binaire                      */
		fichier->blocCour = 1;
		fichier->nbBlocs = 0;
		fichier->nbRetry = 0;
		fichier->tailleTampon = tailleTampon;
		fichier->nbCarTampon = 0;
		fichier->tampon = (char *) kmmAlloc(NULL,tailleTampon);
		fichier->adCour = 0;
		/* Attention, en theorie, dans un
		 * avenir proche, le numero de
		 * secteur sera determine e partir
		 * du nom du fichier.   */
		fichier->zone = NULL;
		fichier->fd_pt=NULL;
		fichier->tailleZone=4096;
		/* Onverture du fichier.... */
		/* La taille de la zone doit corres-
		 * pondre e la taille d'un secteur
		 * de la flash. */
		if (fichier->tampon != NULL)
		{
			fichier->zone = (char *) kmmAlloc(NULL,fichier->tailleZone);
			if (fichier->zone == NULL)
			{
				kmmFree(NULL,(void *) fichier->tampon);
				kmmFree(NULL,(void *) fichier);
				fichier = NULL;
			} else
			{
				memset(fichier->zone, 0, fichier->tailleZone);
			} /* endif(fichier->tampon==NULL          */
		} else
		{
			kmmFree(NULL,(void *) fichier);
			fichier = NULL;
		} /* endif(fichier->zone                          */
	} /* endif(fichier!=NULL                          */
	/* --------------------------------
	 * FIN DE teleFicCreer
	 * --------------------------------     */
	return fichier;
}

/* ----------------------------------
 * teleFicSetNom
 * =============
 * Ecriture du nom du fichier et
 * recuperation du numero de secteur
 * e utiliser pour la programmation.
 * --------------------------------	*/

static int teleFicSetNom(T_teleFic * fichier, char *nom)
{
	int retour = -1;
	if (NULL != fichier)
	{
		if (NULL != nom)
		{
			if (0 != nom[0])
			{
				T_tacConfFichiers confFichier;
				tac_conf_cfg_lire_fichier(&confFichier);
				strcpy(fichier->nom, nom);
				tac_conf_cfg_ajouter_fichier(&confFichier, nom);
				/* Ouvrir le fichier.... */
				{
				char nomFichier[256];
					snprintf(nomFichier,256,"%s/%s",TAC_CONF_REP_VERSIONS,nom);
					fichier->fd_pt=fopen(nomFichier,"w");
					if(NULL==fichier->fd_pt)
					{
						printDebug("teleFicSetNom : Erreur d'ouverture du fichier %s\n",nomFichier);
					}
				}
				fichier->ficSel = tac_conf_cfg_chercher_fichier(&confFichier, nom);
				retour = 0;
			} /* endif(0!=nom[0                                       */
		} /* endif(NULL!*nom                                      */
	} /* endif(NULL!=fichier                          */
	/* ------------------------------
	 * FIN DE teleFicSetNom
	 * --------------------------------     */
	return retour;
}

/* ----------------------------------
 * teleFicAjouterZone
 * ==================
 * Ajout d'un caractere dans
 * le buffer temporaire qui va
 * etre claque.
 * -------------------------------- */

static int teleFicAjouterZone(T_teleFic * fichier, char donnee)
{
	if (NULL != fichier)
	{
		if (NULL != fichier->zone)
		{
			if (NULL != fichier->fd_pt)
			{
				fichier->zone[fichier->adCour++] = donnee;
				if (fichier->tailleZone <= fichier->adCour)
				{
					/* Il s'agit ici de realiser une copie en flash du
					 * nouveau secteur puis d'incrementer le numero de
					 * secteur courant      */
					teleFicFlasherZone(fichier);
					fichier->adCour = 0;
					teleFicEffacerZone(fichier);
				} /* endif(fichier->tailleZone            */
			} /* enif(-1!=fichier->sectCou            */
		} /* NULL!=fichier                                        */
	} /* endif(NULL!=fichier                          */
	/* --------------------------------
	 * FIN DE teleFicAjouterZone
	 * --------------------------------     */
	return 0;
}

/* ----------------------------------
 * teleFicSetMode
 * ==============
 * Pour passer en mode ascii.
 * --------------------------------	*/

static void teleFicSetMode(T_teleFic * fichier, int binaire)
{
	if (NULL != fichier)
	{
		fichier->bin = binaire;
	} /* endif(NULL!=fich */
	/* --------------------------------
	 * FIN DE teleFicSetMode
	 * --------------------------------     */
}

/* ----------------------------------
 * teleFicGetMode
 * ==============
 * Lecture du mode de telechargement
 * --------------------------------	*/

static int teleFicGetMode(T_teleFic * fichier)
{
	int retour = -1;
	if (NULL != fichier)
	{
		retour = fichier->bin;
	} /* endif(NULL!=fichi */
	/* --------------------------------
	 * FIN DE teleFicGetMode
	 * --------------------------------     */
	return retour;
}

/* ----------------------------------
 * teleFicLiberer
 * ==============
 * Liberation de la memoire allouee
 * pour le telechargement.
 * --------------------------------	*/

void teleFicLiberer(T_teleFic * fichier)
{
	if (fichier != NULL)
	{
		if (NULL != fichier->zone)
		{
			kmmFree(NULL,(void *) fichier->zone);
			fichier->zone = NULL;
		}
		if (NULL != fichier->tampon)
		{
			kmmFree(NULL,(void *) fichier->tampon);
			fichier->tampon = NULL;
		}
		kmmFree(NULL,(void *) fichier);
	} /* endif(fichier!=NULL */
	/* --------------------------------
	 * FIN DE teleFicLiberer
	 * --------------------------------     */
}

/* --------------------------------
 * teleLireNom
 * =============
 * La fonction passe les blancs et
 * les tabulations.
 * --------------------------------	*/

char *teleLireNom(char *bloc, char *nom, int max, int taille, int *reste)
{
	char *ptCour = bloc;
	if (ptCour != NULL)
	{
		int index = 0;
		while ((*ptCour != ' ') && (*ptCour != '\t') && (*ptCour != '\n') && (*ptCour != '\r') && (index < taille)
				&& (index < (max - 1)))
		{
			nom[index] = *ptCour;
			index++;
			ptCour++;
		}
		nom[index] = 0;
		taille -= index;
		ptCour = tst_passe_blanc(ptCour, taille, reste);
	}
	return ptCour;
}

/* --------------------------------
 * teleLireChamp
 * =============
 * Lecture d'un champ dans la commande.
 * --------------------------------	*/

static char *teleLireChamp(char *motClef, char *bloc, int taille, int *reste)
{
	char *suite = NULL;
	/* --------------------------------
	 * SI LE BLOC EST CORRECT
	 * --------------------------------     */
	if (bloc != NULL)
	{
		int tailleMotClef = strlen(motClef);
		if (taille >= tailleMotClef)
		{
			if (strncmp(bloc, motClef, tailleMotClef) == 0)
			{
				suite = bloc;
				suite += strlen(motClef);
				taille -= strlen(motClef);
				/*suite=tst_passe_blanc(suite,
				 taille-((int)suite-(int)bloc),reste); */
			} else
			{
				printDebug("Mot clef non trouve %s\n", motClef);
				printDebug("Le bloc est  %s\n", bloc);
			} /* endif(strncmp                                        */
		} else
		{
			printDebug("Taille trop petite %d au lieu de %d\n", taille, tailleMotClef);
		} /* endif(taille>=tailleMotClef          */
	} else
	{
		printDebug("Mot clef non trouve %s\n", motClef);
	} /* endif(bloc!=NULL                                     */
	/* --------------------------------
	 * FIN DE teleLireChamp
	 * --------------------------------     */
	*reste = taille;
	return suite;
}

/* --------------------------------
 * teleLireValeur
 * =============
 * Lecture d'une valeur entiere codee
 * en hexa sur quatres caracteres.
 * --------------------------------	*/

static char *teleLireValeur(char *bloc, int taille, int *reste, int *valeur)
{
	char *suite = NULL;
	/* --------------------------------
	 * SI LE BLOC EST CORRECT
	 * --------------------------------     */
	if (bloc != NULL)
	{
		int tailleValeur = 4;
		if (taille >= tailleValeur)
		{
			char buffer[5];
			strncpy(buffer, bloc, tailleValeur);
			buffer[tailleValeur]=0;
			*valeur = strtol(buffer, NULL, 16);
			suite = bloc + tailleValeur;
			suite = tst_passe_blanc(suite, taille - tailleValeur, reste);
		} /* endif(taille>=tailleMotClef          */
	} /* endif(bloc!=NULL                                     */
	/* --------------------------------
	 * FIN DE teleLireValeur
	 * --------------------------------     */
	return suite;
}

/* --------------------------------
 * teleLireValeurShort
 * =============
 * Lecture d'une valeur sur 2 octets codee
 * en hexa sur quatres caracteres.
 * --------------------------------	*/

static char *teleLireValeurShort(char *bloc, int taille, int *reste, unsigned short *valeur)
{
	char *suite = NULL;
	/* --------------------------------
	 * SI LE BLOC EST CORRECT
	 * --------------------------------     */
	if (bloc != NULL)
	{
		int tailleValeur = 4;
		if (taille >= tailleValeur)
		{
			char buffer[5];
			strncpy(buffer, bloc, tailleValeur);
			buffer[tailleValeur]=0;
			*valeur = (unsigned short) strtol(buffer, NULL, 16);
			suite = bloc + tailleValeur;
			suite = tst_passe_blanc(suite, taille - tailleValeur, reste);
		} /* endif(taille>=tailleMotClef          */
	} /* endif(bloc!=NULL                                     */
	/* --------------------------------
	 * FIN DE teleLireValeurShort
	 * --------------------------------     */
	return suite;
}

/* --------------------------------
 * teleLireBloc
 * ============
 * Lecture d'un bloc de donnee pour
 * le telechargement.
 * Attention, en cas d'erreur d'ana-
 * lyse, on retourne une erreur qui
 * correspond au status e retourner
 * Entree:
 * -	la description du fichier
 * 		en cours de telechargement
 * -	bloc de donnees a traiter,
 * -	taille du bloc a traiter.
 * Retour :
 * -	OK si tout s'est bien passe.
 * -------------------------------- */

static int teleLireBloc(int liaison, char *bloc, int taille, T_lcr_util_buffer * reponse)
{
	int retour; /* Trame non comprise.                          */
	char *ptCour = bloc;
	int reste = taille;
	char nom[256] = "";
	unsigned short blocCour = 0;
	int nbBlocs = 0;
	int nbData = 0;
	int flag_err = 0;
	/* --------------------------------
	 * PREPARATION DE LA REPONSE
	 * --------------------------------     */
	lcr_util_buffer_ajouter(reponse, ANA_SYS_EXPTACHEC, strlen(ANA_SYS_EXPTACHEC));
	lcr_util_buffer_ajouter(reponse, " ", 1);
	lcr_util_buffer_ajouter(reponse, TELE_TELFIC, strlen(TELE_TELFIC));
//	printDebug("Le message de telechargement \n%s\nFIN FIN FIN\n",ptCour);
	/* --------------------------------
	 * NOM DU FICHIER
	 * --------------------------------     */
	if(NULL!=teleFichier)
	{
	ptCour = tst_passe_blanc(ptCour, reste, &reste);
	ptCour = teleLireChamp(TELE_FIC, ptCour, reste, &reste);
	if (ptCour != NULL)
	{
		/* pour le stockage temporaire du nom
		 * du fichier. */
		/* Recuperation du nom du fichier */
		ptCour = teleLireNom(ptCour, nom, 256, reste, &reste);
		if (NULL != ptCour)
		{
			printDebug("Le nom lu est %s\n",nom);
		}

		ptCour = tst_passe_blanc(ptCour, reste, &reste);
		/* --------------------------------
		 * NUMERO DU BLOC COURANT
		 * --------------------------------     */
		ptCour = teleLireChamp(TELE_NUM_BLOC, ptCour, reste, &reste);
		if (NULL != ptCour)
		{
			ptCour = teleLireValeurShort(ptCour, reste, &reste, &blocCour);
			printDebug("Bloc courant %d\n",blocCour);
		}
		/* --------------------------------
		 * NOMBRE DE BLOCS
		 * --------------------------------     */
		ptCour = teleLireChamp(TELE_NB_BLOCS, ptCour, reste, &reste);
		ptCour = teleLireValeur(ptCour, reste, &reste, &nbBlocs);
			printDebug("Nombre blocs  %d\n",nbBlocs);
		/* --------------------------------
		 * TAILLE DES DATAS
		 * --------------------------------     */
		ptCour = teleLireChamp(TELE_TAILLE, ptCour, reste, &reste);
		ptCour = teleLireValeur(ptCour, reste, &reste, &nbData);
			printDebug("Nombre datas  %d\n",nbData);
		/* --------------------------------
		 * ANALYSE DES DONNEES
		 * --------------------------------     */
		ptCour = teleLireChamp(TELE_DATA, ptCour, reste, &reste);
		if (ptCour != NULL)
		{
			/* -------------------------------
			 * TESTER LA COHERENCE DES DONNEES
			 * --------------------------------     */
			if ((1 == blocCour) && (1 == teleFichier->blocCour))
			{
				if (-1 == teleFicSetNom(teleFichier, nom))
				{
					printDebug("Impossible de telecharger le fichier\n");
				}
				else
				{
					printDebug("Le nom du fichier est %s\n",nom);
				} /* endif(-1==teleFicSetNom(teleF     o   */
				teleFichier->nbBlocs = nbBlocs;
			}
			else
			{
				printDebug("blocCour %d teleBlocCour %d\n",blocCour,teleFichier->blocCour);
			}

			/* Controle du nom du fichier           */
			if (strcmp(teleFichier->nom, nom) != 0)
			{
				printDebug("Erreur de nom de fichier attendu %s reel %s\n", teleFichier->nom, nom);
				flag_err = 3;
			}
			/* Verifier que le numero de bloc est
			 * bien le numero de bloc attendu       */
			if (blocCour != teleFichier->blocCour)
			{
				printDebug("Numero de bloc courant incorrect %d\n", blocCour);
				if (blocCour == (teleFichier->blocCour - 1))
				{
					flag_err = 4;
				} else
				{
					flag_err = 3;
				}
			}
			/* Verifier que le nombre de bloc
			 * est coherent */
			if (nbBlocs != teleFichier->nbBlocs)
			{
				printDebug("Nombre de bloc incorrect %d\n", nbBlocs);
				flag_err = 3;
			}

			else
			/* Analyse des donnees                  */
			if ((0 == flag_err) && (nbData <= reste))
			{
				int notFin;
				retour = teleDecoderBloc(teleFichier, ptCour, nbData, &notFin);
				if (-1 == retour)
				{
					/* Si le retour de la fonction est incorrect, on retourne
					 * une erreur et on incremente le compteur d'incoherence.
					 * Au bout de trois incoherences, on abandonne le
					 * telechargement */
					printDebug("Erreur dans l'analyse du bloc %d\n", teleFichier->blocCour);
					flag_err = 3;
				} else
				{
					/* --------------------------------
					 * C'EST FINI, ON CLAQUE...
					 * --------------------------------     */
					if (teleFichier->blocCour == teleFichier->nbBlocs)
					{
						/* Recopie des informations dans la
						 * memoire flash du dernier secteur.*/
						printDebug("Flashage de la zone\n");
						teleFicFlasherZone(teleFichier);
						/* Calcul du checksum de la version     */
						printDebug("Calcul du checksum\n");
						teleFicCalculerChecksum(teleFichier);
						/* Destruction de la structure de
						 * telechargement       */
						printDebug("Le telechargement est fini 1\n");
						teleFicLiberer(teleFichier);
						teleFichier = NULL;
					} else
					{
						/* On regarde si l'on en est au
						 * dernier bloc. Si ce n'est pas
						 * le cas, on incremente le numero
						 * du bloc attendu.     */
						teleFichier->blocCour++;
					} /* endif(teleFichier->blocCour==        */
				} /* endif(-1==retour)                            */
			} else
			{
				printDebug("Il y a un soucis. flag_err %d nbData %d reste %d\n", flag_err, nbData, reste);
			} /* endif((0==flag_err)&&(nbData==       */
		} /* if(ptCour!=NULL)                                     */
	} else
	{
		flag_err = 2;
	} /* endif(ptCour!=NULL                           */
	/* Retourner ici le resultat de la
	 * commande */
	if (0 == flag_err)
	{
		char messBloc[9];
		sprintf(messBloc, " %s%04X", TELE_NUM_BLOC, blocCour);
		lcr_util_buffer_ajouter(reponse, " ST=0000", strlen(" ST=0000"));
		lcr_util_buffer_ajouter(reponse, messBloc, strlen(messBloc));
		if(NULL!=teleFichier)
		{
			teleFichier->nbRetry = 0;
		}
	} else
	{
		/* On distingue le cas de la trame
		 * repetee. Ce cas n'a pas lieu
		 * d'etre selon moi.    */
		if (4 == flag_err)
		{
			teleFichier->nbRetry++;
			if (teleFichier->nbRetry > 3)
			{
				lcr_util_buffer_ajouter(reponse, " ST=0002", strlen(" ST=0002"));
				teleFicLiberer(teleFichier);
				teleFichier = NULL;
				flag_err = 2;
			} else
			{
				lcr_util_buffer_ajouter(reponse, " ST=0001", strlen(" ST=0001"));
			}
		} else
		{
			lcr_util_buffer_ajouter(reponse, " ST=0002", strlen(" ST=0002"));
			teleFicLiberer(teleFichier);
			teleFichier = NULL;
		}
		/* Il n'est pas toujours certain que
		 * l'on connaisse e ce niveau la le
		 * numero de bloc courant. */
		if (0 != blocCour)
		{
			char messBloc[9];
			sprintf(messBloc, " %s%04X", TELE_NUM_BLOC, blocCour);
			lcr_util_buffer_ajouter(reponse, messBloc, strlen(messBloc));
		} /* endif(0!=blocCour                            */
	} /* endif(0==flag_err                            */
	}
	else
	{
		lcr_util_buffer_ajouter(reponse, " ST=0005", strlen(" ST=0005"));
		teleFicLiberer(teleFichier);
		teleFichier = NULL;
	}
	/* --------------------------------
	 * FIN DE teleLireBloc
	 * --------------------------------     */
	return flag_err;
}

/* --------------------------------
 * teleCalculerOctet
 * ================
 * Retour de la valeur sur un octet
 * --------------------------------	*/

unsigned char teleCalculerOctet(char *bloc)
{
	char buffer[3];
	unsigned char valeur;
	strncpy(buffer, bloc, 2);
	buffer[2] = 0;
	valeur = (unsigned char) strtol(buffer, NULL, 16);
	return valeur;
}

/* --------------------------------
 * teleDecoderLigne
 * ================
 * Decodage d'une ligne de code
 * S1-S9.
 * Cette procedure est utilisee dans
 * le cas ou le telechargement est
 * realise sur la liaison serie.
 * --------------------------------	*/

char *teleDecoderLigne(T_teleFic * fichier, char *bloc, int taille, int *reste, int *notFin)
{
	char *ptCour = NULL;

	if ((bloc != NULL) && (taille > 0))
	{
		/* --------------------------------
		 * TEST DU MODE DE TELECHARGEMENT
		 * --------------------------------     */
		ptCour = bloc;
		if (*ptCour == 'S')
		{
			if (taille <= 4)
			{
				strncpy(fichier->tampon, ptCour, taille);
				fichier->nbCarTampon = taille;
				ptCour += taille;
				*reste = 0;
			} else
			{
				/* Calcul de la longueur de la ligne */
				int nbCar = teleCalculerOctet(ptCour + 2);
				/* Si la taille est inferieure a la longueur de la ligne
				 * utilisation du tampon
				 */
				if ((nbCar * 2 + 4) > taille)
				{
					strncpy(fichier->tampon, ptCour, taille);
					fichier->nbCarTampon = taille;
					ptCour += taille;
					*reste = 0;
				} else
				{
					unsigned char bcc = (unsigned char) nbCar;
					unsigned char valCour;
					unsigned char type = ptCour[1];
					int address = 0;
					int indDeb = 0;
					ptCour += 4;
					switch (type)
					{
					case '3':
						valCour = teleCalculerOctet(ptCour);
						bcc += valCour;
						address = (address << 8) + (int) valCour;
						ptCour += 2;
						indDeb++;
					case '2':
						valCour = teleCalculerOctet(ptCour);
						bcc += valCour;
						address = (address << 8) + (int) valCour;
						ptCour += 2;
						indDeb++;
					case '1':
						valCour = teleCalculerOctet(ptCour);
						bcc += valCour;
						address = (address << 8) + (int) valCour;
						ptCour += 2;
						indDeb++;
						valCour = teleCalculerOctet(ptCour);
						bcc += valCour;
						address = (address << 8) + (int) valCour;
						ptCour += 2;
						indDeb++;
						/* Decodage du reste de la trame */
						{
							int indice;
							for (indice = 0; indice < (nbCar - 1 - indDeb); indice++)
							{
								valCour = teleCalculerOctet(ptCour);
								bcc += valCour;
								teleFicAjouterZone(fichier, valCour);
								ptCour += 2;
							}
						}
						/* Calcul du BCC                                */
						valCour = teleCalculerOctet(ptCour);
						ptCour += 2;
						valCour = (unsigned char) ~valCour;
						if (bcc != valCour)
						{
							/* Probleme de checkSum */
							printf("Probleme de chechSum %x et %x\n", bcc, valCour);
							ptCour = NULL;
						}
						break;
					case '9':
					case '8':
					case '7':
						/* C'est fini, on se fout du reste */
						*notFin = 0;
						{
							int indice;
							for (indice = 0; indice < (nbCar - 1); indice++)
							{
								valCour = teleCalculerOctet(ptCour);
								bcc += valCour;
								ptCour += 2;
							}
						}
						/* Calcul du BCC                                */
						valCour = teleCalculerOctet(ptCour);
						ptCour += 2;
						valCour = (unsigned char) ~valCour;
						if (bcc != valCour)
						{
							/* Probleme de checkSum */
							printf("Probleme de chechSum %x et %x\n", bcc, valCour);
							ptCour = NULL;
						}
						break;
					} /* endswitch(type)                                      */
					*reste = taille - (nbCar * 2 + 4);
				} /* endif((nbCar*2+4)>taille)            */
			} /* endif(taille<=4)                                     */
		} else
		{
			printf("Pas de S\n");
			ptCour = NULL;
		} /* endif(*ptCour=='S')                          */
	} /* endif((bloc!=NULL)&&(taille>0))      */
	/* --------------------------------
	 * FIN DE teleDecoderLigne
	 * --------------------------------     */
	return ptCour;
}

/* --------------------------------
 * teleDecoderS1S9
 * ===============
 * Decodage de S1-S9 dans le cadre
 * du telechargement par liaison
 * serie.
 * --------------------------------	*/

static int teleDecoderS1S9(T_teleFic * fichier, char *bloc, int taille, int *notFin)
{
	int erreur = 0;
	char *ptCour = bloc;
	int reste = taille;

	/* --- TRAITEMENT DU TAMPON                     */
	/* Il s'agit ici de traiter les
	 * caracteres qui ne l'ont pas ete
	 * lors du dernier decodage.            */
	if (0 != fichier->nbCarTampon)
	{
		while (reste > 0)
		{
			if (*ptCour == 'S')
			{
				break; /* C'est fini...                                        */
			} else
			{
				/* recopie des premiers caracteres
				 * dans le tampon */
				fichier->tampon[fichier->nbCarTampon++] = *ptCour++;
				reste--;
			}
		} /* endwhile(reste>0                                     */
		/* Analyse des caracteres restants
		 * du dernier decodage  */
		if (NULL == teleDecoderLigne(fichier, fichier->tampon, fichier->nbCarTampon, &fichier->nbCarTampon, notFin))
		{
			erreur = 2;
		}
	} /* endif(fichier->nbCarTampon           */
	/* --------------------------------
	 * DECODAGE DU RESTE DU BLOC
	 * --------------------------------     */
	if (erreur == 0)
	{
		/* --- TRAITEMENT DU BLOC                       */
		while ((reste != 0) && (ptCour != NULL))
		{
			ptCour = teleDecoderLigne(fichier, ptCour, reste, &reste, notFin);
		}
		if (ptCour == NULL)
		{
			erreur = 2;
		}
	}
	/* ------------------------------
	 * FIN DE teleDecoderS1S9
	 * --------------------------------     */
	return erreur;
}

/* ----------------------------------
 * teleDecoderBinaire
 * ==================
 * Copie brute des donnees dans
 * la flash.
 * --------------------------------	*/

static int teleDecoderBinaire(T_teleFic * fichier, char *bloc, int taille, int *notFin)
{
	int erreur = 0;
	int indice;

	/* --- TRAITEMENT DU TAMPON                     */
	/* Il s'agit ici de traiter les
	 * caracteres qui ne l'ont pas ete
	 * lors du dernier decodage.            */
	for (indice = 0; indice < taille; indice++)
	{
		teleFicAjouterZone(fichier, bloc[indice]);
	} /* endfor(indice=0;                                     */
	/* ------------------------------
	 * FIN DE teleDecoderBinaire
	 * --------------------------------     */
	return erreur;
}

/* --------------------------------
 * teleDecoderBloc
 * ===============
 * Decodage d'un bloc recu.
 * La procedure permet de decoder
 * du S1-S9 (telechargement par la
 * liaison serie) ou du code
 * binaire (WorldFip).
 * --------------------------------	*/

int teleDecoderBloc(T_teleFic * fichier, char *bloc, int taille, int *notFin)
{
	int erreur = 0;
	/* --------------------------------     */
	/* DONNEES BINAIRE OU  S1-S9 ?          */
	/* --------------------------------     */
	if ((NULL != bloc) && (taille))
	{
		/* Passage en mode S1-S9			*/
		if ((1 == fichier->blocCour) && (strncmp(bloc, "S1", 2) == 0))
		{
			teleFicSetMode(fichier, 0);
		}
		/* endif((1==fichier->blocCour		*/
		/* --------------------------------
		 * TRAVAIL SUR LE BLOC
		 * --------------------------------     */
		switch (teleFicGetMode(fichier))
		{
		case 1: /* Telechargement binaire. */
			erreur = teleDecoderBinaire(fichier, bloc, taille, notFin);
			break;
		case 0: /* Telechargement S1-S9 */
			erreur = teleDecoderS1S9(fichier, bloc, taille, notFin);
			break;
		default:
			erreur = -1;
			break;
		} /* endswitch(teleFicGetMode                     */
	} else
	{
		erreur = -1;
	} /* endif((NULL!=ptCour)&&(taille))      */
	/* --------------------------------
	 * FIN DE teleDecoderBloc
	 * --------------------------------     */
	return erreur;
}

/* ------------------------------------
 * teleChangerModeTel
 * ==================
 * Passage en mode telechargement.
 * Si l'equipement est deje en mode
 * telechargement, il y a erreur.
 * ------------------------------------ */

static int teleChangerModeTel(int liaison, char *message, int longueur, T_lcr_util_buffer * reponse)
{
	int flag_err;
	lcr_util_buffer_ajouter(reponse, ANA_SYS_EXPTACHEC, strlen(ANA_SYS_EXPTACHEC));
	lcr_util_buffer_ajouter(reponse, " ", 1);
	lcr_util_buffer_ajouter(reponse, TELE_MODETEL, strlen(TELE_MODETEL));
	if (NULL == teleFichier)
	{
		/* -------------------------------
		 * CHANGEMENT TEMPO DIALOGUE FIP
		 * --------------------------------     */
#ifndef SANS_WORLDFIP
		dialogueFipSetTempo(20000);
#endif
		/* -------------------------------
		 * VERIFICATION DU MODE FONCTIONNEMENT
		 * --------------------------------     */
		if (1==supRunLireEtat())
		{
			printDebug("L'application devrait etre arretee pour telecharger\n");
		} /* endif(etaSystTesterAppli                     */
		/* Initialiser ici la structure qui
		 * va etre utilisee pour le
		 * telechargement.                                      */
		teleFichier = teleFicCreer(0x1000);
		if (NULL != teleFichier)
		{
			flag_err = 0;
			lcr_util_buffer_ajouter(reponse, " ST=0000", strlen(" ST=0000"));
		} else
		{
			flag_err = 4;
			lcr_util_buffer_ajouter(reponse, " ST=0002", strlen(" ST=0002"));
		}
		/* Il s'agit donc de retourner un
		 * status 00            */
		printDebug("Passage en mode telechargement\n");
	} else
	{
		/* Rien ne va plus, on demande de
		 * passer en mode telechargement
		 * alors que l'on y est deje.   */
		/* Il s'agit donc de retourner un status 02             */
		printDebug("On est deja en telechargement\n");
		lcr_util_buffer_ajouter(reponse, " ST=0002", strlen(" ST=0002"));
		flag_err = 4;
	} /* endif(NULL==teleFichier)                     */
	/* --------------------------------
	 * FIN DE teleChangerModeTel
	 * --------------------------------     */
	return flag_err;
}

/* --------------------------------
 * teleChangerModeExp
 * ==================
 * Passage en mode exploitation.
 * Si on est en cours de telechar-
 * gement, il y a forcage du mode.
 * --------------------------------	*/

static int teleChangerModeExp(int liaison, char *message, int longueur, T_lcr_util_buffer * reponse)
{
	int flag_err;
	/* Preparation de la reponse            */
	lcr_util_buffer_ajouter(reponse, ANA_SYS_EXPTACHEC, strlen(ANA_SYS_EXPTACHEC));
	lcr_util_buffer_ajouter(reponse, " ", 1);
	lcr_util_buffer_ajouter(reponse, TELE_MODEEXP, strlen(TELE_MODEEXP));
	/* -------------------------------
	 * CHANGEMENT TEMPO DIALOGUE FIP
	 * --------------------------------     */
#ifndef SANS_WORLDFIP
	dialogueFipSetTempo(50000);
#endif
	/* --------------------------------     */
	/* ANALYSE SUIVANT LE MODE                      */
	/* --------------------------------     */
	if (NULL == teleFichier)
	{
		char nom[256] = "";
		char *ptCour = message;
		int lgCour = longueur;
		/* Tout va bien, on peut passer en
		 * mode exploitation sur le fichier
		 * demande.     */
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		ptCour = teleLireChamp(TELE_FIC, ptCour, lgCour, &lgCour);
		teleLireNom(ptCour, nom, 256, lgCour, &lgCour);
		if (0 != nom[0])
		{
			T_tacConfFichiers confFichier;
			int ficSel;
			printDebug("Le nom de la version est %s\n", nom);
			tac_conf_cfg_lire_fichier(&confFichier);
			ficSel = tac_conf_cfg_elire_fichier(&confFichier, nom);
			if (-1 != ficSel)
			{
				printDebug("Passage en mode exploitation de %s\n", nom);
				lcr_util_buffer_ajouter(reponse, " ST=0000", strlen(" ST=0000"));
			} else
			{
				printDebug("La version du fichier n'est pas trouvee\n");
				lcr_util_buffer_ajouter(reponse, " ST=0002", strlen(" ST=0002"));
			}
		} else
		{
			printDebug("Passage en mode exploitation\n");
			/* Il n'y a a priori rien a faire.      */
			lcr_util_buffer_ajouter(reponse, " ST=0000", strlen(" ST=0000"));
		}
		flag_err = 0;
		/* Il s'agit ici de verifier si le fichier existe reellement
		 * et eventuellement positionner la donnee qui va bien
		 * pour proceder au telechargement.     */
		/* Si le fichier n'est pas disponible, on retourne une erreur 02 */

	} else
	{
		flag_err = 4;
		lcr_util_buffer_ajouter(reponse, " ST=0002", strlen(" ST=0002"));
		/* Rien ne va plus, on est en mode telechargement       */
		/* Arret du telechargement en cours     */
		teleFicLiberer(teleFichier);
		teleFichier = NULL;
	} /* endif(NULL==teleFichier)                     */
	/* --------------------------------
	 * FIN DE teleChangerModeExp
	 * --------------------------------     */
	return flag_err;
}

/* --------------------------------
 * teleChangerSysteme
 * ==================
 * Changement du systeme de tele-
 * chargement pour le systeme dont
 * on passe le nom dans la commande.
 * --------------------------------	*/

static int teleChangerSysteme(int liaison, char *message, int longueur, T_lcr_util_buffer * reponse)
{
	int flag_err;
	/* Preparation de la reponse            */
	lcr_util_buffer_ajouter(reponse, ANA_SYS_EXPTACHEC, strlen(ANA_SYS_EXPTACHEC));
	lcr_util_buffer_ajouter(reponse, " ", 1);
	lcr_util_buffer_ajouter(reponse, TELE_SYSTEME, strlen(TELE_SYSTEME));
	/* --------------------------------     */
	/* ANALYSE SUIVANT LE MODE                      */
	/* --------------------------------     */
	if (NULL == teleFichier)
	{
		char nom[256] = "";
		char *ptCour = message;
		int lgCour = longueur;

		if (0==supRunLireEtat())
		{
			printDebug("Le changement doit etre effectue en mode application\n");
			lcr_util_buffer_ajouter(reponse, " ST=0003", strlen(" ST=0003"));
		} /* endif(etaSystTesterAppli                     */
		else
		{
			/* Tout va bien, on peut passer en
			 * mode exploitation sur le fichier
			 * demande.     */
			ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
			ptCour = teleLireChamp(TELE_FIC, ptCour, lgCour, &lgCour);
			teleLireNom(ptCour, nom, 256, lgCour, &lgCour);
			if (0 != nom[0])
			{
				T_tacConfFichiers confFichier;
				int ficSel;
				printDebug("Le nom de la version est %s\n", nom);
				tac_conf_cfg_lire_fichier(&confFichier);
				ficSel = tac_conf_cfg_chercher_fichier(&confFichier, nom);
				/* Recherche du numero du fichier */
				if (-1 != ficSel)
				{
					/* Recopie du fichier la ou ca
					 * va bien.     */
					printDebug("La fonction n'existe plus \n");
					lcr_util_buffer_ajouter(reponse, " ST=0003", strlen(" ST=0003"));
				} else
				{
					printDebug("La version du fichier n'est pas trouvee\n");
					lcr_util_buffer_ajouter(reponse, " ST=0002", strlen(" ST=0002"));
				}
			} else
			{
				/* Il n'y a a priori rien a faire.      */
				lcr_util_buffer_ajouter(reponse, " ST=0000", strlen(" ST=0000"));
			}
			flag_err = 0;
		}
	} else
	{
		flag_err = 4;
		lcr_util_buffer_ajouter(reponse, " ST=0002", strlen(" ST=0002"));
		/* Rien ne va plus, on est en mode telechargement       */
		/* Arret du telechargement en cours     */
		teleFicLiberer(teleFichier);
		teleFichier = NULL;
	}
	/* --------------------------------
	 * FIN DE teleChangerModeExp
	 * --------------------------------     */
	return flag_err;
}

/* --------------------------------
 * teleListerVersion
 * ==================
 * Liste des versions disponibles
 * --------------------------------	*/

static int teleListerVersion(int liaison, char *message, int longueur, T_lcr_util_buffer * reponse)
{
	int flag_err = 0;
	/* Preparation de la reponse            */
	lcr_util_buffer_ajouter(reponse, ANA_SYS_EXPTACHEC, strlen(ANA_SYS_EXPTACHEC));
	lcr_util_buffer_ajouter(reponse, " ", 1);
	lcr_util_buffer_ajouter(reponse, TELE_LISTEVERS, strlen(TELE_LISTEVERS));
	lcr_util_buffer_recopier(reponse, "\n\r", 2);
	/* --------------------------------     */
	/* ANALYSE SUIVANT LE MODE                      */
	/* --------------------------------     */
	{
		T_tacConfFichiers confFichier;
		char *buffer = kmmAlloc(NULL,1000);
		tac_conf_cfg_lire_fichier(&confFichier);
		if (NULL != buffer)
		{
			int nbCar = tac_conf_cfg_traiter_fichier(&confFichier, buffer, 1000);
			if (-1 != nbCar)
			{
				lcr_util_buffer_recopier(reponse, buffer, nbCar);
			}
			kmmFree(NULL,buffer);
		}
	}
	/* --------------------------------
	 * FIN DE teleListerVersion
	 * --------------------------------     */
	return flag_err;
}

/* --------------------------------
 * teleListerVersionCourante
 * ==========================
 * Liste de la version courante
 * --------------------------------	*/

static int teleListerVersionCourante(int liaison, char *message, int longueur, T_lcr_util_buffer * reponse)
{
	int flag_err = 0;
	char buffer[50];

	/* Preparation de la reponse            */
	lcr_util_buffer_ajouter(reponse, ANA_SYS_EXPTACHEC, strlen(ANA_SYS_EXPTACHEC));
	lcr_util_buffer_ajouter(reponse, " ", 1);
	lcr_util_buffer_ajouter(reponse, TELE_VERSION, strlen(TELE_VERSION));
	sprintf(buffer, " %d.%d", versionLireMajeure("Application"), versionLireMineure("Application"));
	lcr_util_buffer_recopier(reponse, buffer, strlen(buffer));
	/* --------------------------------
	 * FIN DE teleListerVersionCourante
	 * --------------------------------     */
	return flag_err;
}

/* --------------------------------
 * teleAnalyser
 * ============
 * Analyse de la commande de tele-
 * chargement.
 * Le resultat de la requete est
 * retourne au besion dans le
 * parametre rep.
 * --------------------------------	*/

int teleAnalyser(int liaison, char *message, int longueur, T_lcr_util_buffer * reponse)
{
	char *ptCour = (char *) message;
	int lgCour = longueur;
	int retour = 0;
	/* On saute le EXPTACHEC                        */
	ptCour += strlen(ANA_SYS_EXPTACHEC);
	lgCour -= strlen(ANA_SYS_EXPTACHEC);
	/* Prochain parametre                           */
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if (strncmp(ptCour, TELE_MODETEL, strlen(TELE_MODETEL)) == 0)
	{
		ptCour += strlen(TELE_MODETEL);
		lgCour -= strlen(TELE_MODETEL);
		teleChangerModeTel(liaison, ptCour, lgCour, reponse);
	} else if (strncmp(ptCour, TELE_TELFIC, strlen(TELE_TELFIC)) == 0)
	{
		/* Telechargement d'un fichier          */
		ptCour += strlen(TELE_TELFIC);
		lgCour -= strlen(TELE_TELFIC);
		teleLireBloc(liaison, ptCour, lgCour, reponse);
		retour = 1;
		printDebug("Sortie de teleAnalyser (TELE_TELFIC) \n");
	} else if (strncmp(ptCour, TELE_MODEEXP, strlen(TELE_MODEEXP)) == 0)
	{
		ptCour += strlen(TELE_MODEEXP);
		lgCour -= strlen(TELE_MODEEXP);
		teleChangerModeExp(liaison, ptCour, lgCour, reponse);
	} else if (strncmp(ptCour, TELE_SYSTEME, strlen(TELE_SYSTEME)) == 0)
	{
		ptCour += strlen(TELE_SYSTEME);
		lgCour -= strlen(TELE_SYSTEME);
		teleChangerSysteme(liaison, ptCour, lgCour, reponse);
	} else if (strncmp(ptCour, TELE_LISTEVERS, strlen(TELE_LISTEVERS)) == 0)
	{
		ptCour += strlen(TELE_LISTEVERS);
		lgCour -= strlen(TELE_LISTEVERS);
		teleListerVersion(liaison, ptCour, lgCour, reponse);
	} else if (strncmp(ptCour, TELE_VERSION, strlen(TELE_VERSION)) == 0)
	{
		ptCour += strlen(TELE_VERSION);
		lgCour -= strlen(TELE_VERSION);
		teleListerVersionCourante(liaison, ptCour, lgCour, reponse);
	}
#ifdef RAD
	else if (strncmp (ptCour, TELE_DETTEL, strlen (TELE_DETTEL)) == 0)
	{
		ptCour += strlen (TELE_DETTEL);
		lgCour -= strlen (TELE_DETTEL);
		teleDetTel (liaison, ptCour, lgCour, reponse);
	}
	else if (strncmp (ptCour, TELE_DETFIC, strlen (TELE_DETFIC)) == 0)
	{
		ptCour += strlen (TELE_DETFIC);
		lgCour -= strlen (TELE_DETFIC);
		teleDetFicCmd (liaison, ptCour, lgCour, reponse);
	}
	else if(strncmp(ptCour,TELE_DETFLASH,strlen(TELE_DETFLASH))==0)
	{
		ptCour+=strlen(TELE_DETFLASH);
		lgCour-=strlen(TELE_DETFLASH);
		teleDetFicFlash(ptCour,lgCour,reponse);
	}
	else if (strncmp (ptCour, TELE_DETPROG, strlen (TELE_DETPROG)) == 0)
	{
		ptCour += strlen (TELE_DETPROG);
		lgCour -= strlen (TELE_DETPROG);
		teleDetProg (liaison, ptCour, lgCour, reponse);
	}
	else if (strncmp (ptCour, TELE_DETETAT, strlen (TELE_DETETAT)) == 0)
	{
		ptCour += strlen (TELE_DETETAT);
		lgCour -= strlen (TELE_DETETAT);
		teleDetEtat (liaison, ptCour, lgCour, reponse);
	}
	else if (strncmp (ptCour, TELE_DETEXP, strlen (TELE_DETEXP)) == 0)
	{
		ptCour += strlen (TELE_DETEXP);
		lgCour -= strlen (TELE_DETEXP);
		teleDetExp (liaison, ptCour, lgCour, reponse);
	}
	else if (strncmp (ptCour, TELE_DETVERS, strlen (TELE_DETVERS)) == 0)
	{
		ptCour += strlen (TELE_DETVERS);
		lgCour -= strlen (TELE_DETVERS);
		teleDetVers (liaison, ptCour, lgCour, reponse);
	}
#endif
	else if (strncmp(ptCour, TELE_TESTFIP, strlen(TELE_TESTFIP)) == 0)
	{
		ptCour += strlen(TELE_TESTFIP);
		lgCour -= strlen(TELE_TESTFIP);
		lcr_util_buffer_ajouter(reponse, ANA_SYS_EXPTACHEC, strlen(ANA_SYS_EXPTACHEC));
		lcr_util_buffer_ajouter(reponse, " ", 1);
		lcr_util_buffer_ajouter(reponse, TELE_TESTFIP, strlen(TELE_TESTFIP));
		lcr_util_buffer_ajouter(reponse, " ST=0000", strlen(" ST=0000"));
#ifndef SANS_WORLDFIP
		dialogueFipSetTestReset(1);
#endif
	}
	/* --------------------------------
	 * FIN DE teleDetTest
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * teleFicFlasherZone
 * ==================
 * Copie de la zone de memoire en
 * flash.
 * On essaie d'effacer la flash avant.
 * --------------------------------	*/

int teleFicFlasherZone(T_teleFic * fichier)
{
	int retour = -1;
	/* Il s'agit ici de recopier un secteur complet dans la flash.  */
	if (NULL != fichier)
	{
		if (NULL != fichier->fd_pt)
		{
		long taille=fichier->adCour;
			if(taille!=fwrite (fichier->zone,sizeof(char), taille,fichier->fd_pt))
			{
				printDebug("teleFicFlasherZone : Probleme d'ecriture %d\n",errno);
			}
			else
			{
				printDebug("teleFicFlasherZone : flashage correct!!! taille %d\n",taille);
			}
			retour = 0;
		} /* endif((-1!=fichier->sectCour         */
	} /* endif(NULL!=fichier)                         */
	/* --------------------------------
	 * FIN DE teleFicFlasherZone
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * teleFicEffacerZone
 * ==================
 * Remise e zero de la zone memoire
 * tampon avant de continuer.
 * --------------------------------	*/

static int teleFicEffacerZone(T_teleFic * fichier)
{
	int retour = -1;
	/* Il s'agit ici de recopier un secteur complet dans la flash.  */
	if (NULL != fichier)
	{
		if (NULL != fichier->zone)
		{
			memset(fichier->zone, 0, fichier->tailleZone);
		}
	} /* endif(NULL!=fichier)                         */
	/* --------------------------------
	 * FIN DE teleFicFlasherZone
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * teleFicCalculerChecksum
 * ==================
 * Copie de la zone de memoire en
 * flash.
 * On essaie d'effacer la flash avant.
 * --------------------------------	*/

static int teleFicCalculerChecksum(T_teleFic * fichier)
{
	int retour = -1;
	/* Il s'agit ici de recopier un secteur complet dans la flash.  */
	if (NULL != fichier)
	{
		unsigned long bcc;
		T_tacConfFichiers conf;
		printDebug("teleFicCalculerChecksum\n");
		if(NULL!=fichier->fd_pt)
		{
			fclose(fichier->fd_pt);
			fichier->fd_pt=NULL;
		}
		/* Ajouter ici l'effacement des secteurs de flash. */
		/* Fin d'effacement     */
		bcc = tac_conf_cfg_calculer_checksum(fichier->ficSel);
		tac_conf_cfg_lire_fichier(&conf);
		tac_conf_cfg_ecrire_checksum(&conf, fichier->ficSel, bcc);
	} /* endif(NULL!=fichier)                         */
	/* --------------------------------
	 * FIN DE teleFicFlasherZone
	 * --------------------------------     */
	return retour;
}
