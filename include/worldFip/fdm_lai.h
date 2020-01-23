/************************************************************
   
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD

  
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm_lai.h
  Description     : B_A_O
                    Fichier entete pour Gestion des fifos inter_taches
                   
  
  Date de creation: Fri Mar 15 15:11:14 MET 1996
  
***********************************************************/

#ifndef ___lai_h
#define ___lai_h


/****************************************************************************/
/*                   CONSTANTS                                              */
/****************************************************************************/

#define MAX_TACHES                              16       /*pas de controles*/
#define MAX_LOGICIELS_PAR_TACHE     16
#define MAX_FILES_PAR_LOGICIEL      16   /*pas de controles*/




/***************************************************************************/
/*                        PUBLIC TYPES                                     */
/***************************************************************************/

typedef struct _INTEG_FILE_ELT {
	struct _INTEG_FILE_ELT * Ptr_Next;
	struct _INTEG_FILE_ELT * Ptr_Prev;
	}INTEG_FILE_ELT;


typedef struct {
    INTEG_FILE_ELT *           Head_Ptr;
    INTEG_FILE_ELT *           Queue_Ptr;
    int                        Nbr_Of_Elt;
    void		       (*Software_Treatment_Fct)(INTEG_FILE_ELT * );
    short                      Mask_For_Place_In_Software;
    short                      Mask_For_Place_In_Task;
    short *                    P_Software_State_Word;
    short *                    P_Task_State_Word;
    void (*User_Fct) ( void* );
    void *User_Ptr;
    }INTEG_FILE_TYPE;






typedef struct _SOFT_REFERENCE_TYPE {
    short       Mask_For_Place_In_Task;
    short       Software_State_Word;
    short *     P_Task_State_Word;
    void        (*Software_Loop_Fct)(struct _SOFT_REFERENCE_TYPE * );
    int                 Nr_Of_External_In_Files;
    INTEG_FILE_TYPE *   Array_Of_In_Files [MAX_FILES_PAR_LOGICIEL];
    }SOFT_REFERENCE_TYPE;


typedef struct {
    void (*Software_Loop_Fct) (SOFT_REFERENCE_TYPE * );
    unsigned short  Nr_Of_External_In_Files;

    struct _FUNCTION {
	void (*Software_Treatment_Fct)();
    	} Array_Of_In_Functions [MAX_FILES_PAR_LOGICIEL];
    } SOFT_INTERFACE_TYPE;



typedef struct {
    short                     Task_State_Word;
    int                       Nr_Of_Softwares;
    SOFT_REFERENCE_TYPE *     Array_Of_Soft_Ref[MAX_LOGICIELS_PAR_TACHE];
    void (*User_Fct) ( void *);
    void  *User_Ptr;
    } TASK_REFERENCE_TYPE;




/***************************************************************************/
/*                        PUBLIC FUNCTIONS                                 */
/***************************************************************************/

#if __cplusplus
extern "C" {
#endif

 void CREATE_TASK_ENVIRONNEMENT (
		TASK_REFERENCE_TYPE * ,
		void (*) ( void *) , void*
		);

void  ATTACH_SOFT_TO_TASK (
			    TASK_REFERENCE_TYPE * ,
			     SOFT_INTERFACE_TYPE * ,
			     SOFT_REFERENCE_TYPE  *,
			     INTEG_FILE_TYPE*
			     );

 void                  	TASK_LOOP(
			     TASK_REFERENCE_TYPE *
			     );

 void                  	SOFT_LOOP(
			     SOFT_REFERENCE_TYPE *
			     );

 void                  	STORE_IN_FIFO_INTEG(
			     INTEG_FILE_TYPE*  ,
			     INTEG_FILE_ELT *
			     );

 void                  	EXTRACT_FROM_FIFO_INTEG(
			     INTEG_FILE_TYPE*  ,
			     INTEG_FILE_ELT *
			     );


INTEG_FILE_ELT *      	GET_FROM_FIFO_INTEG(
			     INTEG_FILE_TYPE*
			     );


INTEG_FILE_ELT *      	CONSULT_FIFO_INTEG(
			     INTEG_FILE_TYPE*
			     );
		  

#if __cplusplus
}
#endif



#endif
