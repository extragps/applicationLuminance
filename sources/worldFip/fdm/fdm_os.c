/************************************************************


 " @(#) All rights reserved (c) 1999                                         "
 " @(#) ALSTOM (Paris, France)                                               "
 " @(#) This computer program may not be used, copied, distributed,          "
 " @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
 " @(#) prior written authorization.                                         "

 ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


 Projet      : Fip Device Manager Version 4

 Nom du fichier  : fdm_os.c
 Description     : interface avec Operating system


 Ce fichier gere ce jour :
 -  pSOS+
 -  VxWors
 -  driver NDIS pour Windows NT
 -  driver Windows NT
 -  driver Solaris

 pour vous aidez dans l'integration d'autres OS ,
 contacter: alain.besse@techn.alstom.com


 Date de creation: Fri Sep 13 13:23:00 MET 1996

 ***********************************************************/
#include "user_opt.h"
#include "fdm.h"

#if			( FDM_WITH_PSOS == YES )

#include "psos.h"
#include "prepc.h"

/*******************************************************************************************/
/*******************************************************************************************/
/*************************   p S O S +   ***************************************************/
/*******************************************************************************************/
/*******************************************************************************************/
#define initial_count_0	0
#define initial_count_1	1

int
FDM_ressources_create ( FDM_REF *Ref)
{
	char Buf[20];
	Ulong Cr;

	/*Co*/
	sprintf( Buf , "Co%02X", Ref->H.K_PHYADR);
	Cr = sm_create( Buf , initial_count_1 , SM_PRIOR , &Ref->FDM_Internal_Use.Semaphore);
	if ( Cr != 0 )
	{
		FDM_Signal_Error ( Ref , _SM_FAILED, 1 );
		return FDM_NOK;
	}

	/*Ta*/
	sprintf( Buf , "Ta%02X", Ref->H.K_PHYADR);
	Cr = sm_create( Buf , initial_count_1 , SM_PRIOR , &Ref->FDM_Internal_Use.Task_Semaphore);
	if ( Cr != 0 )
	{
		FDM_Signal_Error ( Ref , _SM_FAILED, 2 );
		return FDM_NOK;
	}

	/*Bd*/
	sprintf( Buf , "Bd%02X", Ref->H.K_PHYADR);
	Cr = sm_create( Buf , initial_count_1 , SM_PRIOR , &Ref->FDM_Internal_Use.Modif_BD_Semaphore);
	if ( Cr != 0 )
	{
		FDM_Signal_Error ( Ref , _SM_FAILED, 3 );
		return FDM_NOK;
	}

#if			( FDM_WITH_FREE_ACCESS == YES )
	/*Va*/
	sprintf( Buf , "Va%02X", Ref->H.K_PHYADR);
	Cr = sm_create( Buf , initial_count_0 , SM_PRIOR , &Ref->FDM_Internal_Use.Vcom);
	if ( Cr != 0 )
	{
		FDM_Signal_Error ( Ref , _SM_FAILED, 4 );
		return FDM_NOK;
	}

#endif
	return FDM_OK;

}

void
FDM_ressources_delete( FDM_REF *Ref)
{
	sm_delete( Ref->FDM_Internal_Use.Task_Semaphore );
	sm_delete( Ref->FDM_Internal_Use.Semaphore );
	sm_delete( Ref->FDM_Internal_Use.Modif_BD_Semaphore );
#if			( FDM_WITH_FREE_ACCESS == YES )
	sm_delete( Ref->FDM_Internal_Use.Vcom );
#endif

}

#ifndef SPLX_VALUE_FOR_MASK_ALL
#error  SPLX_VALUE_FOR_MASK_ALL missing
#endif

static unsigned long Processeur_State;
static short splx_count;

#if 				( FDM_WITH_CAD_UL == YES )
/* processeurs famille x86 */
void
OS_Enter_Region ( void )
{
	Processeur_State = splhigh();
}

void
OS_Leave_Region ( void )
{
	splx(Processeur_State);
}

void _FIP_PAUSE ( Ulong Milliseconde)
{
	Ulong l;
#define Tick_Periode 10

	l = Milliseconde/Tick_Periode;
	l = ( l == 0 ) ? 1:l;
	tm_wkafert( l );
}

#else
/* processeurs famille i960 , 68xxx , ColdFire , PowerPC */
void
OS_Enter_Region ( void )
{
	Processeur_State = splx( SPLX_VALUE_FOR_MASK_ALL );
	splx_count++;
	if ( splx_count != 1)
	{
		int j;
		int i=0;
		j /=i;/* force arith fault si erreur !!!! */
	}
}

void
OS_Leave_Region ( void )
{
	splx_count = (splx_count-1) & 0xffff; /* force arith fault si erreur !!!! */
	Processeur_State = splx(Processeur_State);
}

#endif

void
OS_semaphore_take( Ulong SM_ID )
{
	sm_p( SM_ID , 0 , 0 );
}

void
OS_semaphore_release ( Ulong SM_ID )
{
	sm_v( SM_ID );
}

/*************************************************/
/****** Gestion partitions memoire       *********/
/*************************************************/

MEMORY_RN CREATE_MEMORY_REGION (
		char * name,
		void * memory_management_addr,
		unsigned long memory_length )
{

	MEMORY_RN Ref;
	Ref.Cr = rn_create ( name ,
			memory_management_addr ,
			memory_length, 64 ,

#if			(FDM_WITH_DELETE_RN_OVERRIDE == YES)
			RN_DEL ,
#else
			0 ,
#endif
			&Ref.rnid ,
			&Ref.asize );

	return Ref;
}

unsigned short DELETE_MEMORY_REGION ( MEMORY_RN* Partition )
{
	return (unsigned short)(rn_delete ( Partition->rnid ));
}

#endif

#if			( FDM_WITH_VXWORKS == YES )
#include "semLib.h"
#include "intLib.h"

/*******************************************************************************************/
/*******************************************************************************************/
/*************************   V x W o r k s    **********************************************/
/*******************************************************************************************/
/*******************************************************************************************/

void
OS_semaphore_take( ULONG semId )
{
	semTake( (SEM_ID)semId , WAIT_FOREVER );
}
void
OS_semaphore_release( ULONG semId )
{
	semGive( (SEM_ID)semId );
}

static void
sm_delete( ULONG *pSemId )
{
	semDelete( (SEM_ID)(*pSemId) );
}

void
FDM_ressources_delete ( FDM_REF *Ref)
{
	sm_delete( &Ref->FDM_Internal_Use.Task_Semaphore );
	sm_delete( &Ref->FDM_Internal_Use.Semaphore );
	sm_delete( &Ref->FDM_Internal_Use.Modif_BD_Semaphore );
#if			( FDM_WITH_FREE_ACCESS == YES )
	sm_delete( &Ref->FDM_Internal_Use.Vcom );
#endif

}

int
FDM_ressources_create ( FDM_REF *Ref )
{
	LOCAL SEM_ID ffSemIdCo;
	LOCAL SEM_ID ffSemIdTa;
	LOCAL SEM_ID ffSemIdBd;

	/*Co*/
	ffSemIdCo = semBCreate( SEM_Q_FIFO, SEM_FULL );
	if( ffSemIdCo == NULL )
	{
		FDM_Signal_Error ( Ref , _SM_FAILED, 1 );
		return FDM_NOK;
	}
	Ref->FDM_Internal_Use.Semaphore = (Ulong)ffSemIdCo;

	/*Ta*/
	ffSemIdTa = semBCreate( SEM_Q_FIFO, SEM_FULL );
	if( ffSemIdTa == NULL )
	{
		FDM_Signal_Error ( Ref , _SM_FAILED, 2 );
		return FDM_NOK;
	}
	Ref->FDM_Internal_Use.Task_Semaphore = (Ulong)ffSemIdTa;

	/*Bd*/
	ffSemIdBd = semBCreate( SEM_Q_FIFO, SEM_FULL );
	if( ffSemIdBd == NULL )
	{
		FDM_Signal_Error ( Ref , _SM_FAILED, 3 );
		return FDM_NOK;
	}
	Ref->FDM_Internal_Use.Modif_BD_Semaphore = (Ulong)ffSemIdBd;

#if			( FDM_WITH_FREE_ACCESS == YES )
	/*Va*/
	{
		LOCAL SEM_ID ffSemIdVa;

		ffSemIdVa = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
		if( ffSemIdVa == NULL )
		{
			FDM_Signal_Error ( Ref , _SM_FAILED, 4 );
			return FDM_NOK;
		}
		Ref->FDM_Internal_Use.Vcom = (Ulong)ffSemIdVa;
	}
#endif

	return FDM_OK;
}

LOCAL int lPreviousMask;
void
OS_Enter_Region(void)
{
	lPreviousMask = intLock();
}

void
OS_Leave_Region(void)
{
	intUnlock( lPreviousMask );
}

void _FIP_PAUSE ( Ulong Milliseconde)
{
	ksleep(Milliseconde);
}

#endif

/*******************************************************************************************/
/*******************************************************************************************/
/*************************   N T - 4 *******************************************************/
/*******************************************************************************************/
/*******************************************************************************************/

#if					( FDM_WITH_NT == YES )

#ifndef				FDM_WITH_SEMAPHORE
#error 	   FDM_WITH_SEMAPHORE must be YES
#endif

#if					( FDM_WITH_SEMAPHORE == NO )
#error 	   FDM_WITH_SEMAPHORE must be YES
#endif

void
OS_Leave_Region(void)
{
	__asm sti;
}

void
OS_Enter_Region(void)
{
	_asm cli;
}

int
FDM_ressources_create ( FDM_REF *Ref )
{

	KeInitializeSpinLock(
			&Ref->FDM_Internal_Use.Semaphore.SPL);

	KeInitializeSpinLock(
			&Ref->FDM_Internal_Use.Task_Semaphore.SPL);

	KeInitializeSpinLock(
			&Ref->FDM_Internal_Use.Modif_BD_Semaphore.SPL);

#if			( FDM_WITH_FREE_ACCESS == YES )
	KeInitializeSemaphore(
			&Ref->FDM_Internal_Use.Vcom,
			0L,
			MAXLONG
	);
#endif
	return 0;
}

void
FDM_ressources_delete ( FDM_REF *Ref)
{
	/*.........*/
}

void
OS_semaphore_take( __Object_Semaphore_Type__ *Semaphore )
{
	KeWaitForSingleObject(
			Semaphore,
			Executive,
			KernelMode,
			FALSE,
			NULL
	);

}

void
OS_semaphore_release( __Object_Semaphore_Type__ *Semaphore )
{
	KeReleaseSemaphore( Semaphore , 0 , 1 , FALSE );

}

void
OS_mutex_take( __Object_Mutex_Type__ *SpinLock )
{
	KeAcquireSpinLock( &SpinLock->SPL, &SpinLock->IRQL );
}

void
OS_mutex_release( __Object_Mutex_Type__ *SpinLock )
{
	KeReleaseSpinLock( &SpinLock->SPL, SpinLock->IRQL );
}

void _FIP_PAUSE ( Ulong Milliseconde)
{
	LARGE_INTEGER Tempo;
	Tempo = RtlConvertLongToLargeInteger(-Milliseconde*100000);
	KeDelayExecutionThread( KernelMode,FALSE, &Tempo);

}

#endif	/* Windows NT */

/*******************************************************************************************/
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
#include <fcntl.h>
#include "kcommon.h"


/* compteur OS_region */
/* structure et union necessaire e la gestion des semaphores */
// Mis en commentaire car inutilise a priori.
//static unsigned int cptRegion = 1;
//static struct semid_ds buf;
//static ushort array;
static int localFip=-1;

int FDM_ressources_create(FDM_REF * Ref)
{
Kmutex *mutex_pt=NULL;
	if(-1==localFip)
	{
		localFip=open("/dev/fip",O_RDWR);
	}

	mutex_pt=kmutexNewExt("Task_Semaphore",false);
	if(NULL!=mutex_pt)
	{
		Ref->FDM_Internal_Use.Task_Semaphore = mutex_pt;
	}
	else
	{
		FDM_Signal_Error(Ref, _SM_FAILED, 1);
		return FDM_NOK;
	}
	
	mutex_pt=kmutexNewExt("Use_Semaphore",false);
	if(NULL!=mutex_pt)
	{
		Ref->FDM_Internal_Use.Semaphore = mutex_pt;
	}
	else
	{
		FDM_Signal_Error(Ref, _SM_FAILED, 2);
		return FDM_NOK;
	}
	mutex_pt=kmutexNewExt("BD_Semaphore",false);
	if(NULL!=mutex_pt)
	{
		Ref->FDM_Internal_Use.Modif_BD_Semaphore = mutex_pt;
	}
	else
	{
		FDM_Signal_Error(Ref, _SM_FAILED, 3);
		return FDM_NOK;
	}

	return FDM_OK;
}

void FDM_ressources_delete(FDM_REF *Ref)
{
	/*.........*/
	kmutexDelete((Kmutex *)Ref->FDM_Internal_Use.Task_Semaphore);
	Ref->FDM_Internal_Use.Task_Semaphore=NULL;
	kmutexDelete((Kmutex *)Ref->FDM_Internal_Use.Semaphore);
	Ref->FDM_Internal_Use.Semaphore=NULL;
	kmutexDelete((Kmutex *)Ref->FDM_Internal_Use.Modif_BD_Semaphore);
	Ref->FDM_Internal_Use.Modif_BD_Semaphore=NULL;
	if(-1!=localFip)
	{
		close(localFip);
		localFip=-1;
	}
}

/* write here your macros for take and release semaphore */

/* Prise du semaphore */
int sm_p(void *mutex)
{
	kmutexLock((Kmutex *)mutex);
}

/* Liberation du semaphore */
int sm_v(void *mutex)
{
	kmutexUnlock((Kmutex *)mutex);
	return 0;
}

void OS_semaphore_take(void * SM_ID)
{
	sm_p(SM_ID);
}

void OS_semaphore_release(void * SM_ID)
{
	sm_v(SM_ID);
}

void _FIP_PAUSE(Ulong milli)
{
	usleep(milli * 1000);
}

void OS_Leave_Region(void)
{
	/* write here your code for enable all interrupt */
	if(-1!=localFip)
	{
	char info;
		write(localFip,&info,1);
	}
//	local_irq_enable();
}

void OS_Enter_Region(void)
{
	if(-1!=localFip)
	{
	char info;
		read(localFip,&info,1);
	}
//	local_irq_disable();
}

#endif /* SCO-UNIX */

/*******************************************************************************************/
/*******************************************************************************************/
/*************************   SOLARIS *******************************************************/
/*******************************************************************************************/
/*******************************************************************************************/

#if																( FDM_WITH_SOLARIS == YES )
#include <sys/ksynch.h>
#include "drv.h"
extern int ddi_cpt;

static unsigned int ddic;

ksema_t sp1;
ksema_t sp2;
ksema_t sp3;
ksema_t sp4;
ksema_t sp5;
ksema_t sp6;
ksema_t sp7;
ksema_t sp8;
ksema_t sp9;
ksema_t sp10;
ksema_t sp11;
ksema_t sp12;
ksema_t sp13;
ksema_t sp14;
ksema_t sp15;
ksema_t sp16;

void
OS_Enter_Region(void)
{

	ddic=ddi_enter_critical();
	ddi_cpt++;
	if (ddi_cpt != 1)
	{
		int j;
		int i=0;
		j/=i;
	}
}

void
OS_Leave_Region(void)
{
	ddi_cpt--;
	if (ddi_cpt !=0)
	{
		int j;
		int i=0;
		j/=i;
	}
	ddi_exit_critical(ddic);
}

int
FDM_ressources_create ( FDM_REF *pRef )
{
	struct fdm_state *fdmp;

	fdmp =(struct fdm_state *) (pRef->S).User_Ctxt;

	switch(fdmp->num_card)
	{
		case 0 :
		pRef->FDM_Internal_Use.Semaphore = 1;

		sema_init(&sp1,1,"FDM_S",SEMA_DRIVER,NULL);

		pRef->FDM_Internal_Use.Task_Semaphore = 2;

		sema_init(&sp2,1,"FDM_Task_S",SEMA_DRIVER,NULL);

		pRef->FDM_Internal_Use.Modif_BD_Semaphore = 3;

		sema_init(&sp3,1,"FDM_Modif_BD_S",SEMA_DRIVER,NULL);
		/*.........*/
#if			( FDM_WITH_FREE_ACCESS == YES )
		pRef->FDM_Internal_Use.Vcom = 4;

		sema_init(&sp4,1,"FDM_Vcom",SEMA_DRIVER,NULL);
#endif
		break;

		case 1 :
		pRef->FDM_Internal_Use.Semaphore = 5;

		sema_init(&sp5,1,"FDM_S",SEMA_DRIVER,NULL);

		pRef->FDM_Internal_Use.Task_Semaphore = 6;

		sema_init(&sp6,1,"FDM_Task_S",SEMA_DRIVER,NULL);

		pRef->FDM_Internal_Use.Modif_BD_Semaphore = 7;

		sema_init(&sp7,1,"FDM_Modif_BD_S",SEMA_DRIVER,NULL);
		/*.........*/
#if			( FDM_WITH_FREE_ACCESS == YES )
		pRef->FDM_Internal_Use.Vcom = 8;

		sema_init(&sp8,1,"FDM_Vcom",SEMA_DRIVER,NULL);
#endif
		break;

		case 2 :
		pRef->FDM_Internal_Use.Semaphore = 9;

		sema_init(&sp9,1,"FDM_S",SEMA_DRIVER,NULL);

		pRef->FDM_Internal_Use.Task_Semaphore = 10;

		sema_init(&sp10,1,"FDM_Task_S",SEMA_DRIVER,NULL);

		pRef->FDM_Internal_Use.Modif_BD_Semaphore = 11;

		sema_init(&sp11,1,"FDM_Modif_BD_S",SEMA_DRIVER,NULL);
		/*.........*/
#if			( FDM_WITH_FREE_ACCESS == YES )
		pRef->FDM_Internal_Use.Vcom = 12;

		sema_init(&sp12,1,"FDM_Vcom",SEMA_DRIVER,NULL);
#endif
		break;

		case 3 :
		pRef->FDM_Internal_Use.Semaphore = 13;

		sema_init(&sp13,1,"FDM_S",SEMA_DRIVER,NULL);

		pRef->FDM_Internal_Use.Task_Semaphore = 14;

		sema_init(&sp14,1,"FDM_Task_S",SEMA_DRIVER,NULL);

		pRef->FDM_Internal_Use.Modif_BD_Semaphore = 15;

		sema_init(&sp15,1,"FDM_Modif_BD_S",SEMA_DRIVER,NULL);
		/*.........*/
#if			( FDM_WITH_FREE_ACCESS == YES )
		pRef->FDM_Internal_Use.Vcom = 16;

		sema_init(&sp16,1,"FDM_Vcom",SEMA_DRIVER,NULL);
#endif
		break;

		default : return FDM_KO;
	}

	return FDM_OK;
}
void
FDM_ressources_delete ( FDM_REF *pRef)
{
	struct fdm_state *fdmp;

	fdmp =(struct fdm_state *) (pRef->S).User_Ctxt;

	switch(fdmp->num_card)
	{
		case 0 :
		sema_destroy(&sp1);
		sema_destroy(&sp2);
		sema_destroy(&sp3);
		/*.........*/
#if			( FDM_WITH_FREE_ACCESS == YES )
		/*Va*/
		sema_destroy(&sp4);
#endif
		break;

		case 1 :
		sema_destroy(&sp5);
		sema_destroy(&sp6);
		sema_destroy(&sp7);
		/*.........*/
#if			( FDM_WITH_FREE_ACCESS == YES )
		/*Va*/
		sema_destroy(&sp8);
#endif
		break;

		case 2 :
		sema_destroy(&sp9);
		sema_destroy(&sp10);
		sema_destroy(&sp11);
		/*.........*/
#if			( FDM_WITH_FREE_ACCESS == YES )
		/*Va*/
		sema_destroy(&sp12);
#endif
		break;

		case 3 :
		sema_destroy(&sp13);
		sema_destroy(&sp14);
		sema_destroy(&sp15);
		/*.........*/
#if			( FDM_WITH_FREE_ACCESS == YES )
		/*Va*/
		sema_destroy(&sp16);
#endif
		break;
	}

}
/* write here your macros for take and release semaphore */

void
OS_semaphore_take( Ulong SM_ID )
{
	switch ( SM_ID )
	{
		case 1:
		sema_p(&sp1);
		break;
		case 2:
		sema_p(&sp2);
		break;
		case 3:
		sema_p(&sp3);
		break;
#if			( FDM_WITH_FREE_ACCESS == YES )
		case 4:
		sema_p(&sp4);
		break;
#endif
		case 5:
		sema_p(&sp5);
		break;
		case 6:
		sema_p(&sp6);
		break;
		case 7:
		sema_p(&sp7);
		break;
#if			( FDM_WITH_FREE_ACCESS == YES )
		case 8:
		sema_p(&sp8);
		break;
#endif
		case 9:
		sema_p(&sp9);
		break;
		case 10:
		sema_p(&sp10);
		break;
		case 11:
		sema_p(&sp11);
		break;
#if			( FDM_WITH_FREE_ACCESS == YES )
		case 12:
		sema_p(&sp12);
		break;
#endif
		case 13:
		sema_p(&sp13);
		break;
		case 14:
		sema_p(&sp14);
		break;
		case 15:
		sema_p(&sp15);
		break;
#if			( FDM_WITH_FREE_ACCESS == YES )
		case 16:
		sema_p(&sp16);
		break;
#endif
	}

}

void
OS_semaphore_release( Ulong SM_ID )
{
	switch ( SM_ID )
	{
		case 1:
		sema_v(&sp1);
		break;
		case 2:
		sema_v(&sp2);
		break;
		case 3:
		sema_v(&sp3);
		break;

#if			( FDM_WITH_FREE_ACCESS == YES )
		case 4:
		sema_v(&sp4);
		break;
#endif
		case 5:
		sema_v(&sp5);
		break;
		case 6:
		sema_v(&sp6);
		break;
		case 7:
		sema_v(&sp7);
		break;
#if			( FDM_WITH_FREE_ACCESS == YES )
		case 8:
		sema_v(&sp8);
		break;
#endif
		case 9:
		sema_v(&sp9);
		break;
		case 10:
		sema_v(&sp10);
		break;
		case 11:
		sema_v(&sp11);
		break;
#if			( FDM_WITH_FREE_ACCESS == YES )
		case 12:
		sema_v(&sp12);
		break;
#endif
		case 13:
		sema_v(&sp13);
		break;
		case 14:
		sema_v(&sp14);
		break;
		case 15:
		sema_v(&sp15);
		break;
#if			( FDM_WITH_FREE_ACCESS == YES )
		case 16:
		sema_v(&sp16);
		break;
#endif
	}
}

void *memset(void *s, int c, size_t n)
{
	char *p;
	int i;

	p=(char*)s;
	for (i=0;i<n;i++) *p++=c;
	return s;
}

void _FIP_PAUSE ( Ulong Milliseconde)
{
	delay(drv_usectohz(Milliseconde*1000));
}

#endif /* SOLARIS */

/*******************************************************************************************/
/*******************************************************************************************/
/*************************   R T X    ******************************************************/
/*******************************************************************************************/
/*******************************************************************************************/

#if											( FDM_WITH_RTX == YES )
#include <windows.h>
#include <wchar.h>
#include <rtapi.h>

void *memset( void *dest, int c, size_t count )
{
	char* D;
	unsigned i;

	D = (char*)dest;
	for(i = 0; i < count; i++)
	{
		*D++ = c;
	}
	return ((void*)D);
}

void *memcpy( void *dest, const void *src, size_t count )
{
	char* D;
	char* S;
	unsigned i;
	D = (char*)dest;
	S = (char*)src;

	for(i = 0; i < count; i++)
	{
		*D++ = *S++;
	}

	return ((void*)D);

}

int memcmp( const void *buf1, const void *buf2, size_t count )
{
	char* B1;
	char* B2;
	unsigned i;
	B1 = (char*)buf1;
	B2 = (char*)buf2;
	for(i = 0; i < count; i++)
	{
		if (*B1 != *B2)
		{
			return (B1> B2 ) ? 1 : -1;
		}
		B1++;
		B2++;
	}
	return 0;

}

size_t strlen( const char *string )
{
	size_t l = 0;
	while (*string++)
	{
		l++;
	}
	return l;
}

int
FDM_ressources_create ( FDM_REF *Ref )
{

	char Name[5];
	char T[] =
	{
		'0','1','2','3','4','5','6','7',
		'8','9','A','B','C','D','E','F'};

	/*Co*/
	Name[0] = 'C';
	Name[1] = 'o';
	Name[2] = T[(Ref->H.K_PHYADR>>4) & 0xF];
	Name[3] = T[ Ref->H.K_PHYADR & 0xF];
	Name[4] = 0;

	Ref->FDM_Internal_Use.Semaphore = RtCreateMutex( NULL , FALSE , Name );

	Name[0] = 'T';
	Name[1] = 'a';
	Name[2] = T[(Ref->H.K_PHYADR>>4) & 0xF];
	Name[3] = T[ Ref->H.K_PHYADR & 0xF];
	Name[4] = 0;

	Ref->FDM_Internal_Use.Task_Semaphore = RtCreateMutex( NULL , FALSE , Name );

	Name[0] = 'B';
	Name[1] = 'd';
	Name[2] = T[(Ref->H.K_PHYADR>>4) & 0xF];
	Name[3] = T[ Ref->H.K_PHYADR & 0xF];
	Name[4] = 0;

	Ref->FDM_Internal_Use.Modif_BD_Semaphore = RtCreateMutex( NULL , FALSE , Name );

	Name[0] = 'A';
	Name[1] = 'l';
	Name[2] = T[(Ref->H.K_PHYADR>>4) & 0xF];
	Name[3] = T[ Ref->H.K_PHYADR & 0xF];
	Name[4] = 0;

	Ref->FDM_Internal_Use.Vcom = RtCreateMutex( NULL , FALSE , Name );

	/*.........*/

	return FDM_OK;
}
void
FDM_ressources_delete ( FDM_REF *Ref)
{

	RtCloseHandle( Ref->FDM_Internal_Use.Semaphore );
	RtCloseHandle( Ref->FDM_Internal_Use.Task_Semaphore );
	RtCloseHandle( Ref->FDM_Internal_Use.Modif_BD_Semaphore );
#if			( FDM_WITH_FREE_ACCESS == YES )
	RtCloseHandle( Ref->FDM_Internal_Use.Vcom );
#endif

	/*.........*/
}

/* write here your macros for take and release semaphore */

void
OS_semaphore_take( __Object_Semaphore_Type__ SM_ID )
{
	if ( RtWaitForSingleObject(SM_ID , INFINITE ) == WAIT_FAILED )
	{
#ifdef _DEBUG
		RtPrintf("\nsemaphore_take failed ");
#endif
	}
}

void
OS_semaphore_release( __Object_Semaphore_Type__ SM_ID )
{
	RtReleaseMutex ( SM_ID );
}

void
OS_Leave_Region()
{
	RtEnableInterrupts();
}

void
OS_Enter_Region()
{
	RtDisableInterrupts();
}

void _FIP_PAUSE ( Ulong Milliseconde)
{
	Sleep((DWORD)Milliseconde);
}

#endif

#if (  ( FDM_WITH_VXWORKS  == NO  ) \
	&& ( FDM_WITH_PSOS     == NO  ) \
	&& ( FDM_WITH_NDIS     == NO  ) \
	&& ( FDM_WITH_NT       == NO  ) \
    && ( FDM_WITH_SCOUNIX  == NO  ) \
    && ( FDM_WITH_SOLARIS  == NO  ) \
    && ( FDM_WITH_RTX	   == NO ))
/********************************************************************************************
 *********************************************************************************************
 Non Standard Monitor Used , write here your code.....
 *********************************************************************************************
 ********************************************************************************************/
#if					( FDM_WITH_SEMAPHORE == YES )

int
FDM_ressources_create ( FDM_REF *pRef )
{
	/*.........*/
	return FDM_OK;
}
void
FDM_ressources_delete ( FDM_REF *Ref)
{
	/*.........*/
}

/* write here your macros for take and release semaphore */

void
OS_semaphore_take( Ulong SM_ID )
{
}

void
OS_semaphore_release( Ulong SM_ID )
{
}

#endif

void
OS_Leave_Region(void)
{
	/* write here your code for enable all interrupt */
}

void
OS_Enter_Region(void)
{
	/* write here your code for diable all interrupt */
}

void
_FIP_PAUSE ( Ulong Milliseconde)
{
	/*write here your code for wait FIP Interrupt or Timeout*/
}

#endif
