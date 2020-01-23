/************************************************************
   
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcs.h
  Description     : Interface  utilisateur ( procedures et Classes ),
					et Interfaces internes (procedures)
                   
 
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 		08.01.2000 
  Date de modification    : 16.06.2000 
		DE 3 : utilisation messagerie mode normalise ou non acquitte
			   ajout enum 	_MCS_MSG_MODE_ACK
		DE 2 : optimisation de la memoire: 
			   ajout creation e t destruction dynamique des conteste
************************************************************************/


#ifndef __mcs_h
#define __mcs_h

#include "fdm.h"
#include "mcs_opt.h"
#include "mcs_os.h"


/* suite a DE 2 */
#define  MCS_WITH_COMMAND_CHANNELS   	_MCS_OPTION_YES
#define  MCS_WITH_CONFIGUSER_SPECIF     _MCS_OPTION_YES

/*!!!!!!!!!!!!!!!!!!!!!! AVERTISSEMENT !!!!!!!!!!!!!!!!!!

Dans ce fichiers INCLUDE, on distingue 
	des zones PUBLIQUES, 
contenant des description types, enums, prototypes de fonctions
concernant les utilisateurs de ce logiciel, 
      et des zones PRIVEES 

contenant des description types, enums, prototypes de fonctions
ne concernant par directement les utilisateurs de ce logiciel, 

Note: il se peut que des types soient manipules par les modules 
utilisateurs, alors que leur contenu ne concerne que le module MCS.
Dans ce cas, il sont decrits en zone PRIVEE.

Les zones sont distinguees par des separateurs du type: 
#########################################################
      PUBLIC INCLUDE AREA / PRIVATE INCLUDE AREA
#########################################################

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/




/*#########################################################
      PUBLIC INCLUDE AREA
###########################################################*/

/*========== Version du logiciel MCS ======================*/
typedef struct   { 

       char        Version;
       char        Revision ;
       char		   Proto;		/*TRUE  = 1 =>version proto
								 FALSE = 0 =>version officielle*/
       char        ProtoNr ;   /* significatif  si Proto=TRUE */
       char        Date[3] ;   /* Annee, Mois, Jour */   

} MCS_VERSION ;


/*#########################################################
      PUBLIC INCLUDE AREA
###########################################################*/
typedef enum   {
    _MCS_CR_OK	 = 0 , 
    _MCS_CR_NOK
}_MCS_CODE_CR;

/*========== Valeurs des comptes-rendus des fonctions =====*/

typedef enum   {
    _MCS_OK	= 0 , 

    _MCS_NOK	, 

    _MCS_INTERNAL_ERROR				=2,

    _MCS_CREATE_ILLEGAL_PARAMETER	=0x10,
    _MCS_CREATE_ALLOCATE_MEMORY_FAULT ,
    _MCS_CREATE_CREATE_SEMAPHORE_FAULT ,

	_MCS_CHANNEL_CREATE_ON_ILLEGAL_INSTANCE,
	_MCS_CHANNEL_CREATE_ILLEGAL_USERCHANNEL_REF,
	_MCS_CHANNEL_CREATE_ILLEGAL_CONNEXION_TEMPO,
	_MCS_CHANNEL_CREATE_ALLOCATE_MEMORY_FAULT,
	_MCS_CHANNEL_CREATE_ILLEGAL_DLL_SIZE ,
    _MCS_CHANNEL_CREATE_ILLEGAL_SERVICE_TYPE ,
    _MCS_CHANNEL_CREATE_ILLEGAL_SDU_TYPE ,
    _MCS_CHANNEL_CREATE_ILLEGAL_STUFFING_MODE ,
    _MCS_CHANNEL_CREATE_ILLEGAL_CALL_BACK_FUNCTION ,
    _MCS_CHANNEL_CREATE_ILLEGAL_LOCAL_ADDRESS ,
    _MCS_CHANNEL_CREATE_ILLEGAL_REMOTE_ADDRESS ,
	_MCS_CHANNEL_CREATE_ILLEGAL_CHANNEL_NR,
    _MCS_CHANNEL_CREATE_ILLEGAL_SDU_SIZE ,
    _MCS_CHANNEL_CREATE_ILLEGAL_NUMBER_OF_PARALLEL_SERVICES ,
    _MCS_CHANNEL_CREATE_ILLEGAL_ANTICIPATION ,
	_MCS_CHANNEL_CREATE_ILLEGAL_REFUSED_DLL_CTXT_CREATION,
	_MCS_CHANNEL_START_ON_INEXISTING_CHANNEL ,
	_MCS_CHANNEL_DELETE_ON_INEXISTING_CHANNEL ,
									
	_MCS_CHANNEL_COMMAND_ON_ILLEGAL_INSTANCE =0x40,	/* ajout suite DE 2*/
    _MCS_CHANNEL_COMMAND_ILLEGAL_CHANNEL_STATE ,	/* ajout suite DE 2*/
    _MCS_CHANNEL_COMMAND_ILLEGAL_SERVICE ,			/* ajout suite DE 2*/
    _MCS_CHANNEL_COMMAND_ILLEGAL_PARAMETERS ,		/* ajout suite DE 2*/

    _MCS_SEND_DATA_ON_INEXISTING_CHANNEL =0x100,
    _MCS_SEND_DATA_ON_NOT_RUNNING_CHANNEL ,
    _MCS_SEND_DATA_ON_IDLE_CHANNEL ,
    _MCS_SEND_DATA_ON_DELETE_IN_PROGRESS_CHANNEL , 
    _MCS_SEND_DATA_INVALID_LENGTH ,
    _MCS_SEND_DATA_INVALID_DESTINATION ,
    _MCS_SEND_DATA_DLL_FAIL ,
  
    _MCS_RECEIVE_DLL_ON_INEXISTING_CHANNEL =0x200,
    _MCS_RECEIVE_DLL_ON_IDLE_CHANNEL ,
    _MCS_RECEIVE_DLL_ON_DELETE_IN_PROGRESS_CHANNEL ,
    _MCS_RECEIVE_DLL_INVALID_PDU ,
    _MCS_RECEIVE_DLL_BROKEN_MESSAGE ,


    _MCS_CHANNEL_CONNECTED_LACK_OF_MEMORY_FOR_USER_IND =0x400,
    _MCS_CHANNEL_CONNECTED_ERROR_IN_PROTOCOL_EXCHANGE ,
    _MCS_CHANNEL_CONNECTED_BROKEN_SEQUENCE ,


    _MCS_SEND_DLL_ON_INEXISTING_CHANNEL =0x500,
    _MCS_SEND_DLL_ON_IMAGE2 , 
    _MCS_SEND_DLL_INTERNAL_ERROR ,
    _MCS_SEND_DLL_UNKNOWN_ERROR ,

    _MCS_END_OF_ERROR_CODE_LIST


} _MCS_ERROR_CODE_LIST ;		


/*==== enumeration pour la liste des defauts possibles, dans 
====== une connexion de type association (A_DATA) =========*/ 

typedef enum  {
   _CONNEXION_FAILURE_In_ProtocoleExchange , 
   _CONNEXION_FAILURE_BrokenSequence , 
   _CONNEXION_FAILURE_SDUTransferError , 
   _CONNEXION_FAILURE_PDUTransferError , 
   /* les 2 champs suivants ne sont pas utilise si  _MCS_WITH_STUFFING_MODE =NO*/
   _CONNEXION_FAILURE_NoPDUTransmission ,
   _CONNEXION_FAILURE_NoPDUReception		
} _CONNEXION_FAILURE_LIST;

/*Note: ces defauts sont produits par la couche transport MCS*/



struct _MCS_REF;
struct _MCSCHANNEL_REF;
struct _MCS_SDU_TO_SEND;
struct _MCS_SDU_RECEIVED;
struct _MCS_CHANNEL_COMMAND;

/*#########################################################
      PUBLIC INCLUDE AREA
###########################################################*/


/*########### OBJET CANAL MCS #################
Objet se trouvant chez MCS, 
--- Partie configurateur des canaux:
USER demande a  MCS la creation  et demarrage des canaux

--- Partie emission reception msg segmentes:
USER parle au canal MCS pour lui demander des transferts de SDU
Le canal parle a USER pour lui indiquer/confirmer transferts de SDU
L'objet CANAL MCS a 2 images:

    vue de MCS(chez lui)  :          MCSCHANNEL_REF
    vue de USER(chez lui) :     USER_MCSCHANNEL_REF

USER  parle a canal en utilisant  -> MCSCHANNEL_REF
canal parle a USER  en utilisant  -> USER_MCSCHANNEL_REF
#########################################################################*/

/* type USER_MCSCHANNEL_REF :
Utilise pour la communication de MCS vers Appli USER,pour l'objet CANAL MCS.*/
typedef unsigned long * USER_MCSCHANNEL_REF;

/* type ALL_MCSCHANNEL_REFS :
Structure qui vehicule l'ensemble des references d'un CANAL MCS,
vu des 2 modules logiciels qui le maniplulent: 
	 - le logiciel applicatif (USER)
     - le logiciel MCS
*/
typedef struct {
           USER_MCSCHANNEL_REF           UserChannelRef;
           struct _MCSCHANNEL_REF        *McsChannelRef;
} ALL_MCSCHANNEL_REFS;



/* description du type du contexte ayant produit l erreur  ou le warning
 ce type est donne dans le parametre P2 des procedures extern_signal_error et warning*/
typedef enum  {
     _MCS_TYPE_MCS_CHANNEL_CONFIGURATION,	
     _MCS_TYPE_MCS_SDU_TO_SEND,		/*  P2= MCS_SDU_TO_SEND*/
     _MCS_TYPE_FDM_MSG_RECEIVED,	/*  P2= FDM_MSG_RECEIVED*/
     _MCS_TYPE_FDM_MSG_TO_SEND,		/*  P2= FDM_MSG_TO_SEND*/
     _MCS_TYPE_MCS_FDM_MSG_TO_SEND,	/*  P2= MCS_FDM_MSG_TO_SEND*/ 
     _MCS_TYPE_MCS_CHANNEL_REF   	/*  P2= MCS_CHANNEL_REF*/
}_MCS_TYPE_ERROR_WARNING;
	




/*=========================================================*/
/*========== Configuration des instances MCS ==============*/
/*=========================================================*/

/*  type MCS_CONFIGURATION :
Stucture qui permet au module ACSE (ou equivalent) de fournir
a MCS l'ensemble des parametres necessaires au fonctionnement d'une
instance.
Cette structure est utilisee lors de l'appel par ACSE de la procedure
mcs_initialize().
*/

typedef struct {

	char *			Name;  /* max 8 caract */        
	FDM_REF	*       Fdm_Ref;

	void           (*Extern_Signal_MCS)  ();
	void           (*Extern_Signal_Error)    (_MCS_TYPE_ERROR_WARNING,void * ,_MCS_ERROR_CODE_LIST);
	void           (*Extern_Signal_Warning ) (_MCS_TYPE_ERROR_WARNING,void * ,_MCS_ERROR_CODE_LIST);
 
	                                /* fct de signal conf commande canal*/
	void            (*Extern_Signal_Command_Cnf) (struct _MCS_REF *,
												 struct _MCS_CHANNEL_COMMAND *);
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
   void            (*Extern_Signal_ConnexionFailure) ( USER_MCSCHANNEL_REF ,
													   _CONNEXION_FAILURE_LIST); 
#endif
					/*options for memory management */	
	MEMORY_RN       *Memory_Management_Ref;

} MCS_CONFIGURATION;	






/*=========================================================*/
/*========== Configuration des canaux     =================*/
/*=========================================================*/

/*======= Liste des enumerations pour configuration d'un canal =====*/
/*==================================================================*/



/*Types utilises dans _MCS_USER_CHANNEL_CONFIGURATION */
/* type _MCS_TYPE_DLL
 Ce type permet de definir qui est a l'autre bout du canal :
_MCS_TYPE_DLL_FULLFIP		-->DLLSize =256
_MCS_TYPE_DLL_MICROFIP		-->DLLSize =122
*/
typedef enum  {
	_MCS_MSG_MODE_NORMALISE,
	_MCS_MSG_MODE_NOACK
}_MCS_MSG_MODE_ACK;

typedef enum  {
		_MCS_TYPE_DLL_FULLFIP,		
		_MCS_TYPE_DLL_MICROFIP
}_MCS_TYPE_DLL;	

typedef enum  {
	_MCS_MSG_TYPE_PERIOD,
	_MCS_MSG_TYPE_APERIOD
}_MCS_MSG_TYPE;


/* on peut utiliser des no de segment autre que  0
	Local_Adress	=  lsap local   +num_segment local   
	Remote_Address	=  lsap distant +num_segment distant 
		avec lsap :		bits 8 e 23
			no segment  bits 0 e 7

	NOTE pour  _MCS_TYPE_DLL_MICROFIP :
				le lsap distant correspond a l'adresse physique de la station 
				et doit donc etre < 256
*/
#if (MCS_WITH_CONFIGUSER_SPECIF  ==  _MCS_OPTION_YES)
/*======= structure configuration d'un canal          =====*/
/*=========================================================*/

typedef struct _MCS_USER_CHANNEL_CONFIGURATION{
      USER_MCSCHANNEL_REF		 UserChannelRef;	/* doit etre unique /canal*/
      unsigned long				 Local_Address;		
      unsigned long				 Remote_Address;
	  unsigned short             ConnexionTempoTicks;
      unsigned short             MaxSDUSize;
      _MCS_TYPE_DLL              DLLType;
 	  /* struct Receive_infos */ 
		void  (*User_Msg_Rec_Proc) (USER_MCSCHANNEL_REF ,struct _MCS_SDU_RECEIVED *);
	  /* Send_infos */
		void (*User_Msg_Ack)       (USER_MCSCHANNEL_REF	,struct _MCS_SDU_TO_SEND *);
	  _MCS_MSG_TYPE					msg_type;
	  _MCS_MSG_MODE_ACK				msg_mode_ack;
} MCS_USER_CHANNEL_CONFIGURATION;
#endif

/*=============================================================*/
/*========== Commande pour creation/destruction des canaux   ==*/
/*=============================================================*/

typedef enum{
	_MCS_CHANNEL_CREATE,
	_MCS_CHANNEL_STOP,
	_MCS_CHANNEL_DELETE
}_MCS_CHANNEL_COMMAND_LIST;

#if (MCS_WITH_CONFIGUSER_SPECIF  ==  _MCS_OPTION_YES)
typedef struct _MCS_CHANNEL_COMMAND{		
       struct _MCS_CHANNEL_COMMAND        *Next;		/* private*/
       struct _MCS_CHANNEL_COMMAND        *Previous;	/* private*/
	   struct _MCS_REF					  *mcs_ref;		/* private*/
	  _MCS_CHANNEL_COMMAND_LIST			  Command;
	  MCS_USER_CHANNEL_CONFIGURATION	  *paramIN_for_create;
	  struct _MCSCHANNEL_REF			  *channelmcs;
	  _MCS_ERROR_CODE_LIST			      Report;
	}MCS_CHANNEL_COMMAND;
#endif
/*
	pour commande = _MCS_CHANNEL_CREATE
	  positionner en entree  paramIN_for_create
  	  dans la procedure de confirmation on aura :
			channelmcs pointeur sur reference du canal cree
			si Report =MCS_OK
			si Report non MCS_OK  alors channelmcs=NULL

	pour commande = _MCS_CHANNEL_STOP ou _MCS_CHANNEL_DELETE
	  positionner en entree  channelmcs : no canal a stopper/deleter
	    dans la procedure de confirmation on aura :
		Report qui indique si ok ou non

*/
 

/*#########################################################
      PUBLIC INCLUDE AREA
###########################################################*/
/*================================================================= 
 typedef pour decrire des blocs de donnees, en emission 
 et en reception, pour les services de communication   MCS et User
 =================================================================*/


/* type DESC_BLOCK_DATA_T:
descripteur de bloc pour l'emission*/

typedef struct _DESC_BLOCK_DATA_T {
    struct _DATA_BLOCK_TO_TRANSFER {
 	unsigned short               Nr_Of_Bytes;
	unsigned char *              Ptr_Data;
	struct _DESC_BLOCK_DATA_T   *Next_Block;
    } User_Block;

    struct _DATA_BLOCK_TO_TRANSFER   Private;

} DESC_BLOCK_DATA_T;


/* type DESC_BLOCK_DATA_R:
descripteur de bloc pour la reception*/

typedef struct _DESC_BLOCK_DATA_R {
 	unsigned short               Nr_Of_Bytes;
	unsigned char *              Ptr_Data;
	struct _DESC_BLOCK_DATA_R   *Next_Block;
} DESC_BLOCK_DATA_R;


typedef struct _DATA_BLOCK_TO_TRANSFER DATA_BLOCK_TO_TRANSFER;



/*#########################################################
      PRIVATE INCLUDE AREA
###########################################################*/

#include "mcsprive.h"





/*#########################################################
      PUBLIC INCLUDE AREA
###########################################################*/

/*========== Reception des messages MCS =======*/
/*=============================================*/
/*type MCS_SDU_RECEIVED :
Si on n'est pas avec l'option Concatenation :
Type du descripteur de message,fourni a l'utilisateur, 
au moment de l'indication de service de reception:
*/
typedef struct _MCS_SDU_RECEIVED  {

       struct _MCS_SDU_RECEIVED        *Next;	
       struct _MCS_SDU_RECEIVED        *Previous;	

       ALL_MCSCHANNEL_REFS				Channel;
       unsigned long                    Source;
       unsigned short                   SDU_Nr;
	   unsigned short                   SDU_Size;
       unsigned int                     Nr_Of_Blocks;
       DESC_BLOCK_DATA_R               *Ptr_Block;
 } MCS_SDU_RECEIVED;





/*========== Emission et Reception des messages MCS =======*/
/*=========================================================*/


/* type MCS_SDU_TO_SEND :
Type du descripteur de message SDU a transmettre a travers un
canal MCS.
Ce type est utilise dans la procedure mcs_send_message(),
et dans la procedure de confirmation de transfert associee
au canal.
Note 1: Ce type (et certains types dont il depend) contient
des champs prives, dont l'utilisation et le contenu ne
concerne pas l'utilisateur
Note 2: le champ 'Report' ne sera utilise que comme parametre de 
SORTIE, que l'on peut consulter au moment de la confirmation 
de transfert
*/
#define _MCS_NOT_SIGNICANT_REMOTE_ADDRESS	  _REMOTE_ADDRESS_NOT_SIGNIFICANT

typedef struct _MCS_SDU_TO_SEND  {

       struct _MCS_SDU_TO_SEND        *Next;			/*private*/
       struct _MCS_SDU_TO_SEND        *Previous;		/*private*/ 
       ALL_MCSCHANNEL_REFS	           Channel;			/*private*/
       unsigned long                   Destination;		/*user*/
       unsigned int                    Nr_Of_Blocks;	/*user*/
       DESC_BLOCK_DATA_T              *Ptr_Block;		/*user*/
       MCS_SDU_TO_SEND_PRIVATE_AREA    Private_MCS;		/*private*/
       _MCS_ERROR_CODE_LIST            Service_Report;	/*cr pour user sur ack*/
 
 } MCS_SDU_TO_SEND;


 

/*#########################################################
				PRIVATE INCLUDE AREA
###########################################################*/
/*======= Liste des enumerations pour 
		  configuration generale mcs d'un canal =====*/
typedef enum   {
    _CHANNEL_FOR_CONNECTED_SERVICES ,
    _CHANNEL_FOR_UNCONNECTED_SERVICES 
}_MCS_CHANNEL_SERVICE_LIST;


typedef enum  {
    _CHANNEL_FOR_TRANSM_AND_RECEP ,
    _CHANNEL_FOR_TRANSM ,
    _CHANNEL_FOR_RECEP 
}_MCS_CHANNEL_COMMUNICATION_LIST;

typedef enum  {
    _CHANNEL_FOR_MULTIFRAME_SDU ,
    _CHANNEL_FOR_MONOFRAME_SDU 
}_MCS_CHANNEL_SDU_LIST;


typedef enum  {
	_MCS_MSG_IMAGE_1	= 1,	
	_MCS_MSG_IMAGE_2 	,	
	_MCS_MSG_IMAGE_1_AND_2		
}_MCS_MSG_IMAGE_LIST;

typedef enum  {
	_MCS_WITH_STUFFING_MODE , 
	_MCS_WITHOUT_STUFFING_MODE ,
	_MCS_STUFFING_CANCELLED
}_MCS_STUFFING_MODE_LIST;

/*========== Configuration generale des canaux MCS =================*/
typedef struct _MCS_CHANNEL_CONFIGURATION{

       USER_MCSCHANNEL_REF				  UserChannelRef;
       char                               *Name ;      /* max 8 caract */                      

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
      _MCS_CHANNEL_SERVICE_LIST            ServType;
      _MCS_CHANNEL_SDU_LIST                SDUType; 
#endif
      _MCS_CHANNEL_COMMUNICATION_LIST      CommType; 

      unsigned long	                       Local_DLL_Address;		
      unsigned long	                       Remote_DLL_Address;		
      unsigned short                       MaxSDUSize;
      unsigned short                       MaxParallelServices;
      unsigned short                       DLLSize;

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
      _MCS_STUFFING_MODE_LIST              StuffingMode;
      unsigned short                       ConnexionTempoTicks;
#endif

	  /* struct Send_infos */
      struct{
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
        unsigned short    Anticipation;
        unsigned short    NbOfRetry;
#endif
		void (*User_Msg_Ack) (USER_MCSCHANNEL_REF  ,
							  struct  _MCS_SDU_TO_SEND *);
      }Send_infos;

 	  /* struct Receive_infos */ 
     struct{
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
        unsigned short       Anticipation;
        unsigned short       NbOfRetry;
#endif
		void  (*User_Msg_Rec_Proc) ( USER_MCSCHANNEL_REF  , 
								    struct  _MCS_SDU_RECEIVED *);
      }Receive_infos;

       _MCS_MSG_IMAGE_LIST   Position;
       unsigned short        Channel_Nr ;	
	   _MCS_MSG_MODE_ACK 	 msg_mode_ack;/* modif 09.05.2000*/
} MCS_CHANNEL_CONFIGURATION;


#if (MCS_WITH_CONFIGUSER_SPECIF  ==  _MCS_OPTION_NO)
/*===================================================
Commande pour creation/destruction des canaux 
=====================================================*/
typedef struct _MCS_CHANNEL_COMMAND{		
       struct _MCS_CHANNEL_COMMAND        *Next;		/* private*/
       struct _MCS_CHANNEL_COMMAND        *Previous;	/* private*/
	   struct _MCS_REF					  *mcs_ref;		/* private*/
	  _MCS_CHANNEL_COMMAND_LIST			  Command;
	  MCS_CHANNEL_CONFIGURATION			  *paramIN_for_create;
	  struct _MCSCHANNEL_REF			  *channelmcs;
	  _MCS_ERROR_CODE_LIST			      Report;
	}MCS_CHANNEL_COMMAND;
#endif
 
/*#########################################################
      PRIVATE INCLUDE AREA
###########################################################*/

#include "mcspriv2.h"
 
#if (MCS_WITH_CONFIGUSER_SPECIF  ==  _MCS_OPTION_YES)
/* procedure decrite dans mcs_user.c*/
MCSCHANNEL_REF*  McsChannelCreateUserSpecif (MCS_REF * ,
								   MCS_USER_CHANNEL_CONFIGURATION *,
								   _MCS_ERROR_CODE_LIST *);

#endif



/*#########################################################
      PUBLIC INCLUDE AREA
###########################################################*/

#ifdef __cplusplus
extern "C" {
#endif

/*========== Fonctions de l'interf. user ==================*/
void	mcs_get_software_version ( MCS_VERSION * );

/* creer une instance de logiciel mcs*/
/* retourne MCS_REF *  si ok
		  ( MCS_REF*)0 si non ok avec le code error  dans Report*/
 MCS_REF *mcs_initialize (
         MCS_CONFIGURATION *    parameters,
         _MCS_ERROR_CODE_LIST * Report);

/*faire tourner une instance de MCS */
 void mcs_fifos_empty( MCS_REF * Mcs ); 
      

_MCS_ERROR_CODE_LIST mcs_channel_command(MCS_REF *Mcs,
										  MCS_CHANNEL_COMMAND *mcschannelcmd);


/* fonction qui permet de faire une demande de transfert de donnees
a travers un canal de type A_DATA ou A_UNIDATA*/
_MCS_ERROR_CODE_LIST mcs_send_message(
					 MCSCHANNEL_REF *Channel,
					 MCS_SDU_TO_SEND* Message);

/* fonction qui permet de recycler les ressources de type indications
que MCS avait fourni a l'utilisateur au moment de l'indication de reception*/

void free_mcs_received_message(MCS_SDU_RECEIVED *);


/* fonction qui permet de recuperer mettre a jour une
chaine de caracteres, avec le symbole correspondant au code d'erreur
A utiliser avec une chaine d'au moins 40 caract
 */

extern void mcs_get_errorandwarningstring (	char *				String,
									  	   _MCS_ERROR_CODE_LIST Code);
/* Procedure de concatenation du message recu
lors de la reception du message, l'utilisateur connait la taille 
du message : MCS_SDU_RECEIVED --> SDU_Size

en entree de cette  procedure , il fournit donc :
parametre 1: un pointeur  ptr_data_user, sur une zone qu'il a alloue 
			et qui est au minimum aussi grande que SDU_Size 
parametre 2:pointeur sur message MCS_SDU_RECEIVED
*/

void  mcs_cat_msg_received(char *ptr_data_user,MCS_SDU_RECEIVED *msg);
#if (_STACK_WITH_WindowsNT == _MCS_OPTION_YES)
void mcs_timer_up_to_date(void);
#endif


/* fonction qui permet de recycler les ressources d'une instance MCS
 fonction appele sur sortie , il faut d abord avoir libere tous les canaux */

void mcs_free (MCS_REF *);

#ifdef __cplusplus
}
#endif


/*******************************************************************/
#endif
/*************** End Of  'mcs.h' File ******************************/






