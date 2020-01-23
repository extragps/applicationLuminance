/*
 * tac_conf.c
 * ===================
 *
 *  Created on: 20 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: tac_conf_fichier.c,v $
 * Revision 1.2  2018/06/04 08:38:39  xg
 * Passage en version V122
 *
 * Revision 1.4  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.3  2008/10/03 16:21:46  xag
 * *** empty log message ***
 *
 * Revision 1.2  2008/09/29 07:58:52  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.1  2008/09/12 14:57:09  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.13  2008/06/20 07:24:10  xag
 * Pour tester l'entete de fichier
 *
 */
/* ********************************
 * LISTE DES INCLUDES
 * ********************************	*/

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "standard.h"
#include "mon_inc.h"
#include "define.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "lcr_idfh.h"
#include "lcr_cf3h.h"
#include "lcr_idfh.h"
#include "lcr_cfs.h"
#include "x01_trch.h"
#include "xversion.h"
#include "mon_debug.h"
#include "tac_conf.h"
#include "amd.h"
#include "extLib.h"
#include "seqLib.h"
#include "ypc_las.h"
#include "identLib.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "lcr_cftp.h"
#include "pip_util.h"
#include "lcr_sc.h"
#include "lcr_st_lcpi.h"

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* ********************************
 * FONCTIONS DES CONSTANTES
 * ********************************	*/

#define TAC_CONF_NOM_FICHIER "fichiers.cfg"
#define TAC_CONF_TAILLE_REPONSE 600

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* Configuration propre aux stations RAD SIRIUS, a revoir pour
 * toute autre application. */

/* ********************************
 * VARIABLES LOCALES
 *********************************** */

//static int tac_conf_fic = 2;
#define TAC_CONF_TYPE_ENTETE  	0
#define TAC_CONF_TYPE_CFG  		1
#define TAC_CONF_FICHIER_CONFIG "config.cfg"

/* ********************************
 * PROTOTYPE DES FONCTIONS DU MODULE
 * ********************************	*/

static void tac_conf_cfg_imprimer_fichier(T_tacConfFichiers * confFichier);
static void _tac_conf_cfg_init(T_tacConfFichiers *confFichier);

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * tac_conf_cfg_raz_fichier
 * ========================
 * Reinitialisation de la configura-
 * tion des fichiers.
 * --------------------------------	*/

void tac_conf_cfg_raz_fichier(void)
{
T_tacConfFichiers confFichier;
	/* Effectuer ici l'initialisation par defaut de la structure.. */
	_tac_conf_cfg_init(&confFichier);
	/* Ecrire le fichier... */
	tac_conf_cfg_ecr_fichier(&confFichier);
}

/* --------------------------------
 * tac_conf_cfg_impimer
 * ====================
 * Impression des caracteristiques
 * du fichier.
 * --------------------------------	*/

void tac_conf_cfg_imprimer(void)
{
	T_tacConfFichiers confFichiers;
	if (-1 != tac_conf_cfg_lire_fichier(&confFichiers))
	{
		tac_conf_cfg_imprimer_fichier(&confFichiers);
	} else
	{
		printDebug("tac_conf_cfg_imprimer: Impossible de lire la configuration des fichiers\n");
	} /* endif(-1!=tac_conf_cfg_lire_fich */
	/* --------------------------------
	 * FIN DE tac_conf_cfg_imprimer
	 * --------------------------------   */
}

/* --------------------------------
 * tac_conf_cfg_traiter_fichiers
 * =============================
 * Recopie dans une chaine de
 * caracteres des fichiers presents
 * dans le systeme.
 * --------------------------------	*/

int tac_conf_cfg_traiter_fichier(T_tacConfFichiers * confFichier, char *buffer, int taille)
{
	int retour = -1;
	char tampon[TAC_CONF_TAILLE_REPONSE];
	int nbCar = 0;
	int ficSel;
	nbCar += snprintf(&tampon[nbCar],TAC_CONF_TAILLE_REPONSE-nbCar, "FIC COURANT=");
	if (0 == (unsigned char) confFichier->courant[0])
	{
		printDebug("Pas de fichier courant\n");
		nbCar += snprintf(&tampon[nbCar], TAC_CONF_TAILLE_REPONSE-nbCar,"\n\r");
	} else
	{
		struct tm heure;
		localtime_r((const time_t *) &confFichier->courantDate, &heure);
		nbCar += snprintf(&tampon[nbCar],TAC_CONF_TAILLE_REPONSE-nbCar,
				"%s DATE=%02d/%02d/%04d %02d:%02d:%02d CKS=%08lX\n\r", confFichier->courant, heure.tm_mday,
				heure.tm_mon + 1, heure.tm_year + 1900, heure.tm_hour, heure.tm_min, heure.tm_sec,
				confFichier->courantBcc);
	}
	nbCar += snprintf(&tampon[nbCar],TAC_CONF_TAILLE_REPONSE-nbCar, "FIC EXPLOIT=");
	if (0 == (unsigned char) confFichier->exploit[0])
	{
		printDebug("Pas de fichier d'exploitation\n");
		nbCar += snprintf(&tampon[nbCar],TAC_CONF_TAILLE_REPONSE-nbCar, "\n\r");
	} else
	{
		printDebug("Fichier d'exploitation %s\n", confFichier->exploit);
		nbCar += snprintf(&tampon[nbCar],TAC_CONF_TAILLE_REPONSE-nbCar, "%s\n\r", confFichier->exploit);
	}
	for (ficSel = 0; ficSel < TAC_CONF_NB_VERSIONS; ficSel++)
	{
		if (0 == (unsigned char) confFichier->version[ficSel].nom[0])
		{
			printDebug("Pas de version %d\n", ficSel);
		} else
		{
			struct tm heure;
			localtime_r((const time_t *) &confFichier->version[ficSel].date, &heure);
		nbCar += snprintf(&tampon[nbCar],TAC_CONF_TAILLE_REPONSE-nbCar,
					"FIC=%s DATE=%02d/%02d/%04d %02d:%02d:%02d CKS=%08lX\n\r", confFichier->version[ficSel].nom,
					heure.tm_mday, heure.tm_mon + 1, heure.tm_year + 1900, heure.tm_hour, heure.tm_min, heure.tm_sec,
					confFichier->version[ficSel].bcc);
			printDebug("version %d nom %s et date %s\n", ficSel, confFichier->version[ficSel].nom, buffer);
		}
	}
	if (nbCar < taille)
	{
		strncpy(buffer, tampon, nbCar);
		retour = nbCar;
	}

	/* -------------------------------
	 * tac_conf_cfg_traiter_fichier
	 * -------------------------------      */
	return retour;
}

/* ---------------------------------
 * tac_conf_calculer_checksum
 * ==========================
 * calcul du checksum pour la version
 * courante.
 * --------------------------------	*/
unsigned long _tac_conf_cfg_calculer_checksum_fichier(char *nomFichier)
{
long bcc=0;
FILE *desc_pt=NULL;
desc_pt=fopen(nomFichier,"r");
if(NULL!=desc_pt)
{
struct stat infos;
if(-1!=fstat(fileno(desc_pt),&infos))
{
long taille=infos.st_size;
long nbCarTot=0;
long nbCarLus=0;
	if(0<taille)
	{
	char buffer[1024];
		while((nbCarTot<taille)&&(-1!=nbCarLus))
		{
			nbCarLus=fread(buffer,sizeof(char),MIN(1024,taille-nbCarTot),desc_pt);
			if(-1!=nbCarLus)
			{
			int indice=0;
				for(indice=0;indice<nbCarLus;indice++)
				{
					bcc+=buffer[indice];
				}
				nbCarTot+=nbCarLus;
			}
			else
			{
				/* Il y a erreur */
				printDebug("_tac_conf_cfg_calculer_checksum_fichier: il y a erreur %ld/%ld\n",taille,nbCarTot);
				bcc=0;
			}
		}
	}

}
else
{
	printDebug("_tac_conf_cfg_calculer_checksum_fichier: erreur de fstat %d\n",errno);
}
fclose(desc_pt);
}
else
{
	printDebug("_tac_conf_cfg_calculer_checksum_fichier: erreur d'ouverture du fichier %s (errno: %d)\n",nomFichier,errno);
}
return bcc;
}

unsigned long tac_conf_cfg_calculer_checksum(int ficSel)
{
	unsigned long bcc = 0;
	/* Recuperation du nom du fichier */
	if((0<=ficSel)&&(TAC_CONF_NB_VERSIONS>ficSel))
	{
	T_tacConfFichiers config;
		tac_conf_cfg_lire_fichier(&config);
		if(0!=config.version[ficSel].nom[0])
		{
		char nomFichier[256];
			snprintf(nomFichier,256,"%s/%s",TAC_CONF_REP_VERSIONS,config.version[ficSel].nom);
			bcc=_tac_conf_cfg_calculer_checksum_fichier(nomFichier);

		}

	}
	/* Ouverture du fichier */
	/* Recuperation de la taille du fichier */
	/* Calcul du checksum */
	return bcc;
}

/* ---------------------------------
 * tac_conf_calculer_checksum_appli
 * ==========================
 * calcul du checksum pour la version
 * courante.
 * --------------------------------	*/

unsigned long tac_conf_cfg_calculer_checksum_appli(void)
{
	unsigned long bcc = 0;
#ifdef CPU432
	bcc=_tac_conf_cfg_calculer_checksum_fichier("pmvMain");
#else
	bcc=_tac_conf_cfg_calculer_checksum_fichier("linux/bin/pmvMain");
#endif
	return bcc;
}

/* --------------------------------
 * tac_conf_cfg_ecrire_checksum
 * ============================
 * Ecriture en flash de la nouvelle
 * valeur du checksum.
 * --------------------------------	*/

int tac_conf_cfg_ecrire_checksum(T_tacConfFichiers * confFichiers, int ficSel, unsigned long bcc)
{
	int retour = -1;
	if (NULL != confFichiers)
	{
		if ((0 <= ficSel) && (TAC_CONF_NB_VERSIONS > ficSel))
		{
			confFichiers->version[ficSel].bcc = bcc;
			if (-1 != tac_conf_cfg_ecr_fichier(confFichiers))
			{
				retour = 0;
			}
		}
	}
	/* ---------------------------------
	 * FIN DE tac_conf_cfg_ecrire
	 * --------------------------------     */
	return retour;
}

int tac_conf_cfg_lire_checksum(T_tacConfFichiers * confFichiers, int ficSel, unsigned long *bcc)
{
	int retour = -1;
	if (NULL != confFichiers)
	{
		if ((0 <= ficSel) && (TAC_CONF_NB_VERSIONS > ficSel))
		{
			*bcc = confFichiers->version[ficSel].bcc;
			retour = 0;
		}
	}
	/* ---------------------------------
	 * FIN DE tac_conf_cfg_ecrire
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * tac_conf_cfg_imprimer_fichier
 * =============================
 * Impression des la configuration
 * des fichiers.
 * --------------------------------	*/

static void tac_conf_cfg_imprimer_fichier(T_tacConfFichiers * confFichier)
{
	int ficSel;
	if (0 == (unsigned char) confFichier->courant[0])
	{
		printDebug("Pas de fichier courant\n");
	} else
	{
		printDebug("Fichier courant %s\n", confFichier->courant);
	}
	if (0 == (unsigned char) confFichier->exploit[0])
	{
		printDebug("Pas de fichier d'exploitation\n");
	} else
	{
		printDebug("Fichier d'exploitation %s\n", confFichier->exploit);
	}
	for (ficSel = 0; ficSel < TAC_CONF_NB_VERSIONS; ficSel++)
	{
		if (0 == (unsigned char) confFichier->version[ficSel].nom[0])
		{
			printDebug("Pas de version %d\n", ficSel);
		} else
		{
			printDebug("version %d nom %s et date %s\n", ficSel, confFichier->version[ficSel].nom, ctime(
					(const time_t *) &confFichier->version[ficSel].date));
		}
	}
	/* -------------------------------
	 * tac_conf_cfg_imprimer_fichier
	 * -------------------------------      */
}

/* --------------------------------
 * tac_conf_cfg_chercher_fichier
 * =============================
 * Recherche du fichier dans la
 * configuration.
 * --------------------------------	*/

int tac_conf_cfg_chercher_fichier(T_tacConfFichiers * confFichier, char *nomFichier)
{
	int ficTrouve = -1;
	int ficSel;
	for (ficSel = 0; ficSel < TAC_CONF_NB_VERSIONS; ficSel++)
	{
		if (0 != (unsigned char) confFichier->version[ficSel].nom[0])
		{
			/* Un emplacement disponible, on
			 * recupere le numero dispo.    */
			if (0 == strncmp(confFichier->version[ficSel].nom, nomFichier, TAC_CONF_FICHIER))
			{
				ficTrouve = ficSel;
				break;
			} /* endif(0==strncmp                             */
		} /* endif(0xFF==confFichier->ve  */
	} /* endfor(ficSel                                */
	/* -----------------------------
	 * END FOR tac_conf_cfg_chercher
	 * ---------------------------- */
	return ficTrouve;
}

/* --------------------------------
 * tac_conf_cfg_elire_fichier
 * =============================
 * Positionnement du nouveau fichier
 * d'exploitation.
 * --------------------------------	*/

int tac_conf_cfg_elire_fichier(T_tacConfFichiers * confFichier, char *nomFichier)
{
	int ficSel;
	ficSel = tac_conf_cfg_chercher_fichier(confFichier, nomFichier);
	/* Si le fichier est present dans
	 * la liste, on positionne son nom
	 * comme fichier d'exploitation.*/
	if (-1 != ficSel)
	{
		int indice;
		for (indice = 0; (indice < (TAC_CONF_FICHIER - 1)) && (indice < strlen(nomFichier)); indice++)
		{
			confFichier->exploit[indice] = nomFichier[indice];
		}
		/* On met des 0 apres                           */
		for (; (indice < TAC_CONF_FICHIER); indice++)
		{
			confFichier->exploit[indice] = 0;
		}
		tac_conf_cfg_ecr_fichier(confFichier);
	} /* endif(-1!=ficSel                             */
	/* -----------------------------
	 * END FOR tac_conf_cfg_chercher
	 * ---------------------------- */
	return ficSel;
}


/* --------------------------------
 * tac_conf_cfg_copier_fichier
 * ===========================
 * La fonction est utilisee pour recopier le
 * fichier a mettre en exploitaiton en lieu et
 * place du fichier application courant...
 * Il va fallori  travailler un peu plus sur
 * le sujet.
 * --------------------------------	*/

static int tac_conf_cfg_copier_fichier(T_tacConfFichiers * confFichier, int ficSel)
{
	int retour = -1;

	/* --------------------------------
	 * FIN DE tac_conf_cfg_copier_fichi
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * tac_conf_cfg_tester_fichier
 * ===========================
 * La procedure permet de tester la
 * version courante et la version
 * souhaitee pour l'exploitation.
 * Si il existe une version d'exploi-
 * tation et que celle ci est differente
 * de la version courante, on procede
 * automatiquement e la recopie de
 * l'une dans l'autre par bloc.
 * --------------------------------	*/

int tac_conf_cfg_tester_fichier(T_tacConfFichiers * confFichier)
{
	int retour = -1;
	/* --------------------------------
	 * TEST DE LA CONFIG
	 * --------------------------------     */
	if ((0 != (unsigned char) confFichier->exploit[0]) && (0 != confFichier->exploit[0]))
	{
		int ficSel;
		/* Il faut charger la nouvelle
		 * version.     */
		/* Recuperation du numero de
		 * fichier.     */
		for (ficSel = 0; ficSel < TAC_CONF_NB_VERSIONS; ficSel++)
		{
			if (0 == strncmp(confFichier->exploit, confFichier->version[ficSel].nom, TAC_CONF_FICHIER))
			{
				break;
			}
		} /* endfor(ficSel                                        */
		/* Allocation d'un bloc de memoire
		 * de taille suffisante */
		if (TAC_CONF_NB_VERSIONS > ficSel)
		{
			retour = tac_conf_cfg_copier_fichier(confFichier, ficSel);
		} /* endif(TAC_CONF_NB_VERSIONS>ficSel)       */
	} else
	{
		/* Il existe un fichier courant         */
		if (0!= (unsigned char) confFichier->courant[0])
		{

			retour = 0;
		} /* endif(0xFF!=confFichier->courant     */
		else
		{
			 int ficSel;
			 retour=1;
			 for(ficSel=0;ficSel<TAC_CONF_NB_VERSIONS;ficSel++)
			 {
			 if(0!=(unsigned char)confFichier->version[ficSel].nom[0])
			 {
			 retour=-1;
			 break;
			 }
			 }
		}
	} /* endif((0xFF!=confFichier->expol      */
	/* --------------------------------
	 * FIN DE tac_conf_cfg_tester_fichier
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * tac_conf_cfg_ajouter_fichier
 * =========================
 * Ajout d'un nouveau fichier dans
 * la configuration.
 * La fonction retourne le numero du
 * secteur dans lequel doit etre
 * sauvegarder le fichier telecharge.
 * --------------------------------	*/

int tac_conf_cfg_ajouter_fichier(T_tacConfFichiers * confFichier, char *nomFichier)
{
	int retour = -1;
	int ficSel;
	/* Si le fichier existe deja avec le
	 * meme nom, on le remplace.    */
	ficSel = tac_conf_cfg_chercher_fichier(confFichier, nomFichier);
	/* On regarde si un fichier a ete
	 * trouve, sinon, on recherche un
	 * emplacement disponible.              */

	if (-1 == ficSel)
	{
		for (ficSel = 0; ficSel < TAC_CONF_NB_VERSIONS; ficSel++)
		{
			if (0 == (unsigned char) confFichier->version[ficSel].nom[0])
			{
				/* Un emplacement disponible, on
				 * recupere le numero dispo.    */
				printDebug("Version libre %d\n", ficSel);
				break;
			} /* endif(0xFF==confFichier->ve  */
		} /* endfor(ficSel                                */
	} /* endif(TAC_CONF_NB                    */
	/* Si aucun fichier n'est dispo-
	 * nible, on elimine le plus
	 * ancien.      */
	if (TAC_CONF_NB_VERSIONS == ficSel)
	{
		int ficCour = 0;
		unsigned long dateCour = confFichier->version[ficCour].date;
		ficSel = 0;
		for (ficCour = 1; ficCour < TAC_CONF_NB_VERSIONS; ficCour++)
		{
			if (dateCour > confFichier->version[ficCour].date)
			{
				dateCour = confFichier->version[ficCour].date;
				ficSel = ficCour;
			} /* endif(dateCour>                              */
		} /* endfor(ficCour=1;ficCour<TA  */
	} /* endif(TAC_CONF                               */
	if (ficSel < TAC_CONF_NB_VERSIONS)
	{
		int indice;
		struct timespec heure;
		printDebug("version selectionnee %d\n", ficSel);
		/* Recopie du nom du fichier    */
		for (indice = 0; (indice < (TAC_CONF_FICHIER - 1)) && (indice < strlen(nomFichier)); indice++)
		{
			confFichier->version[ficSel].nom[indice] = nomFichier[indice];
		}
		/* On met des 0 apres                           */
		for (; (indice < TAC_CONF_FICHIER); indice++)
		{
			confFichier->version[ficSel].nom[indice] = 0;
		}
		printDebug("Le nom du fichier est %s\n", confFichier->version[ficSel].nom);
		/* Recopie de la date courante  */
		clock_gettime(CLOCK_REALTIME, &heure);
		confFichier->version[ficSel].date = heure.tv_sec;
		confFichier->version[ficSel].bcc = 0;
		/* Flash de la nouvelle config et
		 * en cas d'erreur, on ne retour-
		 * ne pas d'info. */
		if (-1 != tac_conf_cfg_ecr_fichier(confFichier))
		{
			/* Recup du numero de secteur   */
			retour = 1;
		} else
		{
			printDebug("Erreur d'ecriture de la configuration\n");
		} /* endif(-1!=tac_conf_cfg_ecr_fic       */
	} /* if(ficSel<TAC_CONF_NB_VERSIONS)      */
	/* --------------------------------
	 * FIN DE tac_conf_cfg_ajouter_fichier
	 * --------------------------------     */
	return retour;
}

static void _tac_conf_cfg_init(T_tacConfFichiers *confFichier)
{
	memset(confFichier,0,sizeof(T_tacConfFichiers));
}
/* --------------------------------
 * tac_conf_cfg_lire_fichier
 * =========================
 * Lecture de la configuration des
 * fichiers. Cette configuration est
 * utilisees pour les operations de
 * telechargement, de mise en service
 * et de lancement des applications.
 * --------------------------------	*/

int tac_conf_cfg_lire_fichier(T_tacConfFichiers * confFichier)
{
	int retour = -1;
	FILE *desc_pt=NULL;
	/* TODO: Introduire la nouvelle méthode de création des fichiers. */
	desc_pt=fopen(TAC_CONF_NOM_FICHIER,"r");
	if(NULL==desc_pt)
	{
		/* Le fichier n'a pu être ouver, on cree le fichier et on sauvegarde une
		 * configuration par defaut.
		 */
		/* Effectuer ici l'initialisation par defaut de la structure.. */
		_tac_conf_cfg_init(confFichier);
		/* Ecrire le fichier... */
		desc_pt=fopen(TAC_CONF_NOM_FICHIER,"w");
		if(NULL!=desc_pt)
		{
			if(1!=fwrite(confFichier,sizeof(T_tacConfFichiers),1,desc_pt))
			{
				/* Gros probleme, il a ete impossible d'ecrire... */
				fclose(desc_pt);
				desc_pt=NULL;
			}
			else
			{
				fclose(desc_pt);
				desc_pt=fopen(TAC_CONF_NOM_FICHIER,"r");


			}
		}

	}
	if(NULL!=desc_pt)
	{
		if(1!=fread(confFichier,sizeof(T_tacConfFichiers),1,desc_pt))
		{

		}
		else
		{
			/* Lecture avec succes du fichier... */
			retour = 1;
		}
		fclose(desc_pt);
	}
	/* --------------------------------
	 * FIN DE tac_conf_cfg_lire_fichier
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * tac_conf_cfg_ecr_fichier
 * ========================
 * Ecriture de la nouvelle configu-
 * ration de fichiers.
 * --------------------------------	*/

int tac_conf_cfg_ecr_fichier(T_tacConfFichiers * confFichier)
{
	int retour = 0;
	FILE *desc_pt=NULL;
	desc_pt=fopen(TAC_CONF_NOM_FICHIER,"w");
	if(NULL!=desc_pt)
	{
		if(1!=fwrite(confFichier,sizeof(T_tacConfFichiers),1,desc_pt))
		{
			/* Gros probleme, il a ete impossible d'ecrire... */
			printf("Erreur d'ecriture du fichier de configuration \n");
		}
		fclose(desc_pt);
	}

	return retour;
}

/* *********************************
 * FIN DE tac_conf_fichier.c
 * ********************************* */
