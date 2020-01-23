/*
 * SupInitLib.c
 * ===================
 *
 * Objet	: Procedures pour l'initialisation de l'application.
 *
 *  Created on: 20 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: SupInitLib.c,v $
 * Revision 1.2  2018/06/04 08:38:38  xg
 * Passage en version V122
 *
 * Revision 1.7  2009/08/24 08:26:07  xgaillard
 * Suppression de l'appel a ajouterModuleApplication qui n'etait pas utilisé.
 *
 * Revision 1.6  2008/10/03 16:21:46  xag
 * *** empty log message ***
 *
 * Revision 1.5  2008/09/29 07:58:51  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.4  2008/09/22 13:39:02  xag
 * Ajout du parametre performance pour l'impression par defaut des traces de performance.
 *
 * Revision 1.3  2008/09/22 07:53:42  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/09/12 14:57:10  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* ********************************
 * LISTE DES INCLUDES
 * ********************************	*/

#include "stdarg.h"
#include "config.h"
#include "Superviseur.h"
#include "time.h"
#include "Semaphore.h"
#include "standard.h"                  /* redefinition des types            */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes         */
#include "x01_str.h"                   /* definition des structures globales */
#include "x01_var.h"                   /* definition des variables globales */
#include "vct_str.h"
#include "x01_vcth.h"
#include "xdg_var.h"
#include "rec_main.h"
#include "Pipe.h"
#include "mon_debug.h"
#include "supInitLib.h"
#include "supRun.h"
#include "abtLib.h"
#include "stAlLib.h"
#include "pip_var.h"
#include "tac_ctrl.h"
#include "semLib.h"
#include "tempLib.h"
#include "lcr_pcp.h"

#include "configurationSram.h"
#include "sramMessage.h"
#include "sramBdd.h"

/* ********************************
 * VARIABLES GLOBALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* ---------------------------------
 * T_supTache
 * ===========
 * Structure creee pour rendre un
 * peu plus lisible le code de
 * lancement des taches.
 * --------------------------------	*/

typedef struct supTache
{
	char *nom;
	int seq;
	int syst; /* La tache doit s'executer aussi en mode systeme !*/
	short prio;
	FUNCPTR fonction;
	int arg;
	FUNCPTR init; /* Fonction a appeler a l'initialisation de la tache. */
	FUNCPTR term; /* Fonction a appeler a la terminaison de la tache. */
} T_supTache;

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static T_supTache listeTaches[] =
{
/*
 {"lon_main", 1,1, 100, (FUNCPTR) lon_main, 0},
 */
{ "tac_ctrl", 1, 1, 100, (FUNCPTR) tac_ctrl_main, 0, NULL, NULL },
{ "tac_las", 1, 0, 100, (FUNCPTR) tac_las, 0, NULL, NULL },
/*
 {"pip_main", 1,1, 100, (FUNCPTR) pip_main, 0},
 */
{ "LCR_SERIE_1", 1, 0, 100, (FUNCPTR) pcp_main, PORT_SERIE, NULL, NULL },
//{ "LCR_SERIE_2", 1, 0, 100, (FUNCPTR) pcp_main, PORT_SERIE_2, NULL, NULL },
{ "LCR_IP", 1, 0, 100, (FUNCPTR) pcp_main, PORT_IP_LCR, NULL, NULL },
{ "LCR_IP_2", 1, 0, 100, (FUNCPTR) pcp_main, PORT_IP_LCR_2, NULL, NULL },
{ "LCR_FIP", 1, 0, 100, (FUNCPTR) pcp_main, PORT_FIP, NULL, NULL },
{ "LCR_PASS", 1, 0, 100, (FUNCPTR) pcp_main, PORT_PASS, NULL,NULL },
{ "lumTraiteur", 0, 0, 100, (FUNCPTR) lumTraiteur_main, 0, lumTraiteur_init, lumTraiteur_term },
{ "sequenceur", 0, 0, 100, (FUNCPTR) sequenceur_main, 0, sequenceur_init, sequenceur_term },

{ NULL, 1, 1, 100, (FUNCPTR) NULL, 0, NULL, NULL } };

/* ********************************
 * PROTOTYPAGE DES FONCTIONS
 * ********************************	*/

/* ********************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ********************************	*/

static int _supInitTache(T_supGestion *, char *, int, short, FUNCPTR, int);
static void _supInitPrintBanner(const char *, char *);

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

static void _supInitPrintBanner(const char *chaine, char *resultat)
{
	printDebug("*********************************************************\n");
	printDebug(chaine, resultat);
	printDebug("*********************************************************\n");
}

/* ----------------------------------
 * tac_ctrl_init_sram
 * ==================
 * Initialisation de la memoire stati-
 * que.
 * ------------------------------------	*/

static void supInitSram()
{

	//#ifdef LINUX
	int32 tailleCour_dw = 0;
	/* Dans le cas de linux, il faut à priori utiliser une partie de la sram */
	printf("Initialisation Sram.....  : adresse %#08x\n", SRAM_EXT_BASE);
	tailleCour_dw = configSramInit((void *) SRAM_EXT_BASE);

	/* Initialisation de la zone des messages */
	printf("Initialisation message    : adresse %#08x\n", (SRAM_EXT_BASE + tailleCour_dw));
	tailleCour_dw += sramMessageInit((void *) (SRAM_EXT_BASE + tailleCour_dw));

	/* Initialisation des pointeurs sur la base de donnee. */

	printf("Initialisation de la base : adresse %#08x, tailleRestante %x\n", (SRAM_EXT_BASE + tailleCour_dw),
			SRAM_EXT_SIZE - tailleCour_dw);

	tailleCour_dw += sramBddInit((void *) (SRAM_EXT_BASE + tailleCour_dw), SRAM_EXT_SIZE - tailleCour_dw);

	printf("Fin d'initialisation, tailleCour_dw %0x(pour %x)\n", tailleCour_dw,SRAM_EXT_SIZE);

	/*
	 sram_pt=sramNew((void *)SRAM_EXT_BASE,SRAM_EXT_SIZE);
	 sram_mem 		= (UINT8 *)SRAM_EXT_BASE;
	 vct_mem_size 	= SRAM_EXT_SIZE;


	 sram_bdd = (T_zdf *) (sram_mem + sizeof (T_x01_sram_var));
	 bdSzMem = vct_mem_size - MAXFIC * sizeof (T_zdf) - sizeof (T_x01_sram_var);

	 sram_add_mem = (LONG) (sram_mem + MAXFIC * sizeof (T_zdf) + sizeof (T_x01_sram_var));
	 sram_var=(T_x01_sram_var *)sram_mem;

	 */

	eriInit();

	tempInit();



}
/* --------------------------------
 * supInitVar
 * ==========
 * Initialisation des variables du
 * module.
 * Lancement de la tache de surveil-
 * lance.
 * --------------------------------	*/

int supInitVar(T_supGestion * pGes)
{
	/* --------------------------------     */
	/* DECLARATION DES VARIABLES            */
	/* --------------------------------     */
	int retour = OK;
	int indice;
	int numPort;
	int32 tickCour_dw=ktickGetCurrent();

	/* --------------------------------     */
	/* INIT DE LA MEMOIRE                           */
	/* --------------------------------     */
	supInitSram();

	/* --------------------------------     */
	/* PHASE D'INITIALISATION DES VAR       */
	/* --------------------------------     */
	pGes->nbTache = 0;
	monDebugInit();
	supRunInit();
	pGes->appli = 0;
	for (indice = 0; indice < SUP_MAX_TACHE; indice++)
	{
		T_supDescTache *tache = &pGes->Tache[indice];
		tache->tacVersMon = NULL;
		tache->monVersTac = NULL;
		tache->tid = 0;
		tache->ptEntree = NULL;
		tache->sequence = 0;
		tache->active = 0;
		tache->bidon = 0;
		tache->position = 0;
		tache->numero = indice;
		tache->nomTache[0] = 0;
		tache->lastTick_dw = tickCour_dw;
	}

	_supInitPrintBanner("				Phase d'initialisation                   \n", "");
	/* Initialisation du mecanisme des
	 * alertes.     */
	stAlInit();
	/* --------------------------------     */
	/* CREATION DE LA TACHE ETAT SYSTEME */
	/* --------------------------------     */
	/* La tache a en charge la gestion
	 * du watchDog de l'application.
	 */
	tacheSpawn("EtatSysteme", 50, 0, 1000, (FUNCPTR) etaSyst, (int) pGes, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	/* --------------------------------
	 * INIT DES LIAISONS PORT
	 * --------------------------------     */
	for (numPort = 0; numPort < NBPORT; numPort++)
	{
		cnxLiaison[numPort] = NULL;
		xdg_las[numPort].conn_pt = NULL;
		MqLasDial[numPort] = -1;
		MqDialLas[numPort] = -1;
		x01_ancrage_TEDI_esclave_em[numPort] = ERROR;
		x01_ancrage_TEDI_esclave_rec[numPort] = ERROR;
		ancrage_pcp_lecmess[numPort] = ERROR;
	} /* endfor(numPort=0;numPort<NPOR        */
	ancrageTedi_ctrl = ERROR;
	x01_ancrage_interne = ERROR;
	/* ---------------------------------
	 * FIN DE supInitVar
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * supInitAncrage
 * ==============
 * Initialisation des ancrages. Les ancrages
 * sont les mecanismes de communication inter
 * taches.
 * --------------------------------	*/

int supInitAncrage(void)
{
	int retour = OK;
	int i;
	char NomPipe[50];
	/* --------------------------------
	 * INIT DES ANCRAGES DIVERS
	 * --------------------------------     */
	ancrageTedi_ctrl = SysMQCreer();
	if (ancrageTedi_ctrl == ERROR)
	{
		retour = ERROR;
	}

	x01_ancrage_interne = SysMQCreer();
	if (x01_ancrage_interne == ERROR)
	{
		retour = ERROR;
	}
	/* --------------------------------
	 * ANCRAGES POUR LES PORTS
	 * --------------------------------     */
	for (i = 0; (i < NBPORT) && (ERROR != retour); i++)
	{
		x01_ancrage_TEDI_esclave_em[i] = SysMQCreer();
		if (x01_ancrage_TEDI_esclave_em[i] == ERROR)
		{
			retour = ERROR;
		}
		x01_ancrage_TEDI_esclave_rec[i] = SysMQCreer();
		if (x01_ancrage_TEDI_esclave_rec[i] == ERROR)
		{
			retour = ERROR;
		}
		ancrage_pcp_lecmess[i] = SysMQCreer();
		if (ancrage_pcp_lecmess[i] == ERROR)
		{
			retour = ERROR;
		}
		sprintf(NomPipe, "DialLCR%d", i);
		MqDialLas[i] = SysPipeCreer(NomPipe, 15, MAX_MESSAGE_LCR + 10);
		if (MqDialLas[i] == ERROR)
		{
			retour = ERROR;
		}
		sprintf(NomPipe, "LasDialLCR%d", i);
		MqLasDial[i] = SysPipeCreer(NomPipe, PIPE_NB_MESS, MAX_MESSAGE_LCR + 10);
		if (MqLasDial[i] == ERROR)
		{
			retour = ERROR;
		}
	}
	/* --------------------------------
	 * FIN DE supInitAncrage
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * supTermAncrage
 * ==============
 * Terminaison des ancrages..
 * --------------------------------	*/

int supTermAncrage(void)
{
	int retour = OK;
	int i;
	char Nom[200];
	/* --------------------------------
	 * INIT DES ANCRAGES DIVERS
	 * --------------------------------     */
	SysMQDetruire(ancrageTedi_ctrl);
	SysMQDetruire(x01_ancrage_interne);
	ancrageTedi_ctrl = ERROR;
	x01_ancrage_interne = ERROR;

	/* --------------------------------
	 * ANCRAGES POUR LES PORTS
	 * --------------------------------     */
	for (i = 0; i < NBPORT; i++)
	{
		sprintf(Nom, "x01_ancrage_TEDI_esclave_em[%d]", i);
		SysMQDetruire(x01_ancrage_TEDI_esclave_em[i]);
		x01_ancrage_TEDI_esclave_em[i] = ERROR;
		sprintf(Nom, "x01_ancrage_TEDI_esclave_rec[%d]", i);
		SysMQDetruire(x01_ancrage_TEDI_esclave_rec[i]);
		x01_ancrage_TEDI_esclave_rec[i] = ERROR;
		sprintf(Nom, "ancrage_pcp_lecmess[%d]", i);
		SysMQDetruire(ancrage_pcp_lecmess[i]);
		ancrage_pcp_lecmess[i] = ERROR;
		SysPipeDetruire(MqDialLas[i]);
		MqDialLas[i] = -1;
		SysPipeDetruire(MqLasDial[i]);
		MqLasDial[i] = -1;
	}
	/* --------------------------------
	 * FIN DE supTermAncrage
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * supInitTache
 * ============
 * Creation d'une nouvelle tache
 * --------------------------------	*/

static int _supInitTache(T_supGestion * pGes, char *nom, int sequence, short priorite, FUNCPTR fonction, int arg1)
{
	int retour = OK;
	int numTache = pGes->nbTache;
	T_supDescTache *tache = &pGes->Tache[numTache];
	/* ------------------------------------
	 * INIT DE LA STRUCTURE
	 * ------------------------------------ */
	tache->ptEntree = fonction;
	tache->sequence = sequence;
	strcpy(tache->nomTache, nom);
	/* Pour les taches sequencees par le
	 * moniteur, il est necessaire de creer
	 * un semaphore. */
	if (sequence)
	{
		tache->tacVersMon = semEventCreate();
		if (tache->tacVersMon == NULL)
		{
			retour = ERROR;
		} else
		{
			tache->monVersTac = semEventCreate();
			if (tache->monVersTac == NULL)
			{
				retour = ERROR;
			} else
			{

				pGes->nbTache++;
			}
		}
	} else
	{
		/* On incremente automatiquement le nombre de tache... */
		pGes->nbTache++;
	}
	if (retour != ERROR)
	{
		/* Lancement de la tache                                */
		/* Attention, le numero de la tache a ete
		 * ajoute comme premier parametre de
		 * lancement.   */
		tacheCreerBis(tache, fonction, nom, arg1);
		ksleep(500);
		tache->active = 1;
		/* ------------------------------------
		 * On laisse la tache s'initialiser
		 * ------------------------------------ */
		printDebug("Tache %s num %d initialisee sem %#x", tache->nomTache, numTache, (unsigned int) tache->tacVersMon);

		if (sequence)
		{
			printDebug(" donne ");
			retour = semSignal(tache->monVersTac);
			if (retour != ERROR)
			{
				//              retour = semWait (tache->tacVersMon);
				//              printDebug (" pris ");
			}
		} /* endif(sequence              */
		printDebug("\n");
	} /* endif(retour==EROR          */
	/* ------------------------------------
	 * FIN DE supInitTache
	 * ------------------------------------ */
	return retour;
}

/* --------------------------------
 * supTermTache
 * ============
 * Destruction de la derniere tache
 * --------------------------------	*/

int supTermTaches(T_supGestion * pGes)
{
	int retour = OK;
	while ((pGes->nbTache > 0) && (pGes->Tache[pGes->nbTache - 1].sequence))
	{
		int numTache = --pGes->nbTache;
		T_supDescTache *tache = &pGes->Tache[numTache];
		EVENT_ID tacVersMon = tache->tacVersMon;
		EVENT_ID monVersTac = tache->monVersTac;
		tache->tacVersMon = NULL;
		tache->monVersTac = NULL;
		tache->active = 0;
		tache->sequence = 0;
		tacheDelete(tache->tid);
		semEventDelete(tacVersMon);
		semEventDelete(monVersTac);
	}
	return retour;
}

/* --------------------------------
 * supInitAppli
 * ============
 * Lancement des taches de l'appli-
 * cation.
 * --------------------------------	*/

int supInitAppli(T_supGestion * pGes, int typeBoot)
{
	int retour = OK;
	/*   if (typeBoot == 1) */
	{
		int indice = 0;

		/* --------------------------------     */
		/* CREATION DES TACHES                          */
		/* --------------------------------     */
		printDebug("		Creation des taches \n");
		printDebug("		------------------- \n");
		while ((NULL != listeTaches[indice].nom) && (OK == retour))
		{
			T_supTache *tache = &listeTaches[indice];
			//			if ((1 == tache->syst) || (1 == typeBoot))
			{
				if (NULL != tache->init)
				{
					((void(*)()) tache->init)();
				}
			}
			indice++;
		}
	}
	{
		int indice = 0;

		/* --------------------------------     */
		/* CREATION DES TACHES                          */
		/* --------------------------------     */
		printDebug("		Creation des taches \n");
		printDebug("		------------------- \n");
		while ((NULL != listeTaches[indice].nom) && (OK == retour))
		{
			T_supTache *tache = &listeTaches[indice];
			//			if ((1 == tache->syst) || (1 == typeBoot))
			{
				retour = _supInitTache(pGes, tache->nom, tache->seq, tache->prio, tache->fonction, tache->arg);
			}
			indice++;
		}
		/* --------------------------------     */
		/* DIALOGUES LCR                                        */
		/* --------------------------------     */
		_supInitPrintBanner("			Fin  de phase d'init : %s				 \n", ((ERROR == retour) ? "ECHOUEE" : "REUSSIE"));
		/* --------------------------------     */
		/* APPLI DEMARREE                                       */
		/* --------------------------------     */
		//		supRunDemarrer();
	}
	return retour;
}

/* --------------------------------
 * supTermAppli
 * ============
 * Suppression des taches applica-
 * tives et fermeture des pipes et
 * messages queues.
 * --------------------------------	*/

int supTermAppli(T_supGestion * pGes)
{
	int retour = OK;
	pGes->appli = 0;
	supTermTaches(pGes);
	supTermAncrage();
	return retour;
}
