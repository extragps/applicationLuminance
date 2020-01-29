#include <stdio.h>
#include <linux/reboot.h>
#include "kcommon.h"
#include "klog.h"
#include "rdtComm.h"
#include "pip_def.h"

#include "ecouteur.h"
#include "event.h"
#include "configuration.h"
#include "sequenceurPriv.h"
#include "sequenceur.h"
#include "sequenceurTraiter.h"
#include "lumAdresse.h"
#include "lumMsg.h"
#include "luminance/lumMsgCreer.h"
#include "lumTraiter.h"
#include "infos/lumInfos.h"
#include "affichage/zone.h"
#include "affichage/page.h"
#include "affichage/caractere.h"
#include "affichage/affichage.h"
#include "affichage/afficheur.h"
#include "thl/thl.h"
#include "es/es.h"
#include "cartes.h"
#include "cartesAfficheur.h"
#include "configurationSram.h"
#include "sramMessage.h"

#include "standard.h"
#include "MQ.h"
#include "mon_def.h"
#include "mon_str.h"
#include "mon_pro.h"
#include "etaSyst.h"
#include "pip_str.h"
#include "pip_var.h"
#include "eriLib.h"
#include "tempLib.h"
#include "lcr_p.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "pmv/tac_ctrl.h"

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* *********************************
 * DEFINITION DES CONSTANTES
 * **********************************	*/

#define SEQUENCEUR_TEMPO_TEST_ANIM 300000	/* Attente de cinq minutes avant nouveau test de l'animation */
#define SEQUENCEUR_TEMPO_TEST_ACTIVATION 60000	/* Attente de une minute avant nouveau test de l'activation */
#define SEQUENCEUR_TEMPO_TEST_ACTIVATION_RETRY 60000	/* Attente de une minute avant nouveau test de l'activation */
#define SEQUENCEUR_RATIO_POLLING 500

/* *********************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * **********************************	*/

static void _sequenceurMessCourIncr(Sequenceur *seq_pt);
static void
		_sequenceurMessCourIncrCaisson(Sequenceur *seq_pt, int32 numCaisson);
static void _sequenceurMessCourSetCaisson(Sequenceur *seq_pt, int32 numCaisson);
static bool _sequenceurMessCourChanged(Sequenceur *seq_pt);
static void _sequenceurProgrammerAffichage(Sequenceur *sequenceur_pt);
static int32 _sequenceurFindEvt(void *evt_p, void *aff_p);
static bool _sequenceurIsTestAnim(Sequenceur *seq_pt);
static void _sequenceurSetTestAnim(Sequenceur *seq_pt, bool val);
static void _sequenceurAjouterEvt(Sequenceur *sequenceur_pt, EventType type,
		int32 tempo);
static bool _sequenceurControlerEvenements(Sequenceur *sequenceur_pt);

/* *********************************
 * VARIABLES LOCALES
 * **********************************	*/

static Sequenceur *_sequenceur_pt = NULL;

static bool sequenceurDefautCom_b = false;

/* *********************************
 * FONCTIONS DU MODULE
 * **********************************	*/

Sequenceur *sequenceurNew() {
	Sequenceur *sequenceur_pt =
			(Sequenceur *) kmmAlloc(NULL,sizeof(Sequenceur));
	if (NULL != sequenceur_pt) {
		int indice;
		sequenceur_pt->mutex_pt = kmutexNew();
		sequenceur_pt->listeEvts_pt = klistNew(0, KTSL_KEEP, eventCompare,
				eventDelete);
		sequenceur_pt->event_pt = keventNew(true);
		sequenceur_pt->ecouteursLum_pt = listeEcouteursNew();
		//		sequenceur_pt->thread_pt=kthreadNew((void*(*)(void*))sequenceurThread,
		//				sequenceur_pt,"Sequenceur",true);
		for (indice = 0; indice < NB_CAISSON; indice++) {
			sequenceur_pt->numMessCour_dw[indice] = 100;
			sequenceur_pt->numMessNext_dw[indice] = 0;
			sequenceur_pt->enCours_b[indice] = false;
			configSetEnCoursCaisson(indice, false);
		}
		sequenceur_pt->etatComm_dw = SEQUENCEUR_ETAT_OK;
		for (indice = 0; indice < THL_NB_MAX; indice++) {
			sequenceur_pt->etatThl_dw[indice] = SEQUENCEUR_ETAT_OK;
		}
		for (indice = 0; indice < AFFICHEUR_NB_MAX; indice++) {
			sequenceur_pt->etatAff_dw[indice] = SEQUENCEUR_ETAT_OK;
		}
		sequenceur_pt->testAnim_b = false;
		sequenceur_pt->reinit_b = true;
		sequenceur_pt->tempoTestActivation_b = false;
	}
	return sequenceur_pt;
}

void sequenceurDelete(Sequenceur *sequenceur_pt) {
	if (NULL != sequenceur_pt) {
		Kmutex *mutex_pt = sequenceur_pt->mutex_pt;
		if (NULL != mutex_pt) {
			kmutexLock(mutex_pt);
		}
		//		if(NULL!=sequenceur_pt->thread_pt)
		//		{
		//			if(!kthreadIsStopped(sequenceur_pt->thread_pt))
		//			{
		//				kthreadStop(sequenceur_pt->thread_pt);
		//			}
		//			kthreadDelete(sequenceur_pt->thread_pt);
		//		}
		keventDelete(sequenceur_pt->event_pt);
		klistDelete(sequenceur_pt->listeEvts_pt);
		listeEcouteursDelete(sequenceur_pt->ecouteursLum_pt);
		if (NULL != mutex_pt) {
			sequenceur_pt->mutex_pt = NULL;
			kmutexUnlock(mutex_pt);
			kmutexDelete(mutex_pt);
		}
		kmmFree(NULL,sequenceur_pt);
	}
}

static bool _sequenceurIsEnCoursCaisson(Sequenceur *seq_pt, int32 numCaisson) {
	return seq_pt->enCours_b[numCaisson];
}
static bool _sequenceurIsEnCours(Sequenceur *seq_pt) {
	bool retour_b = false;
	if (NULL != seq_pt) {
		int indice = 0;
		int nbCaisson = configGetNbCaissons();
		for (indice = 0; (indice < nbCaisson) && (false == retour_b); indice++) {
			retour_b = _sequenceurIsEnCoursCaisson(seq_pt, indice);
		}
	}
	return retour_b;
}

static void _sequenceurSetEnCoursCaisson(Sequenceur *seq_pt, int caisson,
		bool val) {
	if (NULL != seq_pt) {
		seq_pt->enCours_b[caisson] = val;
		configSetEnCoursCaisson(caisson, val);
	}
}
static void _sequenceurSetEnCours(Sequenceur *seq_pt, bool val) {
	if (NULL != seq_pt) {
		int indice = 0;
		int nbCaisson = configGetNbCaissons();
		for (indice = 0; indice < nbCaisson; indice++) {
			_sequenceurSetEnCoursCaisson(seq_pt, indice, val);
		}
	}
}

static bool _sequenceurIsTestAnim(Sequenceur *seq_pt) {
	bool retour_b = false;
	if (NULL != seq_pt) {
		retour_b = seq_pt->testAnim_b;
	}
	return retour_b;
}

static void _sequenceurSetTestAnim(Sequenceur *seq_pt, bool val) {
	if (NULL != seq_pt) {
		seq_pt->testAnim_b = val;
	}
}
//
//void sequenceurStart(Sequenceur *sequenceur_pt)
//{
//	if(NULL!=sequenceur_pt->thread_pt)
//	{
//		kthreadStart(sequenceur_pt->thread_pt);
//	}
//}
//
//
//void sequenceurStop(Sequenceur *sequenceur_pt)
//{
//	if(NULL!=sequenceur_pt->thread_pt)
//	{
//		keventSignal(sequenceur_pt->event_pt);
//		kthreadStop(sequenceur_pt->thread_pt);
//	}
//}
//

void sequenceurAjouterEcouteur(Sequenceur *seq_pt, Ecouteur *ecouteur_pt) {
	if (NULL != ecouteur_pt) {
		listeEcouteursAjouterEcouteur(seq_pt->ecouteursLum_pt, ecouteur_pt);
	}
}

void sequenceurSupprimerEcouteur(Sequenceur *seq_pt, Ecouteur *ecouteur_pt) {
	listeEcouteursSupprimerEcouteur(seq_pt->ecouteursLum_pt, ecouteur_pt);
}

static void sequenceurDiffuser(Sequenceur *seq_pt, LumMsg *mess_pt) {
	listeEcouteursDiffuser(seq_pt->ecouteursLum_pt, mess_pt);
}

static void sequenceurAddDelayedEvent(Sequenceur *seq_pt, Event *evt_pt,
		long delta) {
	eventSetDelta(evt_pt, delta);
	kmutexLock(seq_pt->mutex_pt);
	if (false == klistAdd(seq_pt->listeEvts_pt, evt_pt)) {
		klogPut(NULL, LOG_INFO,
				"sequenceurAddDelayedEvent : Ajout impossible de l'element");
		eventDelete(evt_pt);
	}
	kmutexUnlock(seq_pt->mutex_pt);
}

static void sequenceurLuminosite(ListeEcouteurs *ecouteurs_pt,
		uint8 numCaisson, uint8 luminosite) {
	uint8 valLum[3] = { LUM_MSG_LUMIN_SET, numCaisson, luminosite }; /* A replacer par la valeur forcée ou la valeur calculee. */
	LumMsg *lumMsg_pt = lumMsgNew();
	//	configSetLuminositeCourante(luminosite);
	lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_LEDS, LUM_BROADCAST));
	lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
	lumMsgSetType(lumMsg_pt, 1);
	lumMsgSetTabCar(lumMsg_pt, valLum, 3);
	listeEcouteursDiffuser(ecouteurs_pt, lumMsg_pt);
	lumMsgDelete(lumMsg_pt);
}

static void sequenceurTraiterAfficheur(Sequenceur *seq_pt, Event *evt_pt) {
	Afficheur *aff_pt = (Afficheur *) eventGetData(evt_pt);
	int32 tempo_dw = 60000;
	/* Examen du defaut de com... */
	if (NULL != aff_pt) {
		if (afficheurIsDefautCom(aff_pt)) {
			/* On en fait rien pendant une minute... */
			//			afficheurResetDefautCom(aff_pt);
		} else {
			LumMsg *lumMsg_pt = lumMsgNew();
			lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
			lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_LEDS,
					afficheurGetNumber(aff_pt)));
			tempo_dw = 3000;
			if (false == afficheurIsVers(aff_pt)) {

				uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
				int taille = 1;
				lumMsgSetNbCar(lumMsg_pt, taille);
				buffer[0] = LUM_MSG_VERSION_READ;
				afficheurTrameTryIncr(aff_pt);
			} else if (false == afficheurIsConf(aff_pt)) {
				LumConfig *config_pt = afficheurGetConfig(aff_pt);
				uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
				int taille = 1 + lumConfigGetLgBuffer(config_pt);
				lumMsgSetNbCar(lumMsg_pt, taille);
				buffer[0] = LUM_MSG_CONFIG_WRITE;
				lumConfigWriteBuffer(config_pt, buffer, 1);
				afficheurTrameTryIncr(aff_pt);
			} else if (false == afficheurIsClear(aff_pt)) {
				uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
				int taille = 2;
				lumMsgSetNbCar(lumMsg_pt, taille);
				buffer[0] = LUM_MSG_DISPLAY_BUF_CLR;
				buffer[1] = 0xFF;
				afficheurTrameTryIncr(aff_pt);
			} else if (true == afficheurIsEnCours(aff_pt)) {
				Affichage *affichage_pt = afficheurGetAffichage(aff_pt);
				uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
				if (NULL != affichage_pt) {
					int taille = 1 + affichageGetLgBuffer(affichage_pt);
					lumMsgSetNbCar(lumMsg_pt, taille);
					buffer[0] = LUM_MSG_DISPLAY_BUF_LOAD;
					affichageWriteBuffer(affichage_pt, buffer, 1);
					afficheurTrameTryIncr(aff_pt);
				} else {
					afficheurSetEnCours(aff_pt, false);
					klogPut(
							NULL,
							LOG_INFO,
							"sequenceurTraiterAfficheur: Probleme d'afficheur nul %d",
							afficheurGetNumber(aff_pt));
				}
			} else {
				uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
				int taille = 1;
				lumMsgSetNbCar(lumMsg_pt, taille);
				switch (aff_pt->cptMsgOk_udw % 3) {
				case 0:
					buffer[0] = LUM_MSG_STATUS_READ;
					break;
				case 1:
					buffer[0] = LUM_MSG_STAT_READ;
					break;
				case 2:
					buffer[0] = LUM_MSG_PIXEL_FAULT_READ;
					break;
				}
				tempo_dw = 60000;
				if (0 != configGetPollingAff()) {
					tempo_dw = configGetPollingAff() * SEQUENCEUR_RATIO_POLLING;
				}
			}
			sequenceurDiffuser(seq_pt, lumMsg_pt);
			lumMsgDelete(lumMsg_pt);
		}
	} else {

		klogPut(NULL, LOG_INFO, "sequenceurTraiterAfficheur: pas de donnee!!!");
	}
	sequenceurAddDelayedEvent(seq_pt, evt_pt, tempo_dw);
}

static void sequenceurTraiterThl(Sequenceur *seq_pt, Event *evt_pt) {
	Thl *aff_pt = (Thl *) eventGetData(evt_pt);
	int32 tempo = 60000;
	/* Examen du defaut de com... */
	if (thlIsDefautCom(aff_pt)) {
		/* On en fait rien pendant une minute... */
		/* On en fait rien pendant une minute... */
		thlResetDefautCom(aff_pt);
	} else {

		tempo = 3000;
		if (false == thlIsVers(aff_pt)) {
			LumMsg *lumMsg_pt = lumMsgNew();
			/* Positionner l'adresse locale */
			uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
			int taille = 1;
			lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
			lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_THL, thlGetNumber(
					aff_pt)));

			lumMsgSetNbCar(lumMsg_pt, taille);
			buffer[0] = LUM_MSG_VERSION_READ;
			sequenceurDiffuser(seq_pt, lumMsg_pt);
			lumMsgDelete(lumMsg_pt);
			thlTrameTryIncr(aff_pt);
			/* Faire une demande de version et reprogrammer l'evenement dans une seconde */

		} else if (false == thlIsConf(aff_pt)) {
			LumMsg *lumMsg_pt = lumMsgNew();
			LumThlConfig *config_pt = thlGetConfig(aff_pt);
			/* Positionner l'adresse locale */
			uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
			int taille = 1 + lumThlConfigGetLgBuffer(config_pt);
			lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
			lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_THL, thlGetNumber(
					aff_pt)));

			lumMsgSetNbCar(lumMsg_pt, taille);
			buffer[0] = LUM_MSG_CONFIG_WRITE;
			lumThlConfigWriteBuffer(config_pt, buffer, 1);
			/* diffuser le message... */
			sequenceurDiffuser(seq_pt, lumMsg_pt);
			lumMsgDelete(lumMsg_pt);
			thlTrameTryIncr(aff_pt);
			/*
			 * Faire une demande de version et reprogrammer l'evenement dans une seconde */
		} else {
			LumMsg *lumMsg_pt = lumMsgNew();
			uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
			int taille = 1;
			/* Positionner l'adresse locale */
			lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
			lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_THL, thlGetNumber(
					aff_pt)));
			lumMsgSetNbCar(lumMsg_pt, taille);
			/* Le type de commande dépend du module du nombre de messages... */
			switch (aff_pt->cptMsgOk_udw % 3) {
			case 0:
			default:
				buffer[0] = LUM_MSG_DATA_READ;
				break;

			}
			sequenceurDiffuser(seq_pt, lumMsg_pt);
			lumMsgDelete(lumMsg_pt);
			/* Dans le cas general, on reprogramme une lecture de status... pour l'instant. */
			/* On programme la temporisation a la moitie de la periode de polling. */
			tempo = configGetPollingSonde() * SEQUENCEUR_RATIO_POLLING;
			/* Si tempo est nul, on met dix secondes. */
			tempo = ((tempo != 0) ? tempo : 10000);
		}
	}
	sequenceurAddDelayedEvent(seq_pt, evt_pt, tempo);

}

static void sequenceurTraiterEs(Sequenceur *seq_pt, Event *evt_pt) {
	Es *es_pt = (Es *) eventGetData(evt_pt);
	int32 tempo = 60000;
	/* Examen du defaut de com... */
	if (NULL != es_pt) {
		if (esIsDefautCom(es_pt)) {
			/* On en fait rien pendant une minute... */
			cmd_trc_tr("RESET COM ES com=%d try=%d", es_pt->cptComNok_uw,
					es_pt->cptTrameTry_uw);
			esResetDefautCom(es_pt);
		} else {
			tempo = 3000;
			if (false == esIsConf(es_pt)) {
				LumMsg *lumMsg_pt = lumMsgNew();
				LumEsConfig *config_pt = esGetConfig(es_pt);
				uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
				int taille = 1 + lumEsConfigGetLgBuffer(config_pt);
				/* Positionner l'adresse locale */
				lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
				lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_ES,
						esGetNumber(es_pt)));

				lumMsgSetNbCar(lumMsg_pt, taille);
				buffer[0] = LUM_MSG_CONFIG_WRITE;
				lumEsConfigWriteBuffer(config_pt, buffer, 1);
				/* diffuser le message... */
				sequenceurDiffuser(seq_pt, lumMsg_pt);
				lumMsgDelete(lumMsg_pt);
				es_pt->cptStat_uw = 0;
				esTrameTryIncr(es_pt);
				/*
				 * Faire une demande de version et reprogrammer l'evenement dans une seconde */
			} else if (false == esIsVers(es_pt)) {
				LumMsg *lumMsg_pt = lumMsgNew();
				/* Positionner l'adresse locale */
				uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
				int taille = 1;
				lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
				lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_ES,
						esGetNumber(es_pt)));

				lumMsgSetNbCar(lumMsg_pt, taille);
				buffer[0] = LUM_MSG_VERSION_READ;
				sequenceurDiffuser(seq_pt, lumMsg_pt);
				lumMsgDelete(lumMsg_pt);
				/* Faire une demande de version et reprogrammer l'evenement dans une seconde */
				esTrameTryIncr(es_pt);

			} else {
				LumMsg *lumMsg_pt = lumMsgNew();
				/* Positionner l'adresse locale */
				uint8 *buffer = lumMsgGetTabCar(lumMsg_pt);
				int taille = 1;
				lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
				lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_ES,
						esGetNumber(es_pt)));
				lumMsgSetNbCar(lumMsg_pt, taille);
				/* Le type de commande dépend du module du nombre de messages... */
				switch (es_pt->cptStat_uw % 2) {
				case 1:
					buffer[0] = LUM_MSG_STATUS_READ;
					break;

				default:
					buffer[0] = LUM_MSG_OUTPUT_TOR_READ;
					break;

				}
				es_pt->cptStat_uw++;
				sequenceurDiffuser(seq_pt, lumMsg_pt);
				lumMsgDelete(lumMsg_pt);
				/* Dans le cas general, on reprogramme une lecture de status... pour l'instant. */
				tempo = configGetPollingEs() * SEQUENCEUR_RATIO_POLLING;
				if (0 == tempo) {
					tempo = 5000;
				}
			}
		}
		sequenceurAddDelayedEvent(seq_pt, evt_pt, tempo);
	}
}
static void _sequenceurActiver(Sequenceur *seq_pt, int32 tempo) {
	LumMsg *lumMsg_pt = lumMsgNew();
	printDebug("Activation du message \n");
	lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_LEDS, LUM_BROADCAST));
	lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
	lumMsgSetType(lumMsg_pt, LUM_TYPE_SYNC_ACT);
	lumMsgSetNbCar(lumMsg_pt, 0);
	listeEcouteursDiffuser(seq_pt->ecouteursLum_pt, lumMsg_pt);
	lumMsgDelete(lumMsg_pt);
	/* Ajout d'une temporisation pour l'activation */
	seq_pt->tempoTestActivation_b = true;
	_sequenceurAjouterEvt(seq_pt, EVENT_TEST_ACTIVATION, tempo);
}
static void _sequenceurAnimer(Sequenceur *seq_pt) {
	LumMsg *lumMsg_pt = lumMsgNew();
	lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_LEDS, LUM_BROADCAST));
	lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
	lumMsgSetType(lumMsg_pt, LUM_TYPE_SYNC_ANIM);
	lumMsgSetNbCar(lumMsg_pt, 0);
	listeEcouteursDiffuser(seq_pt->ecouteursLum_pt, lumMsg_pt);
	lumMsgDelete(lumMsg_pt);
}
static void _sequenceurTestPixels(Sequenceur *seq_pt) {
	if (stationIsOk()) {
		uint8 buffer[2];
		int taille = 2;
		LumMsg *lumMsg_pt = lumMsgNew();
		lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_LEDS, LUM_BROADCAST));
		lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
		lumMsgSetType(lumMsg_pt, LUM_TYPE_FCT);
		lumMsgSetNbCar(lumMsg_pt, taille);
		buffer[0] = LUM_MSG_PIXEL_TST_SET;
		buffer[1] = LUM_BROADCAST;
		lumMsgSetTabCar(lumMsg_pt, buffer, taille);
		listeEcouteursDiffuser(seq_pt->ecouteursLum_pt, lumMsg_pt);
		lumMsgDelete(lumMsg_pt);
	}
}
static void _sequenceurChangerChauffage(Sequenceur *seq_pt, bool consigne_b) {
	uint8 valLum[4] = { LUM_MSG_OUTPUT_TOR_WRITE, 0x10, 0, (consigne_b ? 0xFF
			: 0) };
	LumMsg *lumMsg_pt = lumMsgNew();
	lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_ES, configGetAdresseEs()));
	lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
	lumMsgSetType(lumMsg_pt, LUM_TYPE_FCT);
	lumMsgSetTabCar(lumMsg_pt, valLum, 4);
	listeEcouteursDiffuser(seq_pt->ecouteursLum_pt, lumMsg_pt);
	lumMsgDelete(lumMsg_pt);
}

/* Faire ici ce qu'il faut.... */
/* Faire un reset des message de la file */
/* Creer le nouveau message. */
static void _sequenceurDemarrer(Sequenceur *seq_pt, bool consigne_b) {
	uint8 valLum[4] = { LUM_MSG_OUTPUT_TOR_WRITE, 1, 0, consigne_b };
	LumMsg *lumMsg_pt = lumMsgNew();
	essSetConsigne(consigne_b);
	lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_ES, configGetAdresseEs()));
	lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
	lumMsgSetType(lumMsg_pt, LUM_TYPE_FCT);
	lumMsgSetTabCar(lumMsg_pt, valLum, 4);
	listeEcouteursDiffuser(seq_pt->ecouteursLum_pt, lumMsg_pt);
	lumMsgDelete(lumMsg_pt);
}

static void _sequenceurStatus(Sequenceur *seq_pt) {
	uint8 valLum[4] = { LUM_MSG_STATUS_READ, 1, 0 };
	LumMsg *lumMsg_pt = lumMsgNew();
	lumMsgSetDest(lumMsg_pt, lumAdresseNew(LUM_EQUIP_ES, configGetAdresseEs()));
	lumMsgSetSource(lumMsg_pt, lumAdresseNew(LUM_EQUIP_PC, 1));
	lumMsgSetType(lumMsg_pt, LUM_TYPE_FCT);
	lumMsgSetTabCar(lumMsg_pt, valLum, 1);
	listeEcouteursDiffuser(seq_pt->ecouteursLum_pt, lumMsg_pt);
	lumMsgDelete(lumMsg_pt);
}
static void _sequenceurSupprimerEvt(Sequenceur *sequenceur_pt, EventType type) {
	Ktimestamp ts_t;
	Event *evt_pt = NULL;
	Event *evtBis_pt;
	ktsGetTime(&ts_t);
	evt_pt = eventNew(&ts_t, type, NULL, NULL);
	kmutexLock(sequenceur_pt->mutex_pt);
	evtBis_pt = klistFind(sequenceur_pt->listeEvts_pt, _sequenceurFindEvt,
			evt_pt);
	if (NULL != evtBis_pt) {
		if (!klistRemove(sequenceur_pt->listeEvts_pt, evtBis_pt)) {
			klogPut(NULL, LOG_INFO, "_sequenceurSupprimerEvt: cas impossible");
		}
		eventDelete(evtBis_pt);
	}
	eventDelete(evt_pt);
	kmutexUnlock(sequenceur_pt->mutex_pt);
}

static void _sequenceurAjouterEvt(Sequenceur *sequenceur_pt, EventType type,
		int32 tempo) {
	/* Arret sur defaut de communication */
	Ktimestamp ts_t;
	Event *evt_pt = NULL;
	ktsGetTime(&ts_t);
	kAddTime(&ts_t, 0, tempo);
	evt_pt = eventNew(&ts_t, type, NULL, NULL);
	/* TODO : Ajouter ici un evenement pour le sequenceur. */
	sequenceurReprogrammerEvent(sequenceur_pt, evt_pt);
}

static void sequenceurTraiter(Sequenceur *seq_pt, Event *evt_pt) {
	/* Les evenements ne sont traites que si il s'agit d'evenements pour
	 * la carte d'entree sortie ou si le panneau est declare allume. */
	switch (eventGetType(evt_pt)) {
	case EVENT_ALLUMAGE:
		_sequenceurProgrammerAffichage(seq_pt);
		eventDelete(evt_pt);
		break;

	case EVENT_REDEMARRAGE:
		eriSupprimer(E_eriMajeure, ERI_DEF_SABORDAGE);
		stationReset();
		configSetOk(false);
		{
			int indice;
			Ktimestamp ts_t;
			ktsGetTime(&ts_t);
			kAddTime(&ts_t, 5, 0);
			for (indice = 0; indice < configGetNbAfficheurs(); indice++) {
				Event *evtAff_pt = NULL;
				kAddTime(&ts_t, 0, 100);
				evtAff_pt = eventNew(&ts_t, EVENT_AFFICHEUR,
						afficheursChercherParAdresse(configGetAdresseAfficheur(
								indice)), NULL);
				sequenceurReprogrammerEvent(seq_pt, evtAff_pt);
			}
		}
		_sequenceurSupprimerEvt(seq_pt, EVENT_ARRET);
		_sequenceurSupprimerEvt(seq_pt, EVENT_SABORDAGE);
		/* On supprime tout eventuel evenement d'affichage */
		_sequenceurSupprimerEvt(seq_pt, EVENT_AFFICHAGE);
		/* TODO : Remplacer le possionnement a null du message courant */
		//		configSetMessCour(NULL);
		// Positionner la station en mode deleste....
		//		_sequenceurMessCourIncr(seq_pt);
		_sequenceurDemarrer(seq_pt, true);
		eventDelete(evt_pt);
		_sequenceurAjouterEvt(seq_pt, EVENT_STATUS, 100);
		_sequenceurAjouterEvt(seq_pt, EVENT_AFFICHAGE, 3000);
		break;

	case EVENT_DEMARRAGE:
		eriSupprimer(E_eriMajeure, ERI_DEF_SABORDAGE);
		stationReset();
		configSetOk(false);
		{
			int indice;
			Ktimestamp ts_t;
			ktsGetTime(&ts_t);
			kAddTime(&ts_t, 10, 0);
			for (indice = 0; indice < configGetNbAfficheurs(); indice++) {
				Event *evtAff_pt = NULL;
				kAddTime(&ts_t, 0, 100);
				evtAff_pt = eventNew(&ts_t, EVENT_AFFICHEUR,
						afficheursChercherParAdresse(configGetAdresseAfficheur(
								indice)), NULL);
				sequenceurReprogrammerEvent(seq_pt, evtAff_pt);
			}
		}
		_sequenceurSupprimerEvt(seq_pt, EVENT_ARRET);
		/* On supprime tout eventuel evenement d'affichage */
		_sequenceurSupprimerEvt(seq_pt, EVENT_AFFICHAGE);
		/* TODO : Remplacer le possionnement a null du message courant */
		//		configSetMessCour(NULL);
		// Positionner la station en mode deleste....
		_sequenceurMessCourIncr(seq_pt);
		_sequenceurDemarrer(seq_pt, true);
		eventDelete(evt_pt);
		_sequenceurAjouterEvt(seq_pt, EVENT_STATUS, 100);
		_sequenceurAjouterEvt(seq_pt, EVENT_AFFICHAGE, 3000);
		break;
	case EVENT_TEST_PIXEL:
		_sequenceurTestPixels(seq_pt);
		/* Reprogrammation du test pixel... */
		sequenceurAddDelayedEvent(seq_pt, evt_pt, configGetPeriodeTestPixel()
				* 1000);
		break;
	case EVENT_REINIT:
		_sequenceurSupprimerEvt(seq_pt, EVENT_DEMARRAGE);
		_sequenceurSupprimerEvt(seq_pt, EVENT_REDEMARRAGE);

		_sequenceurDemarrer(seq_pt, false);
		eventDelete(evt_pt);
		_sequenceurAjouterEvt(seq_pt, EVENT_REDEMARRAGE, 3000);
		break;
	case EVENT_ARRET:
		/* Faire ici ce qu'il faut.... */
		_sequenceurSupprimerEvt(seq_pt, EVENT_DEMARRAGE);
		_sequenceurDemarrer(seq_pt, false);
		/* TODO : Remplacer le possionnement a null du message courant */
		//		configSetMessCour(NULL);
		eventDelete(evt_pt);
		_sequenceurAjouterEvt(seq_pt, EVENT_STATUS, 100);
		break;
	case EVENT_SABORDAGE:
		/* Faire ici ce qu'il faut.... */
		_sequenceurSupprimerEvt(seq_pt, EVENT_DEMARRAGE);
		_sequenceurSupprimerEvt(seq_pt, EVENT_REDEMARRAGE);
		_sequenceurDemarrer(seq_pt, false);
		/* TODO : Remplacer le possionnement a null du message courant */
		//		configSetMessCour(NULL);
		eventDelete(evt_pt);
		_sequenceurAjouterEvt(seq_pt, EVENT_STATUS, 100);
		if (false == sequenceurDefautCom_b) {
			sequenceurDefautCom_b = true;
			_sequenceurAjouterEvt(seq_pt, EVENT_REDEMARRAGE, 1000);
		}
		break;
	case EVENT_STATUS:
		_sequenceurStatus(seq_pt);
		_sequenceurMessCourIncr(seq_pt);
		eventDelete(evt_pt);
		break;
	case EVENT_AFFICHAGE:
		//		_sequenceurMessCourIncr(seq_pt);
		_sequenceurProgrammerAffichage(seq_pt);
		eventDelete(evt_pt);
		break;
	case EVENT_TEST_ANIM:
		_sequenceurSetTestAnim(seq_pt, false);
		eventDelete(evt_pt);
		break;
	case EVENT_ANIMATION:
		_sequenceurAnimer(seq_pt);
		/* On reprogramme l'evenement */
		if (0 != configGetPeriodeAnimation()) {
			sequenceurAddDelayedEvent(seq_pt, evt_pt,
					configGetPeriodeAnimation() * 1000);
		} else {
			/* Normalement, on ne devrait pas trop passer par ici. */
			eventDelete(evt_pt);
		}
		break;

	case EVENT_AFFICHEUR:
	case EVENT_ES:
	case EVENT_THL:
		if (configIsOn() || (eventGetType(evt_pt) == EVENT_ES)) {
			switch (eventGetType(evt_pt)) {
			case EVENT_AFFICHEUR:
				sequenceurTraiterAfficheur(seq_pt, evt_pt);
				break;
			case EVENT_ES:
				sequenceurTraiterEs(seq_pt, evt_pt);

				break;
			case EVENT_THL:
				sequenceurTraiterThl(seq_pt, evt_pt);
				break;
			default:
				break;
			}
		} else {
			sequenceurAddDelayedEvent(seq_pt, evt_pt, 3000);
		}
		break;
	case EVENT_TEST_ACTIVATION:
		seq_pt->tempoTestActivation_b = false;
		eventDelete(evt_pt);
		break;
	default:
		sequenceurAddDelayedEvent(seq_pt, evt_pt, 3000);
		break;
	}

}

static void _sequenceurMessCourIncrCaisson(Sequenceur *seq_pt, int32 numCaisson) {
	seq_pt->numMessNext_dw[numCaisson]++;
}

static void _sequenceurMessCourSetCaisson(Sequenceur *seq_pt, int32 numCaisson) {
	seq_pt->numMessCour_dw[numCaisson] = seq_pt->numMessNext_dw[numCaisson];
}

/*
 * _sequenceurMessCourIncr
 * =======================
 * Pour forcer le reaffichage de tous les caissons...
 *
 */
static void _sequenceurMessCourIncr(Sequenceur *seq_pt) {
	int32 indice = 0;
	int32 nbCaissons = configGetNbCaissons();
	for (indice = 0; indice < nbCaissons; indice++) {
		_sequenceurMessCourIncrCaisson(seq_pt, indice);
	}
}

static bool _sequenceurMessCourChangedCaisson(Sequenceur *seq_pt,
		int32 numCaisson) {
	bool retour_b = false;
	retour_b = seq_pt->numMessNext_dw[numCaisson]
			!= seq_pt->numMessCour_dw[numCaisson];
	if (sequenceurTraiterChangementMessage(numCaisson)) {
		retour_b = true;
	}
	return retour_b;

}

static bool _sequenceurMessCourChanged(Sequenceur *seq_pt) {
	int32 indice = 0;
	int32 nbCaissons = configGetNbCaissons();
	bool retour_b = false;
	for (indice = 0; (indice < nbCaissons) && (false == retour_b); indice++) {
		retour_b = _sequenceurMessCourChangedCaisson(seq_pt, indice);
	}
	return retour_b;
}

void sequenceurAjouterEvent(Sequenceur *seq_pt, Event *evt_pt) {
	if (NULL != evt_pt) {
		kmutexLock(seq_pt->mutex_pt);
		if (false == klistAdd(seq_pt->listeEvts_pt, evt_pt)) {
			/* Probleme d'enregistrement d'evenement */
			klogPut(NULL, LOG_INFO,
					"sequenceurAjouterEvent : probleme d'ajout d'evenement");
			eventDelete(evt_pt);
		}
		keventSignal(seq_pt->event_pt);
		kmutexUnlock(seq_pt->mutex_pt);
	}
}

static int32 _sequenceurFindEvt(void *evt_p, void *aff_p) {
	int32 retour = 1;
	Afficheur *aff_pt = (Afficheur *) aff_p;
	Event *evt_pt = (Event *) evt_p;
	Event *oth_pt = (Event *) aff_p;
	/* Il faut s'assurer que l'evenement soit celui qui correspond a l'afficheur
	 * passe en parametre. */
	if (NULL != aff_pt) {
		if (eventGetType(evt_pt) == eventGetType(oth_pt)) {
			if (eventGetData(evt_pt) == eventGetData(oth_pt)) {
				retour = 0;
			}
		}
	}
	return retour;
}

static int32 _sequenceurFindEvtEs(void *evt_p, void *type_p) {
	int32 retour = 1;
	EventType evtType = *(EventType *) type_p;
	if (eventGetType((Event *) evt_p) == evtType) {
		retour = 0;
	}
	return retour;
}

void sequenceurSupprimerEvent(Sequenceur *sequenceur_pt, Event *evt_pt) {
	if (NULL != sequenceur_pt) {
		Event *evtBis_pt;
		kmutexLock(sequenceur_pt->mutex_pt);
		evtBis_pt = klistFind(sequenceur_pt->listeEvts_pt, _sequenceurFindEvt,
				evt_pt);
		if (!klistRemove(sequenceur_pt->listeEvts_pt, evtBis_pt)) {
			klogPut(NULL, LOG_INFO,
					"sequenceurReprogrammerEvent: cas impossible");
		}
		eventDelete(evtBis_pt);
		kmutexUnlock(sequenceur_pt->mutex_pt);
	}
	if (NULL != evt_pt) {
		eventDelete(evt_pt);
	}
}

static void _sequenceurReprogrammerEventMx(Sequenceur *sequenceur_pt,
		Event *evt_pt) {
	if (NULL != sequenceur_pt) {
		Event *evtBis_pt;
		evtBis_pt = klistFind(sequenceur_pt->listeEvts_pt, _sequenceurFindEvt,
				evt_pt);
		if (NULL != evtBis_pt) {
			if (!klistRemove(sequenceur_pt->listeEvts_pt, evtBis_pt)) {
				klogPut(NULL, LOG_DEBUG,
						"sequenceurReprogrammerEvent: cas impossible");
			}
			eventDelete(evtBis_pt);
		}

		if (NULL != evt_pt) {
			if (false == klistAdd(sequenceur_pt->listeEvts_pt, evt_pt)) {
				eventDelete(evt_pt);
				klogPut(NULL, LOG_DEBUG,
						"sequenceurReprogrammerEvent: ajout impossible");
			} else {
				klogPut(NULL, LOG_DEBUG, "sequenceurReprogrammerEvent: OK");
			}
		}
	} else {
		if (NULL != evt_pt) {
			eventDelete(evt_pt);
		}
	}
}

void sequenceurReprogrammerEvent(Sequenceur *sequenceur_pt, Event *evt_pt) {
	if (NULL != sequenceur_pt) {
		Event *evtBis_pt;
		kmutexLock(sequenceur_pt->mutex_pt);
		evtBis_pt = klistFind(sequenceur_pt->listeEvts_pt, _sequenceurFindEvt,
				evt_pt);
		if (NULL != evtBis_pt) {
			if (!klistRemove(sequenceur_pt->listeEvts_pt, evtBis_pt)) {
				klogPut(NULL, LOG_INFO,
						"sequenceurReprogrammerEvent: cas impossible");
			}
			eventDelete(evtBis_pt);
		}

		if (NULL != evt_pt) {
			if (false == klistAdd(sequenceur_pt->listeEvts_pt, evt_pt)) {
				eventDelete(evt_pt);
				klogPut(NULL, LOG_INFO,
						"sequenceurReprogrammerEvent: ajout impossible");
			} else {
				klogPut(NULL, LOG_INFO, "sequenceurReprogrammerEvent: OK");
			}
		}

		keventSignal(sequenceur_pt->event_pt);
		kmutexUnlock(sequenceur_pt->mutex_pt);

	} else {
		if (NULL != evt_pt) {
			eventDelete(evt_pt);
		}
	}
}

/* sequenceurProgrammerAffichage
 * =============================
 * Parcours de la liste des afficheurs pour reprogrammer immediatement
 * des evenements d'affichage.
 * Attention, il va falloir s'attacher a ce qu'il n'y ait pas
 * une nouvelle demande d'affichage avant que la précédente soient finie.
 * */

static void _sequenceurProgrammerAffichageCaisson(Sequenceur *sequenceur_pt,
		int numCaisson) {
	int indice = 0;

	int32 nbCarCaisson = configGetNbAfficheursCaisson(numCaisson);
	int32 premCarCaisson = configGetPremAfficheurCaisson(numCaisson);

	kmutexLock(sequenceur_pt->mutex_pt);
	/* Pour tous les afficheurs, reprogrammer l'evenement courant en un
	 * evenement de chargement de buffer. */
	for (indice = 0; indice < nbCarCaisson; indice++) {
		int32 numAfficheur_dw = premCarCaisson + indice;
		Afficheur *aff_pt = afficheursChercherParAdresse(
				configGetAdresseAfficheur(numAfficheur_dw));
		Event *evt_pt = NULL;
		Ktimestamp ts_t;
		Event *evtAff_pt = eventNew(&ts_t, EVENT_AFFICHEUR, aff_pt, NULL);
		/* Recherche de l'evenement relatif au caisson dans la liste des evenements */
		/* Reprogrammation immediate du nouvel evenement. */
		evt_pt = klistFind(sequenceur_pt->listeEvts_pt, _sequenceurFindEvt,
				evtAff_pt);
		if (NULL != evt_pt) {
			if (klistRemove(sequenceur_pt->listeEvts_pt, evt_pt)) {
				/* Reprogrammer immediatement le nouvel evenement */
				eventSetDelta(evt_pt, 100);
				if (false == klistAdd(sequenceur_pt->listeEvts_pt, evt_pt)) {
					klogPut(NULL, LOG_INFO,
							"_sequenceurProgrammerAffichageCaisson: ajout impossible");
					eventDelete(evt_pt);
				}
			} else {
				klogPut(NULL, LOG_INFO,
						"sequenceurProgrammerAffichage: cas impossible");
			}
		}
		eventDelete(evtAff_pt);
	}
	kmutexUnlock(sequenceur_pt->mutex_pt);
}

static void _sequenceurProgrammerAffichage(Sequenceur *sequenceur_pt) {
	int32 nbCaissons_dw = configGetNbCaissons();
	int indice;
	for (indice = 0; indice < nbCaissons_dw; indice++) {
		activation_caisson(indice);
		//		_sequenceurProgrammerAffichageCaisson(sequenceur_pt, indice);
	}

}

Sequenceur *sequenceurGet(void) {
	if (NULL == _sequenceur_pt) {
		_sequenceur_pt = sequenceurNew();
	}
	return _sequenceur_pt;
}

/* sequenceurCtrlTemp
 * ==================
 * Controle de la temperature excessive du caisson...
 * Retour :
 * - La fonction retourne 1 en cas de changement d'etat.
 */

static int _sequenceurCtrlTemp(int depassement) {
	int retour = 0;
	if (pip_cf_tst_sc.temp.actif) {
		if (depassement) {
			if (0 == tempGetAlerte()) {

				tempSetAlerte(1);
				pipSetEtat(pipGetEtat() | PIP_TEMPERATURE);
				eriAjouter(E_eriMajeure, ERI_DEF_TEMPERATURE);
				retour = 1;
			}
			tempSetTempo(pip_cf_tst_sc.temp.tempo);
		} else {
			/* TODO : Regarder si une alerte est en cours.
			 * vérifier par rapport a la temporisation et
			 * faire éventuellement tomber l'alerte. */
			if (tempGetAlerte() != 0) {
				if (0 == tempDecTempo()) {
					tempSetAlerte(0);
					pipSetEtat(pipGetEtat() & (~PIP_TEMPERATURE));
					eriSupprimer(E_eriMajeure, ERI_DEF_TEMPERATURE);
					retour = 1;
				}
			}
		}
	}
	return retour;
}

static bool _sequenceur_status_temps_reel(Sequenceur *seq_pt) {
	UINT8 stTrTemp_uc;
	int indice;
	UINT8 defaut;
	bool retour_b = false;
	/* il n'y a pas de defaut, on conserve l'indication d'init et de terminal connecte. */
	stTrTemp_uc = vct_ST_TR & 0x56;

	/* on teste la presence du terminal */
	/* Tester la presence d'un terminal. Attention, la methode a change,
	 * on teste la presence d'un terminal par la presence de commandes.
	 * Cela n'est pas la bonne maniere e priori. Il faudrait plutot
	 * tester l'etat d'un bit de rebouclage. */
	/* spcTrmTester (); */

	defaut = 0;

	/* TODO : Traiter ici le defaut de communication. */
	if (stationIsDefautCom()) {
		if (seq_pt->etatComm_dw == SEQUENCEUR_ETAT_OK) {
			eriAjouter(E_eriMajeure, ERI_DEF_COMM_LON);
			seq_pt->etatComm_dw = SEQUENCEUR_DEFAUT_MAJEUR;
		}
		defaut |= ST_TR_DEF_MAJ;
	} else {
		if (seq_pt->etatComm_dw == SEQUENCEUR_DEFAUT_MAJEUR) {
			eriSupprimer(E_eriMajeure, ERI_DEF_COMM_LON);
			seq_pt->etatComm_dw = SEQUENCEUR_ETAT_OK;
		}
	}

	if (essDefautAlim()) {
		defaut |= ST_TR_DEF_MIN;
		eriAjouter(E_eriMineure, ERI_DEF_ALIM_AFF);
	} else {
		eriSupprimer(E_eriMineure, ERI_DEF_ALIM_AFF);
	}

	if (!stationIsOk()) {
		defaut |= ST_TR_DEF_MAJ;
	}

	if (0 != tempGetAlerte()) {
		defaut |= ST_TR_DEF_MAJ;
	}

	{

		bool defaut_b = false;
		indice = 0;
		if (!thlsIsOk()) {
			if (seq_pt->etatThl_dw[indice] == SEQUENCEUR_ETAT_OK) {
				eriAjouter(E_eriMineure, ERI_DEF_CELL_LUMINOSITE);
				seq_pt->etatThl_dw[indice] = SEQUENCEUR_DEFAUT_MINEUR;
				defaut_b = true;
			}
			defaut |= ST_TR_DEF_MIN;
		} else {

			if (seq_pt->etatThl_dw[indice] == SEQUENCEUR_DEFAUT_MINEUR) {
				eriSupprimer(E_eriMineure, ERI_DEF_CELL_LUMINOSITE);
				seq_pt->etatThl_dw[indice] = SEQUENCEUR_ETAT_OK;
			}
		}
	}

	/* TODO : Traiter ici le défaut mineur (defaut pixel), sur l'une des cartes afficheur. */
	/* Faire une boucle de traitement sur les afficheurs pour regarder si l'on a des
	 * defauts pixel. */
	/* TODO : Traiter ici le défaut mineur (defaut pixel), sur l'une des cartes afficheur. */
	/* Faire une boucle de traitement sur les afficheurs pour regarder si l'on a des
	 * defauts pixel. */
	for (indice = 0; indice < configGetNbAfficheurs(); indice++) {
		uint8 nbPixHs_uc;
		int numCaisson = configGetCaissonAfficheur(indice);
		int numModule = pip_cf_caisson[numCaisson].id_module;
		int idCaisson = pip_cf_caisson[numCaisson].caisson;
		nbPixHs_uc = afficheursGetDefautPixelNumero(indice);
		switch (nbPixHs_uc) {
		case STATION_DEF_PIXEL_NONE:
			if (seq_pt->etatAff_dw[indice] != SEQUENCEUR_ETAT_OK) {
				eriSupprimerModuleVal(E_eriMajeure, ERI_DEF_PIXEL, numModule,
						idCaisson, indice);
				eriSupprimerModuleVal(E_eriMineure, ERI_DEF_PIXEL, numModule,
						idCaisson, indice);
				seq_pt->etatAff_dw[indice] = SEQUENCEUR_ETAT_OK;
			}
			break;
		case STATION_DEF_PIXEL_MINEUR:
			if (seq_pt->etatAff_dw[indice] != SEQUENCEUR_DEFAUT_MINEUR) {
				if (seq_pt->etatAff_dw[indice] == SEQUENCEUR_DEFAUT_MAJEUR) {
					eriSupprimerModuleVal(E_eriMajeure, ERI_DEF_PIXEL,
							numModule, idCaisson, indice);
				}
				eriAjouterModule(E_eriMineure, ERI_DEF_PIXEL, numModule,
						idCaisson, indice);
				seq_pt->etatAff_dw[indice] = SEQUENCEUR_DEFAUT_MINEUR;
			}
			defaut |= ST_TR_DEF_MIN;
			break;
		case STATION_DEF_PIXEL_MAJEUR:
			if (seq_pt->etatAff_dw[indice] != SEQUENCEUR_DEFAUT_MAJEUR) {
				T_pip_sv_act_caisson action_t;

				sramMessageLireCaisson(indice, &action_t);
				if (action_t.nbAlternance) {
					action_t.act = PIP_ACT_CAR;
					action_t.nbAlternance = 0;
					action_t.extinction = TRUE;
					retour_b = true;
					sramMessageEcrireCaisson(indice, &action_t);
				}
				/* Recuperation du numero du numero de caisson et si
				 * il si le caisson n'est pas eteint, on fait une
				 * demande d'extinction. */
				if (seq_pt->etatAff_dw[indice] == SEQUENCEUR_DEFAUT_MINEUR) {
					eriSupprimerModuleVal(E_eriMineure, ERI_DEF_PIXEL,
							numModule, idCaisson, indice);
				}
				eriAjouterModule(E_eriMajeure, ERI_DEF_PIXEL, numModule,
						idCaisson, indice);
				seq_pt->etatAff_dw[indice] = SEQUENCEUR_DEFAUT_MAJEUR;

			}
			defaut |= ST_TR_DEF_MAJ;
			break;
		}
	}
	/* Si il n'y a pas de defaut majeur, on regarde si il n'y a pas
	 * un sabordage en cours.... */
	if (!(defaut & ST_TR_DEF_MAJ)) {
		if (eriChercher(E_eriMajeure, ERI_DEF_SABORDAGE)) {
			defaut |= ST_TR_DEF_MAJ;
		}
	}

	vct_ST_TR = stTrTemp_uc | defaut;
	return retour_b;

}

void sequenceurSetReinit(bool val) {
	Sequenceur *seq_pt = sequenceurGet();
	if (NULL != seq_pt) {
		seq_pt->reinit_b = val;
	}
}

bool sequenceurIsReinit(void) {
	Sequenceur *seq_pt = sequenceurGet();
	return seq_pt->reinit_b;
}

void sequenceurSignal(void) {
	Sequenceur *seq_pt = NULL;
	seq_pt = sequenceurGet();
	if (NULL != seq_pt) {
		if (NULL != seq_pt->event_pt) {
			keventSignal(seq_pt->event_pt);
		}
	}
}

static bool _sequenceurControlerEvenements(Sequenceur *sequenceur_pt) {
	Ktimestamp ts_t;
	int indice;
	Event *evt_pt;
	Event *evtBis_pt;
	bool retour_b = false;
	/* Initialisation des evenements .... */
	/* Ajout d'un evenement par afficheur pour l'instant. */

	ktsGetTime(&ts_t);
	kAddTime(&ts_t, 10, 0);

	/* Controle de la presence de l'evenement ES */
	evt_pt = eventNew(&ts_t, EVENT_ES, essChercherParAdresse(
			configGetAdresseEs()), NULL);
	/* Rechercher l'evenement dans la liste des evenements.... */
	evtBis_pt = klistFind(sequenceur_pt->listeEvts_pt, _sequenceurFindEvt,
			evt_pt);
	if (NULL == evtBis_pt) {
		if (false == klistAdd(sequenceur_pt->listeEvts_pt, evt_pt)) {
			eventDelete(evt_pt);
		}
		retour_b = true;
	} else {
		eventDelete(evt_pt);
	}

	/* Controle de la presence de l'evenement THL */
	for (indice = 0; indice < configGetNbThls(); indice++) {
		kAddTime(&ts_t, 0, 100);
		evt_pt = eventNew(&ts_t, EVENT_THL, thlsChercherParAdresse(
				configGetAdresseThl(indice)), NULL);
		evtBis_pt = klistFind(sequenceur_pt->listeEvts_pt, _sequenceurFindEvt,
				evt_pt);
		if (NULL == evtBis_pt) {
			if (false == klistAdd(sequenceur_pt->listeEvts_pt, evt_pt)) {
				eventDelete(evt_pt);
			}
			retour_b = true;
		} else {
			eventDelete(evt_pt);
		}
	}

	/* Ajout des evenements afficheurs */
	for (indice = 0; indice < configGetNbAfficheurs(); indice++) {
		kAddTime(&ts_t, 0, 100);
		evt_pt = eventNew(&ts_t, EVENT_AFFICHEUR, afficheursChercherParAdresse(
				configGetAdresseAfficheur(indice)), NULL);
		evtBis_pt = klistFind(sequenceur_pt->listeEvts_pt, _sequenceurFindEvt,
				evt_pt);
		if (NULL == evtBis_pt) {
			if (false == klistAdd(sequenceur_pt->listeEvts_pt, evt_pt)) {
				eventDelete(evt_pt);
			}
			retour_b = true;
		} else {
			eventDelete(evt_pt);
		}
	}
	return retour_b;
}

static void _sequenceurTracerAfficheur(int numAff, Afficheur *aff_pt) {
	char detail[256] = "";
	if (0 != afficheurGetEtat(aff_pt)) {
		int etat = afficheurGetEtat(aff_pt);
		snprintf(detail, 256, "(%s,%s,%s,%s,%s,%s) ", (((AFFICHEUR_DEFAUT_PIXEL
				& etat) != 0) ? "PIX" : ""), (((AFFICHEUR_DEFAUT_CARTE & etat)
				!= 0) ? "SIN" : ""),
				(((AFFICHEUR_DEFAUT_TSD & etat) != 0) ? "TSD" : ""),
				(((AFFICHEUR_DEFAUT_TEST & etat) != 0) ? "TST" : ""),
				(((AFFICHEUR_DEFAUT_LOAD & etat) != 0) ? "LOA" : ""),
				(((AFFICHEUR_DEFAUT_ANIM & etat) != 0) ? "ANI" : ""));
	}
	/* Composer le buffer de reponse.... */

	cmd_trc_tr("DEF A=%02d a:%02ld e:%02X %scom:%s(%d/%d:%d) pHs:%d", numAff,
			configGetAdresseAfficheur(numAff), (unsigned int) afficheurGetEtat(
					aff_pt), detail, (afficheurIsDefautCom(aff_pt) ? "NK"
					: (afficheurIsConf(aff_pt) ? "OK" : "??")),
			afficheurGetNbDefCom(aff_pt), afficheurGetNbDefComTot(aff_pt),
			aff_pt->cptTrameTry_uw, (unsigned int) afficheurGetDefautPixel(
					aff_pt));
}

static void _sequenceurTracerEs(void) {
	char detail[256] = "";
	Es *es_pt = essChercherParIndex(0);
	snprintf(detail, 256, "DEF E=%02d a:%02ld e:%02X %s%scom:%s(%d/%d)%d", 0,
			configGetAdresseEs(), esGetEtat(es_pt),
			(essDefautAlim() ? "Pb alim "
					: ((esGetDefautAlim(es_pt) != 0) ? "Alim HS " : "")),
			(esIsSabordage() ? " Sabordage " : ""),
			(esIsDefautCom(es_pt) ? "NK" : ((esIsConf(es_pt) ? "OK" : "??"))),
			esGetNbDefCom(es_pt), esGetNbDefComTot(es_pt),
			es_pt->cptTrameTry_uw);
	cmd_trc_tr(detail);

}

void sequenceur_init(void) {
	Sequenceur *seq_pt = NULL;
	Ecouteur *ecouteur_pt = ecouteurNew(lumTraiterAjouterLumMessage,
			lumTraiteurGet());

	stationInit();
	/* Initialisation de la structure sequenceur.... */
	seq_pt = sequenceurGet();
	/* Enregistrement du sequenceur aupres des ecouteurs... */
	sequenceurAjouterEcouteur(seq_pt, ecouteur_pt);
	/* Enregistrement du sequenceur aupres du traiteur;
	 */
	lumTraiteurSetSequenceur(lumTraiteurGet(), seq_pt);

}

void sequenceur_term(void) {
	sequenceurDelete(sequenceurGet());
}

void sequenceur_main(int numero) {
	//	Kthread * thread_pt = sequenceur_pt->thread_pt;
	Sequenceur *sequenceur_pt = sequenceurGet();
	int attente = 1000;
	uint32 lastTick_udw = 0;
	uint32 currentTick_udw = 0;
	int32 tempLast_dw = 0;
	uint32 lastTack_udw = 0;
	Ktimestamp tsTemp_t;
	Ktimestamp tsStart_t;
	Ktimestamp tsPrec_t; /* Pour pister les difference de date trop importante. */
	bool init_b = false;

	ktsGetTime(&tsTemp_t);
	tsPrec_t = tsTemp_t;
	tsStart_t = tsTemp_t;
	;
	/* Initialisation des evenements .... */
	/* Ajout d'un evenement par afficheur pour l'instant. */

	{
		Ktimestamp ts_t;
		int indice;
		Event *evtEs_pt;
		ktsGetTime(&ts_t);
		kAddTime(&ts_t, 10, 0);
		kmutexLock(sequenceur_pt->mutex_pt);

		/* Ajout des evenements ES */
		evtEs_pt = eventNew(&ts_t, EVENT_ES, essChercherParAdresse(
				configGetAdresseEs()), NULL);
		if (false == klistAdd(sequenceur_pt->listeEvts_pt, evtEs_pt)) {
			eventDelete(evtEs_pt);
		}
		/* TODO: Il s'agissait a priori du lestage du PMV. Il faut voir comment on demarre.
		 * par defaut on va laisser le demarrage*/
		//		if (configGetDemarrage())
		kAddTime(&ts_t, 5, 0);
		{
			Event *evtDem_pt = NULL;
			kAddTime(&ts_t, 0, 100);
			evtDem_pt = eventNew(&ts_t, EVENT_DEMARRAGE, NULL, NULL);
			if (false == klistAdd(sequenceur_pt->listeEvts_pt, evtDem_pt)) {
				eventDelete(evtDem_pt);
			}
		}
		kAddTime(&ts_t, 5, 0);
		/* Ajout des evenements THL */
		for (indice = 0; indice < configGetNbThls(); indice++) {
			Event *evtThl_pt = eventNew(&ts_t, EVENT_THL,
					thlsChercherParAdresse(configGetAdresseThl(indice)), NULL);
			if (false == klistAdd(sequenceur_pt->listeEvts_pt, evtThl_pt)) {
				eventDelete(evtThl_pt);
			}

		}

		/* Ajout des evenements afficheurs */
		for (indice = 0; indice < configGetNbAfficheurs(); indice++) {
			Event *evtAff_pt = NULL;
			kAddTime(&ts_t, 0, 100);
			evtAff_pt = eventNew(&ts_t, EVENT_AFFICHEUR,
					afficheursChercherParAdresse(configGetAdresseAfficheur(
							indice)), NULL);
			if (false == klistAdd(sequenceur_pt->listeEvts_pt, evtAff_pt)) {
				eventDelete(evtAff_pt);
			}
		}
		//		for(evtEs_pt=klistFirst(sequenceur_pt->listeEvts_pt);evtEs_pt!=null;evtEs_pt=klistNext(sequenceur_pt->listeEvts_pt))
		//		{
		//			eventPrint(evtEs_pt);
		//		}
		kmutexUnlock(sequenceur_pt->mutex_pt);
	}

	//	mnsuspIniPrendre();

	//	while(!kthreadIsStopped(thread_pt))
	while (1) {
		Ktimestamp ts_t;
		bool reactivation_b = false;
		bool notFin = true;
		keventTimedWait(sequenceur_pt->event_pt, attente);
		/* On repositionne l'attente par défaut à une seconde */

		attente = 1000;
		/* Depilement des réponses éventuelles. */
		//		keventTimedWait(sequenceur_pt->event_pt, attente);
		/* On repositionne l'attente par défaut à une seconde */

		/* Depilement des réponses éventuelles. */
		while (notFin) {
			Event *evt_pt = NULL;
			double deltaDate;
			float64 delta;
			ktsGetTime(&ts_t);
			/* ------------------------------------
			 * TRAITEMNET CHANGEMENT DE DATE
			 * ------------------------------------ */
			deltaDate = kGetDeltaTimestamp(&tsPrec_t, &ts_t);
			if (fabs(deltaDate) > 30.) {
				long deltaDateSec = (long) deltaDate;
				printDebug(
						"_sequenceur_main: Cas d'un changement de date de plus de 30 secondes %d\n",
						deltaDateSec);
				/* En cas de changement d'heure, il faut reprogrammer tous les evenements à la bonne heure. */
				/* Une autre solution serait d'utiliser des ticks... */
				kmutexLock(sequenceur_pt->mutex_pt);
				for (evt_pt = klistFirst(sequenceur_pt->listeEvts_pt); evt_pt
						!= NULL; evt_pt
						= klistNext(sequenceur_pt->listeEvts_pt)) {
					eventAddDeltaSec(evt_pt, deltaDateSec);
				}
				kmutexUnlock(sequenceur_pt->mutex_pt);

			}
			/* Mise a jour de l'heure precedente */
			tsPrec_t = ts_t;
			/* Si il y a un evenement. */
			kmutexLock(sequenceur_pt->mutex_pt);

			delta = 0.0;
			/* ******************************************************************* */
			/* XG : AJOUT DU CONTROLE DES EVENEMENTS 							   */
			/* Controle d'un eventuel probleme sur l'un des evenements de la file. */
			if (eventIsPourri()) {
				for (evt_pt = klistFirst(sequenceur_pt->listeEvts_pt); evt_pt
						!= NULL; evt_pt
						= klistNext(sequenceur_pt->listeEvts_pt)) {
					/* Controler l'evenement. Si il n'est pas bon, on le supprime.
					 * Il s'agit de controler son type et eventuellement son timestamp */
					if (!eventIsOk(evt_pt)) {
						/* Supprimer l'evenement de la file. */
						if (klistRemove(sequenceur_pt->listeEvts_pt, evt_pt)) {
							eventLog(evt_pt, LOG_ERROR, "Suppression");
							eventDelete(evt_pt);
						} else {
							klogPut(NULL, LOG_ERROR,
									"Erreur suppression evenement");
						}
					}

				}
				/* Verifier ici la presence de tous les evenements necessaires. */
				/* Controler maintenant la liste des evenements et creer eventuellement
				 * celui qui manque.
				 */
				if (_sequenceurControlerEvenements(sequenceur_pt)) {
					klogPut(NULL, LOG_ERROR, "Il manquait un evenement");
				}

				eventSetPourri(false);
			} /* endif(eventIsPourri(); */
			/* ******************************************************************* */
			/* Recherche de l'evenement ES qui doit etre traite en priorite .... */
			{
				EventType evtType_e = EVENT_ES;
				evt_pt = klistFind(sequenceur_pt->listeEvts_pt,
						_sequenceurFindEvtEs, &evtType_e);
				if (NULL == evt_pt) {
					if (_sequenceurControlerEvenements(sequenceur_pt)) {
						klogPut(NULL, LOG_ERROR, "Il manquait l'evenement ES");
					}
				} else {
					/* On regarde si l'evenement doit etre traite. */
					delta = kGetDeltaTimestamp(eventGetDate(evt_pt), &ts_t);
					/* Si l'evenement ne doit pas etre traite, on controle quand meme
					 * que la tempo ne soit pas superieure au temps de polling.
					 */
					if (delta <= 0.) {
						int32 pollingEs_dw = configGetPollingEs();
						if (0 != pollingEs_dw) {
							if (essIsDefautCom() || (false == essIsOk())) {
								pollingEs_dw = 60;
							}
							if ((delta < -(float64) pollingEs_dw) && (init_b)) {
								Ktimestamp tsLoc_t;
								Event *evtLoc_pt;
								klogPut(
										NULL,
										LOG_ERROR,
										"Probleme, le prochain evenement est vraiment loin,"
											"on force la reprogrammation %lf sec",
										-delta);
								tsLoc_t = ts_t;
								kAddTime(&tsLoc_t, 0, pollingEs_dw * 500);
								/* Creation du nouvel evenement, a l'heure qui va bien. */
								evtLoc_pt = eventNew(&tsLoc_t, EVENT_ES,
										essChercherParAdresse(
												configGetAdresseEs()), NULL);
								/* Reprogrammation de l'evenement */
								_sequenceurReprogrammerEventMx(sequenceur_pt,
										evtLoc_pt);
							} /*endif(delta< ) */
						} /* endif(0!= pollingEs_dw) */
					}
				}
			}
			/* Fin du traitement de l'evenement ES */
			/* Il n'y a pas traitement de l'evenement ES, on peut traiter
			 * */
			evt_pt = klistFirst(sequenceur_pt->listeEvts_pt);
			if (NULL != evt_pt) {
				delta = kGetDeltaTimestamp(eventGetDate(evt_pt), &ts_t);
			}

			kmutexUnlock(sequenceur_pt->mutex_pt);

			if (NULL != evt_pt) {
				/* Test de la date de l'evenement.... */
				/* Si l'evenement doit etre traite, on le fait. */
				delta = kGetDeltaTimestamp(eventGetDate(evt_pt), &ts_t);
				if (delta >= 0.0) {
					init_b = true;
					kmutexLock(sequenceur_pt->mutex_pt);
					if (klistRemove(sequenceur_pt->listeEvts_pt, evt_pt)
							== false) {
						klogPut(NULL, LOG_INFO,
								"extraction impossible de l'evenement");
					}
					kmutexUnlock(sequenceur_pt->mutex_pt);
					//					eventPrint(evt_pt);
					sequenceurTraiter(sequenceur_pt, evt_pt);
				} else {
					int32 pollingEs_dw = configGetPollingEs();
					float64 max_lf = -70.;
					if (0 != pollingEs_dw) {
						max_lf = -(float64) pollingEs_dw;
					}

					if ((delta < max_lf) && (init_b)) {
						kmutexLock(sequenceur_pt->mutex_pt);
						if (klistRemove(sequenceur_pt->listeEvts_pt, evt_pt)
								== false) {
							klogPut(NULL, LOG_ERROR,
									"extraction impossible de l'evenement");
						}
						/* Controler maintenant la liste des evenements et creer eventuellement
						 * celui qui manque.
						 */
						if (_sequenceurControlerEvenements(sequenceur_pt)) {
							klogPut(NULL, LOG_ERROR, "Il manquait un evenement");
						}

						kmutexUnlock(sequenceur_pt->mutex_pt);
						eventLog(evt_pt, LOG_ERROR, "PB DELTA");

					}
					/* Calculer la prochaine attente en fonction du delta trouve...  */
					/* Attente minimale de 50 millis et maximum de une seconde */
					attente = MAX(MIN(attente,(int)(-delta*1000.)),50);
					notFin = false;
				}
			} else {
				/* Il n'y a plus d'evenement, on sort de la boucle... */
				notFin = false;
			}
		}
		/* Stocker la date de coupure. */
		configSetDateCour(&ts_t);

		currentTick_udw = ktickGetCurrent();
		/* Mettre ici le test de la temperature pour allumage du chauffage */
		/* Si -1000, pas d'info... */
		if (-1000 != thlsGetTemperature()) {
			int depassement = 0;
			int32 tempCour_dw = thlsGetTemperature();
			if (tempCour_dw != tempLast_dw) {
				_sequenceurChangerChauffage(sequenceur_pt, tempCour_dw
						< (configGetTemperatureChauffage() * 10));
				tempLast_dw = tempCour_dw;
				tempSetValeur(tempCour_dw);
			}
			/* Attention, il s'agit d'un traitement appele toutes les secondes. */
			if (fabs(kGetDeltaTimestamp(&tsTemp_t, &ts_t)) >= 1.0) {
				if (tempCour_dw > (pip_cf_tst_sc.temp.seuil * 10)) {
					depassement = 1;
				}
				if (_sequenceurCtrlTemp(depassement)) {
					reactivation_b = true;
				} else {
					lastTack_udw = currentTick_udw;
				}
				tsTemp_t = ts_t;
			}
		}

		/* TRAITEMENT DES DEMANDES D'ACTIVATION */
		/* - Verifier qu'il y a un chargement en cours, */
		if (stationIsOk()) {
			uint32 deltaTick_udw;
			sequenceurDefautCom_b = false;
			if (_sequenceurIsEnCours(sequenceur_pt)) {
				if (afficheursControlerChargement()) {
					/* - Controler que tous les afficheurs sont charges, */
					/* - Creer et envoyer la commande d'activation */
					_sequenceurActiver(sequenceur_pt,
							SEQUENCEUR_TEMPO_TEST_ACTIVATION);
					/* Reprogrammation de l'annimation. */
					if (0 != configGetPeriodeAnimation()) {
						Ktimestamp tsLoc_t;
						Event *evtAnim_pt = NULL;
						ktsGetTime(&tsLoc_t);
						kAddTime(&tsLoc_t, configGetPeriodeAnimation(), 0);
						evtAnim_pt = eventNew(&tsLoc_t, EVENT_ANIMATION, NULL,
								NULL);
						sequenceurReprogrammerEvent(sequenceur_pt, evtAnim_pt);
					}

					/* TRAITEMENT DE LA DUREE DE VALIDITE DU MESSAGE.... */
					{
						/* TRAITEMENT DE LA LUMINOSITE */
						{
							int32 nbCaissons_dw = configGetNbCaissons();
							int32 indice = 0;
							for (indice = 0; indice < nbCaissons_dw; indice++) {
								if (_sequenceurIsEnCoursCaisson(sequenceur_pt,
										indice)) {
									bool auto_b = true;
									/* Recupere le niveau de luminosite souhaité */
									/* Regarder si eventuellement il s'agit du mode
									 * automatique.
									 */
									auto_b
											= sequenceurGetModeLuminosite(
													indice);
									configLumSetAutoCaisson(indice, auto_b);
									if (false == auto_b) {
										int32 valeur =
												sequenceurGetValeurLuminosite(
														indice);
										configLumSetValCaisson(indice, valeur);
										printDebug(
												"Consigne lumino pour caisson %d=%d \n",
												indice, valeur);
									} else {
										/* Calculer la luminosite courante pour le caisson */
										printDebug(
												"Luminosite auto pour caisson %d \n",
												indice);
									}
								}
							}

						}
					}
					/* Traitement de la luminosite des caissons charges... */
					/* Fin du traitement. */
					_sequenceurSetEnCours(sequenceur_pt, false);
				}
			} else {
				/* Il faut controler ici que tous les afficheurs soient correctement actives. Sinon, on relance une activation. */
				/* En meme temps, il ne faut pas en envoyer trop souvent.... */
				if (false == sequenceur_pt->tempoTestActivation_b) {
					if (afficheursIsDefautActivation()) {
						_sequenceurActiver(sequenceur_pt,
								SEQUENCEUR_TEMPO_TEST_ACTIVATION_RETRY);
					}
				}

			}
			/* Test de l'animation.... */
			if (afficheursIsDefautAnim() && (!_sequenceurIsTestAnim(
					sequenceur_pt))) {
				/* Reprogrammer ici l'evenement d'animation et programmer un evenement de test
				 * d'animation.... */
				_sequenceurSetTestAnim(sequenceur_pt, true);

				_sequenceurMessCourIncr(sequenceur_pt);
				/*
				 * Pour l'instant on force un reaffichage du message courant....
				 * On pourrait voir a terme si une synchro d'animation ne serait pas
				 * suffisante....
				 */
				_sequenceurAjouterEvt(sequenceur_pt, EVENT_TEST_ANIM,
						SEQUENCEUR_TEMPO_TEST_ANIM);
				{
					Ktimestamp ts_t;
					Event *evtAnim_pt = NULL;
					ktsGetTime(&ts_t);
					/* On attend 2 minutes avant de refaire un test de l'animation.... */
					kAddTime(&ts_t, 120, 0);
					evtAnim_pt = eventNew(&ts_t, EVENT_TEST_ANIM, NULL, NULL);
					sequenceurReprogrammerEvent(sequenceur_pt, evtAnim_pt);
				}

			}

			/* TRAITEMENT DE LA LUMINOSITE pour tous les caissons.... */

			if (configLuminositeIsChanged()) {
				int32 nbCaissons_dw = configGetNbCaissons();
				int32 indice = 0;
				/* On recalculer la luminosite courante par defaut. */
				configSetLuminositeCourante(configCalculerLuminositeCourante());
				for (indice = 0; indice < nbCaissons_dw; indice++) {
					/* Traiter la luminosite specifique au caisson. */
					if (true == configLumIsChangedCaisson(indice)) {
						sequenceurLuminosite(sequenceur_pt->ecouteursLum_pt,
								indice, configLumCalculer(indice));
						configLumSetChangedCaisson(indice, false);
					}
				}
			}

			/* TRAITEMENT EVENTUEL DE L'AFFICHAGE */
			deltaTick_udw = ktickGetDelta(lastTick_udw, currentTick_udw);
			/* TRAITEMENT DE LA DUREE DE VALIDITE */
			if (!configIsTestEnCours()) {
				int32 nbCaissons_dw = configGetNbCaissons();
				int indice = 0;
				for (indice = 0; indice < nbCaissons_dw; indice++) {
					if (0 != configGetValidite(indice)) {
						if (0 != lastTick_udw) {
							if (configRemoveValidite(deltaTick_udw, indice)) {
								T_pip_sv_act_caisson action;
								T_pip_sv_act_caisson *sv_action = &action;
								sramMessageLireCaisson(indice, sv_action);
								sv_action->finDv = true;
								sramMessageEcrireCaisson(indice, sv_action);
								activation_caisson(indice);
							}
						}
					}
				}
			}
			lastTick_udw = currentTick_udw;
			/* TEST DU NOUVEAU MESSAGE */
			if (configSetOk(stationIsOk()) || _sequenceurMessCourChanged(
					sequenceur_pt)) {
				int32 nbCaissons_dw = configGetNbCaissons();
				int indice = 0;
				for (indice = 0; indice < nbCaissons_dw; indice++) {
					int type = (configGetTypeCaisson(indice));
					if ((type == TYPE_DELEST) || configSetOk(stationIsOk())
							|| _sequenceurMessCourChangedCaisson(sequenceur_pt,
									indice)) {
						switch (configGetTypeCaisson(indice)) {
						case TYPE_ALPHA:
							sequenceurTraiterAffichageCaisson(indice);
							_sequenceurProgrammerAffichageCaisson(
									sequenceur_pt, indice);
							_sequenceurSetEnCoursCaisson(sequenceur_pt, indice,
									true);
							_sequenceurMessCourSetCaisson(sequenceur_pt, indice);
							break;
						case TYPE_DELEST:
							if (true == sequenceurTraiterLestage(sequenceur_pt,
									indice)) {
								reactivation_b = true;
							}
							break;
						}
					}
				}
			}

		} else {

			if (configSetOk(stationIsOk())) {
				/* Il y a un defaut ou un arret de la station. */
				/* On procede a un reset de la la station. */
				if (configIsOk()) {
					printf("Attention, la station est OK \n");
				} else {
					Ktimestamp tsLoc_t;
					Event *evt_pt = NULL;
					ktsGetTime(&tsLoc_t);
					printf("Attention, reprogrammation de l'arret forcee \n");
					eriAjouter(E_eriMajeure, ERI_DEF_SABORDAGE);
					if (stationIsDefautCom()) {
						cmd_trc_tr("SABORDAGE DEFAUT COMMUNICATION", 0);
						evt_pt
								= eventNew(&tsLoc_t, EVENT_SABORDAGE, NULL,
										NULL);
					} else {
						int val;
						int num;
						afficheursIsOkNumAff(&num);
						cmd_trc_tr(
								"SABORDAGE DEFAUT STATION %d (ess:%d) (aff:%d) (num:%d)",
								stationIsOk(), essIsOk(), afficheursIsOk(),
								num, 0);
						/* Effectuer ici une trace du probleme ayant genere le defaut */
						for (val = 0; val < configGetNbAfficheurs(); val++) {
							Afficheur *aff_pt = afficheursChercherParIndex(val);
							if (!afficheurIsToutOk(aff_pt)) {
								_sequenceurTracerAfficheur(val, aff_pt);
							}
						}
						if (!essIsOk()) {
							_sequenceurTracerEs();
						}
						esSetSabordage();
						evt_pt = eventNew(&tsLoc_t, EVENT_ARRET, NULL, NULL);
					}
					sequenceurReprogrammerEvent(sequenceur_pt, evt_pt);
					/* On controle ici la date de demarrage et si on a demarre depuis plus
					 * de une heure, on effectue une relance automatique.
					 * Si il y a effectivement un problème, on ne passera pas une seconde fois
					 * par ici et on restera sabordé.
					 */
					if (kGetDeltaTimestamp(&tsStart_t, &tsLoc_t) > 3600.) {
						ksleep(5000);
#ifdef CPU432
						/* Effacement du message courant */
						reboot(LINUX_REBOOT_CMD_RESTART);
#else
						klogPut(NULL, LOG_INFO, "sequenceurThread : Il faudrait redemarrer");
#endif

					} /* enfif(kGetDeltaTimestamp(&tsStart_t... */
				}
			} else if (stationIsDefautCom() && (essIsConsigne())) {
				int val;
				Ktimestamp tsLoc_t;
				Event *evt_pt = NULL;
				ktsGetTime(&tsLoc_t);
				printf(
						"Attention, reprogrammation de l'arret sur defaut de com\n");
				cmd_trc_tr("SABORDAGE DEFAUT RETRY %d", essIsConsigne(), 0);
				eriAjouter(E_eriMajeure, ERI_DEF_SABORDAGE);
				for (val = 0; val < configGetNbAfficheurs(); val++) {
					Afficheur *aff_pt = afficheursChercherParIndex(val);
					if (afficheurIsDefautCom(aff_pt)) {
						_sequenceurTracerAfficheur(val, aff_pt);
					}
				}
				if (essIsDefautCom()) {
					_sequenceurTracerEs();
				}
				if (false == sequenceurDefautCom_b) {
					evt_pt = eventNew(&tsLoc_t, EVENT_SABORDAGE, NULL, NULL);
				} else {
					evt_pt = eventNew(&tsLoc_t, EVENT_ARRET, NULL, NULL);
					esSetSabordage();
				}
				sequenceurReprogrammerEvent(sequenceur_pt, evt_pt);
				/* On est dans la phase de demarrage et si celui ci n'est pas concluant, on
				 * relance la machine. */
			}

		}
		/* Traitement des defauts d'affichage uniquement si on n'est pas en
		 * phase de test. .... */
		if (0 == (pipGetEtat() & PIP_TEST_BP)) {
			switch (afficheursGetDefautPixel()) {
			case STATION_DEF_PIXEL_NONE:
				if (afficheursGetSortieDefauts() != 0) {
					afficheursSetSortieDefauts(0);
				}
				break;
			default: {
				uint16 masqueDefaut = afficheursGetMasqueDefauts();
				/* Calculer ici le masque de defaut. */
				/* Comparer le masque de defaut au masque courant */
				if (masqueDefaut != afficheursGetSortieDefauts()) {
					/* Changer eventuellement le masque de defaut */
					afficheursSetSortieDefauts(masqueDefaut);
				}

			}
				break;
			}
		}
		/* Traitement des demandes de reinitialisation... */
		if (sequenceurIsReinit()) {
			Event *evtDem_pt = NULL;
			sequenceurSetReinit(false);
			kAddTime(&ts_t, 0, 100);
			stationReset();
			evtDem_pt = eventNew(&ts_t, EVENT_REINIT, NULL, NULL);
			if (false == klistAdd(sequenceur_pt->listeEvts_pt, evtDem_pt)) {
				eventDelete(evtDem_pt);
			}
		}

		/* Generation du status temps reel et composition des defauts. */

		_sequenceur_status_temps_reel(sequenceur_pt);

		/* Traitement des demandes de reactivation. Deplace de tac_ctrl. */

		if (TRUE == tac_ctrl_get_force()) {
			tac_ctrl_reactiver_force();
		} else if (reactivation_b) {
			_sequenceurProgrammerAffichage(sequenceur_pt);
			reactivation_b = false;
		}
		//		mnsusp(0);
		etaSystIncrCompteur(numero);

	}

	return;
}
