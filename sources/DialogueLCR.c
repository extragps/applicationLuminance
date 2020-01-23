/*****************************************************************************/
/* 		FICHIER: %M%      REL: %I% DATE: %G%                        		 */
/*DOC*************************************************************************/
/*  					    						SIAT                     */
/*							8, parc technologique de la Sainte Victoire      */
/*												Le Cannet                    */
/*										13590 Meyreuil - FRANCE              */
/*				  							Tel. 04 42 58 63 71              */
/*****************************************************************************/
/*  Fichier : DialogueLCR.c                                                  */
/*  MODULE  : DialogueLCR                                                    */
/*****************************************************************************/
/* Auteur :   Herve ZEITTOUN                                                 */
/* Date de creation : 12 avril 2000                                          */
/*****************************************************************************/
/* OBJET DU MODULE      :                                                    */
/*                                                                           */
/*************************************************************************DOC*/
/* Liste des fonctions du fichier :                                          */
/*****************************************************************************/
/*                           Modifications                                   */
/*                      ***********************                              */
/* Auteur :                                                                  */
/* Date de la modification :                                                 */
/* Description :                                                             */
/*                      ***********************                              */
/*****************************************************************************/

/* ******************************** */
/* FICHIERS INCLUDE					*/
/* ********************************	*/

#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
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
#include "Pipe.h"
#include "anaSys.h"
#include "lcr_util.h"
#include "etaSyst.h"

/* ********************************	*/
/* DEFINITION DES CONSTANTES		*/
/* ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* -------------------------------
 * T_diaBuffer
 * ===========
 * Pour bufferiser les commandes
 * en entree sur la liaison serie.
 * ----------------------------	*/

typedef struct T_diaBuffer
{
	int phase;
	char buff[MAX_MESSAGE_LCR];
	short nbcar_rec;
	int mode;
} T_diaBuffer;

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

static void dialogueInit(T_diaBuffer * diaBuffer)
{
	diaBuffer->mode = -1;
	diaBuffer->nbcar_rec = 0;
	diaBuffer->phase = 0;
}

/* -----------------------------
 * dialogueBufferise
 * =================
 * Afin de bufferiser les commandes
 * Cette fonction est inspiree de
 * la fonction tedi_prot.
 * ----------------------------	*/


static int dialogueBufferise(T_diaBuffer *las, char *buffer,int nbCar)
{
	int retour = 0;
	int carCour=0;
	int car;
	/* debut du mode configuration */
	for (carCour = 0; (carCour < nbCar) && (0 == retour); carCour++)
	{
		car=buffer[carCour];

	switch (las->phase) {
	case 0:
		switch (car) {
		case ENQ:
			las->phase = 1;
			las->nbcar_rec = 0;
			las->buff[las->nbcar_rec++] = car;
			las->mode = MODE_PROTEGE;
			break;
		case '-':
			if (las->nbcar_rec == 0)
			{
				las->mode = MODE_TEST;
				las->phase = 3;
				las->nbcar_rec = 0;
			}
			las->buff[las->nbcar_rec++] = car;
			break;
		case ACK:
		case NACK:
			las->mode = MODE_PROTEGE;
			las->phase = 4;
			if (las->nbcar_rec < MAX_MESSAGE_LCR)
			{
				las->buff[las->nbcar_rec++] = car;
			}
			break;
		case '!':
		case '?':
			if ((las->mode == MODE_TERMINAL) && (las->nbcar_rec == 0))
			{
				las->phase = 4;
			}
			else if ((las->mode == -1) && (las->nbcar_rec == 0))
			{
				las->mode = MODE_TEST;
				las->phase = 4;
			}
			else if ((las->mode == MODE_TEST) && (las->nbcar_rec == 0))
			{
				las->phase = 4;
			}
			if (las->nbcar_rec < MAX_MESSAGE_LCR)
			{
				las->buff[las->nbcar_rec++] = car;
			}
			break;
			/*		case BS :
			 switch (las->mode)
			 {
			 case MODE_TERMINAL :
			 if (las->nbcar_rec)
			 las->nbcar_rec--;
			 break;
			 }
			 break;*/
		case CR:
			/* on regarde si on a un buffer pour recevoir */
			las->mode = MODE_TERMINAL;
			if (las->nbcar_rec < MAX_MESSAGE_LCR)
			{
				las->buff[las->nbcar_rec++] = car;
			}
			retour = 1;
			printDebug("C'est fini\n");
			break;
		case XON:
		case XOF:
			break;
		default:
			if (las->nbcar_rec < MAX_MESSAGE_LCR)
			{
				las->buff[las->nbcar_rec++] = car;
			}
			break;
		}
		break;
	case 1: /* protege */
		/* on traite suivant le caractere */
		switch (car) {
		case ENQ:
			las->nbcar_rec = 0;
			las->buff[las->nbcar_rec++] = car;
			las->mode = MODE_PROTEGE;
			break;
		case 0: /* le nombre de caractere */
			if (las->nbcar_rec < MAX_MESSAGE_LCR)
			{
				las->buff[las->nbcar_rec++] = car;
			}
			break;
		case ETX:
			las->phase = 2;
			/* no break */
		default:
			if (las->nbcar_rec < MAX_MESSAGE_LCR)
			{
				las->buff[las->nbcar_rec++] = car;
			}
			break;
		}
		break;
	case 2:
		if (las->nbcar_rec < MAX_MESSAGE_LCR)
		{
			las->buff[las->nbcar_rec++] = car;
		}
		retour = 1;
		break;
	case 3: /* test */
		switch (car) {
		case ENQ:
			las->nbcar_rec = 0;
			las->buff[las->nbcar_rec++] = car;
			las->mode = MODE_PROTEGE;
			break;
		case '-':
			las->nbcar_rec = 0;
			las->buff[las->nbcar_rec++] = car;
			las->mode = MODE_TEST;
			break;
		case CR:
			if (las->nbcar_rec < MAX_MESSAGE_LCR)
			{
				las->buff[las->nbcar_rec++] = car;
			}
			retour = 1;
			break;
		default:
			if (las->nbcar_rec < MAX_MESSAGE_LCR)
			{
				las->buff[las->nbcar_rec++] = car;
			}
			break;
		}
		break;
	case 4: /* no bloc */
		las->phase = 0;
		if (las->nbcar_rec < MAX_MESSAGE_LCR)
		{
			las->buff[las->nbcar_rec++] = car;
		}
		retour = 1;
		printDebug("C'est fini");
		break;
	}
	}
	return retour;
}

//static int dialogueBufferise(T_diaBuffer * las, char car)
//{
//	int retour = 0;
//	/* debut du mode configuration */
//	printDebug("Traitement du caractere %c\n", car);
//	switch (las->phase)
//	{
//	case 0:
//		switch (car)
//		{
//		case ENQ:
//			las->phase = 1;
//			las->nbcar_rec = 0;
//			las->buff[las->nbcar_rec++] = car;
//			las->mode = MODE_PROTEGE;
//			break;
//		case '-':
//			if (las->nbcar_rec == 0)
//			{
//				las->mode = MODE_TEST;
//				las->phase = 3;
//				las->nbcar_rec = 0;
//			}
//			las->buff[las->nbcar_rec++] = car;
//			break;
//		case ACK:
//		case NACK:
//			las->mode = MODE_PROTEGE;
//			las->phase = 4;
//			if (las->nbcar_rec < MAX_MESSAGE_LCR)
//			{
//				las->buff[las->nbcar_rec++] = car;
//			}
//			break;
//		case '!':
//		case '?':
//			if ((las->mode == MODE_TERMINAL) && (las->nbcar_rec == 0))
//			{
//				las->phase = 4;
//			} else if ((las->mode == -1) && (las->nbcar_rec == 0))
//			{
//				las->mode = MODE_TEST;
//				las->phase = 4;
//			} else if ((las->mode == MODE_TEST) && (las->nbcar_rec == 0))
//			{
//				las->phase = 4;
//			}
//			if (las->nbcar_rec < MAX_MESSAGE_LCR)
//			{
//				las->buff[las->nbcar_rec++] = car;
//			}
//			break;
//			break;
//			/*		case BS :
//			 switch (las->mode)
//			 {
//			 case MODE_TERMINAL :
//			 if (las->nbcar_rec)
//			 las->nbcar_rec--;
//			 break;
//			 }
//			 break;*/
//		case CR:
//			/* on regarde si on a un buffer pour recevoir */
//			las->mode = MODE_TERMINAL;
//			if (las->nbcar_rec < MAX_MESSAGE_LCR)
//			{
//				las->buff[las->nbcar_rec++] = car;
//			}
//			retour = 1;
//			printDebug("C'est fini");
//			break;
//		case XON:
//		case XOF:
//			break;
//		default:
//			if (las->nbcar_rec < MAX_MESSAGE_LCR)
//			{
//				las->buff[las->nbcar_rec++] = car;
//			}
//			break;
//		}
//		break;
//	case 1: /* protege */
//		/* on traite suivant le caractere */
//		switch (car)
//		{
//		case ENQ:
//			las->nbcar_rec = 0;
//			las->buff[las->nbcar_rec++] = car;
//			las->mode = MODE_PROTEGE;
//			break;
//		case 0: /* le nombre de caractere */
//			if (las->nbcar_rec < MAX_MESSAGE_LCR)
//			{
//				las->buff[las->nbcar_rec++] = car;
//			}
//			break;
//		case ETX:
//			las->phase = 2;
//		default:
//			if (las->nbcar_rec < MAX_MESSAGE_LCR)
//			{
//				las->buff[las->nbcar_rec++] = car;
//			}
//			break;
//		}
//		break;
//	case 2:
//		if (las->nbcar_rec < MAX_MESSAGE_LCR)
//		{
//			las->buff[las->nbcar_rec++] = car;
//		}
//		retour = 1;
//		break;
//	case 3: /* test */
//		switch (car)
//		{
//		case ENQ:
//			las->nbcar_rec = 0;
//			las->buff[las->nbcar_rec++] = car;
//			las->mode = MODE_PROTEGE;
//			break;
//		case '-':
//			las->nbcar_rec = 0;
//			las->buff[las->nbcar_rec++] = car;
//			las->mode = MODE_TEST;
//			break;
//		case CR:
//			if (las->nbcar_rec < MAX_MESSAGE_LCR)
//			{
//				las->buff[las->nbcar_rec++] = car;
//			}
//			retour = 1;
//			break;
//		default:
//			if (las->nbcar_rec < MAX_MESSAGE_LCR)
//			{
//				las->buff[las->nbcar_rec++] = car;
//			}
//			break;
//		}
//		break;
//	case 4: /* no bloc */
//		las->phase = 0;
//		if (las->nbcar_rec < MAX_MESSAGE_LCR)
//		{
//			las->buff[las->nbcar_rec++] = car;
//		}
//		retour = 1;
//		printDebug("C'est fini");
//		break;
//	}
//	return retour;
//}
//
/* --------------------------------	*/
/* DialogueLCR						*/
/* ===========						*/
/* La fonction a en charge la lec-	*/
/* ture du dialogue LCR sur une 	*/
/* liaison serie.					*/
/* --------------------------------	*/

FUNCPTR DialogueLCR(int numero, int numPort)
{
	INT status, i;
	int nbCar;
	struct timespec dateCmd;
	int notFin = 1;
	T_diaBuffer diaBuffer; /* Structure de donnees utilisee pour
	 * bufferiser les caracteres en
	 * entree. */
	char bufferCmd[MAX_MESSAGE_LCR]; /* Pour conserver la commande en
	 * cours. */

	int nbCarCmd = 0;
	/* Pour l'enregistrement des traces
	 * de reponses. */
	T_lcr_util_buffer repLcr;
	/* --------------------------------
	 * INITIALISATION
	 * --------------------------------     */
	printDebug("DialogueLCR : Init OK port %d\n\n", numPort);
	lcr_util_buffer_init(&repLcr, 78);
	dialogueInit(&diaBuffer);

	/* --------------------------------
	 * BOUCLE DE TRAITEMENT
	 * --------------------------------     */
	while (notFin)
	{
		int width;
		bool term_b;
		int fd = -1;
		fd_set readFds;
		struct timeval attente =
		{ 5, 0 };
		/* -----------------------------------
		 * Initialisation de la structure du
		 * Select.
		 * --------------------------------- */
		FD_ZERO (&readFds);
		width = 0;
		i = numPort;
		if (NULL != cnxLiaison[i])
		{
			fd = commCnxGetFd(cnxLiaison[i]);
			if (-1 != fd)
			{
				FD_SET (fd, &readFds);
				width = MAX(fd,width);
			}
		}
		if (-1 != MqLasDial[i])
		{
			FD_SET (MqLasDial[i], &readFds);
			width = MAX(MqLasDial[i],width);
		}
		width++;
		/* ----------------------------------
		 * Attente bloquante en lecture
		 * sur l'un des deux ports (las et mq)
		 * ----------------------------------*/
		switch (select(width, &readFds, NULL, NULL, &attente))
		{
		case ERROR:
			if(EINTR!=errno)
			{
				printDebug("Erreur de select : %#x liaison %d\n", errno, i);
				notFin = 0;
			}
			break;
		case 0:
			printDebug("Timeout sur reception. liaison %d\n", i);
			dialogueInit(&diaBuffer);
			/* Attention, ajout pour permettre de se reconnecter sur un port IP si
			 * celui ci n'a pas ete utilise pendant un certain temps. */
			if(xdg_cf_las[numPort].type_e==LIAISON_IP) {
				char data[10];
				/* Attention, il est visiblement possible que la connexion ne soit pas
				 * ouverte a ce stade.
				 */
				if (NULL != cnxLiaison[numPort]) {
					int fd_loc= commCnxGetFd(cnxLiaison[numPort]);
//					printf("Je suis ici et cnxLiaison %#0x et fd %d\n",cnxLiaison[numPort],fd_loc);
					if(-1!=fd_loc) {
						nbCar = commCnxReceive(cnxLiaison[numPort],(UINT8 *) data, 1);
						if(nbCar>0) {
							dialogueBufferise(&diaBuffer, data,nbCar);
						}
					}
				}
			}
			break;
		default:

			/* ----------------------------------
			 * Si des donnes sont arrivees sur
			 * la liaison LAS
			 * ----------------------------------*/
			if (-1 != fd)
			{
				if (FD_ISSET (fd, &readFds))
				{
					char data[MAX_MESSAGE_LCR];

					printDebug("Arrivee d'un message sur la liaison %d\n", i);
					/* ---------------------------------
					 * Lecture des donnees en entree de
					 * la liaison serie.
					 * ---------------------------------*/
					nbCar = commCnxReceive(cnxLiaison[i], data, MAX_MESSAGE_LCR);
					if (nbCar == ERROR)
					{

						printDebug("dialogueLCR: Erreur de lecture : %#x liaison %d\n", errno, i);
						notFin = 0;
					} else
					{
						int carCour;
						int retour = 0;
						/* ---------------------------------
						 * Scrutation PC
						 * --------------------------------     */
						spcSignaler();
						/* TODO: Etudier la chose. Pour l'instant, ce test permet
						 * d'eviter trop de traces de maintenance. */
						/* ---------------------------------
						 * Traitement des caracteres
						 * --------------------------------     */
						retour = dialogueBufferise(&diaBuffer, data,nbCar);

						if (retour == 1)
						{
							/* Pour stocker eventuellement
							 * les infos e retourner sur la
							 * commande systeme.    */
							T_lcr_util_buffer lcrAna;
							T_ficTraceAdresse adresse;
							char adTrace[10];
							sprintf(adTrace, "TRM%03d", i);
							ficTraceAdresseSetStandard(&adresse, adTrace);
							ficTraceSetAdressePort(i, &adresse);
							/* Initialisation du buffer qui
							 * sera eventullement utilise pour
							 * stocker la reponse e la commande
							 * systeme.     */
							lcr_util_buffer_init(&lcrAna, LCR_UTIL_MAX_BUFFER);
							if (anaSys(numPort, diaBuffer.buff, diaBuffer.nbcar_rec, &lcrAna) == 0)
							{
								/* Preparation des infos qui seront
								 * eventuellement utilisee pour le
								 * stockage des infos.  */
								if (1 == supRunLireEtat())
								{
									clock_gettime(CLOCK_REALTIME, &dateCmd);
									nbCarCmd = diaBuffer.nbcar_rec;
									memcpy(&bufferCmd, diaBuffer.buff, diaBuffer.nbcar_rec);

									if (MqDialLas[i] != -1)
									{
										char Buffer[MAX_MESSAGE_LCR + 2];
										/* Ecriture dans le message du
										 * nombre de caracteres et du
										 * message.                                                     */
										memcpy(Buffer, &diaBuffer.nbcar_rec, 2);
										memcpy(&Buffer[2], diaBuffer.buff, diaBuffer.nbcar_rec);
										status = SysPipeEmettre(MqDialLas[i], Buffer, diaBuffer.nbcar_rec + 2);
										if (status == ERROR)
										{
											printDebug("DialogueLCR : erreur "
												"SysPipeEmettre liaison %d\n", i);
										} else
										{
											ficTraceEnregistrerQuestion(&adresse, &dateCmd, diaBuffer. buff,
													diaBuffer. nbcar_rec);
										} /* endif(Status==ERROR                          */
									} else
									{
										printDebug("MqDialLas[i] non ouvert %d\n", i);
									} /* endif(MqDialLas[i]                           */
								}
							} else
							{
								/* Une commande systeme a ete reeue
								 * on effectue reponse. */
								if (0 == lcr_util_buffer_tester_fin(&lcrAna))
								{
									if (commCnxSend(cnxLiaison[i], lcr_util_buffer_lire_buffer(&lcrAna),
											lcr_util_buffer_lire_nb_car(&lcrAna)) == ERROR)
									{
										printDebug("DialogueLCR : erreur write %d\n", i);
										notFin = 0;
									} /* endif(write(fdLiaisonLas */
								}
							} /* endif(anaSys(numPort, */
							/* Reinitialisation du buffer */
							dialogueInit(&diaBuffer);
						} /* endif(retour==1) */
					} /* endif (nbCar == ERROR) */
				} /* endif (FD_ISSET(fdLiaisonLas[i], */
			}
			/* --------------------------------
			 * Lecture de la MQ de dialogue.
			 * --------------------------------*/
			if (FD_ISSET (MqLasDial[i], &readFds))
			{
				char bufferRec[MAX_MESSAGE_LCR + 2];
				int retour;
				memset(bufferRec, '\0', sizeof(bufferRec));
				status = SysPipeLire(MqLasDial[i], bufferRec, sizeof(bufferRec));
				if (status == ERROR)
				{
					printDebug("DialogueLCR : erreur de lecture sur le pipe %d(errno: %d)\n", i, errno);
					notFin = 0;
				}
				/* Trace des reponses et, si la celle
				 * ci est terminee, on enregistre dans
				 * le fichier.  */
				if (notFin)
				{
					retour = lcr_util_buffer_ajouter(&repLcr, bufferRec, status);
					if (0 != retour)
					{
						struct timespec dateRep;
						if (lcr_util_buffer_tester_fin(&repLcr) == 0)
						{
							T_ficTraceAdresse adresse;
							char adTrace[10];
							printDebug("On enregistre la reponse \n");
							clock_gettime(CLOCK_REALTIME, &dateRep);
							sprintf(adTrace, "TRM%03d", i);
							ficTraceAdresseSetStandard(&adresse, adTrace);
							ficTraceEnregistrerReponse(&adresse, &dateRep, &repLcr, bufferCmd, nbCarCmd);
							/* Si la reponse est correcte, on
							 * enregistre aussi dans la liste de
							 * commandes. */
							if (1 == retour)
							{
								ficTraceEnregistrerCommande(&adresse, &dateCmd, bufferCmd, nbCarCmd);
							}
						}
						/* Reinit des donnees pour l'enre-
						 * gistrement des reponses. */
						lcr_util_buffer_init(&repLcr, 78);
					} /* endif(lcr_util_buffer                        */
					printDebug("DialogueLCR : Reception message MQ(%d)\n", i);
					printDebug("DialogueLCR : Emmission du message vers PC \n");
					printDebug("DialogueLCR : Buffer %s\n", bufferRec);
					if (bufferRec[0] != '\\')
					{

						if (commCnxSend(cnxLiaison[i], bufferRec, status) == ERROR)
						{
							printDebug("DialogueLCR : erreur write : %d\n", errno);
							/* A la place du return... */
							notFin = 0;
						} /* endif(write(fdLiaisonLas                     */
					}
				} /* endif (FD_ISSET(MqDialLas[i],        */
			}
		} /* switch(select(width, &readFds,       */
		term_b=commCnxTerminalCheck(cnxLiaison[i]);
		if(term_b)
		{
			spcTrmSignaler();
		}
		/* Pour la gestion du watchDog          */
		etaSystIncrCompteur(numero);
	} /* Fin FOREVER                                          */
	/* --------------------------------
	 * FIN DE dialogueLCR
	 * --------------------------------     */
	return OK;
}
