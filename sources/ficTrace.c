/*
 * ficTrace.c
 * ===================
 * Objet	: Le fichier defini toutes les procedures necessaires e la
 * 		gestion des fichiers de traces :
 * 		- ajout d'un nouvel enregistrement,
 * 		- suppression d'un enregistrement,
 * 		- consultation d'un enregistrement.
 *
 *  Created on: 20 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: ficTrace.c,v $
 * Revision 1.2  2018/06/04 08:38:39  xg
 * Passage en version V122
 *
 * Revision 1.9  2009/04/16 13:18:46  xag
 * Suppression warning de compilation.
 *
 * Revision 1.8  2009/03/12 09:15:06  xag
 * Ajout d'une trace de demarrage.
 *
 * Revision 1.7  2008/10/27 10:56:44  xag
 * *** empty log message ***
 *
 * Revision 1.6  2008/10/13 08:18:16  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.5  2008/09/29 07:58:52  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.4  2008/09/22 13:37:23  xag
 * Mise au point des traces.
 *
 * Revision 1.3  2008/09/22 07:53:43  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/09/12 14:57:11  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>

#include "kcommon.h"
#include "define.h"
#include "stdio.h"
#include "time.h"
#include "limits.h"
#include "values.h"
#include "standard.h"
#include "mon_debug.h"
#include "define.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "extLib.h"
#include "mon_inc.h"
#include "lcr_util.h"
#include "lcr_ident.h"
#include "lcr_cf3h.h"
#include "ficTrace.h"

#include "sramBdd.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/
/* Nombre maximal de caracteres dans
 * l'enregistrement de trace. Cette
 * valeur est liee au nombre maximal
 * de caracteres e imprimer */
#define FIC_TRACE_MAX 57

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

//extern Sram *sram_pt;

/* Pour l'heure, la simulation est
 * limitee e 200 vehicules sur dix
 * voies....    */
/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* --------------------------------
 * T_ficTraceEnreg
 * ===============
 * Infos sur un enregistrement :
 * - pointeur sur l'enregistrement
 *   precedent,
 * - sur l'enregistrement suivant,
 * - date de l'enregistrement.
 * --------------------------------	*/

typedef struct T_ficTraceEnreg
{
	char *suiv;
	char *prec;
	struct timespec date;
	T_ficTraceAdresse adresse;
} T_ficTraceEnreg;

/* --------------------------------
 * T_ficTraceRecherche
 * ===================
 * La structure est utilisee pour
 * l'impression des messages de traces
 * --------------------------------	*/

typedef struct T_ficTraceRecherche
{
	int indexFichier;
	//	struct zdf *info_pt;
	char *cour;
	struct T_ficTraceRecherche *suiv;
} T_ficTraceRecherche;

typedef struct T_ficTraceBuffer
{
	int nbCar;
	char buffer[FIC_TRACE_MAX];
} T_ficTraceBuffer;

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static T_ficTraceAdresse ficAdresse[NBPORT];
static int ficAdresseInit = 0;

/* ********************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ********************************	*/

static int _ficTraceSupprimerEnreg(int indexFichier, T_zdf *info);
static bool _ficTraceControlerPointeur(T_zdf *info,char *pointeur_pt);

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * ficTraceEcrire
 * ==============
 * Copie d'infos dans le fichier.
 * --------------------------------	*/

static int ficTraceEcrire(struct zdf *info, char *dest, const char *source, int taille)
{
	int retour = 0;
	char *deb = (char *) info->pt_deb;
	char *fin = deb + info->max_seq * info->sz_enr;
//	printDebug("Dans ficTraceEcrire dest %#x source %#x taille %d\n", (unsigned int) dest, (unsigned int) source,
//			taille);
	if ((dest >= deb) && (dest < fin))
	{
		/* TODO: Introduire une petite amelioration pour sauvegarder en bloc plutot
		 * que par caractere.
		 * -min de taille et fin-dest,
		 * -max de dest+taille-fin et 0
		 */
		unsigned long total_udw=(fin-dest);
		int tailleEcrite_dw=MIN(total_udw,taille);

		sramBddEcrireEnreg((int32)dest, tailleEcrite_dw,(void *) source);
		if (taille > (int)(total_udw))
		{
		int tailleRestante_dw=taille-(int)total_udw;
			sramBddEcrireEnreg((int32)deb,tailleRestante_dw,(void *)&source[tailleEcrite_dw]);
		}
	} else
	{
		retour = -1;
	} /* endif((dest>=deb     */
	/* ---------------------------------
	 * FIN DE ficTraceEcrire
	 * --------------------------------- */
	return retour;
}

/* --------------------------------
 * ficTraceLire
 * ============
 * Lecture depuis le fichier de traces
 * Valeur de retour :
 * - la fonction retourne -1 si
 *   l'adresse source est incorrecte.
 * --------------------------------	*/

static int ficTraceLire(struct zdf *info, char *dest, const char *source, int taille)
{
	int retour = 0;
	char *deb = (char *) info->pt_deb;
	char *fin = deb + info->max_seq * info->sz_enr;
	if ((source >= deb) && (source < fin))
	{
	unsigned long total_udw=fin-source;
//		printDebug("Lecture a l'adresse %#x de %d caracteres dans %#0x\n",
//				source,MIN(total_udw,taille),dest);
		int tailleEcrite_dw=MIN(total_udw,taille);
		sramBddLireEnreg((int32)source, tailleEcrite_dw,dest );
		if (taille > (int)(total_udw))
		{
//			printDebug("Second morceau adresse %#x de %d caracteres dans %#0x\n",
//					source,taille-(total_udw),dest);
		int tailleRestante_dw=taille-(int)total_udw;
			sramBddLireEnreg((int32)deb,tailleRestante_dw,&dest[tailleEcrite_dw]);
		}
	} else
	{
//		printDebug("Erreur source %#x debu %#x fin %#x\n",source,deb,fin);
		retour = -1;
	}
	/* endif((dest>=deb     */
	/* ---------------------------------
	 * FIN DE ficTraceLire
	 * --------------------------------- */
	return retour;
}

/* --------------------------------
 * ficTraceLireEnreg
 * =================
 * Lecture depuis le fichier de traces
 * Valeur de retour :
 * - la fonction retourne -1 si
 *   l'adresse source est incorrecte.
 * --------------------------------	*/

static int ficTraceLireEnreg(struct zdf *info, T_ficTraceEnreg * dest, const char *source)
{
	/* ---------------------------------
	 * FIN DE ficTraceLireEnreg
	 * --------------------------------- */
	return ficTraceLire(info, (char *) dest, source, sizeof(T_ficTraceEnreg));
}

/* --------------------------------
 * ficTraceSupprimerEnreg
 * ======================
 * Suppression d'un enregistrement.
 * Cette fonction est appelee norma-
 * lement lorsque l'on souhaite
 * ajouter un nouvel enregistrement
 * de trace et qu'il n'y a pas de
 * place disponible.
 * Valeur de retour :
 * - la fonction retourne -1 si une
 *   erreur est intervenue,
 *   par exemple si il n'y a plus
 *   d'enregistrement e supprimer.
 * --------------------------------	*/

static int _ficTraceSupprimerEnreg(int indexFichier, T_zdf *info) //struct zdf *infoIni)
{
	int retour = -1;
	if (info->nb_ecr != 0)
	{
		char *cour;
		int taille;
		/* Lecture de l'adresse de la trace... */
		retour=ficTraceLire(info, (char *) &cour, (char *) info->pt_prem, sizeof(char *));

		if (-1 != retour)
		{
		char *suiv=NULL;
		char *top = ((char *) info->pt_deb) + info->max_seq * info->sz_enr;
			if ((char *) info->pt_prem > cour)
			{
				taille = ((int) top - (int) info->pt_prem) + ((int) cour - (int) info->pt_deb);
			} else
			{
				taille = (int) cour - (int) info->pt_prem;
			}
			info->pt_prem = (STRING) cour;
			cour = NULL;
			/* Ecriture de l'adresse nulle dans l'enregistremetn suivant..... */

			suiv=((char *)info->pt_prem)+sizeof(char *);
			if(suiv>=top)
			{
				suiv-=info->max_seq * info->sz_enr;
			}
			_ficTraceControlerPointeur(info,suiv);
			if(-1==ficTraceEcrire(info,suiv, (char *) &cour, sizeof(char *)))
			{
				printDebug("_ficTraceSupprimerEnreg : erreur de suppression");
			}
			/* Supprimer un enregistrement */
			info->nb_ecr--;
			/* Recalculer la taille */
			info->num_ecr -= taille;
			sramBddEcrireFichier(indexFichier, info);
		}
	}
	/* --------------------------------
	 * FIN DE ficTraceSupprimerEnreg
	 * --------------------------------     */
	return retour;

}
static void				printEnreg(int fichier,T_ficTraceEnreg *enregPrec)
{
	printDebug("Enreg fichier %d\n",fichier);
	printDebug("Suiv : %#0lx\n",(long unsigned int)enregPrec->suiv);
	printDebug("Prec : %#0lx\n",(long unsigned int)enregPrec->prec);
}

static bool _ficTraceControlerPointeur(T_zdf *info,char *pointeur_pt)
{
int tailleMax = info->max_seq * info->sz_enr;
char *fin = ((char *) info->pt_deb) + tailleMax;
bool retour_b=true;
	if((pointeur_pt<info->pt_deb)||(pointeur_pt>=fin))
	{
		printDebug("_ficTraceControlerPointeur : probleme %0x pour deb %0x et fin %0x\n",
				pointeur_pt,(unsigned int)info->pt_deb,fin);
		retour_b=false;
	}
	return retour_b;
}
/* --------------------------------
 * ficTraceAjouterEnreg
 * ======================
 * Ajout d'un nouvel enregistrement
 * dans un fichier de traces.
 *
 * --------------------------------	*/

int ficTraceAjouterEnreg(int indexFichier, struct timespec *date, T_ficTraceAdresse * adresse, int nbCar, char *message)
{
	T_zdf infoTmp;
	T_zdf *info = &infoTmp;
	int retour = 0;
	sramBddLireFichier(indexFichier, info);
	{
		int tailleMax = info->max_seq * info->sz_enr;
		int nbCarEcrits = ((FIC_TRACE_MAX < nbCar) ? FIC_TRACE_MAX : nbCar);
		int tailleTot = nbCarEcrits + sizeof(T_ficTraceEnreg);

		while ((0 == retour) && ((tailleMax - info->num_ecr) < tailleTot))
		{
			retour = _ficTraceSupprimerEnreg(indexFichier, info);
		}
		/* Si il n'y a pas d'erreur, on copie
		 * le nouvel enregistrement. */
		if (0 == retour)
		{
			char *fin = ((char *) info->pt_deb) + tailleMax;
			char *suiv;
			T_ficTraceEnreg enreg;
			T_ficTraceEnreg enregPrec;
			/* Traitement de l'enregistrement       */

			if (0 != info->nb_ecr)
			{
//				printDebug("Il y a deja des nregistrements\n");

	//			printDebug("Lire    enreg : %d : pos %#08x\n",indexFichier,info->pt_ecr);
				retour = ficTraceLire(info, (char *) &enregPrec, (char *) info->pt_ecr, sizeof(T_ficTraceEnreg));
//				printDebug("Lecture a l'adresse %#0x\n",info->pt_ecr);
//				printEnreg(indexFichier,&enregPrec);
				if (-1 != retour)
				{
					/* Controler ici les pointeurs... .*/
					suiv=enregPrec.suiv;
					enreg.prec = (char *) info->pt_ecr;
					_ficTraceControlerPointeur(info,suiv);
					_ficTraceControlerPointeur(info,enreg.prec);
				}
			} else
			{
//				printDebug("Premier enregistrement info->pt_ecr %#0x\n",info->pt_ecr);
				suiv = (char *) info->pt_ecr;
				enreg.prec = NULL;
			} /*  endif(0!=info->nb_ecr                       */

			if (-1 != retour)
			{
				enreg.suiv = suiv + sizeof(T_ficTraceEnreg) + nbCarEcrits;
				if (enreg.suiv >= fin)
				{
					enreg.suiv = enreg.suiv - tailleMax;
				}
				enreg.date = *date;
				enreg.adresse = *adresse;
				/* Controler ici les pointeurs que l'on val ecrire... */
					_ficTraceControlerPointeur(info,enreg.suiv);
					_ficTraceControlerPointeur(info,enreg.prec);

				/* Ecriture entete du message           */
	//			printDebug("Ajouter enreg : %d : pos %#08x pre %#08x suiv %#08x\n",indexFichier,
	//					suiv,enreg.prec,enreg.suiv);
				ficTraceEcrire(info, suiv, (char *) &enreg, sizeof(T_ficTraceEnreg));
				info->pt_ecr = (STRING) suiv;
	//			printDebug("Evolution pt_ : %d : pos %#08x\n",indexFichier,info->pt_ecr);
				_ficTraceControlerPointeur(info,info->pt_ecr);
				suiv += sizeof(T_ficTraceEnreg);
				if (suiv >= fin)
				{
					suiv -= tailleMax;
				}
				_ficTraceControlerPointeur(info,suiv);
				ficTraceEcrire(info, suiv, message, nbCarEcrits);
//				printf("TRACE : Ecriture a l'adresse %08x de %s\n",suiv,message);
				info->num_ecr += sizeof(T_ficTraceEnreg) + nbCarEcrits;
				info->nb_ecr++;
			}
		} /* endif(0==retour)                                     */
	}
//	printDebug("On va ecrire info->pt_ecr %#0x\n",info->pt_ecr);/
	sramBddEcrireFichier(indexFichier, info);
	/* --------------------------------
	 * FIN DE ficTraceAjouterEnreg
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * ficTraceImprimerEnreg
 * =====================
 * Impression d'un enregistrement
 * Entree :
 * - un pointeur sur la description
 *   du fichier,
 * - un pointeur sur l'enregistrement
 * --------------------------------	*/

static int ficTraceImprimerEnreg(INT las, INT mode, STRING buffer, INT * nbCar, INT * bloc, INT derBlk,
		struct usr_ztf *ptMess, int indexFichier, char *debut)
{
	int retour = 0;
	T_ficTraceEnreg enreg;
	int tailleMax;
	int taille;
	/* Nombre de caracteres de trace e
	 * ecrire. */
	int indiceMax = FIC_TRACE_MAX;
	T_zdf infoTmp;
	T_zdf *info = &infoTmp;
	/* Lecture de l'entete de l'enregis-
	 * trement. */
	sramBddLireFichier(indexFichier, info);
	tailleMax = info->max_seq * info->sz_enr;
//	printDebug("Nombre de seq %ld taille %ld total : %d\n", info->max_seq, info->sz_enr, tailleMax);
	retour = ficTraceLireEnreg(info, &enreg, debut);
	if (-1 != retour)
	{
		if (enreg.suiv < debut)
		{
			taille = (int) enreg.suiv + tailleMax - (int) debut;
		} else
		{
			taille = (int) enreg.suiv - (int) debut;
		}
		/* Imprimer l'heure du message          */
		{
			struct tm enregDate;
			localtime_r(&enreg.date.tv_sec, &enregDate);
			tst_send_bloc(las, mode, buffer, nbCar, bloc, derBlk, ptMess, "%02d/%02d %02d:%02d:%02d:%02d %c ",
					enregDate.tm_mday, enregDate.tm_mon + 1, enregDate.tm_hour, enregDate.tm_min, enregDate.tm_sec,
					enreg.date.tv_nsec / 10000000, info->ty_seq);
		}
		/* Traitement de l'adresse en cas
		 * d'extension du code d'erreur.        */
		if (extGet(EXT_ESA))
		{
			switch (enreg.adresse.type)
			{
			case E_ficTraceAdresseFip:
			{
				unsigned char *ptAd = (unsigned char *) &enreg.adresse.val.fip.numero;
				tst_send_bloc(las, mode, buffer, nbCar, bloc, derBlk, ptMess, "%d.%d.%d.%d/%d/%06x ", ptAd[0], ptAd[1],
						ptAd[2], ptAd[3], enreg.adresse.val.fip.module, enreg.adresse.val.fip.ident);
			}
				break;
			case E_ficTraceAdresseStandard:
			{
				char *ptAd = &enreg.adresse.val.standard[0];
				tst_send_bloc(las, mode, buffer, nbCar, bloc, derBlk, ptMess, "%c%c%c%c%c%c ", ptAd[0], ptAd[1],
						ptAd[2], ptAd[3], ptAd[4], ptAd[5]);
			}
				break;
			default:
			{
				tst_send_bloc(las, mode, buffer, nbCar, bloc, derBlk, ptMess, "pb %03d ", enreg.adresse.type);
			}
				break;
			}
			/* Dans le cas de l'extension code
			 * d'erreur, on ecrit moins de carac-
			 * teres.       */
			indiceMax -= 7;
		}
		/* Ecriture de la trace proprement
		 * dite.        */
		{
			char *pt_mess = debut + sizeof(T_ficTraceEnreg);
			int tailleMess = taille - sizeof(T_ficTraceEnreg);

			char tentative[FIC_TRACE_MAX];
			int indice;
				if (pt_mess >= (((char *) info->pt_deb) + tailleMax))
				{
					pt_mess -= tailleMax;
				}
//				printf("TRACE : Lecture de message a l'adresse %#08x\n",pt_mess);
				retour = ficTraceLire(info, tentative, pt_mess, MIN(tailleMess,FIC_TRACE_MAX));
				if (-1 != retour)
				{
					for (indice = 0; (indice < tailleMess) && (indice < indiceMax); indice++)
					{
						tst_send_bloc(las, mode, buffer, nbCar, bloc, derBlk, ptMess, "%c", tentative[indice]);
					}
				}
		}
		/* Ecriture de la fin de ligne.         */
		tst_send_bloc(las, mode, buffer, nbCar, bloc, derBlk, ptMess, "\n\r");
		/* --------------------------------
		 * FIN DE ficTraceImprimerEnreg
		 * --------------------------------     */
	}
	return retour;
}

/* --------------------------------
 * ficTraceTester
 * ==============
 * Test du fichier de trace pour
 * savoir si il est correct. Si
 * ce n'est pas le cas, on opere
 * une reinitialisation
 * --------------------------------	*/

int ficTraceTester()
{
	int retour = 0;
	int fic;
	for (fic = 0; (fic < MAXFIC) && (0 == retour); fic++)
	{
		T_zdf infoTmp;
		T_zdf *info = &infoTmp;
		sramBddLireFichier(fic, info);
		printf("Sequencement du fichier : %c\n", info->ty_seq);

		if (info->ty_seq == cf3_get_char_seq(fic))
		{
			if (0 != info->nb_ecr)
			{
				char *cour = (char *) info->pt_ecr;
				while ((NULL != cour) && (0 == retour))
				{
					T_ficTraceEnreg enreg;
					retour = ficTraceLire(info, (char *) &enreg, cour, sizeof(T_ficTraceEnreg));

					if (-1 != retour)
					{
						cour = enreg.prec;
					}
					else
					{
						printf("Erreur sur un enregistrement \n");
					}
				}
			}
		} else
		{
			retour = -1;
		}

	}
	return retour;
}

/* --------------------------------
 * ficTraceImprimer
 * ================
 * Impression des dates qui
 * correspondent aux criteres.
 * Normalement, plusieurs autres
 * parametres seront utilises pour
 * permettre l'affichage.
 * --------------------------------	*/

void ficTraceImprimer(INT las, INT mode, STRING buffer, INT * nbCar, INT * bloc, INT derBlk, struct usr_ztf *ptMess,
		int nombre, char nature, int dateDebOk, struct tm *dateDeb, int dateFinOk, struct tm *dateFin)
{
	int debut = 0;
	int fin = 0;
	T_ficTraceRecherche *teteEnreg = NULL;
	int retour = 0;
	/* Travail sur la nature des mesures
	 * a afficher. */
	if ('*' == nature)
	{
		debut = 0;
		fin = MAXFIC;
	} else
	{
		int fic;
		for (fic = 0; fic < MAXFIC; fic++)
		{
			T_zdf infoTmp;
			T_zdf *info = &infoTmp;
			sramBddLireFichier(fic, info);
			printDebug("Fichier %d nature %c\n", fic, info->ty_seq);
			if (info->ty_seq == nature)
			{
				printDebug("J'ai trouve le fichier %d\n", fic);
				debut = fic;
				fin = fic + 1;
				break;
			}
		}
	}
	/* Cas particulier ou un nombre est
	 * precise. Dans ce cas, Il n'y a pas
	 * de date de debut mais uniquement une
	 * date de fin. On s'evertue alors a
	 * recomposer l'interrogation.  */
	/* Dans ce cas, il est tenu compte de
	 * la premiere date qui est precisee
	 * et celle ci devient la date de
	 * fin. */
	if (nombre)
	{
		dateFinOk = dateDebOk;
		dateFin = dateFin;
		dateDebOk = 0;
	}
	/* Initialisation de la structure
	 * contenant les pointeurs sur les
	 * traces courantes. Ce travail n'est
	 * realise que si la nature precisee
	 * est correcte. */
	if (debut < fin)
	{
		T_ficTraceRecherche *tabFic;
		/* Allocation des donnees utilisees
		 * pour les traitements. */
		/*  Realiser ici l'allocation et l'initialisation des donnees utili-
		 *  sees pour les tests. Il s'agit d'une structure par fichier de
		 *  donnees dans lequel on va placer un pointeur sur le fichier et
		 *  un pointeur sur l'enregistrement courant. */
		tabFic = (T_ficTraceRecherche *) kmmAlloc(NULL,(fin - debut)
				* sizeof(T_ficTraceRecherche));
		if (NULL != tabFic)
		{
			int notFin = 1;
			int nbEnreg = 0;
			{
				int indice;
				for (indice = 0; indice < (fin - debut); indice++)
				{
					T_zdf infoTmp;
					T_zdf *info = &infoTmp;
					tabFic[indice].indexFichier = debut + indice;
					sramBddLireFichier(debut + indice, info);

					if (0 == info->nb_ecr)
					{
						tabFic[indice].cour = NULL;
					} else
					{
						tabFic[indice].cour = (char *) info->pt_ecr;
						printDebug("Prem enreg %#x\n", (unsigned int) tabFic[indice].cour);
					}
				} /* endfor(indice=0      */
			}

			if (dateFinOk)
			{
				int ficCour;
				struct timespec dateFinSec =
				{ 0, 0 };

				dateFinSec.tv_sec = mktime(dateFin);
				for (ficCour = 0; (ficCour < (fin - debut)) && (0 == retour); ficCour++)
				{
					T_zdf infoTmp;
					T_zdf *info = &infoTmp;
					sramBddLireFichier(tabFic[ficCour].indexFichier, info);
					/* Pour tous les enregistrement plus recents que la date
					 * de fin, on saute et on met e jour la variable temporaire */
					while ((NULL != tabFic[ficCour].cour) && (0 == retour))
					{
						T_ficTraceEnreg enreg;
						retour = ficTraceLire(info, (char *) &enreg, tabFic[ficCour].cour, sizeof(T_ficTraceEnreg));

						if (-1 != retour)
						{
							if ((enreg.date.tv_sec > dateFinSec.tv_sec) || ((enreg.date.tv_sec == dateFinSec.tv_sec)
									&& (enreg.date.tv_nsec > dateFinSec.tv_nsec)))
							{
								tabFic[ficCour].cour = enreg.prec;
							} else
							{
								/* L'enregistrement correspond au
								 * critere, c'est tout bon.     */
								break;
							} /* endif(enreg.date>dateFinSec  */
						}
						else
						{
							printDebug("Probleme avec l'enregistrement %#0x du fichier %d\n",(unsigned int)tabFic[ficCour].cour, tabFic[ficCour].indexFichier);
						}
					} /* endwhile(NULL!=      */
				}
			}

			while (notFin)
			{
				int ficCour;
				char *precSel = NULL;
				int ficSel = -1;
				struct timespec dateSel;
				struct timespec dateCour;
				double dateDiff = DBL_MAX;
				clock_gettime(CLOCK_REALTIME, &dateCour);
				/* Recherche de l'enregistrement le plus recent que l'on puisse
				 * traiter */
				for (ficCour = 0; (ficCour < (fin - debut)) && (0 == retour); ficCour++)
				{
					/* Si l'enregistrement du fichier en cours de traitement est
					 * plus recent, il devient l'enregistrement elu... */
					if ((NULL != tabFic[ficCour].cour) && (0 == retour))
					{
						T_ficTraceEnreg enreg;
						double diffCour;
						T_zdf infoTmp;
						T_zdf *info = &infoTmp;
						tabFic[ficCour].indexFichier = debut+ficCour;
						sramBddLireFichier(debut+ficCour, info);

						retour = ficTraceLireEnreg(info, &enreg, tabFic[ficCour].cour);
						if (-1 != retour)
						{
							diffCour = difftime(dateCour.tv_sec, enreg.date.tv_sec);
							/* Test de la date de l'enregis-
							 * trement.     */
							if (diffCour < dateDiff)
							{
								dateDiff = diffCour;
								ficSel = ficCour;
								dateSel = enreg.date;
								precSel = enreg.prec;
							} else if (diffCour == dateDiff) /* Cas de la meme sec.  */
							{
								if (ficSel != -1)
								{
									if (dateSel.tv_nsec < enreg.date.tv_nsec)
									{
										ficSel = ficCour;
										dateSel = enreg.date;
										precSel = enreg.prec;
									}
								}
							} /* endif(diffCour<dateDiff                      */
						} /* endif(NULL!=tab[ficCour].            */
					}
				}
				if (-1 != ficSel)
				{
//					printDebug("Enreg sel %#x\n", (unsigned int) tabFic[ficSel].cour);
					if (dateDebOk)
					{
						struct timespec dateDebSec =
						{ 0, 0 };

						dateDebSec.tv_sec = mktime(dateDeb);
						/* Comparer la date de l'enregis-
						 * trement trouve */
						if ((dateDebSec.tv_sec > dateSel.tv_sec) || ((dateDebSec.tv_sec == dateSel.tv_sec)
								&& dateDebSec.tv_nsec > dateSel.tv_nsec))
						{
							/* La date de l'enregistrement est
							 * plus ancienne, l'enregistrement
							 * n'est pas valide et on fini donc
							 * la lecture. */
							ficSel = -1;
						}
					}
					/* La date est correcte.                        */
					if (-1 != ficSel)
					{
						T_ficTraceRecherche *enregCour;
						/* Allocation et initialisation d'un
						 * nouvel enregistrement */
						enregCour = (T_ficTraceRecherche *) kmmAlloc(NULL,sizeof(T_ficTraceRecherche));
						enregCour->indexFichier = tabFic[ficSel].indexFichier;
						enregCour->cour = tabFic[ficSel].cour;
						enregCour->suiv = teteEnreg;
						/* Ajouter l'enregistrement e la
						 * liste des enregistrements. */
						teteEnreg = enregCour;
						/* Mise e jour pour la suite de la recherche. */
						tabFic[ficSel].cour = precSel;
						nbEnreg++;
						notFin = (nbEnreg != nombre);
					} else
					{
						notFin = 0;
					} /* endif(trouve                                 */
				} else
				{
					notFin = 0;
				} /* endif(trouve                                         */
			}
			/* Affichage des traces par enregis-
			 * trement et dans l'ordre chronologi-
			 * que. */
			 printDebug("Avant impression enreg : %#0x et retour %d\n",(unsigned int)teteEnreg,retour);
			while ((teteEnreg != NULL) && (0 == retour))
			{
				T_ficTraceRecherche *cour = teteEnreg;
				teteEnreg = cour->suiv;
				retour = ficTraceImprimerEnreg(las, mode, buffer, nbCar, bloc, derBlk, ptMess, cour->indexFichier,
						cour->cour);
				kmmFree (NULL,(void *) cour);
			}
			kmmFree (NULL,(void *) tabFic);
		} else
		{
			/* Allocation memoire impossible        */
		} /* if(NULL!=tabFic)                                     */
	} /* endif(debut<fin                                      */
	/* --------------------------------
	 * FIN DE ficTraceImprimer
	 * --------------------------------     */
}

/* --------------------------------
 * ficTraceEnregistrerQuestion
 * ===========================
 * La fonction enregistre dans
 * le fichier de traces des questions
 * la commande tapee par l'utilisa-
 * teur. L'enregistrement dans le
 * fichier des questions est realise
 * quelque soit la nature de la reponse.
 * --------------------------------	*/

void ficTraceEnregistrerQuestion(T_ficTraceAdresse * adresse, struct timespec *dateCommande, char *commande,
		int nbCarCmd)
{
	T_lcr_util_buffer bufCom;
	lcr_util_buffer_init(&bufCom, FIC_TRACE_MAX);
	/* Analyse du debut de la commande pour detecter une eventuelle iden
	 * tification. */
//	printDebug("Substitution de ID\n");
	commande = lcr_ident_substituer(commande, nbCarCmd, &nbCarCmd, &bufCom);
	/* Si la commande est un CFID, on passe direct e la fin de la fonction
	 * sans realiser d'enregistrement.*/
	if (0 != strncmp(commande, "CFID", strlen("CFID")))
	{
		/* Sinon, on recopie la commande en
		 * traquant une eventuelle identifi-
		 * cation une identification en mode
		 * parametre dans la commande et on
		 * realise les substitutions qui vont
		 * bien puis. */

//		printDebug("Substitution de ID en parametre\n");
		commande = lcr_ident_param_substituer(commande, nbCarCmd, &nbCarCmd, &bufCom);
		ficTraceAjouterEnreg(F_IQ, dateCommande, adresse, lcr_util_buffer_lire_nb_car(&bufCom),
				lcr_util_buffer_lire_buffer(&bufCom));
	}
	/* --------------------------------
	 * FIN DE ficTraceEnregistrerQuestion
	 * --------------------------------     */
}

/* --------------------------------
 * ficTraceEnregistrerReponse
 * ===========================
 * La fonction enregistre dans le
 * fichier de traces des reponses
 * --------------------------------	*/

void ficTraceEnregistrerReponse(T_ficTraceAdresse * adresse, struct timespec *dateReponse, T_lcr_util_buffer * reponse,
		char *commande, int nbCarCmd)
{
	T_lcr_util_buffer bufCom;
	/* Init du buffer.                                      */
	lcr_util_buffer_init(&bufCom, FIC_TRACE_MAX);
	/* Substitution de l'identification     */
//	printDebug("Substitution de ID\n");
	commande = lcr_ident_substituer(commande, nbCarCmd, &nbCarCmd, &bufCom);
	/* Pas d'enregistrement en cas de
	 * commande TRACE ou CFID.      */
	if (0 != strncmp(commande, "CFID", strlen("CFID")) && (0 != strncmp(commande, "TRACE", strlen("TRACE"))))
	{
		ficTraceAjouterEnreg(F_IR, dateReponse, adresse, lcr_util_buffer_lire_nb_car(reponse),
				lcr_util_buffer_lire_buffer(reponse));
	}
	/* --------------------------------
	 * FIN DE ficTraceEnregistrerReponse
	 * --------------------------------     */
}

void ficTraceEnregistrerReponseBis(T_ficTraceAdresse * adresse, struct timespec *dateReponse,
		char *rep,int nbCarRep,
		char *commande, int nbCarCmd)
{
T_lcr_util_buffer bufCom;
										/* Init du buffer.					*/
	lcr_util_buffer_init(&bufCom,FIC_TRACE_MAX);
										/* Substitution de l'identification	*/
//	printDebug("Substitution de ID\n");
	commande=lcr_ident_substituer(commande,nbCarCmd,&nbCarCmd,&bufCom);
										/* Pas d'enregistrement en cas de
										 * commande TRACE ou CFID. 	*/
	if (0 != strncmp(commande, "CFID", strlen("CFID")) && (0 != strncmp(commande, "TRACE", strlen("TRACE"))))
	{
		ficTraceAjouterEnreg(F_IR, dateReponse, adresse,MIN(78,nbCarRep),rep);
	}
										/* --------------------------------
										 * FIN DE ficTraceEnregistrerReponse
										 * --------------------------------	*/
}


/* --------------------------------
 * ficTraceEnregistrerCommande
 * ===========================
 * Si la reponse est correcte, on
 * enregistre la reponse dans le
 * fichier des commandes.
 * --------------------------------	*/

void ficTraceEnregistrerCommande(T_ficTraceAdresse * adresse, struct timespec *dateCommande, char *commande,
		int nbCarCmd)
{
	T_lcr_util_buffer bufCom;
	/* Initialisation du buffer.            */
	lcr_util_buffer_init(&bufCom, FIC_TRACE_MAX);
	/* Analyse du debut de la commande
	 * pour detecter une eventuelle iden
	 * tification. */
	commande = lcr_ident_substituer(commande, nbCarCmd, &nbCarCmd, &bufCom);
	/* On ne traite pas CFID.                       */
	if (0 != strncmp(commande, "CFID", strlen("CFID")))
	{
		commande = lcr_ident_param_substituer(commande, nbCarCmd, &nbCarCmd, &bufCom);
		ficTraceAjouterEnreg(F_IC, dateCommande, adresse, lcr_util_buffer_lire_nb_car(&bufCom),
				lcr_util_buffer_lire_buffer(&bufCom));
		/* Si la commande n'est pas une
		 * demande de traces, on enregistre le
		 * resultat de celle ci. */
	} /* endif(0!=strncmp                                     */
	/* ---------------------------------
	 * FIN DE ficTraceEnregistrerCommande
	 * --------------------------------     */
}

#ifdef PMV
void ficTraceEnregistrerPosition(T_ficTraceAdresse *adresse, STRING chaine)
{
	struct timespec heure;
	clock_gettime(CLOCK_REALTIME, &heure);
	ficTraceAjouterEnreg(F_IP, &heure, adresse, strlen((char*) chaine), (char *) chaine);
}

#endif

/* ----------------------------------
 * ficTraceAdresseSetStandard
 * ==========================
 * --------------------------------	*/

void ficTraceAdresseSetStandard(T_ficTraceAdresse * ad, char *valeur)
{
	ad->type = E_ficTraceAdresseStandard;
	memcpy(&ad->val.standard[0], valeur, TRACE_NB_CAR_ADRESSE);
}

static void ficTraceAdresseInit()
{
	int indice;
	for (indice = 0; indice < NBPORT; indice++)
	{
		ficTraceAdresseSetStandard(&ficAdresse[indice], "I00000");
	}
	ficAdresseInit = 1;
}
void ficTraceSetAdressePort(int port, T_ficTraceAdresse *ad)
{
	if (0 == ficAdresseInit)
	{
		ficTraceAdresseInit();
	}
	ficAdresse[port] = *ad;
}

void ficTraceGetAdressePort(int port, T_ficTraceAdresse *ad)
{
	if (0 == ficAdresseInit)
	{
		ficTraceAdresseInit();
	}
	*ad = ficAdresse[port];
}

/* ----------------------------------
 * ficTraceAdresseSetFip
 * =====================
 * --------------------------------	*/

void ficTraceAdresseSetFip(T_ficTraceAdresse * ad, ULONG num, ULONG id, char module)
{
	ad->type = E_ficTraceAdresseFip;
	ad->val.fip.numero = num;
	ad->val.fip.ident = id;
	ad->val.fip.module = module;
}
