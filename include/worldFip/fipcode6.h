/**********************************************************
@(#) All rights reserved (c) 1999
@(#) ALSTOM (Paris, France)
@(#) This computer program may not be used, copied,
@(#) distributed, corrected, modified, transmitted or
@(#) assigned without ALSTOM`s prior written authorization
***********************************************************/
 
#ifndef __fipcode
#define __fipcode
 
#include "fcod_opt.h"

#if (WITH_ST1)
#define Nb_Page 65
#endif
#if (WITH_ST2)
#define Nb_Page 103
#endif
#if (WITH_ST3)
#define Nb_Page 156
#endif
#if (WITH_ST4)
#define Nb_Page 323
#endif

#define Nb_Page_Max 323

typedef struct {
	unsigned int base;
	unsigned char code[128];
} Fip_code_Page;

typedef struct {
	unsigned int nb;
	const Fip_code_Page * adr[Nb_Page];
} Fip_code_Programme_Type;

extern unsigned short const Tcheksum[Nb_Page_Max];

/* Configuration Systeme */


#define FIP_START_PROG	0x0600	/* Debut adresse programme */
#define FIP_IDENT_LOG	0x0038	/* Adresse Fiche d'identification du LOGICIEL */

#if (WITH_ST1)
#define FIP_ADR_USER	0x2460	/* Debut zone utilisateur */
#endif
#if (WITH_ST2)
#define FIP_ADR_USER	0x2460	/* Debut zone utilisateur */
#endif
#if (WITH_ST3)
#define FIP_ADR_USER	0x3200	/* Debut zone utilisateur */
#endif
#if (WITH_ST4)
#define FIP_ADR_USER	0x6000	/* Debut zone utilisateur */
#endif

#define FIP_CONF_RESEAU	0x2000	/* Debut configuration reseau */
#define FIP_BASE_0	0x2000	/* Ptr debut table descripteur trames */



#define STATION_1	0x0001	/* Equipement de type "Petite Station" */
#define STATION_2	0x0002	/* Equipement de type "Moyenne Station" */
#define STATION_3	0x0003	/* Equipement type "Station Complete sans BA" */
#define STATION_4	0x0004	/* Equipement type "Station Complete" */

#define OPTION_1	0x0000	/* Fonctionnement acces classique memoire */
#define OPTION_2	0x0001	/* Fonctionnement acces libres memoire */

#define TYPE_FIP	0x0000	/* CRC et trame format NFC */
#define TYPE_WORLDFIP	0x0001	/* CRC et trame format IEC */
#define TYPE_SLOWFIP	0x0002	/* FIP + Compatibilite FIPCODE V4.3 / V5.3 */

#define EOC_NIVEAU	0x0000	/* EOC utilise en mode NIVEAU */
#define EOC_PULSE	0x0001	/* EOC utilise en mode PULSE */

/*#define RDD_USER	0x0000	 Redondance medium assure par l'utilisateur */
/*#define RDD_UNUSED	0x0000	 Pas de redondance (carte mono medium)*/
/*#define RDD_FIP	0x0002	 Rdd FIPCODE - TESTP Bi-medium pour CEGELEC */
/*#define RDD_FIPIO_2	0x0006	 Rdd FIPCODE - TESTP Bi-medium pour FIPIO */
/*#define RDD_FIPIO_1	0x0005	 Rdd FIPCODE - TESTP Mono-medium pour FIPIO */

#define RDD_USER		0x0000	/* Redondance medium assure par l'utilisateur */

#define RDD_FIP_MONO		0x0001	/* Rdd FIPCODE - TESTP Mono-medium pour FIPIO */
#define RDD_FIP_BIMED		0x0000	/* si Rdd FIPCODE - TESTP Bi-medium */

#define RDD_AVEC_FIELDUAL	0x0002	/* Rdd FIPCODE - composant FIELDUAL present */
#define RDD_SANS_FIELDUAL	0x0000	/* si Rdd FIPCODE - Pas de redondance (carte mono medium)*/

#define RDD_LISTE_NON_PRETE	0x0004	/* Rdd FIPCODE - emission Liste non Prete */
#define RDD_LISTE_PRETE		0x0000	/* si Rdd FIPCODE - emission Liste Prete */


#define VALID_CHANNEL	0x1	/* Validation d'une voie de communication */
#define INVALID_CHANNEL	0x2	/* Invalidation d'une voie de communication */

#define CHANNEL_VALID	0x1	/* Voie Valide */
#define CHANNEL_INVALID	0x0	/* Voie Invalide */

#define MODE_TXCK	0x0000	/* Horloge transmission interne (TXCK) */
#define MODE_EXTERNE	0x0001	/* Horloge transmission generee en externe */


#define REQUIRED	1	/* */
#define NOT_REQUIRED	0	/* */

#define NULL_WORD	0x0000	/* */
#define NULL_BYTE	0x00	/* */
#define NULL_BIT	0	/* */


/* Variables de communication */

#define FIP_FIFO_DDE_URG	0x21a7	/* FIFO dde urgente R/W variable (option2) */
#define FIP_R_FIFO_DDE_URG	0x21a7	/* Ptr remplissage FIFO dde urgente R/W variable (option2) */
#define FIP_V_FIFO_DDE_URG	0x21a7	/* Ptr vidage FIFO dde urgente R/W variable (option2) */

#define FIP_FIFO_DDE_NOR	0x00a0	/* Adresse FIFO dde normale R/W variable (option2) */
#define FIP_R_FIFO_DDE_NOR	0x21a6	/* Ptr remplissage FIFO dde normale R/W variable (option2) */
#define FIP_V_FIFO_DDE_NOR	0x21a5	/* Ptr vidage FIFO dde normale R/W variable (option2) */

#define STATUS_TRUE	1	/* */
#define STATUS_FALSE	0	/* */

#define MPS_PDU		0x40	/* Type PDU = 40H */
#define SM_MPS_PDU	0x50	/* Type PDU = 50H */

#define CONSUMED	0x02	/* Variable consommee */
#define CONSUMED_PS	0x0a	/* Variable consommee + status promptitude */
#define PRODUCED	0x01	/* Variable produite */

#define CONS		0x0000	/* Variable consommee */
#define CONS_VLG	0x0004	/* Variable consommee + Verification PDU et LG */

#define PROD		0x0040	/* Variable produite */
#define PROD_TIME	0x004c	/* Variable produite de type "heure" */
#define PROD_SP		0x0048	/* Variable produite avec status */
#define PROD_SP_RQ	0x0058	/* Variable produite avec status + RQa */
#define PROD_SP_MSG	0x0068	/* Variable produite avec status + MSGa */
#define PROD_SP_RQ_MSG	0x0078	/* Variable produite avec status + RQa + MSGa */
#define PROD_RQ		0x0050	/* Variable produite + RQa */
#define PROD_MSG	0x0060	/* Variable produite + MSGa */
#define PROD_RQ_MSG	0x0070	/* Variable produite + RQa + MSGa */

#define PDU_LG_1400	0x5007	/* PDU et LG pour variable de presence consommee */
#define PDU_LG_14xx	0x5005	/* PDU et LG pour variable de presence produite */
#define PDU_LG_11xx	0x500f	/* PDU et LG pour variable de rapport produite */
#define PDU_LG_P9002	0x5044	/* PDU et LG pour variable liste des presents produite */
#define PDU_LG_C9002	0x5046	/* PDU et LG pour variable liste des presents consommee */

#define ID_10xx		0x1000 + FIP_ABONNE	/* Identifieur pour variable d'identification abonne xx */
#define ID_11xx		0x1100 + FIP_ABONNE	/* Identifieur pour variable de rapport abonne xx */
#define ID_14xx		0x1400 + FIP_ABONNE	/* Identifieur pour variable de presence abonne xx */
#define ID_1400		0x1400			/* Identifieur pour variable de presence abonne 0 */
#define ID_9002		0x9002			/* Identifieur pour variable liste des presents */

#define DESC_SYNCHRO	0x0000 		/* Ptr vers descripteur de variable de synchronisation */
#define DESC_MSG	0x0010 		/* Ptr vers descripteur pour messagerie (production) */
#define DESC_PRESENCE	0x0040 		/* Ptr vers descripteur variable de presence (produite) */
#define DESC_PRESENT	0x0050 		/* Ptr vers descripteur variable liste des presents (consommee) */
#define DESC_RAPPORT	0x0070 		/* Ptr vers descripteur variable de rapport (produite) */
#define DESC_IDENT	0x0080 		/* Ptr vers descripteur variable d'identification (produite) */

#define FIFO_URGENT	0x01 		/* FIFO stockage demandes aperiodiques urgentes */
#define FIFO_NORMAL	0x00 		/* FIFO stockage demandes aperiodiques normales */


/* Messagerie */

#define MSG_TAB_SEGMENT	0x0100		/* Table des segments messagerie */
#define MSG_TAB_PURGE	0x2280		/* Table des cr purge message (option 2) */

#define SEGMENT_IGORED	0x0000		/* Segment messagerie non reconnu */
#define SEGMENT_CONFIG1	0x0001		/* Segment messagerie accepte quelque soit ID destinataire (BRIDGE) */
#define SEGMENT_CONFIG2	0x0002		/* Segment messagerie accepte si ID destinataire reconnu */

#define MSG_SDN_ACK		0x0	/* Message emis avec service messagerie non acquittee */
#define MSG_SDN_ERROR		0x4	/* Message emis avec erreur avec service messagerie non acquittee */
#define MSG_NO_ACK		0x2	/* Pas d'acquit avec service messagerie acquittee */
#define MSG_ACK_POS		0x1	/* Acquit positif avec service messagerie acquittee */
#define MSG_ACK_POS_REP		0x3	/* Acquit positif apres repetition avec service messagerie acquittee */
#define MSG_ACK_NEG		0x5	/* Acquit negatif avec service messagerie acquittee */
#define MSG_ACK_NEG_REP		0x7	/* Acquit negatif apres repetition avec service messagerie acquittee */


#define MSG_CHANNEL_UNUSED	0x0000	/* Canal periodique non utilise */
#define MSG_CHANNEL_0		0x0000	/* Canal aperiodique 0 utilise */
#define MSG_CHANNEL_1		0x0001	/* Canal periodique 1 utilise */
#define MSG_CHANNEL_2		0x0002	/* Canal periodique 2 utilise */
#define MSG_CHANNEL_3		0x0003	/* Canal periodique 3 utilise */
#define MSG_CHANNEL_4		0x0004	/* Canal periodique 4 utilise */
#define MSG_CHANNEL_5		0x0005	/* Canal periodique 5 utilise */
#define MSG_CHANNEL_6		0x0006	/* Canal periodique 6 utilise */
#define MSG_CHANNEL_7		0x0007	/* Canal periodique 7 utilise */
#define MSG_CHANNEL_8		0x0008	/* Canal periodique 8 utilise */

#define MSG_FIFO_PTR_REC	0x218f	/* ptr fifo reception message */

#define MSG_FIFO_PTR_EMI0	0x21a9	/* ptr fifo emission message canal 0 */
#define MSG_FIFO_PTR_EMI1	0x21aa	/* ptr fifo emission message canal 1 */
#define MSG_FIFO_PTR_EMI2	0x21ab	/* ptr fifo emission message canal 2 */
#define MSG_FIFO_PTR_EMI3	0x21ac	/* ptr fifo emission message canal 3 */
#define MSG_FIFO_PTR_EMI4	0x21ad	/* ptr fifo emission message canal 4 */
#define MSG_FIFO_PTR_EMI5	0x21ae	/* ptr fifo emission message canal 5 */
#define MSG_FIFO_PTR_EMI6	0x21af	/* ptr fifo emission message canal 6 */
#define MSG_FIFO_PTR_EMI7	0x21b0	/* ptr fifo emission message canal 7 */
#define MSG_FIFO_PTR_EMI8	0x21b1	/* ptr fifo emission message canal 8 */

#define MSG_SDN			0x0000	/* Service messagerie non acquittee */
#define MSG_SDA			0x0010	/* Service messagerie acquittee */


#define MSG_BLOC_LIBRE		0x0000	/* Bloc FIFO messagerie = libre */
#define MSG_BLOC_OCCUPE		0x0001	/* Bloc FIFO messagerie = occupe */
#define MSG_BLOC_ENCOURS	0x0003	/* Bloc FIFO messagerie = en cours emission message */



/* Evenements */


#define EVT_FIFO_BA	0x000D	/* Adresse FIFO evenements BA (option2) */
#define EVT_FIFO_VAR	0x2200	/* Adresse FIFO evt variables (option2) */
#define EVT_FIFO_MSG	0x2300	/* Adresse FIFO evt message (option2) */

#define EVT_R_FIFO_VAR	0x2183	/* Ptr remplissage FIFO evt variables (option2) */
#define EVT_R_FIFO_MSG	0x2184	/* Ptr remplissage FIFO evt messagerie (option2) */

#define EVT_V_FIFO_VAR	0x21a3	/* Ptr vidage FIFO evt variables (option2) */
#define EVT_V_FIFO_MSG	0x21a4	/* Ptr vidage FIFO evt messagerie (option2) */


#define NOT_EVT			0X0000	/* Pas d'evenement configure sur E/R de la variable */
#define EVT_SEND_VAR_P		0x8100	/* Numero pour evenement emission variable permanent */
#define EVT_SEND_VAR_T		0x0100	/* Numero pour evenement emission variable temporaire */
#define EVT_RECEIVE_VAR_P	0x8200	/* Numero pour evenement reception variable permanent */
#define EVT_RECEIVE_VAR_T	0x0200	/* Numero pour evenement reception variable temporaire */
#define EVT_RECEIVE_MSG		0x0240	/* Numero pour evenement reception message */
#define EVT_SEND_MSG		0x0140	/* Numero pour evenement emission message */
#define EVT_SEND_APU		0x0130	/* Numero pour evenement emission liste aperiodique urgente */
#define EVT_SEND_APN		0x0131	/* Numero pour evenement emission liste aperiodique normale */
#define EVT_BA_ACTIVITY		0x0400	/* Numero pour evenement BA Actif */
#define EVT_BA_STOP1		0x0401	/* Numero pour evenement arret BA suite time out suspend ou synchro externe */
#define EVT_BA_STOP2		0x0402	/* Numero pour evenement arret BA suite anomalies reseau */
#define EVT_BA_STOP3		0x0404	/* Numero pour evenement arret BA suite commande user */
#define EVT_BA_IDLE		0x0408	/* Numero pour evenement BA Veille */
#define EVT_BA_SUSPEND0		0x0500	/* Numero pour evenement SUSPEND 0 */
#define EVT_BA_SUSPEND1		0x0501	/* Numero pour evenement SUSPEND 1 */
#define EVT_BA_SUSPEND2		0x0502	/* Numero pour evenement SUSPEND 2 */
#define EVT_BA_SUSPEND255	0x05ff	/* Numero pour evenement SUSPEND 255 */

#define EVT_FLAG_VAR		0x0001	/* Masque bit EVT emission/reception variable */
#define EVT_FLAG_MSG_E		0x0002	/* Masque bit EVT emission message */
#define EVT_FLAG_MSG_R		0x0004	/* Masque bit EVT reception message */
#define EVT_FLAG_APU		0x0008	/* Masque bit EVT emission liste aperiodique urgente */
#define EVT_FLAG_APN		0x0010	/* Masque bit EVT emission liste aperiodique normale */
#define EVT_FLAG_BA_ACTIVITY	0x0020	/* Masque bit EVT BA = Actif */
#define EVT_FLAG_BA_SUSPEND	0x0040	/* Masque bit EVT macro-instruction SUSPEND */
#define EVT_FLAG_BA_STOP1	0x0080	/* Masque bit EVT arret BA suite time-out */
#define EVT_FLAG_BA_STOP2	0x0100	/* Masque bit EVT arret BA suite anomalies reseau */
#define EVT_FLAG_BA_STOP3	0x0200	/* Masque bit EVT arret BA suite commande utilisateur */
#define EVT_FLAG_BA_IDLE	0x0400	/* Masque bit EVT BA = IDLE */


#define ACK_POS_EOC		0x0001	/* Aacquittement interruption EOC positif */
#define ACK_NEG_EOC		0x0000	/* Aacquittement interruption EOC negatif */



/* Arbitre de bus */

#define BA_SYNCHRO	0xc0000		/* Adresse indicateur de synchronisme (option2) */

#define BA_NEXT_MACRO	0x0022		/* Instruction programme : Fin macrocycle */
#define BA_SEND_ID_DAT	0x0023		/* Instruction programme : Emission ID DAT */
#define BA_SEND_ID_MSG	0x0025		/* Instruction programme : Emission ID MSG */
#define BA_TESTP	0x0028		/* Instruction programme : Test des presents */
#define BA_TESTP_END	0x0029		/* Instruction programme : Fin test des presents */
#define BA_SUSPEND	0x0038		/* Instruction programme : Arret execution du programme et attente */
#define BA_SEND_MSG	0x0004		/* Instruction programme : Ouverture fenetre aperiodique pour messagerie */
#define BA_SEND_APER	0x000b		/* Instruction programme : Ouverture fenetre aperiodique pour variable */
#define BA_WAIT_TIME	0x0010		/* Instruction programme : Resynchronisation interne */
#define BA_WAIT_SYNC	0x0031		/* Instruction programme : Resynchronisation externe */
#define BA_WAIT_SYNC_SILENT	0x0032		/* Instruction programme : Resynchronisation externe SILENCE*/

#define BA_STOPPED		0x0000				/* Etat BA =  Arret */
#define BA_STARTING		0x0003				/* Etat BA =  En cours demarrage */
#define BA_IDLE			0x0007				/* Etat BA =  Attente */
#define BA_MACRO_END		BA_NEXT_MACRO |	0x8000		/* Etat BA =  Fin macrocycle */
#define BA_SENDING_ID_DAT	BA_SEND_ID_DAT | 0x8000		/* Etat BA =  Emission ID DAT */
#define BA_SENDING_ID_MSG	BA_SEND_ID_MSG | 0x8000		/* Etat BA =  Emission ID MSG */
#define BA_TESTING		BA_TESTP | 0x8000		/* Etat BA =  En cours TESTP */
#define BA_TESTING_END		BA_TESTP_END | 0x8000		/* Etat BA =  Fin TESTP */
#define BA_PENDING		BA_SUSPEND | 0x8000		/* Etat BA =  Attente ordre reprise execution par user */
#define BA_APER_WINDOW		BA_SEND_APER | 0x8000		/* Etat BA =  Traitement dde aperiodique de variable */
#define BA_MSG_WINDOW		BA_SEND_MSG | 0x8000		/* Etat BA =  Traitement dde aperiodique de messagerie */
#define BA_WAITING_TIME 	BA_WAIT_TIME | 0x8000		/* Etat BA =  Resynchronisation interne */
#define BA_WAITING_SYNC		BA_WAIT_SYNC | 0x8000		/* Etat BA =  Resynchronisation externe */
#define BA_WAITING_SYNC_SILENT		BA_WAIT_SYNC_SILENT | 0x8000		/* Etat BA =  Resynchronisation externe SILENCE */




#define BA_NUM_ABONNE		(unsigned short) 0x2024	/* Parametre config : numero abonne */
#define BA_NUM__MAX_ABONNE	(unsigned short) 0x2025	/* Parametre config : numero dernier abonne connectable sur le reseau */
#define BA_PRIORITY		(unsigned short) 0x2026	/* Parametre config : priorite arbitre de bus */
#define BA_START_TIME		(unsigned short) 0x2027	/* Parametre config : Temporisation demarrage arbitre de bus */
#define BA_ELECTION_TIME	(unsigned short) 0x2028	/* Parametre config : Temporisation election arbitre de bus */


/* Codes Commande acces classiques */

#define CMDE_R_PHY	(unsigned char) 0x20	/* Lecture physique 64 mots */
#define CMDE_R_WORD	(unsigned char) 0x21	/* Lecture physique 1 mot */

#define CMDE_W_PHY	(unsigned char) 0x80	/* Ecriture physique 64 mots */
#define CMDE_W_WORD	(unsigned char) 0x81	/* Ecriture physique 1 mot */
#define CMDE_W_DESC	(unsigned char) 0x82	/* Ecriture physique 16 mots */

#define CMDE_R_VAR	(unsigned char) 0x30	/* Lecture d'une variable longueur fixe */
#define CMDE_R_VAR_SVF	(unsigned char) 0x31	/* Lecture d'une variable longueur inconnue */
#define CMDE_R_VAR_TIME	(unsigned char) 0x32	/* Lecture d'une variable de type "heure" */
#define CMDE_R_LPRESENT	(unsigned char) 0x35	/* Lecture de la variable liste des presents */
#define CMDE_W_VAR	(unsigned char) 0x90	/* Ecriture d'une variable */

#define CMDE_R_EVT	(unsigned char) 0x40	/* Lecture d'un evenement */
#define CMDE_ACQ_EOC	(unsigned char) 0xa1	/* Acquittement signal EOC */

#define CMDE_R_MSG	(unsigned char) 0x51	/* Lecture des donnees d'un message */
#define CMDE_PURGE_MSG	(unsigned char) 0x52	/* Raz file emission message */
#define CMDE_W_MSG	(unsigned char) 0xb0	/* Ecriture et demande emission d'un message */

#define CMDE_SEND_APU	(unsigned char) 0xc0	/* Demande aperiodique urgente */
#define CMDE_SEND_APN	(unsigned char) 0xc1	/* Demande aperiodique normale */
#define CMDE_PURGE_APER	(unsigned char) 0xc2	/* Raz file de stockage des demandes aperiodiques */

#define CMDE_START_BA	(unsigned char) 0xd0	/* Demande demarrage du BA */
#define CMDE_STOP_BA	(unsigned char) 0xd1	/* Demande d'arret du BA */
#define CMDE_CHANGE_MAC	(unsigned char) 0xd2	/* Demande changement de macrocycle */
#define CMDE_CONTINUE_BA (unsigned char) 0xd3	/* Demande reprise execution du programme BA */
#define CMDE_SYNC_BA	(unsigned char) 0xd4	/* Demande de resynchronisation externe du programme BA */
#define CMDE_CHANGE_PARAM (unsigned char) 0xd5	/* Changement des parametres de fonctionnement du BA */

#define CMDE_GET_MEDIUM	(unsigned char) 0x28	/* Lecture des informations reseau */
#define CMDE_VALID_MEDIUM (unsigned char) 0xe0	/* Changement etat des voies de communication */
#define CMDE_GES_BRUIT  (unsigned char) 0xe1	/* Valide/Invalide le bruit ON/OFF */

#define CMDE_TST_RFIFO	(unsigned char) 0x21	/* Test en lecture de la FIFO de l'interface user */
#define CMDE_TST_IRQ	(unsigned char) 0x22	/* Test du canal interruption IRQ */
#define CMDE_TST_REG	(unsigned char) 0x24	/* Test des registres de l'interface user */
#define CMDE_TST_TIME	(unsigned char) 0x28	/* Test du registre TIME et du canal interruption EOC */
#define CMDE_TST_WFIFO	(unsigned char) 0x84	/* Test en ecriture de la FIFO de l'interface user */

#define CMDE_START	(unsigned char) 0xff	/* Demarrage du circuit FULLFIP2 -> INITIALISATION */
#define CMDE_VALID	(unsigned char) 0x88	/* Validation du circuit FULLFIP2 -> OPERATIONNEL */
#define CMDE_CLOSE	(unsigned char) 0x00	/* Fin de transaction */




/* Codes Commande acces libres */

#define WRITE_VAR_TIME_USER	0x0002	/* Dde ecriture variable avec status Rafr. dynamique */
#define WRITE_VAR_USER		0x0003	/* Dde ecriture variable */
#define READ_VAR_TIME_USER	0x0004	/* Dde lecture variable avec status Rafr. dynamique */
#define READ_VAR_USER		0x0005	/* Dde lecture variable */
#define READ_VAR_9002_USER	0x0006	/* Dde lecture variable "liste des presents" */
#define SEND_APER_U_USER	0x0008	/* Dde d'aperiodique urgente */
#define SEND_APER_N_USER	0x0009	/* Dde d'aperiodique normale */
#define SEND_MSG_APER_USER	0x000a	/* Dde emission message aperiodique */
#define VALID_MEDIUM_USER	0x000c	/* Validation medium */
#define GET_MEDIUM_USER		0x000d	/* Lecture infos reseau */
#define GESTION_BRUIT_USER	0x000e	/* Valide/Invalide le bruit ON/OFF */
#define READ_EVT_USER		0x000f	/* Lecture famille evenements */
#define START_BA_USER		0x0010	/* Demarrage programme BA */
#define STOP_BA_USER		0x0011	/* Arret BA */
#define CHANGE_MACRO_USER	0x0012	/* Changement macrocycle */
#define CONTINUE_USER		0x0013	/* Reprise execution programme */
#define CHANGE_PARAM_USER	0x0014	/* Changement parametre BA */
#define PURGE_APER_USER		0x0018	/* Purge file aperiodique */
#define PURGE_MSG_USER		0x001c	/* Purge file emission message */
#define ACK_EOC_USER		0x001f	/* Acquittement signal EOC */


	
/* Compte rendu sur Commande */

#define FIP_CR_OK			0x00		/* Commande executee avec succes */


#define FIP_CR_NON_SIGNIFIANT		0x80		/* Variable non recue du reseau */
#define FIP_CR_NON_PRODUCING		0x81		/* Variable pas de type produite */
#define FIP_CR_NON_CONSUMING		0x82		/* Variable pas de type consommee */
#define FIP_CR_NON_COMPATIBLE		0x83		/* Erreur sur configuration de la variable */
#define FIP_CR_FIFO_EVT_EMPTY		0x84		/* FIFO evenement vide */
#define FIP_CR_FIFO_APER_FULL		0x85		/* FIFO stockage demandes aperiodiques saturee */
#define FIP_CR_CMDE_INC 		0x86		/* Commande non supportee par le logiciel */
#define FIP_CR_OVERFLOW			0x87		/* Erreur "OVERFLOW" */
#define FIP_CR_FIFO_MSG_EMI_INC		0x88		/* FIFO messagerie emission inconnue */
#define FIP_CR_FIFO_MSG_EMI_FULL	0x89		/* FIFO messagerie emission saturee */
#define FIP_CR_FIFO_MSG_EMI_EMPTY	0x8a		/* FIFO messagerie emission vide */
#define FIP_CR_FIFO_MSG_REC_EMPTY	0x8b		/* FIFO messagerie reception vide */
#define FIP_CR_UNDERFLOW		0x8c		/* Erreur "UNDERFLOW" */
#define FIP_CR_DEF_CMDE_BA		0x8d		/* Erreur sur commande mode de marche du BA */
#define FIP_CR_DEF_ACQ_EOC		0x8e		/* Erreur sur commande acquittement signal EOC */

#endif
