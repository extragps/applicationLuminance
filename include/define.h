/*DOC-------------------------------------------------------------------------
/                                                                            /
/          DEFINITION DES CONSTANTES COMMUNES A PLUSIEURS TACHES             /
/                                                                            /
-------------------------------------------------------------------------DOC*/

#ifndef DEFINE_RAD_H
#define DEFINE_RAD_H

#ifdef LINUX
#ifndef NULL
#define NULL 0
#endif
#define ERROR -1
#define OK 0
#define TRUE 1
#define FALSE 0
#endif


#define SZ_TETEFIME 80
#define MAX_CAPTEUR_GENEVE    0

#define MAX_MESSAGE 20
/* #define MAX_MESSAGE_LCR 250 */
#define MAX_MESSAGE_LCR 500
#define MAXVILT    64                  /* nombre maximum de donnees vilt dans la fifo vilt       */
#define MAXBRUTE   64                  /* nombre maximum de donnees brutes dans la fifo          */
#define NBCAPTEUR  32                  /* nombre de capteur de mesures definissables=NBDET*4 */
#define NBCAPTEURDEFAUT 12             /* Nombre de capteurs configures par la commande
                                          CFC S.       */
#define NBCANAL    63                  /* nombre de canal pour la station                        */
#define NBCAPTEURCANAL 98              /* liste des capteurs / canaux                    */

#define NB_CPT_TV	5              /* Nombre de compteurs TV. Ajoute par XG le 29 nov      */
                                                /* 2000 pour integrer les donnees 10, 20 et 60 sec.     */
                                                                                /* ---------------------------------
                                                                                 * INFOS POUR LA BASE
                                                                                 * --------------------------------     */
                                                                                /* Il s'agit de definir les constantes
                                                                                 * utilisee pour la taille et l'a-
                                                                                 * dresse de la base. Cela a ete rendu
                                                                                 * necessaire par le fait que vxWorks
                                                                                 * effectue une remise e zero au
                                                                                 * demarrage.   */
#define BD_SZ_MEMOIRE   0x080000
#define BD_AD_MEMOIRE	0x180000
#define BD_INTEGRITE	0x5AA5AAA5

#define VERSION_SIR 0
#define VERSION_GEC 1
#define VERSION_ESC 2
#define VERSION_GEN 3
#define SZ_BLK     256
#define SZ_BLK_DET 80

#define MAXBLK       249
#define MAXLCR       200
#define MAX_10_BLK   220
#define MAX_USR_LCPI	40
#define MAX_INFO_LCPI	1000
#define NBDET     	8              /* nombre de detecteurs     */
#define NBDET_VCMT	32             /* nombre de detecteurs VCMT */
#define NBCOM185  4                    /* nombre de carte com185                             */
#define NBLASDET  1                    /* nb de las affectees aux detecteurs */
#define NBID      5                    /* nombre d'utilisateur de la station */

#define MAXSEQ    5                    /* nombre maxi de seqs configurables  */
#define MAX_TPS_ESSIEU 	8              /* nbre maxi de tps inter essieu mesure par det   */
                                                                                /* ---------------------------------
                                                                                 * AJOUT DE X.GAILLARD LE 20/02/2002
                                                                                 * =================================
                                                                                 * Dans le cadre de l'integration du
                                                                                 * detecteur VCMT.
                                                                                 * --------------------------------     */
#define MAX_DET_CANAL	5              /* Nombre maximal de detection dans la file tampon */
 /* Temps que l'on considere comme acceptable entre
  * la fin d'une detection et le debut de la suivante
  * dans les deux cas suivants :
  * - les detections sont declarees comme adjacentes
  * - les deux detections ne sont pas adjacentes.
  * L'adjacence est signalee par le parametre pres
  * dans la donnee brute en provenance du detecteur.
  * */
#define DELTA_DET_VITESSE		50.0
#define DELTA_DET_VITESSE_PLUS	200.0
#define DELTA_CHEVAUCHE			25.0
#define DELTA_CHEVAUCHE_2		50.0
#define SEUIL_CHEVAUCHEMENT_SIMPLE	4
#define SEUIL_CHEVAUCHEMENT_DOUBLE	7
#define SEUIL_VAM_MAX				25
#define SEUIL_VAM_MAX_CHEVAUCHE		75
                                                                                /* ---------------------------------
                                                                                 * FIN DE AJOUT DE X.GAILLARD
                                                                                 * =================================
                                                                                 * --------------------------------     */

#define NBMCLASS 14                    /* nombre maximum de classes configurables            */
#define NBVCLASS 12                    /* nombre de classes de vitesses                      */
#define NBLCLASS  6                    /* nombre de classes de longueurs                     */
#define NBKCLASS 14                    /* nombre de classes de silhouettes                   */
#define NBPCLASS  6                    /* nombre de classes de poids                         */
#define NBECLASS  6                    /* nombre de classes de poids essieu                  */
#define NBTCLASS  6                    /* nombre de classes de taux d'occupation             */
#define NBKSLASS 14                    /* nombre de classes de silhouettes en surcharge      */
#define NBPMLASS 14                    /* nombre de classes de silhouettes poids moyen       */
#define NBPTLASS 14                    /* nombre de classes de silhouettes poids total       */

#define MAXDATA  17                    /* nombre maximum de types de donnees ou de classe    */
                                        /* contenu dans un bloc de donnees voies              */
#define NBFC      9                    /* nombre de fichiers classifies                      */
#define MAXDD     8                    /* nombre de distances dynamiques dans la fifo        */
#define SZ_LIG_ST_CANAL 42             /* nombre de caracteres d'une ligne de reponse */
                                        /* a la commande ST V                                   */
#define SEUIL_CLASSE_LC  0
#define SEUIL_CLASSE_VC  1
#define SEUIL_CLASSE_KC  2
#define SEUIL_CLASSE_PC  3
#define SEUIL_CLASSE_EC  4
#define SEUIL_CLASSE_TC  5
#define SEUIL_CLASSE_KS  6
#define SEUIL_CLASSE_PM  7
#define SEUIL_CLASSE_PT  8

#define MINUTE   0                     /* pour la periode utilisateur                       */
#define SECONDE  1

/****** numero de code des differentes natures de donnees                    */
#define N_QT     0                     /* debit tout vehicule                                */
#define N_TO     1                     /* taux d'occupation tout vehicule                    */
#define N_VT     2                     /* vitesse tous vehicules                             */
#define N_QL     3                     /* debit poids lourd                                  */
#define N_VL     4                     /* vitesse  poids lourd                                  */
#define N_ST     5                     /* saturation                                         */
#define N_IT     6                     /* temps intervehiculaire moyen                       */
#define N_LC     8                     /* longueurs classifiees                              */
#define N_VC     9                     /* vitesses classifiees                               */
#define N_KC     10                    /* silhouettes classifiees                            */
#define N_PC     11                    /* poids classifies                                   */
#define N_EC     12                    /* poids essieu classifies                            */
#define N_TC     13                    /* taux d'occupation classifies                       */
#define N_TNT    17                    /* toutes natures tous vehicules                      */
#define N_TNC    18                    /* toutes natures classifiees                         */
#define N_TN     19                    /* toutes natures                                     */
#define N_TNTPL  20                    /* toutes natures PL                                  */

#define MAXNATURE 17                   /* nombre de natures de mesures elaborables par la    */

/***** definition des masques pour la lecture des ports                      */
#define MASK_SECURITE  0x01            /* rang du bit dans le port en entree donnant    */
                                                                         /* l'etat du strap de securite                   */
#define MASK_TERMINAL  0x02            /* rang du bit dans le port en entree donnant    */
                                 /* l'etat du strap de validation du terminal     */
#define MASK_FORCAGE   0x04            /* rang du bit dans le port en entree donnant    */
                                                                         /* l'etat du strap de forcage                    */
/***** definition du status temps reel            */
#define STATUS_TR_EDF     0x01         /* b1  EDF */
#define STATUS_TR_INIT    0x02         /* b0  initialisation */
#define STATUS_TR_TERM    0x04         /* b2  TERMINAL */
#define STATUS_TR_ERR_MAJ 0x08         /* b3  erreur majeur ou interne */
#define STATUS_TR_ALERTE  0x10         /* b4  condition d'alerte */
#define STATUS_TR_ERR_MIN 0x20         /* b5  erreur mineure */
#define STATUS_TR_OK      0x40         /* b6  force e 1 */

#define TYPE_INIT_INIT   0
#define TYPE_INIT_RST    1
#define TYPE_INIT_WDG    2

/***** definition du temps de valididite de certaines commandes              */

#define TOP        16                  /* converti des secondes en top timer                */
#define TEMP_ID    480                 /* 30 s                                              */
#define TEMP_TERM  960                 /* 60 s                                              */
#define TEMP_XOFF  480                 /* 30 s                                              */
#define TEMPO_CONTRE_SENS   5          /* 5 secondes */

#define XOFF_EMIS    0x01
#define XON_EMIS     0xfe
#define XOFF_RECU    0x02
#define XON_RECU     0xfd

#define EVT_EDF      0
#define EVT_GAR      1
#define EVT_INI      2
#define EVT_TRM      3
#define EVT_BCL      4
#define EVT_RST      5

/* HORLOGE */


#define INIT_REC_CARTE 1
#define INIT_DEFAUT    2
#define NB_CARTE       4
#define AD_MSO         0xE04000
#define AD_MSR_SIMUL   0xE07000

/* adresse carte com185   */
#define AD_COM_185   0xE00000

/* adresse carte int108   */
#define AD_INT_108   0xE02000

/* sens de circulation    */

#define SENS_NAFF         0            /* circulation non determinee         */
#define SENS_NORMAL       1            /* circulation capteur pair -> impair */
#define SENS_INVERSE      2            /* circulation capteur impair -> pair */
#define SENS_SEUIL_HAUT	  20           /* Pour qu'on affecte un sens, il faut que
                                        * moins de 20 % du traffic passe en sens
                                        * inverse.     */
#define SENS_SEUIL_BAS 	  500          /* ou 500 % dans l'autre sens.  */

#define SENS_MAX_VEHICULE 200          /* Lorsque l'on depasse 200 vehicule sur un
                                        * sens, on divise les compteurs par 2. */
#define SENS_MIN_VEHICULE 10           /* Nombre minimum de vehicules e comptabiliser
                                        * sur une voie avant de determiner le sens de
                                        * circulation. */


typedef enum E_relation
{
  REL_INF,
  REL_SUP,
  REL_DIFF,
  REL_EGA,
  REL_SEUIL_INF,
  REL_SEUIL_SUP,
  REL_MAX
} E_relation;

typedef enum E_relEtat
{
  REL_ETAT_OFF,
  REL_ETAT_ON,
  REL_ETAT_IND
} E_relEtat;

#define REL_INF  0                     /* pour inferieur au seuil                  */
#define REL_SUP  1                     /* pour superieur au seuil                  */
#define REL_DIFF 2                     /* pour different du seuil                  */
#define REL_EGA  3                     /* pour egal au seuil                       */

#define OP_AND   0                     /* pour ET logique                          */
#define OP_OR    1                     /* pour OU logique                          */

#define VI       0                     /* vitesse instantannee                     */
#define II       1                     /* temps intervehiculaire                   */
#define LI       2                     /* longueur d'un vehicule                   */
#define DI       3                     /* distance inter-vehiculaire               */
#define TI       4                     /* temps de presence                        */
#define EI       5                     /* poids d'un essieu                        */
#define HI       6                     /* poids d'un essieu                        */
#define MAX_IND_VILT 5                 /* pas EI */

#define HMVL_VT   0
#define HMVL_LG   1
#define HMVL_TT   2
#define HMVL_TI   3
#define HMVL_SIL  4
#define HMVL_PDS  5
#define HMVL_NE   6
#define HMVL_PE   7

typedef enum T_alerteSysteme
{
  AL_N_AFF,
  AL_CKS,
  AL_GAR,
  AL_RST,
  AL_INI,
  AL_TRM,
  AL_ERR,
  AL_ER1,
  AL_EDF,
  AL_ER2,
  AL_ER3,
  AL_BCL,
  AL_BTR,
  AL_MAX
} T_alerteSysteme;

#define MAX_CRT_IN 6                   /* nombre de critere pour les grandeurs individuelles */
#define MAX_CRT_MY 4                   /* nombre de critere pour les grandeurs moyennes     */
#define MAX_CRT_SY 7                   /* nombre de critere pour les grandeurs moyennes     */
#define MAX_CAR_AL 32                  /* nombre max de caractere  pour message d'alerte    */
#define MAX_MESS_AL 5                  /* nombre max de message d'alerte                    */
#define TP_6MIN 720000L                /* tempo 6 minute pour la surveillance des alertes */
/* pour le formatage des donnees */

#define NB_QT    4
#define QT_V     0
#define QT_B     1
#define QT_H     2
#define QT_J     3

#define NB_D_B   3

#define B_TO     0
#define B_VT     1
#define B_ST     2
/* pour les masques de configuration */
#define MSQ_PDS   0x00000001
#define MSQ_QT    0x00000002
#define MSQ_TO    0x00000004
#define MSQ_VT    0x00000008
#define MSQ_QL    0x00000010
#define MSQ_ST    0x00000020
#define MSQ_IT    0x00000040
#define MSQ_LC    0x00000080
#define MSQ_VC    0x00000100
#define MSQ_KC    0x00000200
#define MSQ_PC    0x00000400
#define MSQ_EC    0x00000800
#define MSQ_TC    0x00001000
#define MSQ_KS    0x00002000
#define MSQ_PM    0x00004000
#define MSQ_PT    0x00008000
#define MSQ_VPL   0x00010000

/* taille des enregistrements */
/* enregistres pour HMVL */


#define SZ_HMVL_BI    4                /* taille des donnees individuelles */
#define SZ_HMVL_MI    3                /* taille des donnees individuelles */


#define SZ_QT_V        3
#define SZ_QT_B        3
#define SZ_QT_H        5
#define SZ_QT_J        6

#define SZ_TO          2
#define SZ_VT          3
#define SZ_ST          1
#define SZ_IT          4

#define NB_DON_V      15
#define NB_DON_B      10
#define NB_DON_H      12
#define NB_DON_J      12

/*********************************************************************/
/*       Pour les alarmes sur cartes MSO                             */
/*********************************************************************/
#define NB_ALARME  48
#define ALARME_NULLE  0
#define ALARME_EDF    1
#define ALARME_DET    2
#define ALARME_TRAFIC 3


#define MAX_DUREE_TOR 100

/*********************************************************************/
/*       Pour la configuration CFMP                                   */
/*********************************************************************/
#define MAX_FORMAT_CFMF   5
#define MAX_NATURE_CFMF   5
#define MAX_ID_CFMF       5

#define MAX_TRACE_MAINT      100
#define MAX_TRACE_AI         30

#define LG_TRACE_AI          120


#define TRACE_TR_ALL         0xFF
#define TRACE_TR_DET         0x02
#define TRACE_TR_VILT        0x04
#define TRACE_TR_BRUTE       0x08
#define TRACE_TR_INT108      0x10
#define TRACE_TR_VALEURS     0x40
#define TRACE_TR_SILHOUETTE 0x20

/* pour le controle des cartes */
#define MAX_CRT_MGP_102      1
#define MAX_CRT_TS_116       1
#define MAX_CRT_TC_116       1
#define MAX_CRT_MRH_132      5
#define MAX_CRT_COM_104      4

#define AD_MGP_102           0xE00000
#define AD_TS_116            0xE10000
#define AD_TC_116            0xE10000
#define AD_MRH_132           0xE00800
#define AD_COM_104           0xE10000

/* pour les type de detecteur */
#define DETECTEUR_MDB185        1      /* detecteur MDB185 dans le RACK      */
#define DETECTEUR_CDE4XCS_COM185 2     /* detecteur CDE4XCS  deporte COM185 */
#define DETECTEUR_CDE4XCS_CPU332 3     /* detecteur CDE4XCS  deporte CPU332 */
#define DETECTEUR_VIDEO_VIP9    10
#define DETECTEUR_VIDEO_VIP22   11
#define DETECTEUR_VIDEO_VIP22_9 12
#define DETECTEUR_PIEZO_INT108  20

#define MAX_DONNEE_SILHOUETTE  20

#define MAX_SCENARIO       3           /* nombre max de scenario configure     */
#define TAILLE_SCENARIO    43690       /* 48 Koctets pour une configuration */

#define MAX_VIP            4
#define MAX_TRC_SILHOUETTE 100

#define TAILLE_CFG_VIP     300

#define VIP_9              9
#define VIP_22             22
#define VIP_22_9           23

#define TYPE_DETECTEUR_BCL_SIAT   1
#define TYPE_DETECTEUR_VIDEO      2
#define TYPE_DETECTEUR_SOFRELA    3

                                                                                /* ---------------------------------
                                                                                 * DEFINITION DES CONSTANTES SILHOUETTE
                                                                                 * --------------------------------     */
#define SIL_SEUIL_LONGUEUR	600
#define SIL_SEUIL_LG_CAM_MAX	1000
#define SIL_SEUIL_LG_BUS_MIN	900
#define SIL_SEUIL_LG_BUS_MAX	1500
#define SIL_DD				300
#define SIL_LD				100
#define SIL_DD_SILH			200
#define SIL_LD_SILH			30
#define SIL_SEUIL_VAM1		25
#define SIL_SEUIL_VAM2		35
#define SIL_SEUIL_NB_EXTR1	4
#define SIL_SEUIL_NB_EXTR2	8

/**********************************************************/
/*                                                        */
/*         definition des constantes globales             */
/*                                                        */
/**********************************************************/

#if CPU332
#define  MAX_PROC      4
#else
#define  MAX_PROC      4
#endif

/* --------------------------------
 * E_formatDate
 * ==========
 * Enumeration des formats de date
 * disponibles.
 * --------------------------------	*/

typedef enum E_formatDate
{
  E_DH0,
  E_DH1,
  E_DH2,
  E_DH3,
  E_DH4,
  E_HR0,
  E_HR1,
  E_HR2,
  E_HR3,
  E_HR4,
  E_HR5,
  E_HR6,
  E_DT0,
  E_DT1
} E_formatDate;

/* --------------------------------
 * T_ficListe
 * ==========
 * La liste des fichiers est
 * maintenant une liste enumeree
 * bornee par le define MAXFIC
 * qui permettra de dimensionner
 * les boucles et les tableaux.
 * --------------------------------	*/
typedef enum T_ficListe
{
  F_IC,
  F_IM,
  F_IP,
  F_IQ,
  F_IR,
  F_IA,
  MAXFIC
} T_ficListe;

/* --------------------------------
 * T_ficInfo
 * =========
 * Il s'agit de preciser dans la
 * structure les informations necessaires
 * e la remise en conf standard ou
 * minimale.
 * Le mot clef est utilise pour l'analyse
 * de la commande de configuration
 * des fichiers.
 * --------------------------------	*/

typedef struct T_ficInfo
{
  char *motClef;
  int min;
  int standard;
} T_ficInfo;

#define F_NC	99
#define F_TN   100
#define	F_MAX MAXFIC

/* ********************************
 * DEFINITION DES VARIABLES EXTERNES
 * ********************************	*/

extern const T_ficInfo ficInfo[MAXFIC];

/* ********************************
 * FIN DE define.h
 * ********************************	*/

#endif
