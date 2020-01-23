/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4
  Nom du fichier  : fdm_os.h
  Description     : Macros d'interface Operating system A COMPLETER EVENTUELLEMENT PAR UTILISATEUR SELON SON OS



  Date de creation: Fri Mar 15 15:11:16 MET 1996

***********************************************************/

#ifndef __fdmos_h
#define __fdmos_h

extern void _FIP_PAUSE  ( Ulong );

#if					( FDM_WITH_PSOS == YES )
#include "psos.h"
#include "prepc.h"
#endif

#if					( FDM_WITH_GNU == YES )
#include <stdlib.h>
#include <string.h>
#endif

#if 					(  FDM_WITH_NDIS == YES )
#include <stdio.h>
#include <string.h>
#endif

#if 					(  FDM_WITH_MICROSOFT == YES )
#include <stdio.h>
#include <string.h>
#endif

#if 					(  FDM_WITH_RTX == YES )

#include "windows.h"
#include "stdio.h"
#include "rtapi.h"
#endif

#if 					(  FDM_WITH_SOLARIS == YES )
#include <sys/types.h>
#include <sys/ddi.h>

#ifdef _KERNEL
void memcpy(void *, void * , int );
#endif

/* attention bcmp retourne 0 ou 1 */
#define memcpy(s1,s2,n) bcopy(s2,s1, n)
#define memcmp(s1,s2,n) bcmp(s1,s2,n)

#endif

#if 					(  FDM_WITH_METAWARE == YES )
#include <stdio.h>
#include <string.h>
#endif

/***********
Utilisation des semaphores:

 	fdm_sm_p   protege le composant FF
	fdm_sm_p_t protege les changements de base ( Switch , Start ... )
	fdm_sm_p_bd protege L'acces a la Base de Donnees fdm
  Acces libres
	fdm_sm_p_vcom protege Read/Write Vcom

***********/
#if					( FDM_WITH_SEMAPHORE == NO )

typedef  int __Object_Semaphore_Type__;

#define OS_fdm_sm_p
#define OS_fdm_sm_v
#define OS_fdm_sm_p_t(Ref)
#define OS_fdm_sm_v_t(Ref)
#define OS_fdm_sm_p_bd(Ref)
#define OS_fdm_sm_v_bd(Ref)
#define OS_fdm_sm_p_vcom
#define OS_fdm_sm_v_vcom

#else

#if (  (FDM_WITH_PSOS    == YES ) \
    || (FDM_WITH_VXWORKS == YES ) \
    || (FDM_WITH_SOLARIS == YES ) \
    || (FDM_WITH_NDIS    == YES ) \
    || (FDM_WITH_METAWARE== YES ) \
    || (FDM_WITH_SCOUNIX == YES ) )


#if (  FDM_WITH_SCOUNIX == YES ) 
typedef  void * __Object_Semaphore_Type__;
#else
typedef  unsigned long __Object_Semaphore_Type__;
#endif

#ifdef __cplusplus
extern "C" {
#endif

 void OS_Leave_Region( void ) ;
 void OS_Enter_Region( void ) ;
 void OS_semaphore_take   ( __Object_Semaphore_Type__   );
 void OS_semaphore_release( __Object_Semaphore_Type__   );

#ifdef __cplusplus
}
#endif

#define OS_fdm_sm_v \
	OS_semaphore_release( Ref->FDM_Internal_Use.Semaphore );

#define OS_fdm_sm_p \
	OS_semaphore_take( Ref->FDM_Internal_Use.Semaphore  );

#define OS_fdm_sm_v_t(Ref) \
	OS_semaphore_release( (Ref)->FDM_Internal_Use.Task_Semaphore );

#define OS_fdm_sm_p_t(Ref) \
	OS_semaphore_take( (Ref)->FDM_Internal_Use.Task_Semaphore );


#define OS_fdm_sm_v_bd(Ref) \
	OS_semaphore_release( (Ref)->FDM_Internal_Use.Modif_BD_Semaphore );

#define OS_fdm_sm_p_bd(Ref) \
	OS_semaphore_take( Ref->FDM_Internal_Use.Modif_BD_Semaphore  );

#if			( FDM_WITH_FREE_ACCESS == YES )

#define OS_fdm_sm_v_vcom \
	OS_semaphore_release( Ref->FDM_Internal_Use.Vcom );

#define OS_fdm_sm_p_vcom \
	OS_semaphore_take( Ref->FDM_Internal_Use.Vcom  );



#endif
#endif


#if ( FDM_WITH_NT == YES  )
#include <ntddk.h>

typedef struct {
	KSPIN_LOCK	SPL;
	KIRQL		IRQL;
	} __Object_Mutex_Type__;

typedef  KSEMAPHORE 	__Object_Semaphore_Type__;

void OS_semaphore_take   ( __Object_Semaphore_Type__ *t );
void OS_semaphore_release( __Object_Semaphore_Type__ *t ) ;

void OS_mutex_take   ( __Object_Mutex_Type__ *t );
void OS_mutex_release( __Object_Mutex_Type__ *t ) ;

#define OS_fdm_sm_v \
	OS_mutex_release( &(Ref)->FDM_Internal_Use.Semaphore );

#define OS_fdm_sm_p \
	OS_mutex_take   ( &(Ref)->FDM_Internal_Use.Semaphore  );

#define OS_fdm_sm_v_t(Ref) \
	OS_mutex_release( &(Ref)->FDM_Internal_Use.Task_Semaphore );

#define OS_fdm_sm_p_t(Ref) \
	OS_mutex_take(    &(Ref)->FDM_Internal_Use.Task_Semaphore );


#define OS_fdm_sm_v_bd(Ref) \
	OS_mutex_release( &(Ref)->FDM_Internal_Use.Modif_BD_Semaphore );

#define OS_fdm_sm_p_bd(Ref) \
	OS_mutex_take(    &(Ref)->FDM_Internal_Use.Modif_BD_Semaphore  );

#if			( FDM_WITH_FREE_ACCESS == YES )

#define OS_fdm_sm_v_vcom \
	OS_semaphore_release( &(Ref)->FDM_Internal_Use.Vcom );

#define OS_fdm_sm_p_vcom \
	OS_semaphore_take(    &(Ref)->FDM_Internal_Use.Vcom  );


#endif


#endif

#if ((FDM_WITH_RTX == YES)  )

typedef  HANDLE __Object_Semaphore_Type__;

#ifdef __cplusplus
extern "C" {
#endif
 void OS_Leave_Region( void ) ;
 void OS_Enter_Region( void ) ;
 void OS_semaphore_take   ( __Object_Semaphore_Type__ t );
 void OS_semaphore_release( __Object_Semaphore_Type__ t ) ;
#ifdef __cplusplus
}
#endif


#define OS_fdm_sm_v \
	OS_semaphore_release( Ref->FDM_Internal_Use.Semaphore );

#define OS_fdm_sm_p \
	OS_semaphore_take( Ref->FDM_Internal_Use.Semaphore  );

#define OS_fdm_sm_v_t(Ref) \
	OS_semaphore_release( (Ref)->FDM_Internal_Use.Task_Semaphore );

#define OS_fdm_sm_p_t(Ref) \
	OS_semaphore_take( (Ref)->FDM_Internal_Use.Task_Semaphore );


#define OS_fdm_sm_v_bd(Ref) \
	OS_semaphore_release( (Ref)->FDM_Internal_Use.Modif_BD_Semaphore );

#define OS_fdm_sm_p_bd(Ref) \
	OS_semaphore_take( Ref->FDM_Internal_Use.Modif_BD_Semaphore  );

#if			( FDM_WITH_FREE_ACCESS == YES )

#define OS_fdm_sm_v_vcom \
	OS_semaphore_release( Ref->FDM_Internal_Use.Vcom );

#define OS_fdm_sm_p_vcom \
	OS_semaphore_take( Ref->FDM_Internal_Use.Vcom  );


#endif
#endif  /* rtx */

#endif	/* sema oui */
#endif  /* file */

