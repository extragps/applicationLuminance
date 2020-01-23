/*
 * sequenceurTraiter.c
 * ===================
 *
 *  Created on: 1 sept. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: sequenceurTraiter.c,v $
 * Revision 1.2  2018/06/04 08:38:44  xg
 * Passage en version V122
 *
 * Revision 1.3  2008/09/29 07:58:56  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.2  2008/09/22 07:53:43  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.1  2008/09/12 15:01:00  xag
 * Archivage de printemps
 *
 */

/* ***********************************
 * LISTE DES INCLUDES
 * *********************************** */

#include <stdio.h>
#include "kcommon.h"
#include "klog.h"
#include "rdtComm.h"
#include "pip_def.h"

#include "ecouteur.h"
#include "event.h"
#include "configuration.h"
#include "sequenceur.h"
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

#include "pip_str.h"
#include "pip_var.h"

#include "sequenceurTraiter.h"

/* Parmi les fonctions du modules a traiter :
 * - recuperation de la duree de validite du message,
 * - recuperation des valeurs de cligotement : temps a on, temps à off,
 * - recuperation des valeurs pour l'alternat.
 * Attention, les donnees sont dans un premier temps recherchees dans l'action et
 * ensuite dans la configuration du caisson.
 */

/* ***********************************
 * FONCTIONS DU MODULE
 * *********************************** */

static bool _actionIsClignotant(T_pip_act_caisson *action)
{
	bool retour_b = false;
	if (action->clig.type == PIP_ACTIF)
	{
		retour_b = true;
	}
	return retour_b;
}

static int _actionTempsOn(T_pip_act_caisson *action)
{
	int val = action->clig.val[0];
	return val;
}

static int _actionTempsOff(T_pip_act_caisson *action)
{
	int val = action->clig.val[1];
	return val;
}

static bool _actionIsAlternat(T_pip_act_caisson *action)
{
	bool retour_b = false;
	if (action->nbAlternance > 1)
	{
		retour_b = true;
	}
	return retour_b;
}
/*
 * _actionAlternatTempsOn
 * =======================
 * Recuperation du temps a on pour l'une des pages...
 *
 */
static int _actionAlternatTempsOn(T_pip_act_caisson *action, int page)
{
	int val = action->alt.val[page * 2] ;
	return val;
}

/*
 * _actionAlternatTempsOff
 * =======================
 * Recuperation du temps a off pour l'une des pages...
 *
 */
static int _actionAlternatTempsOff(T_pip_act_caisson *action, int page)
{
	int val = action->alt.val[page * 2 + 1] ;
	return val;
}
int32 sequenceurTraiterGetValiditeCaisson(int numCaisson)
{
	int32 retour_dw = -1;
	if ((0 <= numCaisson) && (numCaisson < pip_nb_caisson))
	{
		T_pip_act_caisson *action = &pip_act_caisson[numCaisson];
		retour_dw = action->validite;
	}

	return retour_dw;
}

bool sequenceurTraiterChangementMessage(int numCaisson)
{
	bool retour_b = false;
	if ((0 <= numCaisson) && (numCaisson < pip_nb_caisson))
	{
		T_pip_act_caisson *action = &pip_act_caisson[numCaisson];
		if ((action->act_car == TRUE) || (action->act_lum == TRUE) || (action->act_rythme == TRUE) || (action->act_test
				== TRUE))
		{
			retour_b = true;
		}
	}
	return retour_b;
}

bool sequenceurGetModeLuminosite(int32 numCaisson)
{
	bool retour_b = true;
	if ((0 <= numCaisson) && (numCaisson < pip_nb_caisson))
	{
		T_pip_act_caisson *action = &pip_act_caisson[numCaisson];
		retour_b = (action->luminosite.type == TYPE_AUTOMATIQUE);
	}
	return retour_b;
}

int32 sequenceurGetValeurLuminosite(int32 numCaisson)
{
	int32 numSeuil_dw = 0;
	if ((0 <= numCaisson) && (numCaisson < pip_nb_caisson))
	{
		T_pip_act_caisson *action = &pip_act_caisson[numCaisson];
		switch (action->luminosite.type)
		{
		case TYPE_AUTOMATIQUE:
			break;
		case TYPE_SYMBOLIQUE:
		{
			T_pip_cf_caisson *pipCf = &pip_cf_caisson[numCaisson];
			int indTopo = pipCf->indexTopo;
			if (-1 != indTopo)
			{
				int indEquiv = cfes_trouver_type(pip_cf_topo[indTopo].topoModule, pip_cf_topo[indTopo].topoCaisson,
						action->luminosite.valeur.symbole, PIP_EQUIV_EC);
				if (-1 != indEquiv)
				{
					numSeuil_dw = pip_cf_equiv[indEquiv].equiv.valeur;
				}
			}
		}
			break;
		case TYPE_NUMERIQUE:
			numSeuil_dw = action->luminosite.valeur.graph;
			break;
		}
	}
	return numSeuil_dw;
}
bool sequenceurTraiterLestage(Sequenceur *seq_pt, int numCaisson)
{
bool retour_b=false;
	if ((0 <= numCaisson) && (numCaisson < pip_nb_caisson))
	{
		T_pip_act_caisson *action = &pip_act_caisson[numCaisson];
		if ((action->act_car != FALSE) || (action->act_rythme != FALSE) || (action->act_test != FALSE))
		{
			/* Recuperation du pointeur sur les caracteristiques du caisson */
			T_pip_cf_caisson *caisson = &pip_cf_caisson[numCaisson];
			if (caisson->type == TYPE_DELEST)
			{
			bool consigne_b=(action->extinction==TRUE?false:true);
				if(consigne_b==false)
				{
					if(0==(pipGetEtat()&PIP_DELESTE))
					{
						pipSetEtat(pipGetEtat()|PIP_DELESTE);
						retour_b=true;
					}
				}
				else 
				{
					if(0!=(pipGetEtat()&PIP_DELESTE))
					{
						pipSetEtat(pipGetEtat()&(~PIP_DELESTE));
						retour_b=true;
					}
				}
			}
			action->act_lum = FALSE;
			action->act_car = FALSE;
			action->act_rythme = FALSE;
			action->act_test = FALSE;
		}
	}
	return retour_b;
}

/*
 * sequenceurTraiterAffichageCaisson
 * =================================
 * Traiter a partir des informations de configuration et des
 * informations courante, la commande d'affichage d'un caisson.
 * On ne traite ici a priori que les afficheurs alphanumeriques.
 *
 */
void sequenceurTraiterAffichageCaisson(int numCaisson)
{
	if (configIsOn() && (0 <= numCaisson) && (numCaisson < pip_nb_caisson))
	{
		T_pip_act_caisson *action = &pip_act_caisson[numCaisson];
		if ((action->act_car != FALSE) || (action->act_rythme != FALSE) || (action->act_test != FALSE))
		{
			/* Recuperation du pointeur sur les caracteristiques du caisson */
			T_pip_cf_caisson *caisson = &pip_cf_caisson[numCaisson];
			if (caisson->type == TYPE_ALPHA)
			{
				int nbCarCaisson = caisson->nb_car;
				int premCar = caisson->num_car;
				int indice;
				for (indice = 0; indice < nbCarCaisson; indice++)
				{
					int numCar = premCar + indice;
					Affichage *aff_pt = affichageNew();
					Page *page_pt = pageNew();
					//			/* Ajout d'un message pour chargement d'un buffer. Le numero d'afficheur ne tient
					//			 * pas compte pour l'instant du numero de caisson.*/
					/* Traitement de la premiere page.... */
					{
						int code = pip_act_car[numCar].cmd[0].car1;
						affichageCopierCaractere(aff_pt, 0, code);
						if (_actionIsAlternat(action))
						{
							pageSetTempsOn(page_pt, _actionAlternatTempsOn(action, 0));
							pageSetTempsOff(page_pt, _actionAlternatTempsOff(action, 0));
						} else
						{
							pageSetTempsOn(page_pt, 10000);
						}
						if (_actionIsClignotant(action))
						{
							int tempsOn = _actionTempsOn(action);
							int tempsOff = _actionTempsOff(action);
							Zone *zoneClig_pt = zoneCligNew(tempsOn , tempsOff );
							pageAddZone(page_pt, zoneClig_pt);
						}
						affichageAddPage(aff_pt, page_pt);
					}
					if (_actionIsAlternat(action))
					{
						int code = pip_act_car[numCar].cmd[1].car1;
						page_pt = pageNewOffset(5);
						affichageCopierCaractere(aff_pt, 5, code);
						pageSetTempsOn(page_pt, _actionAlternatTempsOn(action, 1));
						pageSetTempsOff(page_pt, _actionAlternatTempsOff(action, 1));
						if (_actionIsClignotant(action))
						{
							int tempsOn = _actionTempsOn(action);
							int tempsOff = _actionTempsOff(action);
							Zone *zoneClig_pt = zoneCligNew(tempsOn , tempsOff );
							pageAddZone(page_pt, zoneClig_pt);
						}
						affichageAddPage(aff_pt, page_pt);

					}

					if (!afficheursSetAffichage(numCar, aff_pt))
					{
						affichageDelete(aff_pt);
					}

				}
			}
		} else
		{
			/* On regarde si il y a une consigne du luminosite... */
			if (action->act_lum != FALSE)
			{
				/* Traitement specifique de la luminosite... */
				bool auto_b = true;
				/* Recupere le niveau de luminosite souhaité */
				/* Regarder si eventuellement il s'agit du mode
				 * automatique.
				 */
				auto_b = sequenceurGetModeLuminosite(numCaisson);
				configLumSetAutoCaisson(numCaisson, auto_b);
				if (false == auto_b)
				{
					int32 valeur = sequenceurGetValeurLuminosite(numCaisson);
					configLumSetValCaisson(numCaisson, valeur);
				}

				action->act_lum = FALSE;
			}
		}

		/* Effectuer ici le traitement de la validite.... */
		action->act_lum = FALSE;
		action->act_car = FALSE;
		action->act_rythme = FALSE;
		action->act_test = FALSE;

	}
}

/* ********************************
 * FIN DE sequenceurTraiter.c
 * ********************************	*/

