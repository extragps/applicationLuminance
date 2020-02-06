
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 10/05/01
 * Fichier	: eriLib.h
 * Objet	: Prototype des fonctions du module.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,10May01,xag  creation.
 * 01b,17Sep01,xag	Ajout d'un code pour le defaut blocage d'un detecteur et
 * 					pour le test des alertes.
 * 02a,
 * ************************************************************************	*/

#ifndef _ERI_LIB_H
#define _ERI_LIB_H

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define ERI_MAX_DATA	20
                                                                                /* Definition des erreurs       */
#define ERI_DEF_COMM_DETECTEUR	2
#define ERI_DEF_COMM_LON		2
#define ERI_DEF_COMM_PIP		2
#define ERI_DEF_CKS_PROGRAMME	3
#define ERI_DEF_CKS_PIP   		4
#define ERI_DEF_SCRUTATION_PC	5
#define ERI_DEF_CONFIG			50
#define ERI_DEF_SABORDAGE	    51
#define ERI_DEF_ALIM_DETECTEUR	52
#define ERI_DEF_PARASURTENSEUR	53
#define ERI_DEF_PIP				51
#define ERI_DEF_SABORDAGE_TRANS 55
#define ERI_DEF_ES 				56
#define ERI_INF_CHAUFFAGE		57
#define ERI_DEF_TRANS			58
#define ERI_DEF_SERIALISATION	59
#define ERI_DEF_TEMP_AFF		60
#define ERI_DEF_CONF_AFF		61
                                                                                /* Erreurs mineures.            */
#define ERI_DEF_FIN_VALIDITE	5
#define ERI_DEF_BOUCLE			50
#define ERI_DEF_PIXEL			50
#define ERI_DEF_FORCAGE_CONFIG	54
#define ERI_DEF_PARAFOUDRE		52
#define ERI_DEF_CELL_LUMINOSITE	51
#define ERI_DEF_ALIM_AFF		52
#define ERI_DEF_TEMPERATURE		53

#define ERI_DEF_ABE_QT_MIN		53
#define ERI_DEF_ABE_QT_MAX		54
#define ERI_DEF_ABE_TT_MIN		55
#define ERI_DEF_ABE_VT_MOY		56

#define ERI_DEF_EFF_MES			60
#define ERI_DEF_PIC_REB			70
#define ERI_DEF_OUB				80

#define ERI_TST_TEST_EN_COURS	3
#define ERI_TST_ALERTE 			50
#define ERI_TST_ABONNEMENT		51

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* --------------------------------
 * E_eriGrav
 * =========
 * Liste des gravites disponibles pour
 * les erreurs ERI.
 * --------------------------------	*/

typedef enum E_eriGrav
{
  E_eriMajeure,
  E_eriMineure = 5,
  E_eriTest = 8
} E_eriGrav;

/* --------------------------------
 * T_eriData
 * ============
 * Informations stockees lors de
 * l'apparition d'une erreur.
 * --------------------------------	*/

typedef struct T_eriData
{
  char grav;
  char nature;
  unsigned char module;
  unsigned char caisson;
  int valeur;
} T_eriData;

/* --------------------------------
 * T_eriList
 * ============
 * Structure de donnees contenant
 * les erreurs qui ont ete detectees.
 * --------------------------------	*/

typedef struct T_eriList
{
  int nbData;
  T_eriData data[ERI_MAX_DATA];
} T_eriList;

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

bool eriAjouter (char, char);
bool eriSupprimer (char, char);
bool eriChercher (char, char);
void eriInit (void);
int eriLireNbErreur (void);
int eriLireErreur (int, char *, char *);
int eriLireErreurModule (int, char *,char *,int *);
//bool eriSupprimerModule (char grav, char nature,unsigned char mod,unsigned char cais);
bool eriSupprimerModuleVal (char grav, char nature,unsigned char mod,unsigned char cais,int val);
bool eriAjouterModuleVal (char grav, char nature,unsigned char mod,unsigned char cais,int val);

/* ********************************
 * FIN DE eriLib.h
 * ********************************	*/

#endif
