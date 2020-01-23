/************************************************************
  
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
   
     
  
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm_gdm.c
  Description     : B_A_O
                    Procedures de Gestion de la Memoire

                   
  
  Date de creation: Fri Mar 15 15:11:04 MET 1996
Version 4.1.1
	suppression de warning GNU
Version 4.3.15
	a/- masquage lors du traitement

***********************************************************/
/***************************************************************************/
/*                       DEPENDANT FILES                                   */
/***************************************************************************/

#include "user_opt.h"

#define __fdm_gdm_c
#include "fdm_gdm.h"


/***************************************************************************/
/*                      PRIVATE CONSTANTS                                  */
/***************************************************************************/
#define T_nil (T_File *) 0

#define nil (void *) 0


/***************************************************************************/
/*                      PRIVATE MACROS                                     */
/***************************************************************************/

#define _INCREMENT ou->U_nbr++;
	
#define _DECREMENT ou->U_nbr--; 



/***************************************************************************/
/*                      PUBLIC VARIABLES                                   */
/***************************************************************************/


/***************************************************************************/
/*                      PRIVATE TYPES                                      */
/***************************************************************************/


typedef struct _private_GDM {
	struct	 _private_GDM * next;
	T_File		      * ident;
	User_GDM	      * User_control;
	} private_GDM ;


typedef struct  {
	private_GDM GDM;
	User_GDM USER;
} MAILLE ;




/***************************************************************************/
/*                      PRIVATE FUNCTIONS                                  */
/***************************************************************************/


/**************************** _ajoute() ************************************/
/*                                                                         */
/***************************************************************************/
static int _ajoute( T_File * ou , MAILLE * quoi ) /**/
{

    MAILLE *P;

    quoi->GDM.ident = (T_File *) nil;

    P =(MAILLE*) ou->queue;

    if (P != nil){
	P->GDM.next =  ( private_GDM  *) quoi;
    } else {
	ou->tete = quoi;
    }

    _DECREMENT;
    ou->queue = quoi;
    return 0;
}



/**************************** _extrait() ***********************************/
/*                                                                         */
/***************************************************************************/
static MAILLE *_extrait(T_File * ou)
{
    MAILLE *p;

    if (ou->tete ==  nil)       return (MAILLE*) nil;


    p = (MAILLE*) ou->tete;

    _INCREMENT;

    if (ou->tete == ou->queue) {
	ou->tete = nil;
	ou->queue= nil;
    } else
	ou->tete = p->GDM.next;

    p->GDM.ident = ou;
    return (p);
}


/**************************** _NEW_USER() **********************************/
/*                                                                         */
/***************************************************************************/
#if							(FDM_WITH_RTX  == YES)
__declspec ( dllexport )
#endif
T_File *_NEW_USER ( MEMORY_RN *type , int dim , int nombre)
{
    T_File *M;
    MAILLE *P;
#if					( FDM_WITH_RTX	== YES )
	DWORD Lasterror;
#endif
    if (nombre == 0 ) return ((T_File *) nil) ;

    OS_Allocate( type , T_File*, M ,  sizeof(T_File));
	
    if (M != nil) {
	register int i;

	M->Type = type;
	M->tete = nil;
	M->queue = nil;
	M->extend = T_nil;
	M->E_nbr = 0;
	M->nb_min = (short)nombre;
	M->U_nb_tot = 0;
	M->etat = _GDM_OPENED;
	for ( i = 0 ; i < nombre ; i++){
	    OS_Allocate(type, MAILLE* , P,  dim+sizeof(private_GDM ) );
		
	    if (P == nil ) goto abort;
	    _ajoute(M,P);
	}
	
	M->U_nbr = 0;
	return M;

    abort:

#if					( FDM_WITH_RTX	== YES )
	Lasterror = GetLastError();
#endif
 
	while ( (P = _extrait( M ) ) != nil) {
	    OS_Free( type , P );
	}
	OS_Free( type , M );

	M = (T_File *) nil ;       
    }
#if					( FDM_WITH_RTX	== YES )
	SetLastError(Lasterror);
#endif

    return  M;
} 


/**************************** _DELETE_USER() *******************************/
/*                                                                         */
/***************************************************************************/
#if							(FDM_WITH_RTX  == YES)
__declspec ( dllexport )
#endif
int _DELETE_USER ( T_File *Identification )
{
    MAILLE *P;

	if ( Identification == (T_File *) 0 ) return -1;

	Identification->etat = _GDM_CLOSED;


	if (Identification->extend != nil) return -1;

	while ( (P = _extrait( Identification ) ) != nil) {
		OS_Free ( Identification->Type , P ) ;
	}

	if ( Identification->U_nb_tot ==  0 )
		OS_Free ( Identification->Type , Identification );

    return 0;
}


/**************************** _ALLOCATE_MEMORY() ***************************/
/*                                                                         */
/***************************************************************************/
#if							(FDM_WITH_RTX  == YES)
__declspec ( dllexport )
#endif
User_GDM *_ALLOCATE_MEMORY( T_File  *Identification )
{
    MAILLE *P;

    if (Identification == nil) {
	return (User_GDM *) nil;
    }
	
    if (Identification->etat == _GDM_CLOSED) {
	return (User_GDM *) nil;
	}

    OS_Enter_Region();

    P = _extrait( Identification ) ;

    if ( P == nil ) {
	if (Identification->extend != nil ) {
	    P = (MAILLE *) _extrait( (T_File *) Identification->extend ) ;
	    if ( P ==  nil ) goto abort ;
	    else
		P->GDM.next = (private_GDM *) Identification;
	}
	else goto abort;
    }
    else
	P->GDM.next = (private_GDM *) Identification;

    Identification->U_nb_tot++;
    P->GDM.User_control = (User_GDM *) &P->USER;
    OS_Leave_Region() ;
    return ( (User_GDM *) &P->USER);

abort:
    OS_Leave_Region() ;
    return (User_GDM *) nil;
}


/**************************** _FREE_MEMORY() *******************************/
/*                                                                         */
/***************************************************************************/
#if							(FDM_WITH_RTX  == YES)
__declspec ( dllexport )
#endif
int _FREE_MEMORY ( User_GDM *U )
{
    private_GDM *P;
    MAILLE *M;
    T_File *T;
	int CR ;

    if ( U ==  (User_GDM *) nil ) {
	return -1;      
    }

    P = (private_GDM *) U;/* conversion de type */
    P--;
    M = (MAILLE *) P;/* reconversion */

    if ( M->GDM.ident ==        (T_File *) nil ) {
	return -1;      
    }
    if ( P->User_control  != U ) {
	return -1;
	}

    OS_Enter_Region();

    T = ( T_File *) M->GDM.next ;
    if ( T  ==  M->GDM.ident ) {
	/*memoire user */
	T->U_nb_tot--;
    } else {
	/*memoire extend */
	T->E_nbr--;
    }
    CR = _ajoute(  M->GDM.ident , M );
    if ( T->etat == _GDM_CLOSED) {
	 _DELETE_USER ( T );
	}

    OS_Leave_Region() ;
    return CR;
}


/**************************** _QUI_SUIS_JE() *******************************/
/*                                                                         */
/***************************************************************************/
T_File *  _GDM__SUIS_JE ( User_GDM *U )
{
    private_GDM *P;
    MAILLE *M;

    P = (private_GDM *) U;/* conversion de type */
    P--;
    M = (MAILLE *) P;/* reconversion */

    if ( M->GDM.ident ==	(T_File *) nil ) {
	return ( T_File *)nil;		/* ident inconnu */
    }
    if ( P->User_control  != U ) {
	return ( T_File *)nil;		/* ident inconnu */
	}

    return( ( T_File *) M->GDM.next) ;
}

void   _GDM__SWITCH_POOL ( User_GDM *U1 , User_GDM *U2  )
{
    private_GDM *P1,*P2;
	
    struct  _private_GDM * N;

    T_File   *T;

    P1 = (private_GDM *) U1;/* conversion de type */
    P1--;

    P2 = (private_GDM *) U2;/* conversion de type */
    P2--;


    T = P1->ident ;
    P1->ident = P2->ident;
    P2->ident = T;

    N = P1->next ;
    P1->next = P2->next;
    P2->next = N;
}

enum	GDM_STATES  _GDM__USER_STATUS   ( T_File  * T )
{
	return T->etat;
}


/***************************** _NEW_EXTEND() *******************************/
/*                                                                         */
/***************************************************************************/
T_File *_NEW_EXTEND ( MEMORY_RN *type  , int dim , int nombre)
{

    return (_NEW_USER (type , dim , nombre));

}


/***************************** _DELETE_EXTEND() ****************************/
/*                                                                         */
/***************************************************************************/
int _DELETE_EXTEND ( T_File *ext )
{
    MAILLE *P;

    if (ext->E_nbr != 0 ) {
	return -1;
    }

    if (ext->U_nbr != 0 ) {
	return -1;
    }

    while ( (P = _extrait( ext ) ) != nil){
	OS_Free ( ext->Type ,  (void *) P ) ;
    }

    OS_Free ( ext->Type ,        ext ) ;
	
    return 0;
}


/***************************** _CONNECT_EXTEND() ***************************/
/*                                                                         */
/***************************************************************************/
int _CONNECT_EXTEND ( T_File *Identification , T_File *extend)
{

    if (Identification->Type != extend->Type ) {
	return -1;
    }

    if (Identification->extend != nil ) {
	return -1;
    }

    extend->E_nbr++;
    Identification->extend = extend ;
    return 0;
}


/*************************** _DISCONNECT_EXTEND() **************************/
/*                                                                         */
/***************************************************************************/
void _DISCONNECT_EXTEND ( T_File *Identification )
{
    T_File *P;

    P = Identification->extend;
    P->E_nbr--;
    Identification->extend = T_nil ;

}

/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/
