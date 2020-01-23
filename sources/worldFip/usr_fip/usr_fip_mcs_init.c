
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 13/11/00
 * Fichier	: usr_fip_mcs_init.c
 * Objet	: Initialisation de la couche mcs.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,20Nov00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/


#include <time.h>
#ifdef VXWORKS
#include <timers.h>
#endif
#include <fdm.h>
#include <mcs.h>
#include <stdio.h>
#include "usr_fip_mess.h"
#include "usr_fip_var.h"
#include "usr_fip_mcs_init.h"

/* ********************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ********************************	*/

static void usr_fip_mcs_signal(void);
static void usr_fip_mcs_signal_error(
		_MCS_TYPE_ERROR_WARNING,
		void *,
		_MCS_ERROR_CODE_LIST);
static void usr_fip_mcs_signal_warning(
		_MCS_TYPE_ERROR_WARNING,
		void *,
		_MCS_ERROR_CODE_LIST);
static void usr_fip_mcs_cmd_cnf(
		struct _MCS_REF *,struct _MCS_CHANNEL_COMMAND *);
static void usr_fip_mcs_failure(USER_MCSCHANNEL_REF , _CONNEXION_FAILURE_LIST);
static void usr_fip_mcs_msg_rec(USER_MCSCHANNEL_REF,struct _MCS_SDU_RECEIVED *);
static void usr_fip_msg_ack(USER_MCSCHANNEL_REF	, struct _MCS_SDU_TO_SEND *);
_MCS_ERROR_CODE_LIST usr_fip_mcs_send_mes (char *message,int);





/* ********************************
 * VARIABLES LOCALES AU MODULE
 * ********************************	*/

/* --------------------------------
 * usr_fip_mcs_ref
 * ===============
 * --------------------------------	*/

static MCS_REF *usr_fip_mcs_ref=NULL;
static int usr_fip_mcs_nb_signal=0;
static struct _MCSCHANNEL_REF *usr_fip_mcs_channel_ref=NULL;

/* --------------------------------
 * usr_fip_mcs_ref
 * ===============
 * Reference MCS qui devrait etre
 * utilisee
static MCS_REF *usr_fip_mcs_ref */

/* --------------------------------
 * usr_fip_mcs_channel
 * ===================
 * Configuration du canal MCS.
 * --------------------------------	*/

static MCS_USER_CHANNEL_CONFIGURATION usr_fip_mcs_channel =
{
	(USER_MCSCHANNEL_REF) 1,
	0x000000, 							/* LSAP = 1000 seg = 0 */
	0x000000,							/* LSAP = 2000 seg = 0 */
	100,
	65535,	/* Taille max message a envoyer */
    _MCS_TYPE_DLL_FULLFIP,	/* Si com avec Fullfip */
 	usr_fip_mcs_msg_rec,
 	usr_fip_msg_ack,
	_MCS_MSG_TYPE_PERIOD, /* Si periodique */
	_MCS_MSG_MODE_NORMALISE
};

/* --------------------------------
 * usr_fip_mcs_config
 * ===================
 * Canal de commande MCS.
 * --------------------------------	*/

static MCS_CONFIGURATION usr_fip_mcs_config=
{
	"MCS_PMV",
	NULL,
	usr_fip_mcs_signal,
	usr_fip_mcs_signal_error,
	usr_fip_mcs_signal_warning,
	usr_fip_mcs_cmd_cnf,
	usr_fip_mcs_failure,
	NULL
};

MCS_CHANNEL_COMMAND usr_fip_mcs_cmd;
DESC_BLOCK_DATA_T 	msgDesc ;
MCS_SDU_TO_SEND 	msgSdu;

/* Pour les tests */

static char *usr_fip_mcs_buffer=NULL;
static int	 usr_fip_mcs_lg_buffer=0;
static int	 usr_fip_mcs_rec=0;

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

void usr_fip_mcs_stop (void)
{
	if (usr_fip_mcs_ref !=NULL)
	{
		if(NULL!=usr_fip_mcs_channel_ref)
		{
		_MCS_ERROR_CODE_LIST report;
			usr_fip_mcs_cmd.Command 			= _MCS_CHANNEL_STOP;
			usr_fip_mcs_cmd.channelmcs 			= usr_fip_mcs_channel_ref;
			report=mcs_channel_command (usr_fip_mcs_ref, &usr_fip_mcs_cmd);
			if(_MCS_OK!=report)
			{
				printf("Erreur a l'arret du canal %#X\n",report);
			}							/* endif(_MCS_OK!=report			*/
			ksleep(100);
			usr_fip_mcs_process();
			usr_fip_mcs_cmd.Command 			= _MCS_CHANNEL_DELETE;
			usr_fip_mcs_cmd.channelmcs 			= usr_fip_mcs_channel_ref;
			report=mcs_channel_command (usr_fip_mcs_ref, &usr_fip_mcs_cmd);
			if(_MCS_OK!=report)
			{
				printf("Erreur a l'effacement du canal%#X\n",report);
			}							/* endif(_MCS_OK!=report			*/
		} 								/* endif (usr_fip_mcs_re */
		ksleep(100);
		usr_fip_mcs_process();
		mcs_free(usr_fip_mcs_ref);
		usr_fip_mcs_ref=NULL;
	}
}

/* --------------------------------
 * usr_fip_mcs_init
 * ================
 * Initialisation de MCS.
 * --------------------------------	*/

int usr_fip_mcs_init (FDM_REF *usr_fip_ref,int adresse)
{
int retour=0;
_MCS_ERROR_CODE_LIST report;
										/* Calcul de l'adresse equipement	*/
	usr_fip_mcs_channel.Local_Address= (adresse<<8);
										/* Creation canal messagerie FDM
										 * pour MCS */
	/*
	usr_fip_mcs_fdm_channel.Identifier+=adresse;
	if (fdm_channel_create (usr_fip_ref,&usr_fip_mcs_fdm_channel)!=FDM_OK)
	{
		retour=-1;
		printf ("Err sur creation canal msg MCS\n");
	}
	else
	*/
	{
		usr_fip_mcs_config.Fdm_Ref=usr_fip_ref;
		usr_fip_mcs_ref =mcs_initialize (&usr_fip_mcs_config,&report);
		if (usr_fip_mcs_ref ==NULL)
		{
		char szText[200];
		int x=0;
			retour=-1;
			x =sprintf (szText,"ERROR :mcs_ref null" );
			if(report != _MCS_OK)
			{
				x +=sprintf (szText+x," Report = " );
				mcs_get_errorandwarningstring ( szText+x , report);
				printf ("Err init MCS : %s\n", szText);
			}
		}
		else
		{
		_MCS_ERROR_CODE_LIST report;
			usr_fip_mcs_cmd.Command 			= _MCS_CHANNEL_CREATE;
			usr_fip_mcs_cmd.paramIN_for_create 	= &usr_fip_mcs_channel;
			usr_fip_mcs_cmd.Report 				= _MCS_OK;
			report=mcs_channel_command (usr_fip_mcs_ref, &usr_fip_mcs_cmd);
			if(_MCS_OK!=report)
			{
				printf("Erreur a la creation du canal MCS\n");
				retour=-1;
				usr_fip_mcs_stop();
			}							/* endif(_MCS_OK!=report			*/
			else
			{
				usr_fip_mcs_process();
			}
		} 								/* endif (usr_fip_mcs_ref ==NULL)	*/
	}
										/* --------------------------------
										 * FIN DE usr_fip_mcs_init
										 * --------------------------------	*/
	return retour;
}

/* --------------------------------
 * usr_fip_mcs_process
 * ====================
 * Traitement des signaux MCS
 * --------------------------------	*/

int usr_fip_mcs_process(void)
{
int retour=0;
	if(usr_fip_mcs_nb_signal)
	{
		mcs_fifos_empty(usr_fip_mcs_ref);
		usr_fip_mcs_nb_signal=0;
		retour=1;
	}									/* endif(usr_fip_mcs_nb_signal		*/
										/* --------------------------------
										 * FIN DE usr_fip_mcs_process
										 * --------------------------------	*/
	return retour;
}

/* --------------------------------
 * usr_fip_mcs_send_mes
 * ====================
 * Fonction plutot applicative
 * permettant l'emission d'un message
 * MCS...
 * --------------------------------	*/

_MCS_ERROR_CODE_LIST usr_fip_mcs_send_mes (char *message,int lg)
{
_MCS_ERROR_CODE_LIST	report;
										/* Initialisation du buffer MCS	*/
	msgSdu.Nr_Of_Blocks = 1;
	msgSdu.Ptr_Block = &msgDesc;
	msgDesc.User_Block.Ptr_Data = (unsigned char *)message;
	msgDesc.User_Block.Nr_Of_Bytes = lg;
	msgDesc.User_Block.Next_Block=NULL;

//	printf ("usr_fip_mcs_send_mes : adresse du bloc %x\n", &msgDesc);
	report=mcs_send_message(usr_fip_mcs_cmd.channelmcs,&msgSdu);
	if(_MCS_OK!=report)
	{
		printf ("Erreur %d de mcs_send_message\n", report);
	}
										/* --------------------------------
										 * FIN DE usr_fip_mcs_send_mes
										 * --------------------------------	*/
	return report;
}



/* --------------------------------
 * usr_fip_mcs_signal_error
 * ========================
 * --------------------------------	*/

static void usr_fip_mcs_signal_error(
		_MCS_TYPE_ERROR_WARNING  MCS_Err,
		void 					*P ,
		_MCS_ERROR_CODE_LIST 	 Code)
{
	char String_Err [100];
	mcs_get_errorandwarningstring (String_Err, Code);
	printf ("Erreur MCS : %s\n", String_Err);
										/* --------------------------------
										 * FIN DE usr_fip_mcs_signal_error
										 * --------------------------------	*/
}

/* --------------------------------
 * usr_fip_mcs_signal_warning
 * ==========================
 * --------------------------------	*/

static void usr_fip_mcs_signal_warning (
		_MCS_TYPE_ERROR_WARNING  MCS_Err,
		void 					*P ,
		_MCS_ERROR_CODE_LIST 	 Code)
{
char String_Err [100];
	mcs_get_errorandwarningstring (String_Err, Code);
	printf ("Warning MCS : %s\n", String_Err);
										/* --------------------------------
										 * FIN DE usr_fip_mcs_signal_warning
										 * --------------------------------	*/
}

/* ---------------------------------
 * usr_fip_mcs_signal
 * ==================
 * Signal permettant de signaler
 * a l'applicatif que la fifo
 * MCS doit etre depilee.
 * --------------------------------	*/

void usr_fip_mcs_signal (void)
{
	/* printf ("Signal MCS\n"); */
	usr_fip_mcs_nb_signal++;
}

/* --------------------------------
 * usr_fip_mcs_cmd_cnf
 * ===================
 * Traitement d'un signal de
 * reception d'une configuration de
 * commande.
 * --------------------------------	*/

void usr_fip_mcs_cmd_cnf (
		struct _MCS_REF *mcs_ref,
		struct _MCS_CHANNEL_COMMAND *mcs_chann_cmd)
{
	printf ("Signal command CNF\n");
	usr_fip_mcs_channel_ref=mcs_chann_cmd->channelmcs;
}

/* --------------------------------
 * usr_fip_mcs_failure
 * ====================
 * Traitement d'un signal de
 * rupture de connexion.
 * --------------------------------	*/

void usr_fip_mcs_failure (
		USER_MCSCHANNEL_REF mcs_channel,
		_CONNEXION_FAILURE_LIST cnx_fail_list)
{
	printf ("Signal Cnx Failure %d\n", cnx_fail_list);
}

/* --------------------------------
 * usr_fip_mcs_msg_rec
 * ===================
 * Traitement de la reception d'un
 * message MCS.
 * --------------------------------	*/
void usr_fip_mcs_msg_rec (
		USER_MCSCHANNEL_REF mcs_channel,
		struct _MCS_SDU_RECEIVED *mcs_sdu_rec)
{
int blocCour;
int	posCour=0;
DESC_BLOCK_DATA_R	*bloc;
	/* TO BE CHANGED
	mcs_cat_msg_received (Mcs_Rec_Buf, mcs_sdu_rec);
	*/
	printf("J'ai recu un message MCS de %d octets\n",mcs_sdu_rec->SDU_Size);


	usr_varComm_rapport.nbMsgLongRecOk++;
	usr_varComm_rapport.nbTransactionOk++;

	if(mcs_sdu_rec->SDU_Size>8)
	{
	T_usr_fip_mess *message=usr_fip_mess_obtenir_libre();
		if(message!=NULL)
		{
			usr_fip_mess_ecrire_cmd_mcs(message,mcs_sdu_rec);
										/* --- TRAITEMENT MESSAGE SUIVANT	*/
			if(message==usr_fip_mess_lire_premier())
			{
				if(usr_fip_mess_envoyer(message,usr_fip_get_fd())==-1)
				{
					printf("Probleme d'ecriture dans le pipe\n");
				}			/* endif(usr_fip_mess_envoyer		*/
			}
		}
		else
		{
			free_mcs_received_message (mcs_sdu_rec);
		}
	}
	else
	{
		free_mcs_received_message (mcs_sdu_rec);
	} 									/* endif(mcs_sdu_rec->SDU_Size>8)	*/
										/* --------------------------------
										 * FIN DE usr_fip_mcs_rec
										 * --------------------------------	*/
}

/* --------------------------------
 * usr_fip_msg_ack
 * ===============
 * Acquitement sur l'envoi d'un
 * message WorldFip
 * --------------------------------	*/

void usr_fip_msg_ack (
		USER_MCSCHANNEL_REF mcs_channel,
		MCS_SDU_TO_SEND *msg)
{
									/* --------------------------------
									 * TEST DE L'ERREUR EVENTUELLE
									 * --------------------------------	*/
	if(0==msg->Service_Report)
	{
									/* Message OK	*/
		usr_varComm_rapport.nbMsgLongEmisOk++;
		usr_varComm_rapport.nbTransactionOk++;
	}
	else
	{
									/* Message KO	*/
		usr_varComm_rapport.nbMsgLongEmisNok++;
		usr_varComm_rapport.nbTransactionNok++;
	}
									/* --------------------------------	*/
									/* LIBERATION DE LA MEMOIRE ALLOUEE	*/
									/* --------------------------------	*/
	{
	T_usr_fip_mess *message=NULL;
	
		message=usr_fip_mess_rechercher_mcs(msg);
		if(message!=NULL)
		{
			usr_fip_mess_liberer(message);
		}
		else
		{
			printf("Message non trouve\n");
		}
	}

}


void usr_fip_mcs_send(void)
{
	if(usr_fip_mcs_rec)
	{
		usr_fip_mcs_send_mes(usr_fip_mcs_buffer,usr_fip_mcs_lg_buffer);
		usr_fip_mcs_rec=0;
	}
}
