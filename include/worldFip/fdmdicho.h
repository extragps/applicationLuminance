/************************************************************
 
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
  
  
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmdicho.h
  Description     : B_A_O
                    Fichier entete pour Gestion Dichotomie
                   
 
  Date de creation: Fri Mar 15 15:11:31 MET 1996
  
***********************************************************/

#ifndef ___dicho
#define ___dicho

#include "user_opt.h"
#include "gdm_os.h"


/***************************************************************************/
/*			PUBLIC CONSTANTS				   */
/***************************************************************************/
#define nil_tab (_TABLE *) 0
#define nil_ele (element *) 0
#define nil_use (char *) 0
	


/***************************************************************************/
/*			PUBLIC TYPES					   */
/***************************************************************************/
typedef struct {
		long objet_num;
		void * ptr;
		} element ;
		
typedef struct {
	int mode;
	int nombre;
	MEMORY_RN  *ref_memory;
	element k[1] ;
	} _TABLE;


/***************************************************************************/
/*			EXTERNAL FUNCTIONS				   */
/***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
 void OS_Leave_Region( void ) ;
 void OS_Enter_Region( void ) ;
#ifdef __cplusplus
}
#endif




/***************************************************************************/
/*		       REPORTS						   */
/***************************************************************************/

enum __dicho_result{
   __dicho_result_ok,
   __dicho_already_existing_elt,
   __dicho_overflow,
   __dicho_extend_memory_lack,
   __dicho_create_memory_lack
   };




/***************************************************************************/
/*		       PUBLIC FUNCTIONS					   */
/***************************************************************************/

void *_DICHO_SCAN_OBJECT( _TABLE ** , long	) ;
 
void *INTERNAL_SCAN_OBJECT( _TABLE ** , long  ) ;

enum __dicho_result _DICHO_ADD_OBJECT ( _TABLE **	, element * );

void * _DICHO_SUP_OBJECT ( _TABLE **	, long	) ;

enum __dicho_result _DICHO_CREATE_FIX ( MEMORY_RN *, _TABLE ** , int	) ;

enum __dicho_result _DICHO_CREATE_VAR ( MEMORY_RN *, _TABLE **	) ;

void _DICHO_REMOVE (  _TABLE ** ) ;


#endif
/*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*/
