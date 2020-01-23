
#ifndef _modemLib_h
#define _modemLib_h

/* ********************************	
 * DECLARATION DES CONSTANTES
 * ********************************	*/

#define MODEM_MAX_DATA	200

/* ********************************	
 * DECLARATION DES TYPES
 * ********************************	*/

typedef enum E_modemCas
{
  E_modemRien,
  E_modemOk,
  E_modemError,
  E_modemEcho,
  E_modemEcho2,
  E_modemReg2,
  E_modemRingAsync,
  E_modemFlux,
  E_modemLigne,
  E_modemReponse,
  E_modemActivation,
  E_modemEtat,
  E_modemDtr,
  E_modemDtr2,
  E_modemVitesse,
  E_modemPinNok,
  E_modemPinDefaut,
  E_modemReseauCherche,
  E_modemReseauOk,
  E_modemAppel,
  E_modemConnexion,
  E_modemConnexion1,
  E_modemConnexion2,
  E_modemDeconnexion,
  E_modemDeconnexion1,
  E_modemDeconnexion2,
  E_modemDeconnecter,
  E_modemRaccrocher,
  E_modemPasDeClef
} E_modemCas;

typedef struct T_modemTable
{
  E_modemCas cas;
  char *clef;
} T_modemTable;

void modemTraiter (E_modemCas cas);
void modemEnvoyerMots (E_modemCas cas);
E_modemCas modemAnalyser (void);
void modemSetCas (E_modemCas cas);
E_modemCas modemGetCas (void);
int modemLectureArreter (void);
void modemAjouterData (char *data, int nbCar);
int modemLire (void);
int modemEcrire (char *chaine);
int modemConnecter (void);
E_modemCas modemAnalyser (void);
int modemRechercher (E_modemCas cas, char *chaine, int nbCar);;
void modemTraiter (E_modemCas cas);
void modemEnvoyerMots (E_modemCas cas);

#endif
