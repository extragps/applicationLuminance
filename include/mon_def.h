/**********************************************************/
/*                                                                                         */
/*         definitions des equivalences generales         */
/*                                                                                         */
/**********************************************************/

#include "mon_conf.h"

#define   NB_PORT_TEDI    2
#define   NB_LAS_PIP      2
#define   MIN_LAS_PIP     0
#define 	NBPORT          6
#define 	PORT_SERIE		0
#define 	PORT_SERIE_2	1
#define 	PORT_PASS 		1
#define 	PORT_FIP		2
#define 	PORT_IP_LCR		3
#define 	PORT_IP_LCR_2	4
#define   	PORT_LUMINANCE  5


#define   BINAIRE         1
#define   TRUE            1
#define   FALSE           0
#define   NUL             0
#define   NIL        0xffff

#define   NILL           -1

#ifdef PMV
#define   EMISSION       0x06          /* enable  Tx , enable  Rx              */
#else
#define   EMISSION       0x05          /* enable  Tx , enable  Rx              */
#endif
#define   RECEPTION      0x09          /* disable Tx , enable  Rx               */
#define   HALF_DUPLEX     0
#define   FULL_DUPLEX     1
#define   PAIRE           0
#define   IMPAIRE         1
#define   SANSPARITE      2
#define   NBCAR_PRINTF    500

#define   SANS_ERREUR     '0'          /* pas d'erreur constatee                */
#define   E_OK            '0'
#define   E_SYNTAXE       '1'          /* erreur de syntaxe                     */
#define   E_OVERRUN       '2'          /* erreur d'ecrasement de donnees        */
#define   E_FRAMING       '3'          /* erreur de trame                       */
#define   E_PARITE        '6'          /* erreur de parite                      */

#define   EVE_EMISSION    0            /* fin d'emission                        */
#define   EVE_RECEPTION   1            /* reception d'un caractere              */
#define   EVE_ERREUR      2            /* reception en erreur                   */
#define   EVE_TIME        3            /* time_out                              */

#define   CR_OK           0
#define   CR_NOK          1
#define   CR_NIL          2
#define   CR_EDF          3            /* absence energie pour les detecteurs */
#define   CR_NUL          3

#define   N_AFF           0xFF
#define   NN_AFF          0x3FFF

#define   MAX_FIFO      300            /* taille max de la FIFO caractere       */
#define   MAX_BUF_EM    300            /* taille max du buffer d'emission       */
#define   MAX_BUF_RE    300            /* taille max du buffer de reception     */
#define   MAX_OPE       250            /* taille du buffer de dialogue          */
#define   MAX_TRACE_DEM   100
#define   MAX_OPE       250            /* taille du buffer de dialogue          */
#define   VAL_BUSR     0x3B2           /* validation du bus rapide sur TSTBUS      */
#define   DEVAL_BUSR   0x3B0           /* devalidation du bus rapide sur TSTBUS    */

#define   SR0       0                  /* valeur du SR pour le demarrage des taches      */
#define   FORMAT0   0                  /* format de la pile pour le demarrage des taches */

#define   DOGEX     0x0000             /* expiration du chien de garde    */
#define   DOGREL    0x00A0             /* relance du chien de garde : 10s */
#define   DOGAN     0x0fff             /* chien de garde annule           */

#define   SUSP_NULLE    0              /* pas de suspension  pour RQSUSP */
#define   SUSP_INFINIE -1              /* suspension infinie pour RQSUSP */
#define   SUSP_SECONDE 16              /* suspension 1 seconde pour RQSUSP */

#define   TYCBAL     0                 /* type de requete pour RQMBX   : RQCBAL    */
#define   TYEMES     1                 /* type de requete pour RQMBX   : RQEMES    */
#define   TYRMES     2                 /* type de requete pour RQMBX   : RQRMES    */
#define   TYESIG     3                 /* type de requete pour RQMBX   : RQRMES    */
#define   TYZMES     4                 /* type de requete pour RQMBX   : RQZMES    */
#define   TYFIN      5                 /* type de requete pour RQMBX   : RQFIN    */

#define   TYIN       3                 /* type de requete pour RQIOP   : RQIN      */
#define   TYOUT      4                 /* type de requete pour RQIOP   : RQOUT     */

#define   TYIN_16    23                /* type de requete pour RQIOP   : RQIN 16  */
#define   TYOUT_16   24                /* type de requete pour RQIOP   : RQOUT 16 */

#define   TYLCAL     5                 /* type de requete pour RQCAL   : RQLCAL    */
#define   TYECAL     6                 /* type de requete pour RQCAL   : RQECAL    */
#define   TYICAL     7                 /* type de requete pour RQCAL   : init      */
#define   TYTCAL     8                 /* type de requete pour RQCAL   : it        */
#define   TYPRECAL   50                /* type de requete pour RQCAL   : it        */
#define   TYSYNCAL   51                /* type de requete pour RQCAL   : it        */

#define   TYPUART    9                 /* type de requete pour QUART_INT  : Prog.Uart */
#define   TYEUART    10                /* type de requete pour QUART_INT  : Emission  */
#define   TYRUART    11                /* type de requete pour QUART_INT  : Reception */
#define   TYRTS      13                /* type de requete pour QUART_INT  : Reception */
#define   TYECHO     14                /* type de requete pour QUART_INT  : echo      */

#define   SCI_PF    0x0000             /* SCI status bit : parity error flag                   */
#define   SCI_FE    0x0002             /* SCI status bit : framing error flag                  */
#define   SCI_NF    0x0004             /* SCI status bit : noise error flag                    */
#define   SCI_OR    0x0008             /* SCI status bit : overrun error flag                  */
#define   SCI_IDLE  0x0010             /* SCI status bit : idle-line detected flag             */
#define   SCI_RAF   0x0020             /* SCI status bit : receiver active flag                */
#define   SCI_RDRF  0x0040             /* SCI status bit : receiver data register full flag    */
#define   SCI_TC    0x0080             /* SCI status bit : transmit complete flag              */
#define   SCI_TDRE  0x0100             /* SCI status bit : transmit data register empty flag   */
#define   SCI_NOERR 0x000F             /* SCI status bit : PF + FE + NF + OR                   */
#define   SCI_TIE   0x0080             /* SCI control bit : transmit interrupt enable          */

#define   PAD      0
#define   SOH      0x01
#define   STX      0x02                /* caractere de debut de bloc                         */
#define   ETX      0x03                /* caractere de fin de message                        */
#define   EOT      0x04                /* caractere de synchronisation                       */
#define   ENQ      0x05                /* caractere de demande de transmission               */
#define   ACK      0x06                /* caractere d'accuse de reception                    */
#define   POL      0x07
#define   SEL      0x08
#define   BS       0x08                /* espace inverse                                     */
#define   LF       0x0A                /* saut de ligne                                      */
#define   CR       0x0D                /* caractere de retour chariot                        */
#define   DLE      0x10                /* echappement de transmission                        */
#define   XON      0x11                /* arret provisoire d'emission                        */
#define   XOF      0x13                /* reprise d'emission                                 */
#define   NACK     0x15                /* accuse de reception negatif                        */
#define   SYNC     0x16                /* synchro                                            */
#define   ETB      0x17                /* fin de bloc                                        */
#define   C21      0x21                /* fin de message                                     */
#define   C24      0x24                /* $                                                  */
#define   C2B      0x2B                /* fin de bloc                                        */
#define   C2D      0x2D                /* demande de transmission                            */
#define   C3F      0x3F                /* accuse de reception negatif                        */
#define   DEL      0x7F


#define MAX_TP_GROUPE 14
#define TP_HS      -1                  /* la temporisation n'a pas ete initialisee */
#define TP_FINI     0                  /* la temporisation vient de se terminer    */
/***** definition des indices de lecture dans la table des cases tempo       */
#define TP_QUART           0           /* case tempo reserve au quart : 0..3       */
#define AI_0               NBPORT      /* case tempo commande AI du module LC0               */
#define RD_0               (AI_0+NBPORT)        /* case tempo commande RD du module LC0               */
#define TR_0        (RD_0+NBPORT)  /* case tempo                               */
#define XF_0        (TR_0+NBPORT)      /* case tempo                               */
#define ID_0        (XF_0+NBPORT)      /* case tempo                               */
#define VT_0        (ID_0+NBPORT)      /* case tempo commande VT du module LC0               */
#define AC_ACQ             (VT_0+NBPORT)        /* case tempo d'acquisition                          */
#define TP_COM             (AC_ACQ+1)
#define TP_COM185          (TP_COM+1)  /* case tempo pour time-out com185      */
#define TP_ALERTE          (TP_COM185+1)
#define TP_SEC_AL          (TP_ALERTE+1)
#define TP_VALID_CONFIG    (TP_SEC_AL+1)
#define TP_ACTIV_CONFIG    (TP_SEC_AL+1)
#define TP_ATTENTE_SIMUL   (TP_ACTIV_CONFIG+1)
#define TP_ATTENTE_LAS     (TP_ATTENTE_SIMUL+1)
#define TP_MODE_BOOT       (TP_ATTENTE_LAS+1)
#ifdef PMV
#define TP_SECT     (TP_MODE_BOOT)
#define TP_TEST_DIODES (TP_SECT+1)
#define TP_125MS    (TP_TEST_DIODES+1)
#define TP_TAC_MST_SLV (TP_125MS+1)
#define TP_TAC_LMP     (TP_TAC_MST_SLV+1)
#define TP_CONTROLE_LMP    (TP_TAC_LMP+1)
#define TP_MIN_GROUPE		(TP_CONTROLE_LMP+1)
#define TP_MAX		 (TP_MIN_GROUPE+MAX_TP_GROUPE+1)
#else
#define TP_MAX             (TP_MODE_BOOT+1)
#endif
#define AC_0    				(TP_MAX+1)
#define TV_0    				(AC_0 + NBCAPTEUR)      /* case tempo voie x NBCAPTEUR                   */
#define NB_TP_MAX    		(TV_0 + NBCAPTEUR)
#define NB_TOP_IT_DET	8
                                                                                /** Nombre de tics entre chaque
										 * synchronisation des detecteurs VCMT
										 * (e priori, c'est 10 secondes pour
										 * 160).	*/
#define NB_TOP_LECT_CO	160
                                                                                /* on ne saborde pas pendant les trois
                                                                                 * minutes qui suivent une reprise
                                                                                 * secteur*/
#define TEMPO_SECTEUR  (8*180)
#define TEMPO_XOFF     (8*30)
/* pour les masque de tache */
#define MASQ_TAC1  0x0001
#define MASQ_TAC2  0x0002
#define MASQ_TAC3  0x0004
#define MASQ_TAC4  0x0008
#define MASQ_TAC5  0x0010
#define MASQ_TAC6  0x0020
#define MASQ_TAC7  0x0040
                                                                                /* HORLOGE */
#define SZ_DATE        8
#define H_SEC  0
#define H_MIN  1
#define H_HOU  2
#define H_DAY  3
#define H_DAT  4
#define H_MON  5
#define H_YEA  6
#define H_INIT 7

#define TEMPO_CONFIG 16

/*  DETECTEUR */
#define  CODE_LECT_CO  0x10
#define  CODE_LECT_VH  0x20
#define  CODE_LECT_VA  0x30
#define  CODE_ECRI_IN  0x40
#define  CODE_LECT_SYNC  0x50
#define  CODE_TELE     0x50
#define  CODE_ECRI_SE  0x60
#define  CODE_LECT_PIC 	 		0x70
#define  CODE_LECT_VERS  		0x80
#define  CODE_LECT_CONF  		0x90
#define  CODE_LECT_OFFSET  		0xA0
#define  CODE_LECT_CONF_VCMT  	0x70
#define  CODE_ECRI_SILH  0xA0
#define  CODE_SYNCHRO    0xF1
