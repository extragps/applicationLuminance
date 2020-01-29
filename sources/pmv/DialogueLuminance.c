/*
 * DialogueLuminance.c
 * ===================
 * Objet : Communication avec les cartes luminance.
 *
 * --------------------------------------------------------
 *  Created on:  27 Aout 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: DialogueLuminance.c,v $
 * Revision 1.2  2018/06/04 08:38:43  xg
 * Passage en version V122
 *
 * Revision 1.7  2009/03/12 09:15:25  xag
 * Ajout du mecanisme de reprise.
 *
 * Revision 1.6  2009/01/19 16:39:26  xag
 * Suppression du test bidon sur les entrees.
 *
 * Revision 1.5  2009/01/09 16:24:19  xag
 * *** empty log message ***
 *
 * Revision 1.4  2008/10/24 15:56:20  xag
 * Ajout du test de l'ouverture des sorties de la liaison.
 *
 * Revision 1.3  2008/10/16 07:53:05  xag
 * Ajout du probleme potentiel de pilotage des entrees.
 *
 * Revision 1.2  2008/10/13 08:18:17  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.1  2008/09/12 14:59:05  xag
 * Archivage de printemps
 *
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "kcommon.h"
#include "klog.h"
#include "standard.h"
#include "define.h"
#include "mon_inc.h"
#include "x01_str.h"
#include "xdg_var.h"
#include "mon_debug.h"
#include "vct_str.h"
#include "lcr_util.h"
#include "supRun.h"
#include "Superviseur.h"
#include "extLib.h"
#include "MQ.h"
#include "Tache.h"
#include "x01_var.h"
#include "spcLib.h"
#include "perfLib.h"
#include "ficTrace.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "lcr_p.h"
#include "lcr_trc.h"
#include "ioPortLib.h"
#include "eriLib.h"
#include "etaSyst.h"

/* Fichiers d'include specifiques luminance */
#include "ecouteur.h"
#include "lumAdresse.h"
#include "lumMsg.h"
#include "luminance/lumMsgCreer.h"

/* ********************************	*/
/* DEFINITION DES CONSTANTES		*/
/* ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

static LumMsgCreateur *_dialogueLum_pt = NULL;
static int32 _lumTraiteurNbDef=0;

/*
 * DialogueLuminanceAjouterEcouteur
 * ================================
 *
 * */

void DialogueLuminanceAjouterEcouteur(Ecouteur *ecouteur_pt)
{
	lumMsgCreateurAjouterEcouteur(_dialogueLum_pt, ecouteur_pt);
}

/*
 * DialogueLuminanceSupprimerEcouteur
 * ================================
 *
 * */

void DialogueLuminanceSupprimerEcouteur(Ecouteur *ecouteur_pt)
{
	lumMsgCreateurSupprimerEcouteur(_dialogueLum_pt, ecouteur_pt);
}

/*
 * DialogueLuminanceInit
 * =====================
 * Initialisation des donnees utiles
 * pour l'initialisation du dialogue
 * avec les cartes luminances.
 * */

void DialogueLuminanceInit(void)
{
	/* Initialisation de la structure permettant la creation des messages */
	_dialogueLum_pt = lumMsgCreateurNew();
}

void DialogueLuminanceTerm(void)
{
	lumMsgCreateurDelete(_dialogueLum_pt);
}

LumMsgCreateur *DialogueLuminanceGet()
{
	if (NULL == _dialogueLum_pt)
	{
		DialogueLuminanceInit();
	}
	return _dialogueLum_pt;
}
/* --------------------------------	*/
/* DialogueLuminance				*/
/* =================				*/
/* --------------------------------	*/

FUNCPTR DialogueLuminance(int numero, int numPort)
{
	INT i;
	int nbCar;
	int notFin = 1;
	/* --------------------------------
	 * INITIALISATION
	 * --------------------------------     */
	printDebug("DialogueLuminance : Init OK port %d\n\n", numPort);

	/* --------------------------------
	 * BOUCLE DE TRAITEMENT
	 * --------------------------------     */
	while (notFin)
	{
		int width;
		fd_set readFds;
		/* 10 secondes de timeout sur le select. */
		struct timeval attente =
		{ 1, 0 };
		int fd = -1;
		/* -----------------------------------
		 * Initialisation de la structure du
		 * Select.
		 * --------------------------------- */
		FD_ZERO(&readFds);
		width = 0;
		if (NULL != cnxLiaison[numPort])
		{
			fd = commCnxGetFd(cnxLiaison[numPort]);
			if (-1 != fd)
			{
				FD_SET(fd, &readFds);
				width = MAX(fd,width);
			}
		}
		width++;
		/* ----------------------------------
		 * Attente bloquante en lecture
		 * sur l'un des deux ports (las et mq)
		 * ----------------------------------*/
		switch (select(width, &readFds, NULL, NULL, &attente))
		{
		case ERROR:
			if (EINTR != errno)
			{
				printDebug(
						"DialogueLuminance Erreur de select : %#x liaison %d\n",
						errno, i);
				notFin = 0;
			}
			break;
		case 0:
			/* Si il n'oy a plus d'activité, c'est louche, on controle la carte */
#ifdef CPU432
			if (-1 != fd)
			{
				char infoLiaisons = 0;
				if (-1 != ioctl(fd, 0x400 + 70, &infoLiaisons))
				{
					if (0 != (infoLiaisons & 0xF0))
					{
						ksleep(100);
						_lumTraiteurNbDef++;
						if(_lumTraiteurNbDef<10)
						{
							infoLiaisons=0;
							if(-1!=ioctl(fd,0x400+69,&infoLiaisons))
							{
							klogPut(NULL,(9==_lumTraiteurNbDef?LOG_ERROR:LOG_INFO),
							"Reset des sorties liaisons %#0x (nbDef=%d)",
								infoLiaisons,_lumTraiteurNbDef);
							if(9==_lumTraiteurNbDef)
							{

								cmd_trc_tr("Reset des sorties liaisons %#0x (nbDef=%d)",
									infoLiaisons,_lumTraiteurNbDef);

							}
							}
							else
							{
							klogPut(NULL,LOG_ERROR,"Erreur reset des sorties liaisons %#0x",errno);
							cmd_trc_tr("Erreur reset des sorties liaisons %#0x",errno);
							}
						}
						else
						{
							/* Cela fait dix fois que l'on essaie, on ne persiste pas */
							klogPut(NULL,LOG_ERROR,"Fin de tentative de reset des liaisons");
						}
					}
					else
					{
						_lumTraiteurNbDef=0;
					}
				}
			}
#endif
			//        			printDebug ("Luminance, timeout sur reception. liaison %d\n", i);
			break;
		default:

			/* ----------------------------------
			 * Si des donnes sont arrivees sur
			 * la liaison LAS
			 * ----------------------------------*/
			if (FD_ISSET(fd, &readFds))
			{
				uint8 data[MAX_MESSAGE_LCR];
				printDebug("Arrivee d'un message sur la liaison %d\n", i);
				/* ---------------------------------
				 * Lecture des donnees en entree de
				 * la liaison serie.
				 * ---------------------------------*/
				nbCar = read(fd, data, MAX_MESSAGE_LCR);
				if (nbCar == ERROR)
				{
					return NULL;
				}
				else
				{
					/* TODO: effectuer les traitements lies au protocole luminance. */
					lumMsgCreateurCaracteresChanged(_dialogueLum_pt, data,
							nbCar);
				} /* endif (nbCar == ERROR) */
			} /* endif (FD_ISSET(fdLiaisonLas[i], */
		} /* switch(select(width, &readFds,   */
		/* Controle du positionnement des sorties de la carte.... */

		etaSystIncrCompteur(numero);
	} /* Fin FOREVER */
	/* --------------------------------
	 * FIN DE dialogueLCR
	 * --------------------------------     */
	return OK;
}
