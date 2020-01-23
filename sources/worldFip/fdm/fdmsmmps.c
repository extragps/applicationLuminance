/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmsmmps.c
  Description     : Gestion des variables SM_MPS



  Date de creation: Fri Mar 15 15:11:46 MET 1996

  Historique des evolutions/modifications
  	Modification : 96.07.26
  	Reference Ra : RA xx
  	But de la modification :
  		modif Buffer--> FBuffer
  		(nom Buffer utilise dans <stdio.h> compilateur METAWARE 16bits)
Version:4.1.1
	a/-  Suppression de warning GNU

Version:4.1.
	a/-  Variable de rapport sur cible "motorola"

Version:4.2.1.
	nouvelle primitive :
	- Ushort fdm_get_local_report ( FDM_REF *Fip , FDM_REPORT_VAR * User_buffer ) ;

Version:4.3.1.  prise en compte de FIPIO
	a/- Update periodique var param segment
	b/- liste des presents MONO - BI
	c/- suppression de DEF_EVT qd UP_DATE ( fait par FIPCODE)

Version:4.4.4.
	a/- READ_VAR_LP (acces libre masquage+ Copmte Rendu SI PB FIPCODE )

***********************************************************/
#include "user_opt.h"

#include "fdm.h"

#include "fdmmacro.h"


Ushort fdm_get_local_report 	( FDM_REF *Fip , FDM_REPORT_VAR * User_buffer )
{
	Uchar buffer [ sizeof (struct _Report_Variable ) + 2 ];
	int i ,j;
	Ushort *Ptr,V;

	memcpy (  &buffer[2] ,  &Fip->FDM_Internal_Use.Report_Variable , sizeof(struct _Report_Variable)  );

	Ptr = &User_buffer->Nb_Of_Transaction_Ok_1;
	for ( j = 0 ,i = 3  ; j < 5 ; j ++) {
		V = (Ushort) buffer[i++];
		V <<= 8;
		V |= (Ushort) buffer[i++];
		*Ptr++ = V;
		i++;/* skip PDU */
	}
	User_buffer->Report 	= VAR_TRANSFERT_OK;
	User_buffer->Subscriber = Fip->H.K_PHYADR;

	return 0;
}

#if		( FDM_WITH_SM_MPS == YES )
static Ushort sm_mps_read_universal ( FDM_MPS_VAR_REF  *EL )
{
	FDM_REF *Fip ;

	if (EL == NULL ) return (FDM_NOK);


	Fip = EL->Ref->Fip;
	if ( EL->Ref->Etat != AE_LE_RUNNING ){	/* !running */
		FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_STATE  , 0);
		return (FDM_NOK);
	}

	if ( EL->Compte_rendu == 1 ) {
		FDM_Signal_Warning ( Fip  , _REQUEST_RAN_ALWAYS  , 0);
		return (FDM_NOK);
	}
	/** Controles d'entree ok **/

	Fip->FDM_Internal_Use.fipgen.Xrf->Var[ EL->cle_Con ] = EL ;
	EL->Compte_rendu = 1;

	_TIMER_RETRIG ( &Fip->FDM_Internal_Use.T_aper_N.Timer);
	_TIMER_START ( &EL->Timer) ;
 	_FIP_UPDATE   ( Fip , EL->cle_Con  , 0 );

	return FDM_OK;
}


/***************************************************************/
/*******
	AE_LE STOP_DELETE simplifie ( pour variable SM_MPS )
********/
/***************************************************************/


static Ushort FDM_DESTROY_SM_MPS_VARIABLE ( FDM_MPS_VAR_REF *EL )
{
	FDM_AE_LE_REF *AE_LE;
	FDM_REF *Fip ;
	Ushort Cr;

	AE_LE = EL->Ref;
	Fip = AE_LE->Fip ;

	if ((Uchar)( EL->definition.ID & 0xff) != Fip->H.K_PHYADR ) {
		Cr = fdm_ae_le_stop   ( EL->Ref ) ;
		Cr |= fdm_ae_le_delete ( EL->Ref ) ;
		return Cr;
	}
	else return FDM_OK;

}


/***************************************************************/
/*******
	cree une AE_LE contenenat une variable SM_MPS presence
	demande un READ_UNIVERSAL de la variable ainsi cree

********/
/***************************************************************/



/*******************************************************/
/********************* Presence  ***********************/
/*******************************************************/


static const FDM_XAE	SM_MPS_Presence =
{
/* r p c l t r e p r m     i         c       p   r         */
/* e o o o i a v r q s     d         d       e   a         */
/* s s m c m f e i a g     e         o       r   n         */
/* e i m a   e f n o         n               i   k         */
  {0,3,2,0,0,0,0,0,0,0},0x1400,     0x05,    0,0,0,
{
  (void (*) ( struct _FDM_REF * ) )       0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
};



static Ushort FDM_GET_PRESENCE ( FDM_MPS_VAR_REF *El ,	FDM_PRESENCE_VAR *User_buffer)
{

	FDM_REF *Fip ;
	Uchar buffer [8];
	FDM_MPS_READ_STATUS Cr ;

	Fip = El->Ref->Fip ;

	if ( User_buffer->Subscriber == Fip->H.K_PHYADR) {
		User_buffer->Ident_Length = Fip->FDM_Internal_Use.Presence.lg_ident;
		User_buffer->BA_Information.BA_Status   = (Uchar)(Fip->FDM_Internal_Use.Presence.Status >> 4);
		User_buffer->BA_Information.BA_Priority = (Uchar)(Fip->FDM_Internal_Use.Presence.Status & 0xf);
		return 0;
	}

	Cr = fdm_mps_var_read_loc ( El , ( FDM_MPS_VAR_DATA * ) buffer );
	if ( Cr.Report != _FDM_MPS_READ_OK ) return 0xffff;

	User_buffer->Ident_Length = buffer[4];
	User_buffer->BA_Information.BA_Status   = (Uchar)(buffer[6] >> 4 );
	User_buffer->BA_Information.BA_Priority = (Uchar)(buffer[6] & 0xf);
	return 0;
}

Ushort fdm_read_presence  ( FDM_REF *Ref , Uchar Subscriber )
{
	FDM_AE_LE_REF *Presence;
	FDM_MPS_VAR_REF *El ;
	DESC_TRAME_TYPE *DT;
	Ulong ID;
	FDM_XAE     X_Presence ;
	Ushort U;

        El = NULL;

	if ( Subscriber== Ref->H.K_PHYADR) {
		_FDM_XAE_Tempo_Ref *T;
		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Presence.Var[0].Timer;

		if ( T->Compte_rendu != 1 ) {
		/* initialisation 1er coup */
			T->Compte_rendu = 1;
			Presence = &Ref->FDM_Internal_Use.SM_MPS_Presence;
			Presence->Etat = AE_LE_CONFIG;
			Presence->dim  = 1;
			Presence->Fip  = Ref ;
			X_Presence     = SM_MPS_Presence ;
			X_Presence.ID 		 = (Ushort)(0x1400 | Subscriber);
			X_Presence.Type.Position = ( Ref->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

			El = fdm_mps_var_create ( Presence ,  &X_Presence );
		}
		T->Timer.T_cur = 1;

		OS_Enter_Region();
		STORE_IN_FIFO_INTEG(  Ref->FDM_Internal_Use.FIFO_SM_MPS ,
				(INTEG_FILE_ELT *) T) ;
		OS_Leave_Region();
		return FDM_OK;
	}

	ID = 0x1400 | Subscriber;

	DT = (DESC_TRAME_TYPE *) _DICHO_SCAN_OBJECT(
			&Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE , ID  );

	if (( DT == NULL ) ||
	    ( DT->Var_Desc_Ptr_Consommee == 0 )) {
		Presence = fdm_ae_le_create ( Ref , 1 , CHANGE_NOT_ALLOWED);
                if (Presence == NULL)
                	return 0xffff;
	        X_Presence = SM_MPS_Presence ;
		X_Presence.ID 		 = (Ushort)ID;
		X_Presence.Type.Position = ( Ref->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

		El = fdm_mps_var_create ( Presence , &X_Presence );
		El->Timer.File    	= Ref->FDM_Internal_Use.FIFO_SM_MPS ;
		El->Timer._.Type_out	= 1;

		U = Internal_fdm_ae_le_start ( Presence ,0x5000);

		if ( U != 0 )
		{
			fdm_ae_le_delete(Presence);
			return U;
		}
	}
	else 
	{
		U = DT->Var_Desc_Ptr_Consommee >> 4;
		El  =  Ref->FDM_Internal_Use.fipgen.Xrf->Var[ U ] ;

	}
    if ( El == NULL ) return 0xffff;

	U = sm_mps_read_universal( El );

	return U;

}



/*******************************************************/
/********************* Repport   ***********************/
/*******************************************************/


static const FDM_XAE	SM_MPS_Repport =
{
/* r p c l t r e p r m     i         c       p    r         */
/* e o o o i a v r q s     d         d       e    a         */
/* s s m c m f e i a g     e         o       r    n         */
/* e i m a   e f n o         n               i    k         */
  {0,3,2,0,0,0,0,0,0,0},0x1100,     0x0f,    0,0, 0,
{
  (void (*) ( struct _FDM_REF * ) )       0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
};



static	Ushort FDM_GET_REPPORT ( FDM_MPS_VAR_REF *El ,	FDM_REPORT_VAR *User_buffer)
{
	FDM_REF *Fip ;
	Uchar buffer [ 64 ];
	FDM_MPS_READ_STATUS Cr ;

	Fip = El->Ref->Fip ;


	if ( User_buffer->Subscriber == Fip->H.K_PHYADR) {
		memcpy (  &buffer[2] ,  &Fip->FDM_Internal_Use.Report_Variable , sizeof(struct _Report_Variable)  );
	} else {

		Cr = fdm_mps_var_read_loc ( El , ( FDM_MPS_VAR_DATA * )buffer );
		if ( Cr.Report != _FDM_MPS_READ_OK ) return 0xffff;
	}

	{
	int i ,j;
	Ushort *Ptr,V;
	Ptr = &User_buffer->Nb_Of_Transaction_Ok_1;
	for ( j = 0 ,i = 3  ; j < 5 ; j ++) {
		V = (Ushort) buffer[i++];
		V <<= 8;
		V |= (Ushort) buffer[i++];
		*Ptr++ = V;
		i++;/* skip PDU */
	}
	}
	return 0;
}

Ushort fdm_read_report  ( FDM_REF *Ref , Uchar Subscriber )
{
	FDM_AE_LE_REF *Repport;
	FDM_MPS_VAR_REF *El ;
	DESC_TRAME_TYPE *DT;
	Ulong ID;
	FDM_XAE     X_Repport ;
	Ushort U;

        El = NULL;
	if (Subscriber == Ref->H.K_PHYADR) {
		_FDM_XAE_Tempo_Ref *T;
		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Repport.Var[0].Timer;

		if ( T->Compte_rendu == 0 ) {
		/* initialisation 1er coup */
			T->Compte_rendu = 2;
			Repport = &Ref->FDM_Internal_Use.SM_MPS_Repport;
			Repport->Etat = AE_LE_CONFIG;
			Repport->dim  = 1;
			Repport->Fip  = Ref ;
			X_Repport     = SM_MPS_Repport ;
			X_Repport.ID  = (Ushort)(0x1100 | Subscriber) ;
			X_Repport.Type.Position = ( Ref->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

			El = fdm_mps_var_create ( Repport ,  &X_Repport );
		}

		T->Timer.T_cur = 1;
		OS_Enter_Region();
		STORE_IN_FIFO_INTEG(  Ref->FDM_Internal_Use.FIFO_SM_MPS ,
				(INTEG_FILE_ELT *) T) ;
		OS_Leave_Region();
		return FDM_OK;
	}

	ID = 0x1100 | Subscriber;

	DT = (DESC_TRAME_TYPE *) _DICHO_SCAN_OBJECT(
			&Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE , ID  );
	if (( DT == NULL ) ||
	    ( DT->Var_Desc_Ptr_Consommee == 0 )) {
		Repport = fdm_ae_le_create ( Ref , 1 , CHANGE_NOT_ALLOWED);
                if (Repport == NULL)
                	return 0xffff;
	        X_Repport = SM_MPS_Repport ;
		X_Repport.ID 		 = (Ushort)ID ;
		X_Repport.Type.Position = ( Ref->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

		El = fdm_mps_var_create ( Repport , &X_Repport );
		El->Timer.File    	= Ref->FDM_Internal_Use.FIFO_SM_MPS ;
		El->Timer._.Type_out	= 1;

		U = Internal_fdm_ae_le_start ( Repport , 0x5000);

		if ( U != 0 )
		{
			fdm_ae_le_delete(Repport);
			return U;
		}
	}
   	else 
	{
		U = DT->Var_Desc_Ptr_Consommee >> 4;
		El  =  Ref->FDM_Internal_Use.fipgen.Xrf->Var[ U ] ;

	}
    
	if ( El == NULL ) return 0xffff;


	U = sm_mps_read_universal( El );

	return U;

}




/*******************************************************/
/**************** identification ***********************/
/*******************************************************/

/* reserv = 1 car variable a lg variable */
static const FDM_XAE	SM_MPS_Identification =
{
/* r p c l t r e p r m     i         c       p   r        */
/* e o o o i a v r q s     d         d       e   a        */
/* s s m c m f e i a g     e         o       r   n        */
/* e i m a   e f n o       n                 i   k        */
  {1,3,2,0,0,0,0,0,0,0},0x1000,    0x00,    0,0, 0,
{
  (void (*) ( struct _FDM_REF * ) )       0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
};




/***************************************************************/
/*******
	cree une AE_LE contenenat une variable SM_MPS
	demande un READ_UNIVERSAL de la variable cree

********/
/***************************************************************/

Ushort fdm_read_identification  ( FDM_REF *Ref , Uchar Subscriber )
{
	FDM_AE_LE_REF *Identification;
	FDM_MPS_VAR_REF *El ;
	DESC_TRAME_TYPE *DT;
	Ulong ID;
	FDM_XAE     X_Identification ;
	Ushort U;

        El = NULL;
	if (Subscriber == Ref->H.K_PHYADR) {
		_FDM_XAE_Tempo_Ref *T;
		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Identification.Var[0].Timer;

		if ( T->Compte_rendu == 0 ) {
		/* initialisation 1er coup */
			T->Compte_rendu = 2;
			Identification = &Ref->FDM_Internal_Use.SM_MPS_Identification;
			Identification->Etat = AE_LE_CONFIG;
			Identification->dim  = 1;
			Identification->Fip = Ref ;
			X_Identification = SM_MPS_Identification ;
			X_Identification.ID = (Ushort)(0x1000 | Subscriber );
			X_Identification.Type.Position = ( Ref->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

			El = fdm_mps_var_create ( Identification ,  &X_Identification );

		}

		T->Timer.T_cur = 1;
		OS_Enter_Region();
		STORE_IN_FIFO_INTEG(  Ref->FDM_Internal_Use.FIFO_SM_MPS ,
				(INTEG_FILE_ELT *) T) ;
		OS_Leave_Region();

		return FDM_OK;
	}

	ID = 0x1000 | Subscriber;
	DT = (DESC_TRAME_TYPE *) _DICHO_SCAN_OBJECT(
			&Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE , ID  );
	if (( DT == NULL ) ||
	    ( DT->Var_Desc_Ptr_Consommee == 0 )) {
		Identification = fdm_ae_le_create ( Ref , 1 , CHANGE_NOT_ALLOWED);
                if (Identification == NULL)
                	return 0xffff;
	    X_Identification = SM_MPS_Identification ;
		X_Identification.ID 		 = (Ushort)ID ;
		X_Identification.Type.Position = ( Ref->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

		El = fdm_mps_var_create ( Identification , &X_Identification );
		El->Timer.File    	= Ref->FDM_Internal_Use.FIFO_SM_MPS ;
		El->Timer._.Type_out	= 1;

		U = Internal_fdm_ae_le_start ( Identification , 0x5000);

		if ( U != 0 )
		{
			fdm_ae_le_delete(Identification);
			return U;
		}
	}
	else 
	{
		U = DT->Var_Desc_Ptr_Consommee >> 4;
		El  =  Ref->FDM_Internal_Use.fipgen.Xrf->Var[ U ] ;

	}
    if ( El == NULL )  return 0xffff;

	U = sm_mps_read_universal( El );

	return U;

}



static Ushort FDM_GET_IDENTIFICATION ( FDM_MPS_VAR_REF *El ,  FDM_IDENT_VAR *User_buffer)
{
	FDM_REF *Ref ;

	Ref = El->Ref->Fip ;

	if  ( (Uchar)( El->definition.ID  ) == Ref->H.K_PHYADR) {
		memcpy ( &User_buffer->Identification , Ref->FDM_Internal_Use.Ident_buffer ,
			Ref->FDM_Internal_Use.Ident_buffer[1]+2 );

		User_buffer->Report     = VAR_TRANSFERT_OK;
		User_buffer->Subscriber = (Uchar)( El->definition.ID & 0xff);
		User_buffer->Identification_length = (Ushort)(User_buffer->Identification[1] + 2);

		return 0;
	}
#if 					( FDM_WITH_FREE_ACCESS == NO )
	{/******* read distant ***************/
	int VAR_STATE ;
	Acces_Key Key ;
	FDM_ERROR_CODE CR ;

	int tempo;

	unsigned int taille;

    	FIFO_Reg;

	unsigned char *dest;

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	Key = El->cle_Con;
	ECRIRE_Key( Key );

	/************ ecriture de la commande */
	ECRITE(UCom ,  CMDE_R_VAR_SVF);


	/* ici pour occupe utilement le micro */
	   FIFO_port = FIFO;
	   dest =  User_buffer->Identification  ;

	/************ attente bit SV */
	WAIT_SV;
	_FIP__tres_petite_tempo ( Ref ) ;

	/************ test de valididite de la variable */
	VAR_STATE = LIRE(Var_State) ;
	if ((VAR_STATE & 0x80) != 0 ) {
                ECRITE(UCom,CMDE_CLOSE) ;
		User_buffer->Report = VAR_TRANSFERT_NOK;;
		User_buffer->Subscriber = Ref->H.K_PHYADR;
                Fin_procedure_no_signal_warning;
	}

	/************ boucle de lecture de l'objet */

	taille = LIRE(Var_Size) ;

	BLOCKINBYTE(FIFO_port,dest,taille);

	/************ compte rendu */
	CR_UState;
	User_buffer->Report = (Uchar)( ( CR.Information.Fipcode_Report._Ustate == 0 ) ?
		 VAR_TRANSFERT_OK : VAR_TRANSFERT_NOK);

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	User_buffer->Subscriber = (Uchar)( El->definition.ID );
	User_buffer->Identification_length = (Ushort)(User_buffer->Identification[1] + 2);

	Fin_procedure_no_signal_warning;

	}
#else
	{
	Ushort Cr ;
	Acces_Key Key ;
	Key = El->cle_Con;
	Cr =  _FIP_READ_VAR ( Ref , Key , &User_buffer->Identification );
	User_buffer->Report = (Uchar)
		((( Cr & ~0x20 ) ==  0 ) ? VAR_TRANSFERT_OK : VAR_TRANSFERT_NOK );
	User_buffer->Subscriber = (Uchar)( El->definition.ID & 0xff);
	User_buffer->Identification_length = User_buffer->Identification[1] + 2;
	return 0;
	}
#endif

}





/*******************************************************/
/**************** Liste Present  ***********************/
/*******************************************************/


static const FDM_XAE	SM_MPS_Liste_Present =
{
/* r p c l t r e p r m     i         c       p    r          */
/* e o o o i a v r q s     d         d       e    a          */
/* s s m c m f e i a g     e         o       r    n          */
/* e i m a   e f n o         n               i    k          */
  {0,3,2,0,0,0,0,0,0,0},0x9002,     0x44,    0,0, 0,
{
  (void (*) ( struct _FDM_REF * ) )       0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
};


Ushort fdm_read_present_list  ( FDM_REF *Ref  )
{
	FDM_AE_LE_REF *Liste_Present;
	FDM_MPS_VAR_REF *El ;
	_FDM_XAE_Tempo_Ref *T;
	FDM_XAE     X_Liste_P ;

	T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Liste_Present.Var[0].Timer;
	if ( T->Compte_rendu == 1 ) return FDM_NOK; /* already in use */

	if ( T->Compte_rendu == 0 ) {
	/* initialisation 1er coup */
		T->Compte_rendu = 2;
		Liste_Present = &Ref->FDM_Internal_Use.SM_MPS_Liste_Present ;
		Liste_Present->Etat = AE_LE_CONFIG;
		Liste_Present->dim  = 1;
		Liste_Present->Fip  = Ref ;
		X_Liste_P = SM_MPS_Liste_Present;
		X_Liste_P.Type.Position = ( Ref->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

		El = fdm_mps_var_create ( Liste_Present ,  &X_Liste_P );
		T->Timer.File    	= Ref->FDM_Internal_Use.FIFO_SM_MPS ;
		El->Timer._.Type_out	= 1;
		Liste_Present->Etat = AE_LE_RUNNING;
		El->cle_Con = CLE_LIST_PRE_C ;
		Ref->FDM_Internal_Use.fipgen.Xrf->Var[ CLE_LIST_PRE_C ] = El;

	} else {
		El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
	}


	if ( Ref->FDM_Internal_Use.Ba.BA_running ) {

#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
		if ( ( Ref->FDM_Internal_Use.Ba.B0_tst_p & 0x1000 ) != 0 )
			T->Timer.T_cur = 1;
		else
			T->Timer.T_cur = 0;

#else
			T->Timer.T_cur = 1;
#endif
		T->Compte_rendu  = 1;

		OS_Enter_Region();
		STORE_IN_FIFO_INTEG(  Ref->FDM_Internal_Use.FIFO_SM_MPS ,
				(INTEG_FILE_ELT *) T) ;
		OS_Leave_Region();

	} else {

		T->Timer.T_cur 	 = Ref->S.Time_Out_Ticks;
		T->Compte_rendu  = 1;
		_TIMER_START ( &El->Timer) ;
		_FIP_DEF_EVT  ( Ref , CLE_LIST_PRE_C , 0x0200   ) ;

	}
	return FDM_OK ;

}


static Ushort _FIP_GET_LISTE_PRESENT ( FDM_REF *Ref ,  FDM_PRESENT_LIST *destination)
{
#if 	(  FDM_WITH_FREE_ACCESS == NO )

	volatile int V;
	int VAR_STATE ;
	FDM_ERROR_CODE CR ;
	int tempo;
	unsigned int taille;
    	FIFO_Reg;
	Uchar *dest , Lg_Trame;


	Debut_procedure ;

	WAIT_Busy;

		/************ ecriture de la commande */
	ECRITE(UCom ,  CMDE_R_LPRESENT );

		/* ici pour occupe utilement le micro */
	FIFO_port = FIFO;
	dest =  &destination->Bus1_P[0];
  	taille = 32 ;
	destination->Report = VAR_TRANSFERT_OK;

	/************ attente bit SV */
	WAIT_SV;
	_FIP__tres_petite_tempo ( Ref ) ;

	/************ test de valididite de la variable */
	VAR_STATE = LIRE(Var_State) ;
	if ((VAR_STATE & 0x80) != 0 ) {
	        ECRITE(UCom,CMDE_CLOSE) ;
		destination->Report = VAR_TRANSFERT_NOK;;
		Fin_procedure_no_signal_warning;
	}

	/************ boucle de lecture de l'objet */
	V = LIRE(FILE_FIP);
	Lg_Trame = LIRE(FILE_FIP);
	V = LIRE(FILE_FIP);
	V = LIRE(FILE_FIP);
	BLOCKINBYTE(FIFO_port,dest,taille);
	if ( Lg_Trame == 0x22 ) {
		memset (  &destination->Bus2_P[0] , 0 , 32 );
	} else {
		V = LIRE(FILE_FIP);
		V = LIRE(FILE_FIP);
		taille = 32 ;
		dest =  &destination->Bus2_P[0];
		BLOCKINBYTE(FIFO_port,dest,taille);
	}
	/************ compte rendu */
	CR_UState;

	/************ close */
   	ECRITE(UCom,CMDE_CLOSE) ;

	WAIT_Busy;

	Fin_procedure_no_signal_warning;
#else

	/***********************************************/
	/**************** Acces libre ******************/
	/**************** FIP_READ_VAR_LP  *************/
	/***********************************************/

	volatile Ushort   *Abs_ADR ,*buffer  ;
	T_Desc_Ptr *Desc_Ptr ;
	volatile Ushort   *FifoUser, *FifoFip ;
	Dial_AE *PDial_AE;
	AE_C_DESC_VAR_TYPE *DescC;
	Ulong P_Value_PTR ;
	int Lg_Var ;
	register unsigned  UTmp;
	int  tempo;
	Ushort CR , Tmp , Lg_Trame ;


	Desc_Ptr = ( T_Desc_Ptr *) Ref->FDM_Internal_Use.AL.BASE_1 ;
	DescC     = (AE_C_DESC_VAR_TYPE*) &Desc_Ptr->Desc[ CLE_LIST_PRE_C ];

	MAC__INIT_REG;

	CR = VAR_TRANSFERT_NOK   ;

	MAC__WAIT_PLACE	;


	MAC__ECRIRE_DEMANDE(READ_VAR_9002_USER,0);

	MAC__WAIT_DONNEE_PRETE;

	if ( DescC->Var_Status  != 7 ) {
		goto abortd;
	}
  	CR =   VAR_TRANSFERT_OK ;
	MAC__ADR_DONNEES(DescC->Trans_Delai);

	buffer = (Ushort *) destination;

	/* Get taille variable */
	Lg_Trame = 	*Abs_ADR++ ;
	Abs_ADR++;

	Lg_Var   = 16  ;
	buffer    = (Ushort*) &destination->Bus1_P[0];

	do {
		UTmp  =  *Abs_ADR++ ;
#if	( FDM_WITH_LITTLE_ENDIAN == YES )
		UTmp = (UTmp >> 8 ) | (UTmp << 8 ) ;
#endif
		*buffer++  = (Ushort) UTmp   ;
	} while ( --Lg_Var != 0  ) ;
	if (  Lg_Trame == 0x5022 ) {
		buffer    = (Ushort*) &destination->Bus2_P[0];
		memset ( (Uchar*)buffer , 0 , 32 );
	} else {
		Abs_ADR++;
		Lg_Var   = 16  ;
		buffer    = (Ushort*) &destination->Bus2_P[0];

		do {
			UTmp  =  *Abs_ADR++ ;
#if	( FDM_WITH_LITTLE_ENDIAN == YES )
			UTmp = (UTmp >> 8 ) | (UTmp << 8 ) ;
#endif
			*buffer++  = (Ushort) UTmp   ;
		} while ( --Lg_Var != 0  ) ;
	}


  	CR =   VAR_TRANSFERT_OK ;

abortd :

	DescC->Var_Status = 0;
	destination->Report = (unsigned char) CR ;
	return CR ;

#endif
}

static Ushort FDM_GET_LISTE_PRESENT ( FDM_REF *Ref ,  FDM_PRESENT_LIST *destination)
{


#if 	( FDM_WITH_REDONDANCY_MGNT == YES )

	if ( Ref->FDM_Internal_Use.Ba.BA_running ) {


		if ( ( Ref->FDM_Internal_Use.Ba.B0_tst_p & 0x1000 ) != 0 ) {
			Uchar *sour;
			Uchar *dest;
		   	destination->Report = VAR_TRANSFERT_OK;
		   	sour = ( Uchar *) &Ref->FDM_Internal_Use.Liste_present.V0[0];
		   	dest = ( Uchar *) &destination->Bus1_P[0];
			memcpy (dest , sour , 32 );
		   	sour = ( Uchar *) &Ref->FDM_Internal_Use.Liste_present.V1[0];
		   	dest = ( Uchar *) &destination->Bus2_P[0];
			memcpy (dest , sour , 32 );
		} else {
		        destination->Report = VAR_TRANSFERT_NOK;
		}
 	        return 0;
	} else {
		return ( _FIP_GET_LISTE_PRESENT ( Ref ,  destination ) );
	}

#else

	return ( _FIP_GET_LISTE_PRESENT ( Ref ,  destination ) ) ;

#endif
}



/*******************************************************/
/****************Synchro BA      ***********************/
/*******************************************************/



Ushort  fdm_read_ba_synchronize ( FDM_REF *Ref  )
{
	FDM_MPS_VAR_REF *El ;
	_FDM_XAE_Tempo_Ref *T;

	T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Synchro_BA.Var[0].Timer;

	El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;

	if ( Ref->FDM_Internal_Use.Ba.BA_running ) {

		if ( T->Compte_rendu == 1 ) {
			FDM_Signal_Warning ( Ref  , _REQUEST_RAN_ALWAYS  , 0);
			return (FDM_NOK);
		}

		OS_Enter_Region();
		T->Timer.T_cur = 1;
		T->Compte_rendu = 1;
		STORE_IN_FIFO_INTEG(  Ref->FDM_Internal_Use.FIFO_SM_MPS ,
				(INTEG_FILE_ELT *) T) ;
		OS_Leave_Region();

		return FDM_OK;

	} else {
		return ( sm_mps_read_universal( El ) );
	}


}



static 	void FDM_GET_SYNCHRO_BA_VAR ( FDM_MPS_VAR_REF *El  , FDM_SYNCHRO_BA_VAR *FBuffer )
{
	FDM_REF *Ref ;
	FDM_MPS_VAR_DATA tmp;
	FDM_MPS_READ_STATUS Cr;

	Ref = El->Ref->Fip ;

	if ( Ref->FDM_Internal_Use.Ba.BA_running ) {
		FBuffer->Report = VAR_TRANSFERT_OK;
		FBuffer->Hi_MC_Nr   = Ref->FDM_Internal_Use.Synchro_BA.MC_High;
		FBuffer->Subscriber = Ref->FDM_Internal_Use.Synchro_BA.Station;
	} else {
		Cr = fdm_mps_var_read_loc ( El , &tmp  );
		if ( Cr.Report != _FDM_MPS_READ_OK ) {
			FBuffer->Report = VAR_TRANSFERT_NOK;
			return;
		}
		FBuffer->Report = VAR_TRANSFERT_OK;
		FBuffer->Hi_MC_Nr    = tmp.FBuffer[2] ;
		FBuffer->Subscriber   = tmp.FBuffer[3] ;
	}
}








/*****************************************************/
/**************** SM_MPS Process *********************/
/*****************************************************/
static void Appli_SM_MPS ( _FDM_XAE_Tempo_Ref *T )
{
	FDM_MPS_VAR_REF *El ;
	FDM_REF *Fip ;


	El = (FDM_MPS_VAR_REF*)T->Ref_XAE;
	El->Timer._.In_File  = 0;
	El->Compte_rendu = 2;
	Fip = El->Ref->Fip ;
	switch ( (El->definition.ID >> 8)){
	case 0x10:
		{
		Ushort U;
		FDM_IDENT_VAR FBuffer;
		memset ( &FBuffer , 0 , sizeof(FBuffer));

		if (El->Timer.T_cur == 0 ) {
			FBuffer.Report = VAR_TRANSFERT_NOK;
			FBuffer.Subscriber = (Uchar)( El->definition.ID );
		}
		else {
			El->Timer.T_cur = 0 ;
			FDM_GET_IDENTIFICATION ( El , &FBuffer );
			}
		U = Fip->S.User_Identification_Prog( Fip , &FBuffer );
		if ( U == VAR_DELETE )  FDM_DESTROY_SM_MPS_VARIABLE ( El );
		}
		break;
	case 0x11:
		{
		Ushort U;
		FDM_REPORT_VAR FBuffer;
		memset ( &FBuffer , 0 , sizeof(FBuffer));
		FBuffer.Subscriber = (Uchar ) El->definition.ID ;

		if (El->Timer.T_cur == 0 ) FBuffer.Report = VAR_TRANSFERT_NOK;
		else {
			El->Timer.T_cur = 0 ;
			FBuffer.Report = VAR_TRANSFERT_OK;
			U = FDM_GET_REPPORT ( El , &FBuffer );
			if ( U != 0) FBuffer.Report = VAR_TRANSFERT_NOK;;
		}
		U = Fip->S.User_Report_Prog( Fip , &FBuffer );
		if ( U == VAR_DELETE )
			FDM_DESTROY_SM_MPS_VARIABLE ( El );
		}
		break;

	case 0x14:
		{
		Ushort U;
		FDM_PRESENCE_VAR FBuffer;
	 	memset ( &FBuffer , 0 , sizeof(FBuffer));
		FBuffer.Subscriber = (Uchar ) El->definition.ID ;

		if (El->Timer.T_cur == 0 ) FBuffer.Report = VAR_TRANSFERT_NOK;
		else {
			El->Timer.T_cur = 0 ;
			FBuffer.Report = VAR_TRANSFERT_OK;
			U = FDM_GET_PRESENCE ( El , &FBuffer );
			if ( U != 0) FBuffer.Report = VAR_TRANSFERT_NOK;;
		}
		U = Fip->S.User_Presence_Prog(  Fip , &FBuffer );
		if ( U == VAR_DELETE )
			FDM_DESTROY_SM_MPS_VARIABLE ( El );
		}
		break;

	case 0x90:
		{
		switch ( El->definition.ID ){
		case 0x9002:
			{
			FDM_PRESENT_LIST FBuffer;
			memset ( &FBuffer , 0 , sizeof(FBuffer));
			if (El->Timer.T_cur == 0 )
				FBuffer.Report = VAR_TRANSFERT_NOK;
			else{
				El->Timer.T_cur = 0 ;
				FDM_GET_LISTE_PRESENT ( Fip , &FBuffer );
			}
			Fip->S.User_Present_List_Prog(  Fip , &FBuffer );
			}
			break;
		case 0x9003:
			{
			FDM_SYNCHRO_BA_VAR FBuffer;

			memset ( &FBuffer , 0 , sizeof(FBuffer));
			if (El->Timer.T_cur == 0 )
				FBuffer.Report = VAR_TRANSFERT_NOK;
			else {
				El->Timer.T_cur = 0 ;
				FDM_GET_SYNCHRO_BA_VAR ( El , &FBuffer );
			}
			Fip->S.User_Synchro_BA_Prog(  Fip ,&FBuffer );
			}
			break;

		}
		break;/* fin case90 */
		}
	}

}

#if			( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )

void _UPDATE_SEG_PARAM_VAR (  _FDM_XAE_Tempo_Ref * T)
{
	FDM_MPS_VAR_REF *El ;
	FDM_REF *Fip ;

	El = (FDM_MPS_VAR_REF*)T->Ref_XAE;
	Fip = El->Ref->Fip ;
	_TIMER_RETRIG ( &Fip->FDM_Internal_Use.T_aper_N.Timer);
	_FIP_UPDATE ( Fip ,  El->cle_Pro  ,  0 );

}

#endif

void FDM_INITIALIZE_TASK_SM_MPS ( FDM_REF *Ref ,
			     void (*USER_FUNCT) ( struct _FDM_REF* ) )
{
	SOFT_REFERENCE_TYPE 	*Tache_MSG_S_ref ;
    	SOFT_INTERFACE_TYPE 	SIT;

        CREATE_TASK_ENVIRONNEMENT  ( &Ref->FDM_Internal_Use.Task_Ref_SM_MPS ,
				     (void (*)(void*))USER_FUNCT, (void*)Ref );

    	SIT.Nr_Of_External_In_Files 	= 1;
    	SIT.Software_Loop_Fct           = SOFT_LOOP ;
	SIT.Array_Of_In_Functions[0].Software_Treatment_Fct =
		( void(*)() ) Appli_SM_MPS ;
	Tache_MSG_S_ref = &Ref->FDM_Internal_Use.Soft_Ref_SM_MPS;

    	ATTACH_SOFT_TO_TASK(&Ref->FDM_Internal_Use.Task_Ref_SM_MPS,
			    &SIT ,
			    Tache_MSG_S_ref,
		       	    Ref->FDM_Internal_Use.Files_SM_MPS);

	Ref->FDM_Internal_Use.FIFO_SM_MPS     =
			Tache_MSG_S_ref->Array_Of_In_Files[ 0 ];

}

#endif

