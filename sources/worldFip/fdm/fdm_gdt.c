/************************************************************
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD



      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm_gdt.c
  Description     : B_A_O
                    Procedures de Gestion des Timers


  Date de creation: Fri Mar 15 15:11:08 MET 1996

Version:4.2.1
	a/- suppression de malloc et free encore utilise

Version:4.3.9
	a/- Masquage des variables "globales"

***********************************************************/

/***************************************************************************/
/*                       DEPENDANT FILES                                   */
/***************************************************************************/
#include "user_opt.h"
#include "fdm_gdm.h"
#include "fdm_gdt.h"



/***************************************************************************/
/*                      PRIVATE CONSTANTS                                  */
/***************************************************************************/


enum  insert_mode { cree , devant , entre , apres  }  ;

#define nil (TEMPO_DEF*)0




/***************************************************************************/
/*                      PRIVATE VARIABLES                                  */
/***************************************************************************/


static TEMPO_DEF *P_ACTUEL ;
static TEMPO_DEF *P_INIT;





/***************************************************************************/
/*                     PRIVATE FUNCTIONS                                   */
/***************************************************************************/
static void _insert_maille ( int  ,TEMPO_DEF  * , TEMPO_DEF *  ) ;



/************************ _insert_maille () ********************************/
/*                                                                         */
/***************************************************************************/
static void _insert_maille (
		int comment ,TEMPO_DEF  * ou , TEMPO_DEF * nouveau )
{
    TEMPO_DEF *Pc ;

    switch ( comment ) {
	case cree:              /* creation premier objet */
	nouveau->Next = nil;
	nouveau->Prec = nil;
	P_ACTUEL = nouveau ;
	break;

	case devant:            /* insert devant le premier objet */
	nouveau->Next = ou ;
	nouveau->Prec = nil;
	ou->Prec = nouveau ;
	P_ACTUEL = nouveau ;
	break;

	case entre:                     /* inserte entre 2 objets */
	Pc =  ou->Next ;
	nouveau->Prec = ou;
	nouveau->Next = Pc ;
	ou->Next = nouveau ;
	Pc->Prec = nouveau ;
	break;

	case apres:                     /* append au dernier objet */
	nouveau->Next = nil;
	nouveau->Prec = ou;
	ou->Next   = nouveau ;
	break;
    }
}




/************************ _TIMER_INITIALIZE () *****************************/
/*                                                                         */
/***************************************************************************/
static TEMPO_DEF Tempo_init;

void _TIMER_INITIALIZE ( void )
{
    P_INIT = P_ACTUEL = &Tempo_init;
    memset( (void *)P_ACTUEL , 0 , sizeof(TEMPO_DEF));
}



/************************** _TIMER_CREATE () *******************************/
/*                                                                         */
/***************************************************************************/
TEMPO_DEF *_TIMER_CREATE ( MEMORY_RN *Rn  )
{
    TEMPO_DEF *Pc ;

    OS_Allocate( Rn , TEMPO_DEF*, Pc ,  sizeof(TEMPO_DEF));
    if (Pc != ( TEMPO_DEF* ) 0 ) {
	memset( (void *)Pc , 0 , sizeof(TEMPO_DEF) );
	Pc->Rn = Rn;
    }
    return Pc ;

}





/************************** _TIMER_START () ********************************/
/*                                                                         */
/***************************************************************************/
int _TIMER_START ( TEMPO_DEF *p)
{
    U_long i,v0;
    TEMPO_DEF *Pc ;
    int comment;
	/*
	0 : creation
	1 : mettre devant le 1er
	2 : mettre apres le dernier
	3 : mettre entre
	*/

    /* controles d'entree */
    if (p->T_init == 0 ) return -1 ;
    if (p->_.Mode) {
	    if (p->_.Periodique) return -1 ;
    }

    if ( !p->_.Type_out ) {
    	if ( p->User_Prog == (void(*) ( struct _TEMPO_DEF *) ) 0 )
	    return -1;
    }
    else {
     	if ( p->File == (INTEG_FILE_TYPE*) 0 )
     	return -1;
    }

    OS_Enter_Region();

    if (p->_.GDT_occup != 0 ) {
	OS_Leave_Region () ;
	return -1;
    }

    Pc = P_ACTUEL ;
    if ( Pc == (TEMPO_DEF *) 0 ) {
	OS_Leave_Region () ;
	return -1;
    }

    v0 = p->T_init ;
    p->_.GDT_occup = 1;
    p->_.In_File   = 0;


    for( i = Pc->T_cur  ; i < v0 ; i+= Pc->T_cur ) {
	if ( Pc->Next == nil ) break;

	Pc = Pc->Next ;

    }

    if (p->_.Mode) {
	while ( v0 == i ) {
	    v0++;
	    if ( Pc->Next != nil ) {
		for(    ; i < v0 ; i+= Pc->T_cur )       {
		    if ( Pc->Next != nil )
			 Pc = Pc->Next ;
		    else break;
		}
	    }
	    else break;
	}
    }
    if ( v0 < i) {
	if ( Pc->Prec != nil ) {
	    p->T_cur = Pc->T_cur - (i - v0);
	    Pc->T_cur = i - v0 ;
	    Pc = Pc->Prec ;
	    comment = entre ;
	} else {
	    Pc->T_cur = i - v0 ;
	    p->T_cur = v0 ;
	    comment = devant;
	}
    } else {
	p->T_cur = v0 - i;
	if ( v0 != i ) {
	    comment = (P_ACTUEL == P_INIT) ? cree : apres ;
	} else {
		comment = ( Pc->Next == nil ) ?  apres : entre;
	}
    }
    _insert_maille( comment , Pc, p) ;
    OS_Leave_Region () ;
    return 0;
}



/************************** _TIMER_STOP () *********************************/
/*                                                                         */
/***************************************************************************/
void _TIMER_STOP ( TEMPO_DEF *p)
{
    TEMPO_DEF *Pp , *Pn ;

    if ( p == (TEMPO_DEF *) 0) return ;

    OS_Enter_Region() ;

    if (p->_.GDT_occup == 0 ) {
	OS_Leave_Region () ;
	return ;
    }
    p->_.GDT_occup = 0;
    p->_.In_File    = 1;

    Pn = p->Next ;
    if ( Pn != nil ) {
	if ( p->Prec == nil ) {
	    /* on vire la premiere */
	    Pn->Prec = nil;
	    Pn->T_cur += p->T_cur ;
	    P_ACTUEL = Pn;
	} else {
	    /* supprime entre */
	    Pp         = p->Prec ;
	    Pn->Prec   = Pp;
	    Pp->Next   = Pn ;
	    Pn->T_cur += p->T_cur ;
	}
    } else {
	/* on vire la tempo en derniere position */
	if (p->Prec == nil)
	    P_ACTUEL = P_INIT;
	else
	    (p->Prec)->Next = nil;
    }
    if ( p->_.Choix_mort ) {
	 OS_Free( p->Rn  , p );/* destruction automatique */
    } else {
	if ( p->_.User_Mode_mort ) {
	   if ( !p->_.Type_out ) {
	       p->_.State = 1;/* reveil appli avec indic tuer*/
   			OS_Leave_Region () ;
			(p->User_Prog)(p) ;
			OS_Enter_Region() ;
	   }
	   else {
		STORE_IN_FIFO_INTEG( p->File , (INTEG_FILE_ELT *) p) ;
	   }
	}
    }
    OS_Leave_Region () ;
}

/************************** _TIMER_STOP () *********************************/
/*                                                                         */
/***************************************************************************/
void _TIMER_KILL ( TEMPO_DEF *p)
{
    TEMPO_DEF *Pp , *Pn ;

    if ( p == (TEMPO_DEF *) 0) return ;

    OS_Enter_Region() ;

    if (p->_.GDT_occup == 0 ) {
	OS_Leave_Region () ;
	return ;
    }
    p->_.GDT_occup = 0;
    p->_.In_File    = 1;

    Pn = p->Next ;
    if ( Pn != nil ) {
	if ( p->Prec == nil ) {
	    /* on vire la premiere */
	    Pn->Prec = nil;
	    Pn->T_cur += p->T_cur ;
	    P_ACTUEL = Pn;
	} else {
	    /* supprime entre */
	    Pp         = p->Prec ;
	    Pn->Prec   = Pp;
	    Pp->Next   = Pn ;
	    Pn->T_cur += p->T_cur ;
	}
    } else {
	/* on vire la tempo en derniere position */
	if (p->Prec == nil)
	    P_ACTUEL = P_INIT;
	else
	    (p->Prec)->Next = nil;
    }
    if ( p->_.Choix_mort ) {
	 OS_Free( p->Rn  , p );/* destruction automatique */
    }
    OS_Leave_Region () ;
}



/************************* _TIMER_RETRIG () ********************************/
/*                                                                         */
/***************************************************************************/
void _TIMER_RETRIG ( TEMPO_DEF *p)
{
    TEMPO_DEF *Pp , *Pn ;

    OS_Enter_Region() ;

    if (p->_.GDT_occup != 0 ) {/* stopper la tempo */
	p->_.GDT_occup = 0;
	Pn = p->Next ;
	if ( Pn != nil ) {
	    if ( p->Prec == nil ) {
	       /* on vire la premiere */
		Pn->Prec = nil;
		Pn->T_cur += p->T_cur ;
		P_ACTUEL = Pn;
	    } else {
		/* supprime entre */
		Pp = p->Prec ;
		Pn->Prec = Pp;
		Pp->Next = Pn ;
		Pn->T_cur += p->T_cur ;
	    }
	} else{
	    /* on vire la tempo en derniere position */
	    if (p->Prec == nil)
	       P_ACTUEL = P_INIT;
	    else
	       (p->Prec)->Next = nil;
	}
    }
    OS_Leave_Region () ;
    _TIMER_START ( p );
}





/*********************** _TIMER_UP_TO_DATE () ******************************/
/*                                                                         */
/***************************************************************************/
void _TIMER_UP_TO_DATE ( void )
{
    TEMPO_DEF *Pc ;

    OS_Enter_Region() ;

    Pc = P_ACTUEL ;

    if ( Pc->T_cur != 0 ) {
	Pc->T_cur--;
	if ( Pc->T_cur == 0 ) { /* tempo echu */
	    do {
		if ( Pc->Next != nil ) {
		    P_ACTUEL = Pc->Next;
		    P_ACTUEL->Prec = nil;
		} else
		    P_ACTUEL = P_INIT;
		Pc->_.GDT_occup = 0;
		Pc->_.In_File   = 1;
		Pc->_.State = 0;
		if ( !Pc->_.Type_out ) {
   			OS_Leave_Region () ;
		  	(Pc->User_Prog)(Pc) ;
    			OS_Enter_Region() ;

		}
		else {
		  STORE_IN_FIFO_INTEG( Pc->File , (INTEG_FILE_ELT *) Pc) ;
		}

		if ( Pc->_.Periodique ) {
		    P_ACTUEL->Prec = nil;
		    if ( Pc->_.Vie_limite ) {
			Pc->Duree_vie--;
			if ( Pc->Duree_vie != 0 ){
   				OS_Leave_Region () ;
				 _TIMER_START ( Pc ) ;
    				OS_Enter_Region() ;
			}
			if ( Pc->Duree_vie == 0 )
				 OS_Free( Pc->Rn  , Pc );
		    }
		    else {
   			OS_Leave_Region () ;
			_TIMER_START ( Pc ) ;
    			OS_Enter_Region() ;
		   }
		}

		Pc = P_ACTUEL ;

		/* si tempo suivante echue := on boucle*/
		if ( P_ACTUEL == P_INIT ) break;

	    } while ( Pc->T_cur == 0 ) ;
	}
    }
    OS_Leave_Region () ;
}


/*********************** _TIMER_RETRIG_ANYWAY () ***************************/
/*                                                                         */
/***************************************************************************/
#define ELT_NULL (INTEG_FILE_ELT*)0

void _TIMER_RETRIG_ANYWAY ( TEMPO_DEF *p)
{

    TEMPO_DEF *Pp , *Pn ;

    OS_Enter_Region() ;

	/******* timer stop *******/
    if (p->_.GDT_occup != 0 ) {/* stopper la tempo */
	p->_.GDT_occup = 0;
	Pn = p->Next ;
	if ( Pn != nil ) {
	    if ( p->Prec == nil ) {
	       /* on vire la premiere */
		Pn->Prec = nil;
		Pn->T_cur += p->T_cur ;
		P_ACTUEL = Pn;
	    } else {
		/* supprime entre */
		Pp = p->Prec ;
		Pn->Prec = Pp;
		Pp->Next = Pn ;
		Pn->T_cur += p->T_cur ;
	    }
	} else{
	    /* on vire la tempo en derniere position */
	    if (p->Prec == nil)
	       P_ACTUEL = P_INIT;
	    else
	       (p->Prec)->Next = nil;
	}
    }

    if (p->_.In_File == 1){

	/*--- extrait fofo integration -*/
	    INTEG_FILE_ELT           *Pn, *Pp;
	    INTEG_FILE_TYPE *File_Integ;
	    INTEG_FILE_ELT * Elt ;

	    File_Integ = p->File ;
	    Elt = (INTEG_FILE_ELT *) p ;

	    if ( Elt->Ptr_Next != ELT_NULL) {
		/*on vire le premier*/
		if (Elt->Ptr_Prev ==  ELT_NULL  ){
		    Pn 		 = Elt->Ptr_Next;
		    Pn->Ptr_Prev = ELT_NULL;
		    File_Integ->Head_Ptr=Pn;
		}else{   /*extrait entre*/
		    Pp 			= Elt->Ptr_Prev;
		    Pn			= Elt->Ptr_Next;
		    Pn->Ptr_Prev	= Pp;
		    Pp->Ptr_Next	= Pn;
		}
	    }else{ /*extrait dernier*/
		Pp			= Elt->Ptr_Prev;
		if (Pp == ELT_NULL ) {
		   File_Integ->Queue_Ptr= Pp ;
		   File_Integ->Head_Ptr	= Pp;
		}
		else {
		  Pp->Ptr_Next 		= ELT_NULL;
		  File_Integ->Queue_Ptr	= Pp;
		}
	    }
	    File_Integ->Nbr_Of_Elt--;
	    if (File_Integ->Nbr_Of_Elt ==0){
		(* ( File_Integ->P_Software_State_Word) ) &=
		   ~(File_Integ->Mask_For_Place_In_Software);
	    }
	  }/* fin if */
   OS_Leave_Region () ;
   _TIMER_START(p);
}

/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/

