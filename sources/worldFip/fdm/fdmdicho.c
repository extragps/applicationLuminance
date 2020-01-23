/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
    
  
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmdicho.c
  Description     : B_A_O
                    Procedures de Gestion Dichotomie
                   
 
  Date de creation: Fri Mar 15 15:11:28 MET 1996
  
***********************************************************/


#define ___dicho_c

/***************************************************************************/
/*                      DEPENDANT FILES                                    */
/***************************************************************************/
#include "user_opt.h"

#include "fdmdicho.h"
#if 					(  FDM_WITH_SOLARIS == YES ) 
void memcpy(void *, void * , int ); 
#define memcpy(s1,s2,n) bcopy(s2,s1, n)
#endif

/***************************************************************************/
/*                     PRIVATE VARIABLES                                   */
/***************************************************************************/
static const element  E_vide = { -1 , nil_use } ;


/***************************************************************************/
/*                     PRIVATE FUNCTIONS                                   */
/***************************************************************************/
static void _TRI_TABLE_BEGIN( _TABLE * );
static void _TRI_TABLE_END  ( _TABLE * );
static int  _DICHO_EXTEND  ( _TABLE ** ) ;
static void _DICHO_COMPRES ( _TABLE ** );
static _TABLE *_DICHO_CREATE ( MEMORY_RN *  , int  );



/******************* _TRI_TABLE_BEGIN () ***********************************/
/*                                                                         */
/***************************************************************************/
static void _TRI_TABLE_BEGIN( _TABLE *x )
{
/* 
	tri par ordre croissant en commancant par le debut
	a faire suite a un ajout d'element
	une table deja trie le deviendra des la premiere passe
*/
    int i,k;
    element bidon;

    k = x->nombre - 2;
    for (   i = 0; i <= k  ; i++ ) {
	if (x->k[i].objet_num > x->k[i+1].objet_num ) {
	     bidon     = x->k[i] ;
	     x->k[i]   = x->k[i+1] ;
	     x->k[i+1] = bidon ;
	}
	else {/* arret des que stable */
	   if ( (x->k[i].objet_num != -1 ) && ( x->k[i+1].objet_num != -1) )
		      break;
	}
    }
}


/******************* _TRI_TABLE_END () *************************************/
/*                                                                         */
/***************************************************************************/
static void _TRI_TABLE_END( _TABLE *x )
{
/* 
tri par ordre croissant en commancant par la fin
	a faire suite a un retrait d'element
	une table deja trie le deviendra des la premiere passe
*/

	int i,k;
	element bidon;


	k = x->nombre - 2;
	for (   i = k ; i != 0  ; i-- ) {
		if (x->k[i].objet_num > x->k[i+1].objet_num ) { 
			bidon     = x->k[i] ;
			x->k[i]   = x->k[i+1] ;
			x->k[i+1] = bidon ;
		}
		if (x->k[i].objet_num == x->k[i+1].objet_num ) 
			break;  /* fin maj  piusque l'on trouve 2 * (-1 )*/             
	}
}


/******************* _DICHO_SCAN_OBJECT () *********************************/
/*                                                                         */
/***************************************************************************/
void *_DICHO_SCAN_OBJECT( _TABLE **ptr , long val )
{
/* 
fonction:
    recherche dichotomique dans une table 
    En fonction d'un numero d'objet 
    retourne l'adresse de l'objet
*/
	int i,j;
	_TABLE *x;

	x = *ptr;
	i = j = (x->nombre)>>1;
	do {
		j = j >> 1;
		if (x->k[i].objet_num == val ) return ( x->k[i].ptr );
		i +=  ((x->k[i].objet_num < val ) ? j : - j);
	} while (j != 0 ) ;
	if (x->k[0].objet_num == val ) return ( x->k[0].ptr );
	return( nil_ele );
	/* pas trouve ! */
}


/******************* INTERNAL_SCAN_OBJECT () *******************************/
/*                                                                         */
/***************************************************************************/
void *INTERNAL_SCAN_OBJECT( _TABLE **ptr , long val ) 
{
/* 
fonction:
    recherche dichotomique dans une table 
    En fonction d'un numero d'objet 
    retourne l'adresse de l'objet
*/
	int i,j;
	_TABLE *x;

	x = *ptr;
	i = j = (x->nombre)>>1;
	do {
		j = j >> 1;
		if (x->k[i].objet_num == val ) return ( &x->k[i] );
		i +=  ((x->k[i].objet_num < val ) ? j : - j);
	} while (j != 0 ) ;
	if (x->k[0].objet_num == val ) return ( &x->k[0] );
	return( nil_ele );
	  /* pas trouve ! */
}


/******************* _DICHO_EXTEND () **************************************/
/*                                                                         */
/***************************************************************************/
static int _DICHO_EXTEND ( _TABLE **ptr )
{
	unsigned int j,k0;
	long l;
	
	_TABLE *x, *new_x;

	x = *ptr ;      
	k0 = x->nombre ;
	l = (sizeof(element) * k0 * 2 ) + sizeof(_TABLE) - sizeof(element);
	if ( (l & 0xffff0000L) != 0 ){
	    return -1;
	}

        OS_Allocate( x->ref_memory , _TABLE* , new_x  ,  l );
	
	if ( new_x == nil_tab ) {
	    return -1;
	}
	
	for (j = 0 ; j < k0 ; j++){
	    new_x->k[j] = E_vide ;
	}
	memcpy((void*) &new_x->k[k0].objet_num , (void*)&x->k[0].objet_num , 
		sizeof(element) * k0 );
	new_x->nombre = k0 * 2 ;
	new_x->mode   = 1;
	new_x->ref_memory   = x->ref_memory;

	OS_Free( new_x->ref_memory , x );
	
	*ptr = new_x;
	return 0;
}


/******************* _DICHO_COMPRES () *************************************/
/*                                                                         */
/***************************************************************************/
static void _DICHO_COMPRES ( _TABLE **ptr )
{
	unsigned int k0;
	long l;
	
	_TABLE *x, *new_x;

	x = *ptr ;      
	k0 = x->nombre /2 ;
	l = (sizeof(element) * k0 * 2 ) + sizeof(_TABLE) - sizeof(element);
        OS_Allocate( x->ref_memory , _TABLE* , new_x  ,  l );
	
	if ( new_x == nil_tab ) {
	    return ; /* il n y a plus de place on verra plus tard ! */ 
	}
	
	memcpy( (void*) &new_x->k[0].objet_num , (void*) &x->k[k0].objet_num , 
		sizeof(element) * k0 );
	new_x->nombre = k0 ;
	new_x->mode = 1;
	new_x->ref_memory   = x->ref_memory;

	OS_Free( new_x->ref_memory , x );
	
	*ptr = new_x;
}


/******************* _DICHO_ADD_OBJECT () **********************************/
/*                                                                         */
/***************************************************************************/
enum __dicho_result _DICHO_ADD_OBJECT ( _TABLE **src  , element *E )
{
	_TABLE  *x;
	element *I;
	int Cr;
	
	
	if ( INTERNAL_SCAN_OBJECT ( src  , E->objet_num ) != nil_ele ){
	   return __dicho_already_existing_elt;
	}
		
	x = *src;

	if (x ->k[0].objet_num != -1 ) { /* plein */
	    if (x->mode) {      
			Cr = _DICHO_EXTEND ( src );
			if ( Cr != 0 ) return __dicho_extend_memory_lack ;
			x = *src;
	    } else {
			return __dicho_overflow;
	    }
	}
	I = (element *)INTERNAL_SCAN_OBJECT ( src  , -1 )  ;
	*I  = *E ;
	 _TRI_TABLE_BEGIN( x ) ;
	 return (__dicho_result_ok );

}


/******************* _DICHO_SUP_OBJECT () **********************************/
/*                                                                         */
/***************************************************************************/
void *_DICHO_SUP_OBJECT ( _TABLE **src  , long objet_num )
{
	_TABLE  *x;
	element *Eu;
	void *S;

	Eu = (element *)INTERNAL_SCAN_OBJECT ( src  , objet_num ) ;
	if ( Eu == nil_ele ){
	      return( nil_ele );
	}
	
	x = *src;
	S = Eu->ptr ;
	*Eu = E_vide ;
	_TRI_TABLE_END( x ) ;
	if (x->mode) {
	    if ( x->nombre > 4 ) {
		if ( x->k[(x->nombre*3)/4 ].objet_num == -1 ) {
			_DICHO_COMPRES ( src ); 
		}
	    }
	}
	return ( S );

}


/******************* _DICHO_CREATE () **************************************/
/*                                                                         */
/***************************************************************************/
static _TABLE *_DICHO_CREATE ( MEMORY_RN *Ref_memory ,int nb )
{
	_TABLE  *new_x;
	int j , k0 ;
	long l;

	k0 = 1 << nb ;
	l = (sizeof(element) * k0  ) + sizeof(_TABLE) - sizeof(element);
	if (( l & 0xffff0000L ) != 0 ) return nil_tab;
	
        OS_Allocate( Ref_memory , _TABLE* , new_x  ,  l );
		
	if ( new_x == nil_tab ) {
	    return new_x;
	}
	
	for (j = 0 ; j < k0 ; j++){
	    new_x->k[j] = E_vide;
	}
	
	new_x->nombre = k0 ;
	new_x->ref_memory = Ref_memory ;

	return new_x;   
}


/******************* _DICHO_CREATE_FIX () **********************************/
/*                                                                         */
/***************************************************************************/
enum __dicho_result _DICHO_CREATE_FIX ( MEMORY_RN *Ref_memory , _TABLE ** T , int k0 )
{
	_TABLE  *new_x;
	
	new_x =  _DICHO_CREATE( Ref_memory , k0) ;
	if (new_x == nil_tab ) return __dicho_create_memory_lack;
	new_x->mode = 0;
	*T = new_x ;
	return __dicho_result_ok        ;
}


/******************* _DICHO_CREATE_VAR () **********************************/
/*                                                                         */
/***************************************************************************/
enum __dicho_result _DICHO_CREATE_VAR ( MEMORY_RN *Ref_memory , _TABLE ** T )
{
	_TABLE  *new_x;

	new_x =  _DICHO_CREATE( Ref_memory , 2) ;
	if (new_x == nil_tab ) return __dicho_create_memory_lack;
	new_x->mode = 1;
	*T = new_x ;
	return __dicho_result_ok        ;

}


/******************* _DICHO_REMOVE () **************************************/
/*                                                                         */
/***************************************************************************/
void _DICHO_REMOVE (  _TABLE **ptr )
{
	_TABLE  *x;

	x = *ptr ;   
	if  ( x != (_TABLE  *) 0 )  
		OS_Free( x->ref_memory , x );
}


/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/
