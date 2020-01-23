#ifndef INCLUDE_XDG_DEF
#define INCLUDE_XDG_DEF

#include "config.h"

#define   AUCUN_PROTOCOLE 0            /* aucun protocole                         */
#define   BSC_PROT        1            /* protocole BSC maitre/esclave            */
#define   LMP_PROT        2            /* protocole carte lampe                   */
#define   DET_PROT        2            /* protocole carte lampe                   */
#define   TEDI_PROT       3            /* protocole TEDI esclave                  */
#define   MASTER_PROT     4            /* protocole maitre                        */
#define   VIP_PROT        5            /* protocole maitre vers module VIP        */
#define   LUM_PROT  6            	   /* protocole Luminance        */
#define   TEMPO_LAS_MAITRE 960         /* tempo d'inhibition de la liaison maitre */

#define VT_HS        0xff              /* inhibition de la commande VT                   */

#define TEDI_BLK     256               /* taille maximum d'un bloc de message  */
#define MAXBLK   249                   /* taille maximum d'un bloc de message hors */
/* pour les compte-rendu d'erreur */
#define CPTRD_OK   0
#define CPTRD_SYNTAXE 1
#define CPTRD_PARAM  2
#define CPTRD_PROTOCOLE 3
#define CPTRD_INTERDIT 4
#define CPTRD_VALEUR_PARAM 6
#define MAXBLK   249                   /* taille maximum d'un bloc de message hors           */

#define TEMPO_MESS 16
#ifndef TEMPO_MESS
#define TEMPO_MESS    200
#endif
/***** definition du temps de valididite de certaines commandes              */

#define XOFF_EMIS    0x01
#define XON_EMIS     0xfe
#define XOFF_RECU    0x02
#define XON_RECU     0xfd
#define MODE_AUCUN        0
#define MODE_PROTEGE      1
#define MODE_TEST         2
#define MODE_TERMINAL     3

#define EMISSION_RECEPTION  1
#define EMISSION_SEULE      2

#define RECEPTION_SEULE     3
#define EMISSION_SYNC       4
#define MAX_BUFF_LAS_REC 300
#define MAX_BUFF_LAS_EM  300

/* nombre max de carte MSR116 par rack */
#define NB_CRT_MRH132      5
#define NB_CRT_MSR116      4

/* nombre max de carte MEO116 par rack */

/* nombre max de carte MGP102 par rack */
#define NB_CRT_MGP102      4

#define NB_CRT_MEO116      NB_CRT_MRH132
#define NB_CRT_MSO116      NB_CRT_MRH132
#define NB_CRT_LON100      4
#define MAX_AD_LON100      127
#define NB_PSEUDO_LON		2
/* pour les adresses de carte */
#define AD_MSR116     FDP_BASE_16+30000
#define AD_COM104     FDP_BASE_16+0x18000
#define AD_MSO116     FDP_BASE_16+0x20000
#define AD_MEO116     FDP_BASE_16+0x20000
#define AD_MGP102     FDP_BASE_16+0x10000
#define AD_LON100     FDP_BASE
#define TAILLE_MSR116 0x800
#define TAILLE_MSO116 0x800
#define TAILLE_MEO116 0x800
#define TAILLE_MGP102 0x800
#define TAILLE_LON100 0x4000
#define LG_SYMB             8
#define NB_CFES             50
#define LG_ID               9
#define NB_CFMP             20

/* pour les cartes echelon */
#define NB_CL501         100           /* nombre max de carte CL501      */
#define NB_COL501        10            /* nombre max de carte COL501     */
#define NB_CD501         30            /* nombre max de carte CD501      */
#define NB_CD503         5             /* nombre max de carte CD503      */
#define NB_ES502         10            /* nombre max de carte ES502      */
#define NB_LT501         3             /* nombre max de carte LT501/TP501 */
/* pour les cartes cellule */
#define TYPE_MGP102  1
#define TYPE_MEO116  2
#define TYPE_MSR116  3
#define TYPE_LT501   4
#define TYPE_ES502   5

#endif
