/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD



      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm_fifo.c
  Description     : B_A_O
                    Procedures de Gestion des Fifos


  Date de creation: Fri Mar 15 15:10:59 MET 1996

***********************************************************/

#define __fifo_int_c

/***************************************************************************/
/*			DEPENDANT FILES 				   */
/***************************************************************************/


#include "fdm_fifo.h"




/******************* STORE_FIFO_INT() **************************************/
/*									   */
/***************************************************************************/
int  STORE_FIFO_INT(
	  FIFO_INT_TYPE * Fifo ,
	  FIFO_INT_ELT_TYPE * Elt)
{

	if ( Elt == (FIFO_INT_ELT_TYPE *)0 )  {
		/* illegal object */
		return -1;
		}
/* controle que l'on inserre pas 2 fois le meme element*/
{
	FIFO_INT_ELT_TYPE * X;
	X = ( FIFO_INT_ELT_TYPE *) 0;
	while ( (X = SCAN_FIFO_INT(Fifo,X)) != ( FIFO_INT_ELT_TYPE *) 0 )  {
	if ( X == Elt ) {
		return -1;
		}
	}
}
	Elt->Ptr_Arriere = Fifo->Queue_Ptr ;
	Elt->Ptr_Avant = (FIFO_INT_ELT_TYPE *)0 ;

	if( Fifo->Queue_Ptr == (FIFO_INT_ELT_TYPE *)0 )
 		Fifo->Head_Ptr = Elt ;

	else
		/* fifo non vide */
		Fifo->Queue_Ptr->Ptr_Avant = Elt ;

	Fifo->Queue_Ptr = Elt ;

	Fifo->Nbr_Of_Elt++ ;
	return 0;

}



/******************* EXTRAIT_FIFO_INT() ************************************/
/*									   */
/***************************************************************************/
void EXTRAIT_FIFO_INT(
     FIFO_INT_TYPE * Fifo ,
     FIFO_INT_ELT_TYPE * Elt)
{
	FIFO_INT_ELT_TYPE *Pn,*Pp;

	if( Elt == (FIFO_INT_ELT_TYPE *)0 )   return ;
	/* Pas d' Elt en attente dans la fifo */
	if( Fifo->Nbr_Of_Elt == 0 )  return ;


	if (Elt->Ptr_Arriere != (FIFO_INT_ELT_TYPE *)0 ) {
		   		/* on vire le premier */
		if ( Elt->Ptr_Avant == (FIFO_INT_ELT_TYPE *)0 ) {
		   	Pn = Elt->Ptr_Arriere;
		   	Pn->Ptr_Avant = (FIFO_INT_ELT_TYPE *)0 ;
			Fifo->Head_Ptr = Pn ;
		}
		else {	/* extrait entre */
			Pp = Elt->Ptr_Avant;
		   	Pn = Elt->Ptr_Arriere;
		   	Pn->Ptr_Avant = Pp;
		   	Pp->Ptr_Arriere =Pn;
		}
	}
	else { 		/* extrait dernier*/
		Pp = Elt->Ptr_Avant;
		if (Pp == (FIFO_INT_ELT_TYPE *)0) {
		    Fifo->Queue_Ptr = Pp ;
		    Fifo->Head_Ptr = Pp ;
		    }
		else {
		    Pp->Ptr_Arriere = (FIFO_INT_ELT_TYPE *)0;
		    Fifo->Queue_Ptr = Pp ;
		}
	}

 	Fifo->Nbr_Of_Elt-- ;


}



/*********************** GET_FIFO_INT() ************************************/
/*									   */
/***************************************************************************/
FIFO_INT_ELT_TYPE * GET_FIFO_INT( FIFO_INT_TYPE * Fifo )
{
	FIFO_INT_ELT_TYPE * temp ;

	if( Fifo->Nbr_Of_Elt == 0 ){
		/* Pas d' Elt en attente dans la fifo */
		return( (FIFO_INT_ELT_TYPE *)0 ) ;

	}else{
		temp = Fifo->Head_Ptr ;

		Fifo->Head_Ptr = temp->Ptr_Avant ;

		if( Fifo->Head_Ptr != (FIFO_INT_ELT_TYPE *)0 )
		    /* FIFO non vide */
		    (Fifo->Head_Ptr)->Ptr_Arriere = (FIFO_INT_ELT_TYPE *)0 ;
		else
		    Fifo->Queue_Ptr = (FIFO_INT_ELT_TYPE *)0 ;

		Fifo->Nbr_Of_Elt-- ;


		return( temp ) ;
	}
}

/*********************** GET_FIFO_INT() ************************************/
/*									   */
/***************************************************************************/
FIFO_INT_ELT_TYPE * CONSULT_FIFO_INT( FIFO_INT_TYPE * Fifo )
{

	if( Fifo->Nbr_Of_Elt == 0 )
		/* Pas d' Elt en attente dans la fifo */
		return( (FIFO_INT_ELT_TYPE *) 0 ) ;

	return ( Fifo->Head_Ptr );


}


/*********************** CREATE_FIFO_INT() *********************************/
/*									   */
/***************************************************************************/
FIFO_INT_TYPE * CREATE_FIFO_INT( MEMORY_RN  *Ref_memory )
{
    FIFO_INT_TYPE  *P;

    OS_Allocate( Ref_memory , FIFO_INT_TYPE * , P  ,  sizeof(FIFO_INT_TYPE) );

    if (P != (void*)0 )
       memset( (void *)P , 0 , sizeof(FIFO_INT_TYPE) );
    P->ref_memory = Ref_memory;
    return P;

}



/*********************** DELETE_FIFO_INT() *********************************/
/*									   */
/***************************************************************************/
void DELETE_FIFO_INT( FIFO_INT_TYPE *Fifo )
{
    OS_Free(Fifo->ref_memory , (char *)Fifo);
}


/*********************** SCAN_FIFO_INT() ***********************************/
/*									   */
/***************************************************************************/


FIFO_INT_ELT_TYPE*   	SCAN_FIFO_INT (
		FIFO_INT_TYPE  *File_Interne ,
		FIFO_INT_ELT_TYPE* Elt
			    )
{
FIFO_INT_ELT_TYPE* temp ;


    if (File_Interne == 0) return ((FIFO_INT_ELT_TYPE*)0);

    if ( File_Interne->Nbr_Of_Elt == 0 ) 	return( (FIFO_INT_ELT_TYPE *)0 ) ;

    if ( Elt == ( FIFO_INT_ELT_TYPE*) 0 )
		temp = File_Interne->Head_Ptr ;
    else
		temp = Elt->Ptr_Avant ;
    return( temp ) ;

}

/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/
