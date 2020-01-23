/************************************************************
   
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcsprive2.h
  Description     : Interfaces internes (procedures)
                    
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000 
************************************************************************/

#ifndef __mcsprive2_h
#define __mcsprive2_h

#include "fdm.h"
#include "mcs_opt.h"
#include "mcs_os.h"



#if (_STACK_WITH_WindowsNT==   _MCS_OPTION_YES)
#define  mcs_timer_up_to_date  _TIMER_UP_TO_DATE
#endif

/*========== Contexte de fonctionnement d'un canal ========*/



/* type MCSCHANNEL_REF :          ici
 type prive representant un canal pour MCS

lors de tout dialogue concernant un service sur l'objet Canal MCS
Le pointeur de reference du canal est fourni par MCS a l'exterieur, 
au moment de la creation du canal
*/
typedef struct _MCSCHANNEL_REF  {

      struct _MCSCHANNEL_REF              *Next; 
      struct _MCSCHANNEL_REF              *Previous; 
      _MCS_CHANNEL_SERVICE_LIST            ServType;
      _MCS_CHANNEL_COMMUNICATION_LIST      CommType; 
      _MCS_CHANNEL_ACKNOWLEDGE_LIST        AckType; 
      _MCS_CHANNEL_SDU_LIST                SDUType; 
      unsigned long						   Local_DLL_Address;		
      unsigned long						   Remote_DLL_Address;		
      unsigned short                       MaxSDUSize;
      unsigned short                       DLLSize;
      unsigned short                       MaxParallelServices;

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
      unsigned short                       T_Anticipation;
      unsigned short                       T_NbOfRetry;
      unsigned short                       ConnexionTempoTicks;
      unsigned short                       TransmWatchDogTempoTicks;
      unsigned short                       RecepWatchDogTempoTicks;
      unsigned short                       R_Anticipation;
      unsigned short                       R_NbOfRetry;

#endif
      void                (*User_Msg_Ack) (  USER_MCSCHANNEL_REF  , 
											  struct  _MCS_SDU_TO_SEND *);
      void                (*User_Msg_Rec_Proc) (
										USER_MCSCHANNEL_REF , 
										struct  _MCS_SDU_RECEIVED *);
      _MCS_MSG_IMAGE_LIST  	           Position;
      unsigned short     	           FIP_Channel_Nr;
      char                             Name[_name_max_length+1];
                                              /* max 8 lettres */

      USER_MCSCHANNEL_REF                 UserChannelRef;

      struct _MCS_REF                     *Mcs;
      FDM_MESSAGING_REF                   *Lsap_Ctxt;
      int                                  CheckNumber;
      _MCS_CHANNEL_STATE_LIST              State;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
      unsigned long                        Statistics[_MCS_CHANNEL_STATISTICS_NUMBER];
#endif
      unsigned short                       CurrentInvocationNumber ;

      T_File                             * PoolForUserIndicationRessources ;

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

      /* pour transmission A_DATA */
      FIFO_INT_TYPE                        RqList                       ;
      struct _TRANSMISSION_WINDOW_TYPE   * TransmitWindowArea	        ; 
      unsigned short                       TransmitWindowBeginningIndix ;     
      unsigned short                       TransmitWindowEmptyingIndix  ;     
      unsigned short                       TransmitWindowFullingIndix   ;     
      SEGMENT_CNTXT_TYPE                   Segmentation                 ;
      _A_DATA_PDU_PURGE_STATE_LIST         SDUPurgeState                ;

      /* pour reception A_DATA */
      struct _RECEPTION_WINDOW_TYPE      * RecepWindowArea	        ; 
      unsigned short                       RecepWindowFullingIndix      ;  
      ASSEMBL_CNTXT_TYPE                   Assembling                   ;
      unsigned char                        BrokenSequence               ;
      T_File                             * PoolForAckRessources         ;
      FIFO_INT_TYPE                        AckRessourcesList            ;
      unsigned int                         SDU_Number                   ;

      /* pour bourrage, en emission*/
      _MCS_STUFFING_MODE_LIST              StuffingMode                 ;
      unsigned short                       StuffingTempoTicks           ;
      unsigned int                         IdleTransmPDUInProgress      ;
      struct _MCS_SDU_TO_SEND              StuffingSDUToSend			;
      MCS_TEMPO_TYPE                       DescTempo_For_Stuffing		;
     

#endif

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
      /*qq ressources pour les temporisations */

      MCS_TEMPO_TYPE                       DescTempo_For_TransmWatchdog ;
      MCS_TEMPO_TYPE                       DescTempo_For_RecepWatchdog  ;

#endif
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
      MCS_CHANNEL_COMMAND                * DeleteRequestRef;
	  unsigned long						 NumberOfInRequests;
	  unsigned long						 NumberOfOutRequests;
#endif


} MCSCHANNEL_REF;




/*#########################################################
      PRIVATE INCLUDE AREA - MACROS
###########################################################*/

#define _PDUMakeAnticipation(Channel)   \
    (((Channel)->T_Anticipation)*2 )                         

#define _PreviousInTransmWindow(indix, antic)         \
       ((indix)-1)&((antic)-1)


                                        
/*glissement fenetre de remplissage mod Q :*/                     
#define _PDUMakeProcessScroll(Channel)                              \
   (Channel)->TransmitWindowFullingIndix = (unsigned short)(        \
     ((Channel)->TransmitWindowFullingIndix+1)&                     \
	   (_PDUMakeAnticipation(Channel)-1));                          
   



/*glissement fenetre de transmission mod Q :*/                     
#define _TransmWindowScroll(Box, Channel)                        \
      (Box)->State=_Free_Box;                                    \
      (Channel)->TransmitWindowBeginningIndix = (unsigned short) \
       ( ( (Channel)->TransmitWindowBeginningIndix +1 ) & \
           (_PDUMakeAnticipation(Channel)-1));
 

/* distance entre nombres de type unsignes short */
#define DISTANCE_REFERENCE	0x00010000

#define DISTANCE_BETWEEN(sup, inf)			     \
      ((unsigned short )((DISTANCE_REFERENCE-(inf)+(sup))&0xFFFF))




/* Prise de semaphore/liberation */
#if _SEMAPHORE_DEBUG
extern	void MCS_BEGIN_CRITICAL(MCS_REF*); 
extern	void MCS_END_CRITICAL(MCS_REF*); 
#else
#if (_STACK_WITH_WindowsNT	==	_MCS_OPTION_YES)
#define MCS_BEGIN_CRITICAL(Mcs)   SemaphoreTake((Mcs)->Semaphore)
#define MCS_END_CRITICAL(Mcs)     SemaphoreRelease((Mcs)->Semaphore)
#endif
#if (_STACK_WITH_WindowsNT	==	_MCS_OPTION_NO)
#define MCS_BEGIN_CRITICAL(Mcs)   OS_semaphore_take((Mcs)->Semaphore)
#define MCS_END_CRITICAL(Mcs)     OS_semaphore_release((Mcs)->Semaphore)
#endif
#endif





/*#########################################################
      PRIVATE INCLUDE AREA
###########################################################*/


#define _REMOTE_ADDRESS_NOT_SIGNIFICANT	      0x8FFFFFFFL

#define _MCS_UNIVERS_ADDRESS	              0x80000000L
    /* _REMOTE_ADDRESS_NOT_SIGNIFICANTdefini dans mstack.h */
#define _MAX_FIP_SIZE                      256
#define _MAX_UNCONNECTED_HEADER            4
#define _MAX_CONNECTED_HEADER              4
#define _MAX_CONNECTED_SDU_SIZE            0xffff 
#define _MAX_CONNECTED_PARALLEL_SERVICE    1000        
#define _MAX_UNCONNECTED_PARALLEL_SERVICE  1000     


/*L'adresse ci-dessous est utilisee pour toutes les adresses
DISTANTES non significativers, de type SOURCE ou DESTINATION.
Utilisee dans le mode connecte:
   Destination, dans le cas du send
   Source, dans le cas du receive
 */




/*#########################################################
      PRIVATE INCLUDE AREA
###########################################################*/

/*========== Fonctions appelees par FDM  ==================*/

/* fonction appelee par FDM, au moment de la confirmation de transfert */
extern void MCSDllConfirmationFunction
    (FDM_MESSAGING_REF*, FDM_MSG_TO_SEND * );

/* fonction appelee par FDM, au moment de la reception d'indication de message */
extern void MCSDllIndicationFunction 
    (FDM_MESSAGING_REF*,FDM_MSG_RECEIVED * );

/*========== Fonctions d'execution d'un evenement elementaire
 pour un canal donne ======================================*/

/*fonction pour le traitement de la reception d'une trame
d'acquittement de PDU de donnees, dans le cas d'un canal de 
type A_DATA Emssion
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecIndDLL_A_DATA_AckPDU(
      MCSCHANNEL_REF     * Channel, 
      FDM_MSG_RECEIVED * Message );
#endif

/*fonction pour le traitement de la reception d'une trame
de PDU de donnees, dans le cas d'un canal de 
type A_UNIDATA Reception.
Ce PDU peut demander un ack ou non.
On ne traite jamais l'acquittement.
*/
void ExecIndDLL_A_UNIDATA_DataPDU(
      MCSCHANNEL_REF     * Channel, 
      FDM_MSG_RECEIVED * Message );

/*fonction pour le traitement de la reception d'une trame
de PDU de donnees, dans le cas d'un canal de 
type A_DATA Reception.
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecIndDLL_A_DATA_DataPDU(
      MCSCHANNEL_REF     * Channel, 
      FDM_MSG_RECEIVED * Message );
#endif







/* fonction pour le traitement d'une confirmation liaison d'un transfert
de donnees A UNIDATA (immediate)*/
void ExecCnfDLLFor_A_UNIDATA_DataPDU(
      MCSCHANNEL_REF * Channel, 
      MCS_FDM_MSG_TO_SEND * Message);



/* fonction pour le traitement d'une confirmation liaison d'un transfert
de PDU de type 'data' a travers un canal A_DATA */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecCnfDLLFor_A_DATA_DataPDU(
      MCSCHANNEL_REF * Channel, 
      MCS_FDM_MSG_TO_SEND * Message);
#endif

/* fonction pour le traitement d'une confirmation liaison d'un transfert
de PDU de type 'acquittement de donnees' a travers un canal A_DATA */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecCnfDLLFor_A_DATA_AckPDU(
      MCSCHANNEL_REF *     Channel, 
      MCS_FDM_MSG_TO_SEND * Message);
#endif

/* fonction pour le traitement d'un timeout, concernant la
non reception de l'acquittement d'un PDU MCS de type "donnees", 
dans le cas d'un canal de type A_DATA Emission
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecTimeOutFor_A_DATA_AckPDU(
      MCSCHANNEL_REF          *     Channel, 
      struct _MCS_TEMPO_TYPE *      Message);
#endif


/*========== Fonctions pour le bourrage  ==================*/

/* fonction pour le traitement d'un timeout, concernant le
bourrage en emission, dans le cas d'un canal de type A_DATA Emission
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecTimeOutFor_A_DATA_Stuffing(
      MCSCHANNEL_REF *              Channel, 
      struct _MCS_TEMPO_TYPE *      Message);
#endif





/*========== Fonctions pour la configuration  =============*/



/*procedure qui preremplit les ressources qui constituent la fenetre 
d'anticipation en emission */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoadDefaultParamInTransmWindow (MCSCHANNEL_REF *);
#endif

/*procedure qui preremplit les ressources qui constituent la fenetre 
d'anticipation en reception */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoadDefaultParamInRecepWindow (MCSCHANNEL_REF *);
#endif

/*procedure qui preremplit les ressources qui sont utiliser pour
produire l'acquittement, en reception */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoadDefaultParamInAcknowledgeRessource (
      MCS_ACK_MSG_TO_SEND *, 
      MCSCHANNEL_REF *);
#endif




/*========== Fonctions de depilage des files d'inter. =====*/

/* fonction a la sortie de la file des commandes des canaux
(depiler une commande et la traiter)*/
void FileOut_CommandElement(void *);

/* fonction a la sortie de la file des demandes de transfert MCS 
(depiler une requete et la traiter)*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void FileOut_SendMCSElement(void *);
#endif

/* fonction a la sortie de la file des confirmation de transfert Liaison 
(depiler une confirmation et la traiter)*/
void FileOut_SendDLLCnfElement(void *);

/* fonction a la sortie de la file des indications de transfert Liaison 
(depiler une indication et la traiter)*/
void FileOut_ReceiveDLLIndElement(void *);


/* fonction a la sortie de la file des signalisation des tempo echues 
(depiler une signalisation et la traiter)*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void FileOut_TimeOutElement(void *);
#endif 

/* fonction a la sortie de la file des liberations de buffers d'indications
MCS (depiler une liberation et la traiter)*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void FileOut_FreeBufferElement(void *);
#endif

/*========== Fonctions 'outils' ===========================*/

/* procedure qui donne le type d'une adresse DLL FIP: renvoie Individuel ou Groupe */
_FIP_ADDRESS_TYPE_LIST FipAddressType ( unsigned long Address );

/* procedure qui teste si un nombre est une puissance de 2 ou pas 
retourne _YES ou _NO*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
int IsAPowerOfTwo(short );
#endif

/* procedure qui calcule le nombre d'octets de donnees
contenues dans une demande utilisateur*/
unsigned short UserDataRequestLength(
    unsigned int           NrOfBlocks ,
    DESC_BLOCK_DATA_T     *P_Start );

/*fonction pour le recyclage des ressources qui constituent
un SDU indique a l'utilisateur.
Cette fonction est soit appelee 'directement' par l'utilisateur,
lorsqu'il s'agit d'un liberation pour un canal de type non
associe, soit a la suite de la mise en file.
*/
void Exec_FreeDataSDURessource(struct _MCS_SDU_RECEIVED *,MCSCHANNEL_REF *);



/* procedure qui verifie que le contenu des 2 types FDM_MSG_R_DESC
et DESC_BLOCK_DATA_R sont identiques.
Cette procedure est appelee au moment de l'initialisation de l'instance.
Si on n'a pas egalite entre ces 2 types, il faut revoir le code de MCS.
Car de nombreux 'cast' sont faits entre ces 2 types
_MCS_ERROR_CODE_LIST CheckReceptionStructure(void);*/


/* procedure qui est appellee apres avoir le resultat d'un transfert 
DLL. Cette procedure met a jour les compteurs de statistiques reserves
pour les resultats de transfert niveau laison de donnees, et retourne 
un resume du resulat
De plus, cette fonction genere les Warnings/Error */
_MCS_ERROR_CODE_LIST GiveSendDLLResultAndRefreshStatistics
      ( 
       MCSCHANNEL_REF *           Channel,
       FDM_MSG_TO_SEND *           FdmToSend);


/*========== Fonctions specifiques A_UNIDATA Emission =====*/

/* fonction pour le traitement d'un transfert de donnees sur un
canal  de type A_UNIDATA (immediat) */
_MCS_ERROR_CODE_LIST Exec_A_UNIDATA_TransferRequest(
          MCSCHANNEL_REF *Channel,
          MCS_SDU_TO_SEND* Message);


/*========== Fonctions specifiques A_DATA Emission ========*/

/* procedure qui declenche les traitements d'emission
d'un canal de type associe */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoopTransmissionMCSConnectedServices(MCSCHANNEL_REF *);
#endif


/*procedure qui declenche ou non (en fonction de la demande et 
des ressources disponibles a l'instant)
la fabrication de MCS_PDU a transferer, dans la fenetre d'antipation 
etendue.
Ces procedures renvoient un indicateur qui informe si elles ont
travaille (_WORK_IS_DONE) ou non (_NOTHING_IS_DONE), ie si elles
ont alimente la machine de transfert des MCS PDUs ou non.
*/ 
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
_PROCESS_ACTIVITY_LIST ProcessMakeMonoframePdus (MCSCHANNEL_REF *);
_PROCESS_ACTIVITY_LIST ProcessMakeMultiframePdus (MCSCHANNEL_REF *);
#endif

/* procedures qui fabriquent un PDU a transferer, dans le cas
d'un canal de type connecte, en emission.
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ProcessMakeOneMonoframePdu (MCS_SDU_TO_SEND *, MCSCHANNEL_REF *);
void ProcessMakeOneMultiframePdu (MCSCHANNEL_REF *);
#endif


/*procedure qui declenche ou non (en fonction de la demande et 
des ressources disponibles a l'instant)
le transfert de MCS_PDU dans la fenetre d'antipation.
Cette procedure renvoie un indicateur qui informe si elle a
travaille (_WORK_IS_DONE) ou non (_NOTHING_IS_DONE), ie si elles
a eventuellement libere des ressources pour fabriquer des 
nouveaux MCS_PDUs.
*/ 
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
_PROCESS_ACTIVITY_LIST ProcessSendPdus (MCSCHANNEL_REF *);
#endif

/* procedure qui engage l'emission du PDU de plus grand
numero dans la fenetre d'anticipation de transfert, et procede a
l'extension de cette fenetre d'une case
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void SendNext_A_DATA_PduAndExtendTransmWindow(MCSCHANNEL_REF *);
#endif


/*procedure qui est appelee lorsque, dans un canal de type connecte emission,
apres n repetitions de l'emission d'un MCS PDU contenant des donnees,
on n'a toujours pas recu d'acquittement et le timeout correspondant sonne.
Elle permet d'engager l'annulation du transfert du SDU auquel appartient le PDU 
dont le transfert a echoue
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void  CancelSDUTransfer(TRANSMISSION_WINDOW_ELEMENT_TYPE * , MCSCHANNEL_REF *);
#endif

/*========== Fonctions specifiques A_DATA Reception ========*/

/*Cette fonction permet de trouver position relative d'un PDU,
defini par un numero de sequence, par rapport a la fenetre de reception
a l'instant T
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
_POSITION_IN_RECEP_WINDOW_LIST FindPositionInRecepWindow (
         unsigned short NumSequence, 
         MCSCHANNEL_REF *  Channel );
#endif


/* Cette fonction engage le glissement de la fenetre de reception, 
jusqu'au PDU de sequence 'SequenceNumber', et stocke dans la ressource
'DLLMessage'.
On peut engager un 'petit' glissement, dans les cas normaux, ou bien
un 'grand' glissement, dans les  autres cas (parametre 'Type').
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void RecepWindowScroll (     
     unsigned short                SequenceNumber   ,
     FDM_MSG_RECEIVED            * DLLMessage       ,
     MCSCHANNEL_REF             * Channel         ,
    _RECEP_WINDOW_SCROLL_TYPE_LIST Type             );      
#endif

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
/* Cette fonction stocke un nouveau PDU contenu dans DLLMessage, et
de sequence 'SequenceNumber', dans la fenetre d'anticipation en reception.
Cette fonction est utilise lorsqu'on recoit un PDU qui est inclus dans
la fenetre de reception exitante a l'instant T.
Cette fonction gere l'antiduplication et l'acquittement du PDU
*/
void StoreReceivePDUWithoutDuplicAndSendAcknowledge(
     unsigned short                SequenceNumber   ,
     FDM_MSG_RECEIVED            * DLLMessage       ,
     MCSCHANNEL_REF             * Channel           );
#endif


#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
/* Cette fonction construit et envoi l'acquittement d'un PDU
dont le numero de sequence est SequenceNumber*/
void SendPDUAcknowledge(
     unsigned short                SequenceNumber   ,
     MCSCHANNEL_REF             * Channel           );

#endif

/* procedure qui declenche les traitements de reception
d'un canal de type associe */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoopReceptionMCSConnectedServices(MCSCHANNEL_REF *);
#endif

/*procedure qui permet la prise en compte du PDU recu le plus ancien
(dans la numerotation) et non encore fourni a l'utilisateur, 
dans le cas d'un canal A_DATA reception de type Monotrame
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
_PROCESS_ACTIVITY_LIST TakeNextMonoframeChannelPDU (
       RECEPTION_WINDOW_ELEMENT_TYPE *, 
       MCSCHANNEL_REF *);
#endif

/*procedure qui permet la prise en compte du PDU recu le plus ancien
(dans la numerotation) et non encore fourni a l'utilisateur, 
dans le cas d'un canal A_DATA reception de type Multitrame
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
_PROCESS_ACTIVITY_LIST TakeNextMultiframeChannelPDU (
       RECEPTION_WINDOW_ELEMENT_TYPE *, 
       MCSCHANNEL_REF *);
#endif


/*procedure qui remet le contexte de reassemblage dans l'etat 
initial, ie sans reassemblage en cours, apres avoir recycle
toutes les ressources qui y etaient stockees*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void  PurgeAndResetAssemblingContext(MCSCHANNEL_REF *);
#endif

/*========== Fonctions pour les temporisations ============*/

/* temporisation pour le time out sur la non reception 
des acquittement des PDU transportant des donnees:
     -creation, a la creation du canal
     -demarrage
     -relance
     -arret
(canal de type Connecte, emission)
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void CreatePDU_A_DATA_Tempo(MCSCHANNEL_REF *);
void StartPDU_A_DATA_Tempo(TRANSMISSION_WINDOW_ELEMENT_TYPE *);
void StopPDU_A_DATA_Tempo(TRANSMISSION_WINDOW_ELEMENT_TYPE *);
#endif

/* temporisation pour l'emission d'un PDU de bourrage
     -creation, a la creation du canal
     -demarrage
(canal de type Connecte, emission)
*/

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void CreateStuffingTempo(MCSCHANNEL_REF *);
void StartStuffingTempo(MCSCHANNEL_REF *);
void RestartStuffingTempo(MCSCHANNEL_REF *);

#endif

/*temporisation servant au controle du
flux d'emissions des PDUs, a travers  un canal d'emission de 
type A_DATA, avec la fonction "bourrage" choisie.
     -creation, a la creation du canal
     -demarrage

*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void CreateTransmWatchdogTempo(MCSCHANNEL_REF *);
void StartTransmWatchdogTempo(MCSCHANNEL_REF *);
void RestartTransmWatchdogTempo(MCSCHANNEL_REF *);

#endif

/*temporisation servant au controle du
flux de reception des PDUs, a travers  un canal de reception de 
type A_DATA, avec la fonction "bourrage" choisie.
     -creation, a la creation du canal
     -demarrage

*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void CreateRecepWatchdogTempo(MCSCHANNEL_REF *);
void StartRecepWatchdogTempo(MCSCHANNEL_REF *);
void RestartRecepWatchdogTempo(MCSCHANNEL_REF *);

#endif


/*fonction, qui sera appelee par le gestionnaire de tempo,
lors de l'echeance de la temporisation qui surveille le
flux d'emission des PDUs, a travers un canal connecte de type
emission.
idem pour la surveillance du flux de reception des PDUS, 
dans le cas d'un canal connecte de type reception.
*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void InternSign_TransmWatchdogTempo(TEMPO_DEF *);
void InternSign_RecepWatchdogTempo(TEMPO_DEF *);
#endif


/*fonction interne, pour le dysfonctionnement d'un canal de 
type A_DATA.
Cette fonction signalera le probleme a ACSE, dans le cas
ou cette option est choisie */
 
void Intern_Signal_ConnexionFailure (
       MCS_REF *,
       MCSCHANNEL_REF *, 
       _CONNEXION_FAILURE_LIST); 


#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
_MCS_ERROR_CODE_LIST CheckCommandTowardsChannelFeaturesAndState
    ( MCSCHANNEL_REF             *  Channel ,
     _MCS_CHANNEL_COMMAND_LIST   Command);
#endif



MCSCHANNEL_REF*  McsChannelCreate (MCS_REF *  ,
								   MCS_CHANNEL_CONFIGURATION *,
								  _MCS_ERROR_CODE_LIST *);



/*========== Fonctions pour la destruction de canal =======*/

/*Fonction qui ne peut etre appellee que suite a une action 
du sequenceur. 
On n'est pas protege au moment de cet appel.
Elle est appelee lorsqu'un evenement reponse revient dans MCS,
apres etre passe dans une file d'interconnexion.
Si une demande de destruction a ete faite, et que le nombre
de ressources de MCS dehors = 0, et que le nbre de ressources 
exterieures dans MCS = 0, alors on detruit le canal.
Sinon, on ne fait rien*/

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
void ExecuteDeleteIfRequestedAndPossible(MCSCHANNEL_REF * Channel);
#endif

/*fonction qui engage toutes les actions preliminaires pour la destruction
du canal.
Fonction appelee au moment de la prise en compte de la destruction du 
canal*/
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
void ExecuteDeletePreliminaryActions(MCSCHANNEL_REF * Channel);
#endif


/*fonction qui engage toutes les actions qui n'on pas
ete effectuees pendant la phase "preliminaire" de la destruction
du canal.
Fonction appelee au moment de la destruction effective du canal.
Cette fonction envoie a ACSE, ou equivalent, la consfirmation 
de la destruction du canal*/
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
void ExecuteDeleteFinalActions(MCSCHANNEL_REF * Channel);
#endif

/*************** End Of  'mcsprive2.h' File ******************************/

#endif
