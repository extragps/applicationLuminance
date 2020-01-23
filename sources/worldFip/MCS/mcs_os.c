/************************************************************ 
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : mcs_os.c
					
  Description     :		
		  - Contient le code pour des procedures permettant
             d'appeler la creation et destruction des semaphores                                       
            Les semaphores crees sont des semaphores BINAIRES

		    Le code correspondant a ces fonctions est modifiable
            par l'utilisateur, dans le cas ou l'integration ne se fait 
            pas dans les environnement connus a ce jour 
			
			 Dans le cas ou on est mappe sur FDM_V4
			 Voir aussi les fichiers User_opt.h et fdm_oc.c

				Note Pour 	Windows NT	:
				Contient en plus le code pour des procedures permettant
                    d'utiliser les sections critiques 
					OS_ENTER_REGIon et OS_LEAVE_REGION 
	                   
 
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000 


************************************************************************/


#include "mcs_opt.h"
#include "mcs_os.h"


#if (_STACK_WITH_WindowsNT == _MCS_OPTION_NO )

#if ( FDM_WITH_SEMAPHORE == NO )
/*=======================================================================*/
/******	sans semaphore 				        *****************/
/*=======================================================================*/
 
_SEMA_RESULT OS_BinarySemaphoreCreate( SEMAPHORE_TYPE * Semaphore,char *name) 
{ return _SemaOK;}

_SEMA_RESULT OS_SemaphoreDelete(SEMAPHORE_TYPE Semaphore)
{ return _SemaOK;}

#else 


#if	( FDM_WITH_PSOS == YES )
/*=======================================================================*/
/******			PSOS				*****************/
/*=======================================================================*/
#define INITIAL_COUNT 1
/*-------------------------------------*/
_SEMA_RESULT OS_BinarySemaphoreCreate( SEMAPHORE_TYPE * Semaphore,char *name) 
/* name pointe sur une chain de 4 caracteres minimum */					 
{
 char SemaphoreName[5];
 int i;
 unsigned long Cr;

   for (i=0 ;i<4 ;i++){
     SemaphoreName[i]=name[i];
   }
   SemaphoreName[4] ='\0';
   Cr=sm_create(  SemaphoreName   , INITIAL_COUNT , SM_PRIOR ,Semaphore);
   if ( Cr != 0 ){
       return _SemaNOK;
   }
   return _SemaOK;
}

/*-------------------------------------*/
_SEMA_RESULT OS_SemaphoreDelete(SEMAPHORE_TYPE Semaphore)
{
    sm_delete(Semaphore);
    return _SemaOK;
}
#endif

/* *******************************************************************************************/
/*******************************************************************************************/
/*************************   SCOUNIX *******************************************************/
/*******************************************************************************************/
/*******************************************************************************************/

#if																	( FDM_WITH_SCOUNIX == YES )

// #include "fdm_kd.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>		/* errno */
#include <stdio.h>
#include "kcommon.h"


/*-------------------------------------*/
_SEMA_RESULT OS_BinarySemaphoreCreate( SEMAPHORE_TYPE * Semaphore,char *name) 
/* name pointe sur une chain de 4 caracteres minimum */					 
{
Kmutex *mutex_pt=NULL;
	mutex_pt=kmutexNewExt(name,false);
	if(NULL!=mutex_pt)
	{
		*Semaphore=mutex_pt;
	   return _SemaOK;
	}
	else
	{
       return _SemaNOK;
	}
}

/*-------------------------------------*/
_SEMA_RESULT OS_SemaphoreDelete(SEMAPHORE_TYPE Semaphore)
{
	/*.........*/
	kmutexDelete((Kmutex *)Semaphore);
    return _SemaOK;
}
#endif


#if			( FDM_WITH_VXWORKS == YES )

#include "semLib.h"
#include "intLib.h"

/*=======================================================================*/
/******			V x W o r k s				*****************/
/*=======================================================================*/
_SEMA_RESULT OS_BinarySemaphoreCreate( SEMAPHORE_TYPE *Semaphore,char *name) 
{
_SEMA_RESULT retour=_SemaOK;
    *Semaphore = (SEMAPHORE_TYPE)semBCreate(  SEM_Q_FIFO, SEM_FULL ) ;
    if( *Semaphore == (SEMAPHORE_TYPE)NULL ) 
	{
    	retour=_SemaNOK;
    }
    return retour;
}

_SEMA_RESULT OS_SemaphoreDelete(SEMAPHORE_TYPE Semaphore)
{ 
_SEMA_RESULT retour=_SemaOK;

	if(semDelete((SEM_ID)Semaphore)==ERROR)
	{
		retour=_SemaNOK;
	}
	return retour;
}
#endif



#if ( 	( FDM_WITH_VXWORKS == NO  ) && 	( FDM_WITH_PSOS == NO )  && (FDM_WITH_SCOUNIX == NO ))

/*=======================================================================*/
/******			Autres OS			*****************/
/*=======================================================================*/
/* write here your macros for
   -create semaphore
   -delete semaphore
 */

_SEMA_RESULT OS_BinarySemaphoreCreate( SEMAPHORE_TYPE * Semaphore,char *name) 
{/* a ecrire par user*/
	return _SemaNOK;}

/*-------------------------------------*/
_SEMA_RESULT OS_SemaphoreDelete(SEMAPHORE_TYPE Semaphore)
{/* a ecrire par user*/
	return _SemaNOK; }

/*-------------------------------------*/
#endif

#endif
#endif



#if (_STACK_WITH_WindowsNT == _MCS_OPTION_YES )
/*=======================================================================*/
/******		MCS-->Window_NT FDM_ NT			       ***************/
/*=======================================================================*/
/* =======  sections critiques  ==============*/
#include <windows.h>

CRITICAL_SECTION  bao_critical_section;

void OS_Init_Region(void)
{
InitializeCriticalSection(&bao_critical_section);
}

void  OS_Enter_Region(void ){
	EnterCriticalSection(&bao_critical_section);
}

void  OS_Leave_Region(void ){
	LeaveCriticalSection(&bao_critical_section);
}

/* =======semaphores ==============*/

_SEMA_RESULT OS_BinarySemaphoreCreate( SEMAPHORE_TYPE * Semaphore,char *name) 
{
    *Semaphore= CreateSemaphore( NULL,1,1,NULL);
    if( *Semaphore == NULL ) {
           return _SemaNOK;
    }
    return _SemaOK;
}

/*-------------------------------------*/
_SEMA_RESULT OS_SemaphoreDelete(SEMAPHORE_TYPE Semaphore)
{
  BOOL Cr ;	
	Cr =  CloseHandle(Semaphore);
	if (Cr == TRUE)
		return _SemaOK;
    else	        
		return _SemaNOK;
}

/*-------------------------------------*/
void SemaphoreTake(  SEMAPHORE_TYPE   Semaphore)
{
	DWORD  Cr;
	Cr= WaitForSingleObject(Semaphore,INFINITE);    
}
/*-------------------------------------*/
void SemaphoreRelease( SEMAPHORE_TYPE  Semaphore)
{
  BOOL Cr ;	
	Cr =  ReleaseSemaphore(Semaphore,1,NULL);
}	

#endif
