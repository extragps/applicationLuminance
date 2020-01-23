/*
 * Mon_s32.c
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: Mon_s32.c,v $
 * Revision 1.2  2018/06/04 08:38:36  xg
 * Passage en version V122
 *
 * Revision 1.7  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.6  2008/10/03 16:21:46  xag
 * *** empty log message ***
 *
 * Revision 1.5  2008/09/29 07:58:51  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.4  2008/09/22 07:53:42  xag
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

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "standard.h"                  /* redefinition des types                         */
#include "Semaphore.h"
#include "Tache.h"
#include "etaSyst.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
#include "x01_vcth.h"
#include "mon_str.h"
#include "standard.h"                  /* redefinition des types */
#include "define.h"
#include "mon_def.h"
#include "Superviseur.h"
#include "mon_ext.h"
#include "mon_pro.h"
#include "lcr_idfh.h"
#include "Pipe.h"
#include "mon_debug.h"
#include "mon_def.h"
#include "tac_ctrl.h"
#include "semLib.h"

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif


/************************************************************************** */
/*                                                                          */
/* MNRELANCE  : relance de l'application									*/
/*                     (acces utilisateur)									*/
/*																			*/
/* appel  : data = mnrestart ()												*/
/*																			*/
/*																			*/
/****************************************************************************/
void mnrestart()
{
	return;
}

/****************************************************************************
 * MNRAZFLL : ecriture en RAM FLASH
 *                     (acces utilisateur)
 *
 * appel  :  mnrwrtf (port,val, add_code)
 *           D1 = port
 *           D2 = adreesse du code en RAM
 *****************************************************************************/

void mnrazfl(void *add, UINT8 * mem_code)
{
	return;
}

/******************************************************************************
 *
 * MNECHO  : gestion de l'echo
 *                     (acces utilisateur)
 *
 * appel  :  mnecho (flag)
 *
 *******************************************************************************/
void mnecho(UINT8 las, UINT8 Flag)
{
	if (PORT_FIP != las)
	{
		/* TODO : Portage LINUX */
#ifdef VXWORKS
		int options = ioctl (xdg_las[las].fdLiaisonLas, FIOGETOPTIONS, 0);
		if (Flag)
		{
			options |= OPT_ECHO;
		}
		else
		{
			options &= ~(int) OPT_ECHO;
		}
		ioctl (xdg_las[las].fdLiaisonLas, FIOSETOPTIONS, options);
#endif
	}
	return;
}

/******************************************************************************
 *
 * MNRWRT_FL : ecriture en RAM FLASH
 *                     (acces utilisateur)
 *
 * appel  :  mnrwrtf (port,val, add_code)
 *           D1 = port
 *           D2 = val
 *           D3 = adreesse du code en RAM
 *******************************************************************************/
void mnwrtfl(UINT * ptr_d, UINT ptr_s, UINT8 * mem_code)
{
	return;
}


/******************************************************************************
 *
 * MNEMUART  : emission vers un uart
 *                     (acces utilisateur)
 *
 * appel  :  mnemuart (port)
 *******************************************************************************/

int mnemuart(int las)
{
	struct xdg_las *las_pt = &xdg_las[las];
	int nbCarEmis;
	int retour;
	/* --------------------------------     */
	/* TRANSMISSION VERS LA TACHE           */
	/* --------------------------------     */
	/* La transmission se fait au travers
	 * d'un pipe a destination d'une
	 * tache de traitement qui peut ren-
	 * voyer sur une liaision serie ou
	 * vers WorldFip.
	 */
	if(PORT_PASS==las) {
		nbCarEmis = dialoguePassRetour(las, las_pt->buff_em, las_pt->nb_car_em);

	} else {
		nbCarEmis = SysPipeEmettreSansSusp(las_pt->liaison, PIPE_NB_MESS, las_pt->buff_em, las_pt->nb_car_em);
	}
	if (nbCarEmis == ERROR) /* Etonnant que l'on ne fasse rien      */
	{ /* dans ce cas d'erreur.                        */
		printDebug("Erreur %d\n", nbCarEmis);
		retour = -1;
	} else if (nbCarEmis != las_pt->nb_car_em)
	{
		/* Tous les caracteres n'ont pas ete
		 * emis, il faut recopier les caracteres
		 * restant en debut de buffer. */
		printDebug("Emission du message %s tronque %d/%d\n", las_pt->buff_em, nbCarEmis, las_pt->nb_car_em);
		las_pt->nb_car_em -= nbCarEmis;
		memcpy(las_pt->buff_em, &las_pt->buff_em[nbCarEmis], las_pt->nb_car_em);
		las_pt->flag_em = 1;
		retour = 0;
	} else
	{
		las_pt->buff_em[las_pt->nb_car_em] = 0;
		printDebug("Emission du message %s correct\n", las_pt->buff_em);
		las_pt->flag_fin_em = 1;
		retour = 1;
	}
	/* --------------------------------
	 * FIN DE mnemuart
	 * --------------------------------     */
	return (retour);
}

/******************************************************************************
 *
 * MNINPUT 16: lecture d'un port  16 bits
 *                     (acces utilisateur)
 *
 * appel  : data = mninput (port)
 *
 * avec   : port = adresse du port
 *          data = valeur lue
 *
 *******************************************************************************/
UINT16 mninput16(UINT16 *adresse)
{
	UINT16 val;
	/* TODO : Portage LINUX */
#ifdef VXWORKS
	if(vxMemProbe((char *)adresse,VX_READ,2,&val)==ERROR)
	{
		printDebug("Probleme de lecture e l'adresse %#0x\n",adresse);
		val=0;
	}
#endif
	return val;
}

/******************************************************************************
 *
 * MNOUTPUT 16 : ecriture d'un port 16 BITS
 *                     (acces utilisateur)
 *
 * appel  :  mnoutput (port,data)
 *
 * avec   : port = adresse du port
 *          data = valeur a ecrire
 *
 *******************************************************************************/
VOID mnoutput16(UINT16* adresse, UINT16 val)
{
	/* TODO : Portage LINUX */
#ifdef VXWORKS
	if(vxMemProbe((char *)adresse,VX_WRITE,2,&val)==ERROR)
	{
		printDebug("Probleme d'ecriture e l'adresse %#0x\n",adresse);
	}
#endif
	return;
}

/******************************************************************************
 *
 * MNCFUART  : configuration d'un uart
 *                     (acces utilisateur)
 *
 * appel  :  mncfuart (port,vitesse, format, parite, stop,protocole,type)
 *
 *******************************************************************************/
VOID mncfuart(LONG las, LONG vitesse, LONG format, LONG parite, LONG stop, LONG media)
{

	printDebug("mncfuart Modification de la config "
		"de l'UART numero %d \n", las);
	printDebug("mncfuart UART numero %d vitesse %d "
		"format %d parite %c stop %d media %c\n", las, vitesse, format, parite, stop, media);
	tac_las_reconf(las);

	/*---------------------------
	 La modif est faite que si
	 valeur est coherente.
	 ---------------------------*/
	/* TODO : Fonction a modifier pour linux..... */
}

/******************************************************************************
 *
 * MNRMES : reception d'un message
 *                     (acces utilisateur)
 *
 * appel  : monMessRec (pt_anc, attente)
 *
 * avec   : pt_anc  = inteur sur l'ancrage
 * 			attente = en millisecondes.
 *          attente = TRUE  si suspension de la tache
 *                    FALSE sinon
 *
 *******************************************************************************/
INT monMessRec(mqd_t MqId, int attente, char *mess)
{
	INT Status;
	int retour = 0;
	char Locmess[200];
	int notFin = 1;
uint32 tickCour_udw=ktickGetCurrent();

	printDebug("monMessRec : MqId %#x attente %d \n", MqId, attente);
	while (notFin)
	{
		if (mess == NULL)
		{
			Status = SysMqLire(MqId, Locmess);
		} else
		{
			Status = SysMqLire(MqId, mess);
		}
		if (Status == ERROR)
		{
			notFin = 0;
		} else
		{
			if (Status > 0)
			{
				/* Trace de debug pour savoir qui
				 * a recu le message.
				 */
				/*
				 * for (i = 0; i<supGestion.nbTache; i++)
				 {
				 if ( supGestion.Tache[i].Tid == taskIdSelf() )
				 {
				 printDebug("%s : Reception du message %s\n",
				 supGestion.Tache[i].Nom, mess);
				 }
				 }
				 *//* endfor(i=0                                           */
				retour = OK;
				notFin = 0;
			} else
			{

				if (attente != FALSE)
				{
					if (attente != (UINT8)SUSP_INFINIE)
					{
						if(ktickGetDeltaCurrent(tickCour_udw)>attente)
						{
							attente=FALSE;
						}
					}
					mnsusp(0);
				} else
				{
					retour = 2;
					notFin = 0;
				} /* endif(attente==TRUE                          */
			} /* endif(Status>0                                       */
		} /* endif(Status==ERROR                          */
	} /* endwhile(notFin)*/
	/* --------------------------------
	 * FIN DE monMessRec
	 * --------------------------------   */
	return retour;
}

/******************************************************************************
 *
 * MNRTS  : emission vers un uart
 *                     (acces utilisateur)
 *
 * appel  :  mnemuart (port,pt_mes)
 *
 *******************************************************************************/
VOID mnrts(UINT8 las)
{
	return;
}

/******************************************************************************
 *
 * MNINPUT : lecture d'un port
 *                     (acces utilisateur)
 *
 * appel  : data = mninput (port)
 *
 * avec   : port = adresse du port
 *          data = valeur lue
 *
 *******************************************************************************/
UINT8 mninput(UINT8 *adresse)
{
	UINT8 val;
	/* TODO : Portage LINUX */
#ifdef VXWORKS
	if(vxMemProbe((char *)adresse,VX_READ,1,&val)==ERROR)
	{
		printDebug("Probleme de lecture 8 bits e l'adresse %#0x\n",adresse);
		val=0;
	}
#endif
	return val;
}

/******************************************************************************
 *
 * MNSUSP : suspends la tache en execution pendant un certain temps
 *                     (acces utilisateur)
 *
 * appel  : mnsusp (duree)
 *
 * avec   : duree = 0          si retour sans suspension
 *                = 0xffffffff si suspension infinie
 *                = autre      si suspension temporisee
 *                > -1         si suspension en attente de signal
 *
 *******************************************************************************/
VOID mnsusp(INT tempo)
{
	INT Status;
	INT i;
#ifndef CPU432
	if(0==tempo)
	{
		tempo=1;
	}
#endif

	for (i = 0; i < supGestion.nbTache; i++)
	{
		if (taskGetId(&supGestion.Tache[i]) == tacheGetId())
		{
			Status = semSignal(supGestion.Tache[i].tacVersMon);
			if (Status == ERROR)
			{
				return;
			}
			if (tempo > 0)
			{
				ksleep(tempo*50);
			} else if (tempo == SUSP_INFINIE)
			{
				/* TODO : A priori, ce n'est jamais utilise.... */
				/* taskSuspend (taskIdSelf ()); */
			}
			Status = semWait(supGestion.Tache[i].monVersTac);
			if (Status == ERROR)
			{
				return;
			}
		}
	}
	return;
}

/******************************************************************************
 *
 * MNSUSPINI : a lancer au depart
 *                     (acces utilisateur)
 *
 * appel  : mnsusp (duree)
 *
 * avec   : duree = 0          si retour sans suspension
 *                = 0xffffffff si suspension infinie
 *                = autre      si suspension temporisee
 *                > -1         si suspension en attente de signal
 *
 *******************************************************************************/
VOID mnsuspIniPrendre()
{
	INT Status;
	INT i;
	bool trouve = false;

	for (i = 0; i < supGestion.nbTache; i++)
	{
		//	  int32 tacheId_dw=taskGetId(&supGestion.Tache[i]);
		//	  int32 tacheCour_dw=taskIdSelf();
		if (taskGetId(&supGestion.Tache[i]) == tacheGetId())
		{

			//    	  semSignal(supGestion.Tache[i].tacVersMon);
			Status = semWait(supGestion.Tache[i].monVersTac);
			trouve = true;
			//          if (Status == ERROR)
			//            {
			//              printDebug ("Erreur tache %s rend semaphore \n", supGestion.Tache[i].Nom);
			//              return;
			//            }
		}
	}
	if (false == trouve)
	{
		printDebug("_mnsuspIniPrendre : petit probleme tache %d\n", i);
	}
	return;
}

VOID mnsuspIniDonner()
{
	INT Status;
	INT i;
	for (i = 0; i < supGestion.nbTache; i++)
	{
		if (taskGetId(&supGestion.Tache[i]) == tacheGetId())
		{
			Status = semSignal(supGestion.Tache[i].monVersTac);
			if (Status == ERROR)
			{
#ifdef VXSIM
				printDebug ("Erreur tache %s rend semaphore \n",
						supGestion.Tache[i].nomTache);
#endif
				return;
			}
		}
	}
	return;
}

/******************************************************************************
 *
 * MNISOP   : lecture/ecriture d'un port  par chaine
 *                     (acces utilisateur)
 *
 * appel  :  mnisop (org,dest,nbcar)
 *
 * avec   : org  = l'adresse d'origine
 *          dest = l'adresse de destination
 *          nbcar= le nombre de caractere a transferer
 *
 *******************************************************************************/
void mnisop(STRING adresse, STRING s, LONG val)
{

	return;
}

/******************************************************************************
 *
 * MNECAL : ecriture du calendrier
 *                     (acces utilisateur)
 *
 * appel  : mnecal (pt_date)
 *
 * avec   : pt_date = adresse de la structure date
 *
 *******************************************************************************/
//
//VOID mnecal(STRING st1)
//{
//	struct timespec heure;
//	struct tm heureFmt;
//	/* On recupere l'ancienne heure pour
//	 * les millis.  */
//	clock_gettime(CLOCK_REALTIME, &heure);
//	/* Calcul du nombre de secondes.        */
//	heure.tv_sec = idf_sec(st1);
//	/* Positionnement de l'heure.           */
//	{
//		struct timeval hBis;
//		hBis.tv_sec=heure.tv_sec;
//		hBis.tv_usec=0;
//		localtime_r(&hBis.tv_sec, &heureFmt);
//	}
//	calSetDate(&heureFmt);
//	/* --------------------------------
//	 * FIN DE mnecal
//	 * --------------------------------     */
//	return;
//}

void mntcal()
{
	struct tm heure;
	heure.tm_sec = 1;
	heure.tm_min = 2;
	heure.tm_hour = 3;
	heure.tm_mday = 12;
	heure.tm_mon = 0;
	heure.tm_year = 106;
	heure.tm_wday = 4;
	heure.tm_yday = 11;
	calSetDate(&heure);
}

void mntcal_2()
{
	struct tm heure;
	calGetDate(&heure);
	printf("%02d/%02d/%04d %02d:%02d:%02d\n", heure.tm_mday, heure.tm_mon + 1, heure.tm_year + 1900, heure.tm_hour,
			heure.tm_min, heure.tm_sec);
}

/* --------------------------------
 * monMessSend
 * ===========
 * Emission d'un message dans une
 * boite aux lettres.
 * --------------------------------	*/

VOID monMessSend(mqd_t MqId, T_usr_ztf ** pt_mes, unsigned Longueur)
{
	INT status;
	status = SysMQEmettre(MqId, (char *) pt_mes, Longueur);

	printDebug("monMessSend : MqId %#x status %d(%d)\n", (unsigned int) MqId, status, errno);
	/* --------------------------------
	 * FIN DE monMessSend
	 * --------------------------------     */
}

/* --------------------------------
 * monMessInit
 * ===========
 * Initialisation d'un message
 * --------------------------------	*/

T_usr_ztf *monMessInit(mqd_t MqId, T_usr_ztf * message)
{
	message->entete.MqId = MqId;
	message->nbcar_max = SZ_BLK;
	message->nbcar_rec = 0;
	message->pt_buff = (STRING) message->buffer;
	message->compte_rendu = CR_OK;
	return message;
}

/******************************************************************************
 *
 * MNOUTPUT  : ecriture d'un port
 *                     (acces utilisateur)
 *
 * appel  :  mnoutput (port,data)
 *
 * avec   : port = adresse du port
 *          data = valeur a ecrire
 *
 *******************************************************************************/
VOID mnoutput(UINT8* adresse, UINT8 val)
{
	/* TODO : Portage LINUX */
#ifdef VXWORKS
	if(vxMemProbe((char *)adresse,VX_WRITE,1,&val)==ERROR)
	{
		printDebug("Probleme d'ecriture 8 bits e l'adresse %#0x\n",adresse);
	}
#endif
}

/******************************************************************************
 *
 * RQSYS : activation des procedures d'initialisation systeme
 *
 *******************************************************************************/
/*
 * mnTraitInitialisation
 * =====================
 * Anciennement rqsys, cette methode a en charge de realiser les initialisations de l'application.
 * Son role est peut être un peu identique a supInitVar et il faudrait voir a regrouper.
 */

VOID mnTraitInitialisation(VOID)
{
	tac_ctrl_init();
}

/**********************************************************/
/*                                                        */
/* MNLCAL : lecture du calendrier                         */
/*                                                        */
/**********************************************************/
//VOID mnlcal(STRING date)
//{
//	struct timespec heure;
//	clock_gettime(CLOCK_REALTIME, &heure);
//	idf_datebis(heure.tv_sec, date);
//	return;
//}

//LONG mnlcalSec(void)
//{
//	struct timespec heure;
//	clock_gettime(CLOCK_REALTIME, &heure);
//	return heure.tv_sec;
//}

/**********************************************************/
/*                                                        */
/*       procedure de recherche des cartes                */
/*                                                        */
/**********************************************************/
INT hdl_crt_trace(buffer)
	STRING buffer;
{
	INT nbcar;
	nbcar = sprintf(buffer, "CRT INT108=%d MSO=%d MSR-SIMUL=%d COM185=%d\n\r", vct_pres_int108, vct_nb_mso,
			vct_pres_msr_simul, vct_pres_com185[0]);
	return (nbcar);
}

/**********************************************************/
/*                                                        */
/*       procedure de recherche des cartes                */
/*                                                        */
/**********************************************************/
INT crt_type_init()
{

	return TRUE;
}

/**********************************************************/
/*                                                        */
/*       procedure de recherche des cartes                */
/*                                                        */
/**********************************************************/
VOID crt_rec_carte()
{

	return;
}
