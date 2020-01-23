/*
 * pip_def.h
 * ===================
 *
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: pip_def.h,v $
 * Revision 1.2  2018/06/04 08:38:42  xg
 * Passage en version V122
 *
 * Revision 1.3  2008/10/03 16:21:45  xag
 * *** empty log message ***
 *
 * Revision 1.2  2008/07/04 17:05:59  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:29:15  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

#ifndef PIP_DEF_H_
#define PIP_DEF_H_

#define ERREUR_MAJEURE       0xFF08
#define ERREUR_TRANS         0x0100
#define ERREUR_SABORDE       0x8000
#define ERREUR_HS_ALLUM1     0x0400
#define ERREUR_HS_ALLUM2     0x0800
#define ERREUR_HS_ALLUM3     0x1000
#define ERREUR_HS_ALLUM      0x1C00
#define ERREUR_HS_ETEINT1    0x0010
#define ERREUR_HS_ETEINT2    0x0020
#define ERREUR_HS_ETEINT3    0x0200
#define ERREUR_HS_DIODES1    0x0040
#define ERREUR_HS_OCC1       0x0080
#define ERREUR_HS_DIODES2    0x2000
#define ERREUR_HS_OCC2       0x4000

/*DOC-------------------------------------------------------------------------
/                                                                            /
/          DEFINITION DES CONSTANTES COMMUNES A PLUSIEURS TACHES             /
/                                                                            /
-------------------------------------------------------------------------DOC*/

#define MAX_COL_CAR    5               /* nombre de colonne par caractere   */
#define MAX_ES_CFTP    50              /* nombre max de param pour cftp ES */
#define MAX_FI         250             /* nombre max de parametre pour fi   */
#define MAX_TX         15              /* nombre max de parametre pour tx   */
#define TYPE_LMP_SEC   0               /* lampe secourues                   */
#define TYPE_LMP_NSEC  1               /* lampe non secourues               */
#define TYPE_LMP_NCTRL 2               /* lampe non controlees              */

/* Definitions pour les actions */

#define PIP_ACT_CAR  0x01
#define PIP_ACT_CLIG 0x02
#define PIP_ACT_LUM  0x04
#define PIP_ACT_VAL  0x08
#define PIP_ACT_TEST 0x10


/* Definitions pour le clignotement et l'altenance.... */

#define PIP_AUCUN 0				/* Pas de clignotement */
#define PIP_DEFAUT 1			/* Valeur par defaut	*/
#define PIP_ACTIF 2				/* Clignotement actif 	*/
#define PIP_NB_VAL_CLIG 2
#define PIP_NB_VAL_ALT 4

/* ----------------------------
 * Etats d'affichage alternatifs
 * ----------------------------	*/

/* Definition des bits d'etat PIP */

#define PIP_VEILLE		0x01
#define PIP_COM			0x02
#define PIP_TEMP		0x04
#define PIP_DELESTE		0x08
#define PIP_TEST_BP		0x10
#define PIP_TEST_TPE	0x20

/* Definition des codes commandes */

#define PIP_AFFICHAGE 	0
#define PIP_DELESTAGE 	1
#define PIP_NEUTRE 		3
#define PIP_TEMPERATURE 4
#define PIP_VEILLE_SANS_COM 	5
#define PIP_VEILLE_AVEC_COM 	6
#define PIP_TEST_BP1	7
#define PIP_TEST_BP2	8
#define PIP_TEST_BP3	9
#define PIP_TEST_BP4	10
#define PIP_TEST_TPE1	11
#define PIP_TEST_TPE2	12
#define PIP_TEST_TPE3	13
#define PIP_TEST_DIODES 14
#define PIP_TEST_TPE4	15
#define PIP_TEST_TPE5	16
#define PIP_TEST_TPE6	17

#define NB_MODULE         10           /* nombre de module                 */
#define NB_CAISSON        10           /* nombre de caisson                */
#define NB_EQUIV		  50
#define NB_LITTERAL		  20
#define NB_DEC            70           /* nombre de decor                  */
#define NB_LAMPE          132          /* nombre de lampe                  */
#define NB_PICTO_OCCULTEUR 10          /* nombre d'occulteur affectes e des
										  picto */
#define NB_BITMAP_OCC     20           /* nombre de bitmap occulteur */
#define NB_MAT_GRAPH      25
#define NB_TEXTE_PREPRO   100
#define NB_CAR_PREPRO     250
#define NB_DEC_GRAPH      5
#define NB_CARTE_GRAPH    50
#define NB_PRISME         5
#define NB_CELLULE        2
#define NB_SABORDAGE      3

#define PIP_EQUIV_AFF	1
#define PIP_EQUIV_EC	2
#define PIP_SYMB_LITTERAL 1
#define PIP_SYMB_NUMERIQUE 2

#define NB_PORTIQUE	NB_CAISSON
#define NB_RAMPE	NB_CAISSON
#define NB_PANNEAU  NB_CAISSON

#define TYPE_ALPHA     	0x01   /* pour les caissons alpha, c'est a dire les lignes de caracteres */
#define TYPE_DELEST     0x10  /* pour les caissons alpha lampe */

#define MAX_AD_CMD169     125    /* l'adresse max des cartes de commande */
#define ADRESSE_LMP_JOCKER 128   /* adresse passe partout pour les modules e diodes */
#define NB_MATRICE_LAS    32     /* nombre max d'adresse par liaison     */

#define NB_FACE_PRISME    3
#define LG_FACE_PRISME    10


#define MAX_CAR_CAISSON   24           /* nombre max de caractere par ligne    */

#define TYPE_NUMERIQUE    	1            /* type de commande P */
#define TYPE_SYMBOLIQUE   	2
#define TYPE_LITTERAL     	3
#define TYPE_MACRO        	4
#define TYPE_STATUS       	5
#define TYPE_NREP         	6
#define TYPE_ETEINT    	  	7
#define TYPE_NEUTRE     	8
#define TYPE_AUTOMATIQUE    9

/* La valeur admise sur luxembourg a ete
 * modifiee pour economiser un peu de
 * place memoie : #define MAX_ALTERNANCE    3 */
#define MAX_ALTERNANCE    2
#define CLIG_DEF          10
#define HYSTERESIS_LUM    3
#define BUFF_MESS_P       128
#define MAX_CAR_POLICE    256
#define ACTIVATION_RIEN     0
#define ACTIVATION_CAISSON  1
#define ACTIVATION_MODULE   2
#define ACTIVATION_PIP      3
#define NB_SEUIL_CELLULE   10

#define TEMPO_STATUS        5          /* tempo d'interrogation en seconde */

/* pour la luminosite */
#define LUM_NUIT_LP            50
#define LUM_JOUR_LP            75
#define LUM_SRB_LP             100
#define LUM_NUIT_D            70
#define LUM_JOUR_D            150
#define LUM_SRB_D             250
#define LUM_AUTO            10
#define DEF_OK              0          /* pas de defaut             */
#define DEF_SECOURS         2          /* lampe en secours          */
#define DEF_HS_ETEINTE      3          /* lampe HS ETEINTE          */
#define DEF_HS_ALLUMEE      4          /* lampe HS ALLUMEE          */
#define DEF_SABORDE         5          /* panneau saborde           */
#define DEF_MAT             6          /* materiel                  */
#define DEF_CFG             7          /* absence de configuration  */
#define DEF_PWR             8          /* defaut d'energie          */
#define DEF_PLAT            9          /* defaut platine            */
#define DEF_SYNCHRO         10         /* absence alimentation lampes */

#define DECOR_SOMBRE        0          /* pour la mise au neutre du panneau */

#define TYPE_HR             1
#define TYPE_DT0            2
#define TYPE_DT1            3
#define TYPE_TP             4

#define P_OK                0
#define P_NK                1
#define P_NL                2

#define DEFAUT_OK            0x01
#define DEFAUT_MIN           0x02
#define DEFAUT_MAJ           0x04

#define ST_TR_DEF_MIN    0x20
#define ST_TR_DEF_MAJ    0x08

#define MAX_TRACE            100
#define DIAL_OK              0x0001
#define DIAL_OCCUPE          0x8000
#define DIAL_DEM_ETAT        0x4000
#define DIAL_CRT_CONFIG      0x0002
#define DIAL_CRT_INIT        0x0004
#define DIAL_CRT_RELEC       0x0008
#define DIAL_CRT_RES_TEST    0x0010
#define DIAL_CRT_NOIR        0x0020

#define TEMPO_DEF_MIN        20        /* duree max d'un defaut mineur         */
#define CPT_RELANCE_DEF_MIN  3         /* nombre de relance par coupure d'alim */
#define ATTENTE_RELANCE      16        /* tempo d'attente relance fin de defaut */
#define TEMPO_ATTENTE_SECTEUR 5        /* en seconde */

#define MAX_PAR_TST          10        /* nombre de parametre cmd TST */

/* le mode de fonctionnement */


/* ********************************
 * FIN DE pip_def.h
 * ********************************	*/

#endif /* PIP_DEF_H_ */
