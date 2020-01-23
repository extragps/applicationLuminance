
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 09/01/01
 * Fichier	: ficTrace.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,09Jan01,xag  creation.
 * ************************************************************************	*/

#ifndef _FIC_TRACE_LIB_H
#define _FIC_TRACE_LIB_H

/* ********************************
 * INCLUDES
 * ********************************	*/

#include "define.h"
#include "mon_inc.h"
#include "vct_str.h"
#include "lcr_util.h"

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define TRACE_NB_CAR_ADRESSE	6

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* --------------------------------
 * E_ficTraceTypeAdresse
 * =====================
 * --------------------------------	*/

typedef enum E_ficTraceTypeAdresse
{
  E_ficTraceAdresseStandard,
  E_ficTraceAdresseFip
} E_ficTraceTypeAdresse;

/* --------------------------------
 * T_ficTraceAdresseFip
 * ====================
 * --------------------------------	*/

typedef struct T_ficTraceAdresseFip
{
  ULONG numero;
  ULONG ident;
  char module;
} T_ficTraceAdresseFip;

/* --------------------------------
 * T_ficTraceAdresse
 * =================
 * --------------------------------	*/

typedef struct T_ficTraceAdresse
{
  E_ficTraceTypeAdresse type;
  union
  {
    T_ficTraceAdresseFip fip;
    char standard[TRACE_NB_CAR_ADRESSE];
  } val;
} T_ficTraceAdresse;

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/
/* int ficTraceSupprimerEnreg (struct zdf *info); */
int ficTraceAjouterEnreg (int indexFichier,
                          struct timespec *date, T_ficTraceAdresse * ad,
                          int nbCar, char *message);
void ficTraceLister (struct zdf *info);
void ficTraceEnregistrerDialogue (int, char *, struct timespec *, char *, int,
                                  struct timespec *, char *, int);
void ficTraceEnregistrerQuestion (T_ficTraceAdresse *, struct timespec *,
                                  char *, int);
void ficTraceEnregistrerReponse (T_ficTraceAdresse *, struct timespec *,
                                 T_lcr_util_buffer *, char *, int);
void ficTraceEnregistrerReponseBis(T_ficTraceAdresse * adresse, struct timespec *dateReponse,
								char *rep,int nbCarRep, char *commande, int nbCarCmd);
void ficTraceEnregistrerCommande (T_ficTraceAdresse *, struct timespec *,
                                  char *, int);
void ficTraceEnregistrerPosition (T_ficTraceAdresse * ad,STRING chaine);
void ficTraceAdresseSetStandard (T_ficTraceAdresse * ad, char *standard);
void ficTraceAdresseSetFip (T_ficTraceAdresse * ad, ULONG num, ULONG id, char mod);
void ficTraceAdresseSetStandard(T_ficTraceAdresse * ad, char *valeur) ;
void ficTraceSetAdressePort(int port, T_ficTraceAdresse *ad) ;
void ficTraceGetAdressePort(int port, T_ficTraceAdresse *ad) ;
int ficTraceTester(void);
void ficTraceImprimer(INT las, INT mode, STRING buffer, INT * nbCar,
		INT * bloc, INT derBlk, struct usr_ztf *ptMess, int nombre,
		char nature, int dateDebOk, struct tm *dateDeb, int dateFinOk,
		struct tm *dateFin);

/* ********************************
 * FIN DE ficTrace.h
 * ********************************	*/
#endif
