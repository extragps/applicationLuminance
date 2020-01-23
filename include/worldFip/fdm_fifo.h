/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm_fifo.h
  Description     : B_A_O
                    Fichier entete pour Gestion des Fifos


  Date de creation: Fri Mar 15 15:11:01 MET 1996

***********************************************************/


#ifndef ___fifo_int
#define ___fifo_int


#include "user_opt.h"
#include "gdm_os.h"


/***************************************************************************/
/*			PUBLIC TYPES					   */
/***************************************************************************/
typedef struct FIFO_INT {
    struct FIFO_INT * Ptr_Avant ;
    struct FIFO_INT * Ptr_Arriere ;
    } FIFO_INT_ELT_TYPE ;

typedef struct _FIFO_INT_TYPE{
    FIFO_INT_ELT_TYPE * Head_Ptr;
    FIFO_INT_ELT_TYPE * Queue_Ptr;
    int  Nbr_Of_Elt;
    MEMORY_RN  *ref_memory;
    } FIFO_INT_TYPE;



/***************************************************************************/
/*		       EXTERNALS					   */
/***************************************************************************/


/***************************************************************************/
/*		       PUBLIC FUNCTIONS					   */
/***************************************************************************/

extern	     FIFO_INT_TYPE * CREATE_FIFO_INT(	MEMORY_RN  *ref_memory );

extern void  DELETE_FIFO_INT( FIFO_INT_TYPE * );

extern int  STORE_FIFO_INT( FIFO_INT_TYPE *  , FIFO_INT_ELT_TYPE *);

extern FIFO_INT_ELT_TYPE * GET_FIFO_INT ( FIFO_INT_TYPE *) ;

extern void  EXTRAIT_FIFO_INT (FIFO_INT_TYPE * ,FIFO_INT_ELT_TYPE *);


extern FIFO_INT_ELT_TYPE* SCAN_FIFO_INT (FIFO_INT_TYPE * File_Interne ,FIFO_INT_ELT_TYPE * Elt );

extern FIFO_INT_ELT_TYPE*   	CONSULT_FIFO_INT ( FIFO_INT_TYPE * File_Interne );

#endif
/*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*/
