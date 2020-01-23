/************************************************************
   
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcs_user.c
  Description     : Procedure de demarrage de X canaux
					Procedure de liberation ressources sur sortie generale
					
                   
 
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000 
  Date de modification : 15.06.2000 
		DE 3 : utilisation messagerie mode normalise ou non acquitte
			   
		DE7 : utilisation no segment autre que 0
			  reprise du controle des adresses locales et distantes

		autre modification : - sur pb dans config ajout pour 
							  restitution des ressources
							- mise mode monotrame ou multitrame
		DE2: pour optimisation des ressources memoire :
			 creation destruction des contextes en dynamique 

		Autre modification: on ne configure la messagerie que 
							sur l'image 1


************************************************************************/


#include "mcs.h"



/*====================================================
					mcs_cat_msg_received
======================================================*/
/* lors de la reception du message , l'utilisateur connait  la taille 
du message : MCS_SDU_RECEIVED --> SDU_Size

en entree de cette  procedure , il fournit donc :
parametre 1: un pointeur  ptr_data_user, sur une zone qu'il a alloue 
			et qui est au minimum aussi grande que SDU_Size 
parametre 2:*/

void  mcs_cat_msg_received(char *ptr_data_user,MCS_SDU_RECEIVED *msg)
{
	DESC_BLOCK_DATA_R	*ptr_block_act;
	DESC_BLOCK_DATA_R	*ptr_block_next;
	unsigned char		*ptr_data_block;
    unsigned int        Nr_Of_Blocks;
	unsigned int		i;
 	unsigned short      Nr_Of_Bytes;
 	unsigned short      j;

	
	Nr_Of_Blocks=msg->Nr_Of_Blocks;
	ptr_block_act=msg->Ptr_Block;
	for (i=0;i<Nr_Of_Blocks;i++){
		Nr_Of_Bytes    = ptr_block_act->Nr_Of_Bytes;
		ptr_data_block = ptr_block_act->Ptr_Data;
		for (j=0;j<Nr_Of_Bytes;j++){
			*ptr_data_user++=*ptr_data_block++;
		}
		ptr_block_next=ptr_block_act->Next_Block;
		ptr_block_act=ptr_block_next;
	}


}

/*====================================================
					mcs_channel_command
======================================================*/
/* cette fonction est appelee par L'utilisateur  lorsque celui-ci souhaite
         creer /demarrer un nouveau canal  
									(NEANT -> IDLE->RUNNING)
		 arreter un canal qui est en cours de fonctionnement
                                   (RUNNING -> IDLE)
         detruire un canal qui est arrete
                                    (IDLE ->NEANT)


Cette fonction stocke la commande dans la file des commandes pour l'instance,
et retourne OK, sauf si on est certain que la reference de l'instance est incorrecte.

On verifie que la commande demandee est supportee.

Dans le cas ou la commande n'est pas une commande de creation de canal, on verifie
que la commande correspond bien a l'etat du canal.

Dans les cas invalides, on renvoie un compte-rendu immediat mauvais.

Les autres controles et traitement de cette commande se font au moment du depilage.

Notes:
- au moment du depilage, on reverifiera l'etat du canal.
- que le nombre de canaux crees ne depasse pas le max possible
*/

 _MCS_ERROR_CODE_LIST mcs_channel_command(MCS_REF *Mcs,
										  MCS_CHANNEL_COMMAND *Command)
{

	INTEG_FILE_TYPE       * File     = & (Mcs->List[_CommandList]);
	MCSCHANNEL_REF        * Channel  = NULL;
	_MCS_ERROR_CODE_LIST    Cr       ;

	if ((Mcs->CheckNumber) != _rangdom_number ) {
	   /* le pointeur sur l'instance est mauvais */
	   return _MCS_CHANNEL_COMMAND_ON_ILLEGAL_INSTANCE;
	}
	
	switch ( Command->Command  ) {
		case _MCS_CHANNEL_CREATE:
		if (Command->paramIN_for_create== NULL )  
		   return _MCS_CHANNEL_COMMAND_ILLEGAL_PARAMETERS;
		break;

		case _MCS_CHANNEL_STOP:
		case _MCS_CHANNEL_DELETE:
		if (Command->channelmcs == NULL )
		return _MCS_CHANNEL_COMMAND_ILLEGAL_PARAMETERS;
		Channel = Command-> channelmcs;		
		break;
    }
	Cr= CheckCommandTowardsChannelFeaturesAndState(Channel, Command->Command );

	if (Cr != _MCS_OK ) return Cr;

    /* la commande est correcte..*/
	Command->mcs_ref=Mcs;
	OS_Enter_Region();
	STORE_IN_FIFO_INTEG(File  , (INTEG_FILE_ELT *)Command );
	OS_Leave_Region();

	return _MCS_OK;

}


/*******************************************************************/
/* FONCTION PRIVATE CheckCommandTowardsChannelFeaturesAndState     */
/*******************************************************************/
_MCS_ERROR_CODE_LIST CheckCommandTowardsChannelFeaturesAndState
    ( MCSCHANNEL_REF             *  Channel ,
     _MCS_CHANNEL_COMMAND_LIST   Command)

{
/* fonction qui verifie la coherence d'une commande demandee par
un utilisateur, et le type et l'etat du canal sur lequel
la commande est demandee.
Cette fonction est appelee au moment ou l'utilisateur effectue 
sa demande, et au moment ou MCS exploite cette demande,
a la sortie de la file.
   -commande create: ok si Channel = NULL
   -commande stop: possible
        -si canal est de type CONNECTE
        -si canal est dans l'etat RUNNING
   -commande delete: possible
        -si canal est de type CONNECTE
        -si canal est dans l'etat IDLE_TO_DELETE

La focntion renvoie:

_MCS_OK   ->  quand la commande est valide
_MCS_CHANNEL_COMMAND_ILLEGAL_CHANNEL_STATE 
         ->  quand la commande est invalide a cause de l'etat du canal
_MCS_CHANNEL_COMMAND_ILLEGAL_SERVICE 
        ->  dans les autres cas

*/

       _MCS_CHANNEL_STATE_LIST     State ;
	   
	if (Channel != NULL)
		State= Channel->State;
	else 
		State=_MCS_CHANNEL_STATE_INEXISTING;

	if (( Command != _MCS_CHANNEL_CREATE)  &&
        ( Command != _MCS_CHANNEL_STOP   ) &&
        ( Command != _MCS_CHANNEL_DELETE ) ){
           return _MCS_CHANNEL_COMMAND_ILLEGAL_SERVICE;
       }

	if ( Command == _MCS_CHANNEL_CREATE ) {
	   if ( Channel !=  NULL )
		 return _MCS_CHANNEL_COMMAND_ILLEGAL_SERVICE;
	}


	if  ( Command == _MCS_CHANNEL_STOP ) {
	   if  (Channel->ServType != _CHANNEL_FOR_CONNECTED_SERVICES  ){
			return _MCS_CHANNEL_COMMAND_ILLEGAL_SERVICE;
	   }
	   if  (State  != _MCS_CHANNEL_STATE_RUNNING )   {  
			return _MCS_CHANNEL_COMMAND_ILLEGAL_CHANNEL_STATE;
	   }
	}

	if  ( Command == _MCS_CHANNEL_DELETE ) {
		if  (Channel->ServType != _CHANNEL_FOR_CONNECTED_SERVICES  ){
			return _MCS_CHANNEL_COMMAND_ILLEGAL_SERVICE;
		}
		if  (State        != _MCS_CHANNEL_STATE_IDLE_TO_DELETE )   {  
			return _MCS_CHANNEL_COMMAND_ILLEGAL_CHANNEL_STATE;
		}
	}
     
    return _MCS_OK;


}



/*====================================================
					McsChannelCreateUserSpecif
======================================================*/

MCSCHANNEL_REF*  McsChannelCreateUserSpecif (MCS_REF * Mcs ,
								   MCS_USER_CHANNEL_CONFIGURATION *config,
								   _MCS_ERROR_CODE_LIST *Report)
{
	MCSCHANNEL_REF				*channelmcs;
	_MCS_ERROR_CODE_LIST		Cr;
	MCS_CHANNEL_CONFIGURATION   In_Conf;
	unsigned short				lsap_remote; 
	char Name[] ="CONNEX_X";

	channelmcs = (MCSCHANNEL_REF*)0;

	/* controle des parametres specifiques */
	if (config->DLLType==_MCS_TYPE_DLL_MICROFIP){
	/*si type = MICROFIP lsap remote =lsap des station microfip 
				 00 xy
			 e   0F xy ; avec  xy = adresse physique station microfip(0 a 256)
	*/
		lsap_remote=(unsigned short)(config->Remote_Address >>8);
		if ( lsap_remote >0xFFF){
			*Report=_MCS_CHANNEL_CREATE_ILLEGAL_REMOTE_ADDRESS;
			return (channelmcs);
		}
	}

	if (config->ConnexionTempoTicks ==0){
		*Report=_MCS_CHANNEL_CREATE_ILLEGAL_CONNEXION_TEMPO;
		return (channelmcs);
	}

	/* Initialisation de MCS_CHANNEL_CONFIGURATION en fonction de 
						MCS_USER_CHANNEL_CONFIGURATION*/

		In_Conf.UserChannelRef=config->UserChannelRef;

		In_Conf.Name =(char*)&Name;                         
		In_Conf.ServType=_CHANNEL_FOR_CONNECTED_SERVICES;
		In_Conf.CommType=_CHANNEL_FOR_TRANSM_AND_RECEP; 

		In_Conf.Local_DLL_Address	=config->Local_Address;		
		In_Conf.Remote_DLL_Address	=config->Remote_Address;		
		In_Conf.MaxSDUSize			=config->MaxSDUSize;
		In_Conf.MaxParallelServices  =1;
		if (config->DLLType==_MCS_TYPE_DLL_MICROFIP){
			In_Conf.DLLSize=122;
		}else{
			In_Conf.DLLSize=256;
		}
	   /* modif 09.05.2000	positionner mode monotrame ou multitrame */
		if (In_Conf.MaxSDUSize >In_Conf.DLLSize){
			In_Conf.SDUType =_CHANNEL_FOR_MULTIFRAME_SDU; 
		}else{
			In_Conf.SDUType =_CHANNEL_FOR_MONOFRAME_SDU; 
		}
		In_Conf.StuffingMode=_MCS_WITHOUT_STUFFING_MODE;
		In_Conf.ConnexionTempoTicks=config->ConnexionTempoTicks;

		In_Conf.Send_infos.Anticipation=1;
		In_Conf.Send_infos.NbOfRetry   =1;
		In_Conf.Send_infos.User_Msg_Ack=config->User_Msg_Ack;

		In_Conf.Receive_infos.Anticipation=1;
		In_Conf.Receive_infos.NbOfRetry   =1;
		In_Conf.Receive_infos.User_Msg_Rec_Proc =config->User_Msg_Rec_Proc; 
		In_Conf.Position =_MCS_MSG_IMAGE_1;

		if (config->msg_type==_MCS_MSG_TYPE_APERIOD){
			/* messagerie aperiodique : canal 0*/
			In_Conf.Channel_Nr =0;
		}else{
			/* on utilisera uniquement le canal 1 en messagerie periodique */
			In_Conf.Channel_Nr =1;	
		}

		In_Conf.msg_mode_ack =config->msg_mode_ack;

	/* tout OK ===> creation reference du canal*/
    channelmcs= McsChannelCreate (Mcs,&In_Conf,&Cr);
	*Report =Cr;
	return (channelmcs);

}















