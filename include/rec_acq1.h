
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 07/01/02
 * Fichier	: rec_acq1.h
 * Objet	: Prototypage des fonctions des modules d'acquisition des
 * 			detecteurs.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,01Dec00,xag  creation.
 * ************************************************************************	*/

#ifndef _REC_ACQ1_H
#define _REC_ACQ1_H

/* ********************************	*/
/* DEFINITION LOCALES				*/
/* ********************************	*/

#define DET_HS          0x02           /* non reponse du detecteur           */
#define DET_EDF         0x04           /* defaut d'alimentation du detecteur */
#define MAX_RES_DET     2
#define REC_ACQ_NB_LAS	2
#define REC_ACQ_NB_CAPT_MDB	4
#define REC_ACQ_NB_CAPT_LAS	16
#define REC_ACQ_NB_DET_MDB_LAS	4
#define REC_ACQ_NB_DET_VCMT_LAS	16
#define REC_ACQ_MAX_RESET	3
                                                                                /* --- CONSTANTES VCMT                  ---     */
#define REC_ACQ_VCMT_GAIN			0x75
#define	REC_ACQ_VCMT_SEUIL_TRIGGER	0x0A
#define	REC_ACQ_VCMT_DELAI_ON		0x05
#define	REC_ACQ_VCMT_DELAI_OFF		0x20
#define	REC_ACQ_VCMT_OUBLI			3600

#define	REC_ACQ_VCMT_SEUIL_VA_MAX	10

/* ********************************	*/
/* DEFINITION DES TYPES				*/
/* ********************************	*/

/* --------------------------------
 * T_rec_acq_las
 * =============
 * Structure contenant les informa-
 * tions sur le type de detecteur
 * present sur la liasion.
 * --------------------------------	*/

typedef struct T_rec_acq_las
{
  char init;
  char conf;
  char reset;                   /* Indicateur de reset de liaison       */
  /* Indicateur pour demande de syn-
   * chronisation. Utilise pour les
   * detecteurs VCMT.     */
  char lect_co;
  char numero;                                                  /** Numero d'ordre de la liaison, et
										  * on commence par 0.	*/
  char numLas;                  /* Numero de liaison serie utilise      */
  E_type_det type;                                              /** Type de detecteur present sur la 
										 * liaison. Le type est positionne	
										 * si tous les detecteurs presents 
										 * sur la liaison ont le meme type. */
  /* Date de la derniere demande de
   * synchronisation.     */
  double date;
  /* Fonction pour effectuer un reinit
   * des detecteur. */
  void (*reinit) (void);
  union
  {
    T_det mdb[REC_ACQ_NB_DET_MDB_LAS];
    T_det_vcmt vcmt[REC_ACQ_NB_DET_VCMT_LAS];
  } det;

} T_rec_acq_las;

/* ********************************	*/
/* VARIABLES GLOBALES DU MODULE		*/
/* ********************************	*/

extern T_rec_acq_las rec_acq_las[REC_ACQ_NB_LAS];

/* ********************************	*/
/* PROTOTYPAGE DES FONCTIONS		*/
/* ********************************	*/


int rec_acq_mdb_lecture_compteur (T_det *);
int rec_acq_mdb_lecture_valeurs (T_det *);
int rec_acq_mdb_lecture_pic_et_rebond (T_det *);
int rec_acq_mdb_lecture_configuration_seuil (T_det *);
int rec_acq_mdb_lecture_donnees (T_det *);

void rec_acq_mdb_traiter_pic (T_det *, T_usr_ztf *);
void rec_acq_mdb_traiter_compteur (T_det *, T_usr_ztf *);
void rec_acq_mdb_traiter_valeur (T_det *, T_usr_ztf *);

int rec_acq_vcmt_lecture_compteur (T_det_vcmt *);
int rec_acq_vcmt_lecture_valeurs (T_det_vcmt *);
int rec_acq_vcmt_lecture_donnees (T_det_vcmt *);

void rec_acq_vcmt_traiter_etat (T_det_vcmt *, T_usr_ztf *);
void rec_acq_vcmt_traiter_compteur (T_det_vcmt *, T_usr_ztf *);
void rec_acq_vcmt_traiter_vehicule (T_det_vcmt *, T_usr_ztf *);
void rec_acq_vcmt_traiter_valeur (T_det_vcmt *, T_usr_ztf *);

VOID rec_acq (int);
void rec_acq_init_message (T_usr_ztf * message, int det, int code,
                           int taille);
int rec_acq_lire_int (unsigned char *buf, int *valeur);
int rec_acq_lire_int_bis (unsigned char *);
void rec_acq_util_reset_valeurs (T_rec_acq_las *);
int rec_acq_get_detecteur (int numCapt, E_type_det type);
void rec_acq_traiter_etat (T_det * ctrl_det, INT etat);
E_type_det rec_acq_traiter_version (T_usr_ztf *, unsigned char *,
                                    unsigned char *);
T_rec_acq_las *rec_acq_get_info_las (int);

/* ********************************	*/
/* FIN DE REC_ACQ1.H				*/
/* ********************************	*/

#endif
