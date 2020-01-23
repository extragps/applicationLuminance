/* --------------------------------------------------------------------------
 * Nom du fichier : usr_fip_msg.c
 * Auteur 	: X. GAILLARD
 * Date 	: 15 Octobre 2000
 * Objet 	:	Le fichier contient les routines de traitement de reception
 * 				de messages et d'acquitement de transmission de message.
 * ------------------------------------------------------------------------	*/

/* ********************************	*/
/* FICHIERS D'INCLUSION				*/
/* ********************************	*/

#include "fdm.h"
#include "time.h"
#include "mcs.h"
#include "usr_fip_msg.h"
#include "usr_fip_mess.h"
#include "usr_fip_var.h"
#include "kcommon.h"

/* ********************************	*/
/* DEFINITION DES CONSTANTES		*/
/* ********************************	*/

#define DEBUG 0
#if DEBUG >0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************	*/
/* VARIABLES EXTERNES				*/
/* ********************************	*/

/* --------------------------------	*/
/* User_Msg_Ack_Proc_Contexte_RAD	*/
/* ==============================	*/
/* Traitement de la confirmation d'	*/
/* emission d'un message sur World- */
/* Fip. Elle est appelee :			*/
/* - sur timeout si il n'y a pas 	*/
/*   acquittement,					*/
/* - sur reception de l'acquitement	*/
/* 	 du message.					*/
/* --------------------------------	*/
void usr_fip_contexte_msg_ack(
		FDM_MESSAGING_REF *ref, 
		FDM_MSG_TO_SEND *msg)
{
	printDebug("J'ai recu un acquitement de transmission de message\n");
										/* ---------------------------------
										 * STATISTIQUES
										 * --------------------------------	*/
	if(msg->Service_Report.Valid)
	{
		usr_varComm_rapport.nbMsgCourtEmisNok++;
		usr_varComm_rapport.nbTransactionNok++;
	}
	else
	{
		usr_varComm_rapport.nbMsgCourtEmisOk++;
		usr_varComm_rapport.nbTransactionOk++;
	}
										/* --------------------------------	*/
										/* EXPLICATION						*/
										/* --------------------------------	*/
	switch(msg->Service_Report.Valid)
	{
	case _FDM_MSG_SEND_OK:
		/* printf("Message bien envoye\n"); */
		break;
	case _FDM_MSG_USER_ERROR:
		printf("Erreur utilisateur\n");
		switch(msg->Service_Report.msg_user_soft_report)
		{
		case _FDM_MSG_REPORT_OK:
			printf("Tout va bien\n");
			break;
		case _FDM_MSG_REPORT_MSG_NOT_ALLOWED:
			printf("Message non autorise\n");
			break;
		case _FDM_MSG_REPORT_CHANNEL_NOT_ALLOWED :
			printf("Canal non autorise\n");
			break;
		case _FDM_MSG_REPORT_ERR_LG_MSG :
			printf("Erreur sur la longueur du message\n");
			break;
		case _FDM_MSG_REPORT_ERR_MSG_INFOS :
			printf("Erreur sur les infos du message\n");
			break;
		case _FDM_MSG_REPORT_INTERNAL_ERROR :
			printf("Erreur interne\n");
			break;
		default:
			printf("Cas non prevu\n");
			break;
		}
		break;
	case _FDM_DATA_LINK_ERROR:
		printf("Erreur de transmission\n");
		switch(msg->Service_Report.Way)
		{
		case _FIP_NOACK_BAD_RP_MSG:
			printf("Pas d'acquittement\n");
			break;
		case _FIP_ACK_NO_REC_AFTER_RETRY:
			printf("Rien apres reessai\n");
			break;
		case _FIP_ACK_NEG_AFTER_RETRY:
			printf("Acquit negatif apres reessai\n");
			break;
		case _FIP_ACK_NEG_NO_RETRY:
			printf("Acquit negatif sans reessai\n");
			break;
		case _FDM_TIME_OUT:
			printf("Time out\n");
			break;
		default:
			printf("Cas non prevu\n");
			break;
		}
		break;
	default:
		printf("Cas non prevu\n");
		break;
	}									/* endswitch(msg->Service_Report	*/
										/* --------------------------------	*/
										/* LIBERATION DE LA MEMOIRE ALLOUEE	*/
										/* --------------------------------	*/
	{
	T_usr_fip_mess *message=NULL;
	message=usr_fip_mess_rechercher(msg);
		if(message!=NULL)
		{
			usr_fip_mess_liberer(message);
		}
		else
		{
			
										/* Cas de l'acquitemenent d'un
										 * message d'aquitement.	*/
			kmmFree(NULL,(void *)msg->Ptr_Block->Ptr_Data);
			kmmFree(NULL,(void *)msg->Ptr_Block);
			kmmFree(NULL,(void *)msg);
			printf("Message non trouve\n");
		}
	}
										/* --------------------------------	*/
										/* FIN De User_Msg_Ack_Proc			*/
										/* --------------------------------	*/

}

/* --------------------------------	*/
/* User_Msg_Rec_Proc_Contexte_RAD	*/
/* ==============================	*/
/* Traitement suite e reception d'un*/
/* message WorldFip.				*/
/* --------------------------------	*/

void usr_fip_contexte_msg_recv(
		FDM_MESSAGING_REF *ref, 
		FDM_MSG_RECEIVED *msg)
{										
int blocCour;
int	nbCar=0;
FDM_MSG_R_DESC	*bloc=msg->Ptr_Block;
										/* ---------------------------------
										 * UNE PETITE TRACE
										 * --------------------------------	*/
	printDebug("J'ai recu un message de Nb blocs %d\n",msg->Nr_Of_Blocks);
										/* ---------------------------------
										 * STATISTIQUES
										 * --------------------------------	*/
	usr_varComm_rapport.nbMsgCourtRecOk++;
	usr_varComm_rapport.nbTransactionOk++;
										/* --- CALCUL DE LA TAILLE RECUE --	*/		for(blocCour=0;blocCour<msg->Nr_Of_Blocks;blocCour++);
	{
	int indice;
		nbCar+=bloc->Nr_Of_Bytes;
		printDebug("Nombre de caracteres %d\n",bloc->Nr_Of_Bytes);
		for(indice=0;indice<bloc->Nr_Of_Bytes;indice++)
		{
			printDebug("%c",bloc->Ptr_Data[indice]);
		}
		bloc=bloc->Next_Block;
	}									/* endfor(blocCour					*/
	printDebug("\n");
							
	if(nbCar>=8)
	{
	T_usr_fip_mess *message=usr_fip_mess_obtenir_libre();
		if(message!=NULL)
		{
										/* --- ENREG DE LA COMMANDE			*/
			usr_fip_mess_ecrire_cmd(message,msg);
										/* --- TRAITEMENT MESSAGE SUIVANT	*/
			if(message==usr_fip_mess_lire_premier())
			{
				if(usr_fip_mess_envoyer(
						message,usr_fip_get_fd())==-1)
				{
					printf("Probleme d'ecriture dans le pipe\n");
				}			/* endif(usr_fip_mess_envoyer		*/
			}						
		}
		else
		{
			/* Probleme d'allocation de memoire, il faut faire
			 * quelquechose.	*/
			if(msg->Ptr_Block!=NULL)	/* Petit test pour pas se tromper.	*/
			{
			FDM_MSG_R_DESC *ptCour=msg->Ptr_Block;
				do						/* Il peut y avoir plusieurs blocks	*/
				{
				FDM_MSG_R_DESC *aLiberer=ptCour;
					ptCour=aLiberer->Next_Block;
					fdm_msg_data_buffer_free(aLiberer);
				}		while (ptCour!=NULL);
			}							/* endif(msg->Ptr_Block				*/
			fdm_msg_ref_buffer_free(msg);
		}
	}
	else
	{
										/* --------------------------------	*/
										/* LIBERATION DES BLOCKS			*/
										/* --------------------------------	*/
										/* Il s'agit ici de liberer les 
										 * blocs de memoire allouees par fdm
										 * pour transporter mon message.
										 */
		if(msg->Ptr_Block!=NULL)		/* Petit test pour pas se tromper.	*/
		{
		FDM_MSG_R_DESC *ptCour=msg->Ptr_Block;
			do							/* Il peut y avoir plusieurs blocks	*/
			{
			FDM_MSG_R_DESC *aLiberer=ptCour;
				ptCour=aLiberer->Next_Block;
				fdm_msg_data_buffer_free(aLiberer);
			}		while (ptCour!=NULL);
		}								/* endif(msg->Ptr_Block				*/
		fdm_msg_ref_buffer_free(msg);
	}
										/* --------------------------------	*/
										/* LIBERATION DU MESSAGE			*/
										/* --------------------------------	*/
										/* --------------------------------	*/
										/* FIN DE User_Msg_Rec_Proc			*/
										/* --------------------------------	*/
}


