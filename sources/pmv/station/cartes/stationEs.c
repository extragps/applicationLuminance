#include <stdio.h>
#include <string.h>
#include "kcommon.h"
#include "klog.h"
#include "standard.h"
#include "affichage/zone.h"
#include "affichage/page.h"
#include "affichage/caractere.h"
#include "affichage/affichage.h"
#include "infos/lumInfos.h"
#include "lumAdresse.h"
#include "lumMsg.h"
#include "luminance/lumMsgCreer.h"
#include "affichage/afficheur.h"
#include "es/es.h"
#include "thl/thl.h"
#include "event.h"
#include "cartes.h"
#include "cartesAfficheur.h"
#include "configuration.h"
#include "eriLib.h"

static Es *esCarte;

void essInit(void)
{
	int indice = 0;
	/* TODO : Initialisation des cartes temperature et luminosite.
	 * Par la meme occasion on peut ajuster leur configuration. */
	esCarte = esNew(indice);
	/* Evolution, on multiplie par deux de delai de polling de l'es pour eviter
	 * les problemes de sabordage (on espère au moins.... */
	esSetPolling(esCarte, configGetPollingEs()*4);
	esSetPadding(esCarte,configGetPaddingBefore(),configGetPaddingAfter());
	esSetNumber(esCarte, configGetAdresseEs());
}

void essTerm(void)
{
	/* Liberation des ressources allouees. */
	esDelete(esCarte);
}

void esReset()
{
	if (NULL != esCarte)
	{
		esCarte->conf_b = false;
	}
}

bool esIsSabordage()
{
	return esCarte->saborde_b;
}

bool esIsDefautAlim()
{
	return esGetDefautAlim(esCarte);
}

bool esIsDefautBis()
{
	return  (essIsDefautCom() || esIsDefaut(esCarte)
			|| (esGetDefautAlim(esCarte) != 0)||esIsSabordage());
}

void esSetSabordage()
{
	if (NULL != esCarte)
	{
		esCarte->saborde_b = true;
	}
}
void esResetSabordage()
{
	if (NULL != esCarte)
	{
		esCarte->saborde_b = false;
	}
}

void essGetInfos(char *infos, int longueur)
{
	if (essIsDefautCom() || essDefautAlim() || esIsDefaut(esCarte)
			|| (esGetDefautAlim(esCarte) != 0)||esIsSabordage())
	{
		snprintf(infos, longueur, " E=00 ");
	}
	else
	{
		infos[0] = 0;
	}
}
bool essDefautAlim()
{
	bool retour_b = false;
	if (configIsOn())
	{
		/* On regarde si les deux alims sont OK!!! */
		if ((esGetInput(esCarte) & ES_ALIM_INPUT) != ES_ALIM_INPUT)
		{
			retour_b = true;
		}
	}
	return retour_b;
}
bool essAlimCoupee()
{
	bool retour_b = false;
	if (configIsOn())
	{
		/* On regarde si les deux alims sont OK!!! */
		if ((esGetInput(esCarte) & ES_ALIM_INPUT) == 0)
		{
			retour_b = true;
		}
	}
	else
	{
		/* L'alim n'est pas sous tension */
		retour_b = true;
	}
	return retour_b;
}

Event * essTraiterMessage(LumMsg *msg_pt)
{
	/* Récupérer l'adresse de la carte. */
	int adresse = lumAdresseGetLumAdresse(lumMsgGetSource(msg_pt));
	Event *evt_pt = NULL;
	Es *es_pt = essChercherParAdresse(adresse);
	if (NULL != es_pt)
	{
		uint8 inputs;
		bool status_b = esTraiterMessage(es_pt, msg_pt);
		if (!status_b)
		{
			uint16 tempoReprog;
			tempoReprog = esTesterReprogrammation(es_pt);
			if (0 != tempoReprog)
			{
				Ktimestamp ts_t;
				ktsGetTime(&ts_t);
				kAddTime(&ts_t, 0, tempoReprog);
				evt_pt = eventNew(&ts_t, EVENT_ES, es_pt, NULL);
			}
		}
		else
		{
			inputs = esGetInput(es_pt);
			if (configSetOn((inputs & ES_ALIM_INPUT) != 0))
			{
				Ktimestamp ts_t;
				ktsGetTime(&ts_t);
				kAddTime(&ts_t, 0, 100);
				evt_pt = eventNew(&ts_t, EVENT_ALLUMAGE, NULL, NULL);

			}
			if ((configIsOn() && (!essIsConsigne())))
			{
				/* Reprogrammer un evenement d'extinction.... */
				Ktimestamp ts_t;
				printf("Consigne of et config on???? \n");
				ktsGetTime(&ts_t);
				kAddTime(&ts_t, 0, 3000);
				eventDelete(evt_pt);
		        cmd_trc_tr ("SABORDAGE DEFAUT CONSIGNE", 0);
				eriAjouter(E_eriMajeure,ERI_DEF_SABORDAGE);
				esSetSabordage();
				evt_pt = eventNew(&ts_t, EVENT_ARRET, NULL, NULL);
			}
			else if (((!configIsOn()) && (essIsConsigne())))
			{
				printf("La station devrait etre allumee \n");
				if (esIncrDefautAlim(es_pt))
				{
					Ktimestamp ts_t;
					ktsGetTime(&ts_t);
					kAddTime(&ts_t, 0, 3000);
					eventDelete(evt_pt);
		        	cmd_trc_tr ("SABORDAGE DEFAUT ALLUMAGE %#0x",inputs, 0);
					eriAjouter(E_eriMajeure,ERI_DEF_SABORDAGE);
					esSetSabordage();
					evt_pt = eventNew(&ts_t, EVENT_ARRET, NULL, NULL);
				}
			}
			else
			{
				if (essIsConsigne())
				{
					esResetDefautAlim(es_pt);
				}
			}

		}

	}
	else
	{
		klogPut(NULL, LOG_INFO, "essTraiterMessage: probleme d'adresse %d",
				adresse);
	}
	return evt_pt;
}

/* essIsOk
 * =======
 * Permet de détermininer si toutes les cartes sont OK...
 * */

bool essIsOk()
{
	return (esCarte->vers_b && esCarte->conf_b);
}

bool essIsDefaut()
{
	/* TODO : Ajouter le traitement des defauts */
	return esIsDefaut(esCarte);
}

void essSetConsigne(bool consigne_b)
{
	esSetConsigne(esCarte, consigne_b);
}

bool essIsConsigne()
{
	return esIsConsigne(esCarte);
}

bool essIsConf()
{
	return esIsConf(esCarte);
}

bool essIsDefautCom()
{
	return esIsDefautCom(esCarte);
}

void essTraiterDefautCom(LumMsg *msg_pt)
{
	int adresse = lumAdresseGetLumAdresse(lumMsgGetDest(msg_pt));
	Es *aff_pt = essChercherParAdresse(adresse);
	if (NULL != aff_pt)
	{
		esTraiterDefautCom(aff_pt, msg_pt);
	}
	else
	{
		klogPut(NULL, LOG_INFO, "essTraiterMessage: probleme d'adresse %d",
				adresse);
	}
}

Es *essChercherParIndex(int index)
{
	Es *es_pt = NULL;
	if (0 == index)
	{
		es_pt = esCarte;
	}
	return es_pt;
}
Es *essChercherParAdresse(int adresse)
{
	Es *es_pt = NULL;
	if (esCarte->numCarte_dw == adresse)
	{
		es_pt = esCarte;
	}
	else
	{
		printf("Adresse de la carte ES non trouvee %d pour %d\n",
				esCarte->numCarte_dw, adresse);
	}
	/* Recherche parmi les deux cartes THS, de celle qui a la bonne adresse. */
	return es_pt;
}
