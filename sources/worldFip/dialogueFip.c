/****************************************************************************/
/* 		FICHIER: %M%      REL: %I% DATE: %G%                        		*/
/*DOC************************************************************************/
/*  					    						SIAT                    */
/*							8, parc technologique de la Sainte Victoire     */
/*												Le Cannet                   */
/*										13590 Meyreuil - FRANCE             */
/*				  							Tel. 04 42 58 63 71             */
/****************************************************************************/
/*  Fichier : DialogueFip.c													*/
/*  MODULE  : DialogueFip                                                   */
/****************************************************************************/
/* Auteur :   Xavier GAILLARD												*/
/* Date de creation : 12 avril 2000                                         */
/****************************************************************************/
/* OBJET DU MODULE      :                                                   */
/*                                                                          */
/************************************************************************DOC*/
/* Liste des fonctions du fichier :                                         */
/****************************************************************************/
/*                           Modifications                                  */
/*                      ***********************                             */
/* Auteur :                                                                 */
/* Date de la modification :                                                */
/* Description :                                                            */
/*                      ***********************                             */
/****************************************************************************/
/*
 * Modifications :
 * ===============
 * 01b,01Oct01	Traitement des warnings WorldFip.
 * 				Parametrage de la tempo fip par la commande EXPTACHEC.
 * 01a,08Mar01	Ajout de la fonction permettant de traiter les messages
 * systemes.
 */

/* ******************************** */
/* FICHIERS INCLUDE					*/
/* ********************************	*/

#include <sys/times.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include "kcommon.h"
#include "define.h"
#include "mon_inc.h"
#include "standard.h"
#include "xdg_var.h"
#include "mon_debug.h"
#include "fdm.h"
#include "fdmtime.h"
#include "man_olga.h"
#include "man_fdmi.h"
#include "usr_fip.h"
#include "mcs.h"
#include "usr_fip_var.h"
#include "usr_fip_init.h"
#include "usr_fip_mess.h"
#include "ioPortLib.h"
#include "lcr_util.h"
#include "supRun.h"
#include "extLib.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "abtLib.h"
#include "perfLib.h"
#include "ficTrace.h"
#include "Pipe.h"
#include "anaSys.h"
#include "stAlLib.h"
#include "etaSyst.h"
#include "tac_ctrl.h"
#include "lcr_p.h"
#include "lcr_trc.h"
#include "ioPortLib.h"
#include "spcLib.h"
#include "usr_fip_mcs_init.h"
#include "dialogueFip.h"

/* ********************************	*/
/* DEFINITION DES CONSTANTES		*/
/* ********************************	*/
/* Constante ajoutee pour traiter le
 * probleme de l'arret du fipCode.      */
#define DIALOGUE_FIP_MAX_WARNING	20
#define DIALOGUE_FIP_TEMPO			20      /* Environ 30 secondes */

#define MAX_CAR		256            /* Ce n'est pas bien beau de declarer
                                        * cela ici. */
#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

int dialogue_fip_debug = 0;
static int cptDial = 0;
static int etatFip = 0;
static int fipMediumInit = 0;
static int fipMediumOk = 0;
static int cptFip = 0;
static int tempoTraceFip = 72000;
/* TODO : Pour les tests, on passe a 50 au lieu de 1200 */
static int tempoFirstResetFip = 1200;
//static int tempoFirstResetFip = 50;
static int tempoSecondResetFip = 400;
static int tempoWarning = 0;
static int fipCptWarning = 0;
static int fipTestReset = 0;
static int fipCptTempoReset = 0;
static int fipCptReinit = 0;

/* ********************************	*/
/* DEFINITION DES VARIABLES EXTERNES*/
/* ********************************	*/

extern TS_InfoVar usr_var_rapport;

/* ********************************	*/
/* DEFINITION DES VARIABLES LOCALES */
/* ********************************	*/

static T_perfElem tpTraitement;
static int dialogueFipTempo = 20000;
static int nbTok = 0;
static int nbFnok = 0;
static int nbFnokTotal = 0;

/* ********************************	*/
/* FONCTIONS DU MODULE              */
/* ********************************	*/

static void fipMediumSetOk(int val)
{
	if (val != fipMediumOk)
	{
		/* Positionnement de l'etat COM */
		if (val)
		{
			pipSetEtat(pipGetEtat() | PIP_COM);
		} else
		{
			pipSetEtat(pipGetEtat() & (~PIP_COM));
		}
		tac_ctrl_set_force(TRUE);
		fipMediumOk = val;
	}
}

int fipMediumGetOk()
{
	return fipMediumOk;
}

static void dialogueFipReset(int origine)
{
	volatile char adresse;

	printf("Reset de worldip, origine %d\n",origine);

	usr_fip_network_stop(C_NoError);
	/*
	 * Cette reinit de WorldFip est maintenant effectuee dans la
	 * procedure usr_fip_network_start.
	 * ----------------------------------------------
	 * *ptUnused=*(unsigned char volatile *)0xE00E4000;
	 * taskDelay(10);
	 * -----------------------------------------------------------	*/
	 etaSystSetPos(12,133);
	usr_fip_mess_liberer_tout();
	 etaSystSetPos(12,134);
	usr_fip_mess_init();
	 etaSystSetPos(12,135);
	ioPortLireAdresse(&adresse);
	 etaSystSetPos(12,136);
	while(C_NoError!=usr_fip_network_start((int) adresse))
	{
		ksleep(1000);
		etaSystIncrCompteur(12);
	}
	 etaSystSetPos(12,137);
	usr_fip_reset_cpt_warning();
	 etaSystSetPos(12,138);
	fipCptWarning = 0;
	fipMediumInit = 0;
	fipCptReinit++;
	fipCptTempoReset = 0;
}

/* --------------------------------
 * dialogueFipSetTestReset
 * =======================
 * --------------------------------	*/

void dialogueFipSetTestReset(int valeur)
{
	fipTestReset = valeur;
}

/* ---------------------------------
 * dialogueFipGetTestReset
 * =========================
 * --------------------------------	*/

static int dialogueFipGetTestReset(void)
{
	return fipTestReset;
}

/* --------------------------------
 * dialogueFipGetTempo
 * ===================
 * --------------------------------	*/

static int dialogueFipGetTempo(void)
{
	return dialogueFipTempo;
}

/* --------------------------------
 * dialogueFipSetTempo
 * ===================
 * Positionnement de la tempo en
 * millisecondes. Cette fonction
 * a ete ajoutee pour passer la
 * tempo e 20 ms en telechargement.
 * --------------------------------*/

void dialogueFipSetTempo(int tempo)
{
	dialogueFipTempo = tempo;
}

/* --------------------------------
 * dialogueFipIncrOvf
 * =================
 * Fonction d'incrementation du
 * compteur d'overflow.
 * --------------------------------	*/

static void dialogueFipIncrOvf(void)
{
	configIncrOvf();
	/* ---------------------------------
	 * FIN DE dialogueFipIncrOvf
	 * --------------------------------     */
}

/* --------------------------------
 * dialogueFipAnaSys
 * =================
 * Analyse e priori des informations
 * en provenance de WorldFip pour
 * eventuellement detecter un telechar-
 * gement.
 * --------------------------------	*/

void dialogueFipAnaSys(T_usr_fip_mess * mess, int fd, char *buffer, int longueur)
{
	/* Pour stocker eventuellement
	 * les infos e retourner sur la
	 * commande systeme.    */
	T_lcr_util_buffer lcrAna;
	int reconnue;
	/* Inserer ici le test eventuel de
	 * l'aquitement d'un message.   */
	if (stAlAcquitFip(PORT_FIP, &buffer[2], longueur - 3) == 1)
	{
		T_usr_fip_mess *suiv;
		printf("Histoire d'acquitement de message\n");

		usr_fip_mess_enlever(mess);
		usr_fip_mess_liberer(mess);
		suiv = usr_fip_mess_lire_premier();
		if (suiv != NULL)
		{
			usr_fip_mess_envoyer(suiv, usr_fip_get_fd());
		} /* endif(mess!=NULL                                     */
	} else
	{
		T_ficTraceAdresse adresse;
		ficTraceAdresseSetFip(&adresse, usr_fip_mess_get_dest(mess), usr_fip_mess_get_ident(mess) & 0xFFFFFF,
				(char) (usr_fip_mess_get_ident(mess) >> 24));
		ficTraceSetAdressePort(PORT_FIP, &adresse);
		lcr_util_buffer_init(&lcrAna, LCR_UTIL_MAX_BUFFER);
		reconnue = anaSys(PORT_FIP, &buffer[2], longueur - 2, &lcrAna);
		if (0 == reconnue)
		{
			/* Ecriture dans le message du
			 * nombre de caracteres et du
			 * message.                                                     */
			if (1 == supRunLireEtat())
			{
				int retour;
				retour = SysPipeEmettre(fd, buffer, longueur);
				if (retour != ERROR)
				{
					struct timespec dateCmd;
					clock_gettime(CLOCK_REALTIME, &dateCmd);
					perfDemarrer(&tpTraitement);
					ficTraceEnregistrerQuestion(&adresse, &dateCmd, &buffer[2], longueur - 2);
				} /* endif(Status==ERROR                          */
			} else
			{
				T_usr_fip_mess *suiv;
				/* NOUVELLE IMPLEMENTATION.     PAS DE
				 * REPONSE DANS LE MODE SOMMEIL         */
				if (TRUE)
				{
					mess->reponse = NULL;
					usr_fip_mess_enlever(mess);
					usr_fip_mess_liberer(mess);

				}
				/* else
				 {
				 lcr_util_buffer_init(&lcrAna,LCR_UTIL_MAX_BUFFER);
				 if(TRUE==extGet(EXT_ECE))
				 {
				 lcr_util_buffer_ajouter(&lcrAna,"%1",strlen("%1"));
				 }
				 else
				 {
				 lcr_util_buffer_ajouter(&lcrAna,"?",strlen("?"));
				 }
				 usr_fip_mess_ecrire_rep(mess,
				 lcr_util_buffer_lire_buffer(&lcrAna),
				 lcr_util_buffer_lire_nb_car(&lcrAna));
				 usr_fip_mess_send_message(mess);
				 } */
				/* EMISSION DU MESSAGE                          */
				/* Lorsque l'on a detecte un acquit     */
				/* Cas de la messagerie courte.         */
				/* Traitement du message suivant
				 *
				 * eventuel.                                            */
				suiv = usr_fip_mess_lire_premier();
				if (suiv != NULL)
				{
					usr_fip_mess_envoyer(suiv, usr_fip_get_fd());
				} /* endif(mess!=NULL                                     */
			} /* endif(1==supRunLireEtat(SUP_RU       */
		} else
		{
			T_usr_fip_mess *suiv;
			usr_fip_mess_ecrire_rep(mess, lcr_util_buffer_lire_buffer(&lcrAna), lcr_util_buffer_lire_nb_car(&lcrAna));
			/* EMISSION DU MESSAGE                          */
			/* Lorsque l'on a detecte un acquit     */
			/* Cas de la messagerie courte.         */
			usr_fip_mess_send_message_mcs(mess, 2 == reconnue);
			/* Traitement du message suivant
			 * eventuel.                                            */
			suiv = usr_fip_mess_lire_premier();
			if (suiv != NULL)
			{
				usr_fip_mess_envoyer(suiv, usr_fip_get_fd());
			} /* endif(mess!=NULL                                     */
		} /* endif(anaSys(PORT_FIP,                       */
	} /* endif(stAlAcquitFip                          */
	printf("Sortie de dialogueFipAnaSys\n");
}

/* --------------------------------
 * DialogueFip
 * ===========
 * La fonction a en charge le traitement
 * des evenements WorldFip.
 * Les traitements realises par le
 * programmes sont :
 * - initialisation de fip (chargement
 * 	du microcode, test de la ram),
 * - boucle de traitement periodique
 *   cadencee sur un select realise sur
 *   le pipe,
 * - appel des routines d'it de WorldFip
 * - mise e jour de la variable de rapport
 *   toutes les secondes.
 * --------------------------------	*/

FUNCPTR DialogueFip(int numero)
{
	int notFin = 0;
	T_lcr_util_buffer repLcr;
	struct timeval delai =
	{ 0, 20000 };
	int fip_ok=0;
	int compteur = 0; /* Compteur utilise pour la mise
	 * a jour de la variable de rapport
	 * toutes les 16 secondes.                      */
	/* --------------------------------
	 * INIT DES MESSAGES
	 * --------------------------------     */
	usr_fip_mess_init();
	usr_fip_set_fd(MqDialLas[PORT_FIP]);
	usr_fip_mess_set_ana_sys_fct(dialogueFipAnaSys);
	usr_fip_mess_set_ovf_fct(dialogueFipIncrOvf);
	lcr_util_buffer_init(&repLcr, 78);
	perfInit(&tpTraitement);
	/* --------------------------------
	 * INITIALISATION
	 * --------------------------------     */
	/* Demarrage de worldfip                */
	/* Pour l'instant on fait une boucle qui
	 * ne s'arrete pas si l'adresse worldFip
	 * ne peut pas être lue.... */

	while (0 == notFin)
	{
		char adresse;
		notFin = ioPortLireAdresse(&adresse);
		if (notFin)
		{
			usr_fip_network_start(adresse);
		} else
		{
			etaSystIncrCompteur(numero);
			ksleep(5000);
		}
	}
	/* --------------------------------
	 * BOUCLE DE TRAITEMENT
	 * --------------------------------     */
	while (notFin)
	{
		int width = 0;
		fd_set readFds;
		int retour;

		/* -----------------------------------
		 * Initialisation de la structure du
		 * Select.
		 * --------------------------------- */
		if (usr_fip_get_fd() != MqDialLas[PORT_FIP])
		{
			usr_fip_set_fd(MqDialLas[PORT_FIP]);
		}
		FD_ZERO (&readFds);

		if (MqLasDial[PORT_FIP] != -1)
		{
			FD_SET (MqLasDial[PORT_FIP], &readFds);
			if (MqLasDial[PORT_FIP] > width)
			{
				width = MqLasDial[PORT_FIP];
			}
		}
		width++;
		/* ----------------------------------
		 * Attente bloquante en lecture
		 * sur l'un des deux ports (las et mq)
		 * ----------------------------------*/
		retour = select(width, &readFds, NULL, NULL, &delai);
		switch (retour)
		{
		case ERROR:
			if (MqLasDial[PORT_FIP] == -1)
			{
				printf("Le pipe a ete ferme\n");
			} else
			{
				if (EINTR != errno)
				{
					printf("J'ai eu une erreur %d\n", errno);
					notFin = 0;
				}
			}
			break;
		default:
			/* --------------------------------
			 * Traiter les evenements en provenan-
			 * ce du pipe.
			 * --------------------------------     */
			if (FD_ISSET (MqLasDial[PORT_FIP], &readFds))
			{
				char buffer[MAX_CAR];
				int nbCar = SysPipeLire(MqLasDial[PORT_FIP], buffer, MAX_CAR);
				if (nbCar != -1)
				{
					T_usr_fip_mess *mess;

					mess = usr_fip_mess_lire_premier();
					if (mess != NULL)
					{
						int retour;
						/* Recuperation de l'identificteur
						 * de message et de l'adresse du
						 * destinataire. */
						/* Composition de l'adresse du
						 * destinataire. */

						retour = lcr_util_buffer_ajouter(&repLcr, buffer, nbCar);
						if (0 != retour)
						{
							int lgCommande;
							char *commande = usr_fip_mess_obtenir_commande(mess, &lgCommande);
							if (NULL != commande)
							{
								struct timespec dateRep;
								T_ficTraceAdresse adresse;
								clock_gettime(CLOCK_REALTIME, &dateRep);
								ficTraceAdresseSetFip(&adresse, usr_fip_mess_get_dest(mess), usr_fip_mess_get_ident(
										mess) & 0xFFFFFF, (char) (usr_fip_mess_get_ident(mess) >> 24));
								ficTraceEnregistrerReponse(&adresse, &dateRep, &repLcr, commande, lgCommande);
								perfArreter(&tpTraitement);
								if (monDebugGet(MON_DEBUG_PERF_FIP))
								{
									printf("\nTraitement commande fip ");
									perfImprimer(&tpTraitement);
								}
								if (1 == retour)
								{
									ficTraceEnregistrerCommande(&adresse, &mess->dateCmd, commande, lgCommande);
								} /* endif(1==retour                                      */
								printf("Liberation du bloc %#0x \n", commande);
								kmmFree (NULL,(void *) commande);
							}
							/* Reinit des donnees pour l'enre-
							 * gistrement des reponses. */
							lcr_util_buffer_init(&repLcr, 78);
						} /* endif(lcr_util_buffer                        */
						/* On ajoute les caracteres trouves
						 * e la reponse.        */
						retour = usr_fip_mess_ecrire_rep(mess, buffer, nbCar);
						/* EMISSION DU MESSAGE                          */
						/* Lorsque l'on a detecte un acquit     */
						if (retour)
						{
							/* Cas de la messagerie courte.         */
							usr_fip_mess_send_message(mess);
							/* Traitement du message suivant
							 * eventuel.                                            */
							mess = usr_fip_mess_lire_premier();
							if (mess != NULL)
							{
								usr_fip_mess_envoyer(mess, usr_fip_get_fd());
							} /* endif(mess!=NULL                                     */
						} /* endif(retour)                                        */
					}
				} /* endif(nbCar!=-1)                                     */
			} /* endif(FD_ISSET                                       */
			/* Fin du traitement par defaut.        */
		case 0:
			/* ----------------------------------
			 * Realiser ici les traitements
			 * WorldFip
			 * --------------------------------     */
			while (fdm_process_it_irq(usr_fip_ref) == IRQ_TO_PROCESS) ;
			while (fdm_process_it_eoc(usr_fip_ref) == IRQ_TO_PROCESS)
				;
			/* Increment du compteur FIP            */
			fdm_ticks_counter();
			/* Traitement des evenements even-
			 * tuels.                                                       */
			if (usr_fip_process())
			{
				spcSignaler();
			}
			/* --------------------------------
			 * TRAITEMENT DES MESSAGES MCS
			 * --------------------------------     */
			/* Il s'agit de depiler les traite-
			 * ments MCS e realiser.        */
			if (usr_fip_mcs_process())
			{
				spcSignaler();
			}
			/* --------------------------------
			 * MISE A JOUR DE L'HEURE
			 * -------------------------------- */
			if (usr_fip_date_recue)
			{
				FDM_MPS_VAR_DATA data;
				FDM_MPS_READ_STATUS status;
				status = fdm_mps_var_read_loc(usr_var_date.VarRef, (void *) &data);
				/* Lecture OK de la date                */
				/* Pas de mise a l'heure si il  */
				/* y en a deja une en route.    */
				printf("Date recue, traitement en cours\n");
				if (0 == status.Report)
				{
					T_VarComm_dateEtHeure *dateEtHeure;
					struct timespec heure;
					dateEtHeure = (T_VarComm_dateEtHeure *) &data.FBuffer[0];
					clock_gettime(CLOCK_REALTIME, &heure);
					if (heure.tv_sec != dateEtHeure->dateSec)
					{
						struct tm dateCal;
						unsigned long dateSec = dateEtHeure->dateSec;
						unsigned long dateNano = dateEtHeure->dateUSec;
						unsigned long rafDyn = dateEtHeure->rafDyn;
						int diffDate;
						int diffDateSec;
						dateSec += (dateNano + rafDyn * 1000) / 1000000000;
						dateNano = (dateNano + rafDyn * 1000) % 1000000000;
						diffDateSec = ((int) heure.tv_sec - (int) dateSec);
						diffDate = (diffDateSec) * 1000000000 + ((int) heure.tv_nsec - (int) dateNano);
						if ((abs(diffDateSec) > 2) || (abs(diffDate) > 1100000000))
						{
							printf("plus de 1 seconde de diff %ld\n", vct_horodate.diff_sec);
							localtime_r((const time_t *) &dateEtHeure->dateSec, &dateCal);
							calSetDate(&dateCal);
						}
					}
				}
				usr_fip_date_recue = 0;
			}
			/* --------------------------------
			 * MISE A JOUR VARIABLE DE RAPPORT
			 * --------------------------------     */
			 etaSystSetPos(numero,130);
			if ((++compteur % 30) == 0)
			{
				FDM_REPORT_VAR rapport;
				unsigned short valRet;
				fdm_mps_var_write_loc(usr_var_rapport.VarRef, (void *) &usr_varComm_rapport);
				/* Traitement des warnings.
				 * Si le compteur de warning depasse
				 * un certain nombre, on effectue un
				 * reset de l'interface WorldFip.
				 * Si le nombre de warning ne varie
				 * pas pendant 30 secondes, on considere
				 * que le probleme etait passage.       */
				if (0 != dialogueFipGetTestReset())
				{
					dialogueFipSetTestReset(0);
					{
						char bufferTemp[100];
						sprintf(bufferTemp, "WORLD FIP : Test reset");
						cmd_trc_tr(bufferTemp);
						dialogueFipReset(1);
					}
				}
				if (fipCptWarning != usr_fip_get_cpt_warning())
				{
					fipCptWarning = usr_fip_get_cpt_warning();
					tempoWarning = 0;
					if (fipCptWarning > DIALOGUE_FIP_MAX_WARNING)
					{
						char bufferTemp[100];
						sprintf(bufferTemp, "WORLD FIP : Reset defaut reseau %d", fipCptReinit);
						cmd_trc_tr(bufferTemp);
						dialogueFipReset(2);
					}
				} else
				{
					if (0 != fipCptWarning)
					{
						if (++tempoWarning > DIALOGUE_FIP_TEMPO)
						{
							usr_fip_reset_cpt_warning();
							tempoWarning = 0;
							fipCptWarning = 0;
						}
					}
				}

				/* fdm_read_report(usr_fip_ref,ioPortLireAdresse()); */
				valRet = fdm_get_local_report(usr_fip_ref, &rapport);
				{
					if (0 == valRet)
					{
						if ((nbTok != rapport.Nb_Of_Transaction_Ok_1) || (nbFnok != rapport.Nb_Of_Frames_Nok_1))
						{
							nbTok = rapport.Nb_Of_Transaction_Ok_1;
							nbFnok = rapport.Nb_Of_Frames_Nok_1;
							nbFnokTotal += nbFnok;
						}
						if (etatFip != rapport.Activity_Status)
						{
							char bufferTemp[100];
							sprintf(bufferTemp, "WORLD FIP : Etat Tok %d Fnok %d FnokTotal %d etat %04X",
									rapport.Nb_Of_Transaction_Ok_1, rapport.Nb_Of_Frames_Nok_1, nbFnokTotal,
									rapport.Activity_Status);
							cmd_trc_tr(bufferTemp);
							etatFip = rapport.Activity_Status;
							if (0x100 & rapport.Activity_Status)
							{
								fipMediumSetOk(1);
							} else
							{
								fipMediumSetOk(0);
							}
							if (1 == fipMediumGetOk())
							{
								fipMediumInit = 1;
								fipCptTempoReset = 0;
							}
						}
						if (0 == fipMediumGetOk())
						{
							if (1 == fipMediumInit)
							{
								/* Le medium a ete ok a une epoque et maintenant, rien
								 * ne marche plus, on fait un reset au bout d'un certain
								 * temps. */
								if (++fipCptTempoReset > tempoSecondResetFip)
								{
									char bufferTemp[100];
									sprintf(bufferTemp, "WORLD FIP : Reset tempoSecondReset  %d", fipCptReinit);
									cmd_trc_tr(bufferTemp);
									dialogueFipReset(3);
								}
							}
							else
							{
								if (++fipCptTempoReset > tempoFirstResetFip)
								{
									char bufferTemp[100];
									sprintf(bufferTemp, "WORLD FIP : Reset tempoFirstReset %d", fipCptReinit);
									cmd_trc_tr(bufferTemp);
									dialogueFipReset(4);
									fipCptWarning = 0;
									fipMediumInit = 0;
									fipCptReinit++;
									fipCptTempoReset = 0;

								}
							}
						}
					} else
					{
						cmd_trc_tr("WORLD FIP : Erreur de demande de rapport");
					}
				}

			} /* endif((compteur%100     */
			 etaSystSetPos(numero,150);
			/* --------------------------------
			 * FIN DE LA BOUCLE DE TRAITEMENT
			 * --------------------------------     */
			/* --------------------------------
			 * TEST DES MESSAGES EN TRAITEMENT
			 * --------------------------------     */
			{
				T_usr_fip_mess *mess = usr_fip_mess_lire_premier();
				if (mess != NULL)
				{
					struct timespec dateCour;
					clock_gettime(CLOCK_REALTIME, &dateCour);
					/* Recuperation de l'age du message
					 * et si celui ci est trop vieux, on
					 * le poubellise et on passe au
					 * suivant.     */
					if (usr_fip_mess_query_timeout(mess, &dateCour))
					{
						usr_fip_mess_enlever(mess);
						usr_fip_mess_liberer(mess);
						mess = usr_fip_mess_lire_premier();
						if (mess != NULL)
						{
							usr_fip_mess_envoyer(mess, usr_fip_get_fd());
						} /* endif(mess!=NULL        */
					} /* endif(usr_fip_mess_query_timeout */
				} /* endif(mess!=NULL)       */
			}
			/* Test des alertes pour le port
			 * worldFip.                                            */
			if (1 == supRunLireEtat())
			{
				stAlTester(PORT_FIP);
				delai.tv_usec = dialogueFipGetTempo();
			} else
			{
				delai.tv_usec = 20000;
			}
			break;
		} /* endswitch(retour                 */
		/* Increment du compteur pour le
		 * watchdog.                                            */
		etaSystSetPos(numero,0);
		etaSystIncrCompteur(numero);
	} /* endwhile(notFin                  */
	/* --------------------------------
	 * FIN DE DialogueFip
	 * --------------------------------     */
	return OK;
}
