/************************************************************
   
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
    
  
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm_lai.c
  Description     : B_A_O
                    Procedures de Gestion des fifos inter_taches
                   
  
  Date de creation: Fri Mar 15 15:11:12 MET 1996
  
***********************************************************/

/****************************************************************************/
/*                   DEPENDANT FILES                                        */
/****************************************************************************/
#include "user_opt.h"
#include "fdm_lai.h"
#include "fdm_gdm.h"
 

/****************************************************************************/
/*                   CONSTANTS                                              */
/****************************************************************************/
#define ELT_NULL (INTEG_FILE_ELT*)0

#define VRAI 1
#define FAUX 0

/****************************************************************************/
/*                   TYPES INTERNES                                         */
/****************************************************************************/


/****************************************************************************/
/*                   FUNCTIONS                                              */
/****************************************************************************/
static INTEG_FILE_ELT *      INTERNAL_GET_FROM_FIFO_INTEG(
			     INTEG_FILE_TYPE*
			     );

/************************* TASK_LOOP() **************************************/
/*INPUT  : Reference of the task which works when function is called        */
/*End    : Signal to the monitor interface that there is no more work to do */
/****************************************************************************/
void                         TASK_LOOP(
			     TASK_REFERENCE_TYPE * Task_Ref
			     )
{ 
    int i;
    int nb_soft;
    int fin=FAUX;
    short * P_State_Word;

    nb_soft     =Task_Ref->Nr_Of_Softwares;
    P_State_Word=&(Task_Ref->Task_State_Word);

    OS_Enter_Region();
     if (*P_State_Word==0) {
        OS_Leave_Region();
    	return;
    }
    do {
       	for(i=0; i<nb_soft; i++) {

	   SOFT_REFERENCE_TYPE  * Soft;
	   short Mask;

	   Soft  =Task_Ref->Array_Of_Soft_Ref[i];
	   Mask  =Soft->Mask_For_Place_In_Task;

	   if( (*P_State_Word & Mask) !=0 ){
	       Soft->Software_Loop_Fct(Soft);
	       (*P_State_Word)  &= ~Mask;
	   }
	   /*else;*/
       	}

   	if (*P_State_Word==0) fin=VRAI;

   } while (fin==FAUX);
   
   OS_Leave_Region();

}



/************************* SOFT_LOOP() **************************************/
/*INPUT  : Reference of the soft which works when function is called        */
/*End    : Signal to the 'gestionnaire' that there is no more work to do    */
/****************************************************************************/
void                         SOFT_LOOP(
			     SOFT_REFERENCE_TYPE * Soft_Ref
			     )
{
    int i;
    int nb_files;
    int fin       =FAUX;
    short * P_State_Word;

    INTEG_FILE_ELT * Message;

    nb_files     = Soft_Ref->Nr_Of_External_In_Files;
    P_State_Word = &(Soft_Ref->Software_State_Word);

    if (*P_State_Word==0) return;

    do {
	for(i=0; i<nb_files; i++) {

	    INTEG_FILE_TYPE     * File;
	    short Mask;

	    File=Soft_Ref->Array_Of_In_Files[i];
	    Mask=File->Mask_For_Place_In_Software;

	    while (File->Nbr_Of_Elt !=0) {
		Message=INTERNAL_GET_FROM_FIFO_INTEG(File);
		/*on a forcement Message!=ELT_NULL*/
 
		OS_Leave_Region();
		File->Software_Treatment_Fct(Message);
		OS_Enter_Region();
		
		if (File->Nbr_Of_Elt ==0 ){
		   (*P_State_Word) &= ~Mask;
		   }
	    }
	}

    	if (*P_State_Word==0) fin=VRAI;

    }while (fin==FAUX);
}

/***********************CREATE_TASK_ENVIRONNEMENT()**************************/
/*INPUT: User_Reference to be called back when signaling end of task work   */
/*                                                       or need of hand    */
/*WORK_IS_FINISHED(User_Reference) or HAND_IS_WANTED(User_Reference)        */
/*OUTPUT:Result of the task creation. NULL_PTR means that an error has      */
/*occured. <>NULL_PTR will be the reference to use to attach software       */
/*to the task.                                                              */


void CREATE_TASK_ENVIRONNEMENT ( 
	TASK_REFERENCE_TYPE *T ,
	void (*User_Fonct) ( void* ) , 
	void* User_Ref
	)
{
      memset (T , 0 , sizeof (TASK_REFERENCE_TYPE));
      T->User_Fct = User_Fonct;
      T->User_Ptr = (void*)User_Ref;
}


/***********************ATTACH_SOFT_TO_TASK()*******************************/
/*INPUT :-Task Reference in which the SOFT will run                        */
/*INPUT :-Characteristics of the SOFT (nbr of IN_FILEs & IN_FUNCTIONS)     */
/*OUTPUT:Reference of the Soft in the Task, including IN_FILEs References  */
/*     NULL-> An error has occured, <>NULL ->NO error                      */
/***************************************************************************/

void  ATTACH_SOFT_TO_TASK (
			     TASK_REFERENCE_TYPE * Task_Ref,
			     SOFT_INTERFACE_TYPE * Interface,
			     SOFT_REFERENCE_TYPE *S,
			     INTEG_FILE_TYPE     *T_F
			     )

{
      int        i;

      memset (S , 0 , sizeof (SOFT_REFERENCE_TYPE));

      S->Mask_For_Place_In_Task=(short)   (1<<
	 (Task_Ref->Nr_Of_Softwares));

      S->P_Task_State_Word=&(Task_Ref->Task_State_Word);
      S->Nr_Of_External_In_Files=Interface->Nr_Of_External_In_Files;
      Task_Ref->Array_Of_Soft_Ref[Task_Ref->Nr_Of_Softwares]=S;

      S->Software_Loop_Fct=Interface->Software_Loop_Fct;

      for (i=0; i<S->Nr_Of_External_In_Files ; i++) {

           memset (&T_F[i] , 0 , sizeof(INTEG_FILE_TYPE) );

	   /*remplissage des parametres de la file*/
	   T_F[i].Software_Treatment_Fct    =  (void (*)(INTEG_FILE_ELT*))
	       (Interface->Array_Of_In_Functions[i]).Software_Treatment_Fct;
	   T_F[i].Mask_For_Place_In_Software=(short)(1<<i);
	   T_F[i].Mask_For_Place_In_Task    =S->Mask_For_Place_In_Task;
	   T_F[i].P_Software_State_Word     =&(S->Software_State_Word);
	   T_F[i].P_Task_State_Word         =S->P_Task_State_Word;
	   T_F[i].User_Fct                  =Task_Ref->User_Fct;
	   T_F[i].User_Ptr                  =Task_Ref->User_Ptr;

	   S->Array_Of_In_Files[i]      =&T_F[i];
      }

      Task_Ref->Nr_Of_Softwares++;


}

/*********************STORE_IN_FIFO_INTEG***********************************/
/*INPUT : Elt=Element to store                                             */
/*        File_Integ= Integration File Reference in which to store         */
/***************************************************************************/
static  INTEG_FILE_ELT*   	SCAN_FIFO_INTEG (
	INTEG_FILE_TYPE  *File_Integ ,
	INTEG_FILE_ELT * Elt );

void STORE_IN_FIFO_INTEG(
		    INTEG_FILE_TYPE* File_Integ ,
		    INTEG_FILE_ELT * Elt
		    )
 
{	

/* ici nous sommes masques ! */
    if ( Elt == ELT_NULL ) {
	return;
	}
    if ( File_Integ  == (INTEG_FILE_TYPE*)0) {
	return;
	}
/* controle que l'on inserre pas 2 fois le meme element*/
{
	INTEG_FILE_ELT * X;
	X = ( INTEG_FILE_ELT *) 0;
	while ( (X = SCAN_FIFO_INTEG(File_Integ,X)) != ( INTEG_FILE_ELT *) 0 )  {
	if ( X == Elt ) {
		return ;
		}
	}
}
    
    Elt->Ptr_Next 	= File_Integ->Queue_Ptr;
    Elt->Ptr_Prev	= ELT_NULL;

    if (File_Integ->Queue_Ptr==ELT_NULL) 
	File_Integ->Head_Ptr=Elt;
    else/*non vide*/
	File_Integ->Queue_Ptr->Ptr_Prev=Elt;

    File_Integ->Queue_Ptr=Elt;
    File_Integ->Nbr_Of_Elt++;

    /*positionner mot d'etat % soft*/
    /*positionner mot d'etat % soft*/
    /*on n'avertit le moniteur QUE SI celui_ci n'est pas deje averti*/

     (* ( File_Integ->P_Software_State_Word) ) |=
	      File_Integ->Mask_For_Place_In_Software;

     if (( * (File_Integ->P_Task_State_Word) )==0) {

	  (* ( File_Integ->P_Task_State_Word) ) |=
	      File_Integ->Mask_For_Place_In_Task;	
   			OS_Leave_Region () ;
	 	 File_Integ->User_Fct(File_Integ->User_Ptr);
    			OS_Enter_Region() ;
	 	 return;

     } 
     
     (* ( File_Integ->P_Task_State_Word) ) |=
	      File_Integ->Mask_For_Place_In_Task;
     		

}


/*********************EXTRACT_FROM_FIFO_INTEG()*****************************/
/*INPUT : Elt=Element to extract                                           */
/*        File_Integ= Integration File Reference from which to extract     */
/***************************************************************************/
void EXTRACT_FROM_FIFO_INTEG(
		    INTEG_FILE_TYPE* File_Integ ,
		    INTEG_FILE_ELT * Elt
		    )

{


	INTEG_FILE_ELT           *Pn, *Pp;

	if (Elt==ELT_NULL) return;

	OS_Enter_Region();

	if ( Elt->Ptr_Next == ELT_NULL) {
	/*on extrait le premier*/
		if (Elt->Ptr_Prev !=  ELT_NULL  ){
		    Pp 			= Elt->Ptr_Prev;
		    Pp->Ptr_Next = ELT_NULL;
		    File_Integ->Head_Ptr=Pp;
		}else{   /*extrait entre*/
			if (Elt->Ptr_Next !=  ELT_NULL  ){
				Pp 			= Elt->Ptr_Prev;
				Pn			= Elt->Ptr_Next;
				Pn->Ptr_Prev	= Pp;
				Pp->Ptr_Next	= Pn;
			} else {
				/* file vide */
				File_Integ->Queue_Ptr= ELT_NULL;
			    File_Integ->Head_Ptr = ELT_NULL;				
			}
		}
	}else{ /* on extrait dernier*/
		Pn	= Elt->Ptr_Next;
		File_Integ->Queue_Ptr= Pn ;
		Pn->Ptr_Prev = ELT_NULL ; 
	}
	
	File_Integ->Nbr_Of_Elt--;
    	if (File_Integ->Nbr_Of_Elt ==0){
		(* ( File_Integ->P_Software_State_Word) ) &=
		   ~(File_Integ->Mask_For_Place_In_Software);
	}
	
	OS_Leave_Region();

}


/*********************INTERNAL_GET_FROM_FIFO_INTEG()*********************************/
/*INPUT  : File_Integ= Integration File Reference from which to get        */
/*OUT_PUT: Oldest Element stored in the File or NULL if empty              */
/***************************************************************************/
static INTEG_FILE_ELT * INTERNAL_GET_FROM_FIFO_INTEG(
		    INTEG_FILE_TYPE* File_Integ
		    )
{
/* ici nous sommes masques ! */
    INTEG_FILE_ELT * temp ;

    if(File_Integ->Nbr_Of_Elt == 0 ){
	return( ELT_NULL ) ;
    }else{
	temp = File_Integ->Head_Ptr ;
	File_Integ->Head_Ptr = temp->Ptr_Prev ;
	if( File_Integ->Head_Ptr != ELT_NULL)
		    /* FIFO non vide */ 
	    (File_Integ->Head_Ptr)->Ptr_Next=ELT_NULL;
	else
	    File_Integ->Queue_Ptr =ELT_NULL ;
	File_Integ->Nbr_Of_Elt-- ;
	if (File_Integ->Nbr_Of_Elt==0){
	   (* ( File_Integ->P_Software_State_Word) ) &=
	      ~(File_Integ->Mask_For_Place_In_Software);
	}
	return( temp ) ;
    }
}


/***************************************************************************/

INTEG_FILE_ELT * GET_FROM_FIFO_INTEG(
		    INTEG_FILE_TYPE* File_Integ
		    )
{
/* ici nous sommes masques ! */

    INTEG_FILE_ELT * temp ;
    

    if ( File_Integ  == (INTEG_FILE_TYPE*)0) return( ELT_NULL );
	
    OS_Enter_Region();
    

    
    if(File_Integ->Nbr_Of_Elt == 0 ){
	OS_Leave_Region();
	return( ELT_NULL ) ;
    }else{
	temp = File_Integ->Head_Ptr ;
	File_Integ->Head_Ptr = temp->Ptr_Prev ;
	if( File_Integ->Head_Ptr != ELT_NULL)
		    /* FIFO non vide */ 
	    (File_Integ->Head_Ptr)->Ptr_Next=ELT_NULL;
	else
	    File_Integ->Queue_Ptr =ELT_NULL ;
	File_Integ->Nbr_Of_Elt-- ;
	if (File_Integ->Nbr_Of_Elt==0){
	   (* ( File_Integ->P_Software_State_Word) ) &=
	      ~(File_Integ->Mask_For_Place_In_Software);
	}
	OS_Leave_Region();
	return( temp ) ;
    }
}

/******************** CONSULT_FIFO_INTEG ***********************************/
/*									   */
/***************************************************************************/
/***************************************************************************
retourne le premier element d'une FIFO d'integration 
ou NULL si vide
****************************************************************************/
INTEG_FILE_ELT *   CONSULT_FIFO_INTEG (
	INTEG_FILE_TYPE* File_Integ
	)
			  
{
    INTEG_FILE_ELT * temp ;	


    if ((INTEG_FILE_TYPE*) File_Integ==0) return((INTEG_FILE_ELT*)0);
	
    OS_Enter_Region();
    if(File_Integ->Nbr_Of_Elt == 0 ){
	OS_Leave_Region();
	return( ELT_NULL ) ;
    }else
	temp = File_Integ->Head_Ptr ;
    OS_Leave_Region();
    return( temp ) ;

}


/*********************** SCAN_FIFO_INTEG() *********************************/
/*									   */
/***************************************************************************/
/***************************************************************************
retourne l'element suivant d'une fifo 
ou le premier si Elt = NULL
***************************************************************************/

static INTEG_FILE_ELT*  SCAN_FIFO_INTEG (
	INTEG_FILE_TYPE  *File_Integ ,
	INTEG_FILE_ELT * Elt
			    )
{
    INTEG_FILE_ELT *Tmp;

    if (File_Integ == 0) return ( ELT_NULL );

    if ( File_Integ->Nbr_Of_Elt == 0 ) 	return( ELT_NULL  ) ;

    if ( Elt ==  ELT_NULL )
		Tmp = File_Integ->Head_Ptr;
    else
		Tmp = Elt->Ptr_Next ;
    return( Tmp ) ;

}


/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/
