/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 04/05/01
 * Fichier	: rec_Util.c
 * Objet	: Fonctions utilitaires utilisees par les modules de reception.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,04May01,xag  creation.
 * ************************************************************************	*/

/* ********************************	
 * FICHIERS D'INCLUDE
 * ********************************	*/

#ifdef VXWORKS
#include "vxworks.h"
#include "tickLib.h"
#endif
#include "stdio.h"
#include "time.h"
#include "limits.h"
#include "standard.h"
#include "mon_debug.h"
#include "define.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "x01_str.h"
#include "x01_var.h"
#include "rec_util.h"
#include "simuLib.h"

/* ********************************		
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

static int rec_util_debug = 0;

/* ********************************	
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************	
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************	
 * FONCTIONS DU MODULE
 * ********************************	*/

UINT8 rec_util_tester_alerte (T_vct_critere * critere, UINT valeur)
{
  UINT8 retour = TRUE;
  switch (critere->relation)
    {
    case REL_SEUIL_SUP:
      if ((critere->prec > critere->seuil)
          || (valeur < critere->seuil) || (REL_ETAT_IND == critere->etat))
        {
          retour = FALSE;
        }
      break;
    case REL_SUP:
      if (valeur < critere->seuil)
        {
          retour = FALSE;
        }
      break;
    case REL_INF:
      if (valeur > critere->seuil)
        {
          retour = FALSE;
        }
      break;
    case REL_SEUIL_INF:
      if ((critere->prec < critere->seuil)
          || (valeur > critere->seuil) || (REL_ETAT_IND == critere->etat))
        {
          retour = FALSE;
        }
      break;
    case REL_EGA:
      if (valeur != critere->seuil)
        {
          retour = FALSE;
        }
      break;
    case REL_DIFF:
      if ((valeur == critere->prec) || (REL_ETAT_IND == critere->etat))
        {
          retour = FALSE;
        }
      break;
    }
  if (rec_util_debug)
    {
      printf ("Les valeurs op %d val %d seuil %d etat %d prec %d retour %d\n",
              critere->relation, valeur, critere->seuil, critere->etat,
              critere->prec, retour);
    }
  critere->prec = valeur;
  return retour;
}

char *rec_util_lire_relation (UINT8 relation)
{
  char *retour = "";
  switch (relation)
    {
    case REL_SUP:
      retour = ">";
      break;
    case REL_SEUIL_SUP:
      retour = ">>";
      break;
    case REL_INF:
      retour = "<";
      break;
    case REL_SEUIL_INF:
      retour = "<<";
      break;
    case REL_EGA:
      retour = "=";
      break;
    case REL_DIFF:
      retour = "=>";
      break;
    }                                  /* endswitch(type)                                      */
  /* --------------------------------
   * FIN DE rec_ing_lire_relation
   * --------------------------------     */
  return retour;
}

char *rec_util_lire_alerte (UINT8 nature)
{
  char *retour = "";
  switch (nature)
    {
    case AL_CKS:
      retour = "CKS";
      break;
    case AL_EDF:
      retour = "EDF";
      break;
    case AL_GAR:
      retour = "GAR";
      break;
    case AL_RST:
      retour = "RST";
      break;
    case AL_INI:
      retour = "INI";
      break;
    case AL_TRM:
      retour = "TRM";
      break;
    case AL_ERR:
      retour = "ERR";
      break;
    case AL_ER1:
      retour = "ER1";
      break;
    case AL_ER2:
      retour = "ER2";
      break;
    case AL_ER3:
      retour = "ER3";
      break;
    case AL_BCL:
      retour = "BCL";
      break;
    case AL_BTR:
      retour = "BTR";
      break;
    }
  return retour;
}

#ifdef RAD
char *rec_ing_lire_type (UINT8 type)
{
  char *retour = "";
  switch (type)
    {
    case N_QT:
      retour = "QT";
      break;
    case N_TO:
      retour = "TT";
      break;
    case N_VT:
      retour = "VT";
      break;
    case N_QL:
      retour = "QL";
      break;
    }                                  /* endswitch(type)                                      */
  /* --------------------------------
   * FIN DE rec_ing_lire_type
   * --------------------------------     */
  return retour;
}

char *rec_int_lire_type (UINT8 nature)
{
  char *retour = "";
  switch (nature)
    {
    case DI:                          /* distance inter vehiculaire */
      retour = "DI";
      break;
    case EI:
      retour = "EI";
      break;
    case II:                          /* temps inter vehiculaire */
      retour = "II";
      break;
    case LI:                          /* longueur */
      retour = "LI";
      break;
    case TI:                          /* temps de presence */
      retour = "TI";
      break;
    case VI:                          /* vitesse */
      retour = "VI";
      break;
    }
  return retour;
}
#endif
