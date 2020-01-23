
/* 
 * Modifications :
 * =============
 * 01a,08Mar01,xag  Suppression de la fonction d'enregistrement de la question.
 * 					Celle ci est directement realisee dans usr_fip_ana_sys.	
 */
/* ********************************	*/
/* INCLUSION DES FICHIERS			*/
/* ********************************	*/

#include "time.h"
#ifdef VXWORKS
#include "vxWorks.h"
#include "timers.h"
#endif
#include "netinet/in.h"
#include "kcommon.h"
#include "fdm.h"
#include "mcs.h"
#include "man_fdmi.h"
#include "man_olga.h"
#include "usr_fip_init.h"
#include "usr_fip_mess.h"
#include "usr_fip_mcs_init.h"
#include "define.h"
#include "mon_debug.h"

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define DEBUG 1
#if DEBUG >0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

static char _leBuffer[61000];

/* ********************************
 * PROTOTYPE DES FONCTIONS LOCALES
 * ********************************	*/

static int usr_fip_mess_ecrire_car(T_usr_fip_mess *,char);
static void usr_fip_mess_copier(T_usr_fip_mess *,char *,int,int);
void usr_fip_mess_init_debut(T_usr_fip_mess *mess);

/* ********************************	*/
/*DECLARATION DES VARIABLE GLOBALES	*/
/* ********************************	*/

T_usr_fip_mess 	usr_fip_tab_mess[USR_FIP_MAX_MESSAGE];
static int 		usr_fip_mess_ordre[USR_FIP_MAX_MESSAGE];
T_usr_fip_mess 	usr_fip_mess_periodique;
T_usr_fip_mess 	usr_fip_mess_st_al;
T_usr_fip_mess *usr_fip_mess_courant=NULL;
static void (*usr_fip_mess_ana_sys)(T_usr_fip_mess *,int,char *,int)=NULL;
static void (*usr_fip_mess_ovf)(void)=NULL;

#define USR_FIP_BIT_ACK	0x01000000

/* ********************************	*/
/* FONCTIONS DU MODULE				*/
/* ********************************	*/

void *usr_fip_mess_alloc(void *ref,int32 taille)
{
void *alloc_p=NULL;
	if(taille>10000)
	{
		if(taille<=61000)
		{
			alloc_p=_leBuffer;
//			printDebug("usr_fip_mess_alloc : %#x de %d octets\n",alloc_p,taille);
		}
		else
		{
			printDebug("Erreur d'allocation memoire...\n");
		}	
	}
	else
	{
		alloc_p=kmmAlloc(ref,taille);
//		printDebug("usr_fip_mess_alloc : %#x de %d octets\n",alloc_p,taille);
	}
	return alloc_p;
}
void usr_fip_mess_free(void *ref,void *alloc_p)
{
//	printDebug("usr_fip_mess_free : %#x\n",alloc_p);
	if(_leBuffer!=alloc_p)
	{
		kmmFree(ref,alloc_p);
	}
}

void *usr_mcs_mess_alloc(void *ref,int32 taille)
{
void *alloc_p=kmmAlloc(ref,taille);
//	printDebug("usr_mcs_mess_alloc : %#x de %d octets\n",alloc_p,taille);
	return alloc_p;
}
void usr_mcs_mess_free(void *ref,void *alloc_p)
{
//	printDebug("usr_mcs_mess_free : %#x\n",alloc_p);
	kmmFree(ref,alloc_p);
}

/* --------------------------------
 * usr_fip_mess_tester_acquitement
 * ===============================
 * Test de l'identifiant de la re-
 * quete pour detecter une eventuelle
 * demande d'acquitement.
 * --------------------------------	*/

static int usr_fip_mess_tester_acquitement(T_usr_fip_mess *mess)
{
int retour=0;
	if(NULL!=mess)
	{
		retour=(USR_FIP_BIT_ACK&mess->ident);
	}
	return retour;
}

/* --------------------------------
 * usr_fip_mess_envoyer_acquite
 * ============================
 * Envoi d'une trame fdm pour
 * signaler la reception de la trame.
 * --------------------------------	*/

static void usr_fip_mess_envoyer_acquitement(T_usr_fip_mess *mess)
{
	if(NULL!=mess)
	{
	FDM_MSG_TO_SEND	*msg=(FDM_MSG_TO_SEND *)usr_fip_mess_alloc(NULL,sizeof(FDM_MSG_TO_SEND));
		if(NULL!=msg)
		{
		FDM_MSG_T_DESC	*block=(FDM_MSG_T_DESC *)usr_fip_mess_alloc(NULL,sizeof(FDM_MSG_T_DESC));
			if(NULL!=block)
			{
										/* Allocation de 10 caracteres pour	
										 * stocker le message a retourner	*/
			char *buffer=(char *)usr_fip_mess_alloc(NULL,10);
				if(NULL!=buffer)
				{
					msg->Nr_Of_Blocks=1;
					msg->Ptr_Block=block;
					block->Nr_Of_Bytes=9;
					block->Next_Block=NULL;
					block->Ptr_Data=(unsigned char *)buffer;
					*(unsigned long *)(&buffer[0])=htonl(mess->ident);
					*(unsigned long *)(&buffer[4])=htonl(mess->dest);
					buffer[8]=0x06;		/* Caractere ACK					*/
					fdm_send_message(usr_fip_contexte_msg.MsgRef,msg);
				}
				else
				{
					usr_fip_mess_free(NULL,(void *)block);
					usr_fip_mess_free(NULL,(void *)msg);
				}
			}
			else
			{
				usr_fip_mess_free(NULL,(void *)msg);
			}							/* endif(NULL!=block				*/
		}								/* endif(NULL!=msg					*/
	}									/* endif(NULL!=mess					*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_
										 * --------------------------------	*/
}

/* ---------------------------------
 * usr_fip_mess_set_ana_sys_fct
 * ============================
 * Enregistrement de la fonction pour
 * l'analyse des messages.
 * --------------------------------	*/

void usr_fip_mess_set_ana_sys_fct(
		void (*ana_sys)(T_usr_fip_mess *,int,char *,int))
{
	usr_fip_mess_ana_sys=ana_sys;
}

/* ---------------------------------
 * usr_fip_mess_set_ovf_fct
 * ============================
 * Enregistrement de la fonction pour
 * l'analyse des messages.
 * --------------------------------	*/

void usr_fip_mess_set_ovf_fct(
		void (*ovf)(void))
{
	usr_fip_mess_ovf=ovf;
}

/* --------------------------------
 * usr_fip_mess_raz
 * ================
 * Pour remettre e zero les champs 
 * d'un message.
 * --------------------------------	*/

static void usr_fip_mess_raz(T_usr_fip_mess *mess,int indice,int type)
{
	mess->type		=type;
	mess->index		=indice;
	mess->utilise	=0;
	mess->commande	=NULL;
	mess->reponse	=NULL;
	mess->tailleCmd	=0;
										/* Pour le cas le plus courant de
										 * message FDM.	*/
	mess->mess.Nr_Of_Blocks=1;
	mess->mess.Ptr_Block=&mess->block;
	mess->block.Nr_Of_Bytes=0;
	mess->block.Next_Block=NULL;
	mess->block.Ptr_Data=&mess->buffer[0];
}

/* --------------------------------	*/
/* usr_fip_mess_liberer_tout		*/
/* =========================		*/
/* --------------------------------	*/

void usr_fip_mess_liberer_tout()
{
int indice;
	for(indice=0;indice<USR_FIP_MAX_MESSAGE;indice++)
	{
	T_usr_fip_mess *mess=&usr_fip_tab_mess[indice];
		usr_fip_mess_liberer(mess);
	}									/* endfor(indice=0;					*/
										/* --------------------------------
										 * INIT DU MESSAGE PERIODIQUE POUR ABT
										 * --------------------------------	*/
	usr_fip_mess_liberer(&usr_fip_mess_periodique);
	usr_fip_mess_liberer(&usr_fip_mess_st_al);
										/* --------------------------------
										 * FIN DE usr_fip_mess_init
										 * --------------------------------	*/
}

/* --------------------------------	*/
/* usr_fip_mess_init				*/
/* =================				*/
/* Initialisation de la table des	*/
/* messages.						*/
/* --------------------------------	*/

void usr_fip_mess_init()
{
int indice;
	for(indice=0;indice<USR_FIP_MAX_MESSAGE;indice++)
	{
	T_usr_fip_mess *mess=&usr_fip_tab_mess[indice];
		usr_fip_mess_ordre[indice]=-1;
		usr_fip_mess_raz(mess,indice,0);
	}									/* endfor(indice=0;					*/
										/* --------------------------------
										 * INIT DU MESSAGE PERIODIQUE POUR ABT
										 * --------------------------------	*/
	usr_fip_mess_raz(&usr_fip_mess_periodique,indice,1);
	usr_fip_mess_raz(&usr_fip_mess_st_al,indice+1,1);
										/* --------------------------------
										 * FIN DE usr_fip_mess_init
										 * --------------------------------	*/
}

/* --------------------------------
 * usr_fip_mess_obtenir_libre	
 * ==========================	
 * Recuperation d'un bloc de message
 * libre.					
 * Retour:				
 * - adresse du message libre ou NULL
 * si aucun message libre n'est dis-
 * ponible.
 * --------------------------------	*/

T_usr_fip_mess *usr_fip_mess_obtenir_libre(void)
{
T_usr_fip_mess *mess=NULL;
int indice;
	for(indice=0;(indice<USR_FIP_MAX_MESSAGE)&&(mess==NULL);indice++)
	{
		if(usr_fip_tab_mess[indice].utilise==0)
		{
			mess=&usr_fip_tab_mess[indice];
			mess->utilise=1;
			break;
		}
	}									/* endfor(indice					*/
	if(mess!=NULL)
	{
	int indDeux;
		for(indDeux=0;indDeux<USR_FIP_MAX_MESSAGE;indDeux++)
		{
			if(usr_fip_mess_ordre[indDeux]==-1)
			{
				usr_fip_mess_ordre[indDeux]=indice;
				break;
			}
		}
	}
	else
	{
		if(NULL!=usr_fip_mess_ovf)
		{
			usr_fip_mess_ovf();
		}								/* endif(NULL!=usr_fip_mess_ovf		*/
	} 									/* endif(mess!=NULL)				*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_obtenir_libre
										 * --------------------------------	*/
	return(mess);
}

/* --------------------------------
 * usr_fip_mess_lire_periodique
 * ===============================
 * Lecture du message periodique et
 * seulement si il est utilise.
 * --------------------------------	*/

T_usr_fip_mess *usr_fip_mess_lire_periodique(void)
{
T_usr_fip_mess *mess=&usr_fip_mess_periodique;
	if(0==mess->utilise)
	{
		mess=NULL;
	}									/* endif(0==mess->utilise)			*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_periodique
										 * --------------------------------	*/
	return &usr_fip_mess_periodique;
}

/* --------------------------------
 * usr_fip_mess_obtenir_periodique
 * ===============================
 * Obtention du pointeur sur le 
 * message periodique
 * --------------------------------	*/

T_usr_fip_mess *usr_fip_mess_obtenir_periodique(void)
{
T_usr_fip_mess *mess=&usr_fip_mess_periodique;
	if(0==mess->utilise)
	{
		mess->utilise=1;
	}
	else
	{
		mess=NULL;
	}									/* endif(0==mess->utilise)			*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_periodique
										 * --------------------------------	*/
	return &usr_fip_mess_periodique;
}

/* --------------------------------
 * usr_fip_mess_copier_periodique
 * ===============================
 * Copier les informations d'abonnement
 * dans le message periodique.
 * --------------------------------	*/

T_usr_fip_mess *usr_fip_mess_copier_periodique(T_usr_fip_mess *source)
{
T_usr_fip_mess *mess=&usr_fip_mess_periodique;
	if(0==mess->utilise)
	{
		mess->dest=source->dest;
		mess->ident=source->ident;
		usr_fip_mess_liberer(mess);
	}
	else
	{
		mess->dest=source->dest;
		mess->ident=source->ident;
	}									/* endif(0==mess->utilise)			*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_periodique
										 * --------------------------------	*/
	return &usr_fip_mess_periodique;
}


/* --------------------------------
 * usr_fip_mess_lire_st_al
 * ===============================
 * Lecture du message utilise pour le 
 * ST AL et seulement si il est dispo.
 * --------------------------------	*/

T_usr_fip_mess *usr_fip_mess_lire_st_al(void)
{
T_usr_fip_mess *mess=&usr_fip_mess_st_al;
	if(0==mess->utilise)
	{
		mess=NULL;
	}									/* endif(0==mess->utilise)			*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_st_al
										 * --------------------------------	*/
	return &usr_fip_mess_st_al;
}

/* --------------------------------
 * usr_fip_mess_obtenir_st_al
 * ===============================
 * Obtention du pointeur sur le 
 * message periodique
 * --------------------------------	*/

T_usr_fip_mess *usr_fip_mess_obtenir_st_al(void)
{
T_usr_fip_mess *mess=&usr_fip_mess_st_al;
	if(0==mess->utilise)
	{
		mess->utilise=1;
	}
	else
	{
		mess=NULL;
	}									/* endif(0==mess->utilise)			*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_st_al
										 * --------------------------------	*/
	return &usr_fip_mess_st_al;
}

/* --------------------------------
 * usr_fip_mess_copier_st_al
 * ===============================
 * Copier les informations d'abonnement
 * dans le message periodique.
 * --------------------------------	*/

T_usr_fip_mess *usr_fip_mess_copier_st_al(T_usr_fip_mess *source)
{
T_usr_fip_mess *mess=&usr_fip_mess_st_al;
	if(0==mess->utilise)
	{
		mess->dest=source->dest;
		mess->ident=source->ident;
		usr_fip_mess_liberer(mess);
	}
	else
	{
		mess->dest=source->dest;
		mess->ident=source->ident;
	}									/* endif(0==mess->utilise)			*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_st_al
										 * --------------------------------	*/
	return &usr_fip_mess_st_al;
}

/* --------------------------------
 * usr_fip_mess_rechercher
 * =======================
 * Rechercher dans la liste des
 * messages celui dont l'adresse
 * l'adresse 
 * Entree :
 * - l'adresse du bloc fdm a recher-
 * 	cher.
 * Retour :
 * - le message trouve ou NULL sinon
 * ---------------------------------- */

T_usr_fip_mess *usr_fip_mess_rechercher(FDM_MSG_TO_SEND *mess)
{
										/* --------------------------------
										 * DECLARATION DES VARIABLES
										 * --------------------------------	*/
T_usr_fip_mess *result=NULL;
int indice;
										/* --------------------------------
										 * BOUCLE DE TRAITEMENT
										 * --------------------------------	*/
	if(mess==&usr_fip_mess_periodique.mess)
	{
		result=&usr_fip_mess_periodique;
	}
	else if(mess==&usr_fip_mess_st_al.mess)
	{
		result=&usr_fip_mess_st_al;
	}
	else
	{
		for(indice=0;(indice<USR_FIP_MAX_MESSAGE)&&(result==NULL);indice++)
		{
			if(mess==&usr_fip_tab_mess[indice].mess)
			{
				result=&usr_fip_tab_mess[indice];
			}							/* endif(mess==&usr_fip_tab_mess	*/
		}								/* endfor(indice=0:(indice<USR_FIP	*/
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_rechercher
										 * --------------------------------	*/
	return(result);
}

/* --------------------------------
 * usr_fip_mess_rechercher_mcs
 * ===========================
 * Rechercher dans la liste des
 * messages celui dont l'adresse
 * l'adresse correspond au message
 * mcs envoye.
 * Entree :
 * - l'adresse du bloc mcs a recher-
 * 	cher.
 * Retour :
 * - le message trouve ou NULL sinon
 * ---------------------------------- */

T_usr_fip_mess *usr_fip_mess_rechercher_mcs(MCS_SDU_TO_SEND *mess)
{
										/* --------------------------------
										 * DECLARATION DES VARIABLES
										 * --------------------------------	*/
T_usr_fip_mess *result=NULL;
int indice;
										/* --------------------------------
										 * BOUCLE DE TRAITEMENT
										 * --------------------------------	*/
	if(mess==usr_fip_mess_st_al.reponse)
	{
		result=&usr_fip_mess_st_al;
	}
	else if(mess==usr_fip_mess_periodique.reponse)
	{
		result=&usr_fip_mess_periodique;
	}
	else
	{
		for(indice=0;(indice<USR_FIP_MAX_MESSAGE)&&(result==NULL);indice++)
		{
			if(mess==usr_fip_tab_mess[indice].reponse)
			{
				result=&usr_fip_tab_mess[indice];
			}							/* endif(mess==&usr_fip_tab_mess	*/
		}								/* endfor(indice=0:(indice<USR_FIP	*/
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_rechercher_mcs
										 * --------------------------------	*/
	return(result);
}

/* --------------------------------
 * usr_fip_mess_liberer_fdm
 * ========================
 * Liberation des buffers de data
 * MCS et de la structure de message.
 * --------------------------------	*/

static void usr_fip_mess_liberer_fdm(T_usr_fip_mess *mess)
{
FDM_MSG_RECEIVED *msg=(FDM_MSG_RECEIVED *)mess->commande;

	if(NULL!=msg)
	{
	FDM_MSG_R_DESC	 *bloc=msg->Ptr_Block;
	int blocCour;
		printDebug("Dans usr_fip_liberer_fdm %#x %#x\n",
			mess,&usr_fip_mess_periodique);
	
		for(blocCour=0;blocCour<msg->Nr_Of_Blocks;blocCour++)
		{
		FDM_MSG_R_DESC *suiv=bloc->Next_Block;
			fdm_msg_data_buffer_free(bloc);
			bloc=suiv;
		}
		fdm_msg_ref_buffer_free(msg);
	}
	mess->commande=NULL;
										/* --------------------------------
										 * FIN DE usr_fip_mess_liberer_fdm
										 * --------------------------------	*/
}

/* --------------------------------
 * usr_fip_mess_liberer_mcs
 * ========================
 * Liberation de la structure de 
 * message en entree.
 * --------------------------------	*/

static void usr_fip_mess_liberer_mcs(T_usr_fip_mess *mess)
{
MCS_SDU_RECEIVED	*msg=(MCS_SDU_RECEIVED *)mess->commande;

	printDebug("Dans usr_fip_liberer_mcs\n");
	
										/* Liberation du message en utilisant 
										 * la procedure qui va bien. */
	free_mcs_received_message(msg);	
	mess->commande=NULL;
										/* --------------------------------
										 * FIN DE usr_fip_mess_liberer_mcs
										 * --------------------------------	*/
}

/* --------------------------------
 * usr_fip_mess_liberer
 * ====================
 * Il s'agit de liberer un message,
 * c'est e dire que celui ci n'est
 * plus utilise et qu'il peut etre 
 * disponible pour autre chose.
 * --------------------------------	*/
extern DESC_BLOCK_DATA_T 	msgDesc ;

void usr_fip_mess_liberer(T_usr_fip_mess *mess)
{
int indice=0;
	mess->utilise=0;
										 /* buffer de sortie	*/
	printDebug("Liberation de la reponse \n");

	if(NULL!=mess->reponse)
	{
	DESC_BLOCK_DATA_T *bloc=mess->reponse->Ptr_Block;
	int	blocCour;
										/* Liberation de tous les blocs.	*/
		for(blocCour=0;blocCour<mess->reponse->Nr_Of_Blocks;blocCour++)
		{
		DESC_BLOCK_DATA_T *suiv=bloc->User_Block.Next_Block;
		int tailleBloc = bloc->User_Block.Nr_Of_Bytes;
										/* Pas de buffer pour le premier
										 * bloc.	*/
			if(0!=blocCour)			
			{
				usr_fip_mess_free(NULL,(void *)bloc->User_Block.Ptr_Data);
			}							/* endif(0!=blocCour				*/
//	 	   	printf("Liberation du bloc(%d/%d) a l'adresse %#0x(MsgDesc %#0x : rep %#x) taille %d \n",
//	 	   		blocCour,mess->reponse->Nr_Of_Blocks,bloc,&msgDesc,mess->reponse,tailleBloc);
			usr_fip_mess_free(NULL,(void *)bloc);
//	 	   	printf("Apres liberation du bloc \n");
			bloc=suiv;
		} 								/* endfor(blocCour=0;blocCour<me	*/
		usr_fip_mess_free(NULL,(void *)mess->reponse);
		mess->reponse=NULL;
// 	   	printf("Apres liberation de la reponse\n");
	}
										/* Liberation de la memoire pour le
										 * buffer d'entree.	*/

	switch(mess->type)
	{
	case 1:
		usr_fip_mess_liberer_fdm(mess);
										/* Liberation buffer d'entree FDM	*/
		break;
	case 2:
		usr_fip_mess_liberer_mcs(mess);
										/* Liberation buffer d'entree MCS	*/
		break;
	default:
		break;
	}									/* endswitch (mess->type			*/
										/* Liberation de la memoire pour le
										 * buffer de sortie	*/
	if(0)
	{
	printDebug("Liberation de la reponse \n");

	if(NULL!=mess->reponse)
	{
	DESC_BLOCK_DATA_T *bloc=mess->reponse->Ptr_Block;
	int	blocCour;
										/* Liberation de tous les blocs.	*/
		for(blocCour=0;blocCour<mess->reponse->Nr_Of_Blocks;blocCour++)
		{
		DESC_BLOCK_DATA_T *suiv=bloc->User_Block.Next_Block;
		int tailleBloc = bloc->User_Block.Nr_Of_Bytes;
										/* Pas de buffer pour le premier
										 * bloc.	*/
			if(0!=blocCour)			
			{
				usr_fip_mess_free(NULL,(void *)bloc->User_Block.Ptr_Data);
			}							/* endif(0!=blocCour				*/
	 	   	printf("Liberation du bloc(%d/%d) a l'adresse %#0x(MsgDesc %#0x : rep %#x) taille %d \n",
	 	   		blocCour,mess->reponse->Nr_Of_Blocks,bloc,&msgDesc,mess->reponse,tailleBloc);
			usr_fip_mess_free(NULL,(void *)bloc);
 	   	printf("Apres liberation du bloc\n");
			bloc=suiv;
		} 								/* endfor(blocCour=0;blocCour<me	*/
		usr_fip_mess_free(NULL,(void *)mess->reponse);
		mess->reponse=NULL;
 	   	printf("Apres liberation de la reponse\n");
	}
	}
										/* Cas particulier du message du 
										 * message periodique.				*/
	if(mess==&usr_fip_mess_periodique)
	{
		usr_fip_mess_init_debut(mess);
	}
	else if(mess==&usr_fip_mess_st_al)
	{
		usr_fip_mess_init_debut(mess);
	}
										/* --------------------------------	*/
										/* FIN DE usr_fip_mess_liberer		*/
										/* --------------------------------	*/
}

void usr_fip_mess_init_debut(T_usr_fip_mess *mess)
{
unsigned long temp;
	*(unsigned long *)(&mess->buffer[0])=htonl(mess->ident);
	*(unsigned long *)(&mess->buffer[4])=htonl(mess->dest);
		mess->block.Nr_Of_Bytes=8;
}

/* --------------------------------
 * usr_fip_mess_enlever
 * ====================
 * Il s'agit de d'enlever le message
 * de la liste des messages en cours
 * de traitement. Attention, le message
 * peut toujours etre utilise (jusqu'e
 * reception d'un acquitement de 
 * transfert.
 * Entree :
 * - le message a enlever.
 * --------------------------------	*/

void usr_fip_mess_enlever(T_usr_fip_mess *mess)
{
int indice=0;
	for(indice=0;indice<USR_FIP_MAX_MESSAGE;indice++)
	{
		if(usr_fip_mess_ordre[indice]==mess->index)
		{
			break;
		}								/* endif(usr_fip_mess_ordre			*/
	}									/* endfor(indice=0					*/
	for(;indice<(USR_FIP_MAX_MESSAGE-1);indice++)
	{
		usr_fip_mess_ordre[indice]=usr_fip_mess_ordre[indice+1];
	}
	if(indice<USR_FIP_MAX_MESSAGE)
	{
		usr_fip_mess_ordre[indice]=-1;
	}
										/* --------------------------------	*/
										/* FIN DE usr_fip_mess_enlever		*/
										/* --------------------------------	*/
}

/* --------------------------------
 * usr_fip_mess_lire_premier
 * =========================
 * Retour :
 * - adresse du message ou NULL si
 *  il n'y a pas de message en cours.
 * --------------------------------	*/

T_usr_fip_mess * usr_fip_mess_lire_premier(void)
{
T_usr_fip_mess *mess=NULL;
	if(usr_fip_mess_ordre[0]!=-1)
	{
	int indice;
		mess=&usr_fip_tab_mess[usr_fip_mess_ordre[0]];
	}									/* endif(usr_fip_mess_ordre			*/
										/* --------------------------------	
										 * FIN DE usr_fip_mess_lire_premier
										 * --------------------------------	*/
	usr_fip_mess_courant=mess;
	return(mess);
}

/* --------------------------------
 * usr_fip_mess_lire_courant
 * =========================
 * Pour obtenir si elles existent
 * les informations sur le message
 * en cours.
 * --------------------------------	*/
 
T_usr_fip_mess * usr_fip_mess_lire_courant(void)
{
	return(usr_fip_mess_courant);
}

/* --------------------------------
 * usr_fip_mess_ecrire_cmd
 * =======================
 * Ecriture de la commande dans
 * le buffer.
 * Retour :
 * - 0 si tout est OK, -1 sinon.
 * --------------------------------	*/

int usr_fip_mess_ecrire_cmd( T_usr_fip_mess   *mess,FDM_MSG_RECEIVED *bloc)
{
int retour=-1;
	if(mess!=NULL)
	{
	FDM_MSG_R_DESC	*cour=bloc->Ptr_Block;
	int				 blocCour;
										/* Message de type FDM			*/
		mess->type=1;
										/* Calcul de la taille totale	*/
		mess->tailleCmd=0;
		for(blocCour=0;blocCour<bloc->Nr_Of_Blocks;blocCour++)
		{
			mess->tailleCmd+=cour->Nr_Of_Bytes;
			cour=cour->Next_Block;
		}
										/* Pointeur sur le message		*/	
		mess->commande=(void *)bloc;
	
		clock_gettime(CLOCK_REALTIME,&mess->dateCmd);
										/* Copier ici les bits de 4 e 7
										 * pour recuperer l'adresse du 
										 * destinataire. */
										/* Recuperation de l'adresse 
										 * destinataire	*/
		{
		unsigned long temp;
			usr_fip_mess_copier(mess,(char *)&temp,0,4);
			mess->ident=ntohl(temp);
			usr_fip_mess_copier(mess,(char *)&temp,4,4);
			mess->dest=ntohl(temp);
		}
										/* Recopier identificateur et
										 * destinataire dans le buffer. */
		usr_fip_mess_copier(mess,(char *)&mess->buffer[0],0,8);
		mess->block.Nr_Of_Bytes=8;
										/* --------------------------------
										 * TRAITEMENT ACQUITEMENT
										 * --------------------------------	*/
		if(usr_fip_mess_tester_acquitement(mess))
		{
			usr_fip_mess_envoyer_acquitement(mess);
		}								/* endif(usr_fip_mess_tester_acqu	*/
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_ecrire
										 * --------------------------------	*/
	return retour;
}

/* --------------------------------
 * usr_fip_mess_ecrire_cmd_mcs
 * ===========================
 * Ecriture de la commande dans
 * le buffer d'une commande MCS.
 * Retour :
 * - 0 si tout est OK, -1 sinon.
 * --------------------------------	*/

int usr_fip_mess_ecrire_cmd_mcs(T_usr_fip_mess   *mess,MCS_SDU_RECEIVED *msg)
{
int retour=-1;
	if(mess!=NULL)
	{
										/* Message de type MCS			*/
		mess->type=2;
										/* Calcul de la taille totale	*/
		mess->tailleCmd=msg->SDU_Size;
										/* Pointeur sur le message		*/	
		mess->commande=(unsigned char *)msg;
	
		clock_gettime(CLOCK_REALTIME,&mess->dateCmd);
										/* Copier ici les bits de 4 e 7
										 * pour recuperer l'adresse du 
										 * destinataire. */
										/* Recuperation de l'adresse 
										 * destinataire	*/
		{
		unsigned long temp;
			usr_fip_mess_copier(mess,(char *)&temp,0,4);
			mess->ident=ntohl(temp);
			usr_fip_mess_copier(mess,(char *)&temp,4,4);
			mess->dest=ntohl(temp);
		}
										/* Recopier identificateur et
										 * destinataire dans le buffer. */
		usr_fip_mess_copier(mess,(char *)&mess->buffer[0],0,8);
		mess->block.Nr_Of_Bytes=8;
										/* --------------------------------
										 * TRAITEMENT ACQUITEMENT
										 * --------------------------------	*/
		if(usr_fip_mess_tester_acquitement(mess))
		{
			usr_fip_mess_envoyer_acquitement(mess);
		}								/* endif(usr_fip_mess_tester_acqu	*/
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_ecrire
										 * --------------------------------	*/
	return retour;
}

/* --------------------------------
 * usr_fip_mess_ecrire_rep
 * =======================
 * Ecriture de la reponse dans le
 * buffer. La fonction retourne 1 si 
 * la commande est finie (acquit
 * trouve) et -1 si la taille est
 * depassee.
 * Retour :
 * - 0 si tout est OK, -1 sinon.
 * --------------------------------	*/

int usr_fip_mess_ecrire_rep(T_usr_fip_mess *mess,char *buffer,int taille)
{
int retour=0;
int carCour=0;

	if(mess!=NULL)
	{
										/* On precise une nouvelle date
										 * pour prolonger l'attente et 
										 * limiter le timeout.	*/
		clock_gettime(CLOCK_REALTIME,&mess->dateCmd);

		if(((mess!=&usr_fip_mess_periodique)&&
			(mess!=&usr_fip_mess_st_al))||(0==mess->utilise))
		{
		while(
				(USR_FIP_MCS_SIZE>mess->mess.Ptr_Block->Nr_Of_Bytes)
				&&(carCour<taille)&&(0==retour))
		{
			if('\\'==buffer[carCour])
			{
				retour=2;
			}
			else if('!'==buffer[carCour])
			{
										/* Traitement du cas de l'acquit court
										 */
				if(8==mess->mess.Ptr_Block->Nr_Of_Bytes)
				{
					usr_fip_mess_ecrire_car(mess,buffer[carCour++]);
				}
				retour=1;
			}
			else
			{
				if('?'==buffer[carCour])
				{
					retour=2;
				}
				else if('%'==buffer[carCour])
				{
					retour=2;
										/* On essaie d'ecrire le code 
										 * d'erreur. */
					if((USR_FIP_MCS_SIZE>
							(mess->mess.Ptr_Block->Nr_Of_Bytes+1))
						&&((carCour+1)<taille))
					{
						usr_fip_mess_ecrire_car(mess,buffer[carCour++]);
					} 					/* endif((USR_FIP_MESS_SIZE>		*/
				}						/* endif('?							*/
				usr_fip_mess_ecrire_car(mess,buffer[carCour++]);
			} 							/* endif('!'==buffer[carCour])		*/
		}								/* endwhile(						*/
		}
	}									/* endif(mess!=NULL					*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_ecrire_rep
										 * --------------------------------	*/
	return retour;
}

/* ----------------------------------
 * usr_fip_mess_obtenir_commande
 * =============================
 * La fonction permet d'obtenir la
 * commande dans un buffer pour
 * pouvoir assurer la gestion des
 * traces.
 * Attention, le buffer doit
 * etre libere apres utilisation.
 * --------------------------------	*/

char *usr_fip_mess_obtenir_commande(T_usr_fip_mess *mess,int *longueur)
{
char *commande=NULL;
	if(NULL!=mess)
	{
	int taille=mess->tailleCmd-8;
		commande=(char *)usr_fip_mess_alloc(NULL,taille);
		if(NULL!=commande)
		{
			usr_fip_mess_copier(mess,commande,8,taille);
			*longueur=taille;
		}								/* endif(NULL!=buffer				*/
	}									/* endif(NULL!=mess					*/
										/* -------------------------------
										 * FIN DE usr_fip_mess_obtenir_com
										 * --------------------------------	*/
	return commande;
}

/* --------------------------------
 * usr_fip_mess_envoyer
 * ====================
 * Envoyer le message 
 * Retour :
 * - la fonction retourne -1 en cas 
 * 	 d'erreur.
 * --------------------------------	*/

int usr_fip_mess_envoyer(T_usr_fip_mess *mess,int fd)
{
int retour=-1;
	if(mess!=NULL)
	{
	unsigned short taille;
										/* Allocation du buffer				*/
		taille=mess->tailleCmd-8;
		/*if(MAX_MESSAGE_LCR>=taille)*/
		{
		char		  *buffer;
			buffer=(char *)usr_fip_mess_alloc(NULL,taille+4);
			if(NULL!=buffer)
			{
										/* Il faudrait ici introduire
										 * quelquechose permettant de 
										 * traiter la reception de messages
										 * MCS.	*/
				usr_fip_mess_copier(mess,&buffer[2],8,mess->tailleCmd-8); 
				buffer[taille+2]=0x0d; 
				buffer[taille+3]=0; 
				taille++;
				memcpy(&buffer[0],(char *)&taille,2);
				printDebug("Taille du buffer %d\n",taille+2);
//				printDebug("Message %s et %#x %#x\n",&buffer[2],
//						buffer[taille],buffer[taille+1]);
				if(NULL!=usr_fip_mess_ana_sys)
				{
					usr_fip_mess_ana_sys(mess,fd,buffer,taille+2);
					retour=0;
				}
				else
				{
					printDebug(
						"Erreur, fonction usr_fip_mess_ana_sys non definie\n");
				}						/* endif(NULL!=usr_fip_ana_sys		*/
				
				
				usr_fip_mess_free(NULL,(void *)buffer);
				
				
				
			}
			else
			{
				printDebug(
						"Erreur d'allocation du buffer (taille %d)\n",taille);
				retour=-1;
			} 							/* endif(NULL!=buffer)				*/
		}
		/* else
		{
			retour=-1;
		} */							/* endif(NULL!=buffer)				*/
	}
	else
	{
		printDebug("Le message est null, je ne peux rien faire\n");
	} 									/* endif(NULL!=mess					*/
										/* --------------------------------
										 * FIN DE usr_fip_mess_envoyer
										 * --------------------------------	*/
	return(retour);
}

/* --------------------------------
 * usr_fip_mess_query_timeout
 * ==========================
 * La fonction permet de savoir si
 * le message est en timeout.
 * --------------------------------	*/

int usr_fip_mess_query_timeout(T_usr_fip_mess *mess,struct timespec *date)
{
int retour=0;					
										/* Calcul de la difference de dates	*/
	if(date->tv_sec>mess->dateCmd.tv_sec)
	{
	int diffSec=(int)difftime(date->tv_sec,mess->dateCmd.tv_sec);
	int diffMilli=(mess->dateCmd.tv_nsec-date->tv_nsec)/1000000;
	int diff=diffSec*1000+diffMilli;
		if(USR_FIP_MESS_MAX_WAIT<diff)
		{
			printDebug("Timeout de usr_fip_mess_query_timeout\n");
			retour=1;
		}
	}
	else if(date->tv_sec==mess->dateCmd.tv_sec)
	{
		if(date->tv_nsec>mess->dateCmd.tv_nsec)
		{
		int diffMilli=(mess->dateCmd.tv_nsec-date->tv_nsec)/1000000;
			if(USR_FIP_MESS_MAX_WAIT<diffMilli)
			{
				printDebug("Timeout de usr_fip_mess_query_timeout\n");
				retour=1;
			}
		}
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_query_timeout
										 * --------------------------------	*/
	return(retour);
}

/* --------------------------------
 * usr_fip_mess_get_dest
 * =====================
 * Obtention du champ destinataire.
 * --------------------------------	*/

unsigned long usr_fip_mess_get_dest(T_usr_fip_mess *mess)
{
	return(mess->dest);
}

/* --------------------------------
 * usr_fip_mess_set_dest
 * =====================
 * Obtention du champ destinataire.
 * --------------------------------	*/

void usr_fip_mess_set_dest(T_usr_fip_mess *mess,unsigned long dest)
{
	if(NULL!=mess)
	{
		mess->dest=dest;
	}
}

/* --------------------------------
 * usr_fip_mess_get_ident
 * =====================
 * Obtention du champ identification
 * --------------------------------	*/

unsigned long usr_fip_mess_get_ident(T_usr_fip_mess *mess)
{
	return(mess->ident);
}

/* --------------------------------
 * usr_fip_mess_set_ident
 * =====================
 * Obtention du champ ident.
 * --------------------------------	*/

void usr_fip_mess_set_ident(T_usr_fip_mess *mess,unsigned long ident)
{
	if(NULL!=mess)
	{
		mess->ident=ident;
	}
}

/* --------------------------------
 * usr_fip_mess_copier_mcs
 * =======================
 * --------------------------------	*/

static void usr_fip_mess_copier_mcs(
		T_usr_fip_mess *mess,char *buffer,
		int debut,int longueur)
{
int 				 blocCour=0;
MCS_SDU_RECEIVED 	*msg=(MCS_SDU_RECEIVED *)mess->commande;
int 				 nbBlocs=msg->Nr_Of_Blocks;
DESC_BLOCK_DATA_R	*bloc=msg->Ptr_Block;
int 				 indCour=debut;
int 				 lgCour=0;
int					 debutBloc=0;
	for(blocCour=0;(blocCour<nbBlocs)&&(lgCour<longueur);blocCour++)
	{
//		printDebug("adresse du bloc %#0x (%d)\n",bloc->Ptr_Data,bloc->Nr_Of_Bytes);
		for(;indCour<(debutBloc+bloc->Nr_Of_Bytes)&&(lgCour<longueur);
				indCour++,lgCour++)
		{
			buffer[lgCour]=bloc->Ptr_Data[indCour-debutBloc];
		}								/* endfor(;indCour<					*/
		debutBloc+=bloc->Nr_Of_Bytes;
		bloc=bloc->Next_Block;
//		/* TODO : Sortie premature de la recopie.  a changer....*/
//		break;
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_copier_mcs
										 * --------------------------------	*/
}

static void usr_fip_mess_copier_fdm(
		T_usr_fip_mess *mess,char *buffer,
		int debut,int longueur)
{
int 				 blocCour=0;
FDM_MSG_RECEIVED 	*msg=(FDM_MSG_RECEIVED *)mess->commande;
int 				 nbBlocs=msg->Nr_Of_Blocks;
FDM_MSG_R_DESC	    *bloc=msg->Ptr_Block;
int 				 indCour=debut;
int 				 lgCour=0;
int					 debutBloc=0;
	for(blocCour=0;(blocCour<nbBlocs)&&(lgCour<longueur);blocCour++)
	{
		for(;indCour<(debutBloc+bloc->Nr_Of_Bytes)&&(lgCour<longueur);
				indCour++,lgCour++)
		{
			buffer[lgCour]=bloc->Ptr_Data[indCour-debutBloc];
		}								/* endfor(;indCour<					*/
		debutBloc+=bloc->Nr_Of_Bytes;
		bloc=bloc->Next_Block;
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_copier_fdm
										 * --------------------------------	*/
}

/* --------------------------------
 * usr_fip_mess_copier
 * ===================
 * Copie d'informations recoltees 
 * dans le buffer d'entree (MCS ou
 * FDM) dans un buffer passe en para-
 * metre.
 * Entree :
 * -	le message a traiter,
 * -	pointeur sur le buffer dest,
 * -	index du premier octet e
 * 		copier,
 * - 	nombre d'octets a copier
 * --------------------------------	*/

static void usr_fip_mess_copier(T_usr_fip_mess *mess,char *buffer,
		int debut,int longueur)
{
	if(NULL!=mess)
	{
		switch(mess->type)
		{
		case 1:
			usr_fip_mess_copier_fdm(mess,buffer,debut,longueur);
			break;
		case 2:
			usr_fip_mess_copier_mcs(mess,buffer,debut,longueur);
			break;
		default:
			break;
		}								/* endif(switch(mess->type			*/
	}									/* endif(NULL!=mess					*/
}

/* --------------------------------
 * usr_fip_mess_ecrire_car
 * =======================
 * Ecriture d'un caractere dans
 * le buffer de reponse.
 * --------------------------------	*/

static int usr_fip_mess_ecrire_car(T_usr_fip_mess *mess,char car)
{
int retour=-1;
										/* Dans le premier cas, nous sommes	
										 * encore en presence d'un message
										 * FDM.	*/
	if(USR_FIP_MESS_SIZE>mess->mess.Ptr_Block->Nr_Of_Bytes)
	{
		mess->buffer[mess->mess.Ptr_Block->Nr_Of_Bytes++]=car;
		retour=0;
	}
	else if(USR_FIP_MESS_SIZE==mess->mess.Ptr_Block->Nr_Of_Bytes)
	{
										/* La longueur aidant, nous en venons
										 * e un message de type MCS. */
										/* Allocation du message */
	MCS_SDU_TO_SEND *msg=(MCS_SDU_TO_SEND *)usr_fip_mess_alloc(NULL,sizeof(MCS_SDU_TO_SEND));
		if(NULL!=msg)
		{
										/* Allocation du premier bloc pour
										 * le message MCS qui fait 256 octets*/
		DESC_BLOCK_DATA_T *bloc=
			(DESC_BLOCK_DATA_T *)usr_fip_mess_alloc(NULL,sizeof(DESC_BLOCK_DATA_T));
			if(NULL!=bloc)
			{
			DESC_BLOCK_DATA_T *blocSuiv=
				(DESC_BLOCK_DATA_T *)usr_fip_mess_alloc(NULL,sizeof(DESC_BLOCK_DATA_T));
				if(NULL!=blocSuiv)
				{
				unsigned char *data=
					(unsigned char *) usr_fip_mess_alloc(NULL,USR_FIP_MCS_BUFFER_SIZE);
					if(NULL!=data)
					{
						mess->reponse=msg;
						msg->Ptr_Block=bloc;
						msg->Nr_Of_Blocks=2;
						bloc->User_Block.Next_Block=blocSuiv;
						bloc->User_Block.Ptr_Data=&mess->buffer[0];
						bloc->User_Block.Nr_Of_Bytes=USR_FIP_MESS_SIZE;
						blocSuiv->User_Block.Next_Block=NULL;
						blocSuiv->User_Block.Nr_Of_Bytes=0;
						blocSuiv->User_Block.Ptr_Data=data;
						data[blocSuiv->User_Block.Nr_Of_Bytes++]=car;
						mess->mess.Ptr_Block->Nr_Of_Bytes++;
						retour=0;
					}
					else
					{
						usr_fip_mess_free(NULL,(void *)blocSuiv);
						usr_fip_mess_free(NULL,(void *)bloc);
						usr_fip_mess_free(NULL,(void *)msg);
					}
				}
				else
				{
					usr_fip_mess_free(NULL,(void *)bloc);
					usr_fip_mess_free(NULL,(void *)msg);
				}
			}
			else
			{
				usr_fip_mess_free(NULL,(void *)msg);
			}
		}
										/* Allocation du bloc suivant de 
										 * 16320 octets. */
	}
	else if(USR_FIP_MCS_SIZE>mess->mess.Ptr_Block->Nr_Of_Bytes)
	{
										/* Ecriture dans un buffer MCS	*/
	
	MCS_SDU_TO_SEND 	*msg=mess->reponse;
	DESC_BLOCK_DATA_T 	*bloc=msg->Ptr_Block;
	int numBloc=0;
		while(NULL!=bloc->User_Block.Next_Block)
		{
			bloc=bloc->User_Block.Next_Block;
		}
		if(USR_FIP_MCS_BUFFER_SIZE==bloc->User_Block.Nr_Of_Bytes)
		{					
										/* Le buffer est plein, il faut
										 * en allouer un nouveau.	*/
			if(USR_FIP_MCS_NB_BUFFER>msg->Nr_Of_Blocks)
			{
			DESC_BLOCK_DATA_T *blocSuiv=
				(DESC_BLOCK_DATA_T *)usr_fip_mess_alloc(NULL,sizeof(DESC_BLOCK_DATA_T));
				if(NULL!=blocSuiv)
				{
				unsigned char *data=
					(unsigned char *)usr_fip_mess_alloc(NULL,USR_FIP_MCS_BUFFER_SIZE);
					if(NULL!=data)
					{
						msg->Nr_Of_Blocks++;
						bloc->User_Block.Next_Block=blocSuiv;
						blocSuiv->User_Block.Next_Block=NULL;
						blocSuiv->User_Block.Nr_Of_Bytes=0;
						blocSuiv->User_Block.Ptr_Data=data;
						data[blocSuiv->User_Block.Nr_Of_Bytes++]=car;
						mess->mess.Ptr_Block->Nr_Of_Bytes++;
						retour=0;
					}
					else
					{
						usr_fip_mess_free(NULL,(void *)blocSuiv);
					}
				}
			} 							/* endif(USR_FIP_MCS_NB_BUFFER		*/
		}
		else
		{
			bloc->User_Block.Ptr_Data[bloc->User_Block.Nr_Of_Bytes++]=car;
			mess->mess.Ptr_Block->Nr_Of_Bytes++;
			retour=0;
		}
	}
	else
	{
										/* La longueur max pour un message
										 * MCS est depassee.	*/
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_ecrire_car
										 * --------------------------------	*/
	return retour;
}

/* --------------------------------
 * usr_fip_mess_send_message_mcs
 * ================================
 * La procedure permet de forcer even-
 * tuellement l'envoi du message par 
 * MCS.
 * ------------------------------------	*/

void usr_fip_mess_send_message_mcs(T_usr_fip_mess *mess,int mcs)
{
	if(((mess==&usr_fip_mess_periodique)
	   ||(mess==&usr_fip_mess_st_al))&&(1==mess->utilise))
	{
		printDebug("Le message est en cours de transmission\n");
	}
	else
	{
		if(8!=mess->mess.Ptr_Block->Nr_Of_Bytes)
		{
			if(USR_FIP_MESS_SIZE<mess->mess.Ptr_Block->Nr_Of_Bytes)
			{
			_MCS_ERROR_CODE_LIST report;
										/* Il s'agit d'un message long		*/
				report=mcs_send_message(
						usr_fip_mcs_cmd.channelmcs,mess->reponse);
			}
			else if(1==mcs)				/* Cas du forcage MCS.				*/
			{
			MCS_SDU_TO_SEND *msg=
				(MCS_SDU_TO_SEND *)usr_fip_mess_alloc(NULL,sizeof(MCS_SDU_TO_SEND));
				if(NULL!=msg)
				{
										/* Allocation du premier bloc pour
										 * le message MCS qui fait 256 octets*/
				DESC_BLOCK_DATA_T *bloc=
					(DESC_BLOCK_DATA_T *)usr_fip_mess_alloc(NULL,sizeof(DESC_BLOCK_DATA_T));
					if(NULL!=bloc)
					{
					_MCS_ERROR_CODE_LIST report;
						mess->reponse=msg;
						msg->Ptr_Block=bloc;
						msg->Nr_Of_Blocks=1;
						bloc->User_Block.Next_Block=NULL;
						bloc->User_Block.Ptr_Data=&mess->buffer[0];
						bloc->User_Block.Nr_Of_Bytes=
							mess->mess.Ptr_Block->Nr_Of_Bytes;
										/* Il s'agit d'un message long		*/
						report=mcs_send_message(
								usr_fip_mcs_cmd.channelmcs,mess->reponse);
					}
					else
					{
						usr_fip_mess_free(NULL,(void *)msg);
					}
				}
			}
			else
			{
				printDebug("Le message est envoye %#08x %#08x %s\n",
					*(unsigned long *)&mess->buffer[0],
					*(unsigned long *)&mess->buffer[4],
					&mess->buffer[8]);
										/* Il s'agit d'un message court		*/
				fdm_send_message(usr_fip_contexte_msg.MsgRef,&mess->mess);
			}							/* endif(USR_FIP_MESS				*/
			if(mess==&usr_fip_mess_periodique)
			{
										/* Si le message est periodique, on
										 * signale que l'on  est dans la 
										 * phase d'emission.	*/
				mess->utilise=1;
			}
			else if(mess==&usr_fip_mess_st_al)
			{
										/* Si le message est celui utilise pour
										 * le st_al on signale que l'on  est 
										 * dans la phase d'emission.	*/
				mess->utilise=1;
			}
			else
			{
										/* On enleve le message de la liste
										 * des messages en cours de 
										 * traitement.	*/
				usr_fip_mess_enlever(mess);	
			}
		}
		else
		{
										/* Il n'y a rien e envoyer.			*/
			if(mess==&usr_fip_mess_periodique)
			{
										/* Si le message est periodique, on
										 * signale que l'on  est dans la 
										 * phase d'emission.	*/
				mess->utilise=1;
			}
			else if(mess==&usr_fip_mess_st_al)
			{
										/* Si le message est celui utilise pour
										 * le st_al on signale que l'on  est 
										 * dans la phase d'emission.	*/
				mess->utilise=1;
			}
			else
			{
										/* On enleve le message de la liste
										 * des messages en cours de 
										 * traitement.	*/
				usr_fip_mess_enlever(mess);	
				usr_fip_mess_liberer(mess);
			}
		}
	}
										/* --------------------------------
										 * FIN DE usr_fip_mess_send_mess
										 * --------------------------------	*/
}

/* --------------------------------
 * usr_fip_mess_send_message
 * =========================
 * Emission d'un message. La procedure
 * determine le type de canal a utiliser
 * MCS ou FDM.
 * --------------------------------	*/

void usr_fip_mess_send_message(T_usr_fip_mess *mess)
{
										/* Pas de forcage MCS par defaut.	*/
	usr_fip_mess_send_message_mcs(mess,0);
										/* --------------------------------
										 * FIN DE usr_fip_mess_send_mess
										 * --------------------------------	*/
}

/* ********************************	*/
/* FIN DE usr_fip_mess.c			*/
/* ********************************	*/
