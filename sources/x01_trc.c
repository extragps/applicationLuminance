/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES               **/
/**                                                                       **/
/***************************************************************************/
#ifdef VXWORKS
#include "vxworks.h"
#endif
#include "standard.h"                  /* redefinition des types                         */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
/***************************************************************************/
/**                                                                       **/
/*    SEGMENT : PARAMETRE_SIREDO    CLASS : RAMZP                          */
/**                                                                       **/
/***************************************************************************/
#if STATION
/*#option sep_on segment x01_trc x01_trc class x01_trc x01_trc*/
#endif

/*************************************************************************/
/*  pour la gestion de la simulation                                     */
/*************************************************************************/
struct vct_simulation
{
  UINT8 phase;
  UINT compteur;
  UINT compteur_debit;
  UINT relais;
};

/*************************************************************************/
/*  pour la gestion de la simulation                                     */
/*************************************************************************/
struct vct_simulation vct_simulation;

#if STATION
/*#option sep_off*/
#endif
