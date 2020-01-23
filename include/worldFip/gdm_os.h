/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
    

  
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : gdm_os.h
  Description     : Gestion des partitions memoiresA COMPLETER EVENTUELLEMENT PAR UTILISATEUR SELON SON OS
                   
                   
  
  Date de creation: Fri Mar 15 15:11:53 MET 1996
  
***********************************************************/

#ifndef __gdm_os_h
#define  __gdm_os_h


#if 	(  FDM_WITH_MICROSOFT == YES ) 
#pragma warning ( disable : 4100)
#endif
 
/*************************************************************/
/********** Custumer Memory Management interface *************/
/*************************************************************/
/* this a pSOS+ example implementation for memory management */
/* if you don't use pSOS+, you can use standard malloc ,free */
/*************************************************************/

typedef struct {
	unsigned long rnid ;
	unsigned long asize ;
	unsigned long Cr ;
} MEMORY_RN;

#if						( FDM_WITH_PSOS == YES )
#include "psos.h"

#define OS_Allocate(Memory_region,type,P,dim) \
	{\
	(Memory_region)->Cr = rn_getseg((Memory_region)->rnid,dim,1,0,(void*)&P);\
	if ( (Memory_region)->Cr!= 0 ) P = (type) 0;\
	}

#define OS_Free(Memory_region,pointeur) \
	(Memory_region)->Cr = rn_retseg((Memory_region)->rnid,pointeur);

#endif

#if					( FDM_WITH_NT	== YES )
#include <ntddk.h>

#define OS_Allocate(Memory_region,type,P,dim) \
    P = (type) ExAllocatePool ( NonPagedPool , (ULONG) dim );

#define OS_Free(Memory_region,pointeur) \
    ExFreePool ((PVOID) pointeur) ;

#endif

#if 	(  FDM_WITH_SOLARIS == YES )

#include <sys/types.h>
#include <sys/kmem.h>
#include <sys/param.h>
#include <sys/cmn_err.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
void *memset(void *s, int c, size_t n);


#define OS_Allocate(Memory_region,type,P,dim)				\
	{								\
	long *temp;							\
       	temp = (long*) kmem_alloc(dim+sizeof(long), KM_NOSLEEP);	\
	*temp++ = dim+MEMORY_RNsizeof(long);					\
	P=(type)temp;							\
	}									\

#define OS_Free(Memory_region,pointeur)				\
	{							\
	long *temp;						\
	temp = (long*)pointeur;					\
	--temp;							\
	kmem_free(temp, *temp);					\
	}

#endif

#if 					(  FDM_WITH_RTX == YES )
#include "windows.h"
#include "stdio.h"
#include "rtapi.h"


#define OS_Allocate(Memory_region,type,P,dim)		\
	P = (type) HeapAlloc((HANDLE) Memory_region, 0 , (DWORD) dim );

#define OS_Free(Memory_region,pointeur) \
	HeapFree((HANDLE) Memory_region,0,(PVOID) pointeur );


#endif

#if 	( FDM_WITH_PSOS == NO ) && ( FDM_WITH_NT == NO ) && (  FDM_WITH_SOLARIS == NO ) && (( FDM_WITH_RTX == NO ))

#include <stdlib.h>
#include <string.h>
#include "kcommon.h"


#define OS_Allocate(Memory_region,type,P,dim)  P = (type) usr_mcs_mess_alloc (NULL, (size_t) dim );

#define OS_Free(Memory_region,pointeur) usr_mcs_mess_free (NULL,(char *) pointeur) ; 

#endif
//
//#if __cplusplus
//extern "C" {
//#endif
//
//MEMORY_RN 
//CREATE_MEMORY_REGION (char * name,
//	void * memory_management_addr,
//	unsigned long memory_length ) ;   
//
//unsigned short  
//DELETE_MEMORY_REGION ( MEMORY_RN* Partition );
//
//#if __cplusplus
//}
//#endif
//

#endif
