/************************************************************
   
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcs_os.h
  Description     : contient les prototypes des procedures permettant
                    de creer et deleter les semaphores dans l'environnement
					cible choisi. 
                    Les semaphores crees sont des semaphores BINAIRES

				   Le code correspondant a ces fonctions est dans le fichier
                   mcs_os.c, qui est modifiable par l'integrateur en fonction
                   de ces besoins. 
					
					Cas particulier:
					Si option _STACK_WITH_WindowsNT 
					on est mappe sur  Fdm_NT :
					Prototypes supplementaires :
							void OS_Init_Region(void);
							void  OS_Enter_Region(void );
							void  OS_Leave_Region(void );

            
 
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000 


************************************************************************/
#ifndef __mcs_os_h
#define __mcs_os_h

#include "mcs_opt.h"

typedef enum {
    _SemaOK = 0 ,
    _SemaNOK 
   } _SEMA_RESULT ; 

#if (_STACK_WITH_WindowsNT == _MCS_OPTION_NO )
#include "user_opt.h"
#include "fdm_os.h"

typedef	unsigned long	SEMAPHORE_TYPE;


/************************************************************************/ 
/*		         Fonctions semaphoresbinaires		                    */
/************************************************************************/ 
/* creer un semaphore binaire - 
en entree : pointeur sur semaphore cree
	  : pointeur sur le nom du semaphore : min = 4 caracteres  */
_SEMA_RESULT OS_BinarySemaphoreCreate( SEMAPHORE_TYPE * , char* );

/* detruire un semaphore */
_SEMA_RESULT OS_SemaphoreDelete( SEMAPHORE_TYPE );

#endif

#if (_STACK_WITH_WindowsNT == _MCS_OPTION_YES)
/*=======================================================================*/
/******			MCS sous Windows NT					***************/
/*=======================================================================*/
#include <windows.h>
typedef	HANDLE	SEMAPHORE_TYPE;

void  OS_Init_Region(void);
void  OS_Enter_Region(void );
void  OS_Leave_Region(void );
void  Semaphore_take( SEMAPHORE_TYPE);
void  Semaphore_release( SEMAPHORE_TYPE ) ;


/* creer un semaphore binaire - 
en entree : pointeur sur semaphore cree
	  : pointeur sur le nom du semaphore : min = 4 caracteres  */
_SEMA_RESULT OS_BinarySemaphoreCreate( SEMAPHORE_TYPE * , char* );
/* detruire un semaphore */
_SEMA_RESULT OS_SemaphoreDelete( SEMAPHORE_TYPE );
/* prendre un semaphore */
void  SemaphoreTake  ( SEMAPHORE_TYPE );
/* liberer un semaphore */
void  SemaphoreRelease( SEMAPHORE_TYPE  );
#endif
#endif

