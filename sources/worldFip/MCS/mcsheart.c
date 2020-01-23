/************************************************************
   
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcsheart.c
  Description     : Ensemble des traitements qui font fonctionner
                    les canaux de communication MCS.                         
   
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000 

************************************************************************/


#include "mcs.h"


/*******************************************************************/
/********** FONCTION PUBLIC-USER mcs_fifos_empty()  ****************/
/*******************************************************************/
void mcs_fifos_empty( MCS_REF * Mcs )
{
   /*faire tourner une instance de MCS */
 
    TASK_LOOP( &Mcs->Task );
   
} 

 


/*******************************************************************/
/********** FONCTION PUBLIC-USER mcs_send_message() ****************/
/*******************************************************************/
_MCS_ERROR_CODE_LIST mcs_send_message (
        MCSCHANNEL_REF                  * Channel, 
        MCS_SDU_TO_SEND                  * Message)
/* cette fonction est appelee par l'utilisateur lorsque celui-ci souhaite
   envoyer un message sur un canal existant.

   Les actions engagees dependent du type de canal:
	- si le canal est de type A_UNIDATA, le traitement est effectue 
                immediatement
        - si le canal est de type A_DATA, les demandes sont fiforisees 
          en attente de traitement.


    Cette fonction controle que le pointeur sur le canal n'est pas une mauvaise 
    reference. Si le pointeur est mauvais, on renvoie un compte-rendu NOK.
    Et renvoie un message d'erreur grave (Fatal Erreur)

    Puis la fonction teste l'etat du canal:
	si le canal est dans l'etat IDLE ou DELETE_IN_PROGRESS, rejette aussi la demande
        , et appelle la procedure de WARNING correspondante.

        Puis, dans le cas d'un canal A_DATA, teste le  stocke le message dans la file des 
        demandes d'emission pour l'instance MCS, fait +1 dans le compteur des demandes entrees
        (qui sert pour la destruction) pour ce canal et retourne OK.

        Dans le cas d'un canal de type A_UNIDATA, engage directement le traitement de la demande
        de transfert et retourne OK. Sauf si rejet de la DLL (FDM)).

NOTES: 
1) dans le cas d'un canal de type A_UNIDATA, le canal ne peut-etre dans l'etat DELETE
IN PROGRESS car le service de destruction n'est pas autorise.
2) la sequence ( test etat & compteur++) doit etre protegee vis a vis du changement
d'etat du canal, qui peut arriver si une commande de STOP ou DELETE est prise en compte par 
MCS. Pour etre propre on se protege aussi vis a vis de l'ordre START, dans les 2 cas A_UNIDATA et
A_DATA.
3) les compteurs de statistiques sont incrementes 

*/

{    /* ici */
       MCS_REF *  Mcs         = Channel->Mcs;
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
       INTEG_FILE_TYPE*  File = & (Mcs->List[_SendMCSList]);
#endif



       MCS_BEGIN_CRITICAL(Mcs);

       
       if ((Channel->CheckNumber) != _rangdom_number ) {
           /* le pointeur sur le canal est mauvais */

            MCS_END_CRITICAL(Mcs);
			Mcs->Extern_Signal_Error (
				   _MCS_TYPE_MCS_SDU_TO_SEND,
                   (void *)Message,
                   _MCS_SEND_DATA_ON_INEXISTING_CHANNEL ) ;
           return _MCS_SEND_DATA_ON_INEXISTING_CHANNEL;
       }
       

       /* verification de l'etat */     
       switch(Channel->State) {

            case _MCS_CHANNEL_STATE_IDLE_TO_START:
            case _MCS_CHANNEL_STATE_IDLE_TO_DELETE:
            MCS_END_CRITICAL(Mcs);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            Channel->Statistics[_SendDATARequestOnIDLEChannel]++;
#endif	   
            Mcs->Extern_Signal_Warning (  
				   _MCS_TYPE_MCS_SDU_TO_SEND,
                   (void *)Message,
                   _MCS_SEND_DATA_ON_IDLE_CHANNEL ) ;
            return _MCS_SEND_DATA_ON_IDLE_CHANNEL;            
            
        
            case _MCS_CHANNEL_STATE_RUNNING:
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            Channel->Statistics[_SendDATARequestOnRUNNINGChannel]++;
#endif
            break;

    
            default:
            MCS_END_CRITICAL(Mcs);
            Message->Service_Report=_MCS_INTERNAL_ERROR;
					Mcs->Extern_Signal_Error (  
					_MCS_TYPE_MCS_SDU_TO_SEND,
                   (void *)Message,
                   Message->Service_Report ) ;            
            return _MCS_INTERNAL_ERROR;

       }

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

       Message->Private_MCS.Stuffing=_MCS_NO;
     
       if ((Channel->ServType ) ==_CHANNEL_FOR_CONNECTED_SERVICES ) {
          Message->Channel.McsChannelRef=Channel;
          Message->Channel.UserChannelRef=Channel->UserChannelRef;
      OS_Enter_Region();
          STORE_IN_FIFO_INTEG(File  , (INTEG_FILE_ELT *)Message );
      OS_Leave_Region();

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
          Channel->NumberOfInRequests ++;
#endif

          MCS_END_CRITICAL(Mcs);
          return _MCS_OK;
       } else {
#endif

          MCS_END_CRITICAL(Mcs);
          return Exec_A_UNIDATA_TransferRequest(Channel, Message);

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
       }
#endif
}

/*******************************************************************/
/********** FONCTION PUBLIC-USER free_mcs_received_message()********/
/*******************************************************************/
void free_mcs_received_message(MCS_SDU_RECEIVED * Message)
{

/* fonction, appelee par l'utilisateur,
qui permet de recycler les ressources de type indications
que MCS avait fourni a l'utilisateur au moment de l'indication de 
reception.


   Les actions engagees dependent du type de canal:
	- si le canal est de type A_UNIDATA, le traitement est effectue 
                immediatement
        - si le canal est de type A_DATA, les demandes sont fiforisees 
          en attente de traitement.

*/

    MCSCHANNEL_REF * Channel = Message->Channel.McsChannelRef;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     /* compteur-> liberation SDU par l'utilisateur */
     Channel->Statistics[ _FreeDataSDUReceived ]++;
#endif 


 
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

       if ((Channel->ServType ) ==_CHANNEL_FOR_CONNECTED_SERVICES ) {

          OS_Enter_Region();
          STORE_IN_FIFO_INTEG(
	     &(Channel->Mcs->List[_FreeBufferList]) , 
             (INTEG_FILE_ELT *)Message );
         OS_Leave_Region();

       } else {
#endif

       Exec_FreeDataSDURessource(Message,Channel);

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
       }
#endif

}


/*******************************************************************/
/****** FONCTION PUBLIC-FDM MCSDllConfirmationFunction() ***********/
/*******************************************************************/
void MCSDllConfirmationFunction (
     FDM_MESSAGING_REF* FDM_Channel_Ref,
     FDM_MSG_TO_SEND *  Message )

{
/* cette fonction est appelee par FDM au moment de la confirmation 
de transfert DLL.
On verifie, par acquis de conscience que le pointeur du canal 
pointe bien sur quelque chose. Dans le cas contraire, on siganle une
erreur GRAVE.

On ne se preocupe pas ici de l'etat du canal, les procedures de traitement
se chageront de ca.

Dans le cas ou le canal est de type A_UNIDATA, on declenche immediatement
le traitement de la confirmation en appelant la procedure adequate.

Dans le cas ou le canal est de type A_DATA, on stocke la confirmation dans
la file correspondantee, si c'est la confirmation d'une demande de transfert, 
    -d'un PDU MCS
    -d'un acquittement de PDU MCS
    -d'un PDU ACSE


Lors de la demande de transmission DLL, MCS avait etendu la
ressource passee a FDM, et rajoute un champ permettant d'indiquer si
demande ACSE ou non. La valeur de ce champ permet d'aiguiller vers
A_DATA-Transfert de donnees, type PDU ou acquittement de PDU, ou
bien A_DATA-Transfert de PDUs ACSE.
*/

 
  
      MCS_FDM_MSG_TO_SEND * MyDemand = 
         (MCS_FDM_MSG_TO_SEND *) Message;

      MCSCHANNEL_REF * Channel= MyDemand->Channel;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     /* compteur-> confirmation de transfert DLL */
     Channel->Statistics[ _CnfDLLReceived ]++;
#endif 

      if (Channel->ServType == _CHANNEL_FOR_UNCONNECTED_SERVICES ) {
            ExecCnfDLLFor_A_UNIDATA_DataPDU(Channel, MyDemand);
      } else {

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

          OS_Enter_Region();
            STORE_IN_FIFO_INTEG( 
                   & (Channel->Mcs->List[_SendDLLCnfList]) , 
                  (INTEG_FILE_ELT *)Message );
         OS_Leave_Region();
#endif
      }
      
}

/*******************************************************************/
/****** FONCTION PUBLIC-FDM MCSDllIndicationFunction() *************/
/*******************************************************************/
void MCSDllIndicationFunction (
    FDM_MESSAGING_REF* Msg_Ref,
    FDM_MSG_RECEIVED * Msg_Received)
{

/* cette fonction est appelee par FDM au moment de l'indication 
d'un transfert DLL.
On recupere la reference du canal MCS auquel le message s'adresse

Si le canal n'existe pas, (ie si on se rend compte
que la reference pointe sur n'importe quoi)
on considere que c'est une erreur grave, 
On en informe l'integrateur, et on libere immediatement le message 
recu.


Si le canal n'est pas dans l'etat RUNNING, on rejette le message
en mettant a jpur les compteurs de statistiques,et en provoquqnt un Warning.

Si le canal est dans l'etat RUNNING, on stocke le message dans la file
correspondante, en on incremente le compteur des indications 
dll entrees sur ce canal.
*/

    MCSCHANNEL_REF  *Channel = (MCSCHANNEL_REF *) 
            (Msg_Received->User_Qid);
    MCS_REF          *Mcs     = Channel-> Mcs;


       MCS_BEGIN_CRITICAL(Mcs);
       
       if ((Channel->CheckNumber) != _rangdom_number ) {
           /* le pointeur sur le canal est mauvais */

            MCS_END_CRITICAL(Mcs);
			Mcs->Extern_Signal_Error ( 
				  _MCS_TYPE_FDM_MSG_RECEIVED,
                   (void *)Msg_Received,
                   _MCS_RECEIVE_DLL_ON_INEXISTING_CHANNEL ) ;
		{
		/* liberer dans le bon ordre  */
		    FDM_MSG_R_DESC * BlocData = Msg_Received->Ptr_Block;

            fdm_msg_ref_buffer_free(Msg_Received);
		    fdm_msg_data_buffer_free(BlocData);

        }
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
 
            return  ;
       }
       

       /* verification de l'etat */
     
       switch(Channel->State) {

            case _MCS_CHANNEL_STATE_IDLE_TO_START:
            case _MCS_CHANNEL_STATE_IDLE_TO_DELETE:
            MCS_END_CRITICAL(Mcs);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            Channel->Statistics[_ReceiveFromDLLOnIDLEChannel]++;
#endif
	    Mcs->Extern_Signal_Warning (  
					_MCS_TYPE_FDM_MSG_RECEIVED,
                   (void *)Msg_Received,
                   _MCS_RECEIVE_DLL_ON_IDLE_CHANNEL) ;

		{
		    /* liberer dans le bon ordre */ 

		    FDM_MSG_R_DESC * BlocData = Msg_Received->Ptr_Block;

            fdm_msg_ref_buffer_free(Msg_Received);
            fdm_msg_data_buffer_free(BlocData);

		}
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
            return;            

         
            case _MCS_CHANNEL_STATE_RUNNING:
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            Channel->Statistics[_ReceiveFromDLLOnRUNNINGChannel]++;
#endif
            break;

    
            default:
            MCS_END_CRITICAL(Mcs);
			Mcs->Extern_Signal_Error ( 
					_MCS_TYPE_FDM_MSG_RECEIVED,
                   (void *)Msg_Received,
                   _MCS_INTERNAL_ERROR) ;   
		{
		    /* liberer dans le bon ordre  */

		    FDM_MSG_R_DESC * BlocData = Msg_Received->Ptr_Block;

            fdm_msg_ref_buffer_free(Msg_Received);
            fdm_msg_data_buffer_free(BlocData);

		}

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
            return ;

       }

       OS_Enter_Region();
       STORE_IN_FIFO_INTEG(
	     &(Mcs->List[_ReceiveDLLList]) , 
             (INTEG_FILE_ELT *)Msg_Received);
       OS_Leave_Region();
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
       Channel->NumberOfInRequests +=2;
#endif

       MCS_END_CRITICAL(Mcs);

}

/*******************************************************************/
/****** FONCTION PRIVATE Exec_A_UNIDATA_TransferRequest() **********/
/*******************************************************************/
_MCS_ERROR_CODE_LIST Exec_A_UNIDATA_TransferRequest(
          MCSCHANNEL_REF *Channel,
          MCS_SDU_TO_SEND* Message)

{
/* cette fonction est appelee directement par
le send_message(), lorsque la demande concerne un canal A_UNIDATA.
On fait un controle sur la requete (longueur, adresse destination), puis on
construit le PDU MCS, et on demande son transfert au
niveau DLL
*/
   unsigned short          lg;
   unsigned short          invocation_nr;
   MCS_FDM_MSG_TO_SEND *   P_Area= &(Message->Private_MCS.McsFdmToSend);

   lg=UserDataRequestLength(Message->Nr_Of_Blocks, Message->Ptr_Block);


   /* controle de la longueur */
   if ((lg==0) || (lg > Channel->MaxSDUSize) ) {
	Channel->Mcs->Extern_Signal_Warning (
			_MCS_TYPE_MCS_CHANNEL_REF,
           (void *) Channel ,
           _MCS_SEND_DATA_INVALID_LENGTH );

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     /* compteur-> invalidite de la demande faite par l'utilisateur  */
     Channel->Statistics[ _RejectDataSDUTransmissionRequest ]++;
#endif 
      return _MCS_SEND_DATA_INVALID_LENGTH;
  
   }

    
    /* controle de l'adresse destination :
    on controle que l'utilisateur donne une veritable 
    adresse liaison de donnees, sur 24 bits.
    MCS positionnera le bit de poids fort a 1 
    pour la transmission sans FDM */

    if ( (( Message->Destination) & 0xFF000000 ) != 0) {
	Channel->Mcs->Extern_Signal_Warning ( 
			_MCS_TYPE_MCS_SDU_TO_SEND,
           (void *) Message ,
           _MCS_SEND_DATA_INVALID_DESTINATION );

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     /* compteur-> invalidite de la demande faite par l'utilisateur  */
     Channel->Statistics[ _RejectDataSDUTransmissionRequest ]++;
#endif 
     return _MCS_SEND_DATA_INVALID_DESTINATION;
 
    }
   
 
    /*construction de l'entete */
    P_Area->Data[0]=0x03;
            /* non acquitte, priorite 0 : */
    P_Area->Data[1]=0x10;
    MCS_BEGIN_CRITICAL(Channel->Mcs);
    invocation_nr=++Channel->CurrentInvocationNumber;
    MCS_END_CRITICAL(Channel->Mcs);


    P_Area->Data[2]= (unsigned char) (invocation_nr >> 8);
    P_Area->Data[3]= (unsigned char) (invocation_nr );

    /* construction du micro descripteur */
    {
       FDM_MSG_T_DESC * D= (FDM_MSG_T_DESC *)
          (&(Message->Ptr_Block->Private));
          
       D->Nr_Of_Bytes = 4;
       D->Ptr_Data    = P_Area->Data;
       D->Next_Block  = (FDM_MSG_T_DESC *) 
          (&(Message->Ptr_Block->User_Block));
    }

    /* construction du descripteur */
    P_Area->FdmToSend.Nr_Of_Blocks = Message->Nr_Of_Blocks +1;
    P_Area->FdmToSend.Ptr_Block    = (FDM_MSG_T_DESC *) 
          (&(Message->Ptr_Block->Private));
    P_Area->FdmToSend.Remote_DLL_Address
           = Message->Destination | 0x80000000L;
    

    /* referencements*/
    P_Area->Channel    =Channel;
    P_Area->Ctxt       =(void *) Message;

    /* demande de transfert */

    { 
         unsigned short Cr;

        Cr=fdm_send_message(
         Channel->Lsap_Ctxt,
         & (P_Area->FdmToSend));

        if (Cr==FDM_OK) {
            return _MCS_OK;
        
        } else {
	   Channel->Mcs->Extern_Signal_Error (  
			_MCS_TYPE_MCS_SDU_TO_SEND,
             (void *) Message ,
              _MCS_INTERNAL_ERROR );
           return _MCS_INTERNAL_ERROR;
        }
    }

}

/*******************************************************************/
/****** FONCTION PRIVATE UserDataRequestLength() *******************/
/*******************************************************************/
unsigned short UserDataRequestLength(
        unsigned int            NrOfBlocks ,
        DESC_BLOCK_DATA_T     * P )
{
/* 
cette fonction calcule la longueur totale des donnees 
contenues dans la demande de transfert Msg
   renvoie cette longueur,
   renvoie 0 si longueur = 0 ou si un pointeur de bloc est invalide
*/   


     unsigned short         lg = 0;

     if (NrOfBlocks==0) {
           return 0;
     }
     
     do { 
        if ( P== ( DESC_BLOCK_DATA_T *) 0 ) {
            return 0;
        }
        lg += P->User_Block.Nr_Of_Bytes;
        NrOfBlocks--;
        P=P->User_Block.Next_Block;
     } while (NrOfBlocks!=0);

     return lg;
 
}

/*******************************************************************/
/****** FONCTION PRIVATE GiveSendDLLResultAndRefreshStatistics() ***/
/*******************************************************************/

_MCS_ERROR_CODE_LIST GiveSendDLLResultAndRefreshStatistics(
       MCSCHANNEL_REF *           Channel , 
       FDM_MSG_TO_SEND *            FdmToSend )
{
/* procedure qui est appelee apres avoir le resultat d'un transfert 
DLL. Cette procedure met a jour les compteurs de statistiques reserves
pour les resultats de transfert niveau laison de donnees, et retourne 
un resume du resulat.
De plus, cette fonction genere les Warnings/Error 


Liste des erreurs repertoriees:

     Erreur            // Compteur                                  //   Retour                  //  Warning/Error
------------------------------------------------------------------------------------------------------------------------------------------
Pas d'ereur            //  [ _SendToDLLNoError ]                    // _MCS_OK                   //    --
canal FIP n'existe pas //  [ _SendToDLLInexistingChannel ]          // _MCS_SEND_DATA_FAIL       //  W ( _MCS_SEND_DLL_ON_INEXISTING_CHANNEL )      
emission en Image 2    //  [ _SendToDLLBadImage          ]          // _MCS_SEND_DATA_FAIL       //  W ( _MCS_SEND_DLL_ON_IMAGE2 )      
erreur de longueur, ou de pointeur
dans la demande de transfert       
                       //  [ _SendToDLLInternalError     ]          // _MCS_SEND_DATA_FAIL       //  E ( _MCS_SEND_DLL_INTERNAL_ERROR )

Pas d'ack recu, apres
    retry             
                       //  [ _SendToDLLNoAckReceivedDefault   ]     // _MCS_SEND_DATA_FAIL       //    --
Mauvais RP-Msg (cas noack)                    
                       //  [ _SendToDLLBadRPMsgDefault   ]          // _MCS_SEND_DATA_FAIL       //    --
Recep ACK -                    
                       //  [ _SendToDLLNegAckReceivedDefault   ]    // _MCS_SEND_DATA_FAIL       //    --

Time Out                    
                       //  [ _SendToDLLTimeOutDefault   ]    // _MCS_SEND_DATA_FAIL       //    --



Autres Erreurs         //  [ _SendToDLLUnknownError      ]          // _MCS_SEND_DATA_FAIL       //  W ( _MCS_SEND_DLL_UNKNOWN_ERROR )

*/
   FDM_MSG_SEND_SERVICE_REPORT Cr= FdmToSend->Service_Report;

   if (Cr.Valid==_FDM_MSG_SEND_OK) {
       /* le transfert DLL s'est bien passe */
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
       Channel->Statistics[ _SendToDLLNoError ]++;
#endif
       return _MCS_OK ;

   } else if (Cr.Valid==_FDM_MSG_USER_ERROR) {
       /* les erreurs theoriquement impossibles sont les suivantes:
            -erreur sur longueur de message
          les autres erreurs peuvent arriver, en fonction des 
          manipulations externes:
       */
   
       switch (Cr.msg_user_soft_report) {
           case _FDM_MSG_REPORT_CHANNEL_NOT_ALLOWED:
           /* le canal en messagerie FIP n'existe pas */
           Channel->Mcs->Extern_Signal_Warning( 
			   _MCS_TYPE_MCS_CHANNEL_REF,
               (void *) Channel, 
               _MCS_SEND_DLL_ON_INEXISTING_CHANNEL);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLInexistingChannel ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;

           case _FDM_MSG_REPORT_MSG_NOT_ALLOWED:
           /* l'emission a ete demandee alors que IMAGE=2 */
           Channel->Mcs->Extern_Signal_Warning(
			   _MCS_TYPE_MCS_CHANNEL_REF,
               (void *) Channel, 
              _MCS_SEND_DLL_ON_IMAGE2 );

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLBadImage ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;

           case _FDM_MSG_REPORT_ERR_LG_MSG:
           case _FDM_MSG_REPORT_ERR_MSG_INFOS:
           /* Erreur longueur ou structure de donnees */
           Channel->Mcs->Extern_Signal_Warning( 
			   _MCS_TYPE_MCS_CHANNEL_REF,
               (void *) Channel, 
               _MCS_SEND_DLL_INTERNAL_ERROR );
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLInternalError ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;

           default:
           Channel->Mcs->Extern_Signal_Warning( 
			   _MCS_TYPE_MCS_CHANNEL_REF,
               (void *) Channel, 
                _MCS_SEND_DLL_UNKNOWN_ERROR);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLUnknownError ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;
       }

  } else if (Cr.Valid==_FDM_DATA_LINK_ERROR) {
           /*erreur type liaison de donnees */

       switch (Cr.Way) {

           case _FIP_ACK_NO_REC_AFTER_RETRY:
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLNoAckReceivedDefault ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;

           case _FIP_NOACK_BAD_RP_MSG:
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLBadRPMsgDefault ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;


           case _FIP_ACK_NEG_AFTER_RETRY:
           case _FIP_ACK_NEG_NO_RETRY:
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLNegAckReceivedDefault ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;


           case _FDM_TIME_OUT:
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLTimeOutDefault ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;

           default:
           Channel->Mcs->Extern_Signal_Warning( 
			   _MCS_TYPE_MCS_CHANNEL_REF,
               (void *) Channel, 
               _MCS_SEND_DLL_UNKNOWN_ERROR);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           Channel->Statistics[ _SendToDLLUnknownError ]++;
#endif
           return _MCS_SEND_DATA_DLL_FAIL ;

      }

  } else {
       Channel->Mcs->Extern_Signal_Warning( 
			   _MCS_TYPE_MCS_CHANNEL_REF,
               (void *) Channel, 
          _MCS_SEND_DLL_UNKNOWN_ERROR);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
       Channel->Statistics[ _SendToDLLUnknownError ]++;
#endif
       return _MCS_SEND_DATA_DLL_FAIL ;
  }
     

}


/*******************************************************************/
/****** FONCTION PRIVATE ExecCnfDLLFor_A_UNIDATA_DataPDU() *********/
/*******************************************************************/
void ExecCnfDLLFor_A_UNIDATA_DataPDU(
      MCSCHANNEL_REF     * Channel,
      MCS_FDM_MSG_TO_SEND * Message)

/* cette fonction est appelee au moment de la confirmation
de transfert DLL, lorsque la demande  concernee est une 
requete A_UNIDATA.
MCS traite directement cet evenement, sans passer par 
une fiforisation.
Cette procedure 
	-teste le compte-rendu de transfert niveau DLL
        et met a jour le compte-rendu differe pour l'utilisateur
	-met a jour les compteurs de statistiques
        -confirme au niveau utilisateur, en appelant la procedure 
        associee.
*/
{
   _MCS_ERROR_CODE_LIST Cr;

   MCS_SDU_TO_SEND * User_Rq
               = (MCS_SDU_TO_SEND *) Message->Ctxt;

   Cr= GiveSendDLLResultAndRefreshStatistics(
                Channel , &(Message->FdmToSend));

   if (Cr == _MCS_OK) 
       User_Rq->Service_Report = _MCS_OK; 
   else 

       User_Rq->Service_Report = _MCS_NOK; 

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     /* compteur-> succes ou echec de la demande de transfert du SDU  */
     if (Cr== _MCS_OK) 
         Channel->Statistics[ _SendDATARequestOK ]++;
     else 
         Channel->Statistics[ _SendDATARequestNOK ]++;

     /* compteur->confirmation de transfert du SDU  */
     Channel->Statistics[ _ConfirmDataSDUTransmission ]++;

#endif 

    /* confirmer a l'utilisateur */
   Channel->User_Msg_Ack (Channel->UserChannelRef,User_Rq);  

}


/*******************************************************************/
/****** FONCTION PRIVATE FileOut_SendMCSElement() ******************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void FileOut_SendMCSElement( void * Send_Rq)
/* cette fonction est appelee par le sequenceur, quand celui-ci vide
la file des demandes de transferts MCS faites pat l'utilisateur, et
en attente dans la file d'interconnexion.
Ces demandes concernent necessairement des
  Demandes de transfert de donnees a travers des canaux de 
  type connecte

Actions a effectuer:
  -tester la validite de la longueur des donnees a transmettre, et confirmer -
  si longueur invalide.
  ( on ne controle pas   l'adresse destination car en mode connecte l'adresse destination 
    est fourni dans le contexte de messagerie )

  -si on on peut traiter tout ou partie de la demande: on traite
  -si on n'a pas de ressources pour traiter, on stocke la demande dans une file 
  interne au canal.

Dans la pratique, et pour faciliter l'algorithmie, on stockera toujours la demande 
dans la file interne, puis on s'interrogera si on peut traiter ou non.


Note: si la presente fonction est appelee, alors le canal est dans l'etat RUNNING. 
Car, au moment de la commande STOP_CANAL ou DELETE_CANAL, la file d'integration
aura ete purgee des demandes en attente pour le canal considere.


  
*/
{
      MCS_SDU_TO_SEND * Rq      = ( MCS_SDU_TO_SEND *) Send_Rq;
      MCSCHANNEL_REF * Channel = Rq->Channel.McsChannelRef;
      unsigned short    lg;
      _MCS_ERROR_CODE_LIST Cr    = _MCS_OK;


    lg=UserDataRequestLength(Rq->Nr_Of_Blocks, Rq->Ptr_Block);

     /*supprime controle de l'adresse destination
    if ( Rq->Destination  != _MCS_NOT_SIGNICANT_REMOTE_ADDRESS) {
         Cr=  _MCS_SEND_DATA_INVALID_DESTINATION;
    }
	*/
    /* controle de la longueur */
    if ((lg==0) || (lg > Channel->MaxSDUSize) ) {
		       
		 Cr=  _MCS_SEND_DATA_INVALID_LENGTH;

	}


   if (Cr != _MCS_OK) {
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )

    /* decrementer compteur des demandes IN*/
      MCS_BEGIN_CRITICAL(Channel->Mcs);
      Channel->NumberOfInRequests--;
      MCS_END_CRITICAL(Channel->Mcs);     
#endif

      Rq->Service_Report= Cr;
	  Channel->Mcs->Extern_Signal_Warning ( 
			_MCS_TYPE_MCS_SDU_TO_SEND,
              (void *) Rq , 
               Rq->Service_Report);
          /* confirmer negativement a l'utilisateur */
      Channel->User_Msg_Ack (Channel->UserChannelRef,Rq);  

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
	 /*compteur-> echec de la demande de transfert du SDU  */
	 Channel->Statistics[ _SendDATARequestNOK ]++;

	 /*compteur->confirmation de transfert du SDU  */
	 Channel->Statistics[ _ConfirmDataSDUTransmission ]++;

	 /*compteur->rejet de transfert du SDU  */
	 Channel->Statistics[ _RejectDataSDUTransmissionRequest ]++;

#endif 

          return ;
     }
  
      
    /* stockage dans file interne*/
    STORE_FIFO_INT(
         &(Channel->RqList),
        (FIFO_INT_ELT_TYPE * ) Rq );       

    /* lancement boucle de test ressources&traitement*/
    LoopTransmissionMCSConnectedServices(Channel);

}

#endif

/*******************************************************************/
/****** FONCTION PRIVATE FileOut_SendDLLCnfElement() ****************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void FileOut_SendDLLCnfElement(void * Elem )
{
/* cette fonction est appelee par le sequenceur, quand celui-ci vide
la file des confirmations de transferts dll demandes par MCS, et
en attente dans la file d'interconnexion.
Ces confirmations concernent necessairement des transferts:
    -de PDU de type A_DATA donnees (canal d'emission connecte)
    -de PDU de type A_DATA acquittements (canal de reception connecte )
    -de PDU de type A_DATA -ACSE
 

Actions a effectuer:
  
    Aiguiller ces messages vers les procedures de traitement 
    adequates. en fonction du type de confirmation

Note: si la presente fonction est appelee, alors le canal peut-etre
 dans un etat quelconque.
*/

     MCS_FDM_MSG_TO_SEND * Cnf;

     Cnf= (MCS_FDM_MSG_TO_SEND *) Elem;

     switch(Cnf->Type) {

         case _MCS_CONNECTED_DATA_TRANSFER_FOR_PDU:
         ExecCnfDLLFor_A_DATA_DataPDU (
            Cnf->Channel,
            Cnf);
         break;

         case _MCS_CONNECTED_DATA_TRANSFER_FOR_ACK:
         ExecCnfDLLFor_A_DATA_AckPDU (
            Cnf->Channel,
            Cnf);
         break;

         default: 
         /*Aie! impossible*/
         break;
     }
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
     ExecuteDeleteIfRequestedAndPossible(Cnf->Channel);
#endif
 

 
}
#endif


/*******************************************************************/
/****** FONCTION PRIVATE LoopTransmissionMCSConnectedServices() ****/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void LoopTransmissionMCSConnectedServices(MCSCHANNEL_REF * Channel)
/*
Cette fonction est appelee: 
- derriere le depilement d'une demande de transfert MCS, 
qui avait ete ensuite stockee dans la file interne du canal 
reservee a cet effet
- etc..



Cette fonction lance les 2 traitements suivants:
	-fabrication de PDUs par decoupage de SDU
	-transfert acquittes de PDUs
Ces traitements sont vus comme des processus tournant
en parallele.
La fabrication des PDUs alimente la machine de transfert 
de PDUs, et la machine de transfert des PDUs libere des 
ressources permettant la fabrication de nouveaux PDUs.

De plus, par l'intermediaire de la procedure ProcessSendPdus(),
la fenetre de transfert est exploree et on confirme le transfert
des SDUs dont le transfert de chaque PDU est termine.

Cette boucle se termine lorsque les 2 processus n'ont plus
de ressources pour continuer a travailler.
Alors, toute occurence d'un evenement concernant l'un ou l'autre
des processus devra rappeler cette boucle.
*/
{

    for (;;) {

       if (Channel->SDUType==_CHANNEL_FOR_MULTIFRAME_SDU) {
           ProcessMakeMultiframePdus(Channel);
       } else {
           ProcessMakeMonoframePdus(Channel);
       }

       if (ProcessSendPdus (Channel) == _NOTHING_IS_DONE ) 
       {
       		break;
       }

    }  
      
}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE ProcessMakeMonoframePdus() ****************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

_PROCESS_ACTIVITY_LIST ProcessMakeMonoframePdus (MCSCHANNEL_REF *Channel)
/* 
Cette procedure met en route la machine de fabrication des PDUs,
dans le cas Monotrame.
Une ressource est disponible pour la fabrication si le pointeur 
de remplissage pointe sur une case libre.
Un travail est a faire si la file des requetes en attente sur ce canal 
est non vide.
Cette fonction fournit un compte -rendu immediat, qui informe
l'appelant du fait qu'un travail a ou n'a pas ete effectue.
*/
{
   MCS_SDU_TO_SEND                     * Rq;
   TRANSMISSION_WINDOW_ELEMENT_TYPE    * CaseToFull;
   _PROCESS_ACTIVITY_LIST                Activ=_NOTHING_IS_DONE;

   for (;;) {

        CaseToFull= &(Channel->TransmitWindowArea->Box
               [Channel->TransmitWindowFullingIndix]);
        
        if (CaseToFull->State != _Free_Box ) break;
        Rq=(MCS_SDU_TO_SEND *) GET_FIFO_INT(
                 &(Channel->RqList));
        if (Rq==0) break;

        ProcessMakeOneMonoframePdu(Rq, Channel);
        Activ=_WORK_IS_DONE;

   }

   return Activ;

}

#endif

/*******************************************************************/
/****** FONCTION PRIVATE ProcessMakeOneMonoframePdus() *************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void ProcessMakeOneMonoframePdu (
     MCS_SDU_TO_SEND * Message,
     MCSCHANNEL_REF * Channel)
{
/* 
Cette fonction est utilisee par un canal connecte d'emission,
de type Monotrame.
Elle permet la fabrication d'un PDU, a partir de la requete,
c'est a dire la mise a jour de la case correspondante 
dans la fenetre d'anticipation.
Il faut:
	-determiner le numero de sequence
	-mettre a jour les donnees a transmettre, sous forme de
       liste chainee de blocs, en ajoutant l'entete
	-changer l''etat' de la case qu'on vient de remplir
	-referencer, dans la case, la demande utilisateur
        -faire glisser la fenetre de remplissage
*/
        
    TRANSMISSION_WINDOW_ELEMENT_TYPE*  Box           = 
       &(Channel->TransmitWindowArea->Box[Channel->TransmitWindowFullingIndix]);

    MCS_FDM_MSG_TO_SEND *              P_Area        = 
            &(Box->AreaToSend);
    unsigned short                     invocation_nr = 
                   Channel->CurrentInvocationNumber;


    Box->SequenceNumber= invocation_nr;
    Box->State         = _Available_Pdu;
    Box->RequestRef    = Message;

    /*construction de l'entete : la taille et la nature ont deja ete 
      preremplis*/

    P_Area->Data[2]= (unsigned char) (invocation_nr >> 8);
    P_Area->Data[3]= (unsigned char) (invocation_nr );

	/* pour le PDU suivant */
	Channel->CurrentInvocationNumber++;

    /* construction du micro descripteur, tout est fait,
       sauf le chainage sur le bloc suivant */
    {
       FDM_MSG_T_DESC * D= (FDM_MSG_T_DESC *)
          (&(Box->BlockToSend.User_Block));
          
       D->Next_Block  = (FDM_MSG_T_DESC *) 
	      (&(Message->Ptr_Block->User_Block));
    }

    /* construction du descripteur */
    P_Area->FdmToSend.Nr_Of_Blocks = Message->Nr_Of_Blocks +1;
    
    /* referencement de la demande utilisateur*/
    Box->RequestRef    =Message;

   /* faire glisser la fenetre */
    _PDUMakeProcessScroll(Channel);                             


}
#endif


/*******************************************************************/
/****** FONCTION PRIVATE ProcessMakeMultiframePdus() ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

_PROCESS_ACTIVITY_LIST ProcessMakeMultiframePdus (MCSCHANNEL_REF *Channel)
/* 
Cette procedure met en route la machine de fabrication des PDUs,
dans le cas Multitrame.
Une ressource est disponible pour la fabrication si le pointeur 
de remplissage pointe sur une case libre.
Un travail est a faire si la machine de segmentation est
occupee par un message en cours de segmentation, ou bien
si la file des requetes en attente sur ce canal 
est non vide.
Cette fonction fournit un compte -rendu immediat, qui informe
l'appelant du fait qu'un travail a ou n'a pas ete effectue.
*/
{
   MCS_SDU_TO_SEND                     * Rq;
   _PROCESS_ACTIVITY_LIST                Activ    = _NOTHING_IS_DONE;
   SEGMENT_CNTXT_TYPE                  * SegCntxt = &Channel-> Segmentation; 
   TRANSMISSION_WINDOW_ELEMENT_TYPE    * CaseToFull;            

   for (;;) {

        CaseToFull= &(Channel->TransmitWindowArea->Box
               [Channel->TransmitWindowFullingIndix]);
        
        if (CaseToFull->State != _Free_Box ) break;

        /* y a -il une message en cours de segmentation, ou bien
        faut-il en depiler un nouveau, et preparer sa segmentation ? */

        if (SegCntxt->State!=_EmptyMachine);
        else {
           Rq=(MCS_SDU_TO_SEND *) GET_FIFO_INT(
                 &(Channel->RqList));
           if (Rq==0) break;
           else { /* mettre a jour le contexte de segmentation*/
               SegCntxt->State                 = _BeginningMachine;
               SegCntxt->RequestRef            = Rq;
               SegCntxt->Nr_Of_RemainingBlocks = Rq->Nr_Of_Blocks;
               SegCntxt->Nr_Of_RemainingBytes  = (Rq->Ptr_Block->User_Block).
                                                          Nr_Of_Bytes;
               SegCntxt->Ptr_Data              = (Rq->Ptr_Block->User_Block).
                                                          Ptr_Data;
               SegCntxt->Ptr_CurrentDesc       = Rq->Ptr_Block;
           }
    
        }
        ProcessMakeOneMultiframePdu(Channel);
        Activ=_WORK_IS_DONE;

   }

   return Activ;

}

#endif

/*******************************************************************/
/****** FONCTION PRIVATE ProcessMakeOneMultiframePdu() *************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void ProcessMakeOneMultiframePdu (MCSCHANNEL_REF * Channel)
{
/* 
Cette fonction est utilisee par un canal connecte d'emission,
de type Multitrame.
Elle permet la fabrication d'un PDU, dans la case pointee par
l'indice de rempissage,
La fabrication consiste en la mise a jour de la case correspondante 
dans la fenetre d'anticipation.
Il faut:
	-determiner le numero de sequence
	-mettre a jour les donnees a transmettre, sous forme de
       liste chainee de blocs, en ajoutant l'entete
	-changer l''etat' de la case qu'on vient de remplir
	-referencer, dans la case, la demande utilisateur
        -faire glisser la fenetre de remplissage

La construction des blocs de donnees de fait selon un algorithme qui
sera decrit dans le document de conception.
Le principe general est de remplir les trames au maximum de leur 
capacite en donnees, jusqu'a ce que la totalite du message soit segmente.

*/

    TRANSMISSION_WINDOW_ELEMENT_TYPE*  Box           = 
       &(Channel->TransmitWindowArea->Box[Channel->TransmitWindowFullingIndix]);

    unsigned short                     invocation_nr = 
                   Channel->CurrentInvocationNumber;


    unsigned short             FreePlace =  Channel
                            ->DLLSize-_MAX_CONNECTED_HEADER;

	 /* le premier bloc de donnees, (ie pas l'entete) a son descripteur contenu
	 dans la fenetre d'anticipation. Les descripteurs suivants seront pris dans
	 le message utilisateur, partie "Private"
     le chainage du descripteur de l'entete sur le descripteur du premier bloc
	 de donnees n'ont pas ete fait au moment du pre-remplissage des ressources
	 car ces donnees peuvent etre inexistantes => pointeur NULL
	*/

	
	 
    DATA_BLOCK_TO_TRANSFER *   NextBlock = &(Box->BlockToSend.Private);
    DATA_BLOCK_TO_TRANSFER *   Pint ; /* pointeur "intermediaire"*/
	DATA_BLOCK_TO_TRANSFER *   B_Header= &(Box->BlockToSend).User_Block;


    Box->SequenceNumber= invocation_nr;
    Box->State         = _Available_Pdu;
    Box->Stuffing      = Channel->Segmentation.RequestRef->
        Private_MCS.Stuffing;

    /* referencement de la demande utilisateur*/
    Box->RequestRef    =Channel->Segmentation.RequestRef;

    /*construction de l'entete : la taille et la nature ont deja ete 
    preremplis*/
    Box->AreaToSend.Data[2]= (unsigned char) (invocation_nr >> 8);
    Box->AreaToSend.Data[3]= (unsigned char) (invocation_nr );

	/* pour le PDU suivant*/
	Channel->CurrentInvocationNumber++; 


    /*initialiser le nombre de blocs a 1: l'unique bloc entete*/
    Box->AreaToSend.FdmToSend.Nr_Of_Blocks = 1;


    /* tester si PDU vide */
    if (Channel->Segmentation.Nr_Of_RemainingBlocks==0) {
         Box->AreaToSend.Data[1]=
         Box->Type = __HEAD_BME;
         Channel->Segmentation.State=_EmptyMachine;      
		 B_Header->Next_Block=0;
         /* faire glisser la fenetre */
         _PDUMakeProcessScroll(Channel); 
         return;     
    }



    /*segmentation PDU non vide */

	B_Header->Next_Block= (DESC_BLOCK_DATA_T * ) NextBlock;


    do {

        if (FreePlace < Channel->Segmentation.Nr_Of_RemainingBytes) {
	    NextBlock->Nr_Of_Bytes= FreePlace;
	    NextBlock->Ptr_Data=Channel->Segmentation.Ptr_Data;
	    Box->AreaToSend.FdmToSend.Nr_Of_Blocks++;

	    if(Channel->Segmentation.State==_BeginningMachine) {
                Box->AreaToSend.Data[1]=
                Box->Type = __HEAD_BEG;
		Channel->Segmentation.State=_RunningMachine;
	    }
	    else {
                Box->AreaToSend.Data[1]=
                Box->Type = __HEAD_MID;
	    }

	    Channel->Segmentation.Nr_Of_RemainingBytes-= FreePlace;
	    Channel->Segmentation.Ptr_Data+=FreePlace;
	    break;
	} else { /*Assez de place pour le bloc en cours*/
	    NextBlock->Nr_Of_Bytes=
	       Channel->Segmentation.Nr_Of_RemainingBytes;
	    Channel->Segmentation.Nr_Of_RemainingBlocks--;
	    NextBlock->Ptr_Data=Channel->Segmentation.Ptr_Data;
	    Box->AreaToSend.FdmToSend.Nr_Of_Blocks++;
	    FreePlace=(unsigned short) (FreePlace
		-Channel->Segmentation.Nr_Of_RemainingBytes);

	    if(Channel->Segmentation.Nr_Of_RemainingBlocks==0){
		if (Channel->Segmentation.State==_BeginningMachine) {
                   Box->AreaToSend.Data[1]=
                   Box->Type = __HEAD_BME;
		} else { /*En cours*/
                   Box->AreaToSend.Data[1]=
                   Box->Type = __HEAD_END;
		}
		Channel->Segmentation.State=_EmptyMachine;
		break;
	    } else { /*il reste encore des blocs a traiter...*/
		Channel->Segmentation.Ptr_CurrentDesc=
		((Channel->Segmentation.Ptr_CurrentDesc->User_Block).Next_Block);
		Channel->Segmentation.Nr_Of_RemainingBytes=
		   (Channel->Segmentation.Ptr_CurrentDesc->User_Block).
                                       Nr_Of_Bytes;
		Channel->Segmentation.Ptr_Data=
		   (Channel->Segmentation.Ptr_CurrentDesc->User_Block).Ptr_Data;

		/*test si fin PDU*/
		if(FreePlace==0) {
		    if (Channel->Segmentation.State==_BeginningMachine) {
                        Box->AreaToSend.Data[1]=
                        Box->Type = __HEAD_BEG;
			Channel->Segmentation.State=_RunningMachine;
		    }
		    else { /*en cours*/
                        Box->AreaToSend.Data[1]=
                        Box->Type = __HEAD_MID;
		    }
		    break;
		} else { /*il reste encore de la place dans la trame*/
		    Pint=NextBlock;
		    NextBlock=
		      &(Channel->Segmentation.Ptr_CurrentDesc->Private);
		    Pint->Next_Block=(DESC_BLOCK_DATA_T*)NextBlock;
		}
	    }
	}

   } while(1);

    
   /* faire glisser la fenetre */
    _PDUMakeProcessScroll(Channel);      

}
#endif


/*******************************************************************/
/****** FONCTION PRIVATE ProcessSendPdus() *************************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
_PROCESS_ACTIVITY_LIST ProcessSendPdus (MCSCHANNEL_REF * Channel)
{
/*procedure qui 
  -declenche (ou non) le transfert de MCS_PDU dans la fenetre 
    d'antipation.
  -met a jour la fenetre d'anticipation sur transfert, en fonction des 
  resultats des transferts des PDUs elementaires.
  -confirme au niveau de l'utilisateur les transferts de MCS_SDU qui sont 
  termines.

Cette procedure renvoie un indicateur qui informe si elle a
travaille (_WORK_IS_DONE) ou non (_NOTHING_IS_DONE), ie si elle
a eventuellement libere des ressources pour fabriquer des 
nouveaux MCS_PDUs.

Principe de fonctionnement: 
   (1)-on envoie tous les PDUS ayant ete mis a disposition
   par le processus de fabrication des PDUs, a condition
   qu'il soient dans le cadre de la fenetre d'anticipation.
   Dans ce cas, la fenetre d'anticipation reelle s'agrandit.

   (2)-on reduit la fenetre d'anticipation (en d'autres termes on
   la fait glisser) lorsque que le transfert du PDU le plus ancien 
   de cette fenetre est completement termine.
   Dans ce cas, (1) peut-etre relance puisque le cadre de la fenetre 
   d'anticipation s'aest deplace.
*/

   
  TRANSMISSION_WINDOW_ELEMENT_TYPE    *Box;
   _PROCESS_ACTIVITY_LIST              Activ     = _NOTHING_IS_DONE;
   int                                 LoopAgain = _MCS_NO;


   do { 

       LoopAgain = _MCS_NO;


       /* envoyer tous les PDUs a transferer */
       for (;;) {
           
           Box= &(Channel->TransmitWindowArea->Box
               [Channel->TransmitWindowEmptyingIndix]);
        
           if (Box->State != _Available_Pdu ) break;

           if ( (unsigned short ) (DISTANCE_BETWEEN(
               Channel->TransmitWindowEmptyingIndix,
               Channel->TransmitWindowBeginningIndix) ) >
              Channel->T_Anticipation - 1 )  break;


           /* on a un PDU pret a transferer, et appartenant au cadre: */
           /* on demande le transfert du PDU */
           /* on agrandit la fenetre d'anticipation de transfert*/
 
           SendNext_A_DATA_PduAndExtendTransmWindow(Channel);

           Activ=_WORK_IS_DONE;

       }

       /* faire glisser la fenetre d'anticipation lorsque les PDUs
       les plus anciens ont leur traitement termine (phase exploration):
          -le traitement d'un PDU est termine s'il est dans l'etat _Wait_End_OK
           ou _Wait_End_NOK.
          -un PDU, dont le traitement est termine, et qui est du type _HEAD_END 
          (fin de message) declenche une confirmation de service chez
          l'utilisateur, lorsqu'il sort de la fenetre d'anticipation quand
          celle-ci glisse
       */
       
       {  
           unsigned int n;
           unsigned int indix    = Channel->TransmitWindowBeginningIndix;
           unsigned int Continue =_MCS_YES;
 
            for (n=0;((n<Channel->T_Anticipation)&&(Continue==_MCS_YES));n++) {

               Box= &(Channel->TransmitWindowArea->Box[indix]);

               switch (Box->State) {
                    case _Wait_End_OK:
                    if ((Box->Type !=__HEAD_BEG)&&(Box->Type !=__HEAD_MID)){
                        Box->RequestRef->Service_Report=_MCS_OK;
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
                        Channel->Statistics[_SendDATARequestOK]++;
#endif
                        if (Box->Stuffing==_MCS_YES) {
                            Channel->IdleTransmPDUInProgress = _MCS_NO;
                        } else {

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
                            MCS_BEGIN_CRITICAL(Channel->Mcs);
                            Channel->NumberOfInRequests--;
                            MCS_END_CRITICAL(Channel->Mcs);
#endif
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
                           Channel->Statistics[ _ConfirmDataSDUTransmission ]++;
#endif
                            Channel->User_Msg_Ack (Channel->UserChannelRef,
                                                   Box->RequestRef);  
                        }
                    }
                    _TransmWindowScroll(Box, Channel) ;
                    Activ=_WORK_IS_DONE;
 
                    break;
  

                    case _Wait_End_NOK:
                    if ((Box->Type !=__HEAD_BEG)&&(Box->Type !=__HEAD_MID)){
                        Box->RequestRef->Service_Report=_MCS_NOK;
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
                        Channel->Statistics[_SendDATARequestNOK]++;
#endif
                        if (Box->Stuffing==_MCS_YES) {
                            Channel->IdleTransmPDUInProgress = _MCS_NO;
                        } else {
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )

                            MCS_BEGIN_CRITICAL(Channel->Mcs);
                            Channel->NumberOfInRequests--;
                            MCS_END_CRITICAL(Channel->Mcs);
#endif               
             
                            /*signaler a ACSE que defaut transmission sur SDU*/
                            Intern_Signal_ConnexionFailure(
                                Channel->Mcs,
                                Channel,
                                _CONNEXION_FAILURE_SDUTransferError);

                            Channel->User_Msg_Ack (Channel->UserChannelRef,Box->RequestRef);  
                      }
                    }
                    _TransmWindowScroll(Box, Channel) ;
                    Activ=_WORK_IS_DONE;
 
                    break;
          
                    default:
                    Continue =_MCS_NO;
                    break;

               }

             indix=(indix+1)&(_PDUMakeAnticipation(Channel)-1);
          }
       
      }
       
      
   } while (LoopAgain==_MCS_YES);

   return Activ;
   
}

#endif

/*******************************************************************/
/*** FONCTION PRIVATE SendNext_A_DATA_PduAndExtendTransmWindow() ***/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void SendNext_A_DATA_PduAndExtendTransmWindow(MCSCHANNEL_REF * Channel)
{
/* procedure qui engage l'emission du PDU de plus grand
numero dans la fenetre d'anticipation de transfert, et procede a
l'extension de cette fenetre d'une case.

*/


    TRANSMISSION_WINDOW_ELEMENT_TYPE*  Box           = 
             &(Channel->TransmitWindowArea->Box[Channel
                ->TransmitWindowEmptyingIndix]);

    if (Box->PurgeState==_Cancelled) {
      /*on n'engage pas les transferts de PDU appartenant a un
      SDU dont le transfert d'un PDU au moins a echoue*/
        Box->State=_Wait_End_NOK;
    } else {
       unsigned short Cr;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
       Channel->Statistics[ _DataPDUTranmission ]++;
#endif
       Box->SendCredit = Channel->T_NbOfRetry; 
       /*nombre max d'emissions possible -1 
          car on en commence un immediatement*/

       /*demande de transfert a FDM */
  
       Cr=fdm_send_message(Channel->Lsap_Ctxt, 
              &(Box->AreaToSend.FdmToSend));

       if (Cr != 0) {
         /* signifie que le contexte de messagerie FDM n'existe pas (plus),
            ce qui est impossible */

	    Channel->Mcs->Extern_Signal_Error (  
					_MCS_TYPE_FDM_MSG_TO_SEND,
                   (void *)(&(Box->AreaToSend.FdmToSend)),
                   _MCS_INTERNAL_ERROR ) ;

                       /* ERREUR FATALE !!! */
 
       } else {

#if (MCS_WITH_COMMAND_CHANNELS== _MCS_OPTION_YES )
            Channel->NumberOfOutRequests ++;
#endif

            Box->State=_Wait_For_ConfAndAck;	
       }			   

    }

    /* temporisations */
    if ( Channel->StuffingMode == _MCS_WITH_STUFFING_MODE ) {
        RestartStuffingTempo(Channel);
        RestartTransmWatchdogTempo(Channel);
    }


    /*extension de la fenetre d'anticipation*/
    Channel->TransmitWindowEmptyingIndix = 
      (Channel->TransmitWindowEmptyingIndix + 1)
            & (_PDUMakeAnticipation(Channel)-1);
      
    Box  =   &(Channel->TransmitWindowArea->Box[Channel
                ->TransmitWindowEmptyingIndix]);


    /*Mise a jour de l'etat de transfert du prochain PDU a transmettre, en fonction
      de l'etat de transfert du SDU  auquel il appartient.
      Reset de la case "etat de transfert du SDU courant", dans le cas ou le PDU 
      est le dernier du SDU */ 
    if ((Box->State==_Available_Pdu)&&(Channel->SDUPurgeState ==_Cancelled)) {
	 Box->PurgeState=_Cancelled;
	 if(Box->Type==__HEAD_END) {
           Channel->SDUPurgeState = _Transmitting;
	  	 }
    } else if ((Box->State==_Available_Pdu)||(Box->State==_Free_Box)){
	 Box->PurgeState=_Transmitting;
      
    }
}

#endif


/*******************************************************************/
/*** FONCTION PRIVATE ExecCnfDLLFor_A_DATA_DataPDU() ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecCnfDLLFor_A_DATA_DataPDU(
      MCSCHANNEL_REF     * Channel, 
      MCS_FDM_MSG_TO_SEND * Message)
{
/*cette fonction est appelee apres le depilement d'une confirmation
de transfert DLL, concernant le transfert d'un PDU vehiculant des 
donnees MCS.
Le traitement consiste a:
  -decrementer le compteur des ressources sorties
  -consulter le resultat de transfert DLL et mettre a jour les compteurs de
  statistiques associes.
  -changer l'etat du PDU en cours de transfert
  -dans le cas ou l'acquittement de ce PDU n'a pas deja ete recu,
  lancer la temporisation. 
 -relancer la machine de traitement A_DATA
*/

   
       TRANSMISSION_WINDOW_ELEMENT_TYPE*  Box          
          = (TRANSMISSION_WINDOW_ELEMENT_TYPE*)(Message->Ctxt);

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
        Channel->NumberOfOutRequests--;
#endif

       /*maj statisticques DLL */
       GiveSendDLLResultAndRefreshStatistics(
             Channel,
             &(Message->FdmToSend) );


       switch (Box->State) {

           case _Wait_For_ConfAndAck:

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            /* compteur-> "on a recu la conf DLL de l'emission d'un PDU"  */
            Channel->Statistics[ _CnfDLLReceivedForDataPDU ]++;
#endif        

            if (Box->PurgeState == _Cancelled) {
               Box->State=_Wait_End_NOK;
               break;
            }

            Box->State=_Wait_For_AckAndTimeOut;
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
            Channel->NumberOfOutRequests ++ ; 
#endif
            StartPDU_A_DATA_Tempo(Box);
            break;

            case _Wait_For_Cnf:
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            /* compteur-> "on a recu la conf DLL de l'emission
             d'un PDU, alors que l'acquittement avait deja ete recu"  */
            Channel->Statistics[ _CnfDLLReceivedForDataPDUAfterAckReceived ]++;
#endif        
             Box->State=_Wait_End_OK;
             break;


             default:
             /* impossible ! */
             break;
         
 
       }
     
        LoopTransmissionMCSConnectedServices(Channel);

}
#endif  

/*******************************************************************/
/*** FONCTION PRIVATE ExecTimeOutFor_A_DATA_AckPDU() ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecTimeOutFor_A_DATA_AckPDU(
      MCSCHANNEL_REF *     Channel, 
      MCS_TEMPO_TYPE *      Message)

{

/*cette fonction est appelee apres le depilement d'un time out,
concernant la non reception de l'acquittement suite au transfert 
d'un PDU vehiculant des donnees MCS, a travers un canal de type 
A_DATA Emission.


Le traitement consiste a:
  -decrementer le compteur des ressources sorties
  -mettre a jour les compteurs de statistiques en fonction des cas
et traiter l'evenement:

Si l'acquittement n'a effectivement jamais ete recu:
  -si le nombre max de reprises n'est pas atteint, relancer un transfert.
  -sinon, changer l'etat du PDU en cours de transfert (_FIN_NOK), et declencher 
  l'annulation du transfert du SDU dont ce PDU fait partie

Si l'acquittement a deja ete recu:
  -changer l'etat de la demande -> _FIN_OK

Dans tous les cas, relancer la machine de traitement A_DATA
*/


   
    TRANSMISSION_WINDOW_ELEMENT_TYPE*  Box          
          =(( MCS_A_DATA_T_TEMPO_TYPE*)Message)->BoxRef;

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
    Channel->NumberOfOutRequests--;
#endif

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     /* compteur-> "on a recu un time-out de non reception de l'ack
      sur le transfert d'un PDU"  */
     Channel->Statistics[ _TimeOutReceivedForDataPDU ]++;
#endif        
    
    switch (Box->State){
  
         case _Wait_For_AckAndTimeOut:
  
         if ( (Box->SendCredit != 0 )             &&
              (Box->PurgeState != _Cancelled )    ){

              unsigned short Cr;

              Box->SendCredit --; 

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
             /* compteur-> "on effectue une repetition sur le transfert d'un PDU"  */
            Channel->Statistics[ _DataPDUNewTransfer ]++;
#endif        

            /*demande de transfert a FDM */
  
             Cr=fdm_send_message(Channel->Lsap_Ctxt, 
              &(Box->AreaToSend.FdmToSend));

             if (Cr != 0) {
                /* signifie que le contexte de messagerie FDM 
                   n'existe pas (plus), ce qui est impossible */

	        Channel->Mcs->Extern_Signal_Error (  
				_MCS_TYPE_FDM_MSG_TO_SEND,
                   (void *)(&(Box->AreaToSend.FdmToSend)),
                   _MCS_INTERNAL_ERROR ) ;

                       /* ERREUR FATALE !!! */

             } else {
#if (MCS_WITH_COMMAND_CHANNELS== _MCS_OPTION_YES )
            Channel->NumberOfOutRequests ++;
#endif

                 Box->State=_Wait_For_ConfAndAck;	
             }			   

         } else {
            Box->State = _Wait_End_NOK;
            CancelSDUTransfer(Box, Channel);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
             /* compteur-> "le transfert d'un PDU a echoue, 
                malgre les repetitions"  */
            Channel->Statistics[ _DataPDUTransferFailed ]++;
#endif        

            Intern_Signal_ConnexionFailure(
                   Channel->Mcs,
                   Channel,
                   _CONNEXION_FAILURE_PDUTransferError);
         }
         break;


         case _Wait_For_TimeOut:
         Box->State = _Wait_End_OK;
         break;

         default:
         /*impossible !! */
         break;

    }
    

    LoopTransmissionMCSConnectedServices(Channel);

}
#endif


/*******************************************************************/
/*** FONCTION PRIVATE ExecIndDLLFor_A_DATA_DataPDU() ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecIndDLL_A_DATA_AckPDU(
      MCSCHANNEL_REF     * Channel, 
      FDM_MSG_RECEIVED     * Message )
{
/*cette fonction est appelee apres le depilement puis le tri des indications
de la couche liaison de donnees.
Cette indication correspond a l'acquittement d'un PDU MCS, correspondant
a un transfert de donnees a travers un canal de type A_DATA Emission

Le traitement consiste a:

  -verifier que l'acquittement correspond bien a un PDU de la fenetre
  d'anticipation.
  -si oui: mettre a jour l'etat du PDU en cours de transfert
  -si non, ne rien faire
  -liberer la ressource laison de donnees
  -decrementer le compteur des ressources 'in'
  -relancer la machine de traitement A_DATA
*/

    unsigned short CurrentSeqNumber = 
               (Message->Ptr_Block->Ptr_Data[2]<<8) |
               (Message->Ptr_Block->Ptr_Data[3]); 

    unsigned short BeginSeqNumber=
       (Channel->TransmitWindowArea->Box[Channel
                ->TransmitWindowBeginningIndix]).SequenceNumber;

    unsigned short LastElementIndix=
       ((Channel->TransmitWindowEmptyingIndix)-1)&
          (_PDUMakeAnticipation(Channel)-1);

    unsigned short EndSeqNumber=
       Channel->TransmitWindowArea->
          Box[LastElementIndix].SequenceNumber;

    unsigned int Inside= _MCS_YES;


    if (DISTANCE_BETWEEN (CurrentSeqNumber, BeginSeqNumber)
      >=  Channel->T_Anticipation) {
       Inside=_MCS_NO;
    } 

    if (DISTANCE_BETWEEN (EndSeqNumber, CurrentSeqNumber)
      >=  Channel->T_Anticipation) {
       Inside=_MCS_NO;
    } 
   
    if (Inside==_MCS_YES) {

        unsigned int CurrentIndix=CurrentSeqNumber &
              (_PDUMakeAnticipation(Channel)-1);
  
        TRANSMISSION_WINDOW_ELEMENT_TYPE*  Box =      
          &(Channel->TransmitWindowArea->Box[CurrentIndix]);

        switch(Box->State) {
           case _Wait_For_ConfAndAck :
           Box->State=_Wait_For_Cnf;
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           /* compteur-> "on a recu un acquittement de PDU, 
               et avant la confirmation de transfert liaison"  */
            Channel->Statistics[ _DataPDUAckReceivedBeforeCnfDLLReceived ]++;
#endif        
           break;

           case _Wait_For_AckAndTimeOut :
           Box->State=_Wait_For_TimeOut;
           StopPDU_A_DATA_Tempo(Box);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           /* compteur-> "on a recu un acquittement de PDU */ 
            Channel->Statistics[ _DataPDUAckReceived ]++;
#endif        
           break;

           default:
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           /* compteur-> "on a recu un acquittement de PDU, alors que
                 cet acquittement a deja ete recu"  */
            Channel->Statistics[ _MultipleDataPDUAckReceived ]++;
#endif
           break;

        }

   } else  {
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
           /* compteur-> "on a recu un acquittement de PDU, 
               en dehors de la fenetre d'anticipation"  */
           Channel->Statistics[ _DataPDUAckReceivedForUnknownPDU ]++;
#endif        

   }


#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
   MCS_BEGIN_CRITICAL(Channel->Mcs);
   Channel->NumberOfInRequests -=2;
   MCS_END_CRITICAL(Channel->Mcs);
#endif  

	{	/* liberer dans le bon ordre */ 
		FDM_MSG_R_DESC * BlocData = Message->Ptr_Block;

        fdm_msg_ref_buffer_free(Message);
        fdm_msg_data_buffer_free(BlocData);
   }

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 


    LoopTransmissionMCSConnectedServices(Channel);
   /*Note: il n'y a rien a faire si PDU n'appartient pas*/
   

}
#endif

/*******************************************************************/
/*** FONCTION PRIVATE ExecIndDLLFor_A_DATA_DataPDU() ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void  CancelSDUTransfer(
    TRANSMISSION_WINDOW_ELEMENT_TYPE * Box, 
    MCSCHANNEL_REF                  * Channel)
{
/*procedure qui est appelee lorsque, dans un canal de type connecte emission,
apres n repetitions de l'emission d'un MCS PDU contenant des donnees,
on n'a toujours pas recu d'acquittement et le timeout correspondant sonne.
Elle permet d'engager l'annulation du transfert du SDU auquel appartient le PDU 
dont le transfert a echoue.
Le traitement consiste a parcourir la fenetre d'ancticipation pour y retrouver les
PDUs appartenant au SDU  dont 'Box' contient le PDU dont le transfert a echoue, et de 
positionner l'etat de purge a 'canceled', ce qui aura pour effet de ne plus engager 
la transmission de ces PDUs.

Dans le cas ou l'etat de la case 'Box' est 'canceled', c'est que ce travail a deja ete
fait auparavant, et qu'il n'y a plus rien a faire.

Si le dernier PDU di SDU n'appartient pas a la fenetre de transfert, on positionne
l'indicateur de purge globale du SDU a 'canceled' de maniere a provoquer l'annulation
des transferts des PDUs futurs, qui rentreront dans la fenetre d'anticiation apres 
extension de la fenetre  d'anticiation.
*/


    unsigned char Type =Box->Type ;


    if (Channel->SDUType==_CHANNEL_FOR_MONOFRAME_SDU) {
	return;
    } else {
	if (Box->PurgeState==_Cancelled) {
	    return;
	}
	switch (Type) {
	    case __HEAD_BME:
	    case __HEAD_END:
	    return;

	    case __HEAD_BEG:
	    case __HEAD_MID: {
	    /*recherche aval, vers sequences posterieures*/
		unsigned short NumLast, NumCurr;
		int            i, i_last;
		int            NrOfBoxes;
		int            EndFound=_MCS_NO;

		i_last=_PreviousInTransmWindow(
                    Channel->TransmitWindowEmptyingIndix, 
                    _PDUMakeAnticipation(Channel));

		NumLast= (Channel->TransmitWindowArea->Box[i_last]).
                       SequenceNumber;

		NumCurr   = Box->SequenceNumber;
		NrOfBoxes = NumLast-NumCurr;

		i         = (NumCurr+1)&(_PDUMakeAnticipation(Channel)-1);

		while (NrOfBoxes!=0) {
		    TRANSMISSION_WINDOW_ELEMENT_TYPE * P_Box =
                         &(Channel->TransmitWindowArea->Box[i]);

		    P_Box->PurgeState  = _Cancelled;
		    P_Box->SendCredit  = 0;

		    if (P_Box->Type==__HEAD_BEG){
			EndFound=_MCS_YES;
			break;
		    }
		    i=(i+1)&(_PDUMakeAnticipation(Channel)-1);
		    NrOfBoxes--;
		}
		if (EndFound==_MCS_NO) {
                    Channel->SDUPurgeState=_Cancelled;
		} else { }

	    }
	    return;
	}
    }

}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE FileOut_TimeOutElement() ******************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void FileOut_TimeOutElement(void * Tempo)
/* cette fonction est appelee par le sequenceur, quand celui-ci vide
la file des temporisations echues, que le logiciel avait engagees au cours
du fonctionnement des canaux.

Ces demandes concernent les temporisations suivantes:

    -temporisation sur la non reception de l'acquittement des PDUs, suite
    a l'emission de ces PDUs, au niveau de la couche liaison de donnees.
    -temporisation pour l'envoi du bourrage a travers un canal de type
    A_DATA emission, sur lequel on a demande le bourrage.

Cette fonction distingue  ces 2 type de temporisations, et les aiguille vers
les fonctions de traitement associees.

De plus, le champs "In_File" du descripteur de temporisation est positionne 
a 0, pour indiquer au logiciel gestionnaire de temporisation que ce descripteur
n'est plus dans la file d'interconnexion.

*/
{
    TEMPO_DEF      * T0 = (TEMPO_DEF *) Tempo;
    MCS_TEMPO_TYPE * T1 = (MCS_TEMPO_TYPE *) Tempo;

  
    T0->_.In_File=0;

    switch (T1->Type) {

        case _MCS_TEMPO_FOR_CONNEXION_PDU_ACK_TIMEOUT:
        ExecTimeOutFor_A_DATA_AckPDU( T1->Channel, T1);
        break;

        case _MCS_TEMPO_FOR_STUFFING:
        ExecTimeOutFor_A_DATA_Stuffing( T1->Channel, T1);
        break;

        default:  /*impossible*/
        break;            
    }
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
     ExecuteDeleteIfRequestedAndPossible(T1->Channel);
#endif
}

#endif






/*******************************************************************/
/****** FONCTION PRIVATE  FileOut_FreeBufferElement() **************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void FileOut_FreeBufferElement(void * SDU)
{
/* cette fonction est appelee par le sequenceur, quand celui-ci vide
la file des SDU a recycler.
Cette file peut contenir des SDU ACSE a recycler.

On appelle la procedure de traitement adequate.
*/

 

   Exec_FreeDataSDURessource(
       (MCS_SDU_RECEIVED *)SDU, 
       ((MCS_SDU_RECEIVED *)SDU)->Channel.McsChannelRef);

   /* relance de la machine de reception, au cas ou il lui manquait des 
      ressources pour indiquer a l'utilisateur des SDU recus*/
   
   LoopReceptionMCSConnectedServices(
        ((MCS_SDU_RECEIVED *)SDU)->Channel.McsChannelRef);

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
       ExecuteDeleteIfRequestedAndPossible(((MCS_SDU_RECEIVED *)SDU)->Channel.McsChannelRef);
#endif


}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE ExecTimeOutFor_A_DATA_Stuffing() **********/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void ExecTimeOutFor_A_DATA_Stuffing(
      MCSCHANNEL_REF *     Channel, 
      MCS_TEMPO_TYPE *      Message)
{
/*Cette fonction est appelee a la suite du depilement d'une
temporisation servant a gerer l'emission du bouurage
dans un canal de type A_DATA Emission.
Traitement:
   -si aucun PDU de bourrage n'est, actuellement en cours de transmission, 
    envoyer une trame de bouurage dans le canal.
   -si non, ne rien faire.

Dans tous les cas, mettre a jour les compteurs de statistiques
appropries.

Dans le cas ou on emet le bourrage, on ne relance pas tout de
suite la temporisatiion, qui sera relancee au moment du transfert liaison 
au niveau liaison de donnees.
Par contre, dans le cas ou on n'emet pas le bourrage, on relance la 
temporisation.
*/

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
       Channel->NumberOfOutRequests -- ; 
#endif
  
  
    if (Channel->IdleTransmPDUInProgress == _MCS_YES) {
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
        /* compteur-> "on doit renvoyer un ID de bouurage maintenant, mais la 
           confirmation dll du bourrage d'avant n'est toujours pas arrivee." 
            = underflow sur transm. PDU de bourrage */
            Channel->Statistics[ _Underrunstuffing ]++;
#endif

    } else {
        Channel->IdleTransmPDUInProgress=_MCS_YES; 
        STORE_FIFO_INT( 
           &(Channel->RqList)         ,
           (FIFO_INT_ELT_TYPE *)(&(Channel->StuffingSDUToSend))) ;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
        /* compteur-> "une demande de transmission de bourrage est effectuee" */
            Channel->Statistics[ _StuffingProduced ]++;
#endif
        LoopTransmissionMCSConnectedServices(Channel);
    }

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
    Channel->NumberOfOutRequests ++ ; 
#endif

     StartStuffingTempo(Channel);
}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE InternSign_TransmWatchdogTempo() **********/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void InternSign_TransmWatchdogTempo(TEMPO_DEF * Tempo)
{
/* cette fonction est appelee directement par le gestionnaire de tempo,
lorsque la temporisation qui surveille le flux d'emission des PDUs 
a echu.
Dans ce cas, on avertit ACSE du dysfonctionnement du canal, et on relance 
la temporisation.

*/
     MCS_TEMPO_TYPE * T = (MCS_TEMPO_TYPE *) Tempo;

     Intern_Signal_ConnexionFailure (
           T->Channel->Mcs, 
           T->Channel, 
            _CONNEXION_FAILURE_NoPDUTransmission);
          
     StartTransmWatchdogTempo ( T->Channel);
}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE InternSign_TransmWatchdogTempo() **********/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void InternSign_RecepWatchdogTempo(TEMPO_DEF * Tempo)
{
/* cette fonction est appelee directement par le gestionnaire de tempo,
lorsque la temporisation qui surveille le flux de reception des PDUs 
a echu.
Dans ce cas, on avertit ACSE du dysfonctionnement du canal, et on relance 
la temporisation.

*/
     MCS_TEMPO_TYPE * T = (MCS_TEMPO_TYPE *) Tempo;

     Intern_Signal_ConnexionFailure (
            T->Channel->Mcs, 
            T->Channel, 
            _CONNEXION_FAILURE_NoPDUReception);
          
     StartRecepWatchdogTempo ( T->Channel);


}

#endif

/*******************************************************************/
/****** FONCTION PRIVATE FileOut_ReceiveDLLIndElement() ************/
/*******************************************************************/
void FileOut_ReceiveDLLIndElement(void * Element)
{
/*Cette fonction est appelee par le sequenceur, apres depilement
d'une indication de reception de message venant du reseau.
Ces messages peuvent etre de differents types:
  -PDU de type message pour canal UNIDATA (reception)
  -PDU de type message pour canal A_DATA (reception)
  -PDU de type acquittement pour canal A_DATA (emission)
  -PDU de type A_RELEASE_RQ pour canal A_DATA (emission/reception)
  -PDU de type A_RELEASE_IND pour canal A_DATA (emission/reception)
  -PDU de type A_ABORT pour canal A_DATA (emission/reception)

Traitement: 
Cette fonction regarde le contenu du message recu, 
et en deduit le type de PDU recu. Elle verifie la coherence
entre le type de PDU recu, et les caracteristiques du canal
auquel ce PDU est adresse.
Si coherent: aiguille le PDU vers la fonction de traitement
Si non coherent, ou si le PDU est illisible, genere un warning,
et rejete le message

Note: lorsque cette fonction est appelee, le canal est necessairement
dans l'etat "RUNNING" car la file des indications DLL recues est purgee
des messages a destination d'un canal en cours de fermeture, et aucun message 
pour un canal dans l'etat IDLE ne rentre dans cette file
*/


    FDM_MSG_RECEIVED            * Message= (FDM_MSG_RECEIVED *) Element;
    FDM_MSG_R_DESC              * Desc = Message-> Ptr_Block;
    unsigned char               * Data = Desc->Ptr_Data;

    MCSCHANNEL_REF             * Channel = (MCSCHANNEL_REF *) Message->User_Qid;             
              
    unsigned char                 Emit, Recep;
    unsigned char                 Multiframe;
    unsigned char                 Connected;

    unsigned char                 HeaderLength;
    unsigned char                 PDUType;
    unsigned char                 AcseParam;

    if (( Channel->CommType == _CHANNEL_FOR_TRANSM_AND_RECEP)||
        ( Channel->CommType == _CHANNEL_FOR_TRANSM) )
                       Emit=_MCS_YES;
    else               Emit=_MCS_NO;

    if (( Channel->CommType == _CHANNEL_FOR_TRANSM_AND_RECEP)||
        ( Channel->CommType == _CHANNEL_FOR_RECEP) )
                       Recep=_MCS_YES;
    else               Recep=_MCS_NO;

    if ( Channel->SDUType == _CHANNEL_FOR_MULTIFRAME_SDU) Multiframe=_MCS_YES;
    else                                                  Multiframe =_MCS_NO;

    if ( Channel->ServType == _CHANNEL_FOR_CONNECTED_SERVICES) Connected=_MCS_YES;
    else                                                       Connected =_MCS_NO;


    /* controler que le message recu de la DLL a bonne allure */
    /* sinon, on considere que erreur grave, et on ne libere pas
    car liberer quoi exactement ? */

    if ( (Desc==NULL)||(Data==NULL)||(Desc->Nr_Of_Bytes<1)) {
        Channel->Mcs->Extern_Signal_Error (
			_MCS_TYPE_FDM_MSG_RECEIVED,
             Element, _MCS_RECEIVE_DLL_BROKEN_MESSAGE );
        return;      
    }




    /* dans le cas connecte, on n'a pas besoin de verifier 
    que le source correspond bien au source defini pour ce canal, car ce
    controle est fait par FDM, a partir du moment ou on a cree,
    pour ce canal MCS, un contexte de messagerie avec source et destination
    parfaitement definis*/

    HeaderLength  = Data[0];
    PDUType       = Data[1];
    AcseParam     = Data[2];
    

   
    /*---------------------- PDU == 1X ou 3X ------------------*/
    /*------------------- Msg A_UNIDATA NOACK ou ACK ----------*/
    if  (((PDUType >> 4) == (unsigned char)1 ) ||
         ((PDUType >> 4) == (unsigned char)3 ) ){
    
	if( (HeaderLength         != 3)				    ||
	    (Connected            ==_MCS_YES)                           ||
	    (Desc->Nr_Of_Bytes    <= (unsigned int)(HeaderLength+1))||
	    (Recep                ==_MCS_NO)                            ||
	    (Desc->Nr_Of_Bytes    > Channel->DLLSize )		    ){

             Channel->Mcs->Extern_Signal_Warning (
						_MCS_TYPE_FDM_MSG_RECEIVED,
                        Element, 
                        _MCS_RECEIVE_DLL_INVALID_PDU );
	     goto ERREUR;
        } else {
	    
             ExecIndDLL_A_UNIDATA_DataPDU(Channel, Message);
	     return;
        }
    }
    

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
    
    /*---------------------- PDU == 20	-----------------------*/
    /*------------------- Msg A_DATA_ACK_MONO -----------------*/
    else if( PDUType == __HEAD_NIL ){

	if( (HeaderLength        != 3)				    ||
	    (Connected           ==_MCS_NO)	                            ||
	    (Multiframe          ==_MCS_YES)                            ||
	    (Recep               ==_MCS_NO)	                            ||
	    (Desc->Nr_Of_Bytes   <= (unsigned int)(HeaderLength+1)) ||
	    (Desc->Nr_Of_Bytes   > Channel->DLLSize )               ){

             Channel->Mcs->Extern_Signal_Warning (
						_MCS_TYPE_FDM_MSG_RECEIVED,
                        Element, 
                        _MCS_RECEIVE_DLL_INVALID_PDU );
	     goto ERREUR;
        } else {

             ExecIndDLL_A_DATA_DataPDU(Channel,Message);
	     return;
        }
    }
#endif


#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

    /*---------------------- PDU == 24,25,26,27----------------*/
    /*------------------- Msg A_DATA_ACK_MULTI-----------------*/
    else if( (PDUType == __HEAD_BME) ||
	     (PDUType == __HEAD_BEG) ||
	     (PDUType == __HEAD_MID) ||
	     (PDUType == __HEAD_END) ){

	if( (HeaderLength        != 3)				    ||
	    (Connected           ==_MCS_NO)	                            ||
	    (Multiframe          ==_MCS_NO)                             ||
	    (Recep               ==_MCS_NO)	                            ||
	    (Desc->Nr_Of_Bytes   <= (unsigned int)(HeaderLength+1)) ||
	    (Desc->Nr_Of_Bytes   > Channel->DLLSize )               ){

             Channel->Mcs->Extern_Signal_Warning (
						_MCS_TYPE_FDM_MSG_RECEIVED,
                        Element, 
                        _MCS_RECEIVE_DLL_INVALID_PDU );
	     goto ERREUR;
        } else {

             ExecIndDLL_A_DATA_DataPDU(Channel, Message);
	     return;
        }
    }
#endif

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

    /*---------------------- PDU == 60	-----------------------*/
    /*--------------------- Acquit MCS	ASSOCIE----------------*/
    if(PDUType == __HEAD_ACK_DATA){

	if( (HeaderLength        != 3)				    ||
	    (Connected           ==_MCS_NO)	                            ||
	    (Emit                ==_MCS_NO)	                            ||
	    (Desc->Nr_Of_Bytes   != (unsigned int)4)                ){

             Channel->Mcs->Extern_Signal_Warning (
						_MCS_TYPE_FDM_MSG_RECEIVED,
                        Element, 
                        _MCS_RECEIVE_DLL_INVALID_PDU );
	     goto ERREUR;
        } else {

             ExecIndDLL_A_DATA_AckPDU(Channel, Message);
	     return;
        }
    }

#endif



    /*les autres PDU sont refuses */
    Channel->Mcs->Extern_Signal_Warning (
			_MCS_TYPE_FDM_MSG_RECEIVED,
           Element, 
           _MCS_RECEIVE_DLL_INVALID_PDU );

    ERREUR :

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            /*compteur -> reception d'un PDU invalide, tout court ou par
            rapport aux caracteristiques du canal */
            Channel->Statistics[ _InvalidPDUReceived ]++;
#endif


#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
        if (Connected    == _MCS_YES )	{                                   
            MCS_BEGIN_CRITICAL(Channel->Mcs);
            Channel->NumberOfInRequests-=2;
            MCS_END_CRITICAL(Channel->Mcs);
       }
#endif
		/* attention a l'ordre dans les liberationd: */
        fdm_msg_ref_buffer_free(Message);
        fdm_msg_data_buffer_free(Desc);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 



}

/*******************************************************************/
/****** FONCTION PRIVATE ExecIndDLL_A_UNIDATA_DataPDU() ************/
/*******************************************************************/
void ExecIndDLL_A_UNIDATA_DataPDU(
      MCSCHANNEL_REF     * Channel, 
      FDM_MSG_RECEIVED    * Message )


/*fonction pour le traitement de la reception d'une trame
de PDU de donnees, dans le cas d'un canal de 
type A_UNIDATA Reception.
Ce PDU peut demander un ack ou non.
On ne traite jamais l'acquittement.

Actions:
  -mettre a jour les compteurs de statistiques 
  -allouer un descripteur d'indication
  -le remplir et indiquer au niveau USER

*/

{

    MCS_SDU_RECEIVED * DescRecep;
    FDM_MSG_R_DESC   * Desc        = Message->Ptr_Block;
    unsigned char    * Data        = Desc->Ptr_Data;

  
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
    /* compteur-> "on a recu un PDU de donnees correct" */
    Channel->Statistics[ _ValidPDUReceived ]++;
#endif

    DescRecep= (MCS_SDU_RECEIVED *)
       _ALLOCATE_MEMORY  (Channel->PoolForUserIndicationRessources);

    if (DescRecep == NULL ) {

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
          /* compteur-> "manque de memoire pour descripteur de SDU " */
          Channel->Statistics[ _LackOfMemoryToIndicate_DataSDU ]++;
#endif

          /* recycler le messager recu */
          fdm_msg_ref_buffer_free(Message);
          fdm_msg_data_buffer_free(Desc);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 

          return;

    } 

    /*mise a jour du descripteur d'indication */
    /*les informations statiques etant peu nombreuses, 
     on les positionne la plutot qu'
     au moment de la configuration du canal*/ 
          
    DescRecep->Channel.UserChannelRef   = Channel->UserChannelRef;
    /*on se fiche de Acse_Ref */
    DescRecep->Channel.McsChannelRef    = Channel;
    DescRecep->Nr_Of_Blocks        = 1;

 
    DescRecep->Source              = Message->Remote_DLL_Address & 0xFFFFFF;
    /* DescRecep->SDU_Nr  non utilise */
    DescRecep->Ptr_Block           = (DESC_BLOCK_DATA_R * ) Desc;

    /*decaler le pointeur de donnees, pour depasser l'entete */
    Desc->Nr_Of_Bytes  -=4;
    Desc->Ptr_Data     +=4;
      
	/* calcul taille totale du message - 28.12.99*/
	DescRecep->SDU_Size =Desc->Nr_Of_Bytes;

    /* recyclage du descripteur DLL */
    fdm_msg_ref_buffer_free(Message);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
#endif 

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
    /* compteur-> "on a fourni a l'utilisateur un SDU" */
    /*indication a l'utilisateur*/
    Channel->Statistics[ _DataSDUIndicated ]++;
#endif

    Channel->User_Msg_Rec_Proc( 
           Channel->UserChannelRef, 
           DescRecep);
           
    
}



/*******************************************************************/
/****** FONCTION PRIVATE FindPositionInRecepWindow() ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

_POSITION_IN_RECEP_WINDOW_LIST FindPositionInRecepWindow (
         unsigned short     NumSequence, 
         MCSCHANNEL_REF *  Channel )

{
/*
Cette fonction permet de trouver position relative d'un PDU,
defini par un numero de sequence, par rapport a la fenetre de reception
a l'instant T
*/


    unsigned short     A = Channel->R_Anticipation;
    unsigned short     numHead, numQueue;

    RECEPTION_WINDOW_ELEMENT_TYPE   * CaseToFull = 
         &(Channel->RecepWindowArea->Box
               [Channel->RecepWindowFullingIndix]);

    numHead = CaseToFull->SequenceNumber;
    numQueue= (unsigned short)(numHead-A);

    if (DISTANCE_BETWEEN(numHead,NumSequence) <A)
	      return _Inside_Window_Position;

    if (DISTANCE_BETWEEN(numQueue,NumSequence) <A)
	      return _Upper_Window_Position; /*dans extension*/

    if (DISTANCE_BETWEEN(NumSequence,numHead) <A)
	      return _Under_Window_Position;

    return _FarFrom_Window_Position;


}

#endif

/*******************************************************************/
/****** FONCTION PRIVATE ExecCnfDLLFor_A_DATA_AckPDU() *************/
/*******************************************************************/

/* fonction pour le traitement d'une confirmation liaison d'un transfert
de PDU de type 'acquittement de donnees' a travers un canal A_DATA */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecCnfDLLFor_A_DATA_AckPDU(
      MCSCHANNEL_REF *     Channel, 
      MCS_FDM_MSG_TO_SEND * Message)
{

/* fonction pour le traitement d'une confirmation liaison d'un transfert
de PDU de type 'acquittement de donnees' a travers un canal A_DATA.
On ne se preocupe pas du resultat de transfert: on se contente de 
"ranger" la ressource utilisee pour emettre l'acquittement dans la file
correspondante
*/


#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _CnfDLLReceivedForAckPDU ]++;
#endif 
 
    GiveSendDLLResultAndRefreshStatistics(
         Channel,     
         &(Message->FdmToSend));

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
    Channel->NumberOfOutRequests --;
#endif

    STORE_FIFO_INT(
         &(Channel->AckRessourcesList),
        (FIFO_INT_ELT_TYPE * ) Message) ;       



}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE ExecIndDLL_A_DATA_DataPDU() ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void ExecIndDLL_A_DATA_DataPDU(
      MCSCHANNEL_REF     * Channel, 
      FDM_MSG_RECEIVED    * Message )


/*fonction pour le traitement de la reception d'une trame
de PDU de donnees, dans le cas d'un canal de 
type A_DATA Reception.

Actions:
   - mettre a jour les compteurs de statistiques
   - dans le cas ou le mode bourrage est choisi, relancer 
     la temporisation de surveillance du flux en reception
   - si le PDU est dans la fenetre, le memoriser dans cette fenetre,
     en gerant l'antiduplication, et envoyer l'acquittement.
   - dans le cas contraire:
        . provoquer un petit glissement de fenetre si le PDU est un PDU
          nouveau, juste au dessus de la fenetre (cas normal)
        . provoquer un grand glissement (ie une resync) lorsque le PDU
          recu est completement en dehors de la fenetre.
  - dans tous les cas, lancer la machine de reassemblage, pour donner 
    eventuellement a l'utilisateur les SDUs complets.
     

*/
{
   unsigned short SequenceNumber;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
   /* compteur-> "on a recu un PDU de donnees, correct" */
   Channel->Statistics[ _ValidPDUReceived ]++;

   if (Message->Ptr_Block->Nr_Of_Bytes==4) {
        /* compteur-> "on a recu un PDU de bourrage" */
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
        Channel->Statistics[ _StuffingReceived ]++;
#endif
   }
#endif

    /*retrig tempo*/
    if (Channel->StuffingMode == _MCS_WITH_STUFFING_MODE ) {
        RestartRecepWatchdogTempo(Channel);
    }

    SequenceNumber= (unsigned short)(
	 ((Message->Ptr_Block->Ptr_Data[2])<<8 ) |
	  (Message->Ptr_Block->Ptr_Data[3])
	 );

    switch (FindPositionInRecepWindow(SequenceNumber,Channel)){
       case _Inside_Window_Position:
       StoreReceivePDUWithoutDuplicAndSendAcknowledge(
             SequenceNumber, 
             Message,
             Channel);
       break;

       case _Upper_Window_Position:	/* dans extension */
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
        /* compteur-> "on a recu un PDU tardif (vieux)" */
        Channel->Statistics[ _OldDataPDUReceived ]++;
#endif 
 

      	{
#if (MCS_WITH_ACCSE == _MCS_OPTION_YES )
       MCS_BEGIN_CRITICAL(Mcs)  ;
       Channel->NumberOfInRequests -=2; /* pour indication utilisateur */
       MCS_END_CRITICAL(Mcs);
#endif
		/* liberer dans le bon ordre  */

		FDM_MSG_R_DESC * BlocData = Message->Ptr_Block;

        fdm_msg_ref_buffer_free(Message);
        fdm_msg_data_buffer_free(BlocData);

   }


#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
       break;

       case _Under_Window_Position:
       RecepWindowScroll(
                    SequenceNumber,
                    Message,
                    Channel,
                    _Small_Scroll);
       break;

       case _FarFrom_Window_Position:

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
        /* compteur-> "on a recu un PDU en dehors de la fenetre d'anticipation en
           reception" */   
     Channel->Statistics[ _OutOfWindowPDUReceived ]++;
#endif 

       RecepWindowScroll(
                   SequenceNumber, 
                   Message,
                   Channel,
                  _Large_Scroll);
       break;
    }

    LoopReceptionMCSConnectedServices(Channel);



}

#endif


/*******************************************************************/
/****** FONCTION PRIVATE StoreReceivePDUWithoutDuplicAnd       
                                   SendAcknowledge() ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void StoreReceivePDUWithoutDuplicAndSendAcknowledge(
     unsigned short                SequenceNumber   ,
     FDM_MSG_RECEIVED            * DLLMessage       ,
     MCSCHANNEL_REF             * Channel           )
{
/* Cette fonction stocke un nouveau PDU contenu dans DLLMessage, et
de sequence 'SequenceNumber', dans la fenetre d'anticipation en reception.
Cette fonction est utilise lorsqu'on recoit un PDU qui est inclus dans
la fenetre de reception exitante a l'instant T.
Cette fonction gere l'antiduplication et l'acquittement du PDU
*/

    RECEPTION_WINDOW_ELEMENT_TYPE * Box  = 
         &(Channel->RecepWindowArea->Box
               [SequenceNumber & (Channel->R_Anticipation-1)]);
     MCS_REF  *Mcs= Channel->Mcs; 

    if(Box->State==_Available_Pdu){

       /* PDU deja recu */
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
       MCS_BEGIN_CRITICAL(Mcs);   
       Channel->NumberOfInRequests -=2; 
       MCS_END_CRITICAL(Mcs);
#endif
	   {   /* recycler dans le bon ordre !*/
	       FDM_MSG_R_DESC * BlocData = DLLMessage->Ptr_Block;
        
		   fdm_msg_ref_buffer_free(DLLMessage);
           fdm_msg_data_buffer_free(BlocData);
       }

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
       /* compteur-> "on a recu un PDU que 'on a deja recu" */
       Channel->Statistics[ _MultipleDataPDUReceived ]++;
#endif 

    } else {
	 /*Stockage dans fenetre PDU Recu*/
	Box->State            = _Available_Pdu;
	Box->SequenceNumber   = SequenceNumber;
	Box->DLLMessage       = DLLMessage;
    }

    SendPDUAcknowledge(SequenceNumber,Channel);


}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE SendPDUAcknowledge() **********************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void SendPDUAcknowledge(
     unsigned short                SequenceNumber   ,
     MCSCHANNEL_REF             * Channel           )
{
/* Cette fonction construit et emet l'acquittement, pour un canal
A_DATA Reception, d'un PDU de numero 'SequenceNumber'.
Les ressources memoire permettant d'emettre cet acquittement
sont stockees dans une file.
Ces ressources ont ete preremplies au moment de la creation du canal.
Seul le numero de sequence est a remplir dans le PDU.
*/

    MCS_FDM_MSG_TO_SEND * PDUAckRessource;
    unsigned short        Cr;

  
    PDUAckRessource = (MCS_FDM_MSG_TO_SEND *)
        GET_FIFO_INT(&(Channel->AckRessourcesList));


    if (PDUAckRessource == NULL ) {
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
       /* compteur-> "manque de memoire pour emettre un acquittement" */
        Channel->Statistics[ _LackOfMemoryToProduceDataPDUAck ]++;
#endif 
        return;
    } 

    
   PDUAckRessource->Data[2] = (unsigned char )
                  (SequenceNumber>>8);
   PDUAckRessource->Data[3] = (unsigned char )
                  SequenceNumber;

   Cr=fdm_send_message(
         Channel->Lsap_Ctxt,
         &(PDUAckRessource->FdmToSend));

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
   /* compteur-> "acquittement emis" */
   Channel->Statistics[ _DataPDUAckProduced ]++;
#endif 

   if (Cr!=FDM_OK) {
	 Channel->Mcs->Extern_Signal_Error (  
			 _MCS_TYPE_MCS_FDM_MSG_TO_SEND,
             (void *) PDUAckRessource ,
             _MCS_INTERNAL_ERROR );
   }
 else {

#if (MCS_WITH_COMMAND_CHANNELS== _MCS_OPTION_YES )
            Channel->NumberOfOutRequests ++;
#endif


   }
 
}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE LoopReceptionMCSConnectedServices() *******/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoopReceptionMCSConnectedServices(MCSCHANNEL_REF * Channel)
{
/* procedure qui declenche les traitements de reception
d'un canal de type associe:
Ce traitement consiste en une exploration de la fenetre de reception,
le but etant de fournir a l'utilisateur les SDU constitues.

On parcourt alors la fenetre de reception du plus vieux numero au plus recent.
On a la contrainte de donner les SDUs dans l'ordre, c'est pourquoi 
l'exploration s'arrete des qu'on trouve une case de la fenetre, telle que
le PDU correspondant n'est pas encore arrive: on attendra (sauf en cas
de rupture) que ce PDU arrive pour fournir les PDUs suivants a l'utilisateur.

On peut rencontrer des PDUs de bourrage dans cette fenetre. Dans ce cas, on 
libere les ressources associees.

Lorsqu'on rencontre un PDU present, et que l'on n'a pas encore donne a l'utilisateur,
alors on construit l'indication correspondante, en prenant une ressource dans
la file interne reservee a cet effet.

Le SDU construit est numerote, puis indique a l'utilisateur.

Dans le cas ou on n'a pas de memoire, on genere un Warning, et on arrete l'exploration.

*/

    unsigned short i,j;
    unsigned short A = Channel->R_Anticipation;

    j= (unsigned short)(
        (Channel->RecepWindowFullingIndix+1) & (A-1));

    for(i=0;i<A;i++){

       RECEPTION_WINDOW_ELEMENT_TYPE * Box =   
            &(Channel->RecepWindowArea->Box[j]);



       if ( Box->State!=_Available_Pdu ) break;

       if ( Box->UserState==_NotYetTakenIntoAccount ) {

	     if (Channel->SDUType == _CHANNEL_FOR_MONOFRAME_SDU) {
		 if ( TakeNextMonoframeChannelPDU (Box, Channel )
                     ==_NOTHING_IS_DONE ) break;
                         
	     } else {
		 if (TakeNextMultiframeChannelPDU (Box, Channel )
                     ==_NOTHING_IS_DONE ) break;
	     }
        }
  

	j=(unsigned short) ((j+1)&(A-1));

    }   

}

#endif



/*******************************************************************/
/****** FONCTION PRIVATE TakeNextMonoframeChannelPDU() *************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
_PROCESS_ACTIVITY_LIST TakeNextMonoframeChannelPDU (
       RECEPTION_WINDOW_ELEMENT_TYPE * Box, 
       MCSCHANNEL_REF               * Channel)
{

/*procedure qui permet la prise en compte du PDU recu le plus ancien
(dans la numerotation) et non encore fourni a l'utilisateur, 
dans le cas d'un canal A_DATA reception de type Monotrame.

Cette fonction est appelee au moment de l'exploration de la fenetre de
reception, par la procedure LoopReceptionMCSConnctedServices(),
pour le PDU le plus ancien a chaque instant non fourni a l'utilisateur.

On regarde si c'est du bourrage: dans ce cas, on libere la ressource memoire
et le traitement est termine.

Dans le cas contraire, on va construire l'indication utilisateur, et lui
la fournir si on a une ressource memoire.

Si on est en manque de memoire, on genere un warning, et on sort.
Note: Cette situation signifie que l'utilisateur garde des ressources trop
longtemps, ou bien la demande de liberation des ressources traine trop 
longtemps dans la file d'interconnexion, ou encore que le nombre de services en // 
a ete mal dimensionne.

Rupture:
C'est ici que l'on traite l'indicateur de rupture, en inbtroduisant
un decalage dans la numerotation des SDUs.
L'indicateur est resette.
(l'indicateur de rupture avait ete positionne lorsque, suite a l'arrivee
d'un PDU de numero plus grand (plus recent) que la fenetre d'anticipation
a l'instant T, on avait sorti de cette fenetre un PDU qui n'etait pas encore recu, ou bien
un PDU qui n'avait pas encore ete fourni a l'utilisateur)

Compte-rendu: 
Cette fonction retourne un indicateur qui
informe du blocage, ou non de l'exploration.
L'exploration doit s'arreter lorsqu'on n'a plus de memoire 
pour fabriquer l'indication pour l'utilisateur.

*/


    MCS_SDU_RECEIVED   * DescRecep;
    FDM_MSG_RECEIVED   * Message      = Box->DLLMessage;
    FDM_MSG_R_DESC     * Desc         = Message->Ptr_Block;
    unsigned char      * Data         = Desc->Ptr_Data;
	
	MCS_REF			   *Mcs			  =Channel->Mcs;			

    if (Channel->BrokenSequence==_MCS_YES) {
        Channel->SDU_Number ++;
        Channel->BrokenSequence=_MCS_NO;
    }


    if (Message->Ptr_Block->Nr_Of_Bytes==4){ /*bourrage*/
         Box->UserState = _AlreadyTakenIntoAccount ;
          /* recycler le message bourrage */
		
		 /* attention a l'ordre de liberation : d'abord desc, puis donnees...*/
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
         MCS_BEGIN_CRITICAL(Mcs);   
         Channel->NumberOfInRequests -=2; 
         MCS_END_CRITICAL(Mcs);
#endif

          fdm_msg_ref_buffer_free(Message);
          fdm_msg_data_buffer_free(Desc);
 
		 
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
	      return _WORK_IS_DONE;

    }

    DescRecep= (MCS_SDU_RECEIVED *)
       _ALLOCATE_MEMORY  (Channel->PoolForUserIndicationRessources);

    if (DescRecep == NULL ) {
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
          /* compteur-> "manque de memoire pour fournir un SDU a l'utilisateur" */
          Channel->Statistics[ _LackOfMemoryToIndicate_DataSDU ]++;
#endif 
          Channel->Mcs->Extern_Signal_Warning(
			  _MCS_TYPE_MCS_CHANNEL_REF,
               (void *) Channel,
               _MCS_CHANNEL_CONNECTED_LACK_OF_MEMORY_FOR_USER_IND );
          return _NOTHING_IS_DONE;

    } 

    /*mise a jour du descripteur d'indication */
    /*les informations statiques etant peu nombreuses, 
     on les positionne la plutot qu'
     au moment de la configuration du canal*/ 
          
    DescRecep->Channel.UserChannelRef   = Channel->UserChannelRef;
    /*on se fiche de Acse_Ref */
    DescRecep->Channel.McsChannelRef    = Channel;
    DescRecep->Nr_Of_Blocks        = 1;

 
    DescRecep->Source              = _MCS_NOT_SIGNICANT_REMOTE_ADDRESS;
    DescRecep->SDU_Nr              = Channel->SDU_Number++;
    DescRecep->Ptr_Block           = (DESC_BLOCK_DATA_R * ) Desc;

    /*decaler le pointeur de donnees, pour depasser l'entete */
    Desc->Nr_Of_Bytes  -=4;
    Desc->Ptr_Data     +=4;
 
	/* calcul taille totale du message - 28.12.99*/
	DescRecep->SDU_Size =Desc->Nr_Of_Bytes;
	

    /* recyclage du descripteur DLL */
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
    MCS_BEGIN_CRITICAL(Mcs);   
    Channel->NumberOfInRequests --; 
    MCS_END_CRITICAL(Mcs);
#endif

    fdm_msg_ref_buffer_free(Message);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
#endif 

    /*indication a l'utilisateur*/
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
       Channel->NumberOfOutRequests++;
#endif

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
    /* compteur->une indication de SDU recu passee a l'utilisateur */
     Channel->Statistics[ _DataSDUIndicated ]++;
#endif 
    
     Box->UserState = _AlreadyTakenIntoAccount ;

	 Channel->User_Msg_Rec_Proc( 
           Channel->UserChannelRef, 
           DescRecep);
           

	return _WORK_IS_DONE;

}

#endif


/*******************************************************************/
/****** FONCTION PRIVATE TakeNextMultiframeChannelPDU() ************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

_PROCESS_ACTIVITY_LIST TakeNextMultiframeChannelPDU (
       RECEPTION_WINDOW_ELEMENT_TYPE * Box, 
       MCSCHANNEL_REF               * Channel)
{

/*procedure qui permet la prise en compte du PDU recu le plus ancien
(dans la numerotation) et non encore fourni a l'utilisateur, 
dans le cas d'un canal A_DATA reception de type Multitrame.

Cette fonction est appelee au moment de l'exploration de la fenetre de
reception, par la procedure LoopReceptionMCSConnctedServices(),
pour le PDU le plus ancien a chaque instant non fourni a l'utilisateur.

On regarde si c'est du bourrage: dans ce cas, on libere la ressource memoire
et le traitement est termine.

On verifie que le PDU recu est correct, du point de vue du protocole, et
notemment par rapport a ce que l'on a deja recu:
Ce qui est incorrect du point de vue du protocole:
   - traiter un PDU de bourrage alors qu'on est en cours de reassemblage
   - traiter un PDU vide de donnees, et tq l'entete ne doit pas = BME.
   - alors qu'on n'est pas en cours de reassemblage, traiter un PDU 
     dont l'entete ne correspond pas a un debut de message
   - alors qu'on est en cours de reassemblage, traiter un PDU dont l'entete
     correspond a un debut de message.

Actions en cas d'erreur de protocole:
    -provoquer un warning
    -avertir ACSE que dysfonctionnenemt
    -rejeter le PDU, mais faire comme si il avait effecttivement ete recu
     et pris.


Cas ou le protocole est "correct":
    -mettre a jour le contexte de reassemblage, si PDU est 'debut' ou 'mil'
    -construire le SDU si PDU = SDU entier, puis indiquer a l'utilisateur
    -finir l'assemblage du SDU si PDU est 'fin', puis indiquer a l'utilisateur
     dans ce cas, on remet la machine de reassemblage dans l'etat inactif
    

Si on est en manque de memoire, on genere un warning, et on sort.
Note: Cette situation signifie que l'utilisateur garde des ressources trop
longtemps, ou bien la demande de liberation des ressources traine trop 
longtemps dans la file d'interconnexion, ou encore que le nombre de services
 en // a ete mal dimensionne.


Dans tous les cas, mettre a jour les statistiques.

   

Rupture:
C'est ici que l'on traite l'indicateur de rupture, en introduisant
un decalage dans la numerotation des SDUs.
L'indicateur est resette ET la machine de reassemblage remise dans
l'etat inactif (ie vide).
(l'indicateur de rupture avait ete positionne lorsque, suite a l'arrivee
d'un PDU de numero plus grand (plus recent) que la fenetre d'anticipation
a l'instant T, on avait sorti de cette fenetre un PDU qui n'etait pas encore 
recu, ou bien un PDU qui n'avait pas encore ete fourni a l'utilisateur)


Compte-rendu: Cette fonction retourne un indicateur qui
informe du blocage, ou non de l'exploration.
L'exploration doit s'arreter lorsqu'on n'a plus de memoire 
pour fabriquer l'indication pour l'utilisateur.


*/
	/* ajout 4 datas suivantes pour calcul de SDU_size 28.12.99 */
	unsigned int i ;
	unsigned short		size_all_blocks;	
	DESC_BLOCK_DATA_R   *bloc_data_R;
	DESC_BLOCK_DATA_R   *next_bloc_data_R;

    MCS_SDU_RECEIVED   * DescRecep;

	MCS_REF			   * Mcs		  = Channel->Mcs;
    FDM_MSG_RECEIVED   * Message      = Box->DLLMessage;
    FDM_MSG_R_DESC     * Desc         = Message->Ptr_Block;
    unsigned char      * Data         = Desc->Ptr_Data;

    unsigned char	 Header       = Data[1];
   
    if (Channel->BrokenSequence==_MCS_YES) {
        Channel->SDU_Number ++;
        Channel->BrokenSequence=_MCS_NO;
        /* on poubelle le contexte de reassemblage */
        PurgeAndResetAssemblingContext(Channel);
        
    }


    /*bourrage, ou bien PDU vide */  
    if (Message->Ptr_Block->Nr_Of_Bytes==4){ 

	if ((Header!=__HEAD_BME)||
            (Channel->Assembling.State!=_EmptyMachine)){
            /* ce nest donc pas du bourrage, et c'est incorrect*/

/*	 suppression  warning car on a deja appel a Intern_Signal_ConnexionFailure
  Channel->Mcs->Extern_Signal_Warning ( 
					_MCS_TYPE_MCS_CHANNEL_REF,
                   (void *)Channel,
                   _MCS_CHANNEL_CONNECTED_ERROR_IN_PROTOCOL_EXCHANGE ) ;
*/
	    Intern_Signal_ConnexionFailure (  
                   Channel->Mcs, 
                   Channel,
                   _CONNEXION_FAILURE_In_ProtocoleExchange ) ;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            /* compteur-> erreur de protocole sur reception PDU */
             Channel->Statistics[ _DataPDUReceivedWithProtocolErrors ]++;
#endif 
       } 

       Box->UserState = _AlreadyTakenIntoAccount ;
        /* recycler le message */

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
       MCS_BEGIN_CRITICAL(Mcs);   
       Channel->NumberOfInRequests -=2; 
       MCS_END_CRITICAL(Mcs);
#endif

	    /* liberer dans le bon ordre*/
        fdm_msg_ref_buffer_free(Message);
	    fdm_msg_data_buffer_free(Desc);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
        return _WORK_IS_DONE;

    }

    /* le PDU contient donc des donnees */

    if (Channel->Assembling.State==_EmptyMachine) {
	/*les sequences attendues sont DEB ou DMF*/

	switch (Header){

	    case __HEAD_BEG:
	    Channel->Assembling.State         = _RunningMachine;
	    Channel->Assembling.Nr_Of_Blocks  = 1;

            /*decaler le pointeur de donnees, pour depasser l'entete */
        Desc->Nr_Of_Bytes  -=4;
        Desc->Ptr_Data     +=4;

	    Channel->Assembling.LineStart     = (DESC_BLOCK_DATA_R * )Desc;
	    Channel->Assembling.LineEnd       = (DESC_BLOCK_DATA_R * )Desc;

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
        MCS_BEGIN_CRITICAL(Mcs);   
        Channel->NumberOfInRequests --; 
        MCS_END_CRITICAL(Mcs);
#endif

        fdm_msg_ref_buffer_free(Message);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
#endif 
	    Box->UserState                     = _AlreadyTakenIntoAccount;
	    return _WORK_IS_DONE;

	    case __HEAD_BME:

            DescRecep= (MCS_SDU_RECEIVED *)
                 _ALLOCATE_MEMORY  (Channel->PoolForUserIndicationRessources);

            if (DescRecep == NULL ) {
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
               /* compteur->manque de memoire pour fournir a l'utilisateur un SDU */
               Channel->Statistics[ _LackOfMemoryToIndicate_DataSDU ]++;
#endif 
                Channel->Mcs->Extern_Signal_Warning(
					_MCS_TYPE_MCS_CHANNEL_REF,
                  (void *) Channel,
                   _MCS_CHANNEL_CONNECTED_LACK_OF_MEMORY_FOR_USER_IND );
               return _NOTHING_IS_DONE;

            } 


            /*mise a jour du descripteur d'indication */
            /*les informations statiques etant peu nombreuses, 
            on les positionne la plutot qu'
            au moment de la configuration du canal*/ 
          
            DescRecep->Channel.UserChannelRef   = Channel->UserChannelRef;
            /*on se fiche de Acse_Ref */
            DescRecep->Channel.McsChannelRef    = Channel;
            DescRecep->Nr_Of_Blocks        = 1;

 
            DescRecep->Source              = _MCS_NOT_SIGNICANT_REMOTE_ADDRESS;
            DescRecep->SDU_Nr              = Channel->SDU_Number++;
            DescRecep->Ptr_Block           = (DESC_BLOCK_DATA_R * ) Desc;

            /*decaler le pointeur de donnees, pour depasser l'entete */
            Desc->Nr_Of_Bytes  -=4;
            Desc->Ptr_Data     +=4;
 
			/* calcul taille totale du message - 28.12.99*/
			DescRecep->SDU_Size =Desc->Nr_Of_Bytes;
			
            /* recyclage du descripteur DLL */
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
            MCS_BEGIN_CRITICAL(Mcs);   
            Channel->NumberOfInRequests --; 
            MCS_END_CRITICAL(Mcs);
/* suppress           Channel->NumberOfOutRequests--;
*/
#endif

            fdm_msg_ref_buffer_free(Message);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            Channel->Statistics[ _FreeDLLRefBuffer ]++;
            /*compteur->indication a l'utilisateur*/
               Channel->Statistics[ _DataSDUIndicated ]++;
#endif 
 	        Box->UserState   = _AlreadyTakenIntoAccount;



			Channel->User_Msg_Rec_Proc( 
                Channel->UserChannelRef, 
                DescRecep);
           
            return _WORK_IS_DONE;
	    

	    default :
            /*erreur de protocole */
            PurgeAndResetAssemblingContext(Channel);
/*	 suppression  warning car on a deja appel a Intern_Signal_ConnexionFailure
	    Channel->Mcs->Extern_Signal_Warning ( 
					_MCS_TYPE_MCS_CHANNEL_REF,
                   (void *)Channel,
                   _MCS_CHANNEL_CONNECTED_ERROR_IN_PROTOCOL_EXCHANGE ) ;
*/
			Intern_Signal_ConnexionFailure (  
                   Channel->Mcs, 
                   Channel,
                   _CONNEXION_FAILURE_In_ProtocoleExchange ) ;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            /* compteur ->erreur de protocole sur reception d'un PDU */
            Channel->Statistics[ _DataPDUReceivedWithProtocolErrors ]++;
#endif 
 	    Box->UserState                     = _AlreadyTakenIntoAccount;
            /* recycler le message */

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
            MCS_BEGIN_CRITICAL(Mcs);   
            Channel->NumberOfInRequests -=2; 
            MCS_END_CRITICAL(Mcs);
#endif

            /* liberer dans le bon ordre*/
           fdm_msg_ref_buffer_free(Message);
           fdm_msg_data_buffer_free(Desc);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
            return _WORK_IS_DONE;
           
        }

    } else {


	/*les sequences attendues sont MIL ou FIN*/
	    
        switch (Header){

	    case __HEAD_MID:
	    Channel->Assembling.Nr_Of_Blocks++;

            /*decaler le pointeur de donnees, pour depasser l'entete */
            Desc->Nr_Of_Bytes  -=4;
            Desc->Ptr_Data     +=4;
	    Channel->Assembling.LineEnd->Next_Block  = 
                                 (DESC_BLOCK_DATA_R * )Desc;
	    Channel->Assembling.LineEnd              = 
                                 (DESC_BLOCK_DATA_R * ) Desc;

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
            MCS_BEGIN_CRITICAL(Mcs);   
            Channel->NumberOfInRequests --; 
            MCS_END_CRITICAL(Mcs);
#endif

            fdm_msg_ref_buffer_free(Message);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
#endif 
 	    Box->UserState                     = _AlreadyTakenIntoAccount;
            return _WORK_IS_DONE;

	    case __HEAD_END:
            DescRecep= (MCS_SDU_RECEIVED *)
                 _ALLOCATE_MEMORY  (Channel->PoolForUserIndicationRessources);

            if (DescRecep == NULL ) {
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
               /* compteur->manque memoire pour indication SDU a l'utilisateur */
               Channel->Statistics[ _LackOfMemoryToIndicate_DataSDU ]++;
#endif 
               Channel->Mcs->Extern_Signal_Warning(
				   _MCS_TYPE_MCS_CHANNEL_REF,
                  (void *) Channel,
                   _MCS_CHANNEL_CONNECTED_LACK_OF_MEMORY_FOR_USER_IND );
               return _NOTHING_IS_DONE;

            } 

	    Channel->Assembling.State=_EmptyMachine;

            /*decaler le pointeur de donnees, pour depasser l'entete */
            Desc->Nr_Of_Bytes  -=4;
            Desc->Ptr_Data     +=4;
	    Channel->Assembling.LineEnd->Next_Block=(DESC_BLOCK_DATA_R * )Desc;


            /*mise a jour du descripteur d'indication */
            /*les informations statiques etant peu nombreuses, 
            on les positionne la plutot qu'
            au moment de la configuration du canal*/ 
          
            DescRecep->Channel.UserChannelRef   = Channel->UserChannelRef;
            /*on se fiche de Acse_Ref */
            DescRecep->Channel.McsChannelRef    = Channel;
            DescRecep->Nr_Of_Blocks       = Channel->Assembling.Nr_Of_Blocks+1;

 
            DescRecep->Source              = _MCS_NOT_SIGNICANT_REMOTE_ADDRESS;
            DescRecep->SDU_Nr              = Channel->SDU_Number++;
            DescRecep->Ptr_Block           = Channel->Assembling.LineStart;

	    size_all_blocks=0;
	   /* calcul taille totale du message - 28.12.99*/
	   bloc_data_R=DescRecep->Ptr_Block;		/* descript 1er bloc */
	   for (i=0;i<DescRecep->Nr_Of_Blocks ;i++){
		size_all_blocks +=bloc_data_R->Nr_Of_Bytes;
		next_bloc_data_R=bloc_data_R->Next_Block;
		bloc_data_R=next_bloc_data_R;
	    }
	    DescRecep->SDU_Size =size_all_blocks;



            /* recyclage du descripteur DLL */
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
            MCS_BEGIN_CRITICAL(Mcs);   
            Channel->NumberOfInRequests --; 
            MCS_END_CRITICAL(Mcs);
/*suppress  	Channel->NumberOfOutRequests ++; */
#endif

            fdm_msg_ref_buffer_free(Message);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            Channel->Statistics[ _FreeDLLRefBuffer ]++;
            /*compteur -> indication a l'utilisateur*/
            Channel->Statistics[ _DataSDUIndicated ]++;
#endif 
            Channel->User_Msg_Rec_Proc( 
                Channel->UserChannelRef, 
                DescRecep);
           
	    Box->UserState                  = _AlreadyTakenIntoAccount;
            return _WORK_IS_DONE;


	    default :
            /*erreur protocole */
            PurgeAndResetAssemblingContext(Channel);
/*	 suppression  warning car on a deja appel a Intern_Signal_ConnexionFailure
	    Channel->Mcs->Extern_Signal_Warning (  
					_MCS_TYPE_MCS_CHANNEL_REF,
                   (void *)Channel,
                   _MCS_CHANNEL_CONNECTED_ERROR_IN_PROTOCOL_EXCHANGE ) ;
*/	    Intern_Signal_ConnexionFailure (  
                   Channel->Mcs, 
                   Channel,
                   _CONNEXION_FAILURE_In_ProtocoleExchange ) ;
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
             /* compteur -> erreur de protocole sur reception de PDU */
            Channel->Statistics[ _DataPDUReceivedWithProtocolErrors ]++;
#endif 
 	    Box->UserState                     = _AlreadyTakenIntoAccount;
            /* recycler le message */
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
            MCS_BEGIN_CRITICAL(Mcs);   
            Channel->NumberOfInRequests -=2; 
            MCS_END_CRITICAL(Mcs);
#endif

            /*liberer dans le bon ordre*/
            fdm_msg_ref_buffer_free(Message);
			fdm_msg_data_buffer_free(Desc);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
            return _WORK_IS_DONE;
 	}
    }

}

#endif




/*******************************************************************/
/****** FONCTION PRIVATE TakeNextMultiframeChannelPDU() ************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void  PurgeAndResetAssemblingContext(MCSCHANNEL_REF * Channel)
{

/*procedure qui remet le contexte de reassemblage dans l'etat 
initial, ie sans reassemblage en cours, apres avoir recycle
toutes les ressources qui y etaient stockees*/


    int 	        i;
    DESC_BLOCK_DATA_R * P1, *P2;

	MCS_REF *Mcs  = Channel->Mcs;
    if (Channel->Assembling.State ==_EmptyMachine) {
	return;
    }

    P1=Channel->Assembling.LineStart;

    for (i=1; i<=(int)Channel->Assembling.Nr_Of_Blocks;i++) {
         P2=P1->Next_Block;

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
         MCS_BEGIN_CRITICAL(Mcs);   
         Channel->NumberOfInRequests --; 
         MCS_END_CRITICAL(Mcs);
#endif

       fdm_msg_data_buffer_free((FDM_MSG_R_DESC* )P1);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 

       P1=P2;
    }

    Channel->Assembling.State =_EmptyMachine;


}

#endif



/*******************************************************************/
/****** FONCTION PRIVATE RecepWindowScroll() ***********************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

void RecepWindowScroll (     
     unsigned short                 SequenceNumber   ,
     FDM_MSG_RECEIVED             * DLLMessage       ,
     MCSCHANNEL_REF              * Channel         ,
     _RECEP_WINDOW_SCROLL_TYPE_LIST Type             )

{

/* Cette fonction engage le glissement de la fenetre de reception, 
jusqu'au PDU de sequence 'SequenceNumber', et stocke dans la ressource
'DLLMessage'.
On peut engager un 'petit' glissement, dans les cas normaux, ou bien
un 'grand' glissement, dans les  autres cas (parametre 'Type').
*/


    unsigned int                   NrOfBoxesToModify;
    unsigned int                   i;
    RECEPTION_WINDOW_ELEMENT_TYPE *Box;
    unsigned short                 A = Channel->R_Anticipation;
    unsigned short                 NumSeqFulling =
       (Channel->RecepWindowArea->Box[Channel->RecepWindowFullingIndix])
             .SequenceNumber;
      
	int RuptureIsAlreadySignaled=_MCS_NO;
	MCS_REF						   *Mcs =Channel->Mcs;


    NrOfBoxesToModify=(unsigned int) DISTANCE_BETWEEN(
           SequenceNumber,
           NumSeqFulling);

    if(Type==_Large_Scroll){
	     NrOfBoxesToModify=(unsigned short)(
		    NrOfBoxesToModify+A-1);
    }

    /*Purge et reinit sauf num*/
    for(i=0;i< NrOfBoxesToModify;i++){

	RECEPTION_WINDOW_ELEMENT_TYPE* Box_Queue;
	int	   	               Q_ind;

	Q_ind       = ((Channel->RecepWindowFullingIndix)+1)&(A-1);
	Box_Queue   = &(Channel->RecepWindowArea->Box[Q_ind]);

	if  ((Box_Queue->State       == _Available_Pdu)         &&
             (Box_Queue->UserState   == _NotYetTakenIntoAccount)){


#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
             MCS_BEGIN_CRITICAL(Mcs);   
             Channel->NumberOfInRequests -=2; 
             MCS_END_CRITICAL(Mcs);
#endif
	   { /*liberer dans le bon ordre  */
			FDM_MSG_R_DESC * BlocData = DLLMessage->Ptr_Block;

            fdm_msg_ref_buffer_free(DLLMessage);
		    fdm_msg_data_buffer_free(BlocData);

        }

            
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
     Channel->Statistics[ _FreeDLLRefBuffer ]++;
     Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif 
	}

	if ((Box_Queue->State!=_Available_Pdu)               ||
            (Box_Queue->UserState==_NotYetTakenIntoAccount)  ){

            /*rupture*/
       if (RuptureIsAlreadySignaled==_MCS_NO) {

          RuptureIsAlreadySignaled = _MCS_YES;

/*	 suppression  warning car on a deja appel a Intern_Signal_ConnexionFailure
	      Channel->Mcs->Extern_Signal_Warning ( 
				_MCS_TYPE_MCS_CHANNEL_REF,
                   (void *)Channel,
                   _MCS_CHANNEL_CONNECTED_BROKEN_SEQUENCE ) ;
*/
	      Intern_Signal_ConnexionFailure ( 
                   Channel->Mcs, 
                   Channel,
                   _CONNEXION_FAILURE_BrokenSequence);
	   }

	    Channel->BrokenSequence = _MCS_YES;

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
            /* compteur->rupture de sequence, par manque de PDUs recus */
               Channel->Statistics[ _SequenceBreakInDataPDUList ]++;
#endif 
 
	}



	Box_Queue->State             =_Free_Box;
	Box_Queue->UserState         =_NotYetTakenIntoAccount;
	Box_Queue->SequenceNumber   +=A;

	/*reinit tete fenetre*/
	Channel->RecepWindowFullingIndix  =   (unsigned short)
	     (((Channel->RecepWindowFullingIndix)+1)&(A-1));

	/*main SDU_MAKE*/
	LoopReceptionMCSConnectedServices(Channel);
	}

    /*stockage du message recu + envoi ACK*/
    Box                     = 
          &(Channel->RecepWindowArea->Box[SequenceNumber&(A-1)]);
    Box->State              = _Available_Pdu;
    Box->DLLMessage         = DLLMessage;

    SendPDUAcknowledge(SequenceNumber,Channel);

    /*main SDU_MAKE*/
    LoopReceptionMCSConnectedServices(Channel);


}

#endif

/*******************************************************************/
/****** FONCTION PRIVATE Exec_FreeDataSDURessource() ***************/
/*******************************************************************/
void Exec_FreeDataSDURessource(
       MCS_SDU_RECEIVED * Message,
       MCSCHANNEL_REF *  Channel)
{
/*fonction pour le recyclage des ressources qui constituent
un SDU indique a l'utilisateur.
Cette fonction est soit appelee 'directement' par l'utilisateur,
lorsqu'il s'agit d'un liberation pour un canal de type non
associe, soit a la suite de la mise en file.


*/

    int                 i;
    DESC_BLOCK_DATA_R * p, *p2;
	MCS_REF			 *Mcs		  = Channel->Mcs;
    int              NrOfBlocks   = Message->Nr_Of_Blocks;

    p          =  Message->Ptr_Block;

    for (i=0; i<NrOfBlocks; i++) {

		/*nouveau bloc*/
		p2=p;
        p=p->Next_Block;
       
	/*liberer bloc*/
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
        if (Channel->ServType == _CHANNEL_FOR_CONNECTED_SERVICES ) {
            MCS_BEGIN_CRITICAL(Mcs);   
            Channel->NumberOfInRequests --; 
            MCS_END_CRITICAL(Mcs);
/*suppress           Channel->NumberOfOutRequests --;  */
        }
#endif

        fdm_msg_data_buffer_free((FDM_MSG_R_DESC*) p2);

#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
        Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif

    }
    /*liberer_desc */
    _FREE_MEMORY((User_GDM*)Message);
}


/*******************************************************************/
/****** FONCTION PRIVATE Intern_Signal_ConnexionFailure() **********/
/*******************************************************************/

void Intern_Signal_ConnexionFailure (
       MCS_REF          * Mcs,
       MCSCHANNEL_REF  * Channel, 
       _CONNEXION_FAILURE_LIST Error)
{
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )

/*fonction interne, pour le dysfonctionnement d'un canal de 
type A_DATA.
Cette fonction signalera le probleme a ACSE, dans le cas
ou cette option est choisie */
    if ( Channel->State == _MCS_CHANNEL_STATE_RUNNING ){
		 Mcs->Extern_Signal_ConnexionFailure (
         Channel->UserChannelRef,  Error);
	}
#endif
#endif
}



/*************** End Of  'mcsheart.c' File *************************/
