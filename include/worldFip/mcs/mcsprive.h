/************************************************************
   
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcsprive.h
  Description     : Interfaces internes (procedures)
                   
 
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000 


************************************************************************/


#ifndef __mcsprive_h
#define __mcsprive_h

#include "fdm.h"
#include "mcs_opt.h"
#include "mcs_os.h"



struct _MCSCHANNEL_REF;
/*!!!!!!!!!!!!!!!!!!!!!! !!!!!!!!!!!!!!!!!!

Dans ce fichiers INCLUDE, on distingue des zones PRIVEES 

contenant des description types, enums, prototypes de fonctions
ne concernant par directement les utilisateurs de ce logiciel, 

Note: il se peut que des types soient manipules par les modules 
utilisateurs, alors que leur contenu ne concerne que le module MCS.

*/










/*#########################################################
      PRIVATE INCLUDE AREA
###########################################################*/

/*========== Enumerations et definitions internes =========*/

#define _MCS_YES 0
#define _MCS_NO  1

typedef enum  {
    _MCS_CONNECTED_DATA_TRANSFER_FOR_PDU ,
    _MCS_CONNECTED_DATA_TRANSFER_FOR_ACK 
}_MCS_CONNECTED_SERVICE_LIST;				



/*enum _MCS_INDICATION_TYPE_LIST :
Liste des indications possibles donnes par MCS, soit a l'utilisateur
*/
typedef enum {
   _MCS_INDICATION_Other ,
   _MCS_INDICATION_For_Data 
}_MCS_INDICATION_TYPE_LIST;			




typedef enum {
     _INDIV_ADDR ,
     _GROUP_ADDR 
} _FIP_ADDRESS_TYPE_LIST;

typedef enum {
     _NOTHING_IS_DONE ,
     _WORK_IS_DONE 
} _PROCESS_ACTIVITY_LIST;

/* enum _MCS_CHANNEL_STATE_LIST : 
Note: cette eunum est utilisee en interne uniquement 
Represente les etats des canaux MCS, a chaque instant
*/
typedef enum  {
     _MCS_CHANNEL_STATE_INEXISTING,
     _MCS_CHANNEL_STATE_IDLE_TO_START,
     _MCS_CHANNEL_STATE_RUNNING,
	 _MCS_CHANNEL_STATE_IDLE_TO_DELETE,
     _MCS_CHANNEL_STATE_DELETE_IN_PROGRESS

}_MCS_CHANNEL_STATE_LIST;

/* enum _MCS_CHANNEL_ACKNOWLEDGE_LIST :
Cette enum est utilisee en interne uniquement 
*/
typedef enum  {
    _CHANNEL_WITH_PDU_ACK ,
    _CHANNEL_WITHOUT_PDU_ACK 
}_MCS_CHANNEL_ACKNOWLEDGE_LIST;

/*enum  _MCS_INTEG_FILES_LIST :
Liste des files d'integration utilisees en interne
par MCS. Permet de les indicer, au momemt de leur remplissage, 
ainsi qu'au moment de leur Purge.
*/
typedef enum  { 
                _CommandList,
				_ReceiveDLLList   ,

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
                _TimeOutList      ,
                _FreeBufferList   ,        
                _SendMCSList      ,
                _SendDLLCnfList   ,
#endif         
                _MCS_INTEG_FILE_NUMBER
}_MCS_INTEG_FILES_LIST;



/* enum _A_DATA_PDU_STATE_LIST :
Liste des etats d'un PDU a transmettre, dans le
cas d'une transmission A_DATA (fenetre d'emission)
ou dans le cas d'une reception A_DATA 
(fenetre de reception)
*/
typedef enum  {
   _Free_Box                ,
   _Available_Pdu           ,

   _Wait_For_ConfAndAck     ,
   _Wait_For_AckAndTimeOut  ,
   _Wait_For_Cnf            ,
   _Wait_For_TimeOut        ,
   _Wait_End_OK             ,
   _Wait_End_NOK
} _A_DATA_PDU_STATE_LIST ;

/*enum _A_DATA_PDU_PURGE_STATE_LIST :
Liste des etats "purge" d'un PDU ou d'un SDU
*/
typedef enum {
    _Transmitting ,
    _Cancelled
} _A_DATA_PDU_PURGE_STATE_LIST ;


/* enum _RECEPTION_A_DATA_PDU_USER_STATE_LIST :
Liste des etats, par rapport a la reception par 
l'utilisateur, d'un PDU recu dans le cas d'une
reception A_DATA
*/
typedef enum  {
   _NotYetTakenIntoAccount ,
   _AlreadyTakenIntoAccount 
} _RECEPTION_A_DATA_PDU_USER_STATE_LIST;


typedef enum  {
    _EmptyMachine ,
    _BeginningMachine , 
    _RunningMachine
} _MACHINE_STATE_LIST;


/* liste des entetes possibles, dans les PDUs 
de type A_DATA
*/
#define __HEAD_NIL          (unsigned char)0x20
#define __HEAD_ACK_DATA     (unsigned char)0x60
#define __HEAD_BME	    (unsigned char)0x27
#define __HEAD_BEG	    (unsigned char)0x24
#define __HEAD_MID	    (unsigned char)0x25
#define __HEAD_END	    (unsigned char)0x26

#define __HEAD_A_RELEASE_RQ    (unsigned char)0x90
#define __HEAD_A_RELEASE_RP    (unsigned char)0xD0
#define __HEAD_A_ABORT	       (unsigned char)0xB0



/*liste des types de temporisations gerees par le logiciel MCS */
typedef enum {
   _MCS_TEMPO_FOR_ANYTHING = 0 ,
   _MCS_TEMPO_FOR_CONNEXION_PDU_ACK_TIMEOUT ,
   _MCS_TEMPO_FOR_STUFFING ,
   _MCS_TEMPO_FOR_TRANSMISSION_TIME_OUT ,
   _MCS_TEMPO_FOR_RECEPTION_TIME_OUT 
} _MCS_TEMPO_TYPE_LIST;



#define _STUFFINGTOCONNEXIONTIMEFACTOR 3
/* rapport entre l'unite de temps choisie pour l'emission
du bourrage, et l'unite de temps pour les time outs sur la non
reception de l'acquittement d'un PDU (canal type connecte)
   StuffingTicks = _STUFFINGTOCONNEXIONTIMEFACTOR * ConnexionTimeOut
*/
#define _STUFFINGTOCONNEXIONTIMEFACTOR 3


/* rapport entre l'unite de temps choisie pour l'emission
du bourrage, et l'unite de temps pour les time outs -watchdog 
sur l'emission et la reception des PDUs (canal type connecte) 
   TimeOutTicks = _WDOGTOSTUFFINGTIMEFACTOR * StuffingTicks
*/
#define _WDOGTOSTUFFINGTIMEFACTOR 3

/*========== Pour la reception A_DATA  ====================*/

/* enum _POSITION_IN_RECEP_WINDOW_LIST :
Liste des positions d'un PDU, defini par son numero de 
sequence, dans la fenetre de reception */

typedef enum {
    _Inside_Window_Position ,
    _Upper_Window_Position ,
    _Under_Window_Position ,
    _FarFrom_Window_Position 
} _POSITION_IN_RECEP_WINDOW_LIST;

/* enum _RECEP_WINDOW_SCROLL_TYPE_LIST:
Liste des types de glissement de fenetre de reception
que l'on peut effectuer.
*/
typedef enum {
    _Small_Scroll ,
    _Large_Scroll 
} _RECEP_WINDOW_SCROLL_TYPE_LIST;





/*#########################################################
      PRIVATE INCLUDE AREA
###########################################################*/


/* type PRIVE MCS_FDM_MSG_TO_SEND :
Ce type est utilise dans le but de faire une extension
a la ressource memoire qui est utilisee pour une demande 
de transfert de message a FIP DEVICE MANAGER. Cette
extension permet de posionner des informations dans 
la ressource passe a FDM, informations que l'on retrouve 
au moment de la confirmation de transfert par FIP DEVICE 
MANAGER puisque c'est le memee pointeur que l'on retrouve.
Ces informations permettent de retrouver le contexte de 
fonctionnement correspondant a la demande, et ce au moment de 
de la confirmation.
Les informations sont
	-la reference du canal (->Channel)
	-le type de message (ACSE ou DATA) dans le cas d'un transfert
	a travers un canal connecte (A_DATA) (->Type)
*/
typedef struct {

     FDM_MSG_TO_SEND                  FdmToSend;
     struct _MCSCHANNEL_REF          *Channel;
     _MCS_CONNECTED_SERVICE_LIST      Type;
     void *                           Ctxt;
     unsigned char                    Data[4];

} MCS_FDM_MSG_TO_SEND;


/* type PRIVE MCS_ACK_MSG_TO_SEND :
Ce type est utilise pour projeter sur la ressource qui nous servira
pour emettre un acquittement de PDU, dans le cas d'un canal connecte
de reception.
*/
typedef struct {
     MCS_FDM_MSG_TO_SEND               McsFdmToSend;
     FDM_MSG_T_DESC                    Desc;
} MCS_ACK_MSG_TO_SEND;



/* type PRIVE MCS_SDU_TO_SEND_PRIVATE_AREA :
Ce type est utilise pour la zone privee a MCS, contenue
dans le type manipule par MCS MCS_SDU_TO_SEND.
Cette zone sert au logiciel MCS, pour son fonctionnement
interne, et evite d'effectuer des allocations memoire 
pour stocker le contexte de fonctionnement ou manipuler
des nouvelles donnees
*/
 
typedef struct _MCS_SDU_TO_SEND_PRIVATE_AREA{
   MCS_FDM_MSG_TO_SEND               McsFdmToSend;
   unsigned char                     Stuffing;
} MCS_SDU_TO_SEND_PRIVATE_AREA;
/* Modif ACSE-09-12-99 suppression type PRIVE 
ACSE_SDU_TO_SEND_PRIVATE_AREA */




/*#########################################################
      PRIVATE INCLUDE AREA
###########################################################*/

#define _rangdom_number 0x4394

#define _name_max_length 8


struct _TRANSMISSION_WINDOW_TYPE;
struct _RECEPTION_WINDOW_TYPE;

/*==== Contexte de fonctionnement d'une iinstance MCS =====*/

/* type MCS_REF :
type prive representant une instance de MCS, pour MCS
Des pointeurs sur ce type sont utilise pour le dialogue 
    ACSE-> MCS
    USER-> MCS 
lors de tout dialogue concernant un service sur l'objet Instance MCS
Le pointeur de reference de l'instance est fourni par MCS a l'exterieur, 
au moment de la creation de l'instance
*/        
typedef struct _MCS_REF  {

       struct _MCS_REF        *Next; 
       struct _MCS_REF        *Previous; 
       FDM_REF   *             Fdm_Ref;
       void            (*Extern_Signal_MCS)  ();
       void            (*Extern_Signal_Error)(_MCS_TYPE_ERROR_WARNING,
												void * ,  _MCS_ERROR_CODE_LIST);
       void            (*Extern_Signal_Warning ) (_MCS_TYPE_ERROR_WARNING,
												  void * , _MCS_ERROR_CODE_LIST);

		void			(*Extern_Signal_Command_Cnf)(struct _MCS_REF *,
												 struct _MCS_CHANNEL_COMMAND *);
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
		void            (*Extern_Signal_ConnexionFailure) ( USER_MCSCHANNEL_REF ,
													   _CONNEXION_FAILURE_LIST); 
#endif
	   /*fct de signal conf commande */
       MEMORY_RN       *Memory_Management_Ref;
                                /*options for memory management */
       char					  Name[_name_max_length+1];
                                   /* max 8 lettres */
       FIFO_INT_TYPE           ListOfChannels;
       int                     CheckNumber;  /* pour verfier etat */
      
       TASK_REFERENCE_TYPE     Task; 
       SOFT_REFERENCE_TYPE     Soft; 
       INTEG_FILE_TYPE         List[_MCS_INTEG_FILE_NUMBER];
       SEMAPHORE_TYPE          Semaphore;

} MCS_REF;					





/*========== Segmentation et Reassemblage =================*/


struct _MCS_SDU_TO_SEND;
struct _MCS_SDU_RECEIVED;

/* SEGMENT_CNTXT_TYPE :
Type qui contient le contexte de la machine de
segmentation, pour le fonctionnement d'un 
canal A_DATA multitrame, en emission
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
typedef struct _SEGMENT_CNTXT_TYPE {

   unsigned int 			Nr_Of_RemainingBlocks;
   unsigned int 			Nr_Of_RemainingBytes;
   unsigned char              * Ptr_Data;
   DESC_BLOCK_DATA_T  * Ptr_CurrentDesc;
   _MACHINE_STATE_LIST	        State;
   struct _MCS_SDU_TO_SEND   * RequestRef;

} SEGMENT_CNTXT_TYPE ;
#endif


/* ASSEMBL_CNTXT_TYPE :
Type qui contient le contexte de la machine de
reassemblage pour le fonctionnement d'un 
canal A_DATA multitrame, en reception.
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
typedef struct ASSEMBL_CNTXT_TYPE {

   unsigned int 			Nr_Of_Blocks ;
   DESC_BLOCK_DATA_R *	    LineStart    ;
   DESC_BLOCK_DATA_R *		LineEnd      ;
   _MACHINE_STATE_LIST	    State        ;

} ASSEMBL_CNTXT_TYPE;
#endif



/*========== Services de temporisation ====================*/

/* type MCS_TEMPO_TYPE_MCS_CONNECTED_SERVICE_LIST :
Type prive utilise a chaque demande de service de temporisation.
ce type est une extension du type TEMPO_DEF, dont les champs 
supplementaires MySignalFunction et Channel servent a aiguiller 
le descripteur de temporisation, au moment de l'echeance (
ie lorsque l'evenement echeance est traite par le sequenceur)
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
typedef struct _MCS_TEMPO_TYPE {
	     TEMPO_DEF                   Desc;
         struct _MCSCHANNEL_REF *    Channel;
         _MCS_TEMPO_TYPE_LIST        Type;
      }   MCS_TEMPO_TYPE;
#endif

                                        




#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
struct _TRANSMISSION_WINDOW_ELEMENT_TYPE;
/* type MCS_A_DATA_T_TEMPO_TYPE :
Type utilise pour lancer des temporisations concernant
le transfert des PDUs dans un canal de type connecte.
Ce type, qui constitue une extension du type MCS_TEMPO_TYPE,
permet de retrouver la reference du PDU concerne, au moment du
traitement de l'echeance de temporisation.
*/
typedef struct _MCS_A_DATA_T_TEMPO_TYPE {

    MCS_TEMPO_TYPE	                       Desc;
    struct _TRANSMISSION_WINDOW_ELEMENT_TYPE * BoxRef;

} MCS_A_DATA_T_TEMPO_TYPE;
#endif


/*========== Fenetres de segmentation/reassemblage ========*/

/* type TRANSMISSION_WINDOW_ELEMENT_TYPE :
Ce type decrit un element de la fenetre d'anticipation en emission, 
pour un canal de type connecte.
Il contient les elements necessaires au traitement complet d'un PDU
a transmettre.
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
typedef struct _TRANSMISSION_WINDOW_ELEMENT_TYPE {

      unsigned short                         SequenceNumber;
      _A_DATA_PDU_STATE_LIST                 State;
      _A_DATA_PDU_PURGE_STATE_LIST           PurgeState;
      unsigned char			     Type;
      unsigned int                           Stuffing;
      unsigned int                           SendCredit;
      struct _MCS_SDU_TO_SEND *              RequestRef;
      MCS_FDM_MSG_TO_SEND                    AreaToSend;
      DESC_BLOCK_DATA_T                      BlockToSend;
      struct _MCS_A_DATA_T_TEMPO_TYPE        TempoDesc;
    
} TRANSMISSION_WINDOW_ELEMENT_TYPE;
#endif


/*type RECEPTION_WINDOW_ELEMENT_TYPE :
Ce type decrit un element de la fenetre d'anticipation en reception,
pour un canal de type connecte.
Il contient les elements necessaires au traitement complet d'un PDU
a recu.
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
typedef struct _RECEPTION_WINDOW_ELEMENT_TYPE {

      unsigned short                         SequenceNumber ;
      _A_DATA_PDU_STATE_LIST                 State          ;
      _RECEPTION_A_DATA_PDU_USER_STATE_LIST  UserState      ; 
      FDM_MSG_RECEIVED                     * DLLMessage     ;

} RECEPTION_WINDOW_ELEMENT_TYPE;
#endif


/*type TRANSMISSION_WINDOW_TYPE :
Type qui decrit la fenetre d'anticipation en emission,
pour un canal de type connecte.
Le nombre d'elements de cette fenetre depend du taux 
d'anticipation. On prendra n = 2 x l'anticipation en emission.
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
typedef struct _TRANSMISSION_WINDOW_TYPE {

   TRANSMISSION_WINDOW_ELEMENT_TYPE   Box[1];

} TRANSMISSION_WINDOW_TYPE;
#endif

/*type RECEPTION_WINDOW_TYPE :
Type qui decrit la fenetre d'anticipation en reception,
pour un canal de type connecte.
Le nombre d'elements de cette fenetre depend du taux 
d'anticipation. On prendra n = anticipation en emission.
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
typedef struct _RECEPTION_WINDOW_TYPE {

   RECEPTION_WINDOW_ELEMENT_TYPE   Box[1];

} RECEPTION_WINDOW_TYPE;
#endif



_FIP_ADDRESS_TYPE_LIST FipAddressType ( unsigned long Address );







	
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )

/* enum _MCS_CHANNEL_STATISTICS_LIST :
Liste des compteurs de statistiques mis a lour par MCS.
Cette enum permet les compteurs, au moment de leur remplissage,
et au moment de leur exploitation par l'utilisateur 
*/

typedef enum  {

     _SendDATARequestOK ,
     _SendDATARequestNOK ,
     _SendDATARequestOnRUNNINGChannel,
     _SendDATARequestOnIDLEChannel ,
     _RejectDataSDUTransmissionRequest ,
     _ConfirmDataSDUTransmission ,

     _ReceiveFromDLLOnRUNNINGChannel,
     _ReceiveFromDLLOnIDLEChannel,

     _FreeDLLRefBuffer , 
     _FreeDLLDataBuffer ,

     _SendToDLLNoError ,
     _SendToDLLInexistingChannel ,
     _SendToDLLBadImage ,
     _SendToDLLInternalError ,
     _SendToDLLNoAckReceivedDefault ,
     _SendToDLLBadRPMsgDefault ,
     _SendToDLLNegAckReceivedDefault ,
     _SendToDLLTimeOutDefault ,
     _SendToDLLUnknownError ,

    _CnfDLLReceived ,

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
    _DataPDUTranmission ,
    _CnfDLLReceivedForAckPDU ,
    _CnfDLLReceivedForDataPDU ,
    _CnfDLLReceivedForDataPDUAfterAckReceived ,
    _TimeOutReceivedForDataPDU  ,
    _DataPDUNewTransfer  ,
    _DataPDUTransferFailed  ,
    _DataPDUAckReceived  ,
    _DataPDUAckReceivedBeforeCnfDLLReceived ,
    _MultipleDataPDUAckReceived  ,
    _DataPDUAckReceivedForUnknownPDU ,
#endif
    _InvalidPDUReceived ,
    _ValidPDUReceived ,

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
    _DataPDUReceivedWithProtocolErrors ,
#endif
  
    _LackOfMemoryToIndicate_DataSDU ,
    _DataSDUIndicated   , 
    _FreeDataSDUReceived  ,   

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
    _OldDataPDUReceived ,
    _OutOfWindowPDUReceived ,
    _MultipleDataPDUReceived ,
    _DataPDUAckProduced ,
    _LackOfMemoryToProduceDataPDUAck ,
    _SequenceBreakInDataPDUList ,

    _StuffingProduced , 
    _StuffingReceived ,
    _Underrunstuffing , 
#endif
     _MCS_CHANNEL_STATISTICS_NUMBER
}_MCS_CHANNEL_STATISTICS_LIST;		

#endif




#endif
/*************** End Of  'mcsprive.h' File ******************************/
