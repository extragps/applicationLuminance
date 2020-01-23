/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
 

      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmtime.cpp
  Description     : multi-production de l'heure


Version:4.3.1
	a/- lecture impossble si READ_ONLY

Version:4.3.6
	a/- Controle lors  de la creation qd bureau vote only
	b/- allocation lors  de la creation qd bureau vote only
	c/-trame etat suite a reception trame 0 elu

Version:4.6.1
	a/-recodage suite a DE read/write VCOM d'heure


***********************************************************/
#include "user_opt.h"

#include "fdmtime.h"

#if			( FDM_WITH_GT == YES )
static const Uchar Information_Report_Frame_Init[] = {
0x03,0x10,0x00,0x00,
0x83,
0x00,0x00,0x00,0x00,
0x85,
0x00,0x00,
0x81,
0x00,0x02,
0x00,0x01,
0x00,0x00,
0x00,0x02,

};

#define Frame_size  sizeof(Information_Report_Frame_Init )

Ushort
fdm_generic_time_set_candidate_for_election
		( FDM_GENERIC_TIME_REFERENCE *Ref , enum FDM_BOOLEAN S )
{
#if	( FDM_WITH_CONTROLS == YES )
	if ( Ref == NULL ) return 0xffff;
#endif
	Ref->Eligible = S ;
	return 0;

}

Ushort
fdm_generic_time_set_priority ( FDM_GENERIC_TIME_REFERENCE *Ref , GT_PRIORITY prio )
{
#if	( FDM_WITH_CONTROLS == YES )
	if ( Ref == NULL ) return 0xffff;
#endif
	Ref->Priorite = prio.Val ;
	return 0;
}


typedef struct  {
	FDM_MSG_TO_SEND Desc;
	FDM_MSG_T_DESC  Block ;
	Uchar Information_Report_Frame[Frame_size+2];
} TIME_MSG_TO_SEND;

TIME_MSG_TO_SEND*
_Information_Report__Create ( FDM_GENERIC_TIME_REFERENCE *Ref )
{
	return 	  ( TIME_MSG_TO_SEND*) _ALLOCATE_MEMORY(  Ref->MemPool ) ;
}

void
_Information_Report__Write (
	FDM_MESSAGING_REF *DLL ,
	TIME_MSG_TO_SEND * Message ,
	Ushort Val )
{

	Message->Information_Report_Frame[Frame_size+0] = ( Uchar ) ( Val >> 8 );
	Message->Information_Report_Frame[Frame_size+1] = ( Uchar )   Val  ;

    	fdm_send_message( DLL ,&Message->Desc );

}



typedef struct Tempo_def {
	  TEMPO_DEF			Ticks_Election;
	  FDM_GENERIC_TIME_REFERENCE    *P;
	}  Tempo_def;


/*************************************************************************/

static void
Ind_Ordre (
	FDM_MESSAGING_REF * DL ,
	FDM_MSG_RECEIVED  *Msg_recu )

{
	FDM_GENERIC_TIME_REFERENCE *Ref;
	Uchar Hi, Lo;
	Ushort Status,S;
	int i;
	enum _FDM_CHANGE_TYPE Var_Type;

	Ref = ( FDM_GENERIC_TIME_REFERENCE *) Msg_recu->User_Ctxt;

	Hi = Msg_recu->Ptr_Block->Data_Buffer[Frame_size+0] ;
	Lo = Msg_recu->Ptr_Block->Data_Buffer[Frame_size+1] ;
	if (Hi == 0 ){
		Ref->StationActive = Lo;

		Status = Ref->Fip->H.K_PHYADR;
		Status <<= 8;
		Status |= (( Ref->Fip->H.K_PHYADR == Lo ) ? active :  standby ) << 4;
		Status |= Ref->Priorite ;
	} else {
		Status = 0xffff;
		Ref->StationActive = -1;
	}

	S =  Hi;S <<= 8;S |= Lo;

	if ( Ref->Status != Status  ) {
		Ref->Status = Status   ;
		i = ( Ref->Fip->H.K_PHYADR == S ) ? -1 : 0 ;

		Ref->user_def.User_Signal_Mode( i );

		Var_Type = ( i!=0 ) ? _PRODUCED : _CONSUMED ;
		fdm_mps_var_change_prod_cons ( Ref->Generic_var  ,  Var_Type , Var_Type );
	}

	fdm_msg_data_buffer_free  (  Msg_recu->Ptr_Block );
	fdm_msg_ref_buffer_free   (  Msg_recu );
}

static void
Ind_Etat (
	FDM_MESSAGING_REF * DL ,
	FDM_MSG_RECEIVED  *Msg_recu )
{
/*	  |
	  + Ack_Etat
	  |
	-----	-------------------------
	| 5 |---| MAJ staton		|
	-----	-------------------------
*/	FDM_GENERIC_TIME_REFERENCE *Ref;
	EtatAbonnes	Tmp 	 ;

	Ref = ( FDM_GENERIC_TIME_REFERENCE *) Msg_recu->User_Ctxt;

	Tmp.abonne =  Msg_recu->Ptr_Block->Data_Buffer[Frame_size+0];
	Tmp.Prio   = Msg_recu->Ptr_Block->Data_Buffer[Frame_size+1] ;
	Tmp.State  =Msg_recu->Ptr_Block->Data_Buffer[Frame_size+1]>>4 ;

	Ref->Actual[Tmp.abonne] =Tmp;

	fdm_msg_data_buffer_free  (  Msg_recu->Ptr_Block );
	fdm_msg_ref_buffer_free   (  Msg_recu );
}

static void
Ack_Etat (
		FDM_MESSAGING_REF   * DL ,
		FDM_MSG_TO_SEND     * Msg_cnf )
{
/*	  |
	  + Ack_Etat
	  |
	-----	-------------------------
	| 4 |---| _TIMER_START		|
	-----	-------------------------
*/
	FDM_GENERIC_TIME_REFERENCE *Ref;

	_FREE_MEMORY ((User_GDM *) Msg_cnf );
	Ref = (FDM_GENERIC_TIME_REFERENCE *) DL->Send_infos.User_Ctxt;
	Ref->Election_State = ( Msg_cnf->Service_Report.Valid == _FDM_MSG_SEND_OK )? fdm_true : fdm_false ;
	Ref->etat_emission_etat = 0;

}

static void
Ack_Ordre (
		FDM_MESSAGING_REF   * DL ,
		FDM_MSG_TO_SEND     * Msg_cnf )
{
	_FREE_MEMORY ((User_GDM *) Msg_cnf );
}





static void
Tempo_Etat ( TEMPO_DEF * T)
{
/*	  |
	  + Tempo_Etat
	  |
	-----   -------------------------
	| 3 |---| SendMsg(Etat)		|
	-----   -------------------------
*/
	Tempo_def *P_Ref;
	FDM_GENERIC_TIME_REFERENCE *Ref;
	FDM_MESSAGING_REF  *DLL;
	TIME_MSG_TO_SEND* Ir;
	Ushort	Status;

	P_Ref = ( Tempo_def*) T;
	Ref = P_Ref->P;

	if (Ref->Stopped) return;

	DLL = Ref->EmettreEtat ;

	Status = ((int) Ref->Fip->H.K_PHYADR) << 8;

	if ( Ref->Fip->H.K_PHYADR == Ref->StationActive ) {
		if ( Ref->Eligible == fdm_true ) Status |= active << 4 ;
		else Status |= not_eligible ;
	} else {
		if ( Ref->Eligible == fdm_true ) Status |= standby << 4 ;
		else Status |= not_eligible << 4 ;
	}
	Status |=  Ref->Priorite ;

	if ( 	Ref->etat_emission_etat == 0 ) {
		Ir = _Information_Report__Create( Ref );
		if ( Ir != (TIME_MSG_TO_SEND*) 0 ){
			Ref->etat_emission_etat = 1;
      			 _Information_Report__Write (DLL, Ir , Status);
      		}
	}
}
/***************************************************************/
/*	  |
	  + Tempo_Election
	  |
	-----   -------------------------
	| 1 |---| Choix producteur	|
	-----   -------------------------
*/
/***************************************************************/
static void
Tempo_Election ( TEMPO_DEF * T)
{
	Tempo_def *P_Ref;
	FDM_GENERIC_TIME_REFERENCE *Ref;
	int K_PHYADR ;
	EtatAbonnes	Tmp 	=  { 0, unavailable, 0} ;
	EtatAbonnes	StActif =  { 0, unavailable, 0} ;
	int  S ;

	P_Ref = ( Tempo_def*) T;
	Ref = P_Ref->P;

	if (Ref->Stopped) return;

	K_PHYADR = Ref->Fip->H.K_PHYADR ;
	if ( Ref->user_def.With_MPS_Var_Produced ) {
		if ( Ref->Election_State == fdm_false)  Tmp.State = not_eligible ;
		else {
			Tmp.State  =  ( Ref->Eligible == fdm_true ) ? standby : not_eligible ;
		}

		Tmp.abonne = K_PHYADR;
		Tmp.Prio   = Ref->Priorite ;
	}
	memcpy ((void*) &Ref->Old ,(void*) &Ref->Actual , sizeof( Ref->Actual ));
	memset ((void*) &Ref->Actual , 0 , sizeof( Ref->Actual ));
	/* Controle */
	{
		int i  ;
		/*
		on cherche la station la + prio en standby => Tmp
		La station active => S
		*/
		if (Tmp.State == not_eligible ) {
			for (i = 0 ; i < 256 ; i++)
			{  /*recherche une station en standby*/
				if ( Ref->Old[i].State == standby )
				{
					Tmp = Ref->Old[i] ;
					break;
				}
			}
		}
		for (i = 0 ; i < 256 ; i++)
		{
			if (
				( Ref->Old[i].State == standby ) &&
				( Ref->Old[i].Prio > Tmp.Prio ) )
			{
				Tmp = Ref->Old[i] ;
			}
		}
		for (i = 0 ; i < 256 ; i++)
		{
			if ( Ref->Old[i].State == active ){
				StActif = Ref->Old[i]  ;
				break;
			}
		}
		S = -1;/* recherche d'un elu */
		if ((StActif.State >= 2) && (Tmp.State >= 2) ){
			S = ( StActif.Prio < Tmp.Prio ) ?  Tmp.abonne : StActif.abonne ;
			goto finrecherche;
		}
		if (StActif.State >= 2) {
			S =  StActif.abonne ;
			goto finrecherche;
		}
		if (Tmp.State >= 2) {
			S =  Tmp.abonne ;
			goto finrecherche;
		}

	}
/*
	  |
	  + BA actif
	  |
	-----   -------------------------
	| 2 |---| SendMsg(Ordre) 	|
	-----   -------------------------
*/

finrecherche:
	 { /*0*/
		BA_INF_STATUS Status;
		FDM_REF *Fip;
		FDM_MESSAGING_REF  *DLL;
		TIME_MSG_TO_SEND* Ir;

		Fip =  Ref->Fip;
		fdm_ba_status(Fip , &Status);

		if ( Status.Actif_Running != RUNNING ) /*      >>>>>> */ return;

		DLL = Ref->EmettreOrdre ;
		Ir = _Information_Report__Create( Ref );
		if ( Ir != (TIME_MSG_TO_SEND*) 0 )
      			_Information_Report__Write ( DLL,Ir ,(Ushort) S );
	{/*1*/
	int i;
	enum _FDM_CHANGE_TYPE Var_Type;

	Ushort Status;

	if (S != -1 ){
		Status = K_PHYADR<<8;
		Status |= (( K_PHYADR == S ) ? active :  standby ) << 4 ;
		Status |= Ref->Priorite;
	} else {
		Status = 0xffff;
	}
	Ref->Status = Status ;

	/* Changement de Station Active ?*/
	if (  Ref->StationActive   != S ) {
		Ref->StationActive = S;
		i = ( K_PHYADR == S ) ? -1 : 0 ;
		Ref->user_def.User_Signal_Mode( i );

		Var_Type = ( K_PHYADR == S ) ? _PRODUCED : _CONSUMED ;
		fdm_mps_var_change_prod_cons ( Ref->Generic_var  ,  Var_Type , Var_Type );
	}

	}/*1*/

	}/*0*/
}

#endif

/*****************************************************************************/
/***************     M e s s a g e r i e    d e f i n t i o n ****************/
/*****************************************************************************/
#if			( FDM_WITH_GT == YES )

#define NULL_PTR (void*)0

static const   FDM_MESSAGING_TO_SEND    DLL_Emetteur_Etat = {
	_FDM_MSG_IMAGE_1_AND_2 ,
	{  Ack_Etat , NULL_PTR , NULL_PTR , 0 } ,
	0x000f0000L , 0x00A00000L
	} ;

static const  FDM_MESSAGING_TO_REC   DLL_Recepteur_Etat = {
	_FDM_MSG_IMAGE_1_AND_2 ,
	{  Ind_Etat , NULL_PTR , NULL_PTR ,10 , 10 },
	0x00A00000L , 0x80000000L
	} ;


static const FDM_MESSAGING_TO_SEND   DLL_Emetteur_Ordre = {
	_FDM_MSG_IMAGE_1_AND_2 ,
	{  Ack_Ordre , NULL_PTR , NULL_PTR , 0 } ,
	0x000f0000L , 0x00A00100L
	} ;

static const FDM_MESSAGING_TO_REC   DLL_Recepteur_Ordre = {
	_FDM_MSG_IMAGE_1_AND_2 ,
	{  Ind_Ordre , NULL_PTR , NULL_PTR ,10 , 10 },
	0x00A00100L ,  0x80000000L
	} ;


#endif

/*****************************************************************************/
/**********  V a r i a b l e   M  P  S   t r a i t e m e n t  ****************/
/*****************************************************************************/
#if	( FDM_WITH_GT_ONLY_CONSUMED == YES ) || ( FDM_WITH_GT == YES )


static const FDM_MPS_READ_STATUS Cr_Neg  = { _FDM_MPS_USER_ERROR  , 1,1,1,1 };

static void
DONNE_L_HEURE_CEGELEC(FDM_MPS_VAR_TIME_DATA* H)
{
	FDM_GENERIC_TIME_VALUE	P_Val;
	Ulong T;
	P_Val.Signifiance = H->FBuffer[3] ;

#if ( FDM_WITH_LITTLE_ENDIAN == YES )
	P_Val.Number_of_second   	= SWAB_ONE_LONG ( *(Ulong*)(&H->FBuffer[4]) );
	P_Val.Number_of_nanoseconde 	= SWAB_ONE_LONG ( *(Ulong*)(&H->FBuffer[8]) );
#else
	P_Val.Number_of_second   	=   *(Ulong*) (&H->FBuffer[4]) ;
	P_Val.Number_of_nanoseconde 	=   *(Ulong*) (&H->FBuffer[8]) ;
#endif
	T = H->Network_Delay * 1000;
	P_Val.Number_of_nanoseconde 	+= T;
	if ( P_Val.Number_of_nanoseconde >= 1000L*1000L*1000L )  {
		P_Val.Number_of_second        += (P_Val.Number_of_nanoseconde / (1000L*1000L*1000L)) ;
		P_Val.Number_of_nanoseconde   %= (1000L*1000L*1000L) ;
	}

	fdm_generic_time_give_value(&P_Val);
}

FDM_MPS_READ_STATUS
fdm_generic_time_read_loc ( const FDM_GENERIC_TIME_REFERENCE *GT  )
{
	FDM_MPS_VAR_TIME_DATA H;
	FDM_MPS_READ_STATUS Cr;

	if ( GT->Generic_var == NULL ) 	return Cr_Neg;

#if	( FDM_WITH_GT_ONLY_CONSUMED == YES ) || ( FDM_WITH_GT == YES )

	if (( GT->Status & 0xff ) ==  active ) 	return Cr_Neg;

	Cr =   fdm_mps_var_time_read_loc ( GT->Generic_var  , DONNE_L_HEURE_CEGELEC , &H);

	return Cr;
#else
	return Cr_Neg;
#endif

}

#endif

#if (  FDM_WITH_GT_ONLY_PRODUCED == YES ) || ( FDM_WITH_GT == YES )
/*
ceci est une variable globale , on peut car la validite des
donnees uniquement lors de l'ecriture , et on est masque
durant l'ecriture.
*/
#if (  FDM_WITH_SOLARIS		== YES ) || ( FDM_WITH_IA64 == YES )
static void 
CP_ONE_LONG ( unsigned long SRC , unsigned char * DST )
{
	int i;
	for (i=0;i<4;i++){
		*DST++ =  (SRC >> (24 - (i*8))) & 0xFF ;
	}
}
#endif

static FDM_GENERIC_TIME_VALUE GT_Val;

#if (  FDM_WITH_SOLARIS		== YES )
static unsigned char  GT_Buffer[12];
#endif

static Uchar *
Get_GT_Value ( void )
{

	GT_Val = fdm_generic_time_get_value ();
	GT_Val.Signifiance &= 0xff;
	GT_Val.Signifiance |= 0x0B800900L;

#if ( FDM_WITH_LITTLE_ENDIAN == YES )
	GT_Val.Signifiance 		= SWAB_ONE_LONG ( GT_Val.Signifiance );
	GT_Val.Number_of_second 	= SWAB_ONE_LONG ( GT_Val.Number_of_second );
	GT_Val.Number_of_nanoseconde 	= SWAB_ONE_LONG ( GT_Val.Number_of_nanoseconde  );
#endif

#if (  FDM_WITH_SOLARIS		== YES ) || ( FDM_WITH_IA64 == YES )
	CP_ONE_LONG ( GT_Val.Signifiance , 		&GT_Buffer[0] );
	CP_ONE_LONG ( GT_Val.Number_of_second , 	&GT_Buffer[4] );
	CP_ONE_LONG ( GT_Val.Number_of_nanoseconde , 	&GT_Buffer[8] );
	return GT_Buffer;	
#else
	return ( Uchar *)&GT_Val;
#endif
}


Ushort
fdm_generic_time_write_loc
	( const FDM_GENERIC_TIME_REFERENCE *GT )
{
	if ( GT->Generic_var == NULL ) return 0xffff;
	return fdm_mps_var_time_write_loc ( GT->Generic_var, Get_GT_Value ,GT->user_def.Delta_Time_Location );
}

#endif


#define _Sync	0
#define _Prod	1
#define _Cons	2
#define _PrCo	3

#define _Local	1
#define _remot	0
#define _TimeV	1
#define _Refre	1
#define _Indic	1
#define _RQa	1
#define _MSGa	1
#define _vide	0

#if !defined ( FDM_WITH_GT ) && !defined (FDM_WITH_GT_ONLY_CONSUMED ) && !defined (  FDM_WITH_GT_ONLY_PRODUCED )
#error  FDM_WITH_GT_xxx not define
#endif

#if (  FDM_WITH_GT_ONLY_PRODUCED == YES ) || ( FDM_WITH_GT == YES )
static const FDM_XAE DEF_Generic_Time = {
{ _vide,3,_PrCo,_Local,_TimeV,_vide,_vide,_vide,_vide,_vide },
	0x9802, 12 ,  0 , 0 , 0 ,
{
  (void (*) ( struct _FDM_REF * ))        0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *, FDM_MPS_READ_STATUS , FDM_MPS_VAR_DATA *)) 0
},
};
#else
static const FDM_XAE DEF_Generic_Time = {
{ _vide,3,_PrCo,_Local,_TimeV,_vide,_vide,_vide,_vide,_vide },
	0x9802, 12 ,  0 , 0 , 0 ,
{
  (void (*) ( struct _FDM_REF * ) )       0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
};
#endif

/*****************************************************************************/
/***************   Creation Contexte gestion de  l'heure    ******************/
/*****************************************************************************/
/*
	-----   -------------------------
	| 0 |---| initialisation	|
	-----   -------------------------
*/

#define _MISSING_PERIOD 1
#define _CONFLICT_TYPE  2

FDM_GENERIC_TIME_REFERENCE  *
fdm_generic_time_initialize  (
		FDM_REF *Fip ,
		const FDM_GENERIC_TIME_DEFINITION  *User_param
		)
{
FDM_GENERIC_TIME_REFERENCE   *Ref;

Ref = (  FDM_GENERIC_TIME_REFERENCE* ) FDM_Malloc ( Fip , sizeof( FDM_GENERIC_TIME_REFERENCE ));
if ( Ref == ( FDM_GENERIC_TIME_REFERENCE* ) 0 ) return Ref ;

memset ( Ref , 0 , sizeof( FDM_GENERIC_TIME_REFERENCE ));
Ref->user_def = *User_param ;
Ref->Fip = Fip;

#if ( FDM_WITH_GT == YES )
{
	/* initialisation pool memoire */
	TIME_MSG_TO_SEND *Ptr;


if ( User_param->With_Choice_Producer ) {
	int i;
#if	( FDM_WITH_CONTROLS == YES )
	if ( User_param->User_Signal_Mode == NULL ) {
		FDM_Signal_Warning ( (struct _FDM_REF *)Fip  ,
				_INDICATION_PROG_MISSING,  0  );
		goto abort;
	}

#endif


	Ref->MemPool = _NEW_USER ( Fip->H.Memory_Management_Ref , sizeof ( TIME_MSG_TO_SEND ) , 2 ) ;
	if ( Ref->MemPool == ( T_File * ) 0 ) {
		FDM_Free ( Fip , (char*) Ref );
		return (( FDM_GENERIC_TIME_REFERENCE* ) 0 );
	}
	for (i = 0 ; i < 2 ; i++ ) {
		/* preinit des 2 buffers crees */
		Ptr = ( TIME_MSG_TO_SEND* )_ALLOCATE_MEMORY(  Ref->MemPool ) ;
		memset ( Ptr , 0 , sizeof( TIME_MSG_TO_SEND ));
		Ptr->Desc.Ptr_Block 	= &Ptr->Block;
		Ptr->Desc.Nr_Of_Blocks 	= 1;
		Ptr->Block.Nr_Of_Bytes	= Frame_size+2 ;
		Ptr->Block.Ptr_Data	= (Uchar*)&Ptr->Information_Report_Frame;

		memcpy( Ptr->Information_Report_Frame ,
			 Information_Report_Frame_Init ,
			 sizeof ( Information_Report_Frame_Init ));
		_FREE_MEMORY ((User_GDM *)  Ptr );
	}
     {
	/* initialisation des DL emission */
	FDM_MESSAGING_TO_SEND  FMSend;

	FMSend = DLL_Emetteur_Ordre ;
	FMSend.sending.Channel_Nr = User_param->Channel_Nr ;
	FMSend.sending.User_Ctxt    = ( void* ) Ref ;
	FMSend.Local_DLL_Address |= ((Ulong)Fip->H.K_PHYADR) << 8;
	Ref->EmettreOrdre  = fdm_messaging_to_send_create ( Fip , & FMSend) ;

	FMSend = DLL_Emetteur_Etat ;
	FMSend.sending.Channel_Nr = User_param->Channel_Nr ;
	FMSend.sending.User_Ctxt    = ( void* ) Ref;
	FMSend.Local_DLL_Address |= ((Ulong)Fip->H.K_PHYADR) << 8;
	Ref->EmettreEtat  = fdm_messaging_to_send_create  ( Fip , & FMSend ) ;
    }
   {
	/* initialisation des DL reception */
	FDM_MESSAGING_TO_REC   FMReceiv;

	FMReceiv  = DLL_Recepteur_Ordre ;
	FMReceiv .receiving.User_Ctxt  = ( void* ) Ref ;
	Ref->RecevoirOrdre  = fdm_messaging_to_rec_create ( Fip , & FMReceiv ) ;

	FMReceiv  = DLL_Recepteur_Etat ;
	FMReceiv .receiving.User_Ctxt  = ( void* ) Ref ;
	Ref->RecevoirEtat  = fdm_messaging_to_rec_create  ( Fip , & FMReceiv  ) ;
   }
   /* initialisation Tempos */
   Ref->ticks_etat .T_init  		= User_param->Ticks_Election ;
   Ref->ticks_etat .User_Prog 		= Tempo_Etat ;
   Ref->ticks_etat._.Periodique 	= 1;
   Ref->P1 = Ref;

   Ref->Ticks_Election.T_init  		= User_param->Ticks_Election * 3 ;
   Ref->Ticks_Election._.Periodique 	= 1;
   Ref->Ticks_Election.User_Prog 	= Tempo_Election ;
   Ref->P2 = Ref;
   Ref->Eligible  	= fdm_true;
   Ref->Election_State 	= fdm_false;
   Ref->Priorite = 0 ;

   Ref->StationActive = -1;
   Ref->Status 	= ((int)Fip->H.K_PHYADR << 8) | ( standby << 4) | 0xf;
   }

}
#endif

/************************************************************************************/
/*****************************M P S *************************************************/
/************************************************************************************/
{
Ushort CR ;
FDM_XAE Var = DEF_Generic_Time;

#if (  FDM_WITH_GT_ONLY_PRODUCED == YES )
Ref->user_def.With_MPS_Var_Produced = fdm_true ;
#endif

#if (   FDM_WITH_GT_ONLY_CONSUMED == YES )
Ref->user_def.With_MPS_Var_Consumed = fdm_true ;
#endif

#if	( FDM_WITH_CONTROLS == YES )
	if ( Ref->user_def.With_MPS_Var_Produced ) {
		if  ( User_param->Refreshment == 0 ) {
			FDM_Signal_Warning ( (struct _FDM_REF *)Fip  ,
				 _GEN_AE_LE_ILLEGAL_CONFIGURATION ,  _MISSING_PERIOD);
			goto abort;
		}
	}

	if ( Ref->user_def.With_MPS_Var_Consumed ) {
		if  ( User_param->Promptness == 0 ) {
			FDM_Signal_Warning ( (struct _FDM_REF *)Fip  ,
				 _GEN_AE_LE_ILLEGAL_CONFIGURATION ,  _MISSING_PERIOD);
			goto abort;
		}
	}
#endif


	if ( Ref->user_def.With_MPS_Var_Consumed || Ref->user_def.With_MPS_Var_Produced ) {
		Ref->Generic_AELE = fdm_ae_le_create(Fip,1,CHANGE_NOT_ALLOWED);


		if ( User_param->With_Choice_Producer ) {
			if ( Ref->user_def.With_MPS_Var_Consumed == fdm_false ) {
				if (  Ref->user_def.With_MPS_Var_Produced ) {
					/* si produit seul + election , il faut configurer en mode Prod <-> Cons */
					Var.Type.Communication 	= _PrCo ;
					Var.Type.Refreshment 	= _Refre;
					Var.Refreshment_Period	= User_param->Refreshment;
					Var.Promptness_Period	= 1000;
					Var.Type.Position 	= User_param->Image ;
					goto suite;
				}
			}
		}

		if ( Ref->user_def.With_MPS_Var_Produced && Ref->user_def.With_MPS_Var_Consumed) {
			Var.Type.Communication 	= _PrCo ;
			Var.Type.Refreshment 	= _Refre;
		} else {
			if ( Ref->user_def.With_MPS_Var_Produced ) {
				Var.Type.Communication 	= _Prod  ;
			}
			if ( Ref->user_def.With_MPS_Var_Consumed ) {
				Var.Type.Communication 	= _Cons  ;
				Var.Type.Refreshment 	= _Refre;
			}
		}

		Var.Promptness_Period	= User_param->Promptness;
		Var.Refreshment_Period	= User_param->Refreshment;
		Var.Type.Position 	= User_param->Image ;
suite :
		Ref->Generic_var = fdm_mps_var_create( Ref->Generic_AELE,  &Var );
		CR = fdm_ae_le_start( Ref->Generic_AELE ) ;
		if (CR != 0 ) goto abort;
	}
}

#if ( FDM_WITH_GT == YES )

if ( User_param->With_Choice_Producer ) {
	_TIMER_START ( &Ref->Ticks_Election );
	if ( Ref->user_def.With_MPS_Var_Produced )
		_TIMER_START ( &Ref->ticks_etat   );
}

#endif

	return Ref;

abort:
	fdm_generic_time_delete ( Ref );
	return (( FDM_GENERIC_TIME_REFERENCE *)NULL);

}

Ushort
fdm_generic_time_delete ( FDM_GENERIC_TIME_REFERENCE *GT)
{

        if ( GT == NULL) return (FDM_NOK);


	if ( GT->user_def.With_Choice_Producer ) {
		GT->Stopped  =  fdm_true;
		_TIMER_KILL ( &GT->Ticks_Election );
		_TIMER_KILL ( &GT->ticks_etat   );
 		while (fdm_messaging_delete(GT->EmettreEtat) == FDM_NOK )
		{
			_FIP_PAUSE(100);
		}
 		while (fdm_messaging_delete(GT->EmettreOrdre)== FDM_NOK )
		{
			_FIP_PAUSE(100);
		}
 		fdm_messaging_delete(GT->RecevoirEtat);
 		fdm_messaging_delete(GT->RecevoirOrdre);
	}
	if ( GT->user_def.With_MPS_Var_Consumed || GT->user_def.With_MPS_Var_Produced ) {
	 	fdm_ae_le_stop   	( GT->Generic_AELE  );
	 	fdm_ae_le_delete 	( GT->Generic_AELE  );
	}

	FDM_Free ( GT->Fip , (void *) GT );
	return FDM_OK;
}
