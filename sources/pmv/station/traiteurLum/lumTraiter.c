#include <stdio.h>
#include "kcommon.h"
#include "klog.h"
#include "pip_def.h"

#include "ecouteur.h"
#include "rdtComm.h"
#include "lumAdresse.h"
#include "lumMsg.h"
#include "luminance/lumMsgCreer.h"
#include "event.h"
#include "configuration.h"
#include "sequenceur.h"
#include "lumTraiter.h"
#include "infos/lumInfos.h"
#include "affichage/zone.h"
#include "affichage/page.h"
#include "affichage/caractere.h"
#include "affichage/affichage.h"
#include "affichage/afficheur.h"
#include "cartes.h"
#include "configurationSram.h"

#include "standard.h"
#include "MQ.h"
#include "mon_def.h"
#include "mon_str.h"
#include "mon_pro.h"
#include "etaSyst.h"
#include "Superviseur.h"

static int32 _lumTraiteurErr_dw = 0;

static void _lumTraiteurIncrErr(void)
{
	_lumTraiteurErr_dw++;
}

int32 lumTraiteurGetErr(void)
{
	return _lumTraiteurErr_dw;
}

/* *********************************
 * VARIABLES LOCALES
 * **********************************	*/

static LumTraiteur *_lumTraiteur_pt = NULL;

/* *********************************
 * FONCTIONS DU MODULE
 * **********************************	*/

LumTraiteur *lumTraiteurNew()
{
	LumTraiteur *traiteur_pt = (LumTraiteur *) kmmAlloc(NULL,
			sizeof(LumTraiteur));
	if (NULL!=traiteur_pt)
	{
		traiteur_pt->cour_pt = NULL;
		traiteur_pt->conn_pt = NULL;
		traiteur_pt->event_pt = keventNew(true);
		traiteur_pt->mutex_pt = kmutexNew();
		traiteur_pt->listeReponses_pt = klistNew(0, 0, NULL,lumMsgDelete);
		traiteur_pt->listeMessages_pt = klistNew(0, 0, lumMsgCompare,
				lumMsgDelete);
		traiteur_pt->listeAffichages_pt = klistNew(0, 0, NULL,lumMsgDelete);
		traiteur_pt->sequenceur_pt = NULL;
		traiteur_pt->messCour_pt = NULL;
		traiteur_pt->messAff_b = false;
		traiteur_pt->numAffCour_dw = -1;
		traiteur_pt->ecouteursLum_pt = listeEcouteursNew();

	}
	return traiteur_pt;
}

void lumTraiteurSetSequenceur(LumTraiteur *traiteur_pt, Sequenceur *seq_pt)
{
	if (NULL!=traiteur_pt)
	{
		traiteur_pt->sequenceur_pt = seq_pt;
	}
}
void lumTraiteurDelete(void *traiteur_p)
{
	LumTraiteur *traiteur_pt = (LumTraiteur *) traiteur_p;
	if (NULL!=traiteur_pt)
	{
		Kmutex *mutex_pt = traiteur_pt->mutex_pt;
		if (NULL!=mutex_pt)
		{
			kmutexLock(mutex_pt);
		}
		if (NULL!=traiteur_pt->thread_pt)
		{
			if (!kthreadIsStopped(traiteur_pt->thread_pt))
			{
				kthreadStop(traiteur_pt->thread_pt);
			}
			kthreadDelete(traiteur_pt->thread_pt);
		}
		lumTraiteurSetComm(traiteur_pt, NULL);
		if (NULL!=traiteur_pt->cour_pt)
		{
			lumMsgDelete(traiteur_pt->cour_pt);
		}
		listeEcouteursDelete(traiteur_pt->ecouteursLum_pt);
		lumMsgDelete(traiteur_pt->messCour_pt);
		klistDelete(traiteur_pt->listeReponses_pt);
		klistDelete(traiteur_pt->listeMessages_pt);
		klistDelete(traiteur_pt->listeAffichages_pt);
		keventDelete(traiteur_pt->event_pt);
		/* TODO : Faire ce qui va bien pour supprimer les ressources
		 * allouées pour le thread de traitement. */
		if (NULL!=mutex_pt)
		{
			traiteur_pt->mutex_pt = NULL;
			kmutexUnlock(mutex_pt);
			kmutexDelete(mutex_pt);
		}
		kmmFree(NULL,traiteur_pt);
	}

}
void lumTraiteurStart(LumTraiteur *traiteur_pt)
{
	if (NULL!=traiteur_pt->thread_pt)
	{
		kthreadStart(traiteur_pt->thread_pt);
	}
}

void lumTraiteurStop(LumTraiteur *traiteur_pt)
{
	if (NULL!=traiteur_pt->thread_pt)
	{
		kthreadStop(traiteur_pt->thread_pt);
	}
}

void lumTraiteurSetComm(LumTraiteur *traiteur_pt, CommCnx *conn_pt)
{
	if (NULL!=traiteur_pt)
	{
		if (NULL!=traiteur_pt->conn_pt)
		{
			commCnxStop(traiteur_pt->conn_pt);
			commCnxDelete(traiteur_pt->conn_pt);
			traiteur_pt->conn_pt = NULL;
		}
		if (NULL==traiteur_pt->conn_pt)
		{
			traiteur_pt->conn_pt = conn_pt;
		}
	}
}

LumMsg *lumTraiteurGetMessCour(LumTraiteur *traiteur_pt)
{
	LumMsg *rep_pt = NULL;
	;
	kmutexLock(traiteur_pt->mutex_pt);
	if (NULL!=traiteur_pt->messCour_pt)
	{
		rep_pt = lumMsgClone(traiteur_pt->messCour_pt);
	}
	kmutexUnlock(traiteur_pt->mutex_pt);
	return rep_pt;
}
void lumTraiteurSetMessCour(LumTraiteur *traiteur_pt, LumMsg *msg_pt)
{
	kmutexLock(traiteur_pt->mutex_pt);
	if (NULL!=traiteur_pt->messCour_pt)
	{
		lumMsgDelete(traiteur_pt->messCour_pt);
		traiteur_pt->messCour_pt = NULL;
	}

	traiteur_pt->messCour_pt = msg_pt;
	kmutexUnlock(traiteur_pt->mutex_pt);
}

void lumTraiterAjouterLumMessage(void *traiteur_p, void *event_p)
{
	LumTraiteur *traiteur_pt = (LumTraiteur *) traiteur_p;
	/* Prendre le mutex de gestion de la liste */
	if (NULL!=traiteur_pt)
	{
		/* Cloner le message */
		LumMsg *msg_pt = lumMsgClone((LumMsg *) event_p);
		/* Si il s'agit d'un message de demarrage, on purge la liste de messages. */
		if (lumAdresseGetEquipement(lumMsgGetDest(msg_pt)) == LUM_EQUIP_ES)
		{
			if (lumMsgGetType(msg_pt) == LUM_TYPE_FCT)
			{
				if (lumMsgGetTabCar(msg_pt)[0] == LUM_MSG_OUTPUT_TOR_WRITE)
				{
					/* Il s'agit d'une programmation de demarrage ou d'arret,
					 * on force la suppression des messages. */
					kmutexLock(traiteur_pt->mutex_pt);
					klistDeleteElements(traiteur_pt->listeMessages_pt);
					kmutexUnlock(traiteur_pt->mutex_pt);

				}
			}
		}

		kmutexLock(traiteur_pt->mutex_pt);
		/* Ajouter le message */
		if (false == klistAdd(traiteur_pt->listeMessages_pt, msg_pt))
		{
			/* Le message est deja present, on le detruit. */
			klogPut(NULL,LOG_INFO,
					"lumTraiterAjouterMessage : message deja present");
			lumMsgDelete(msg_pt);
		}
		kmutexUnlock(traiteur_pt->mutex_pt);
		/* Signaler le process de traitement */
		keventSignal(traiteur_pt->event_pt);

	}

}

void lumTraiterAjouterLumAffichage(void *traiteur_p, void *event_p)
{
	LumTraiteur *traiteur_pt = (LumTraiteur *) traiteur_p;
	/* Prendre le mutex de gestion de la liste */
	if (NULL!=traiteur_pt)
	{
		/* Cloner le message */
		LumMsg *msg_pt = lumMsgClone((LumMsg *) event_p);
		int numAffCour_dw = -1;

		kmutexLock(traiteur_pt->mutex_pt);
		/* Ajouter le message */

		numAffCour_dw = lumMsgGetNumAff(msg_pt);
		if (-1 != numAffCour_dw)
		{
			if (traiteur_pt->numAffCour_dw != numAffCour_dw)
			{
				/* Supprimer tous les elements de la liste d'affichage */
				klistDeleteElements(traiteur_pt->listeAffichages_pt);
				/* Supprimer la consigne de luminosite */
				/* Supprimer la demande d'activation si elle est positionnee */
				/* Positionner le numero d'affichage courant */
				traiteur_pt->numAffCour_dw = numAffCour_dw;
			}
			/* Si il s'agit d'une consigne de lumindosite */
			if (lumMsgGetType(msg_pt) == LUM_TYPE_FCT)
			{
				if (lumMsgGetTabCar(msg_pt)[0] != LUM_MSG_LUMIN_SET)
				{
					/* Ajout du message dans la liste */
					if(false==klistAdd(traiteur_pt->listeAffichages_pt, msg_pt))
					{
						klogPut(NULL,	LOG_INFO,
												"lumTraiterAjouterMessage :ajout impossible");
						lumMsgDelete(msg_pt);
					}
				}
			}
			else
			{
				/* Big probleme, on ne devrait pas arriver ici */
				klogPut(
						NULL,
						LOG_INFO,
						"lumTraiterAjouterMessage : type de message incorrect %d",
						lumMsgGetType(msg_pt));
				lumMsgDelete(msg_pt);
			}

			{
				/* Le message est deja present, on le detruit. */
				klogPut(NULL,LOG_INFO,
						"lumTraiterAjouterMessage : message deja present");
				lumMsgDelete(msg_pt);
			}
		}
		else
		{
			klogPut(NULL,LOG_INFO,
					"lumTraiterAjouterLumAffichage : pas de numero d'affichage.");
			lumMsgDelete(msg_pt);
		}
		kmutexUnlock(traiteur_pt->mutex_pt);
		/* Signaler le process de traitement */
		keventSignal(traiteur_pt->event_pt);

	}

}

void lumTraiterAjouterLumReponse(void *traiteur_p, void *event_p)
{
	LumTraiteur *traiteur_pt = (LumTraiteur *) traiteur_p;
	/* Prendre le mutex de gestion de la liste */
	if (NULL!=traiteur_pt)
	{
		/* Cloner le message */
		LumMsg *msg_pt = lumMsgClone((LumMsg *) event_p);
		kmutexLock(traiteur_pt->mutex_pt);
		/* Ajouter le message */
		if(false==klistAdd(traiteur_pt->listeReponses_pt, msg_pt))
		{
			lumMsgDelete(msg_pt);
			klogPut(NULL,LOG_INFO,"lumTraiterAjouterLumReponse: ajout impossible de la reponse");
		}
		kmutexUnlock(traiteur_pt->mutex_pt);
		/* Signaler le process de traitement */
		keventSignal(traiteur_pt->event_pt);

	}
}
/*
 * lumTraiterMessage
 * =================
 * Traitement des reponses de messages.
 * Lors du traitement, on peut avoir eventuellement à reprogrammer un evenement.
 *
 * */

static void lumTraiterMessage(LumTraiteur *traiteur_pt, LumMsg *msg_pt,
		LumMsg *ori_pt)
{
	/* Tester le type de carte et en fonction de cela, orientation vers la bonne routine
	 * de traitement */
	LumEquipType type_e = lumAdresseGetEquipement(lumMsgGetSource(msg_pt));
	Event *evt_pt = NULL;
	switch (type_e)
	{
	case LUM_EQUIP_LEDS:
		klogPut(NULL,LOG_INFO, "lumTraiterMessage : carte driver leds");
		evt_pt = afficheursTraiterMessage(msg_pt, ori_pt);
		break;
	case LUM_EQUIP_ES:
		evt_pt = essTraiterMessage(msg_pt);
		break;
	case LUM_EQUIP_THL:
		evt_pt = thlsTraiterMessage(msg_pt);
		break;
	case LUM_EQUIP_PC:
	default:
		klogPut(NULL,LOG_INFO, "lumTraiterMessage : carte non traitee");
		break;
	}
	if (NULL!=evt_pt)
	{
		sequenceurReprogrammerEvent(traiteur_pt->sequenceur_pt, evt_pt);
	}
}

static void lumTraiterDefautCom(LumTraiteur *traiteur_pt, LumMsg *msg_pt)
{
	/* Tester le type de carte et en fonction de cela, orientation vers la bonne routine
	 * de traitement */
	LumEquipType type_e = lumAdresseGetEquipement(lumMsgGetDest(msg_pt));
	switch (type_e)
	{
	case LUM_EQUIP_LEDS:
		klogPut(NULL,LOG_INFO, "lumTraiterDefautCom : carte driver leds");
		afficheursTraiterDefautCom(msg_pt);
		break;
	case LUM_EQUIP_ES:
		essTraiterDefautCom(msg_pt);
		break;
	case LUM_EQUIP_THL:
		thlsTraiterDefautCom(msg_pt);
		break;
	case LUM_EQUIP_PC:
	default:
		klogPut(NULL,LOG_INFO, "lumTraiterDefautCom : carte non traitee");
		break;
	}
}
//static void * lumTraiterThread(LumTraiteur *traiteur_pt )

LumTraiteur *lumTraiteurGet()
{
	if (NULL==_lumTraiteur_pt)
	{
		_lumTraiteur_pt = lumTraiteurNew();
	}
	return _lumTraiteur_pt;
}

void lumTraiteur_init(void)
{
	LumTraiteur *trait_pt = lumTraiteurGet();
	Ecouteur *ecouteur_pt = ecouteurNew(lumTraiterAjouterLumReponse, trait_pt);
	lumTraiteurSetComm(trait_pt, supInitGetConnLuminance());

	lumMsgCreateurAjouterEcouteur(DialogueLuminanceGet(), ecouteur_pt);

}

void lumTraiteur_term(void)
{
	lumTraiteurDelete(lumTraiteurGet());
}

void lumTraiteur_main(int numero)
{
	LumTraiteur *traiteur_pt = lumTraiteurGet();
	int32 attente = 1000;

	//	mnsuspIniPrendre();

	//	while(!kthreadIsStopped(thread_pt))
	while (true)
	{
		keventTimedWait(traiteur_pt->event_pt, attente);
		/* On repositionne l'attente par défaut à une seconde */
		attente = 200;
		/* Depilement des réponses éventuelles. */
		{
			bool notFin = true;
			while (notFin)
			{
				LumMsg *lumMsg_pt = NULL;
				kmutexLock(traiteur_pt->mutex_pt);
				lumMsg_pt = klistFirst(traiteur_pt->listeReponses_pt);
				if (NULL!=lumMsg_pt)
				{
					klistRemove(traiteur_pt->listeReponses_pt, lumMsg_pt);
					klogPut(NULL,LOG_INFO, "lumTraiterThread : reponseRecu");
				}
				else
				{
					notFin = false;
				}
				kmutexUnlock(traiteur_pt->mutex_pt);
				if (NULL!=lumMsg_pt)
				{
					if (true == lumMsg_pt->ok)
					{
						LumMsg *messCour_pt = lumTraiteurGetMessCour(
								traiteur_pt);
						/* Traitement du message recu */
						lumTraiterMessage(traiteur_pt, lumMsg_pt, messCour_pt);
						/* Suppression du message courant */
						klogPut(NULL,LOG_INFO,
								"lumTraiterThread : suppression message courant");
						/* ------------------------------------------- */
						lumTraiteurSetMessCour(traiteur_pt, NULL);

						klogPut(NULL,LOG_INFO,
								"lumTraiterThread : suppression reponse");
						lumMsgDelete(messCour_pt);
					}
					else
					{
						cmd_trc_tr("ERREUR REPONSE LUMINANCE (%d,%d)",
								lumAdresseGetEquipement(lumMsgGetSource(
										lumMsg_pt)), lumAdresseGetLumAdresse(
										lumMsgGetSource(lumMsg_pt)), 0);
					}
					lumMsgDelete(lumMsg_pt);
				}
			}
		}
		/* Traitement du message en cours d'envoi. Si aucune reponse n'a ete fournie au
		 * bout du timeout, on supprimer le message et on déclare une erreur.
		 * On pourra dans un second temps faire un retry. */
		if (NULL!=traiteur_pt->messCour_pt)
		{
			uint32 delta_udw;
			delta_udw = ktickGetDeltaCurrent(traiteur_pt->lastTick_dw);
			/* test du timeout de reception de reponse */
			if (delta_udw > 1000)
			{
				/* La reponse n'est pas arrivee dans les temps. on trace une
				 * erreur sur l'afficheur qui a posé problème */
				klogPut(NULL,LOG_ERROR,
						"lumTraiterThread : nom reponse de l'afficheur %d",
						lumAdresseGetLumAdresse(lumMsgGetDest(
								traiteur_pt->messCour_pt)));
				/* Ajout du traitement de la non reponse */
				_lumTraiteurIncrErr();
				lumTraiterDefautCom(traiteur_pt, traiteur_pt->messCour_pt);

				/* Suppression du message courant */
				lumTraiteurSetMessCour(traiteur_pt, NULL);
			}
		}
		/* Traitement des messages dans la file d'envoi si il n'y a pas
		 * de message en attente de réponse. */

		if (NULL==traiteur_pt->messCour_pt)
		{
			LumMsg *lumMsg_pt = NULL;
			bool traite_b = false;

			while (false == traite_b)
			{
				kmutexLock(traiteur_pt->mutex_pt);
				lumMsg_pt = klistFirst(traiteur_pt->listeMessages_pt);
				if (NULL!=lumMsg_pt)
				{
					klistRemove(traiteur_pt->listeMessages_pt, lumMsg_pt);
					klogPut(NULL,LOG_INFO, "lumTraiterThread : messageRecu");
					traiteur_pt->lastTick_dw = ktickGetCurrent();
				}
				else
				{
					/* Il n'y a plus de message a traiter */
					traite_b = true;
				}
				kmutexUnlock(traiteur_pt->mutex_pt);
				if (NULL!=lumMsg_pt)
				{
					/* On regarde si le message doit être emis ou non....
					 * Dans le cas ou c'est non, le message est supprime. */
					if ((configIsOn() && (essIsConsigne()))
							|| (lumAdresseGetEquipement(
									lumMsgGetDest(lumMsg_pt)) == LUM_EQUIP_ES))
					{
						int taille;
						uint8 *buffer = lumMsgGetBufferWithPadding(lumMsg_pt,
								&taille, configGetPaddingBefore(),
								configGetPaddingAfter());
						/* Envoyer le message... */
						if (NULL!=traiteur_pt->conn_pt)
						{
							/* Les caracteres ne sont envoyes que lorsque la liaison est ouverte
							 * sinon bus error ! */
							commCnxSend(traiteur_pt->conn_pt, (char *) buffer,
									taille);
						}
						kmmFree(NULL,buffer);
						/* Controler si le message est diffuser en broadcast */
						if (lumMsgIsBroadcast(lumMsg_pt))
						{
							lumMsgDelete(lumMsg_pt);
							attente = 50;
						}
						else
						{
							attente = 300;
							lumTraiteurSetMessCour(traiteur_pt, lumMsg_pt);
						}
						traite_b = true;
					}
					else
					{
						/* Suppression du message car il ne doit pas être traite */
						lumMsgDelete(lumMsg_pt);
					}
				}
			}
		}

		//		mnsusp(0);
		etaSystIncrCompteur(numero);
	}
	return;
}
