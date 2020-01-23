
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 15/11/00
 * Fichier	: tac_conf.h
 * Objet	: Gestion de la configuration de la station.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 02a,20Feb01,xag  Ajout du traitement des fichiers application.
 * 01a,15Nov00,xag  creation.
 * ************************************************************************	*/

#ifndef _TAC_CONF_H
#define _TAC_CONF_H

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define TAC_CONF_NB_VERSIONS 3
#define TAC_CONF_FICHIER  12

#ifdef CPU432
#define TAC_CONF_REP_VERSIONS "/var/flash/versions"
#else
#define TAC_CONF_REP_VERSIONS "./versions"
#endif

/* ********************************
 * DEFINITION DES TYPES DU MODULE
 * ********************************	*/

/* --------------------------------
 * T_tacConfFichiers
 * =================
 * Structure contenant les infor-
 * mations utiles pour la gestion
 * du telechargement.
 * Le fichier courant est le nom du
 * fichier en cours d'exploitation.
 * Le fichier exploit est le nom du
 * fichier qui doit etre mis en
 * exploitation. Si le fichier est
 * different du fichier courant, le
 * basculement s'effectuera lors du
 * lancement de l'application, le
 * controle s'effectuant e ce moment
 * la.
 * Les versions disponibles sont
 * stockees dans un tableau dans
 * lequel sont definies :
 * -	le nom du fichier,
 * -	la date de telechargement,
 * -	l'adresse du fichier.
 * --------------------------------	*/

typedef struct T_tacConfFichiers
{
  char courant[TAC_CONF_FICHIER];
  char exploit[TAC_CONF_FICHIER];
  unsigned long courantBcc;
  unsigned long courantDate;
  struct
  {
    char nom[TAC_CONF_FICHIER];
    unsigned long date;
    void *adresse;
    unsigned long bcc;
  } version[TAC_CONF_NB_VERSIONS];
  long bcc;
} T_tacConfFichiers;

/* ********************************
 * PROTOTYPAGE DES FONCTION DU MODULE
 * ********************************	*/

/* Fonction reservee e la lecture de la configuration de
 * l'equipement.	*/
unsigned short tac_conf_cfg_get_cnf (void);
VOID 	tac_conf_cfg_ecr_conf (void);
VOID 	tac_conf_cfg_ecr_entete (VOID);
UINT8 	tac_conf_cfg_lec_entete (VOID);
UINT8 	tac_conf_cfg_lec_conf (void);
void 	tac_conf_cfg_raz_entete(void);
void 	tac_conf_cfg_raz_conf (void);

/* Fonctions reservee a la gestion des version de l'application
 * ainsi qu'a la mise en service. */

int 	tac_conf_cfg_lec_fichier (T_tacConfFichiers *);
int 	tac_conf_cfg_ecr_fichier (T_tacConfFichiers *);
int 	tac_conf_cfg_copier_systeme (int);
int 	tac_conf_cfg_chercher_fichier (T_tacConfFichiers *, char *);
int 	tac_conf_cfg_elire_fichier (T_tacConfFichiers *, char *);
int 	tac_conf_cfg_traiter_fichier (T_tacConfFichiers *, char *, int);
int 	tac_conf_cfg_tester_fichier (T_tacConfFichiers *);
unsigned long tac_conf_cfg_calculer_checksum (int);
unsigned long tac_conf_cfg_calculer_checksum_appli (void);
int 	tac_conf_cfg_lire_taille (void);
void * 	tac_conf_cfg_lire_flash(int ficSel,int *tailleFic);
int 	tac_conf_cfg_lire_fichier(T_tacConfFichiers * confFichier);
int 	tac_conf_cfg_ajouter_fichier(T_tacConfFichiers * confFichier, char *nomFichier);
void 	tac_conf_cfg_raz_fin_fichier(int ficSel, int sectCour);
int 	tac_conf_cfg_ecrire_checksum(T_tacConfFichiers * confFichiers, int ficSel, unsigned long bcc);

#endif
/* *********************************
 * FIN DE tac_conf.h
 * ********************************* */

