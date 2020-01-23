/***************************************************************************/
/*       FICHIER: %M%      REL: %I% DATE: %G%                              */
/*DOC***********************************************************************/
/*                                       SIAT                              */
/*                     8, parc technologique de la Sainte Victoire         */
/*                                    Le Cannet                            */
/*                              13590 Meyreuil - FRANCE                    */
/*                                   Tel. 04 42 58 63 71                   */
/***************************************************************************/
/*  Fichier : usr_fip_mess.c                                               */
/*  MODULE  : usr_fip                                                      */
/***************************************************************************/
/* Auteur :   Xavier GAILLARD                                              */
/* Date de creation : O6 octobre 2000                                      */
/***************************************************************************/
/* OBJET DU MODULE      :                                                  */
/*		Inplementation des fonctions utilisateur de WorldFip			   */
/*                                                                         */
/***********************************************************************DOC*/
/* Liste des fonctions du fichier :                                        */
/***************************************************************************/
/*                           Modifications                                 */
/*                      ***********************                            */
/* Auteur :                                                                */
/* Date de la modification :                                               */
/* Description :                                                           */
/*                      ***********************                            */
/***************************************************************************/

#ifndef __INCusr_fip_messh
#define __INCusr_fip_messh

/* ******************************** */
/* DEFINITION DES CONSTANTES		*/
/* ******************************** */

#define USR_FIP_MAX_MESSAGE	5
#define USR_FIP_MESS_SIZE	256
#define USR_FIP_MCS_SIZE	0xFFFF
#define USR_FIP_MCS_NB_BUFFER	5
#define USR_FIP_MCS_BUFFER_SIZE	16320
#define USR_FIP_MESS_MAX_WAIT	7000	/* Timeout de 1 s. 				*/

/* ******************************** */
/* DEFINITION DES TYPES				*/
/* ******************************** */

/* --------------------------------
 * T_usr_fip_mess
 * =================
 * Message traite par l'application.
 * Un message recu de WorldFip occupe
 * une case qui est liberee quand la
 * reponse est retournee par l'appli-
 * cation.
 * --------------------------------	*/

typedef struct T_usr_fip_mess
{
	int				utilise;			/* Permet de savoir si le message
										 * est utilise.	*/
	int				index;
	int				type;				/* Type de commande : FDM ou MCS?	*/
	unsigned long 	date;
	FDM_MSG_TO_SEND	mess;
	FDM_MSG_T_DESC	block;
	unsigned long	ident;				/* identificateur de la requete		*/
	unsigned long	dest;				/* Adresse du destinataire reponse	*/
	unsigned char   buffer[USR_FIP_MESS_SIZE];
	struct timespec dateCmd;
	unsigned long   tailleCmd;
	unsigned char   *commande;
	MCS_SDU_TO_SEND	*reponse;
}				T_usr_fip_mess;

/* ******************************** */
/* VARIABLES GLOBALES				*/
/* ******************************** */

extern T_usr_fip_mess usr_fip_tab_mess[USR_FIP_MAX_MESSAGE];

/* ******************************** */
/* FONCTIONS DU MODULE				*/
/* ******************************** */

void usr_fip_mess_init(void);
void usr_fip_mess_liberer_tout(void);
T_usr_fip_mess *usr_fip_mess_obtenir_libre(void);
T_usr_fip_mess *usr_fip_mess_rechercher(FDM_MSG_TO_SEND *);
T_usr_fip_mess *usr_fip_mess_rechercher_mcs(MCS_SDU_TO_SEND *);

T_usr_fip_mess *usr_fip_mess_obtenir_st_al(void);
T_usr_fip_mess *usr_fip_mess_lire_st_al(void);
T_usr_fip_mess *usr_fip_mess_copier_st_al(T_usr_fip_mess *);

T_usr_fip_mess *usr_fip_mess_obtenir_periodique(void);
T_usr_fip_mess *usr_fip_mess_lire_periodique(void);
T_usr_fip_mess *usr_fip_mess_copier_periodique(T_usr_fip_mess *);

T_usr_fip_mess *usr_fip_mess_lire_premier(void);
T_usr_fip_mess *usr_fip_mess_lire_courant(void);
char *usr_fip_mess_obtenir_commande(T_usr_fip_mess *,int *);
void usr_fip_mess_liberer(T_usr_fip_mess *);
void usr_fip_mess_enlever(T_usr_fip_mess *);
int usr_fip_mess_ecrire_cmd(T_usr_fip_mess *,FDM_MSG_RECEIVED *);
int usr_fip_mess_ecrire_cmd_mcs(T_usr_fip_mess *,MCS_SDU_RECEIVED *);
int usr_fip_mess_ecrire_rep(T_usr_fip_mess *,char *,int);
int usr_fip_mess_lire(T_usr_fip_mess *,char *,int);
int usr_fip_mess_envoyer(T_usr_fip_mess *,int);
int usr_fip_mess_query_timeout(T_usr_fip_mess *,struct timespec *);
unsigned long usr_fip_mess_obtenir_date(T_usr_fip_mess *);
void usr_fip_mess_send_message(T_usr_fip_mess *);
void usr_fip_mess_send_message_mcs(T_usr_fip_mess *,int);
void usr_fip_mess_set_ana_sys_fct(void (*)(T_usr_fip_mess *,int,char *,int));
void usr_fip_mess_set_ovf_fct(void (*)(void));

unsigned long usr_fip_mess_get_dest(T_usr_fip_mess *);
unsigned long usr_fip_mess_get_ident(T_usr_fip_mess *);
void usr_fip_mess_set_dest (T_usr_fip_mess *,unsigned long);
void usr_fip_mess_set_ident(T_usr_fip_mess *,unsigned long);
void usr_fip_mess_init_debut(T_usr_fip_mess *);

#endif
