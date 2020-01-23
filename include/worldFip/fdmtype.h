/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmtype.h
  Description     : Fichier d'entete - Types prives



  Date de creation: Fri Mar 15 15:11:48 MET 1996

Version:4.3.1
	a/ FDM_WITH_WIN32

***********************************************************/

#ifndef __fdmtype_h
#define __fdmtype_h



/********???????????????????????????*****************/
#define free_Acc_fifo_full	1
#define free_Acc_time_out	2
#define free_Acc_DV_full	3

#ifndef  TEST_P_DURATION
#define TEST_P_DURATION 10000
#endif


#ifndef  FDM_WITH_FIPCODE_31_25
#error   FDM_WITH_FIPCODE_31_25  not define
#endif

#ifndef  FDM_WITH_FIPCODE_5000
#error   FDM_WITH_FIPCODE_5000  not define
#endif

#ifndef  FDM_WITH_FIPCODE_1000
#error   FDM_WITH_FIPCODE_1000  not define
#endif

#ifndef  FDM_WITH_CAD_UL
#error   FDM_WITH_CAD_UL  not define
#endif
#if 	( FDM_WITH_CAD_UL	 == 	YES )
#error  PS : verifier la syntaxe de unaligne
#pragma unaligne
#endif

#if (( FDM_WITH_FIPCODE_31_25 == NO )&&( FDM_WITH_FIPCODE_1000 == NO )&&( FDM_WITH_FIPCODE_5000 == NO ))
#error  No Fipcode define
#endif


#if 	( FDM_WITH_CHAMP_IO	 == 	YES )

#ifndef  FDM_WITH_WIN32
#error   FDM_WITH_WIN32  not define
#endif

#ifndef  FDM_WITH_MSDOS
#error   FDM_WITH_MSDOS  not define
#endif

#ifndef  FDM_WITH_MSDOS
#error   FDM_WITH_MSDOS  not define
#endif

#ifndef  FDM_WITH_VXWORKS
#error   FDM_WITH_VXWORKS  not define
#endif

#ifndef  FDM_WITH_RTX
#error   FDM_WITH_RTX  not define
#endif

#ifndef  FDM_WITH_OPTIMIZED_BA
#error   FDM_WITH_OPTIMIZED_BA  not define
#endif


#if 	( FDM_WITH_WIN32	 ==	YES )
#define __Port_Type__	unsigned short
#define __Reg_cx__	ecx
#define __Reg_dx__	edx
#endif

#if 	( FDM_WITH_MSDOS	 ==	YES )
#define __Port_Type__	unsigned
#define __Reg_cx__	cx
#define __Reg_dx__	dx
#endif


#if 	( FDM_WITH_RTX == YES )
#define __Port_Type__	unsigned
#endif

#if 	( FDM_WITH_METAWARE == YES )
#define __Port_Type__	unsigned
#endif

#if 	 (FDM_WITH_X86 == YES)
#define __Port_Type__	unsigned
#endif

#if 	( FDM_WITH_SOLARIS	 ==	YES )
#define __Port_Type__	caddr_t
#define __Reg_cx__	cx
#define __Reg_dx__	dx
#endif

#endif  /* FDM_WITH_CHAMP_IO	 == 	YES */



/********???????????????????????????*****************/
#ifndef FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT
#error  FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT not define
#endif

#ifndef FDM_WITH_CONTROLS
#error  FDM_WITH_CONTROLS not define
#endif

#ifndef FDM_WITH_APER
#error  FDM_WITH_APER  not define
#endif

#if 		( FDM_WITH_APER == YES )
#ifndef FDM_WITH_SM_MPS
#error  FDM_WITH_SM_MPS  not define
#endif
#else
#define  FDM_WITH_SM_MPS  NO
#endif

#ifndef FDM_WITH_BA
#error  FDM_WITH_BA not defined
#endif

#ifndef FDM_WITH_DIAG
#error  FDM_WITH_DIAG not defined
#endif

#ifndef FDM_WITH_BI_MEDIUM
#error  FDM_WITH_BI_MEDIUM not defined
#endif

#ifndef FDM_WITH_REDONDANCY_MGNT
#error  FDM_WITH_REDONDANCY_MGNT not defined
#endif

#ifndef FDM_WITH_FREE_ACCESS
#error  FDM_WITH_FREE_ACCESS not defined
#endif

#ifndef FDM_WITH_CHAMP_IO
#error  FDM_WITH_CHAMP_IO not defined
#endif


#ifndef FDM_WITH_FIPCODE_LINKED
#error  FDM_WITH_FIPCODE_LINKED not defined
#endif

#ifndef FDM_WITH_LITTLE_ENDIAN
#error  FDM_WITH_LITTLE_ENDIAN   not defined
#endif


#if 	(  FDM_WITH_MICROSOFT == YES )
#pragma warning ( disable : 4505 4100)
#endif

#if  	( FDM_WITH_NDIS	== YES )
#define 	FDM_WITH_SEMAPHORE	YES
#endif

#if  	( FDM_WITH_PSOS	== YES )
#undef		FDM_WITH_SEMAPHORE
#define 	FDM_WITH_SEMAPHORE	YES
#endif

#if  	( FDM_WITH_VXWORKS	== YES )
#undef		FDM_WITH_SEMAPHORE
#define 	FDM_WITH_SEMAPHORE	YES
#endif

#if  	( FDM_WITH_SCOUNIX == YES )
#undef		FDM_WITH_SEMAPHORE
#define 	FDM_WITH_SEMAPHORE	YES
#endif	/* SCO-UNIX */


#include "fdm_gdm.h"
#include "fdm_fifo.h"
#include "fdmdicho.h"
#include "fdm_lai.h"
#include "fdm_gdt.h"

#define _FIP_TICKS_COUNTER	_TIMER_UP_TO_DATE
#define _FIP_fifo_empty(fifo)    TASK_LOOP(fifo)
#define FDM_INITIALISATIONS	_TIMER_INITIALIZE

typedef struct _FDM_MPS_VAR_REF  {
   	FDM_XAE 		definition;
	Ushort			cle_Pro, cle_Con ;
	int 			Valid ;
	TEMPO_DEF		Timer;
	int 			Compte_rendu;
	struct _FDM_MPS_VAR_REF * Ref_XAE;/* pointe sur la variable */
   	struct _FDM_AE_LE_REF 	* Ref;    /* pointe sur AE_LE       */
	void			* Irp ;   /* User usage		*/
	void			* pDATAS;
} FDM_MPS_VAR_REF ;


typedef struct {
	TEMPO_DEF		T;
	int 			Knal;
	struct _FDM_REF		*Fip;
 } TEMPO_DEF_C ;

typedef struct  {
	Ushort Event_Definition ;
	Ushort Additional_Information ;
	} FIP_EVT_Type ;


typedef struct _FDM_PRIVATE_DLI_T {
		struct 	_FDM_PRIVATE_DLI_T 	*Prev;
		struct 	_FDM_PRIVATE_DLI_T 	*Next;
		Ushort 				evt_addit_info;
		int 				Service_Report  ;
		Ushort				Channel_Nr;
		struct _FDM_MESSAGING_REF	*Ref_mdl;
		struct _FDM_MSG_TO_SEND		*User_Msg;
	} FDM_PRIVATE_DLI_T;




typedef struct {
	T_File			*GM1;	/* Pool memoire Descripteur*/
	T_File			*GM2;	/* Pool memoire data	   */
} PRIVATE_FDM_DLI_R ;


typedef struct {
	Ushort Nb_Of_Transaction_Ok_1;
	Ushort Nb_Of_Transaction_Ok_2;
	Ushort Nb_Of_Frames_Nok_1;
	Ushort Nb_Of_Frames_Nok_2;
	Ushort Activity_Status;
	Ushort Nb_No_Echo_Transmit_Error;
	} FDM_STATISTICS;


#endif

