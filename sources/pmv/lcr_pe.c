
/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_pe                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 17/11/2005                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Fonction d'analyse et de traitement d'une demande /
/ de lecture de l'etat d'affichage d'un module.								  /
/-----------------------------------------------------------------------------/
/                                HISTORIQUE                                   /
/-----------------------------------------------------------------------------/
/ DATE   |   AUTEUR    | VERSION | No de fiche de modification                /
/-----------------------------------------------------------------------------/
/        |             |         |                                            /
/-----------------------------------------------------------------------------/
/        |             |         |                                            /
/-----------------------------------------------------------------------------/
/                    INTERFACES EXTERNES DU SOUS-MODULE                       /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/-----------------------------------------------------------------------------/
/ DONNEES EXPORTEES :                                                         /
/-----------------------------------------------------------------------------/
/ FONCTIONS IMPORTEES :                                                       /
/-----------------------------------------------------------------------------/
/ FONCTIONS EXPORTEES :                                                       /
/-----------------------------------------------------------------------------/
/               DONNEES ET FONCTIONS INTERNES DU SOUS-MODULE                  /
/-----------------------------------------------------------------------------/
/ DONNEES INTERNES :                                                          /
/-----------------------------------------------------------------------------/
/ FONCTIONS INTERNES :                                                        /
/-------------------------------------------------------------------------DOC*/

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include <string.h>
#include "standard.h"

#include "mon_def.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_util.h"
#include "pip_util.h"
#include "lcr_p.h"
#include "lcr_cfes.h"
#include "mon_debug.h"

#include "sramMessage.h"
#include "configurationSram.h"
#include "configuration.h"
#include "cartes.h"

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* *******************************
 * DEFINITION DES STRUCTURE
 * ********************************	*/

typedef struct pe_param
{
  char *txt;
  int val;
  int (*fct_pe) (int, STRING, INT);
  int (*fct_ps) (int, STRING, INT);
  int (*fct_aff) (int, STRING, INT);
  int exclu;
} T_pe_param;


/* ********************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ********************************	*/

static int lcr_pe_af (int indCaisson, STRING buffer, INT j);
static int lcr_ps_af (int indCaisson, STRING buffer, INT j);
static int lcr_pe_cl (int indCaisson, STRING buffer, INT j);
static int lcr_ps_cl (int indCaisson, STRING buffer, INT j);
static int lcr_pe_at (int indCaisson, STRING buffer, INT j);
static int lcr_ps_at (int indCaisson, STRING buffer, INT j);
static int lcr_pe_ec (int indCaisson, STRING buffer, INT j);
static int lcr_ps_ec (int indCaisson, STRING buffer, INT j);
static int lcr_pe_dv (int indCaisson, STRING buffer, INT j);
static int lcr_ps_dv (int indCaisson, STRING buffer, INT j);
static int lcr_pe_mm (int indCaisson, STRING buffer, INT j);
static int lcr_ps_mm (int indCaisson, STRING buffer, INT j);
static int lcr_aff_ec (int indCaisson, STRING buffer, INT j);
static int lcr_aff_dv (int indCaisson, STRING buffer, INT j);

/* ********************************
 * VARIABLES GLOBALES
 * ********************************	*/

static T_pe_param tabPar[] = {
  {"AF", PE_AF, lcr_pe_af, lcr_ps_af, lcr_ps_af,0},
  {"MM", PE_MM, lcr_pe_mm, lcr_ps_mm, lcr_pe_mm,TYPE_DELEST},
  {"DV", PE_DV, lcr_pe_dv, lcr_ps_dv, lcr_aff_dv,TYPE_DELEST},
  {"EC", PE_EC, lcr_pe_ec, lcr_ps_ec, lcr_aff_ec,TYPE_DELEST},
  {"CL", PE_CL, lcr_pe_cl, lcr_ps_cl, lcr_pe_cl,TYPE_DELEST},
  {"AT", PE_AT, lcr_pe_at, lcr_ps_at, lcr_pe_at,TYPE_DELEST},
  {NULL, 0, NULL,0}
};

static int lcr_pe_zero (STRING buffer, INT j)
{
  buffer[j++] = '0';
  return j;
}

/* ----------------------------------
 * lcr_pe_param
 * ============
 * Affichage de la liste des parametres
 * indiques pour une amc donnee.
 * --------------------------------	*/

int lcr_pe_param (int mod, int cais, int indCaisson, int parametres,
                  STRING buffer, INT j, INT qualif)
{
  int indParam = 0;
  /* Impression du numero de module. */
  j += sprintf (&buffer[j], "AM=%d", mod);
  if (-1 != cais)
    {
      j += sprintf (&buffer[j], ".%d", cais);
    }
  /* Impression des parametres . */
  while (NULL != tabPar[indParam].txt)
    {
      if (0 != (parametres & tabPar[indParam].val))
        {
   		T_pip_cf_caisson *cfCaisson = &pip_cf_caisson[indCaisson];

          /* Affichage du parametre pour le caisson */

			if(tabPar[indParam].exclu!=cfCaisson->type)
			{
			  j += sprintf (&buffer[j], " %s=", tabPar[indParam].txt);
			  if (NULL != tabPar[indParam].fct_pe)
				{
				  switch (qualif)
					{
					case PE_ENTREE:
					  j = tabPar[indParam].fct_pe (indCaisson, buffer, j);
					  break;
					case PE_SORTIE:
					  j = tabPar[indParam].fct_ps (indCaisson, buffer, j);
					  break;
					case PE_AFF:
					  j = tabPar[indParam].fct_aff (indCaisson, buffer, j);
					  break;
					}
				}
			}
        }
      indParam++;
    }
  buffer[j]=0;
  return j;
}

/* -------------------------------
 * lcr_ps_af
 * =========
 * Affichage du parametre affichage
 * de l'amc exprime en symbole.
 * --------------------------------	*/

static int lcr_ps_af(int indCaisson, STRING buffer, INT j)
{
	/* On recupere la structure dans laquelle il y a les infos
	 * et on affiche. */
	T_pip_sv_act_caisson caisson;
	T_pip_sv_act_caisson *sv_action = &caisson;
	T_pip_cf_caisson *cfCaisson = &pip_cf_caisson[indCaisson];

	/* Lecture du message affiche. */

	activation_caisson_lire_action(indCaisson, sv_action);

	switch (cfCaisson->type)
	{

	case TYPE_ALPHA:
		if (esIsDefautBis())
		{
//			j = lcr_pe_zero(buffer, j);
			buffer[j++] = '"';
			buffer[j++] = '"';
		}
		else
		{
			if (sv_action->extinction)
			{
				buffer[j++] = '0';
			}
			else if (sv_action->neutre)
			{
				buffer[j++] = 'N';
			}
			else if (sv_action->nbAlternance)
			{
				int indice;
				printDebug("lcr_ps_af %d\n", sv_action->nbAlternance);
				for (indice = 0; indice < sv_action->nbAlternance; indice++)
				{
					T_pip_action *action = &sv_action->action[indice];
					printDebug("lcr_ps_af %d et type %d\n", indice,
							action->type);
					switch (action->type)
					{
					case TYPE_SYMBOLIQUE:
						j += dv1_scpy(&buffer[j], action->valeur.symbole, min (LG_SYMB, strlen (action->valeur.symbole)));
						break;
					case TYPE_LITTERAL:
						buffer[j++] = '"';
						j += dv1_scpy(&buffer[j], action->valeur.litteral.txt,
								min (BUFF_MESS_P,
										action->valeur.litteral.nbcar_txt));
						buffer[j++] = '"';
						break;
					case TYPE_NUMERIQUE:
						j += sprintf(&buffer[j], "%d", action->valeur.graph);
						break;
					case TYPE_NEUTRE:
						buffer[j++] = 'N';
						break;
					default:
						break;
					}
					buffer[j++] = '/';
				}
				j--;
			}
			else
			{
				/* On teste eventuellement le eteint ou le neutre. */
				if (sv_action->neutre == TRUE)
				{
					buffer[j++] = 'N';
				}
				else
				{
					buffer[j++] = '0';
				}
			}
		}
		break;
	case TYPE_DELEST:
		/* Il faudrait rechercher l'equivalence symbolique ici... */
		j += sprintf(&buffer[j], "%d", ((0 == (pipGetEtat() & PIP_DELESTE)) ? 1
				: 0));
		break;
	}
	return j;

}

/* -------------------------------
 * lcr_pe_af
 * =========
 * Affichage du parametre affichage
 * de l'amc.
 * --------------------------------	*/

static int lcr_pe_af(int indCaisson, STRING buffer, INT j)
{
	T_pip_cf_caisson *caisson = &pip_cf_caisson[indCaisson];
	T_pip_act_caisson *action = &pip_act_caisson[indCaisson];

	switch (caisson->type)
	{

	case TYPE_ALPHA:
		if (esIsDefautBis())
		{
			//    		j = lcr_pe_zero (buffer, j);
			/* en cas de defaut, on retourne une chaine d'espaces. */
			int m;
			buffer[j++] = '"';
			for (m = 0; m < caisson->nb_car; m++)
			{
				buffer[j++] = ' ';
			}
			buffer[j++] = '"';
		}
		else
		{
			int trv = FALSE;
			/* si le caisson est au neutre  */
			printDebug("lcr_pe_af, %d alternances\n", action->nbAlternance);
			if (action->nbAlternance == 1)
			{
				int m = caisson->num_car;
				while (m < caisson->num_car + caisson->nb_car)
				{
					if (!pip_car_is_null(&pip_act_car[m].cmd[0]))
					{
						trv = TRUE;
					}
					m++;
				}
			}
			else if (action->nbAlternance > 1)
				trv = TRUE;

			/* si le caisson est actif */
			if ((action->nbAlternance) && (trv))
			{
				int l;
				printDebug("lcr_pe_af, trouve\n", action->nbAlternance);
				for (l = 0; l < action->nbAlternance; l++)
				{
					int m;
					buffer[j++] = '"';
					m = caisson->num_car;
					printDebug("lcr_pe_af, num_car %d nb_car %d\n", m,
							caisson->nb_car);
					while (m < caisson->num_car + caisson->nb_car)
					{
						if ((!pip_act_car[m].def_car)
								|| (x01_status3.rebouclage))
						{
							j
									+= pip_car_dec(&pip_act_car[m].cmd[l],
											&buffer[j]);
						}
						else
						{
							buffer[j++] = '_';
						}
						m++;
					}
					buffer[j++] = '"';
					buffer[j++] = '/';
				}
				j--;
			}
			else
			{
				/*le caisson est au noir */
				j = lcr_pe_zero(buffer, j);
			}
		}
		break;
	case TYPE_DELEST:
		j += sprintf(&buffer[j], "%d", ((0 == (pipGetEtat() & PIP_DELESTE)) ? 1
				: 0));
		break;
	default:
		j = lcr_pe_zero(buffer, j);
		break;
	}
	return j;
}

/* --------------------------------
 * lcr_pe_at
 * =========
 * Affichage du parametre clignotement
 * de l'amc.
 * --------------------------------	*/

static int lcr_pe_at (int indCaisson, STRING buffer, INT j)
{
  T_pip_act_caisson *action = &pip_act_caisson[indCaisson];
  if (action->alt.type != PIP_AUCUN)
    {
      if (action->alt.type == PIP_DEFAUT)
        {
          j += pip_alt_imprimer (&buffer[j], &pip_cf_caisson[indCaisson].alt);
        }
      else
        {
          j += pip_alt_imprimer (&buffer[j], &action->alt);
        }
    }
  else
    {
      j = lcr_pe_zero (buffer, j);
    }

  return j;
}

/* --------------------------------
 * lcr_ps_at
 * =========
 * Affichage du parametre clignotement
 * de l'amc.
 * --------------------------------	*/

static int lcr_ps_at(int indCaisson, STRING buffer, INT j)
{
	T_pip_sv_act_caisson caisson;
	T_pip_sv_act_caisson *action = &caisson;
	T_pip_cf_caisson *cfCaisson = &pip_cf_caisson[indCaisson];
	sramMessageLireCaisson(indCaisson, action);

	switch (cfCaisson->type)
	{
	case TYPE_DELEST:
	default:
		if ((action->alt.type != PIP_AUCUN) && (action->nbAlternance > 1))
		{
			if (action->alt.type == PIP_DEFAUT)
			{
				j += pip_alt_imprimer(&buffer[j],
						&pip_cf_caisson[indCaisson].alt);
			}
			else
			{
				j += pip_alt_imprimer(&buffer[j], &action->alt);
			}
		}
		else
		{
			j = lcr_pe_zero(buffer, j);
		}
		break;
	}

	return j;
}

/* --------------------------------
 * lcr_pe_cl
 * =========
 * Affichage du parametre clignotement
 * de l'amc.
 * --------------------------------	*/

static int lcr_pe_cl (int indCaisson, STRING buffer, INT j)
{
  T_pip_act_caisson *action = &pip_act_caisson[indCaisson];
  printDebug ("lcr_pe_cl, nombre d'alternances : %d\n", action->nbAlternance);
  if (action->nbAlternance == 1)
    {
      /* si il y a du clignotement */
      if (action->clig.type == PIP_DEFAUT)
        {
          j +=
            pip_clig_imprimer (&buffer[j], &pip_cf_caisson[indCaisson].clig);
        }
      else
        {
          j += pip_clig_imprimer (&buffer[j], &action->clig);
        }
    }
  else
    {
      j = lcr_pe_zero (buffer, j);
    }

  return j;
}

/* --------------------------------
 * lcr_ps_cl
 * =========
 * Affichage du parametre clignotement
 * de l'amc.
 * --------------------------------	*/

static int lcr_ps_cl(int indCaisson, STRING buffer, INT j)
{
	T_pip_sv_act_caisson caisson;
	T_pip_sv_act_caisson *action = &caisson;
	T_pip_cf_caisson *cfCaisson = &pip_cf_caisson[indCaisson];
	sramMessageLireCaisson(indCaisson, action);

	switch (cfCaisson->type)
	{
	case TYPE_DELEST:
	default:
		printDebug("lcr_ps_cl, nombre d'alternances : %d\n",
				action->nbAlternance);
		if (action->nbAlternance == 1)
		{
			/* si il y a du clignotement */
			if (action->clig.type == PIP_DEFAUT)
			{
				j += pip_clig_imprimer(&buffer[j],
						&pip_cf_caisson[indCaisson].clig);
			}
			else
			{
				j += pip_clig_imprimer(&buffer[j], &action->clig);
			}
		}
		else
		{
			j = lcr_pe_zero(buffer, j);
		}
		break;
	}
	return j;
}

static int lcr_aff_ec (int indCaisson, STRING buffer, INT j)
{
  T_pip_act_caisson *action = &pip_act_caisson[indCaisson];
  switch (action->luminosite.type)
    {
    case TYPE_AUTOMATIQUE:
      j += dv1_scpy (&buffer[j], "AU", 0);
      break;
    case TYPE_NUMERIQUE:
      j += sprintf (&buffer[j], "%d", action->luminosite.valeur.graph);
      break;
      break;
    case TYPE_SYMBOLIQUE:
      j += sprintf (&buffer[j], "%s", action->luminosite.valeur.symbole);
      break;
    }
  return j;
  return j;
}

static int lcr_ps_ec(int indCaisson, STRING buffer, INT j)
{
	T_pip_act_caisson *actionR = &pip_act_caisson[indCaisson];
	T_pip_sv_act_caisson caisson;
	T_pip_sv_act_caisson *action = &caisson;
	T_pip_cf_caisson *cfCaisson = &pip_cf_caisson[indCaisson];

	sramMessageLireCaisson(indCaisson, action);
	if (esIsDefautBis())
	{
		j += dv1_scpy(&buffer[j], "0/0", 0);
	}
	else
	{
		switch (cfCaisson->type)
		{
		case TYPE_DELEST:
		default:
			switch (action->luminosite.type)
			{
			case TYPE_AUTOMATIQUE:
				j += dv1_scpy(&buffer[j], "AU", 0);
				break;
			case TYPE_NUMERIQUE:
				j += sprintf(&buffer[j], "%d", action->luminosite.valeur.graph);
				break;
			case TYPE_SYMBOLIQUE:
				j += sprintf(&buffer[j], "%s",
						action->luminosite.valeur.symbole);
				break;
			}
			{
				int indTopo = pip_cf_caisson[indCaisson].indexTopo;
				if (-1 != indTopo)
				{
					int tMod = pip_cf_topo[indTopo].topoModule;
					int tCai = pip_cf_topo[indTopo].topoCaisson;
					int indexLoc = cfes_chercher_valeur(tMod, tCai,
							PIP_SYMB_NUMERIQUE, PIP_EQUIV_EC,
							actionR->num_seuil);
					if (-1 != indexLoc)
					{
						T_pip_cf_equiv *inf = &pip_cf_equiv[indexLoc];
						j += sprintf(&buffer[j], "/");
						j += dv1_scpy(&buffer[j], inf->symbole,
								min(strlen(inf->symbole),LG_SYMB));
					}
					else
					{
						j += sprintf(&buffer[j], "/%d", configLumLireSeuil(
								indCaisson));
					}
				}
				else
				{
					j += sprintf(&buffer[j], "/%d", configLumLireSeuil(
							indCaisson));
				}
			}
			break;
		}
	}
	return j;
}

static int lcr_pe_ec (int indCaisson, STRING buffer, INT j)
{
  T_pip_act_caisson *action = &pip_act_caisson[indCaisson];
  switch (action->luminosite.type)
    {
    case TYPE_AUTOMATIQUE:
      j += dv1_scpy (&buffer[j], "AU", 0);
      break;
    case TYPE_NUMERIQUE:
      j += sprintf (&buffer[j], "%d", action->luminosite.valeur.graph);
      break;
    case TYPE_SYMBOLIQUE:
      {
        int indTopo = pip_cf_caisson[indCaisson].indexTopo;
        if (-1 != indTopo)
          {
            int tMod = pip_cf_topo[indTopo].topoModule;
            int tCai = pip_cf_topo[indTopo].topoCaisson;
            int indexLoc = cfes_trouver_type (tMod, tCai,
                                           action->luminosite.valeur.symbole,
                                           PIP_EQUIV_EC);
            if (-1 != indexLoc)
              {
                int valeur = pip_cf_equiv[indexLoc].equiv.valeur;
                j += sprintf (&buffer[j], "%d", valeur);
              }
            else
              {
                j += sprintf (&buffer[j], "??");
              }
          }
        else
          {
            j += sprintf (&buffer[j], "_?");
          }
      }
      break;
    }
  j += sprintf (&buffer[j], "/%d",configLumLireSeuil(indCaisson));
  return j;
}

static int lcr_pe_mm(int indCaisson, STRING buffer, INT j)
{
	int etat = pipGetEtat();
	/* Dans un premier temps, on regarde si le caisson est dans un etat correct.... */
	if (esIsDefautBis())
	{
		j += dv1_scpy(&buffer[j], "OK", 0);
	}
	else if (configIsEnCoursCaisson(indCaisson))
	{
		j += dv1_scpy(&buffer[j], "000/001", 0);
	}
	else if ((etat & PIP_VEILLE) == PIP_VEILLE)
	{
		j += dv1_scpy(&buffer[j], "OK", 0);
	}
	else
	{
		if ((etat & PIP_TEMP) == PIP_TEMP)
		{
			j += dv1_scpy(&buffer[j], "OK", 0);
		}
		else if ((etat & PIP_DELESTE) == PIP_DELESTE)
		{
			j += dv1_scpy(&buffer[j], "OK", 0);
		}
		else if ((etat & PIP_TEST_BP) == PIP_TEST_BP)
		{
			j += dv1_scpy(&buffer[j], "TS", 0);
		}
		else if ((etat & PIP_TEST_TPE) == PIP_TEST_TPE)
		{
			j += dv1_scpy(&buffer[j], "TS", 0);
		}
		else
		{
			j += dv1_scpy(&buffer[j], "OK", 0);
		}
	}
	return j;
}

static int lcr_ps_mm(int indCaisson, STRING buffer, INT j)
{
	int etat = pipGetEtat();
	T_pip_cf_caisson *cfCaisson = &pip_cf_caisson[indCaisson];

	switch (cfCaisson->type)
	{
	case TYPE_DELEST:
	default:
		if (esIsDefautBis())
		{
			j += dv1_scpy(&buffer[j], "OK", 0);
		}
		else

		if (configIsEnCoursCaisson(indCaisson))
		{
			j += dv1_scpy(&buffer[j], "000/001", 0);
		}
		else if ((etat & PIP_VEILLE) == PIP_VEILLE)
		{
			j += dv1_scpy(&buffer[j], "FM", 0);
		}
		else
		{
			if ((etat & PIP_TEMP) == PIP_TEMP)
			{
				j += dv1_scpy(&buffer[j], "FM", 0);
			}
			else if ((etat & PIP_DELESTE) == PIP_DELESTE)
			{
				j += dv1_scpy(&buffer[j], "FM", 0);
			}
			else if ((etat & PIP_TEST_BP) == PIP_TEST_BP)
			{
				j += dv1_scpy(&buffer[j], "TS", 0);
			}
			else if ((etat & PIP_TEST_TPE) == PIP_TEST_TPE)
			{
				j += dv1_scpy(&buffer[j], "TS", 0);
			}
			else
			{
				j += dv1_scpy(&buffer[j], "OK", 0);
			}
		}
		break;
	}
	return j;
}

static int lcr_aff_dv(int indCaisson, STRING buffer, INT j)
{
	T_pip_cf_caisson *caisson = &pip_cf_caisson[indCaisson];
	T_pip_act_caisson *action = &pip_act_caisson[indCaisson];
	if (esIsDefautBis())
	{
		j += dv1_scpy(&buffer[j], "O/0", 0);
	}
	else
	{
	if (action->validite_ori != 0L)
	{
		int validite_ori = action->validite_ori;
		if (-1 == validite_ori)
		{
			j += pip_dv_imprimer(&buffer[j], caisson->validite, 1);
		}
		else
		{
			j += pip_dv_imprimer(&buffer[j], validite_ori, 1);
		}
	}
	else
	{
		j += sprintf(&buffer[j], "0");
	}
	}
	return j;
}

static int lcr_pe_dv(int indCaisson, STRING buffer, INT j)
{
	T_pip_cf_caisson *caisson = &pip_cf_caisson[indCaisson];
	T_pip_act_caisson *action = &pip_act_caisson[indCaisson];
	if (esIsDefautBis())
	{
		j += dv1_scpy(&buffer[j], "O/0", 0);
	}
	else
	{
		if (action->validite_ori != 0L)
		{
			int validite_ori = action->validite_ori;
			//      int validite = action->validite;
			if (-1 == validite_ori)
			{
				j += pip_dv_imprimer(&buffer[j], caisson->validite, 1);
			}
			else
			{
				j += pip_dv_imprimer(&buffer[j], validite_ori, 1);
			}
			buffer[j++] = '/';

			j += pip_dv_imprimer(&buffer[j], (long) (configGetValidite(
					indCaisson) / 1000), 1);
		}
		else
		{
			j += sprintf(&buffer[j], "0/0");
		}
	}
	return j;
}

static int lcr_ps_dv(int indCaisson, STRING buffer, INT j)
{
	T_pip_cf_caisson *caisson = &pip_cf_caisson[indCaisson];
	T_pip_sv_act_caisson caisson2;
	T_pip_sv_act_caisson *action = &caisson2;
	T_pip_cf_caisson *cfCaisson = &pip_cf_caisson[indCaisson];
	sramMessageLireCaisson(indCaisson, action);

	switch (cfCaisson->type)
	{
	case TYPE_DELEST:
	default:
		if ((action->validite_ori != 0L) && (action->validite != 0))
		{
			int validite_ori = action->validite_ori;
			int validite = action->validite;
			if (-1 == validite_ori)
			{
				j += pip_dv_imprimer(&buffer[j], caisson->validite, 1);
			}
			else
			{
				j += pip_dv_imprimer(&buffer[j], validite_ori, 1);
			}
			buffer[j++] = '/';
			j += pip_dv_imprimer(&buffer[j], (long) (configGetValidite(
					indCaisson) / 1000), 1);
		}
		else
		{
			j += sprintf(&buffer[j], "0/0");
		}
		break;
	}

	return j;
}

static void lcr_pe_topo (INT las, INT mode, STRING buffer,
                         T_usr_ztf * pt_mess,
                         INT mod, INT cais, INT parametres, INT qualif)
{
  int j = 0;
  int bloc = 0;
  int indDeb, indFin;
  int fini = FALSE;
  indDeb = pip_util_ind_deb_topo (mod, cais);
  indFin = pip_util_ind_fin_topo (mod, cais);
  printDebug ("lcr_pe_topo : debut %d fin %d\n", indDeb, indFin);
  if (indDeb < indFin)
    {
      int indice;
      for (indice = indDeb; (FALSE == fini) && (indice < indFin); indice++)
        {
          /* Affichage des infos demandees pour l'AM */
          T_pip_cf_topo *topo = &pip_cf_topo[indice];
          if (indice != indDeb)
            {
              j += sprintf (&buffer[j], "\n\r");
            }
          j = lcr_pe_param (topo->topoModule, topo->topoCaisson,
                            topo->indexCaisson, parametres, buffer, j,
                            qualif);
          fini = tedi_test_bloc (las, mode, buffer, &j, &bloc, pt_mess);
        }
      printDebug ("Envoi de la totale lg %d val %s\n", j, buffer);
 		if (x01_status3.rebouclage)
		{
			buffer[j++] = x01_status1.x01_ST_TR;
		} else
		{
			buffer[j++] = vct_ST_TR;
		}

      tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
    }
  else
    {
      /* Erreur de numero de module */
      tedi_ctrl_ext (las, mode, 2);
    }

}

static void lcr_pe_construct (INT las, INT mode, STRING buffer,
                              T_usr_ztf * pt_mess,
                              INT mod, INT cais, INT parametres, INT qualif)
{
  int j = 0;
  int bloc = 0;
  int indDeb, indFin;
  int fini = FALSE;
  indDeb = pip_util_ind_deb_const (mod, cais);
  indFin = pip_util_ind_fin_const (mod, cais);
  printDebug ("lcr_pe_construct : debut %d fin %d", indDeb, indFin);
  if (indDeb < indFin)
    {
      int indice;
      for (indice = indDeb; (FALSE == fini) && (indice < indFin); indice++)
        {
          /* Affichage des infos demandees pour l'AM */
          T_pip_cf_caisson *caisson = &pip_cf_caisson[indice];

          if (indice != indDeb)
            {
              j += sprintf (&buffer[j], "\n\r");
            }
          j = lcr_pe_param (caisson->id_module, caisson->caisson,
                            indice, parametres, buffer, j, qualif);
          fini = tedi_test_bloc (las, mode, buffer, &j, &bloc, pt_mess);
        }
 		if (x01_status3.rebouclage)
		{
			buffer[j++] = x01_status1.x01_ST_TR;
		} else
		{
			buffer[j++] = vct_ST_TR;
		}

      tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);

    }
  else
    {
      /* Erreur de numero de module */
      tedi_ctrl_ext (las, mode, 2);
    }

}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_pe                                                 /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 18/11/05												  /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_pe.c                                             /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : commande de l'etat d'affichage d'un module.			  /
/-------------------------------------------------------------------------DOC*/

VOID lcr_pe (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
             T_usr_ztf * pt_mess)
{
  int sortieSymb = 0;
  char *ptCour = (char *) &buffer[position + strlen ("P")];
  int lgCour = lg_mess - position - strlen ("P");
  int flag_err = 0;
  int mod, cais, amOk;
  int parametres = PE_TOUS_PARAM;
  ptCour[lgCour] = 0;

  /* Controle du type de commande, PE ou PS */
  switch (*ptCour)
    {
    case 'E':
      sortieSymb = 0;
      break;
    case 'S':
      sortieSymb = 1;                  /* Sortie en symbolique */
      break;
    default:
      flag_err = 1;
      break;
    }
  /* Lecture des parametres */
  if (0 == flag_err)
    {
      printDebug ("Analyse des parametres\n");
      ptCour++;
      lgCour--;
      ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
      if (0 < lgCour)
        {
          printDebug ("Traitement du AM %s\n", ptCour);
          /* Traitement des parametres de la liaison.    */
          /* - Lecture de l'AM   */
          if (0 == strncmp (ptCour, "AM=", strlen ("AM=")))
            {
              printDebug ("Traitement du AM\n");
              ptCour += strlen ("AM=");
              lgCour -= strlen ("AM=");
              ptCour =
                tst_lit_am_bis (ptCour, lgCour, &lgCour, &mod, &cais, &amOk);
              /* - Lecture eventuelle de l'un des modifier, par defaut,
               * tout est lu.  */
              printDebug ("Traitement des parametres %d.%d %d\n",
                          mod, cais, amOk);
              if (amOk)
                {

                  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
                  printDebug ("Il reste %d car %s flag_err %d\n",
                              lgCour, ptCour, flag_err);
                  if (0 < lgCour)
                    {
                      int trv = 0;
                      int indCour = 0;
                      /* Controle du parametre e lister. */
                      while ((NULL != tabPar[indCour].txt) && (0 == trv))
                        {
                          if (0 == strncmp (ptCour, tabPar[indCour].txt,
                                            strlen (tabPar[indCour].txt)))
                            {
                              parametres = tabPar[indCour].val;
                              trv = 1;
                              ptCour += strlen (tabPar[indCour].txt);
                              lgCour -= strlen (tabPar[indCour].txt);
                              ptCour =
                                tst_passe_blanc (ptCour, lgCour, &lgCour);
                              parametres = tabPar[indCour].val;
                            }
			    indCour++;
                        }
                      if ((0 == trv) || lgCour)
                        {
                          printDebug ("Probleme parametre %d et lgCour %d\n",
                                      trv, lgCour);
                          flag_err = 2; /* Erreur, parametre non reconnu ou en
                                           trop.    */
                        }

                    }
                }
              else
                {
                  printDebug ("Probleme de lecture...\n");
                  flag_err = 2;        /* Erreur d'interpretation de l'am */
                }
            }
          else
            {
              printDebug ("Pas de AM\n");
              flag_err = 2;            /* il y a obligatoirement un parametre AM= */
            }
        }
      else
        {
          printDebug ("Autre probleme\n");
          flag_err = 2;
        }
    }
  if (0 == flag_err)
    {
      if (pip_nb_topo)
        {
          /* Affichage suivant les am topologiques */
          lcr_pe_topo (las, mode, buffer, pt_mess, mod, cais,
                       parametres, sortieSymb);
        }
      else
        {
          /* Affichage suivant les am constructeur */
          lcr_pe_construct (las, mode, buffer, pt_mess, mod, cais,
                            parametres, sortieSymb);
        }

      /* Sortie des parametres d'affichage */
      if (mod == NN_AFF)
        {
          /* Affichage de tous les caisson */
        }
      else if (cais == N_AFF)
        {
          /* Affichage de tous les caisson d'un module */
        }
      else
        {
          /* Affichage d'un caisson. */
        }
    }
  else
    {
      /* Erreur de commande */
      tedi_ctrl_ext (las, mode, flag_err);
    }

}
