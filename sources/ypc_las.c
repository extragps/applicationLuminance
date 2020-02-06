/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 01/12/00
 * Fichier	: ypc_las.c
 * Objet	: Gestion des liaisons qui autrefois etaient uniquement des
 * 			liaisons series.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,01Dec00,xag  creation.
 * ************************************************************************	*/
/*
 * ypc_las.c
 * ===================
 *
 * Objet	: Gestion des liaisons qui autrefois etaient uniquement des
 * 			liaisons series.
 *
 * --------------------------------------------------------
 *  Created on: 1 decembre 2000
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: YPC_LAS.c,v $
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.8  2009/01/23 09:39:56  xag
 * Retour en arriere pour le passage en mode 485
 *
 * Revision 1.7  2009/01/19 16:26:48  xag
 * Suppression du controle de flux bidon.
 *
 * Revision 1.6  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.5  2008/10/03 16:21:45  xag
 * *** empty log message ***
 *
 * Revision 1.4  2008/09/22 07:53:42  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/09/12 14:57:09  xag
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

#include <mq.h>
#include <pipe.h>
#include <string.h>
#include <superviseur.h>
#include "standard.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "mon_inc.h"
#include "xdg_var.h"
#include "x01_str.h"
#include "x01_var.h"
#include "ted_prot.h"
#include "det_prot.h"
#include "tac_conf.h"
#include "mon_debug.h"
#include "ypc_las.h"
#include "etaSyst.h"
#include "rdtComm.h"

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * PROTOTYPE DES FONCTIONS LOCALES
 * ********************************	*/

/**
 * ypc_las_get_param
 * =================
 * Creation du parametre de connexion en fonction des informations de
 * configuration du port...
 */


static CommCnxParam *_tac_las_get_param(int numPort)
{
	T_xdg_las *las_pt = &xdg_las[numPort];
	T_xdg_cf_las *cfg_pt = &xdg_cf_las[numPort];
	CommCnxParam *param_pt = NULL;
	int parite = TTY_NOPAR;
	switch (cfg_pt->parite)
	{
	case PAIRE:
		parite = TTY_PAREVEN;
		break;
	case IMPAIRE:
		parite = TTY_PARODD;
		break;
	case SANSPARITE:
		parite = TTY_NOPAR;
		break;
	}
	param_pt = commCnxSerialParamNew(numPort, las_pt->nomDev, cfg_pt->vitesse, cfg_pt->data, parite, cfg_pt->stop,
		(numPort==PORT_LUMINANCE?TTY_RTSCTSMOD223 : 0));
	return param_pt;
}

void tac_las_reconf(int numPort)
{
CommCnxParam *param_pt=_tac_las_get_param(numPort);
	if(NULL!=param_pt)
	{
		if(NULL!=xdg_las[numPort].conn_pt)
		{
			commCnxReconf(xdg_las[numPort].conn_pt,param_pt);
		}
		commCnxParamDelete(param_pt);
	}
}

/**
 * tac_las_demarre
 * ===============
 * Demarrage d'une liaison.
 * @param : le numero de port a demarrer
 */
void tac_las_demarre(int port)
{
	T_xdg_cf_las *cfgLas_pt=&xdg_cf_las[port];
	T_xdg_las *las_pt = &xdg_las[port];
	CommCnxParam *param_pt = _tac_las_get_param(port);
	if (NULL != param_pt)
	{
		las_pt->conn_pt = commCnxNewSimple(param_pt);
		commCnxParamDelete(param_pt);
		if(NULL!=las_pt->conn_pt) {
			printDebug("Tache Serie demarree port %d\n",cfgLas_pt->numPortIp);
		}
	}
	cfgLas_pt->type_e=LIAISON_SERIE;
	cnxLiaison[port] = las_pt->conn_pt;
}

/**
 * ypc_ctrl_las
 * ============
 * Decrementation des temporisations sur les laisons.
 */
void tac_las_ctrl()
{
	INT i;
	x01_ctrl_timer++;
	for (i = 0; i < NBPORT; i++)
	{
		T_xdg_las *xdg = &xdg_las[i];
		if (xdg->time_out)
		{
			if (--xdg->time_out == 0)
			{
				xdg->flag_time_out = TRUE;
				/* on libere la liaison en reception */
				xdg->dispo_rec = TRUE;
			}
		}
		if (xdg->attente_em > 0)
		{
			if (--xdg->attente_em == 0)
			{
				xdg->flag_em = TRUE;
			}
		}
	}
	/* -------------------------------
	 * FIN DE ypc_ctrl_las
	 * --------------------------------     */
}

/**
 * tac_las_init
 * ============
 * Initialisation des structure contenant les informations echangees
 * sur les liaisons.
 */

void tac_las_init(void)
{
	INT i;

	for (i = 0; i < NBPORT; i++)
	{
		T_xdg_las *xdg = &xdg_las[i];

		xdg->phase = 0;
		xdg->attente_em = 0;
		xdg->flag_em = 0;
		xdg->time_out = 0;
		xdg->flag_time_out = FALSE;
		xdg->flag_fin_em = 1;
		xdg->pt_in_rec = 0;
		xdg->pt_out_rec = 0;
		xdg->nb_car_em = 0;
		xdg->pt_mes = NULL;

		switch (i)
		{
		case PORT_SERIE:
			xdg_cf_las[i].media = 'P';
			xdg_cf_las[i].cfg = TRUE;
			xdg_cf_las[i].protocole = TEDI_PROT;
			break;
		case PORT_PASS:
			xdg_cf_las[i].type_e = LIAISON_PASS;
			xdg_cf_las[i].protocole = TEDI_PROT;
			break;
		case PORT_FIP:
			xdg_cf_las[i].type_e = LIAISON_WFIP;
			xdg_cf_las[i].cfg = TRUE;
			xdg_cf_las[i].protocole = TEDI_PROT;
			break;
		case PORT_LUMINANCE:
			xdg_cf_las[i].protocole = LUM_PROT;
			xdg_cf_las[i].cfg = TRUE;
			break;
			break;
//		case PORT_SERIE_2:
//			xdg_cf_las[i].media = 'P';
//			xdg_cf_las[i].cfg = TRUE;
//			xdg_cf_las[i].protocole = TEDI_PROT;
			break;
		case PORT_IP_LCR_2:
			xdg_cf_las[i].media = 'P';
			xdg_cf_las[i].protocole = TEDI_PROT;
			break;
		case PORT_IP_LCR:
			xdg_cf_las[i].media = 'E';
			xdg_cf_las[i].protocole = TEDI_PROT;
			break;
		default:
			xdg_cf_las[i].protocole = AUCUN_PROTOCOLE;
			xdg_cf_las[i].cfg = FALSE;
			break;
		}

		/* on configure la liaison  */
		if(i!=PORT_IP_LCR)
		{
		if (xdg_cf_las[i].protocole != AUCUN_PROTOCOLE)
		{
			xdg_cf_las[i].cfg = TRUE;
			mncfuart(i, xdg_cf_las[i].vitesse, xdg_cf_las[i].data, xdg_cf_las[i].parite, xdg_cf_las[i].stop,
					xdg_cf_las[i].media);
		}
		}
	}
}

/* FICHIER: %M%      REL: %I% DATE: %G% */
/*DOC***************************************************************************
 *  SIAT.  *   FICHE DE  FONCTION:  tac_las_init_cfg
 ********************************************************************************
 * OBJET DE LA FONCTION : module d'init de de la configuration des las
 * PARAMETRES           :
 * VALEURS RENDUES      :
 * FONCTIONS APPELEES   :
 * FONCTIONS APPELANTES :
 ***************************************************************************DOC*/
/**
 * tac_las_init_cfg_las
 * ====================
 * Initialisation en fonction des types des liaisons.
 * @param las le numero de la liaison a traiter.
 */
void tac_las_init_cfg_las(INT las)
{
	T_xdg_cf_las *xdg_cf = &xdg_cf_las[las];

	xdg_cf->cfg = FALSE;
	xdg_cf->media = 'P';
	xdg_cf->vitesse = 9600;
	xdg_cf->data = 8;
	xdg_cf->parite = PAIRE;
	xdg_cf->stop = 1;
	xdg_cf->attente_em = 0;
	xdg_cf->suffixe = 0;
	xdg_cf->prefixe = 0;
	xdg_cf->protect = FALSE;

	/* Configuration speciale pour les ports lcr sur liaison serie */
	if (las < NB_PORT_TEDI)
	{
		xdg_cf->data = 7;
	}

	switch (las)
	{
//	case PORT_TPE:
//		xdg_cf->vitesse = 19200;
//		xdg_cf->parite = SANSPARITE;
//		break;
	case PORT_LUMINANCE:
		xdg_cf->vitesse = 115200;
		xdg_cf->parite = SANSPARITE;
		break;
	case PORT_PASS:
		xdg_cf->numPortIp = CPU432_PORT_PASS;
		break;
	case PORT_IP_LCR:
		xdg_cf->numPortIp = CPU432_PORT;
		break;
	default:
		xdg_cf->numPortIp = CPU432_PORT + las - PORT_IP_LCR;
		break;

//	case PORT_DET:
//		xdg_cf->media = 'l';
//		break;
	}
	/* ***************************
	 * fin de tac_las_init_cfg_las
	 * *************************** */
}

/**
 * tac_las_init_cfg
 * ================
 * Initialisation de la configuration de toutes les liaisons.
 */
void tac_las_init_cfg(void)
{
	INT i;
	for (i = 0; i < NBPORT; i++)
	{
		tac_las_init_cfg_las(i);
	}
}

/**
 * tac_las
 * =======
 * Tache de gestion du dialogue LCR.
 */

void tac_las(int numero)
{ /* --------------------------------     */
	/* DECLARATION DES VARIABLES            */
	/* -------------------------------- */
	/* Des variables statiques, je n'ai- */
	/* me pas du tout cela.                         */
	static char buff_rec[NBPORT][MAX_MESSAGE_LCR];
	INT nbMes;
	INT i, j;
	unsigned short nbCar;
	int notFin = 1;
	/*---------------------------------	*/
	/* DEBUT DU CODE                                        */
	/*---------------------------------	*/
	printDebug("tac_las : debut\n");
	/*---------------------------------	*/
	/* SYNCHRO AVEC LE SUPERVISEUR          */
	/*---------------------------------	*/
	mnsuspIniPrendre();

	x01_ctrl_timer = 0;
	printDebug("tac_las : Init OK\n\n");
	mnsusp(0);
	/*---------------------------------	*/
	/* BOUCLE DE TRAITEMENT                         */
	/*---------------------------------	*/
	while (notFin)
	{
		struct timeval timeout =
		{ 0, 0 };
		fd_set masque;
		int width = 0;
		int retour;
		/* -------------------------------- */
		/* CREATION DU MASQUE                           */
		/* -------------------------------- */
		FD_ZERO (&masque);
		for (i = 0; i < NBPORT; i++)
		{

			FD_SET (MqDialLas[i], &masque);
			width = ((width < (MqDialLas[i] + 1)) ? MqDialLas[i] + 1 : width);
		}
		/* -------------------------------- */
		/* TEST DES LIAISONS                            */
		/* -------------------------------- */
		retour = select(width, &masque, NULL, NULL, &timeout);
		switch (retour)
		{
		case ERROR:
			if (EINTR != errno)
			{
				printDebug("tac_las: Il y a une erreur dans le select %d\n", errno);
				notFin = 0;
			}
			break;
		case 0:
			/* Rien a faire, sortie sur timeout     */
			/* break; */
		default:

			/*---------------------------------	*/
			/* Pour toutes les liaisons configu- */
			/* rees on traite la reception.         */
			if (0 != retour)
			{
				printDebug("tac_las : attention retour %d\n", retour);
			}
			for (i = 0; (i < NBPORT) && (0 != notFin); i++)
			{
				/* -------------------------------- */
				/* Lecture de la MQ de dialogue.        */
				/* --------------------------------     */
				nbMes = 0; /* On consulte le nombre de messages */
				/* dans la fifo.                                        */
				/* LECTURE DES MESSAGES.                        */
				if (FD_ISSET (MqDialLas[i], &masque))
				{
					printDebug("Avant SysPipe i %d\n", i);
					nbMes = SysPipeLire(MqDialLas[i], buff_rec[i], sizeof(buff_rec[i]));
					printDebug("Apres SysPipe nbMes %d\n", nbMes);
					if (nbMes == ERROR)
					{
						printDebug("tac_las: Il y a une erreur dans l'emission su le pipe %d\n", errno);
						notFin = 0;
					} /* endif(nbMes==ERROR                           */
				} /* endfi(nbMes>0                                        */
				/* --------------------------------     */
				/* Traitement des messages.                     */
				/* --------------------------------     */
				if ((0 != notFin) && (nbMes > 0))
				{
					memcpy(&nbCar, buff_rec[i], 2); /* Recup de la longueur du      */
					/* message.                                                     */
					printDebug("\ntac_las %d : reception de dial du message %s\n "
						"de longueur %d\n", i, &buff_rec[i][2], nbCar);
					/*---------------------------------	*/
					/* Pour tout les caracteres.            */
					/* --------------------------------     */
					for (j = 2; j < nbCar + 2; j++)
					{
						printDebug("tac_las %d :reception de dial car %c\n", i, buff_rec[i][j]);
						/* selon le protocole                           */
						switch (xdg_cf_las[i].protocole)
						{
						case TEDI_PROT: /* Reception sur une liaison de         */
							/* commande.                                            */
							printDebug("tac_las %d : TEDI_PROT\n", i);
							tedi_prot_reception(i, buff_rec[i][j]);
							break;
							/* Pas de prototocole détecteur. */
//						case DET_PROT: /*      Reception d'un detecteur.               */
//							printDebug("tac_las %d : DET_PROT \n", i);
//							det_prot_reception(i, buff_rec[i][j]);
//							break;
						} /* switch (xdg_cf_las[i].protocole) */
						if (++xdg_las[i].pt_out_rec >= MAX_BUF_EM)
						{
							xdg_las[i].pt_out_rec = 0;
						} /* endif(xdg_las                                        */
					} /* fin for (j=0; j < nbCar; j++)        */
				} /* fin if (nbCar > 0)                           */
			} /* fin for (i=0;i < NBPORT;i++)         */
			/* --------------------------------
			 * EMISSIONS
			 * --------------------------------     */
			for (i = 0; (0 != notFin) && (i < NBPORT); i++)
			{
				/* si il y a un retard et que l'on      */
				/* doit emetre                                          */
				if (xdg_las[i].flag_em)
				{
					xdg_las[i].flag_em = FALSE;
					switch (xdg_cf_las[i].protocole)
					{
					case TEDI_PROT:
						switch (mnemuart(i))
						{
						case 1: /* Fin d'emission OK                            */
							tedi_fin_emission(i, CR_OK);
							break;
						case -1: /* Fin d'emission NOK                           */
							tedi_fin_emission(i, CR_NOK);
							break;
						case 0: /* Il reste des caracteres a emettre */
							break;
						} /* endswith(mnemuart                            */
						break;
					default:
						mnemuart(i);
						break;
					} /* endswitch(xdg_cf_las                         */
				} /* endif(xdg_las[i                                      */

				if (xdg_las[i].flag_time_out)
				{
					xdg_las[i].flag_time_out = FALSE;
					switch (xdg_cf_las[i].protocole)
					{
					case TEDI_PROT:
						tedi_prot_time(i);
						break;

//					case DET_PROT:
//						det_prot_time(i);
//						break;
					}
				}
				if (xdg_las[i].flag_fin_em == 1)
				{
					xdg_las[i].flag_fin_em = 2;
					switch (xdg_cf_las[i].protocole)
					{
					case TEDI_PROT:
						tedi_prot_emission(i);
						break;
//					case DET_PROT:
//						det_prot_emission(i);
//						break;
					} /* endswitch(xdg_cf_las                         */
				} /* endif(xdg_las[i].flag                        */
				if ((xdg_las[i].pt_mes == NULL) && (xdg_las[i].flag_fin_em))
				{
					char Buffer[MSG_SIZE];
					T_usr_ztf *pt_mes = NULL;
					switch (xdg_cf_las[i].protocole)
					{
					case TEDI_PROT:
						pt_mes = NULL;
						if (monMessRec(x01_ancrage_TEDI_esclave_em[i], FALSE, (char *) Buffer) == OK)
						{
							memcpy(&pt_mes, Buffer, sizeof(struct usr_ztf *));
							if (pt_mes != NULL)
							{
								/* on transmet le selecting */
								tedi_em(i, pt_mes, 0);
								/* on occupe la liaison en reception */
								xdg_las[i].dispo_rec = FALSE;
							}
						}

						break;
					} /* endswitch                                            */
				} /* endif ((xdg_las[i].pt_mes ==         */
				/* on met a jour le mode de fonc-       */
				/* tionnement du port                           */
				if (rqtemp[TR_0 + i] == TP_FINI)
				{
					rqtemp[TR_0 + i] = TP_HS;
					x01_set[i].echo = FALSE;
					x01_set[i].cr = FALSE;
					x01_set[i].lpl = 0;
					x01_set[i].mtel = FALSE;
					mnecho((UINT8) i, FALSE);
				} /* fin de if(rqtemp[TR_0+i                      */
			} /* fin du for (i = 0; i < NBPORT;       */
			break;
		} /* endswitch(retour                                     */
		etaSystIncrCompteur(numero);
		/* --------------------------------     */
		/* RETOUR AU MONITEUR                           */
		/* --------------------------------     */
		/* On attend avant de reprendre d'      */
		/* d'obtenir la main du superviseur. */
		mnsusp(0);
		/* taskDelay(10); */
	} /* fin du while(TRUE)                           */
	/* --------------------------------     */
	/* FIN DE tac_las                                       */
	/* --------------------------------     */
	printDebug("tac_las: Fin de la procedure\n");
	return;
}

//int tac_las_send_direct(int i, char *buffer, int nbCar)
//{
//	return SysPipeEmettre(MqDialLas[i], buffer, nbCar);
//}

