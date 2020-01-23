/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm_gdt.h
  Description     : B_A_O
                    Fichier entete pour Gestion des Timers



  Date de creation: Fri Mar 15 15:11:10 MET 1996

Version:4.2.1
	a/- suppression de malloc et free encore utilise

***********************************************************/


#ifndef ___gdt
#define ___gdt


/***************************************************************************/
/*                    DEPENDANT FILES                                      */
/***************************************************************************/

#include "fdm_lai.h"



/***************************************************************************/
/*                     PUBLIC CONSTANTS                                    */
/***************************************************************************/
#define U_long     unsigned long
#define U_short    unsigned short
#define Boolean    1



/***************************************************************************/
/*                     PUBLIC TYPES                                        */
/***************************************************************************/
typedef struct _TEMPO_DEF {
	struct _TEMPO_DEF       *Next;
	struct _TEMPO_DEF       *Prec;
	U_long                          T_init;
	U_long                          T_cur;
	U_short                         Duree_vie;
	struct {
		unsigned Reserved       : 7;
		unsigned Type_out       : Boolean;
		unsigned In_File        : Boolean ;
		unsigned GDT_occup      : Boolean ;
		unsigned Choix_mort     : Boolean ;
		unsigned User_Mode_mort : Boolean ;
		unsigned Mode           : Boolean ;
		unsigned Periodique     : Boolean ;
		unsigned Vie_limite     : Boolean ;
		unsigned State          : Boolean ;
		} _ ;
	/*****user area ***********/
	void (*User_Prog) 	( struct _TEMPO_DEF * ) ;
	INTEG_FILE_TYPE 	* File;
	MEMORY_RN 		*Rn ;
	} TEMPO_DEF ;



/***************************************************************************/
/*                     PUBLIC FUNCTIONS                                    */
/***************************************************************************/
#if __cplusplus
extern "C" {
#endif
 void        _TIMER_INITIALIZE ( void );

 TEMPO_DEF *_TIMER_CREATE ( MEMORY_RN *type  );

 int        _TIMER_START ( TEMPO_DEF *);

 void       _TIMER_STOP ( TEMPO_DEF *);

 void       _TIMER_UP_TO_DATE ( void );

 void       _TIMER_RETRIG ( TEMPO_DEF *);

 void       _TIMER_RETRIG_ANYWAY ( TEMPO_DEF *);

 void 	    _TIMER_KILL ( TEMPO_DEF *p);

#if __cplusplus
}
#endif

#define IS_TEMPO_CLEAN(T)		\
	 ((( T ->_.GDT_occup ) | (  T ->_.In_File ) ) == 0) ? 1 : 0



/***************************************************************************/
/*                     EXTERNAL FUNCTIONS                                  */
/***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
 void OS_Leave_Region( void ) ;
 void OS_Enter_Region( void ) ;
#ifdef __cplusplus
}
#endif


#endif
/*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*/
