/************************************************************
   
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcstools.c
  Description     :Procedures pour faciliter la mise en oeuvre de MCS
				   procedure pour permettre l'affichage des erreurs et warnings 
                   
 
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000 

***********************************************************/


#include "mcs.h"

#include <string.h>


/********** FONCTION PUBLIC-MCS mcs_get_errorandwarningstring() ******/
void mcs_get_errorandwarningstring (
	char *               String,
	_MCS_ERROR_CODE_LIST Code)

{
/* cette fonction remplit la chaine de caractere String donnee par l'appelant
   par le symbole representant le code de l'erreur 
	
   On supposera que la chaine fournie fait au moins 40 caracteres.

   Les codes d'erreur traites doivent faire moins de 40 caracteres.


  ATTENTION: cette procedure doit etre mise a jour lorsqu'on ajoute ou on supprime 
  des codes d'erreur.

 */


   switch (Code) {

       case _MCS_OK:
	   strcpy(String, "_MCS_OK");
       break;

       case _MCS_NOK:
	   strcpy(String, "_MCS_NOK");
       break;

       case _MCS_INTERNAL_ERROR:
	   strcpy(String, "_MCS_INTERNAL_ERROR");
       break;

       case _MCS_CREATE_ILLEGAL_PARAMETER:
	   strcpy(String, "_MCS_CREATE_ILLEGAL_PARAMETER");
       break;

       case _MCS_CREATE_ALLOCATE_MEMORY_FAULT:
	   strcpy(String, "_MCS_CREATE_ALLOCATE_MEMORY_FAULT");
       break;


 	   case _MCS_CREATE_CREATE_SEMAPHORE_FAULT:
	   strcpy(String, "_MCS_CREATE_CREATE_SEMAPHORE_FAULT");
       break;

	   case _MCS_CHANNEL_CREATE_ON_ILLEGAL_INSTANCE:
       strcpy(String, "_MCS_CHANNEL_CREATE_ON_ILLEGAL_INSTANCE");
	   break;
	   case _MCS_CHANNEL_CREATE_ILLEGAL_USERCHANNEL_REF:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_USERCHANNEL_REF");
       break;

	   case _MCS_CHANNEL_CREATE_ILLEGAL_CONNEXION_TEMPO:
       strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_CONNEXION_TEMPO");
	   break;

       case _MCS_CHANNEL_CREATE_ALLOCATE_MEMORY_FAULT:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ALLOCATE_MEMORY_FAULT");
       break;

       case _MCS_CHANNEL_CREATE_ILLEGAL_DLL_SIZE:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_DLL_SIZE");
       break;


       case _MCS_CHANNEL_CREATE_ILLEGAL_SERVICE_TYPE:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_SERVICE_TYPE");
       break;

       case _MCS_CHANNEL_CREATE_ILLEGAL_SDU_TYPE:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_SDU_TYPE");
       break;

       case _MCS_CHANNEL_CREATE_ILLEGAL_STUFFING_MODE:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_STUFFING_MODE");
       break;

       case _MCS_CHANNEL_CREATE_ILLEGAL_CALL_BACK_FUNCTION:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_CALL_BACK_FUNCTION");
       break;
	   
       case _MCS_CHANNEL_CREATE_ILLEGAL_LOCAL_ADDRESS:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_LOCAL_ADDRESS");
       break;
   
       case _MCS_CHANNEL_CREATE_ILLEGAL_REMOTE_ADDRESS:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_REMOTE_ADDRESS");
       break;

       case _MCS_CHANNEL_CREATE_ILLEGAL_CHANNEL_NR:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_CHANNEL_NR");
       break;

       case _MCS_CHANNEL_CREATE_ILLEGAL_SDU_SIZE:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_SDU_SIZE");
       break;

       case _MCS_CHANNEL_CREATE_ILLEGAL_NUMBER_OF_PARALLEL_SERVICES:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_NUMBER_OF_PARALLEL_SERVICES");
       break;

       case _MCS_CHANNEL_CREATE_ILLEGAL_ANTICIPATION:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_ANTICIPATION");
       break;

 
	   case _MCS_CHANNEL_CREATE_ILLEGAL_REFUSED_DLL_CTXT_CREATION:
	   strcpy(String, "_MCS_CHANNEL_CREATE_ILLEGAL_REFUSED_DLL_CTXT_CREATION");
       break;

	   case _MCS_CHANNEL_START_ON_INEXISTING_CHANNEL:
	   strcpy(String, "_MCS_CHANNEL_START_ON_INEXISTING_CHANNEL");
       break;

       case _MCS_CHANNEL_COMMAND_ON_ILLEGAL_INSTANCE:
	   strcpy(String, "_MCS_CHANNEL_COMMAND_ON_ILLEGAL_INSTANCE");
       break;
       case _MCS_CHANNEL_COMMAND_ILLEGAL_CHANNEL_STATE:
	   strcpy(String, "_MCS_CHANNEL_COMMAND_ILLEGAL_CHANNEL_STATE");
       break;
       case _MCS_CHANNEL_COMMAND_ILLEGAL_SERVICE:
	   strcpy(String, "_MCS_CHANNEL_COMMAND_ILLEGAL_SERVICE");
       break;
       case _MCS_CHANNEL_COMMAND_ILLEGAL_PARAMETERS:
	   strcpy(String, "_MCS_CHANNEL_COMMAND_ILLEGAL_PARAMETERS");
       break;

	   
	   
	   
	   case _MCS_SEND_DATA_ON_INEXISTING_CHANNEL:
	   strcpy(String, "_MCS_SEND_DATA_ON_INEXISTING_CHANNEL");
       break;

       case _MCS_SEND_DATA_ON_NOT_RUNNING_CHANNEL:
	   strcpy(String, "_MCS_SEND_DATA_ON_NOT_RUNNING_CHANNEL");
       break;

       case _MCS_SEND_DATA_ON_IDLE_CHANNEL:
	   strcpy(String, "_MCS_SEND_DATA_ON_IDLE_CHANNEL");
       break;

       case _MCS_SEND_DATA_ON_DELETE_IN_PROGRESS_CHANNEL:
	   strcpy(String, "_MCS_SEND_DATA_ON_DELETE_IN_PROGRESS_CHANNEL");
       break;

       case _MCS_SEND_DATA_INVALID_LENGTH:
	   strcpy(String, "_MCS_SEND_DATA_INVALID_LENGTH");
       break;

       case _MCS_SEND_DATA_INVALID_DESTINATION:
	   strcpy(String, "_MCS_SEND_DATA_INVALID_DESTINATION");
       break;
 
       case _MCS_SEND_DATA_DLL_FAIL:
	   strcpy(String, "_MCS_SEND_DATA_DLL_FAIL");
       break;

	   case _MCS_RECEIVE_DLL_ON_INEXISTING_CHANNEL:
	   strcpy(String, "_MCS_RECEIVE_DLL_ON_INEXISTING_CHANNEL");
       break;
 
       case _MCS_RECEIVE_DLL_ON_IDLE_CHANNEL:
	   strcpy(String, "_MCS_RECEIVE_DLL_ON_IDLE_CHANNEL");
       break;
  
       case _MCS_RECEIVE_DLL_ON_DELETE_IN_PROGRESS_CHANNEL:
	   strcpy(String, "_MCS_RECEIVE_DLL_ON_DELETE_IN_PROGRESS_CHANNEL");
       break;
  
       case _MCS_RECEIVE_DLL_INVALID_PDU:
	   strcpy(String, "_MCS_RECEIVE_DLL_INVALID_PDU");
       break;
  
       case _MCS_RECEIVE_DLL_BROKEN_MESSAGE:
	   strcpy(String, "_MCS_RECEIVE_DLL_BROKEN_MESSAGE");
       break;
  
       case _MCS_CHANNEL_CONNECTED_LACK_OF_MEMORY_FOR_USER_IND:
	   strcpy(String, "_MCS_CHANNEL_CONNECTED_LACK_OF_MEMORY_FOR_USER_IND");
       break;

       case _MCS_CHANNEL_CONNECTED_ERROR_IN_PROTOCOL_EXCHANGE:
	   strcpy(String, "_MCS_CHANNEL_CONNECTED_ERROR_IN_PROTOCOL_EXCHANGE");
       break;

       case _MCS_CHANNEL_CONNECTED_BROKEN_SEQUENCE:
	   strcpy(String, "_MCS_CHANNEL_CONNECTED_BROKEN_SEQUENCE");
       break;

       case _MCS_SEND_DLL_ON_INEXISTING_CHANNEL:
	   strcpy(String, "_MCS_SEND_DLL_ON_INEXISTING_CHANNEL");
       break;

       case _MCS_SEND_DLL_ON_IMAGE2:
	   strcpy(String, "_MCS_SEND_DLL_ON_IMAGE2");
       break;

       case _MCS_SEND_DLL_INTERNAL_ERROR:
	   strcpy(String, "_MCS_SEND_DLL_INTERNAL_ERROR");
       break;

       case _MCS_SEND_DLL_UNKNOWN_ERROR:
	   strcpy(String, "_MCS_SEND_DLL_UNKNOWN_ERROR");
       break;

	   default:
	   strcpy(String, "Unknown MCS Report code");
       break;
   }
}















/* end of file */







