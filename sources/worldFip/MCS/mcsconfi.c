/************************************************************

"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization

  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcsconfi.c
  Description     : configuration des objets MCS:
                    instance mcs
					creation d un canal _ start d'un canal


  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000
  Modification 24.04.2000:
						include mcspriv2.h au lieu mcsprive2.h
						suppression procedure CheckReceptionStructure
  Modification 09.05.2000:
				 DE 3 : choix messagerie acquittee ou non
					1)	ajout procedure proc_msg_no_ack


				DE2:  optimisation des ressources memoires:
					  creation/destruction des contextes



************************************************************************/


#include "mcs.h"

FIFO_INT_TYPE                 ListOfMCSs;


/* Version 1.0     du 20 juin 2000 */


static const MCS_VERSION ThisVersion= {

       1 ,           /* Version			*/
       0 ,           /* Revision		*/
       0 ,			 /* c'est un proto  */
       0 ,           /* numero proto	*/
       {00 ,          /* Date: Annee		*/
       20 ,           /* Date: Mois		*/
       06 }           /* Date: Jour		*/

};

/*====================================================
					mcs_get_software_version
======================================================*/
/*  permet dde connaitre la version du logiciel
*/
void	mcs_get_software_version ( MCS_VERSION * Version){
		*Version = ThisVersion;

}

/*******************************************************************/
/********** FONCTION PUBLIC-ACSE  mcs_initialize() *****************/
/*******************************************************************/
MCS_REF *mcs_initialize ( MCS_CONFIGURATION    * Parameters,
                          _MCS_ERROR_CODE_LIST * Report)
{
/*
Cette fonction cree un contexte de fonctionnement de mcs,
associe a un contexte de FIP DEVICE MANAGER ie a un FULLFIP2
Actions:
   -controle les parametres de fonctionnement choisis
   -memorise ces parametres ds la structure privee MCS_REF qui aura
   ete allouee au prealable dans la ZAD
   -met a jour un compte-rendu de service
   -renvoie le pointeur sur MCS_REF, ou NULL quand probleme

*/

    MCS_REF *P;
    /* verification des typedef, en langage C, de FDM et MCS pour ce
    qui concerne les blocs de donnees en reception venant du reseau */

   ListOfMCSs.Nbr_Of_Elt=0;
   ListOfMCSs.Head_Ptr=NULL;
   ListOfMCSs.Queue_Ptr=NULL;
   ListOfMCSs.ref_memory=0;


    if ((Parameters->Extern_Signal_MCS                  == NULL ) ||
        (Parameters->Extern_Signal_Warning              == NULL ) ||
        (Parameters->Extern_Signal_Error                == NULL ) ||
		(Parameters->Extern_Signal_Command_Cnf			== NULL ) ||
		(Parameters->Extern_Signal_ConnexionFailure		== NULL )){
         *Report=_MCS_CREATE_ILLEGAL_PARAMETER;
         return NULL;

   }

   OS_Allocate( Parameters->Memory_Management_Ref ,
                MCS_REF*,
                P ,
                sizeof(MCS_REF) );


   if ( P == (MCS_REF *) 0 ) {
          *Report=_MCS_CREATE_ALLOCATE_MEMORY_FAULT;
          return NULL;
   }

   memset ((char*)P , 0 , sizeof(MCS_REF));


   /* creer le semaphore pour la protection de la base de donnees
   correspondant a cette instance */

   {
       _SEMA_RESULT Cr;

       Cr=OS_BinarySemaphoreCreate( & (P->Semaphore), NULL);

       if (Cr != _SemaOK ) {
         *Report=_MCS_CREATE_CREATE_SEMAPHORE_FAULT;
          OS_Free (Parameters->Memory_Management_Ref,P);
          return NULL;
      }
   }

   /* memoriser les parametres utilisateurs */
   P->Fdm_Ref                           = Parameters->Fdm_Ref;
   P->Extern_Signal_MCS                 = Parameters->Extern_Signal_MCS;
   P->Extern_Signal_Error               = Parameters->Extern_Signal_Error;
   P->Extern_Signal_Warning             = Parameters->Extern_Signal_Warning;
   P->Extern_Signal_Command_Cnf			= Parameters->Extern_Signal_Command_Cnf;
   P->Extern_Signal_ConnexionFailure	= Parameters->Extern_Signal_ConnexionFailure;
   P->Memory_Management_Ref             = Parameters->Memory_Management_Ref;
   memcpy ( P->Name,
            Parameters->Name,
            _name_max_length);

   /*creer les taches logiciel */
   {
	SOFT_REFERENCE_TYPE 	*P_Soft = &(P->Soft) ;
	TASK_REFERENCE_TYPE 	*P_Task = &(P->Task) ;
    	SOFT_INTERFACE_TYPE 	 SoftInterface;

        /* associer chaque file a sa procedure de traitement */
     SoftInterface.Array_Of_In_Functions[_CommandList].Software_Treatment_Fct
 		= FileOut_CommandElement;

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
        SoftInterface.Array_Of_In_Functions[_SendMCSList].Software_Treatment_Fct
 		= FileOut_SendMCSElement;

    SoftInterface.Array_Of_In_Functions[_SendDLLCnfList].Software_Treatment_Fct
		= FileOut_SendDLLCnfElement;

     SoftInterface.Array_Of_In_Functions[_TimeOutList ].Software_Treatment_Fct
		= FileOut_TimeOutElement;

    SoftInterface.Array_Of_In_Functions[_FreeBufferList ].Software_Treatment_Fct
		= FileOut_FreeBufferElement;
#endif

                 SoftInterface.Array_Of_In_Functions[_ReceiveDLLList].Software_Treatment_Fct =
             FileOut_ReceiveDLLIndElement;


        CREATE_TASK_ENVIRONNEMENT (P_Task ,
                                  (void (*)(void*)) (Parameters->Extern_Signal_MCS),
                                  (void *) P);

    	SoftInterface.Nr_Of_External_In_Files 	= _MCS_INTEG_FILE_NUMBER;
 /*       SoftInterface.Software_Loop_Fct         = SLOW_SOFT_LOOP_WITH_TURN_ON_FILES; */
        SoftInterface.Software_Loop_Fct         = SOFT_LOOP;

        ATTACH_SOFT_TO_TASK( P_Task ,
                             &SoftInterface ,
			     P_Soft,
			     P->List);


   }


   MCS_BEGIN_CRITICAL(P);
   /* memoriser la cle */
   P->CheckNumber            =_rangdom_number;
   /*ajouter dans la liste des contextes mcs */
   STORE_FIFO_INT( &ListOfMCSs ,(FIFO_INT_ELT_TYPE *) P);
   MCS_END_CRITICAL(P);

	*Report=_MCS_OK;
   return P;


}


/*******************************************************************/
/********** FONCTION PUBLIC-ACSE  mcs_initialize() *****************/
/*******************************************************************/
void mcs_free ( MCS_REF *ref)
{
/*Cette fonction libere un contexte de fonctionnement de mcs, */

	MEMORY_RN       *Memory_Management_Ref;
	Memory_Management_Ref=ref->Memory_Management_Ref;
	OS_SemaphoreDelete( ref->Semaphore );
    OS_Free (Memory_Management_Ref,ref);

}

/*******************************************************************/
/********** FONCTION PRIVATE proc_msg_no_ack() ********************/
/*******************************************************************/
/* modif  09.05.2000
procedure de messagerie non acquittee :
dans tous les cas retourne non aquittement*/
#if (_STACK_WITH_WindowsNT != _MCS_OPTION_YES)
enum _FDM_MSG_ACK_TYPE proc_msg_no_ack(struct _FDM_REF *Ref,
									   unsigned long remote_ack)
{
	return _FDM_MSG_ACK_TYPE_SDN;
}
#endif

/*******************************************************************/
/********** FONCTION PRIVATE mcs_create_channel() ********************/
/*******************************************************************/

MCSCHANNEL_REF*  McsChannelCreate (MCS_REF * Mcs ,
								   MCS_CHANNEL_CONFIGURATION *In,
								   _MCS_ERROR_CODE_LIST *Report)
{

/*
Cette fonction cree un contexte de fonctionnement pour un canal mcs,
Actions:
   -controle les parametres de fonctionnement choisis
   -memorise ces parametres ds la structure privee MCSCHANNEL_REF qui aura
   ete allouee au prealable dans la ZAD
   -met a jour un compte-rendu de service, ainsi que le pointeur de retour
   -renvoie le pointeur sur MCSCHANNEL_REF, ou NULL quand probleme

   -dans tous les cas, seul le  compte-rendus Out->Report est positionnes
   par CETTE PROCEDURE.


*/
	_MCS_ERROR_CODE_LIST    cr;
	MCSCHANNEL_REF		   *P;
    int              Emit  = _MCS_NO;
    int              Recep = _MCS_NO;
	cr=_MCS_OK;


    /* controler les parametres du canal */
    {
		/* controler UserChannelRef*/
		if (In-> UserChannelRef==NULL){
			cr=_MCS_CHANNEL_CREATE_ILLEGAL_USERCHANNEL_REF;
			goto fin_control_parameters;
		}

		/*controle de la taille de la couche liaison de donnees configuree */
		if (In->DLLSize > _MAX_FIP_SIZE  ) {
			cr=_MCS_CHANNEL_CREATE_ILLEGAL_DLL_SIZE;
			goto fin_control_parameters;
		}

         /* controle des types */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_NO )
         if (In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES ) {
            cr=_MCS_CHANNEL_CREATE_ILLEGAL_SERVICE_TYPE;
			goto fin_control_parameters;
		}
#endif

         if (In->ServType==_CHANNEL_FOR_UNCONNECTED_SERVICES ) {
             if (In->SDUType==_CHANNEL_FOR_MULTIFRAME_SDU ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_SDU_TYPE;
 				goto fin_control_parameters;
           }
         }

/* verifier que si le mode 'bourrage' est demande, alors le canal est forcement du type
  connecte */
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
         if ((In->ServType==_CHANNEL_FOR_UNCONNECTED_SERVICES )&&
              (In->StuffingMode==_MCS_WITH_STUFFING_MODE) ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_STUFFING_MODE;
 				goto fin_control_parameters;
         }
#endif

        /* controle des adresses liaisons, cas connecte*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
         if (In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES ) {
           /* l'adresse locale et l'adresse remote doivent etre toutes 2 du type
                individuelles, non specifiees,  non bridge */

             if ((In->Local_DLL_Address & 0xFF000000 )!=0 ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_LOCAL_ADDRESS;
 				goto fin_control_parameters;

             }
             if ((In->Remote_DLL_Address & 0xFF000000) !=0 ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_REMOTE_ADDRESS;
 				goto fin_control_parameters;

             }
             if (  (FipAddressType (In->Local_DLL_Address) )== _GROUP_ADDR ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_LOCAL_ADDRESS;
 	 			goto fin_control_parameters;

             }
             if ((FipAddressType (In->Remote_DLL_Address)) == _GROUP_ADDR ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_REMOTE_ADDRESS;
 	 			goto fin_control_parameters;
             }
        }
#endif
        if (In->Channel_Nr >8 ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_CHANNEL_NR;
 				goto fin_control_parameters;

         }


        /* controle des procedures de rappel*/
        if ((In->CommType == _CHANNEL_FOR_TRANSM_AND_RECEP  ) ||
            (In->CommType == _CHANNEL_FOR_TRANSM  )           ){
            Emit=_MCS_YES;
            if (In->Send_infos.User_Msg_Ack == NULL ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_CALL_BACK_FUNCTION;
   	         	goto fin_control_parameters;
             }
        }

        if ((In->CommType == _CHANNEL_FOR_TRANSM_AND_RECEP  ) ||
            (In->CommType == _CHANNEL_FOR_RECEP  )           ){
            Recep=_MCS_YES;
            if (In->Receive_infos.User_Msg_Rec_Proc == NULL ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_CALL_BACK_FUNCTION;
				goto fin_control_parameters;             }
        }

        /* controle des adresses liaisons, cas non connecte*/
         if (In->ServType==_CHANNEL_FOR_UNCONNECTED_SERVICES ) {
            /* l'adresse remote est non definie.
               l'adresse locale est de type non specifiee, non bridge
               elle peut etre individuelle ou de groupe. Si le canal est de type
	       Emission, l'adresse locale ne peut pas etre de groupe*/

             if (In->Remote_DLL_Address !=_MCS_UNIVERS_ADDRESS ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_REMOTE_ADDRESS;
   				goto fin_control_parameters;
             }

             if (In->Local_DLL_Address & 0xFF000000 !=0 ) {
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_LOCAL_ADDRESS;
   				goto fin_control_parameters;
             }

             if (Emit==_MCS_YES) {
                if (FipAddressType (In->Local_DLL_Address) == _GROUP_ADDR ) {
                    cr=_MCS_CHANNEL_CREATE_ILLEGAL_LOCAL_ADDRESS;
   	               	goto fin_control_parameters;
                }
             }
         }


        /* controle des taux d'anticipation */


#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
        if (In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES ) {
            if (Emit==_MCS_YES) {
                if ((In->Send_infos.Anticipation == 0 )||
                    (IsAPowerOfTwo( (short) (In->Send_infos.Anticipation))==_MCS_NO)
                                                      ){
                    cr=_MCS_CHANNEL_CREATE_ILLEGAL_ANTICIPATION;
   	              	goto fin_control_parameters;
                }
            }
            if (Recep==_MCS_YES) {
                if ((In->Receive_infos.Anticipation == 0 )||
                    (IsAPowerOfTwo( (short) (In->Receive_infos.Anticipation))==_MCS_NO)
                                                          ){
                    cr=_MCS_CHANNEL_CREATE_ILLEGAL_ANTICIPATION;
   	              	goto fin_control_parameters;
                }
            }
        }
#endif

        /* controle du nombre max de services en //, tous les cas */
        if ((In->MaxParallelServices > _MAX_CONNECTED_PARALLEL_SERVICE)||
            (In->MaxParallelServices==0)                               ){
               cr=_MCS_CHANNEL_CREATE_ILLEGAL_NUMBER_OF_PARALLEL_SERVICES;
   	         goto fin_control_parameters;
        }

        /* controle de SDU_Size, cas connecte*/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
        if (In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES ) {
            if ((In->MaxSDUSize > _MAX_CONNECTED_SDU_SIZE)||
                (In->MaxSDUSize==0)                       ){
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_SDU_SIZE;
   	              	goto fin_control_parameters;
             }
        }
#endif


        /* controle de SDU_Size, cas non connecte*/
        if (In->ServType==_CHANNEL_FOR_UNCONNECTED_SERVICES ) {
            if ((In->MaxSDUSize > (In->DLLSize - _MAX_UNCONNECTED_HEADER ))||
                (In->MaxSDUSize==0)                                        ){
                cr=_MCS_CHANNEL_CREATE_ILLEGAL_SDU_SIZE;
   	          	goto fin_control_parameters;
             }
        }

fin_control_parameters:
		if (cr!=_MCS_OK){
			*Report=cr;
/*			Mcs->Extern_Signal_Warning (_MCS_TYPE_MCS_CHANNEL_CONFIGURATION,
										(void *)In, cr ) ;
*/
			return 	(MCSCHANNEL_REF *) 0;

		}
    }/* fin des controles de parameters */
	 /*---------------------------------*/


    /* allocations memoire */
	/*---------------------*/

    OS_Allocate( Mcs->Memory_Management_Ref ,
                MCSCHANNEL_REF*,
                P ,
                sizeof(MCSCHANNEL_REF) );


    if ( P == (MCSCHANNEL_REF *) 0 ) {
         cr=_MCS_CHANNEL_CREATE_ALLOCATE_MEMORY_FAULT;
   		 goto fin_alloc_memory;
   }
   memset ((char*)P , 0 , sizeof(MCSCHANNEL_REF));



#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

   /* allocation de la fenetre d'emission */
   if ((Emit==_MCS_YES) && ( In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES)) {

        OS_Allocate( Mcs->Memory_Management_Ref ,
                TRANSMISSION_WINDOW_TYPE *,
                P->TransmitWindowArea ,
                In->Send_infos.Anticipation * 2* sizeof(TRANSMISSION_WINDOW_ELEMENT_TYPE) );

        if ( P->TransmitWindowArea == NULL ) {
            OS_Free (Mcs->Memory_Management_Ref,P);
            cr=_MCS_CHANNEL_CREATE_ALLOCATE_MEMORY_FAULT;
   			goto fin_alloc_memory;
        } else {
            memset ((char*)P->TransmitWindowArea , 0 ,
            In->Send_infos.Anticipation*2 * sizeof(TRANSMISSION_WINDOW_ELEMENT_TYPE));
        }
   }

   /* allocation de la fenetre de reception */

   if ((Recep==_MCS_YES) && ( In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES)) {

        OS_Allocate( Mcs->Memory_Management_Ref ,
                RECEPTION_WINDOW_TYPE *,
                P->RecepWindowArea ,
                In->Receive_infos.Anticipation * sizeof(RECEPTION_WINDOW_ELEMENT_TYPE) );

         if ( P->RecepWindowArea == NULL ) {
              if (P->RecepWindowArea != NULL ) {
                  OS_Free (Mcs->Memory_Management_Ref,P->RecepWindowArea );
              }
              OS_Free (Mcs->Memory_Management_Ref,P);
              cr=_MCS_CHANNEL_CREATE_ALLOCATE_MEMORY_FAULT;
   	          goto fin_alloc_memory;

          } else {
              memset ((char*)P->RecepWindowArea , 0 ,
              In->Receive_infos.Anticipation * sizeof(RECEPTION_WINDOW_ELEMENT_TYPE));
          }
   }

#endif
fin_alloc_memory:
		if (cr!=_MCS_OK){
			*Report=cr;
/*
			Mcs->Extern_Signal_Warning (_MCS_TYPE_MCS_CHANNEL_CONFIGURATION,
										(void *)In, cr ) ;
*/
			return 	(MCSCHANNEL_REF *) 0;

		}
/* fin_alloc_memory */
/*------------------*/

     /*creer le contexte de messagerie pour le canal*/
	 /*---------------------------------------------*/

     /* les caracteristiques du contexte dependent du type
     de canal cree ie:
         1: canal connecte                           => Lsap E+R
         2: canal non connecte E+R                   => Lsap E+R
         4: canal non connecte E                     => Lsap E
         6: canal non connecte R                     => Lsap R

     Le nombre de ressources en reception depend des caracteristiques
     du canal cree.
     Pour eviter que l'integrateur se pose des questions sur le choix du nombre
     de ressources, on fixe ce nombre, en fonction des caracteristiques.
     Les formules utilisees peuvent etre modifiees ulterieurement:



          CANAL CONNECTE EMISSION:
             On recoit les acquittements, dont on recycle les ressources
             immediatement.
                          NbDesc necess = Anticipation_E
                          NbData necess = Anticipation_E

          CANAL CONNECTE RECEPTION:
             On recoit les PDUs, que l'on garde jusqu'a constituer un SDU, que
             l'utilisateur recyclera un peu plus tard. Ces PDUS peuvent etre
             reemis par l'emetteur.

             Le nombre maximum de messages recus chez l'utilisateur est borne
             par 'NbreDeServicesEnParallele'.

             Et le canal MCS emetteur peut reemettre les PDUs de la fenetre
	     d'anticipation.

  	     On recoit aussi des A_RELEASE et des A_ABORT.
             Soit 'NbPDUsParSDU', le nombre max de PDUs qui constituent un
             message recu.


                   NbData necess = 2 +
                                   NbPDUsParSDU x NbreDeServicesEnParallele +
	                           Anticipation_R.

                   NbDesc necess = Anticipation_R x 2
		                 car les descripteurs sont recycles rapidement.

           CANAL NON CONNECTE EMISSION (donc sans ACK) :
              On ne recoit rien

           CANAL NON CONNECTE RECEPTION:
             On recoit 'NbreDeServicesEnParallele'.

                          NbData necess = NbreDeServicesEnParallele
                          NbDesc necess = NbData necess

            */

   {

     FDM_MESSAGING_REF *      Lsap;

     unsigned int NbDesc=0;
     unsigned int NbData=0;


#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

     if (In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES ) {

         if (Emit==_MCS_YES) {
            NbData=In->Send_infos.Anticipation;
            NbDesc=NbData;
         }

         if (Recep==_MCS_YES) {
            NbData+=(In->Receive_infos.Anticipation*2)
                   +(((In->MaxSDUSize/(In->DLLSize-_MAX_CONNECTED_HEADER)) +1)
				   *In->MaxParallelServices) ;
            NbDesc+=2*In->Receive_infos.Anticipation;
         }
    }

#endif

   if (In->ServType==_CHANNEL_FOR_UNCONNECTED_SERVICES ) {
         if (Recep==_MCS_YES) {
            NbData=In->MaxParallelServices;
            NbDesc=NbData;
         }
   }


   if ( ( (Emit==_MCS_YES) && (Recep==_MCS_YES) )                    ||
        (In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES ) ){

        FDM_MESSAGING_FULLDUPLEX Par;

        memset ((char*)&Par , 0 , sizeof(FDM_MESSAGING_FULLDUPLEX));

        switch (In->Position) {
             case _MCS_MSG_IMAGE_1:
             Par.Position=_FDM_MSG_IMAGE_1;
             break;

             case _MCS_MSG_IMAGE_2:
             Par.Position=_FDM_MSG_IMAGE_2;
             break;

             case _MCS_MSG_IMAGE_1_AND_2:
             Par.Position=_FDM_MSG_IMAGE_1_AND_2;
             break;
        }

        Par.sending.User_Msg_Ack        =MCSDllConfirmationFunction;
        Par.sending.User_Qid            =(void *) P;
        Par.sending.Channel_Nr          =In->Channel_Nr;

        Par.receiving.User_Msg_Rec_Proc =MCSDllIndicationFunction;
        Par.receiving.User_Qid          =(void *) P;

        Par.receiving.Number_Of_Msg_Desc   = NbDesc;
        Par.receiving.Number_Of_Msg_Block  = NbData;

        Par.Local_DLL_Address           =In->Local_DLL_Address;
        Par.Remote_DLL_Address          =In->Remote_DLL_Address;

/* modif  09.05.2000 : messagerie normalise ou sans acquit:
	pour option _STACK_WITH_WindowsNT:
		Remote_DLL_Address  bit 24 e 25 =0 :-> normalise
						    bit 24 e 25 =2:	-> non acquitee
	si pas option _STACK_WITH_WindowsNT:appel de la fonction
		fdm_change_messaging_acknowledge_type
*/
#if (_STACK_WITH_WindowsNT == _MCS_OPTION_YES)
		if (In->msg_mode_ack ==_MCS_MSG_MODE_NOACK){
		Par.Remote_DLL_Address  |=0x2000000;
		}
#endif
        Lsap=fdm_messaging_fullduplex_create(Mcs->Fdm_Ref, &Par);

        if (Lsap == NULL ) {
            cr=_MCS_CHANNEL_CREATE_ILLEGAL_REFUSED_DLL_CTXT_CREATION;
            goto abort;
       }
#if (_STACK_WITH_WindowsNT != _MCS_OPTION_YES)
		if (In->msg_mode_ack ==_MCS_MSG_MODE_NOACK){
			fdm_change_messaging_acknowledge_type(Lsap,proc_msg_no_ack);
		}
#endif
/*fin  modif  09.05.2000 : messagerie normalise ou sans acquit*/

    } else if (Emit==_MCS_YES)  {
            /* seulement emission et non connecte */

        FDM_MESSAGING_TO_SEND Par;
        memset ((char*)&Par , 0 , sizeof(FDM_MESSAGING_TO_SEND));

        switch (In->Position) {
             case _MCS_MSG_IMAGE_1:
             Par.Position=_FDM_MSG_IMAGE_1;
             break;
             case _MCS_MSG_IMAGE_2:
             Par.Position=_FDM_MSG_IMAGE_2;
             break;
             case _MCS_MSG_IMAGE_1_AND_2:
             Par.Position=_FDM_MSG_IMAGE_1_AND_2;
             break;
        }

        Par.sending.User_Msg_Ack        =MCSDllConfirmationFunction;
        Par.sending.User_Qid            =(void *) P;
        Par.sending.Channel_Nr          =In->Channel_Nr;

        Par.Local_DLL_Address           =In->Local_DLL_Address;
        Par.Remote_DLL_Address          =In->Remote_DLL_Address;

/*modif 09.05.2000 messagerie normalise ou sans acquit*/
#if (_STACK_WITH_WindowsNT == _MCS_OPTION_YES)
		if (In->msg_mode_ack ==_MCS_MSG_MODE_NOACK){
		Par.Remote_DLL_Address  |=0x2000000;
		}
#endif
        Lsap=fdm_messaging_to_send_create(Mcs->Fdm_Ref, &Par);

        if (Lsap == NULL ) {
            cr=_MCS_CHANNEL_CREATE_ILLEGAL_REFUSED_DLL_CTXT_CREATION;
            goto abort;
        }
#if (_STACK_WITH_WindowsNT != _MCS_OPTION_YES)
		if (In->msg_mode_ack ==_MCS_MSG_MODE_NOACK){
			fdm_change_messaging_acknowledge_type(Lsap,proc_msg_no_ack);
		}
#endif
/*fin  modif  09.05.2000 : messagerie normalise ou sans acquit*/

    } else if (Recep==_MCS_YES)  {
            /* seulement reception et non connecte */

        FDM_MESSAGING_TO_REC Par;
        memset ((char*)&Par , 0 , sizeof(FDM_MESSAGING_TO_REC));

        switch (In->Position) {
             case _MCS_MSG_IMAGE_1:
             Par.Position=_FDM_MSG_IMAGE_1;
             break;
             case _MCS_MSG_IMAGE_2:
             Par.Position=_FDM_MSG_IMAGE_2;
             break;
             case _MCS_MSG_IMAGE_1_AND_2:
             Par.Position=_FDM_MSG_IMAGE_1_AND_2;
             break;
        }

        Par.receiving.User_Msg_Rec_Proc =MCSDllIndicationFunction;
        Par.receiving.User_Qid          =(void *) P;

        Par.receiving.Number_Of_Msg_Desc   = NbDesc;
        Par.receiving.Number_Of_Msg_Block  = NbData;

        Par.Local_DLL_Address           =In->Local_DLL_Address;
        Par.Remote_DLL_Address          =In->Remote_DLL_Address;

        Lsap=fdm_messaging_to_rec_create(Mcs->Fdm_Ref, &Par);

        if (Lsap == NULL ) {
            cr=_MCS_CHANNEL_CREATE_ILLEGAL_REFUSED_DLL_CTXT_CREATION;
             goto abort;
        }

    } else {
         /* impossible */
         cr=_MCS_INTERNAL_ERROR;
         goto abort;
    }

    P->Lsap_Ctxt=Lsap;
  } /* fin creation LSAP */


   /*creer les pools memoire */

   /* pool memoire pour les indications utilisateurs
      on en prend le nombre de services en parallele*/
   if (Recep==_MCS_YES) {

       P->PoolForUserIndicationRessources =
           _NEW_USER (
                Mcs->Memory_Management_Ref ,
                sizeof (MCS_SDU_RECEIVED),
                In->MaxParallelServices );

       if (P->PoolForUserIndicationRessources ==NULL) {
			cr =_MCS_CHANNEL_CREATE_ALLOCATE_MEMORY_FAULT;
            goto abort;
	   }
   }

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
    /* pool memoire pour les acquittements de PDU a emettre
      on en prend 2 x le taux d'anticipation*/

   if ((Recep==_MCS_YES)&&(In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES)) {

       P->PoolForAckRessources =
           _NEW_USER (
                Mcs->Memory_Management_Ref ,
                sizeof (MCS_ACK_MSG_TO_SEND),
                2 * In->Receive_infos.Anticipation );

       if (P->PoolForAckRessources ==NULL) {
		   	cr =_MCS_CHANNEL_CREATE_ALLOCATE_MEMORY_FAULT;
            goto abort;
	   }
       /* on fait l'allocation, le preremplissage, et le stockage dans
       la file APRES le creation du LSAP, pour etre sur que celui -ci existe*/

   }
#endif


   /* memoriser les parametres utilisateurs */
   P->ServType               = In->ServType;
   P->CommType               = In->CommType;
   if (In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES) {
      P->AckType                = _CHANNEL_WITH_PDU_ACK;
   } else {
      P->AckType                = _CHANNEL_WITHOUT_PDU_ACK;
   }
   P->SDUType                = In->SDUType;
   P->Local_DLL_Address      = In->Local_DLL_Address;
   P->Remote_DLL_Address     = In->Remote_DLL_Address;
   P->MaxSDUSize             = In->MaxSDUSize;
   P->DLLSize                = In->DLLSize;
   P->MaxParallelServices    = In->MaxParallelServices;

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
   P->T_Anticipation         = In->Send_infos.Anticipation;
   P->T_NbOfRetry            = In->Send_infos.NbOfRetry;

   P->ConnexionTempoTicks    = In->ConnexionTempoTicks;
   P->StuffingMode           = In->StuffingMode;
   P->StuffingTempoTicks     = _STUFFINGTOCONNEXIONTIMEFACTOR *
								 In->ConnexionTempoTicks;
   P->TransmWatchDogTempoTicks= _WDOGTOSTUFFINGTIMEFACTOR *
								 P->StuffingTempoTicks;
   P->RecepWatchDogTempoTicks = P->TransmWatchDogTempoTicks;

   P->R_Anticipation         = In->Receive_infos.Anticipation;
   P->R_NbOfRetry            = In->Receive_infos.NbOfRetry;
   P->User_Msg_Ack           = In->Send_infos.User_Msg_Ack;
#endif

   P->FIP_Channel_Nr         = In->Channel_Nr;
   P->User_Msg_Rec_Proc      = In->Receive_infos.User_Msg_Rec_Proc;
   P->Position               = In->Position;

   memcpy ( P->Name,  In->Name,_name_max_length);

   P->UserChannelRef         = In->UserChannelRef;
   P->Mcs                    = Mcs;



#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
/* allocation, preremplissage, et stockage dans la file des ressources
utilisees pour envoyer l'acquittement des PDU*/

   if (P->PoolForAckRessources != NULL) {

       MCS_ACK_MSG_TO_SEND * R;
       for (;;) {
           R=(MCS_ACK_MSG_TO_SEND *)
              _ALLOCATE_MEMORY (P->PoolForAckRessources);
           if (R==NULL) break;
           LoadDefaultParamInAcknowledgeRessource(R,P);
           STORE_FIFO_INT(&(P->AckRessourcesList),
						   (FIFO_INT_ELT_TYPE * ) R );
       }
    }
#endif


#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

   /*conditions initiales en segmentation */
   P->Segmentation.State = _EmptyMachine;
   P->SDUPurgeState      = _Transmitting;

   /*conditions initiales en assemblage */
   P->Assembling.State   = _EmptyMachine;
   P->BrokenSequence     = NO;

   /*conditions initiales en bourrage */
   P->IdleTransmPDUInProgress = _MCS_NO;

   /*initialiser les descripteurs de temporisations du canal*/
   if (P->StuffingMode == _MCS_WITH_STUFFING_MODE  ) {
       CreateStuffingTempo(P);
       CreateTransmWatchdogTempo(P);
       CreateRecepWatchdogTempo(P);
   }

   /*initialiser la fenetre d'anticipation en emission*/
   if ((Emit==_MCS_YES) && ( In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES)) {
       LoadDefaultParamInTransmWindow(P);
   }

   /*initialiser la fentre d'anticipation en reception*/

   if ((Recep==_MCS_YES) && ( In->ServType==_CHANNEL_FOR_CONNECTED_SERVICES)) {
       LoadDefaultParamInRecepWindow(P);
   }

   /*initialiser la ressource servant au bourrage dans le canal A_DATA_Emission*/
   if (P->StuffingMode == _MCS_WITH_STUFFING_MODE  ) {

     P->StuffingSDUToSend.Channel.McsChannelRef   =P;
     P->StuffingSDUToSend.Nr_Of_Blocks           =0;
     P->StuffingSDUToSend.Private_MCS.Stuffing   =_MCS_YES;
   }


#endif




/*   tout est  OK	*/
   /* memoriser la cle */
   P->CheckNumber            =_rangdom_number;

   /*ajouter dans la liste des canaux du contexte mcs */
   STORE_FIFO_INT( &Mcs->ListOfChannels ,(FIFO_INT_ELT_TYPE *) P);
  *Report				     =_MCS_OK;
   return P;



	/* abort :Dans les cas de rejet: revenir propre... */
   /*--------------------------------------------------*/
abort:

   if (P->Lsap_Ctxt != NULL) {
       fdm_messaging_delete(P->Lsap_Ctxt);
   }
   if (P->PoolForUserIndicationRessources != NULL ) {
        _DELETE_USER (P->PoolForUserIndicationRessources);
   }

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
   if (P->PoolForAckRessources != NULL ) {
        _DELETE_USER (P->PoolForAckRessources);
   }
#endif

   memset ((char*)P , 0 , sizeof(MCSCHANNEL_REF));
   OS_Free(Mcs->Memory_Management_Ref,P);

   *Report=cr;
/*
	Mcs->Extern_Signal_Warning (_MCS_TYPE_MCS_CHANNEL_CONFIGURATION,
										(void *)In, cr ) ;
*/
   return (MCSCHANNEL_REF*)0;


}


/*******************************************************************/
/********** FONCTION PUBLIC mcs_channel_start() ********************/
/*******************************************************************/
/* retoune cr =_MCS_OK si tout ok
			cr =_MCS_NOK si canal n'est pas dans etat attente start (par ex deja running)
			cr = _MCS_CHANNEL_START_ON_INEXISTING_CHANNEL
*/
_MCS_ERROR_CODE_LIST mcs_channel_start  (ALL_MCSCHANNEL_REFS   channel )
{
	_MCS_ERROR_CODE_LIST cr;
	MCSCHANNEL_REF *channel_ref;
	channel_ref=channel.McsChannelRef;
	if (channel_ref->CheckNumber !=_rangdom_number){
		cr=_MCS_CHANNEL_START_ON_INEXISTING_CHANNEL;
		return cr;
	}
	if (channel_ref->State== _MCS_CHANNEL_STATE_IDLE_TO_START){

#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

        /*demarrer les temporisations du canal*/
        if (channel_ref->StuffingMode == _MCS_WITH_STUFFING_MODE  ) {
            StartStuffingTempo(channel_ref);
            StartTransmWatchdogTempo(channel_ref);
            StartRecepWatchdogTempo(channel_ref);

         }
#endif
	   channel.McsChannelRef->State= _MCS_CHANNEL_STATE_RUNNING;
	   cr=_MCS_OK;
	}else {
	  cr=_MCS_NOK;
	}

	return cr;
}

/*******************************************************************/
/********* FONCTION PRIVATE-OUTILS  IsAPowerOfTwo() ****************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )

int IsAPowerOfTwo(short X )
{
/* Procedure qui test si le nombre est une puissance de 2 ou non */
/* on verifie que parmi les bits b0..b14 de X, un seul est a 1...*/

    unsigned char i;
    unsigned char Nb_bits=0;
    short         Y;

	for (i=0; i<=14; i++) {
            Y= X >> i;
            if ((Y & 0x0001) != 0 ) {
                Nb_bits++;
            }

        }

        if (Nb_bits ==1) return _MCS_YES;
        else             return _MCS_NO;

}
#endif

/*******************************************************************/
/***** FONCTION PRIVATE LoadDefaultParamInTransmWindow() ***********/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoadDefaultParamInTransmWindow (MCSCHANNEL_REF * Channel)
{
/* cette fonction preremplit les parametres des ressources memoire
qui constituent la fenetre d'anticipation en Emission.
Ces ressources sont constituees de:
	-l'etat
	-le type de PDU (preremplissage utile dans le cas monotrame)
	-le nombre max de repetitions
 	-une ressource de type MCS_FDM_MSG_TO_SEND, contenant
        un descripteur de service FDM, avec sont extension,
        une zone de 4 octets de donnees
 	-une ressource de type DESC_BLOCK_DATA_T contenant
        un (en fait 2) micro descripteur de donnees
 	-une ressource de type MCS_A_DATA_TEMPO_TYPE, permettant
        de demander une temporisation sur le transfert d'un PDU
*/


     int            i;
     unsigned short Size=_PDUMakeAnticipation(Channel);;



     for (i=0; i<Size ; i ++ ) {
         TRANSMISSION_WINDOW_ELEMENT_TYPE * Box =
              &(Channel->TransmitWindowArea->Box[i]);

         Box->State         = _Free_Box;
         Box->Type          = __HEAD_NIL;
         Box->SendCredit    = 0;
         Box->PurgeState    = _Transmitting;
         Box->Stuffing      = _MCS_NO;


        {
            MCS_FDM_MSG_TO_SEND *A= &(Box->AreaToSend);

            A->FdmToSend.Ptr_Block   = (FDM_MSG_T_DESC*)
              &((Box->BlockToSend).User_Block);
            A->Channel               = Channel;
            A->Type                  = _MCS_CONNECTED_DATA_TRANSFER_FOR_PDU;
            A->Ctxt                  = (void *) Box;
            A->Data[0]               = 0x03;
            A->Data[1]               = __HEAD_NIL;
        }

        {
            DATA_BLOCK_TO_TRANSFER *B_Header= &(Box->BlockToSend).User_Block;

            B_Header->Nr_Of_Bytes          = 4;
            B_Header->Ptr_Data             = &((Box->AreaToSend).Data[0]);

        }

     }


    /* initialisation descripteurs de tempo*/
    CreatePDU_A_DATA_Tempo(Channel);
}

#endif


/*******************************************************************/
/***** FONCTION PRIVATE LoadDefaultParamInRecepWindow() ************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoadDefaultParamInRecepWindow (MCSCHANNEL_REF * Channel)
{
/* cette fonction preremplit les parametres des ressources memoire
qui constituent la fenetre d'anticipation en Reception.
Ces ressources sont constituees de:
	-l'etat
	-le type de PDU (preremplissage utile dans le cas monotrame)
	-le nombre max de repetitions
 	-une ressource de type MCS_FDM_MSG_TO_SEND, contenant
        un descripteur de service FDM, avec son extension,
        une zone de 4 octets de donnees
 	-une ressource de type DESC_BLOCK_DATA_T contenant
        un (en fait 2) micro descripteur de donnees
 	-une ressource de type MCS_A_DATA_TEMPO_TYPE, permettant
        de demander une temporisation sur le transfert d'un PDU
*/

     int            i;
     unsigned short Size=Channel->R_Anticipation;


     for (i=0; i<Size ; i ++ ) {
        RECEPTION_WINDOW_ELEMENT_TYPE * Box =
              &(Channel->RecepWindowArea->Box[i]);

         Box->State         = _Free_Box;
         Box->UserState     = _NotYetTakenIntoAccount;

     }
}

#endif


/*******************************************************************/
/***** FONCTION PRIVATE LoadDefaultParamInAcknowledgeRessource()****/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void LoadDefaultParamInAcknowledgeRessource (MCS_ACK_MSG_TO_SEND * Ressource, MCSCHANNEL_REF * Channel)
{
/*procedure qui preremplit les ressources qui sont utiliser pour
produire l'acquittement, en reception */

     MCS_FDM_MSG_TO_SEND * McsFdmToSend = &(Ressource->McsFdmToSend);
     FDM_MSG_TO_SEND     * FdmToSend    = &(McsFdmToSend->FdmToSend);
     FDM_MSG_T_DESC      * Desc         = &(Ressource->Desc);

     FdmToSend->Nr_Of_Blocks = 1;
     FdmToSend->Ptr_Block    = Desc;

     Desc->Nr_Of_Bytes        = 4;
     Desc->Ptr_Data           = McsFdmToSend->Data;

     McsFdmToSend->Channel    = Channel;

     McsFdmToSend->Type       = _MCS_CONNECTED_DATA_TRANSFER_FOR_ACK;
     McsFdmToSend->Data[0]    = 0x03;
     McsFdmToSend->Data[1]    = 0x60;
}
#endif
/*******************************************************************/
/***** FONCTION PRIVATE CreateStuffingTempo () *********************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void CreateStuffingTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction prepare le descripteur de temporisation,
servant a gerer la temporisation pour l'envoi d'un PDU de bourrage
dans le cas d'un canal de type connecte emission.

Cette fonction est appelee une fois que le contexte du canal,
reference par 'Channel' contient les parametres concernant cette temporisation,
soit le nombre de ticks necessaires.


La creation consiste a :
 pour chaque descripteur de temporisation:
   -remplir les parametres du descripteur
   -remplir les parametres de son extension

*/

      /*le descripteur elementaire, pour le service de temporisation*/
      TEMPO_DEF     * D = &(Channel->DescTempo_For_Stuffing.Desc);


     D->T_init          = Channel->StuffingTempoTicks;
/*   D->Duree_vie       = 0;
     D->_.Reserved      = 0;
     D->_.GDT_occup     = 0;
     D->_.Choix_mort    = 0;
*/
     D->_.Mode=1; /*non garanti*/
/*
     D->_.Periodique    = 0;
     D->_.Vie_limite    = 0;
*/
     D->_.Type_out      = 1;
/*
     D->_.User_Mode_mort= 0;
*/

     D->File            = &(Channel->Mcs->List[_TimeOutList]);

     Channel->DescTempo_For_Stuffing.Channel = Channel;
     Channel->DescTempo_For_Stuffing.Type    =_MCS_TEMPO_FOR_STUFFING;


}
#endif

/*******************************************************************/
/***** FONCTION PRIVATE StartStuffingTempo () **********************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void StartStuffingTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction lance la temporisation,
servant a gerer la temporisation pour l'envoi d'un PDU de bourrage
dans le cas d'un canal de type connecte emission.


*/
      TEMPO_DEF     * D = &(Channel->DescTempo_For_Stuffing.Desc);

      _TIMER_START(D);
}
#endif

/*******************************************************************/
/***** FONCTION PRIVATE RestartStuffingTempo () ********************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void RestartStuffingTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction relance la temporisation,
servant a gerer la temporisation pour l'envoi d'un PDU de bourrage
dans le cas d'un canal de type connecte emission.

Cette temporisation est mise en file lors de son echeance,
ce qui impose d'utiliser le TIMER_RETRIG_ANYWAY

*/

      TEMPO_DEF     * D = &(Channel->DescTempo_For_Stuffing.Desc);

      _TIMER_RETRIG_ANYWAY(D);
}
#endif

/*******************************************************************/
/***** FONCTION PRIVATE CreateTransmWatchdogTempo () ***************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void CreateTransmWatchdogTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction prepare le descripteur de temporisation,
servant a gerer le controle du flux d'emission des PDUs,
dans le cas d'un canal de type connecte emission.

Cette fonction est appelee une fois que le contexte du canal,
reference par 'Channel' contient les parametres concernant cette temporisation,
soit le nombre de ticks necessaires.


La creation consiste a :
 pour chaque descripteur de temporisation:
   -remplir les parametres du descripteur
   -remplir les parametres de son extension

*/

      /*le descripteur elementaire, pour le service de temporisation*/
      TEMPO_DEF     * D = &(Channel->DescTempo_For_TransmWatchdog.Desc);


     D->T_init          = Channel->TransmWatchDogTempoTicks;
/*   D->Duree_vie       = 0;
     D->_.Reserved      = 0;
     D->_.GDT_occup     = 0;
     D->_.Choix_mort    = 0;
*/
     D->_.Mode=1; /*non garanti*/
/*
     D->_.Periodique    = 0;
     D->_.Vie_limite    = 0;
     D->_.Type_out      = 0;
     D->_.User_Mode_mort= 0;
*/

     D->User_Prog            = InternSign_TransmWatchdogTempo;
     Channel->DescTempo_For_Stuffing.Channel = Channel;


}
#endif

/*******************************************************************/
/***** FONCTION PRIVATE StartTransmWatchdogTempo () ****************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void StartTransmWatchdogTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction lance la temporisation,
servant a gerer la temporisation pour le controle du flux d'emission
de PDU dans le cas d'un canal de type connecte emission.


*/
      TEMPO_DEF     * D = &(Channel->DescTempo_For_TransmWatchdog.Desc);

      _TIMER_START(D);
}
#endif

/*******************************************************************/
/***** FONCTION PRIVATE RestartTransmWatchdogTempo () **************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void RestartTransmWatchdogTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction relance la temporisation,
servant a gerer la temporisation pour le controle du flux d'emission
de PDU dans le cas d'un canal de type connecte emission.

Cette temporisation n'est pas mise en file lors de son echeance,
ce qui impose d'utiliser le TIMER_RETRIG

*/

      TEMPO_DEF     * D = &(Channel->DescTempo_For_TransmWatchdog.Desc);

      _TIMER_RETRIG(D);
}

#endif



/*******************************************************************/
/***** FONCTION PRIVATE CreateRecepWatchdogTempo () ****************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void CreateRecepWatchdogTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction prepare le descripteur de temporisation,
servant a gerer le controle du flux de reception des PDUs,
dans le cas d'un canal de type connecte reception.

Cette fonction est appelee une fois que le contexte du canal,
reference par 'Channel' contient les parametres concernant cette temporisation,
soit le nombre de ticks necessaires.


La creation consiste a :
 pour chaque descripteur de temporisation:
   -remplir les parametres du descripteur
   -remplir les parametres de son extension

*/

      /*le descripteur elementaire, pour le service de temporisation*/
      TEMPO_DEF     * D = &(Channel->DescTempo_For_RecepWatchdog.Desc);


     D->T_init          = Channel->RecepWatchDogTempoTicks;
/*   D->Duree_vie       = 0;
     D->_.Reserved      = 0;
     D->_.GDT_occup     = 0;
     D->_.Choix_mort    = 0;
*/
     D->_.Mode=1; /*non garanti*/
/*
     D->_.Periodique    = 0;
     D->_.Vie_limite    = 0;
     D->_.Type_out      = 0;
     D->_.User_Mode_mort= 0;
*/

     D->User_Prog            = InternSign_RecepWatchdogTempo;
     Channel->DescTempo_For_Stuffing.Channel = Channel;


}
#endif

/*******************************************************************/
/***** FONCTION PRIVATE StartRecepWatchdogTempo () ****************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void StartRecepWatchdogTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction lance la temporisation,
servant a gerer la temporisation pour le controle du flux de reception
des PDUs dans le cas d'un canal de type connecte reception.


*/
      TEMPO_DEF     * D = &(Channel->DescTempo_For_RecepWatchdog.Desc);

      _TIMER_START(D);
}
#endif

/*******************************************************************/
/***** FONCTION PRIVATE RestartTransmWatchdogTempo () **************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void RestartRecepWatchdogTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction relance la temporisation,
servant a gerer la temporisation pour le controle du flux de reception
des PDUs dans le cas d'un canal de type connecte reception.

Cette temporisation n'est pas mise en file lors de son echeance,
ce qui impose d'utiliser le TIMER_RETRIG

*/

      TEMPO_DEF     * D = &(Channel->DescTempo_For_TransmWatchdog.Desc);

      _TIMER_RETRIG(D);
}
#endif




/*******************************************************************/
/***** FONCTION PRIVATE CreatePDU_A_DATA_Tempo () ******************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void CreatePDU_A_DATA_Tempo (MCSCHANNEL_REF * Channel)
{
/* cette fonction prepare les descripteurs de temporisations,
servant a gerer le time-out sur la non reception de l'acquittement
d'un PDU MCS contenant des donnees, dans le cas d'un canal de type
connecte emisssion.

Cette fonction est appelee une fois que le contexte du canal,
reference par 'Channel' contient les parametres concernant cette temporisation,
soit le nombre de ticks necessaires.
Il est necessaire que la fenetre d'anticipation pour le transfert des
messages aient ete crees.
Le taux d'anticipation aura du etre initialise.

Remarque: on a theoriquement besoin d'un nombre de descripteurs de tempo
egal au taux d'anticipation en emission.
Pour simplifier la programmation, on en prendra 2 fois plus, soit un
par case de la fenetre d'anticipation.

La creation consiste a :
 pour chaque descripteur de temporisation:
   -remplir les parametres du descripteur
   -remplir les parametres de son extension

*/

   int i;

   for (i=0; i< _PDUMakeAnticipation(Channel); i++) {

      /*le grand descripteur, specifique A_DATA, et pour cette tempo uniquement*/
      MCS_A_DATA_T_TEMPO_TYPE * D0 =
           &((Channel->TransmitWindowArea->Box[i]).
            TempoDesc);

      /*le moyen descripteur, specifique MCS */
      MCS_TEMPO_TYPE         * D1 = &(D0->Desc);

      /*le descripteur elementaire, pour le service de temporisation*/
      TEMPO_DEF              * D2 = &(D1->Desc);


     D2->T_init          = Channel->ConnexionTempoTicks;
/*   D2->Duree_vie       = 0;
     D2->_.Reserved      = 0;
     D2->_.GDT_occup     = 0;
     D2->_.Choix_mort    = 0;
*/
     D2->_.Mode          = 1; /*non garanti*/
/*   D2->_.Periodique    = 0;
     D2->_.Vie_limite    = 0;
*/
     D2->_.Type_out      = 1;
     D2->_.User_Mode_mort= 1;

     D2->File            = &(Channel->Mcs->List[_TimeOutList]);

     D1->Channel         = Channel;
     D1->Type            =_MCS_TEMPO_FOR_CONNEXION_PDU_ACK_TIMEOUT;

     D0->BoxRef           = &(Channel->TransmitWindowArea->Box[i]);
   }
}
#endif

/*******************************************************************/
/*** FONCTION PRIVATE StartPDU_A_DATA_Tempo() **********************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void StartPDU_A_DATA_Tempo(TRANSMISSION_WINDOW_ELEMENT_TYPE * Box)
{
/* Dans le cas d'un canal connecte, emission, lamcement de la
temporisation qui sert a gerer le time out sur la non reception
de l'acquittement des PDU

*/
      TEMPO_DEF * Tempo = &(Box->TempoDesc.Desc.Desc);

      _TIMER_START(Tempo);
}
#endif

/*******************************************************************/
/*** FONCTION PRIVATE StopPDU_A_DATA_Tempo() ***********************/
/*******************************************************************/
#if (MCS_WITH_CONNECTED_SERVICES == _MCS_OPTION_YES )
void StopPDU_A_DATA_Tempo(TRANSMISSION_WINDOW_ELEMENT_TYPE * Box)
{
/* Dans le cas d'un canal connecte, emission, arret de la
temporisation qui sert a gerer le time out sur la non reception
de l'acquittement des PDU

*/
      TEMPO_DEF * Tempo = &(Box->TempoDesc.Desc.Desc);

      _TIMER_STOP(Tempo);


}
#endif


/*******************************************************************/
/***** FONCTION PRIVATE FipAddressType() ***************************/
/*******************************************************************/
_FIP_ADDRESS_TYPE_LIST FipAddressType ( unsigned long Address )
/* procedure qui donne le type d'une adresse DLL FIP: renvoie Individuel ou Groupe */

{
Ulong  I_G;	/* Individu- Groupe */
Ulong  S_R;	/* Segment reseau   */

	I_G = Address & 0x00800000L;
	S_R = Address & 0x00000080L;
	if (S_R ==0) {
		if ( I_G==0 )
				 return _INDIV_ADDR;
	}
	return _GROUP_ADDR;
}




/*******************************************************************/
/***** FONCTION PRIVATE CheckReceptionStructure() ******************/
/*******************************************************************/
/*_MCS_ERROR_CODE_LIST CheckReceptionStructure(void)
{
 procedure qui verifie que le contenu des 2 types FDM_MSG_R_DESC
et DESC_BLOCK_DATA_R sont identiques.
Cette procedure est appelee au moment de l'initialisation de l'instance.
Si on n'a pas egalite entre ces 2 types, il faut revoir le code de MCS.
Car de nombreux 'cast' sont faits entre ces 2 types
      -retourne _MCS_OK si les 2 types sont identiques
      -retourne _MCS_NOK si les 2 types ne sont pas identiques



     DESC_BLOCK_DATA_R A;
     FDM_MSG_R_DESC    B;

     if ( sizeof (DESC_BLOCK_DATA_R ) != sizeof (FDM_MSG_R_DESC )) {
         return _MCS_NOK;
     }


     A.Nr_Of_Bytes   = 0x3496;
     A.Ptr_Data      = (unsigned char *) 0x12345678;
     A.Next_Block    = (DESC_BLOCK_DATA_R *) 0x58395602;

     B.Nr_Of_Bytes   = 0x3496;
     B.Ptr_Data      = (unsigned char *) 0x12345678;
     B.Next_Block    = (FDM_MSG_R_DESC *) 0x58395602;


     if (memcmp ( (char * )&A,
                  (char * )&B,
                   sizeof (DESC_BLOCK_DATA_R )   ) == 0) {
         return _MCS_OK;
     } else {
         return _MCS_NOK;
     }
}

*/




/*******************************************************************/
/***** FONCTION PRIVATE StopTransmWatchdogTempo () *****************/
/*******************************************************************/
void StopTransmWatchdogTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction arrete la temporisation,
servant a gerer la temporisation pour le controle du flux d'emission
de PDU dans le cas d'un canal de type connecte emission.

Cette fonction est utilisee au moment de la destruction du canal
*/
      TEMPO_DEF * Tempo  = &(Channel->DescTempo_For_TransmWatchdog.Desc);

      _TIMER_STOP(Tempo);
  }


/*******************************************************************/
/***** FONCTION PRIVATE StopRecepWatchdogTempo () ******************/
/*******************************************************************/
void StopRecepWatchdogTempo(MCSCHANNEL_REF * Channel)
{
/* cette fonction arrete la temporisation,
servant a gerer la temporisation pour le controle du flux de reception
de PDU dans le cas d'un canal de type connecte emission.

Cette fonction est utilisee au moment de la destruction du canal
*/
      TEMPO_DEF * Tempo  = &(Channel->DescTempo_For_RecepWatchdog.Desc);

      _TIMER_STOP(Tempo);
  }

/*******************************************************************/
/****** FONCTION PRIVATE ExecuteDeletePreliminaryActions() *********/
/*******************************************************************/
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
void ExecuteDeletePreliminaryActions(MCSCHANNEL_REF * Channel)
{
/*fonction qui engage toutes les actions preliminaires pour la destruction
du canal.
Fonction appellee au moment de la prise en compte de la destruction du
canal
Les actions preliminaires sont les suivantes:

    -purger, de la file d'interconnexion contenant les demandes
     de transfert MCS en attente, toutes les demandes concernant
     le canal que l'on veut detruire.

     Cette purge ne se fait pas necessairement sous section critique,
     car si les utilisateurs peuvent remplir en meme temps cette file, les
     nouveaux elements ne peuvent concerner ce canal. On n'aura donc
     pas a les purger. D'autre part, cette file ne peut etre vider em
     meme temps. Enfin. on purge en partant de la tete de file.
     Tout cela fait que si on part de la tete, et que l'on continue
     jusqu'au dernier element que CETTE PROCEDURE repere, on est sur
     de tout purger, meme si on n'est pas en section critique.

     Pour chaque element purge, on produit une confirmation vers l'utilisateur.
     Les  Warnings ne sont par produits  dans ce cas...

    -idem pour la file contenant les indications de la couche DLL
     Pour chaque element purge, on recycle les ressources memoire concernees.
     Les  Warnings ne sont par produits  dans ce cas...

    -purger la file interne du canal qui contient les demandes de transfert MCS
    en attente.

    -arreter + neutraliser les temporisations de watchdog (au nbre de 2, si Stuffing)

    -annuler les transferts en cours ou a venir, des PDUs contenus dans
    la fenetre d'emission

    -positionner le flag de segmentation a "cancelled", pour que le transfert
    a venir des PDUs qui seron fabriques par cette machine soit annule

    -neutraliser les demarrages et retrigs de la tempo utilisee pour l'emission
    des trames de bourrage.

    -vider la fenetre de reception des PDUs ayant ete recus, en recyclant les
    ressources qui supportent ces PDUs.

    -vider la machine de reassemblage, en recyclant les ressources qui
    supportent les PDUs qui constituent cette chaine.

*/


    INTEG_FILE_TYPE   *File;
    INTEG_FILE_ELT    *Elem;

    /*purge de la file des demandes de transfert MCS en attente: */
    {
        MCS_SDU_TO_SEND * McsRq;
        File = & (Channel->Mcs->List[_SendMCSList]);
        Elem = CONSULT_FIFO_INTEG (File);

        for (;;) {
           if (Elem == NULL) break;
           McsRq = (MCS_SDU_TO_SEND *) Elem;

           if (McsRq->Channel.McsChannelRef == Channel ) {
               EXTRACT_FROM_FIFO_INTEG( File, Elem);
               /* confirmer a l'utilisateur */
               McsRq->Service_Report =_MCS_SEND_DATA_ON_NOT_RUNNING_CHANNEL;
               Channel->User_Msg_Ack (Channel->UserChannelRef,McsRq);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
               Channel->Statistics[ _ConfirmDataSDUTransmission ]++;
               Channel->Statistics[ _SendDATARequestNOK ]++;
#endif
                /* personne ne peut modifier ou consulter ce compteur
                       EN CE MOMENT ->  section critique non necessaire */
               Channel->NumberOfInRequests --;

           }
           Elem=Elem -> Ptr_Next;
       }

   }/* fin purge de la file des demandes de transfert MCS en attente */

    /*purge de la file des indications DLL en attente: */
    {
        FDM_MSG_RECEIVED * DLLInd;
        File = & (Channel->Mcs->List[_ReceiveDLLList]);
        Elem = CONSULT_FIFO_INTEG (File);
        for (;;) {
           if (Elem == NULL) break;
           DLLInd = (FDM_MSG_RECEIVED *) Elem;
           if ((MCSCHANNEL_REF *) DLLInd->User_Qid == Channel ) {
               EXTRACT_FROM_FIFO_INTEG( File, Elem);
               /* recycler */
               fdm_msg_data_buffer_free(DLLInd->Ptr_Block);
               fdm_msg_ref_buffer_free(DLLInd);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
               Channel->Statistics[ _FreeDLLRefBuffer ]++;
               Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif
               /* personne ne peut modifier ou consulter ce compteur
                       EN CE MOMENT ->  section critique non necessaire */
               Channel->NumberOfInRequests-=2;
           }

           Elem=Elem -> Ptr_Next;
       }
   }/* fin purge de la file des indications DLL en attente */

    /*purge de la file INTERNE des demandes de transfert MCS en attente: */
    {
        MCS_SDU_TO_SEND * McsRq;
        for (;;) {

           McsRq = ( MCS_SDU_TO_SEND *) GET_FIFO_INT(&(Channel->RqList));
           if (McsRq == NULL) break;

           /* confirmer a l'utilisateur */
           McsRq->Service_Report = _MCS_SEND_DATA_ON_NOT_RUNNING_CHANNEL;
           Channel->User_Msg_Ack (  Channel->UserChannelRef, McsRq);
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
               Channel->Statistics[ _ConfirmDataSDUTransmission ]++;
               Channel->Statistics[ _SendDATARequestNOK ]++;
#endif
           /* personne ne peut modifier ou consulter ce compteur
              EN CE MOMENT ->  section critique non necessaire */
           Channel->NumberOfInRequests --;
           }
   }/* fin purge de la file interne des demandes de transfert MCS en attente */

   /* arreter les temporisations de Watchdog */
   /* neutraliser les temporisations, pour qu'elles ne redemarrent pas */

   if (Channel->StuffingMode == _MCS_WITH_STUFFING_MODE) {
        StopTransmWatchdogTempo(Channel);
        StopRecepWatchdogTempo(Channel);

        Channel->StuffingMode = _MCS_STUFFING_CANCELLED;
   }

   /* annuler les transferts de PDU dans la fenetre d'anticipation en emission */
   {
       int i;

       for (i=0 ; i< _PDUMakeAnticipation(Channel) ; i ++) {
            (Channel->TransmitWindowArea->Box[i]).PurgeState = _Cancelled;
       }
   }

   /* annuler les transferts de PDU en attente dans la machine de segmentation */
   {
        Channel->SDUPurgeState = _Cancelled ;
   }

   /* vider la machine de reassemblage en recyclant les ressources*/
   {
      unsigned int              i;
      FDM_MSG_R_DESC * Elem, *NextElem;

      if (Channel->Assembling.State != _EmptyMachine) {

            Elem = (FDM_MSG_R_DESC *)
              Channel->Assembling.LineStart;

            for (i=0; i<Channel->Assembling.Nr_Of_Blocks; i++ ) {
                NextElem = Elem->Next_Block;
                fdm_msg_data_buffer_free(Elem)  ;
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
                Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif
                /* personne ne peut modifier ou consulter ce compteur
                       EN CE MOMENT ->  section critique non necessaire */
                Channel->NumberOfInRequests--;
          }
       }
   }

   /* vider la fenetre de receptiomge en recyclant les ressources*/
   {
      int              i    ;
      FDM_MSG_R_DESC * Elem ;

      for (i=0; i<Channel->R_Anticipation; i++ ) {
          if ( ((Channel->RecepWindowArea->Box[i]).State      == _Available_Pdu )           &&
               ((Channel->RecepWindowArea->Box[i]).UserState  == _NotYetTakenIntoAccount )  ){

                fdm_msg_data_buffer_free(Elem)  ;
#if (MCS_WITH_COUNTERS == _MCS_OPTION_YES )
                Channel->Statistics[ _FreeDLLDataBuffer ]++;
#endif
                /* personne ne peut modifier ou consulter ce compteur
                       EN CE MOMENT ->  section critique non necessaire */
                Channel->NumberOfInRequests--;
           }
      }
   }
}
#endif

/*******************************************************************/
/****** FONCTION PRIVATE ExecuteDeleteFinalActions() ***************/
/*******************************************************************/
void ExecuteDeleteFinalActions(MCSCHANNEL_REF * Channel)
{

/*fonction qui engage toutes les actions qui n'on pas
ete effectuees pendant la phase "preliminaire" de la destruction
du canal.
Fonction appelee au moment de la destruction effective du canal.
*/
      MCS_REF *                    Mcs;
      MCS_CHANNEL_COMMAND   * DeleteRequestRef;
	  Mcs					= Channel->Mcs;
	  DeleteRequestRef		= Channel->DeleteRequestRef;

     /* vider le reservoir de ressources pour les acquittements
     de PDU */
     {
       FIFO_INT_ELT_TYPE *   R;
         for (;;) {
            R=GET_FIFO_INT(&(Channel->AckRessourcesList));
            if (R==NULL) break;
            _FREE_MEMORY((User_GDM*)R);
        }
     }

     /*detruire les pools memoire
           - pour les acquittements
           - pour les indications MCS->USER de messages
           - pour les indications MCS->ACSE de SDU Acse*/

      _DELETE_USER (Channel->PoolForAckRessources);
      _DELETE_USER (Channel->PoolForUserIndicationRessources);


    /* detruire le LSAP */
      fdm_messaging_delete(Channel->Lsap_Ctxt);

    /*liberer la memoire utilisee pour la fenetre d'emission */
    OS_Free( Channel->Mcs->Memory_Management_Ref,
             Channel->TransmitWindowArea);

    /*liberer la memoire utilisee pour la fenetre de reception */
    OS_Free( Channel->Mcs->Memory_Management_Ref,
             Channel->RecepWindowArea);

    /*supprimer de la liste des canaux*/
    EXTRAIT_FIFO_INT(& (Mcs->ListOfChannels),(FIFO_INT_ELT_TYPE *) Channel);

   /*razer tout avant de liberer la memoire */
   memset ((char*)Channel , 0 , sizeof(MCSCHANNEL_REF));
   OS_Free(Mcs->Memory_Management_Ref,Channel);

    DeleteRequestRef->Report = _MCS_OK;

   /* confirmer au niveau de Acse */
    Mcs->Extern_Signal_Command_Cnf(Mcs,DeleteRequestRef);

}


/*******************************************************************/
/****** FONCTION PRIVATE ExecuteDeletePreliminaryActions() *********/
/*******************************************************************/
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
void ExecuteDeleteIfRequestedAndPossible(MCSCHANNEL_REF * Channel)
{
/*Fonction qui ne peut etre appellee que suite a une action
du sequenceur.
On n'est pas protege au moment de cet appel.
Elle est appelee lorsqu'un evenement reponse revient dans MCS,
apres etre passe dans une file d'interconnexion.
Si une demande de destruction a ete faite, et que le nombre
de ressources de MCS dehors = 0, et que le nbre de ressources
exterieures dans MCS = 0, alors on detruit le canal.
Sinon, on ne fait rien*/

    if (Channel->State != _MCS_CHANNEL_STATE_DELETE_IN_PROGRESS )
          return;

    if  ((Channel->NumberOfInRequests != 0 )  ||
         (Channel->NumberOfOutRequests != 0 ) ){
          return;
    }

    ExecuteDeleteFinalActions(Channel);
}
#endif



/*******************************************************************/
/***** FONCTION PRIVATE FileOut_CommandElement() *******************/
/*******************************************************************/
/* procedure modifie pour tenir compte de l'option
MCS_WITH_CONFIGUSER_SPECIF:
 dans ce cas on n'utilise pas le mode _MCS_WITH_STUFFING_MODE
 donc lors de la creation du canal on passe directement au mode
_MCS_CHANNEL_STATE_RUNNING sans passer par
le mode_MCS_CHANNEL_STATE_IDLE_TO_START
on n'utilise pas la commande start */
void FileOut_CommandElement(void * Command)
{
/*
Cette fonction est appelee par le sequenceur apres
depilement de la commande utilisateur qui avait ete stockee dans
la file adequate.
Actions:
Verifier que la commande est correcte, en fonction de l'etat courant du
canal.
Si C'est OK, appeler la fonction de traitement correspondant a la commande,
attendre le resultat, changer l'etat du canal et confirmer.
Si C'est NOK, confirmer en envoyant un compte-rendu negatif.

La seule commande qui peut mal se derouler est la commande de creation,
qui peut echouer lorsque les parametres sont incorrects, ou pas de memoire,
ou etc...

Confirmation:
Le resultat de la commande est passe dans le champ "report" de la ressource
vehiculant la commande.
Le resultat peut etre
   - nok car etat canal nok % commande
   - nok car echec (seulement pour la creation)
   - ok, et dans ce cas l'etat du canal change.
Notes:
-c'est la procedure de creation de canal McsChannelCreate() qui modifie l'etat du canal.
*/

	_MCS_ERROR_CODE_LIST Cr;
    MCS_CHANNEL_COMMAND * Comm = ( MCS_CHANNEL_COMMAND * ) Command;
    MCS_REF             * Mcs  = Comm->mcs_ref;
	MCSCHANNEL_REF		*channelmcs;

	if (Comm->Command == _MCS_CHANNEL_CREATE) {
#if (MCS_WITH_CONFIGUSER_SPECIF == _MCS_OPTION_YES )
		 /* partie configuration User */
			/* paramIN_for_create pointe sur MCS_CHANNEL_USER_CONFIGURATION*/
         channelmcs= McsChannelCreateUserSpecif (Mcs,Comm->paramIN_for_create,&Cr);
#else
		 /* paramIN_for_create pointe sur MCS_CHANNEL_CONFIGURATION*/
		 channelmcs= McsChannelCreate( Mcs, Comm->paramIN_for_create,&Cr);
#endif
		 if (Cr !=_MCS_OK){
			 /* indique defaut specifiaque config user*/
			Comm->Report=Cr;
			Comm->channelmcs= NULL;
			Mcs->Extern_Signal_Command_Cnf(Mcs,Comm);
			return;
		 }
		 else{
			 MCS_BEGIN_CRITICAL(Mcs) ;
#if (MCS_WITH_CONFIGUSER_SPECIF == _MCS_OPTION_YES )
			 channelmcs->State = _MCS_CHANNEL_STATE_RUNNING;
#else
			 channelmcs->State = _MCS_CHANNEL_STATE_IDLE_TO_START ;
#endif
			MCS_END_CRITICAL(Mcs) ;
			Comm->Report=_MCS_OK;
			Comm->channelmcs= channelmcs;
			Mcs->Extern_Signal_Command_Cnf(Mcs,Comm);
			return;
		 }
	}

#if (MCS_WITH_CONFIGUSER_SPECIF == _MCS_OPTION_NO )

	 if ( Comm->Command == _MCS_CHANNEL_START) {
        /*verifier la coherence de la commande par rapport a l'instant t */
        Cr= CheckCommandTowardsChannelFeaturesAndState
                 (Comm->channelmcs, Comm->Command );

        if (Cr != _MCS_OK ) {
            /* Mcs->Extern_Signal_Warning (Command,  Cr);*/
			 Comm->Report =Cr ;
			  /* confirmer au niveau User */
             Mcs->Extern_Signal_Command_Cnf(Mcs, Comm);
			 return;
        }
		/*demarrer les temporisations du canal*/
		if (channelmcs->StuffingMode == _MCS_WITH_STUFFING_MODE  ) {
			StartStuffingTempo(channelmcs);
			StartTransmWatchdogTempo(channelmcs);
			StartRecepWatchdogTempo(channelmcs);
#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
			channelmcs->NumberOfOutRequests ++ ; /* pour la tempo de bourrage */
#endif
		}

		 MCS_BEGIN_CRITICAL(Mcs) ;
		 channelmcs->State = _MCS_CHANNEL_STATE_RUNNING;
		 MCS_END_CRITICAL(Mcs) ;
		 Comm->Report =Cr ;
		/* confirmer au niveau User */
         Mcs->Extern_Signal_Command_Cnf(Mcs, Comm);
		return;
     }
#endif


#if (MCS_WITH_COMMAND_CHANNELS == _MCS_OPTION_YES )
	if ( Comm->Command == _MCS_CHANNEL_STOP) {
        /*verifier la coherence de la commande par rapport a l'instant t */
        Cr= CheckCommandTowardsChannelFeaturesAndState
                 (Comm->channelmcs, Comm->Command );
        if (Cr != _MCS_OK ) {
            /* Mcs->Extern_Signal_Warning (Command,  Cr);*/
			 Comm->Report =Cr ;
			  /* confirmer au niveau User */
             Mcs->Extern_Signal_Command_Cnf(Mcs, Comm);
			 return;
        }

        MCS_BEGIN_CRITICAL(Mcs);
        Comm->channelmcs->State = _MCS_CHANNEL_STATE_IDLE_TO_DELETE;
        MCS_END_CRITICAL(Mcs);

        ExecuteDeletePreliminaryActions(Comm->channelmcs);
        Comm->Report =_MCS_OK;
			  /* confirmer au niveau User */
        Mcs->Extern_Signal_Command_Cnf(Mcs,Comm);
	    return;
     }


     if ( Comm->Command == _MCS_CHANNEL_DELETE) {
        /*verifier la coherence de la commande par rapport a l'instant t */
        Cr= CheckCommandTowardsChannelFeaturesAndState
                 (Comm->channelmcs, Comm->Command );

        if (Cr != _MCS_OK ) {
             /*Mcs->Extern_Signal_Warning (Command,  Cr);*/
			 Comm->Report =Cr;
			/* confirmer au niveau User */
             Mcs->Extern_Signal_Command_Cnf(Mcs, Comm);
			 return;
        }

        MCS_BEGIN_CRITICAL(Mcs);
        Comm->channelmcs->State = _MCS_CHANNEL_STATE_DELETE_IN_PROGRESS;
        MCS_END_CRITICAL(Mcs);
        Comm->channelmcs->DeleteRequestRef = Comm;
		ExecuteDeleteIfRequestedAndPossible(Comm->channelmcs);

         /* on ne confirme pas le delete ici. En effet, il ne sera fait
         que au moment ou la destruction sera effective: peut-etre dans
         cet appel de ExecuteDeleteIfRequestedAndPossible() ou plus tard,
         dans un autre appel de cette procedure.
         Par contre, on reference la ressources contenant la commande de destruction,
         dans le canal, pour pouvoir la retrouver au moment ou on confirmera.
        */
        return;
    }
#endif

}
/*************** End Of  'mcsconfi.c' File *************************/
