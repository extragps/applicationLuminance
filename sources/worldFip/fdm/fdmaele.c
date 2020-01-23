/************************************************************
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "
  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD

      Projet      : Fip Device Manager Version 4
  Nom du fichier  : fdmaele.c
  Description     : Procedure de gestion des ae_le

  Date de creation: Fri Mar 15 15:11:18 MET 1996
  Historique des evolutions/modifications
  	Modification : 96.07.26
  	Reference Ra : RA xx
  	But de la modification :
  		modif Buffer--> FBuffer
  		(nom Buffer utilise dans <stdio.h> compilateur METAWARE 16bits)
Version:4.1.3
	a/-compte-rendus primitives read_loc :=
		il retournait OK si variable avec Raffraichissement OK et promptitude HS !
Version:4.2.1
	a/-start aele il n'est plus obligatoire que l'ensemble des VComs soient presentes.
Version:4.2.1
	a/- controle lg var heure
	b/- suppression de DEF_EVT qd UP_DATE ( fait par FIPCODE)
Version:4.3.15
	a/controle param avant usuge pour read?write_loc
***********************************************************/
#include "user_opt.h"

#include "fdm.h"


FDM_AE_LE_REF *fdm_ae_le_create (
	FDM_REF *Ref ,
	int  nbre ,
	 enum  _FDM_CHANGE_ALLOWED  Flg )
{
	int i;
	FDM_AE_LE_REF * P ;
	if ( nbre == 0 ) {
		FDM_Signal_Warning ( Ref  , _GEN_AE_LE_ILLEGAL_LENGTH , 0 );
		return ((FDM_AE_LE_REF *)NULL);
	}
	if ( nbre > (( (int)Ref->FDM_Internal_Use.Page_2000.Last_Var_Time >> 4) - 7 )) {
		FDM_Signal_Warning ( Ref  , _GEN_AE_LE_ILLEGAL_LENGTH , 0 );
		return ((FDM_AE_LE_REF *)NULL);
	}
	i = sizeof(FDM_AE_LE_REF) + ((nbre - 1) * sizeof(FDM_MPS_VAR_REF)) ;
	P = ( FDM_AE_LE_REF *) FDM_Malloc( Ref,  i );
	if (P == ( FDM_AE_LE_REF * ) NULL ) {
		FDM_Signal_Warning ( Ref  , _ALLOCATE_MEMORY_FAULT , 0 );
		return ((FDM_AE_LE_REF *)NULL);
	}
	memset ( P , 0 , i );
	P->dim = nbre ;
	P->Etat = AE_LE_CONFIG ;
	P->Fip = Ref;
	P->Flg = Flg ;
	return ((FDM_AE_LE_REF *) P );
}

#if 			( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT ==	NO )
static void EOC_Pulse_Bouchon_Synchro_Proc (struct _FDM_REF *Ref ) {}
#endif
FDM_MPS_VAR_REF * fdm_mps_var_create (	FDM_AE_LE_REF *AE  ,  const FDM_XAE *Var)
{
	FDM_MPS_VAR_REF * El;
	FDM_REF *Fip;
	int rang ;
	rang = Var->Rank;
	Fip = AE->Fip;
	if ( AE->Etat != AE_LE_CONFIG ){          /* EN CONFIGURATION */
		FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_STATE , 0 );
		return (NULL);
	}
	if ( rang >= AE->dim  ){
		FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_RANG , 0 );
		return (NULL);
		}
	El = &AE->Var[rang] ;
	El->definition 	= *Var;
	if ( (AE->Fip->S.Type & TWO_IMAGE_MODE) == 0 )
		El->definition.Type.Position = 1;
       	else {
		if  (El->definition.Type.Position == 0) {
			FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_POSITION , 0 );
			return (NULL);
			}
		}
	if ( El->definition.Type.With_Time_Var) {
		if (( El->definition.Type.Communication & 1 ) != 0 ) {
			if (( El->definition.Type.RQa  ) || ( El->definition.Type.MSGa  )) {
				FDM_Signal_Warning ( Fip  ,  _GEN_AE_LE_ILLEGAL_CONFIGURATION, 0 );
				return (NULL);
			}
		}
		if (( El->definition.Type.Communication &  2) != 0 ) {
			if ( El->definition.Type.Refreshment == 0 )  {
				FDM_Signal_Warning ( Fip  ,  _GEN_AE_LE_ILLEGAL_CONFIGURATION, 0 );
				return (NULL);
			}
		}
		if ( (El->definition.Var_Length & 1 ) != 0 ) { /* must be %2 ! */
			FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_LENGTH , 0 );
			return (NULL);
		}
		if ( El->definition.Var_Length  < 11 ) {
			FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_LENGTH , 0 );
			return (NULL);
		}
		El->definition.Var_Length +=4;
	}
	if (( Var->Type.Communication & 1  ) != 0 ) {
		/* variable produite */
		if ( El->definition.Type.Indication != 0 ) {
			if ( El->definition.Signals.User_Signal_Asent  ==
				(void (*)(struct _FDM_MPS_VAR_REF *))0 ) {
				FDM_Signal_Warning ( Fip ,  _INDICATION_PROG_MISSING,  1) ;
				return (NULL);
			}
		}
	}
	if (( Var->Type.Communication & 2  ) != 0 ) {
		/* variable consommee */
		if ( El->definition.Type.Indication != 0 ) {
			if ( El->definition.Signals.User_Signal_Areceive ==
				(void (*)(struct _FDM_MPS_VAR_REF *))0 ) {
				FDM_Signal_Warning ( Fip ,  _INDICATION_PROG_MISSING,  2 ) ;
				return (NULL);
			}
		}
	}
#if 			( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT ==	YES )
	if ( El->definition.Type.Communication  == 0) {
		if (El->definition.Signals.User_Signal_Synchro  ==
			(void (*)(struct _FDM_REF *))0 ) {
			FDM_Signal_Warning ( Fip , _GEN_AE_LE_SYNCHRO_PROG_MISSING, 0 );
			return (NULL);
		}
	}
#else

	if ( El->definition.Type.Communication  == 0) {
    		if (Fip->S.Type & EOC_PULSE_MODE ) {
			if (El->definition.Signals.User_Signal_Synchro  != NULL ) {
				FDM_Signal_Warning ( Fip , _GEN_AE_LE_SYNCHRO_PROG_ILLEGAL  , 0 );
				return (NULL);
			}
			El->definition.Signals.User_Signal_Synchro = EOC_Pulse_Bouchon_Synchro_Proc;
    		} else {
			if (El->definition.Signals.User_Signal_Synchro  == NULL ) {
				FDM_Signal_Warning ( Fip , _GEN_AE_LE_SYNCHRO_PROG_MISSING  , 0 );
				return (NULL);
			}
		}
    	}
#endif
#if ( FDM_WITH_FIPCODE_5000 == YES )
if ( El->definition.Type.Reserved == 0 )  {
	if ( El->definition.Type.Communication  != 0) {
    		switch ( Fip->S.Mode) {
		case  WORLD_FIP_5000 :
		case  FIP_5000 :
			if ( El->definition.Var_Length  < 4 ) {
			FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_LENGTH , 0 );
			return (NULL);
			}
			break;
		default:
			break;
		}
	}
}
#endif
	El->Ref_XAE	= &AE->Var[rang] ;
	El->Ref 	= AE;
	El->Valid 	= 0x55aa;
	El->Timer.T_init  	= Fip->S.Time_Out_Ticks;
	El->Timer._.Type_out	= 1;
	El->Timer._.Mode	= 1;
	El->Timer._.User_Mode_mort = 1;
	El->Timer.File    	= Fip->FDM_Internal_Use.FIFO_MPS ;
	return ( El );
}

Ushort Internal_fdm_ae_le_start  ( FDM_AE_LE_REF *AE ,Ushort PDU )
{
	int rang, CR ;
	FDM_REF *Fip ;
	if ( AE == NULL ) return (FDM_NOK);
	Fip = AE->Fip ;
	if ( AE->Etat != AE_LE_CONFIG){          /* EN CONFIGURATION */
		FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_STATE  , 0 );
		return (FDM_NOK);
	}

	OS_fdm_sm_p_bd( Fip ) ;
	for ( rang = 0 ; rang < AE->dim ; rang++){ /* creation */
		FDM_MPS_VAR_REF * El;
		El = &AE->Var[rang] ;
		if ( El->Valid == 0x55aa ) {
			CR = CRE_DTRAME( Fip , &El->definition , PDU , NULL ) ;
			if (CR !=0) goto abort;
		}
	}
	FDM_STOP_DB_MOD ( Fip ) ;
	{/* Affectation des cles de descripteurs de variables */
	    for ( rang = 0 ; rang < AE->dim ; rang++){ /* creation */
		FDM_MPS_VAR_REF * El;
		DESC_TRAME_TYPE *DT;
		El = &AE->Var[rang] ;
		if ( El->Valid == 0x55aa ) {
			DT = (DESC_TRAME_TYPE *) _DICHO_SCAN_OBJECT(
				&Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE ,
				(Ulong) El->definition.ID  );
			El->cle_Pro = DT->Var_Desc_Ptr_Produite  >> 4 ;
			El->cle_Con = DT->Var_Desc_Ptr_Consommee >> 4 ;
		}
	    }
	}
	AE->Etat = AE_LE_RUNNING;
	OS_fdm_sm_v_bd( Fip ) ;
	return FDM_OK;
abort :
	for (  ; rang >= 0; rang--){ /* destruction suite a erreur */
		FDM_MPS_VAR_REF * El;
		El = &AE->Var[rang] ;
		if ( El->Valid == 0x55aa ) {
			DESC_TRAME_TYPE *DT;
			DT = (DESC_TRAME_TYPE *) _DICHO_SCAN_OBJECT(
				&Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE ,
				(Ulong) El->definition.ID  );
			if ( DT != (DESC_TRAME_TYPE *) 0 ) {
				DT->position &= ~_position_Var;
				SUP_DTRAME(  Fip ,  AE->Var[rang].definition.ID ) ;
			}
		}
	}
        DELETE_DTRAME ( Fip  ) ;
	OS_fdm_sm_v_bd( Fip ) ;
	return (FDM_NOK);
}


Ushort fdm_ae_le_start  ( FDM_AE_LE_REF *AE )
{
	return ( Internal_fdm_ae_le_start  ( AE , 0x4000 ));
}
Ushort fdm_ae_le_stop   ( FDM_AE_LE_REF * AE )
{
	int rang ;
	FDM_REF *Fip ;
	if ( AE == NULL ) return (FDM_NOK);
	if ( AE->Etat != AE_LE_RUNNING ){          /* En Run */
		FDM_Signal_Warning ( AE->Fip  , _GEN_AE_LE_ILLEGAL_STATE , 0);
		return (FDM_NOK);
	}
	AE->Etat = AE_LE_CONFIG;
	Fip = AE->Fip ;
	OS_fdm_sm_p_bd(Fip) ;
	for ( rang = 0 ; rang < AE->dim ; rang++){
		DESC_TRAME_TYPE *DT;
		FDM_MPS_VAR_REF * El;
		Ulong ID ;
                Ushort Mask;
		El = &AE->Var[rang] ;
		if ( El->Valid == 0x55aa ) {
			ID = AE->Var[rang].definition.ID ;
			DT = (DESC_TRAME_TYPE *) _DICHO_SCAN_OBJECT(
				&Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE , ID  );
                        Mask = _position_Var | _position_I1 | _position_I2 | _position_Cons;
			DT->position &= ~Mask;
			SUP_DTRAME(  Fip ,(Ushort) ID ) ;
		}
	}
	FDM_STOP_DB_MOD ( Fip );
	OS_fdm_sm_v_bd(Fip) ;
	return FDM_OK;
}
Ushort fdm_ae_le_delete   ( FDM_AE_LE_REF * AE )
{
	int i ;
	FDM_REF *Fip;
	if ( AE == NULL ) return (FDM_NOK);
	if ( AE->Etat != AE_LE_CONFIG){          /* En Config */
		FDM_Signal_Warning ( AE->Fip  , _GEN_AE_LE_ILLEGAL_STATE , 0 );
		return (FDM_NOK);
	}
	Fip = AE->Fip ;
	i = sizeof(FDM_AE_LE_REF) + ((AE->dim - 1) * sizeof(FDM_MPS_VAR_REF)) ;
	memset ( AE , 0 , i );
	FDM_Free( Fip , (char*)AE);
	return FDM_OK;
}

Ushort fdm_mps_var_change_id   ( FDM_MPS_VAR_REF * El , Ushort ID )
{
	if (El == NULL ) return (FDM_NOK);
	if ( El->Ref->Etat == AE_LE_RUNNING ){          /* En Config */
		FDM_Signal_Warning ( El->Ref->Fip  , _GEN_AE_LE_ILLEGAL_STATE , 0 );
		return (FDM_NOK);
	}
	El->definition.ID = ID;
	return FDM_OK;
}
Ushort fdm_mps_var_change_RQa   ( FDM_MPS_VAR_REF * El , enum FDM_FLAGS F )
{
	if (El == NULL ) return (FDM_NOK);
	if ( El->Ref->Etat == AE_LE_RUNNING ){          /* En Config */
		FDM_Signal_Warning ( El->Ref->Fip  , _GEN_AE_LE_ILLEGAL_STATE , 0 );
		return (FDM_NOK);
	}
#if	( FDM_WITH_CONTROLS == YES )
	if ( El->definition.Type.With_Time_Var  )  {
		FDM_Signal_Warning ( El->Ref->Fip  , _GEN_AE_LE_ILLEGAL_MODIFICATION  , 0 );
		return (FDM_NOK);
	}
#endif
	El->definition.Type.RQa =  F;
	return FDM_OK;
}
Ushort fdm_mps_var_change_MSGa   ( FDM_MPS_VAR_REF * El , enum FDM_FLAGS F )
{
	if (El == NULL ) return (FDM_NOK);
	if ( El->Ref->Etat == AE_LE_RUNNING ){          /* En Config */
		FDM_Signal_Warning ( El->Ref->Fip  , _GEN_AE_LE_ILLEGAL_STATE , 0 );
		return (FDM_NOK);
	}
#if	( FDM_WITH_CONTROLS == YES )
	if ( El->definition.Type.With_Time_Var  )  {
		FDM_Signal_Warning ( El->Ref->Fip  , _GEN_AE_LE_ILLEGAL_MODIFICATION  , 0 );
		return (FDM_NOK);
	}
#endif
	El->definition.Type.MSGa =  F;
	return FDM_OK;
}
Absolute_ADR _SCAN_DTRAME_IN_FIP ( FDM_REF * Ref , int image , Ushort ID  )
{
/*
fonction:
    recherche dichotomique dans la table de descripteur de trame
     de l'adresse du descripteur (En fonction de l'ID)
    retourne l'adresse de l'objet dans la BD FIP
*/
	int j;
	Absolute_ADR SCAN ;
	Ushort  IDs;
	SCAN = Ref->FDM_Internal_Use.fipgen.BASE_0[ image ] +
 		 (1 << ( Ref->FDM_Internal_Use.Frame_Par  + 3 )) ;
	j = 1 << ( Ref->FDM_Internal_Use.Frame_Par  + 3 ) ;
	do {
		j = j >> 1;
		_FIP_READ_WORD (  Ref , SCAN , &IDs ) ;
		if ( IDs == ID  ) return ( SCAN );
		SCAN +=  ( IDs < ID ) ? ( j ) : (- j );
	} while (j >= 0x10 ) ;
	return( 0xffffffffL ); /* pas trouve ! */
}



Ushort fdm_mps_var_change_periods	(
	FDM_MPS_VAR_REF * El ,
		Ulong Refreshment_Period ,
		Ulong Promptness_Period )
{
	Ushort Cr ;
	FDM_REF *Fip ;
	if (El == NULL ) return (FDM_NOK);
	if ( El->Ref->Etat == AE_LE_CONFIG ){          /* En Config */
		El->definition.Refreshment_Period = Refreshment_Period ;
		El->definition.Promptness_Period  = Promptness_Period ;
		return FDM_OK;
	}
	Fip = El->Ref->Fip ;
	Cr = 0;
	if ( El->Ref->Etat == AE_LE_RUNNING){          /* En run */
		Ushort Ur,Up;
		DESC_TRAME_TYPE *DT;
		Absolute_ADR ADR ;
		Ur =  Convert_User_time ( Fip ,  Refreshment_Period );
		if ( Ur == 0xffff)  return (FDM_NOK);
		Up =  Convert_User_time ( Fip ,  Promptness_Period );
		if ( Up == 0xffff)  return (FDM_NOK);
			OS_fdm_sm_p_bd(Fip) ;
		DT = (DESC_TRAME_TYPE *) _DICHO_SCAN_OBJECT(
			&Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE ,
			(Ulong) El->definition.ID  );
		if (( El->definition.Type.Communication & 1) != 0 ) {
		/* var Produite */
			DT->Production_Period = Ur ;
			if (( DT->position & _position_I1 ) != 0 ) {
				ADR = _SCAN_DTRAME_IN_FIP ( Fip , 0 , El->definition.ID ) ;
				if ( ADR == 0xffffffffL) goto abort;
				Cr |= _FIP_WRITE_DESCRIPTEUR ( Fip , ADR , &DT->Identifier  );
			}
			if (( DT->position & _position_I2 ) != 0 ) {
				ADR = _SCAN_DTRAME_IN_FIP ( Fip , 1 , El->definition.ID ) ;
				if ( ADR == 0xffffffffL) goto abort;
				Cr |= _FIP_WRITE_DESCRIPTEUR ( Fip , ADR , &DT->Identifier  );
			}
			if ( Cr  != 0 ) goto abort_pas_grave;
		}
		if (( El->definition.Type.Communication & 2) != 0 ) {
		/* var Consomme */
			Ushort  TYPE_LENGTH ;
                        Absolute_ADR Ad  ;
			TYPE_LENGTH  =
				El->definition.Var_Length | TypeLength_C;
			if ( Up != 0 ) TYPE_LENGTH |= TypeLength_Promptness ;
			Ad = Fip->FDM_Internal_Use.fipgen.BASE_1 + DT->Var_Desc_Ptr_Consommee  ;
 			Cr = _FIP_WRITE_WORD  ( Fip , Ad        , TYPE_LENGTH  ) ;
 			Cr |=_FIP_WRITE_WORD  ( Fip , Ad  + 10  , Up  ) ;
			if ( Cr  != 0 ) goto abort_pas_grave;
		}

	El->definition.Refreshment_Period = Refreshment_Period ;
	El->definition.Promptness_Period  = Promptness_Period ;
	OS_fdm_sm_v_bd(Fip) ;
	return FDM_OK;
	}
abort :
	FDM_Signal_Error ( Fip  , _INTEGRITY_DATA_BASE_FAULT , 0 );
	OS_fdm_sm_v_bd(Fip) ;
	return (FDM_NOK);

abort_pas_grave:
	OS_fdm_sm_v_bd(Fip) ;
	return (FDM_NOK);
}

/********
 Change la nature d'une variable **
 0 = cons ; 1 = pro
**********/
Ushort fdm_mps_var_change_prod_cons ( FDM_MPS_VAR_REF *El ,
	 enum _FDM_CHANGE_TYPE  _Image_1_Var_type ,
	 enum _FDM_CHANGE_TYPE  _Image_2_Var_type )
{
	Ushort Cr ;
	FDM_REF *Fip ;
	if (El == NULL ) return (FDM_NOK);
	if ( El->Ref->Etat == AE_LE_CONFIG ){          /* En Config */
		FDM_Signal_Warning ( El->Ref->Fip  , _GEN_AE_LE_ILLEGAL_MODIFICATION  , 0);
		return FDM_NOK;
	}
	Fip = El->Ref->Fip ;
	Cr = 0;
	if ( El->Ref->Etat == AE_LE_RUNNING ){          /* En run */
		DESC_TRAME_TYPE *DT;
		Absolute_ADR ADR ;
			OS_fdm_sm_p_bd(Fip) ; /*<<*/
		DT = (DESC_TRAME_TYPE *) _DICHO_SCAN_OBJECT(
			&Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE ,
			(Ulong) El->definition.ID  );
		if ( El->definition.Type.Communication != 3 ) goto abort;
		DT->position &= ~(_position_Ip1 | _position_Ip2);
		DT->position |=
			( _Image_1_Var_type == PRODUCED ) ? _position_Ip1 : 0;
		DT->position |=
			( _Image_2_Var_type == PRODUCED ) ? _position_Ip2 : 0;
		if ( ( DT->position & _position_I1 ) !=0 ) {
			   ADR = _SCAN_DTRAME_IN_FIP ( Fip , 0 , El->definition.ID ) ;
			   if ( ADR == 0xffffffffL) goto abort1;
			   if ((DT->position & _position_Ip1) != 0 ) {
				DT->Var_Desc_Ptr   = DT->Var_Desc_Ptr_Produite;
				DT->reserved_FIPCODE = DT->Var_Desc_Ptr;
				}
			   else {
				DT->Var_Desc_Ptr   = DT->Var_Desc_Ptr_Consommee;
				DT->reserved_FIPCODE = 0 ;
				}
			   Cr |= _FIP_WRITE_WORD ( Fip , ADR + 2 ,  DT->Var_Desc_Ptr );
			   Cr |= _FIP_WRITE_WORD ( Fip , ADR + 5 ,  DT->reserved_FIPCODE );
		}
		if ( ( DT->position & _position_I2 ) !=0 ) {
			   ADR = _SCAN_DTRAME_IN_FIP ( Fip , 1 , El->definition.ID ) ;
			   if ( ADR == 0xffffffffL) goto abort1;
			   if ((DT->position & _position_Ip2) != 0 ) {
				DT->Var_Desc_Ptr   = DT->Var_Desc_Ptr_Produite;
				DT->reserved_FIPCODE = DT->Var_Desc_Ptr;
				}
			   else {
				DT->Var_Desc_Ptr   = DT->Var_Desc_Ptr_Consommee;
				DT->reserved_FIPCODE = 0 ;
				}
			   Cr |= _FIP_WRITE_WORD ( Fip , ADR + 2 , DT->Var_Desc_Ptr   );
			   Cr |= _FIP_WRITE_WORD ( Fip , ADR + 5 , DT->reserved_FIPCODE );
		}
		OS_fdm_sm_v_bd(Fip) ; /*<<*/
		if (Cr !=0 ) return FDM_NOK;
		else         return FDM_OK ;
	}
abort:
			OS_fdm_sm_v_bd(Fip) ; /*<<*/
	FDM_Signal_Warning ( El->Ref->Fip  , _GEN_AE_LE_ILLEGAL_MODIFICATION  , 0 );
	return FDM_NOK;
abort1:
			OS_fdm_sm_v_bd(Fip) ; /*<<*/
	FDM_Signal_Error ( Fip  , _INTEGRITY_DATA_BASE_FAULT , 0 );
	return FDM_NOK;

}

Ushort fdm_mps_var_change_priority   ( FDM_MPS_VAR_REF * El , unsigned Prio )
{
	if (El == NULL ) return (FDM_NOK);
	El->definition.Type.Priority = Prio ;
	return FDM_OK;
}

Ushort fdm_switch_image ( FDM_REF * Ref ,  enum  IMAGE_NR image )
{
	if ( ( Ref->S.Type & TWO_IMAGE_MODE ) == 0 ) {
		FDM_Signal_Warning ( Ref  , _USER_SWITCH_FAULT  , 0 );
	 	return (FDM_NOK);
	}
	OS_fdm_sm_p_t(Ref);
	Ref->FDM_Internal_Use.fipgen.image = image ;
	FDM_Change_Base_0 (Ref);
	OS_fdm_sm_v_t(Ref);
	return  FDM_OK ;
}

/*********************************************************
     ___________________________________
Cr =|........ |CNF|..|AP|..|..|VS|..|AR|
     -----------------------------------
CNF : CoNFirmation
AP  : Asynchronous Promptness
VS  : Variable Signifiance
AR  : Asynchronous Refreshment
Cr = FFFF si operation impossible
Cr = 0 si Ok
********************************************************/
FDM_MPS_READ_STATUS fdm_mps_var_read_loc (FDM_MPS_VAR_REF  *EL ,FDM_MPS_VAR_DATA *FBuffer )
{
	Ushort Cr_final;
	FDM_MPS_READ_STATUS User_CR = { _FDM_MPS_USER_ERROR  , 1,1,1,1 };
	if (EL == NULL ) return ( User_CR );
	if (EL->Ref == NULL ) return ( User_CR );
	if ( EL->Ref->Etat != AE_LE_RUNNING){	/* !running */
		FDM_Signal_Warning ( EL->Ref->Fip  , _GEN_AE_LE_ILLEGAL_STATE  , 0 );
		return ( User_CR );
	}
#if	( FDM_WITH_CONTROLS == YES )
	if ( EL->cle_Con == 0 ) {	/* ! consommable */
		FDM_Signal_Warning ( EL->Ref->Fip  , _NON_CONSUMING_VARIABLE , 0 );
		return ( User_CR );
	}
#endif
 	Cr_final  = _FIP_READ_VAR( EL->Ref->Fip ,  EL->cle_Con  , (Uchar * )FBuffer) ;
	if (( Cr_final  & 0x80 ) != 0){
		if (( Cr_final  & 0xf ) != 0){
			FDM_Signal_Warning ( EL->Ref->Fip ,  _FIPCODE_RESPONSE_IMPOSSIBLE , 0 );
			return ( User_CR );
		}
	}
	User_CR.Report =  _FDM_MPS_STATUS_FAULT;
	if (( Cr_final  & 0x80 ) != 0)	return ( User_CR );
	User_CR.Non_Significant = 0;
	if (( Cr_final  & 0x20 ) != 0)	User_CR.Promptness_false = 0;
	if ( EL->definition.Type.Refreshment ) {
          	 /* read status */
		Cr_final = (Ushort ) FBuffer->FBuffer[ FBuffer->Pdu_Length - 1 ] ;
		if (( Cr_final  & 0x4 ) == 0)	return ( User_CR );
		User_CR.Signifiance_status_false = 0;
		if ( ( Cr_final  & 0x1 ) == 1) {
			User_CR.Refresment_false = 0 ;
		}
	} else {
		User_CR.Signifiance_status_false = 0;
		User_CR.Refresment_false = 0 ;
	}
	if (( User_CR.Signifiance_status_false
		| User_CR.Refresment_false
		| User_CR.Promptness_false   ) == 0 ) User_CR.Report =  _FDM_MPS_READ_OK ;
	return ( User_CR );
}
FDM_MPS_READ_STATUS fdm_mps_var_time_read_loc (FDM_MPS_VAR_REF  *EL ,
		void (*User_Give_Value)(FDM_MPS_VAR_TIME_DATA* ),
		FDM_MPS_VAR_TIME_DATA *FBuffer )
{
	FDM_MPS_READ_STATUS User_CR = { _FDM_MPS_USER_ERROR  , 1,1,1,1 };
	if (EL == NULL ) return ( User_CR );
	if ( EL->Ref->Etat != AE_LE_RUNNING){	/* !running */
		FDM_Signal_Warning ( EL->Ref->Fip  , _GEN_AE_LE_ILLEGAL_STATE  , 0 );
		return ( User_CR );
	}
#if	( FDM_WITH_CONTROLS == YES )
	if ( EL->cle_Con == 0 ) {	/* ! consommable */
		FDM_Signal_Warning ( EL->Ref->Fip  , _NON_CONSUMING_VARIABLE , 0 );
		return ( User_CR );
	}
#endif
 	User_CR  = _FIP_READ_VAR_TIME ( EL->Ref->Fip ,  EL->cle_Con  ,  User_Give_Value , FBuffer) ;
	return ( User_CR );
}

/********************************************************
Cr = FFFF si operation impossible
*********************************************************/
Ushort fdm_mps_var_write_loc (FDM_MPS_VAR_REF  *EL , USER_BUFFER_TO_READ	FBuffer )
{
	Ushort CR ;
	if (EL == NULL ) return (FDM_NOK);
	if (EL->Ref == NULL ) return (FDM_NOK);

	if ( EL->Ref->Etat != AE_LE_RUNNING ){	/* !running */
		FDM_Signal_Warning ( EL->Ref->Fip  , _GEN_AE_LE_ILLEGAL_STATE  , 0 );
		return (FDM_NOK);
	}
#if	( FDM_WITH_CONTROLS == YES )
	if ( EL->cle_Pro == 0 ) {	/* ! productible */
		FDM_Signal_Warning ( EL->Ref->Fip  , _NON_PRODUCING_VARIABLE , 0 );
		return (FDM_NOK);
	}
#endif
 	CR = _FIP_WRITE_VAR( EL->Ref->Fip ,  EL->cle_Pro  , FBuffer) ;
	if ( CR != 0) return (FDM_NOK);
	return ( FDM_OK);
}
Ushort fdm_mps_var_time_write_loc (FDM_MPS_VAR_REF  *EL ,
		Uchar * (*User_Get_Value)(void),
		const Ulong *Delta )
{
	FDM_ERROR_CODE CR ;
	if (EL == NULL ) return (FDM_NOK);

	if ( EL->Ref->Etat != AE_LE_RUNNING ){	/* !running */
		FDM_Signal_Warning ( EL->Ref->Fip  , _GEN_AE_LE_ILLEGAL_STATE  , 0 );
		return (FDM_NOK);
	}
#if	( FDM_WITH_CONTROLS == YES )
	if ( EL->cle_Pro == 0 ) {	/* ! productible */
		FDM_Signal_Warning ( EL->Ref->Fip  , _NON_PRODUCING_VARIABLE , 0 );
		return (FDM_NOK);
	}
	if ( EL->definition.Type.With_Time_Var == 0 ) {	/* ! time */
		FDM_Signal_Warning ( EL->Ref->Fip  , _NON_TIME_VARIABLE, 0 );
		return (FDM_NOK);
	}
#endif
 	CR.Information.Additional_Report = _FIP_WRITE_VAR_TIME (
		EL->Ref->Fip ,
		EL->cle_Pro  ,
		User_Get_Value ,
		Delta
		) ;
	if (( CR.Information.Fipcode_Report._Var_State & 0x80 ) != 0){
		if ( (  CR.Information.Fipcode_Report._Var_State & 0xf ) != 1 ) {
			FDM_Signal_Warning ( EL->Ref->Fip , _FIPCODE_RESPONSE_IMPOSSIBLE ,0 );
			return (FDM_NOK);
		}
	}
	return ( FDM_OK);
}

#if 				( FDM_WITH_APER == YES )
/********************************************************
Cr = FFFF si operation impossible
*********************************************************/
Ushort fdm_mps_var_write_universal (FDM_MPS_VAR_REF  *EL , USER_BUFFER_TO_READ  FBuffer )
{
	FDM_ERROR_CODE CR ;
	FDM_REF *Fip ;
	Ushort Cr ;
	if (EL == NULL ) return (FDM_NOK);

	Fip = EL->Ref->Fip;
	if ( EL->Ref->Etat != AE_LE_RUNNING){	/* !running */
		FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_STATE , 0 );
		return (FDM_NOK);
	}

#if	( FDM_WITH_CONTROLS == YES )
	
	if ( EL->cle_Pro == 0 ) {	/* ! productible */
		FDM_Signal_Warning ( Fip  , _NON_PRODUCING_VARIABLE , 0 );
		return (FDM_NOK);
	}

	if ( EL->definition.Signals.User_Signal_Var_Prod == NULL ) {
		FDM_Signal_Warning ( Fip  , _INDICATION_PROG_MISSING , 0);
		return (FDM_NOK);
	}

#endif
	
	if (( Fip->FDM_Internal_Use.fipgen.image == IMAGE_2 ) &&
	    ( EL->definition.Type.Scope == 0) ){
		FDM_Signal_Warning ( Fip  , _IMAGE_FAULT_ON_REQUEST  , 0);
		return (FDM_NOK);
	}
	if ( EL->Compte_rendu == 1 ) {
		FDM_Signal_Warning ( Fip  , _REQUEST_RAN_ALWAYS , 0);
		return (FDM_NOK);
	}
	if ( EL->definition.Type.Indication ) {
		FDM_Signal_Warning ( Fip  , _INDICATION_VARIABLE_IN_USE , 0);
		return (FDM_NOK);
	}


	
	Fip->FDM_Internal_Use.fipgen.Xrf->Var[ EL->cle_Pro ] = EL ;
	/** Controles d'entree ok **/
 	CR.Information.Additional_Report = _FIP_WRITE_VAR( Fip ,  EL->cle_Pro  , FBuffer) ;
	if (( CR.Information.Fipcode_Report._Var_State & 0x80 ) != 0){
		if ( ( CR.Information.Fipcode_Report._Var_State & 0xf ) != 1 ) {
			FDM_Signal_Warning ( EL->Ref->Fip , _FIPCODE_RESPONSE_IMPOSSIBLE ,0 );
			return (FDM_NOK);
		}
	}
	if ( EL->definition.Type.Priority == 0)
		_TIMER_RETRIG ( &Fip->FDM_Internal_Use.T_aper_N.Timer);
	else
		_TIMER_RETRIG ( &Fip->FDM_Internal_Use.T_aper_U.Timer);

	EL->Compte_rendu = 1;
	_TIMER_START ( &EL->Timer) ;
	if ( EL->definition.Type.Scope == 0) {
 		Cr  = _FIP_UPDATE   ( Fip , EL->cle_Pro , (short) EL->definition.Type.Priority );
		if ( Cr != 0) return ( FDM_NOK );
	}
	else {
		Cr  =_FIP_DEF_EVT  ( Fip , EL->cle_Pro ,  Receive_Evt ) ;
		if ( Cr != 0) return ( FDM_NOK );
	}
	return ( FDM_OK);
}


Ushort fdm_mps_var_read_universal ( FDM_MPS_VAR_REF  *EL )
{
	FDM_REF *Fip ;
	Ushort Cr;
	if (EL == NULL ) return (FDM_NOK);

	Fip = EL->Ref->Fip;
	if ( EL->Ref->Etat != AE_LE_RUNNING){	/* !running */
		FDM_Signal_Warning ( Fip  , _GEN_AE_LE_ILLEGAL_STATE  , 0);
		return (FDM_NOK);
	}
#if	( FDM_WITH_CONTROLS == YES )
	if ( EL->cle_Con == 0 ) {	/* ! productible */
		FDM_Signal_Warning ( Fip  , _NON_CONSUMING_VARIABLE , 0 );
		return (FDM_NOK);
	}
	if ( EL->definition.Type.Indication ) {
		FDM_Signal_Warning ( Fip  , _INDICATION_VARIABLE_IN_USE , 0);
		return (FDM_NOK);
	}

	if ( EL->definition.Signals.User_Signal_Var_Cons == NULL ) {
		FDM_Signal_Warning ( Fip  , _INDICATION_PROG_MISSING , 0);
		return (FDM_NOK);
	}

#endif
	if (( Fip->FDM_Internal_Use.fipgen.image == IMAGE_2 ) &&
	    ( EL->definition.Type.Scope == 0) ){
		FDM_Signal_Warning ( Fip  , _IMAGE_FAULT_ON_REQUEST , 0 );
		return (FDM_NOK);
	}
	if ( EL->Compte_rendu == 1 ) {
		FDM_Signal_Warning ( Fip  , _REQUEST_RAN_ALWAYS  , 0);
		return (FDM_NOK);
	}
	/** Controles d'entree ok **/
	Fip->FDM_Internal_Use.fipgen.Xrf->Var[ EL->cle_Con ] = EL ;
	if ( EL->definition.Type.Priority == 0)
		_TIMER_RETRIG ( &Fip->FDM_Internal_Use.T_aper_N.Timer);
	 else
		_TIMER_RETRIG ( &Fip->FDM_Internal_Use.T_aper_U.Timer);
	EL->Compte_rendu = 1;
	_TIMER_START ( &EL->Timer) ;
	if ( EL->definition.Type.Scope == 0) {
 		Cr  = _FIP_UPDATE   ( Fip , EL->cle_Con , (short) EL->definition.Type.Priority );
		if ( Cr != 0) return ( FDM_NOK );
	}
	else {
		Cr  =_FIP_DEF_EVT  ( Fip , EL->cle_Con ,  Receive_Evt ) ;
		if ( Cr != 0) return ( FDM_NOK );
	}
	return FDM_OK;
}
#endif


/*****************************************************/
/******************* MPS Process *********************/
/*****************************************************/

void Appli_MPS ( _FDM_XAE_Tempo_Ref *T )
{
	Ushort U;
	FDM_MPS_VAR_REF *El ;
	El = (FDM_MPS_VAR_REF *) T->Ref_XAE;
	El->Timer._.In_File  = 0;
	if ( El->Timer.T_cur == 0 ) {
		El->Compte_rendu = 4;/* t.out */
		U = FDM_NOK ;
	} else {
		El->Compte_rendu = 2;/* Ok */
		U = 0 ;
	}
	switch ( El->definition.Type.Communication) {
	case 1:
		El->definition.Signals.User_Signal_Var_Prod( El, U );
		break;
	case 2:
		{
		FDM_MPS_VAR_DATA Var;
		FDM_REF *Fip ;
                FDM_MPS_READ_STATUS User_CR = { _FDM_MPS_USER_ERROR  , 1,1,1,1 };
		Fip = El->Ref->Fip;
		if ( U == 0 ) {
 			U = _FIP_READ_VAR( Fip ,  El->cle_Con  , &Var.Pdu_Type) ;
			if (( U  & 0x80 ) == 0)	{
				User_CR.Non_Significant  = 0;
				User_CR.Promptness_false = 0;
				if ( El->definition.Type.Refreshment ) {
   			       	 /* read status */
					U = (Ushort ) Var.FBuffer[ Var.Pdu_Length - 1 ] ;
					User_CR.Signifiance_status_false =
						 (( U  & 0x4 ) == 0)? 1 : 0 ;
                                	User_CR.Refresment_false =
						 (( U  & 0x1 ) == 0)? 1 : 0 ;
					if ((User_CR.Signifiance_status_false | User_CR.Refresment_false ) == 0)
						User_CR.Report = _FDM_MPS_READ_OK ;
				} else  {
					User_CR.Signifiance_status_false = 0;
					User_CR.Refresment_false =0;
					User_CR.Report = _FDM_MPS_READ_OK ;
				}
			} else 	User_CR.Report =  _FDM_MPS_STATUS_FAULT;

		}
		else 	User_CR.Report = _FDM_MPS_STATUS_TO ;

		El->definition.Signals.User_Signal_Var_Cons( El , User_CR , &Var );
		break;
		}
	}
}


void FDM_INITIALIZE_TASK_MPS (
	FDM_REF *Ref,
	void (*USER_FUNCT) ( struct _FDM_REF* )
	)
{
	SOFT_REFERENCE_TYPE 	*Tache_MSG_S_ref ;
    	SOFT_INTERFACE_TYPE 	SIT;
	CREATE_TASK_ENVIRONNEMENT  (
		&Ref->FDM_Internal_Use.Task_Ref_MPS,
		(void (*)(void*)) USER_FUNCT,(void*)Ref
	);
    	SIT.Nr_Of_External_In_Files 	= 1;
    	SIT.Software_Loop_Fct           = SOFT_LOOP ;
	SIT.Array_Of_In_Functions[0].Software_Treatment_Fct =
					( void(*)() ) Appli_MPS ;
	Tache_MSG_S_ref = &Ref->FDM_Internal_Use.Soft_Ref_MPS;

        ATTACH_SOFT_TO_TASK(
			&Ref->FDM_Internal_Use.Task_Ref_MPS ,
			&SIT,
			Tache_MSG_S_ref,
			Ref->FDM_Internal_Use.Files_MPS);
	Ref->FDM_Internal_Use.FIFO_MPS = Tache_MSG_S_ref->Array_Of_In_Files[0 ];

}



/******* fin ******/
