/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD




      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmgenbd.c
  Description     : Generation de la base de donnees Fip


  Date de creation: Fri Mar 15 15:11:35 MET 1996

Version:4.0.1
   	a/- Ref Ra : RA 21
  	    sur fdm_stop_network liberer les ressources crees pour la messagerie reception

  	b/-modif Buffer--> FBuffer
  	   nom Buffer utilise dans <stdio.h> compilateur METAWARE 16bits)

Version:4.1.1
	a/- l'init de variable de rapport casse les 2 1er octets de donnees de
		la variable de presence
	b/- si file ! (BA AMTRQ_Dim % 0x40 ) le recalcul est faux.
	c/- init TIMER-CNT cas xxx_1000 compilateur GNU rale ! switch non
		justifie
Version:4.1.2
	a/-simplification FDM_CONFIGURATION_SOFT-> NB_OF_DIFFERENT_ID_PROG_BA

Version:4.2.1
	a/- Variable de type cons-prod si refresh
	b/- pre init de variables  MPS 	sur cible 16bits ( manque cast sur Ulong)
		ds CRE_DVAR
	c/- Tslot*2
	d/-simplification :suppression des champs Frame_Par et Desc_Par
		remplace par NB_OF_USER_MPS_VARIABLE
	e/- TXCK - interne/externe configurable

Version:4.3.1
	a/- Configuration var param segment
	b/- liste des presents - (Mono-ou Bi )

Version:4.3.4
	a/- Variable d'identification

Version:4.3.5
	a/- Controle dimension Base_0 % Frame_par

Version:4.3.5
	a/- Stop DB mod := complement du dernier
	b/- opt FIPIO

Version:4.3.17
	a/- pb de config de LSAP mono-image sur VCOM bi-image

Version:4.4.4
	a/-Cons message sur LSAP "pure" ( voir Fipcode 6.4)

Version:4.5.1
	a/-calcul tempo en fin de MC a 3125

Version:4.9.5
	a/- SUP_DTRAME

Version:4.9.7
	a/- option de compil FDM_WITH_OPTIMIZED_BA


***********************************************************/


#define __fdmgenbd_c
#include "fdm.h"

/******************* Utilitaires *********************************/
const  char About_fdm[] =
	{"All Rights Reserved. @ 1996-2001 , ALSTOM , Paris (France)  " };

static  const
FDM_VERSION Version = {{ 4 , 9 , 9 } , { 6 , 9 , 10 }};

const FDM_VERSION *fdm_get_version ( void )
{ return &Version ; }

void *
FDM_Malloc (  FDM_REF *Ref , Ulong nombre_d_octet )
{
	void * Pointer;
	OS_Allocate( Ref->H.Memory_Management_Ref ,void*,Pointer,nombre_d_octet);
#if			( FDM_WITH_PSOS == YES )
	if ( Ref->H.Memory_Management_Ref->Cr != 0 )
		FDM_Signal_Warning (  Ref , _RN_FAILED , Ref->H.Memory_Management_Ref->Cr );
#endif
	return Pointer;
}

void
FDM_Free(FDM_REF *Ref,void * segment_adr)
{
	OS_Free(Ref->H.Memory_Management_Ref , segment_adr);
#if			( FDM_WITH_PSOS == YES )
	if ( Ref->H.Memory_Management_Ref->Cr != 0 )
		FDM_Signal_Warning (  Ref , _RN_FAILED ,Ref->H.Memory_Management_Ref->Cr );
#endif
}


/*formatage de type string */
static int
ASNS( unsigned char    ap , unsigned char * cp , char * argument)
{
	int lg;

	*cp++ = ap;
	lg = strlen(argument);
	*cp++ = (char)lg;
	if (lg < 119 )  memcpy(cp,argument,lg);
	return (lg+2) ;

}
/*formatage de type tableau */
static int
ASNT(unsigned char ap ,  unsigned char * cp , char * argument)
{
	int lg ;

	*cp++ = ap;
	lg = *argument++;
	*cp++ = (char)lg;
	if ( lg < 116 )  memcpy(cp,argument,lg);
	memcpy(cp,&argument[0],lg);
	return (lg+2) ;

}


void
FDM_Signal_Error ( FDM_REF *Ref , enum CODE_ERROR  Default_Type , Ulong Add_info )
{
	FDM_ERROR_CODE CR;

	CR.Fdm_Default		  		= Default_Type ;
	CR.Information.Additional_Report	= Add_info ;
	Ref->H.User_Signal_Fatal_Error (Ref, CR );
}

void
FDM_Signal_Warning ( FDM_REF *Ref , enum CODE_ERROR   Default_Type , Ulong Add_info )
{
	FDM_ERROR_CODE CR;

	CR.Fdm_Default		  		= Default_Type ;
	CR.Information.Additional_Report	= Add_info ;
	Ref->H.User_Signal_Warning (Ref, CR );

}




static Ushort
dem_mem( FDM_REF *Ref )
/* dim = 1 ou 2 uniquement */
/* ce SP cherche un emplacement memoire disponible
   il retourne un numero de page libre
	0 si tout est occupe
*/
{
	unsigned int i,j;

	j = Ref->FDM_Internal_Use.fipgen.Dim_Map_V  ;
	for (i = 0 ; i < j ; i++) {
		if ((Ref->FDM_Internal_Use.fipgen.ptr_MAP_v->bit_map[i>>4] & (1<<(i & 0xf))) == 0) {
			/* trouve */
			Ref->FDM_Internal_Use.fipgen.ptr_MAP_v->bit_map[i>>4] =
				(unsigned short)(Ref->FDM_Internal_Use.fipgen.ptr_MAP_v->bit_map[i>>4] | (1<<(i & 0xf)));
			return ( (Ushort)( Ref->FDM_Internal_Use.fipgen.BASE_2/64 + i ) );
		}
	}

return ( 0 );
}

static void
lib_mem( FDM_REF *Ref , Ushort adr  )
/* ce SP libere un emplacement memoire disponible */
{
	int i;

	i = (int)( (adr & 0x7fff) - Ref->FDM_Internal_Use.fipgen.BASE_2/64 );
	Ref->FDM_Internal_Use.fipgen.ptr_MAP_v->bit_map[i>>4] =
	      (unsigned short)(Ref->FDM_Internal_Use.fipgen.ptr_MAP_v->bit_map[i>>4] & ~(1<<(i & 0xf)));
}



#if							(  FDM_WITH_BA == YES )
Ushort
Calcul_tempo_31_25 ( FDM_REF *Ref  )
{
	switch (Ref->S.Mode) {
    	case WORLD_FIP_31   :
    	case FIP_31    :
	{	long L;
		L = Ref->FDM_Internal_Use.Page_2000.BA_Nr_Max +2;
		L = ( L* 4096 ) + 6260 ; /* 6260= id_dat silence; silence = 4096*/
		L /=  Ref->S.TSlot ;
		return ( ( Ushort ) L) ;
	}
	default :
		return 0;
	}
}
#endif

static Ushort
dem_cle( FDM_REF *Ref )
{
/* ce SP cherche un DV libre et l'occupe
retourne la cle du descriteur de variable
*/
	unsigned int i;
	int j;
	Ushort k ;

	for (i = 0 ; i < (unsigned int) Ref->FDM_Internal_Use.Desc_Par ; i++) {
		if (Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] != 0xffff) {
			k = 1;
			for (j = 0 ; j < 16 ; j++) {
				if ((Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] & k) == 0) {
					Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] =
						(unsigned short)(Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] | k);
					return( (Ushort)(i*16) + j );
				}
				k = (unsigned short)(k << 1);
			}
		}
	}
	return 0xffff;
}

#if		( FDM_WITH_MESSAGING == YES)

static Ushort dem_cle_bar( FDM_REF *Ref )
{
/*
ce SP cherche un DV libre (recherche par la fin) et l'occupe
retourne la cle du descriteur de variable
*/
	unsigned short  i,j,k;

	for (i =( unsigned short)(Ref->FDM_Internal_Use.Desc_Par - 1) ; i != 0xffff ;i--) {
		if (Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] != 0xffff) {
			k = 0x8000;
			for (j = 15 ; j != 0xffff ; j--) {
				if ((Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] & k) == 0) {
					Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] =
						(unsigned short)(Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] | k);
					return( (i*16) + j );
				}
				k >>= 1;
			}
		}
	}
	return 0xffff;
}

#endif

static void lib_cle(FDM_REF *Ref, Ushort num)
{
	int i,j;

	i = num/16;
	j = num % 16;
	Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] =
		(unsigned short)(Ref->FDM_Internal_Use.fipgen.ptr_MAP->bit_map[i] & ~(1<<j));
}



Ushort Convert_User_time ( FDM_REF *Ref, Ulong Tempo )
{
	Ulong ll;

	if ( Tempo == 0 ) return 0;

	ll = Tempo/Ref->S.TSlot ;
	if ( Ref->S.TSlot == 62 ) {
		Ulong llc;
		llc = Tempo/63;
		ll -=  ( ll - llc)/2;
	}
	if ((( ll & 0xffffc000L ) != 0 ) || ( ll == 0 ) ){
		FDM_Signal_Warning ( Ref ,  _VAR_PERIODE_FAULT , ll ) ;
		return 0xffff;
	}

	return ( (Ushort ) ll );
}




/* recycle cle et page d'un descripteur de variable "isole" de la base de travaille FIP */
void
SUP_DVAR ( FDM_REF *Ref , DESC_TRAME_TYPE *DT )
{
	Ushort data ;
	Ulong Adr ;
	if (( DT->Var_Desc_Ptr == 0) || ( DT->Var_Desc_Ptr == 0x10)) {
		return ;
	}
	if ( DT->Var_Desc_Ptr_Produite != 0) {
		Adr = Ref->FDM_Internal_Use.fipgen.BASE_1 + DT->Var_Desc_Ptr_Produite  ;
 		_FIP_READ_WORD (  Ref , Adr +3 , &data ) ;
		lib_mem( Ref ,  data  ) ;
#if  (  FDM_WITH_FREE_ACCESS == YES )
 		_FIP_READ_WORD (  Ref , Adr+4, &data ) ;
		lib_mem( Ref ,  data   ) ;
#endif
		lib_cle( Ref, ( Ushort )( DT->Var_Desc_Ptr_Produite  >> 4 ) ) ;
	}
	if ( DT->Var_Desc_Ptr_Consommee != 0) {
		Adr = Ref->FDM_Internal_Use.fipgen.BASE_1 + DT->Var_Desc_Ptr_Consommee  ;
 		_FIP_READ_WORD (  Ref , Adr+3, &data ) ;
		lib_mem( Ref ,  data   ) ;

#if  (  FDM_WITH_FREE_ACCESS == YES )
 		_FIP_READ_WORD (  Ref , Adr+4, &data ) ;
		lib_mem( Ref ,  data   ) ;
#endif
		lib_cle( Ref, ( Ushort )( DT->Var_Desc_Ptr_Consommee  >> 4 ) ) ;
	}}

/* recyclage des cles */
void
DELETE_DTRAME ( FDM_REF *Ref  )
{
	DESC_TRAME_TYPE *DT;

	for (;;) {
		DT = ( DESC_TRAME_TYPE *) GET_FIFO_INT ( &Ref->FDM_Internal_Use.fipgen.Del_encours ) ;
		if ( DT == NULL ) break;
		if ( DT->Var_Desc_Ptr   != Synchro_Id_Desc )
			SUP_DVAR ( Ref , DT );
		FDM_Free( Ref , (char*) DT );
	}
}



/*******************
extrait  les descripteurs de trames de la BD interne
et range dans une fifo tampon
DELETE_DTRAME recyclera les ressources quand
des descripteur de trames seront deconnectes du reseau.
(a la fin de  FDM_STOP_DB_MOD )

*********************/
void
SUP_DTRAME ( FDM_REF *Ref , Ushort  Identifieur )
{
	DESC_TRAME_TYPE *DT;
        Ushort Mask;
	DT = (DESC_TRAME_TYPE *)  _DICHO_SCAN_OBJECT(
		&Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE , Identifieur );
	if ( DT == (DESC_TRAME_TYPE *) 0 ) return;

	if (( DT->position &
		(_position_Var | _position_SAP | _position_SAPx |
		 _position_CH_I1 | _position_CH_I2 |
		_position_MSG_I1 | _position_MSG_I2 ) ) == 0 ) {
		/* on ne raz que les variables ne faisant parie de rien !!! */


		DT = (DESC_TRAME_TYPE*)  _DICHO_SUP_OBJECT( &Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE ,
			(Ulong) Identifieur );
		if ( DT->Var_Desc_Ptr   == Synchro_Id_Desc ) {
			  Ref->FDM_Internal_Use.fipgen.Xrf->Var[0] =  NULL  ;
		} else {
			Ushort cle;
			cle = ( Ushort) (DT->Var_Desc_Ptr >> 4) ;
			Ref->FDM_Internal_Use.fipgen.Xrf->Var[ cle ] = NULL ;
		}
		STORE_FIFO_INT ( &Ref->FDM_Internal_Use.fipgen.Del_encours ,
				( FIFO_INT_ELT_TYPE* ) DT );

	}
	Mask = _position_MSG_I1 | _position_MSG_I2 | _position_SAP;
	if (( DT->position &  Mask ) == Mask ){
		SUP_DVAR ( Ref , DT );
		DT->Var_Desc_Ptr = 0x20 ;
		DT->Var_Desc_Ptr_Consommee = 0;
	}
}




Ushort
CRE_DVAR( FDM_REF * Ref , const FDM_XAE * Var , Ushort PDU , const Uchar * FBuffer)
{
/* creation d'un descripteur de variable
   retourne 0xffff si erreur
   la cle du descripteur si ok;
*/
	Ushort K;
	DESC_VAR_TYPE DV;
	Ushort CR;

	memset ( &DV , 0 , sizeof(DESC_VAR_TYPE));

	K =  dem_cle( Ref );

	if ( K == 0xffff ) {
		FDM_Signal_Warning ( Ref ,  _VARIABLE_DESCRIPTOR_TABLE_OVERFLOW , 0 ) ;
		return K;
	}

	DV.C_Value_Ptr = dem_mem ( Ref  );
	if ( DV.C_Value_Ptr == 0  ) {
		FDM_Signal_Warning ( Ref , _PRIVATE_FULLFIP_RAM_OVERFLOW,  0) ;
		goto abort ;

	}

	DV.Access_Key   = K;
	DV.ID   = Var->ID ;

	switch ( Var->Type.Communication ) {
	case 1 :/* variable produite */
#if  (  FDM_WITH_FREE_ACCESS == YES )

		DV.C_Value_User  = dem_mem ( Ref  );
		if ( DV.C_Value_User  == 0  ) {
			FDM_Signal_Warning ( Ref , _PRIVATE_FULLFIP_RAM_OVERFLOW,  0) ;
			goto abort ;
		}

#endif
		DV.Var_Type     	= Var_Type_P ;
		DV.Type_Length 		= TypeLength_P  | Var->Var_Length ;
		DV.PDU_Type_Length   	=  Var->Var_Length ;

		if ( Var->Refreshment_Period != 0 ) {
			DV.PDU_Type_Length += 1;
			DV.Var_Type     |= Var_Type_SR  ;
		}
		if ( Var->Type.RQa )  	DV.Var_Type |=   Var_Type_RQa ;

		if  ( Var->Type.MSGa )  DV.Var_Type |= Var_Type_MSGa ;

		if ( Var->Type.With_Time_Var )
					DV.Var_Type |=   Var_Type_SRD ;

		if ( FBuffer != NULL ) {
			_FIP_WRITE_PHY_BYTE( Ref , ((Ulong)DV.C_Value_Ptr) * 64 , FBuffer , Var->Var_Length );
			DV.C_Value_Ptr |= 0x8000;
		}
		if ( DV.PDU_Type_Length > 126 ) {
			FDM_Signal_Warning ( Ref ,   _LENGTH_VAR_FAULT  , 0 ) ;
			goto abort;
		}
		DV.PDU_Type_Length |= PDU;
		if ( Var->Type.Indication != 0 ) {
			Ref->FDM_Internal_Use.fipgen.Xrf->Var[K] = (FDM_MPS_VAR_REF*) Var;
			DV.Evenement = Send_Evt_permanent ;
		}

		break;

	case 2 :  /* variable consommee */
#if  (  FDM_WITH_FREE_ACCESS == YES )

		DV.C_Value_User  = dem_mem ( Ref  );
		if ( DV.C_Value_User == 0  ) {
			FDM_Signal_Warning ( Ref , _PRIVATE_FULLFIP_RAM_OVERFLOW,  0) ;
			goto abort ;
		}

#endif
		DV.Var_Type     	= Var_Type_C ;
		DV.Type_Length 		= TypeLength_C | Var->Var_Length;
		DV.PDU_Type_Length   	= Var->Var_Length ;

		DV.PDU_Type_Length = Var->Var_Length + 2;
		if ( Var->Type.Refreshment ) DV.PDU_Type_Length ++;
		if ( DV.PDU_Type_Length > 128 ) {
			FDM_Signal_Warning ( Ref ,  _LENGTH_VAR_FAULT , 0) ;
			goto abort;
		}

		if ( Var->Type.With_Time_Var ) {
			switch ( Ref->FDM_Internal_Use.Page_2000.Timer_Cnt & 3 ) {
			case 0: /** 0.4 Ys a 2.5Mhz **/
				DV.Trans_Delai = ((((Ushort)DV.PDU_Type_Length*8)+49) * 4 ) /10 ;
				break;
			case 1: /** 1 Ys a 1Mhz **/
				DV.Trans_Delai = (((Ushort)DV.PDU_Type_Length*8)+49) * 1 ;
				break;
			case 3: /** 32 Ys a 32.25 Khz **/
				DV.Trans_Delai = (((Ushort)DV.PDU_Type_Length*8)+49) * 32 ;
				break;
			}
		}
		if ( Var->Type.Reserved == 1 ) DV.PDU_Type_Length  = PDU  ;

		else {
			DV.PDU_Type_Length  |= PDU  ;
			DV.Var_Type = Var_Type_VLG;
		}
		if ( Var->Promptness_Period != 0 ) {
			DV.Consumption_Period     = Convert_User_time ( Ref , Var->Promptness_Period );
			DV.Type_Length  |= TypeLength_Promptness    ;
		}
		if ( DV.Consumption_Period     == 0xffff ) goto abort;

		if ( Var->Type.Indication != 0 ) {
			Ref->FDM_Internal_Use.fipgen.Xrf->Var[K] = (FDM_MPS_VAR_REF*) Var ;
			DV.Evenement   = Receive_Evt_permanent;
		}

		break;
	}


	CR = _FIP_WRITE_DESCRIPTEUR ( Ref ,
		Ref->FDM_Internal_Use.fipgen.BASE_1 + ( K * 16) ,
 		(Ushort *) &DV );

	if ( CR == 0 ) return K ;

abort :
	lib_cle ( Ref , K ) ;
	if ( DV.C_Value_Ptr  != 0) lib_mem( Ref , DV.C_Value_Ptr  );
#if  (  FDM_WITH_FREE_ACCESS == YES )
	if ( DV.C_Value_User != 0) lib_mem( Ref , DV.C_Value_User);
#endif
	return 0xffff;
}




int FDM_place_dans_base_x ( FDM_REF * Ref , Ushort V )
{
	int i,k,nb,LgMax;
	_TABLE *x;
	Ushort Mask;
	DESC_TRAME_TYPE *pD;

	x = Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE;
	nb = x->nombre;

	LgMax = (int)Ref->FDM_Internal_Use.Frame_Par ;
	LgMax = 1 << LgMax ;
	LgMax-- ;

      if ( (V & 1)!=0) {
		/* Image 1*/
		Mask = _position_I1 | _position_CH_I1 | _position_MSG_I1 ;
		k = 0 ;
		for (i = 0 ; i < nb ; i++ ) {
			if ( x->k[i].objet_num < 0 ) continue;
			pD = ( DESC_TRAME_TYPE *) x->k[i].ptr;
			if (( pD->position &  Mask ) == 0 ) continue;
			k ++;

		}

		if ( k >= LgMax ) return ((int)FDM_NOK);

	}
      if ( (V & 2) !=0) {

	/**/
		if ( ( Ref->S.Type & TWO_IMAGE_MODE ) != 0 ) {
			Mask = _position_I2 | _position_CH_I2 | _position_MSG_I2 ;

			k = 0 ;
			for (i = 0 ; i < nb ; i++ ) {
				if ( x->k[i].objet_num < 0 ) continue;
				pD = ( DESC_TRAME_TYPE *) x->k[i].ptr;
				if (( pD->position &  Mask) == 0 ) continue;
				k ++;
			}

			if ( k >= LgMax ) return ((int)FDM_NOK);
		} else return FDM_NOK;

	}
	return FDM_OK ;
}


Ushort CRE_DTRAME( FDM_REF * Ref ,
	 FDM_XAE * Var , Ushort PDU ,  const Uchar * FBuffer )
{

DESC_TRAME_TYPE *DT , *DTx  ;
element E  ;
Ushort old_descr_image;
Ushort new_descr_image;

/*********** CONTROLES ************/
	/*********** CONTROLES ************/
		/*********** CONTROLES ************/
			/*********** CONTROLES ************/
				/*********** CONTROLES ************/

DT =(DESC_TRAME_TYPE *) FDM_Malloc ( Ref, sizeof(DESC_TRAME_TYPE));
if ( DT == ( DESC_TRAME_TYPE *) 0 ) {
	FDM_Signal_Warning ( Ref ,   _ALLOCATE_MEMORY_FAULT , 0 ) ;
	return 0xffff;
}

old_descr_image =0;
memset ( DT       , 0 , sizeof ( DESC_TRAME_TYPE ));
DTx = ( DESC_TRAME_TYPE *)  _DICHO_SCAN_OBJECT( &Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE ,
			(Ulong) Var->ID );
if ( DTx != NULL ) {
	if (( DTx->position & _position_Var ) != 0 ) goto erreur ;
	/* Controles compatibilite - si canal msg cree  */
	if (( DTx->position & (_position_CH_I1 | _position_CH_I2)) != 0 ){
		/* Si  canal Msg   alors */
		if ( Var->Type.Communication  != 1 )
			goto erreur ;	/* must be produite ! */
		if ( Var->Type.MSGa  )
		    	goto erreur ; 	/* period et aper ??? */
	}

	*DT  = *DTx  ;

	if ((DT->position & (_position_I1 | _position_CH_I1 |  _position_MSG_I1))!=0)
		old_descr_image |=_position_I1;

	if ((DT->position & (_position_I2 | _position_CH_I2 |  _position_MSG_I2))!=0)
		old_descr_image |=_position_I2;
}

if ( Var->Type.Position  & 1)
	DT->position  |= _position_I1 ;
if ( Var->Type.Position  & 2)
	DT->position  |= _position_I2 ;


new_descr_image =0;
if ((DT->position & (_position_I1 | _position_CH_I1 |  _position_MSG_I1))!=0)
	new_descr_image |=_position_I1;

if ((DT->position & (_position_I2 | _position_CH_I2 |  _position_MSG_I2))!=0)
	new_descr_image |=_position_I2;

if ( FDM_place_dans_base_x(Ref,(Ushort) (old_descr_image ^ new_descr_image) )  == (int) FDM_NOK ) {
	FDM_Signal_Warning ( Ref ,   _FRAME_DESCRIPTOR_TABLE_OVERFLOW  , 0) ;
	goto abort ;
	}

/*********** CREATION ************/
	/*********** CREATION ************/
		/*********** CREATION ************/
			/*********** CREATION ************/
				/*********** CREATION ************/

E.objet_num 	= (Ulong) Var->ID;
E.ptr 		= DT ;

DT->Identifier   	= Var->ID;

switch ( Var->Type.Communication ) {
case 0 :/************ Synchro *************/
	/* variable de synchro eoc */
	if ( Ref->FDM_Internal_Use.fipgen.Xrf->Var[0] == NULL ) {
		Ref->FDM_Internal_Use.fipgen.Xrf->Var[0] =
			(FDM_MPS_VAR_REF*) Var ;
	} else {
		/**** erreur deja une !!!  *****/
		FDM_Signal_Warning ( Ref ,  _SYNCHRO_VAR_ALREADY_EXIST ,0  ) ;
		goto abort ;
	}

	break;
case 1 :/************ Produite *************/
	if ( Var->Refreshment_Period != 0 ) {
		DT->Production_Period    = Convert_User_time ( Ref , Var->Refreshment_Period );
		if ( DT->Production_Period    == 0xffff ) goto abort;
	}
	DT->Var_Desc_Ptr    	  = CRE_DVAR( Ref , Var , PDU , FBuffer) << 4;
	if ( DT->Var_Desc_Ptr     == 0xfff0 ) goto abort;
	DT->reserved_FIPCODE = DT->Var_Desc_Ptr;
	DT->Var_Desc_Ptr_Produite = DT->Var_Desc_Ptr;

	if  ( Var->Type.MSGa ) {
		DT->Msg_Channel  =  Msg_Channel_Aper;
		DT->position 	|=  _position_MSGa;
        }

	break;

case 2 :/************ consomme *************/
	DT->Var_Desc_Ptr    	= CRE_DVAR( Ref , Var , PDU , NULL) << 4;
	if ( DT->Var_Desc_Ptr    == 0xfff0 ) goto abort;
	DT->Var_Desc_Ptr_Consommee = DT->Var_Desc_Ptr;
	DT->position 		|=  _position_Cons;

	break;

case 3 :/*** consomme puis produite *******/
	{
	if ( Var->Refreshment_Period != 0 ) {
		DT->Production_Period    = Convert_User_time ( Ref , Var->Refreshment_Period );
		if ( DT->Production_Period    == 0xffff ) goto abort;
	}
	Var->Type.Communication	= 2;/* Consommee */
	DT->position 		|=  _position_VaD;

	DT->Var_Desc_Ptr    	= CRE_DVAR( Ref , Var , PDU , NULL) << 4;
	if ( DT->Var_Desc_Ptr    == 0xfff0 ) goto abort;
	DT->Var_Desc_Ptr_Consommee = DT->Var_Desc_Ptr;

	Var->Type.Communication	= 1;/* Produite */
	DT->Var_Desc_Ptr_Produite = CRE_DVAR( Ref , Var , PDU , FBuffer) << 4;
	if ( DT->Var_Desc_Ptr_Produite == 0xfff0 ) {
		SUP_DVAR ( Ref , DT);
		goto abort;
		}
	}
	Var->Type.Communication	= 3;/* Consommee */

	break;
}

DT->position 		|= _position_Var;
if ( DTx != NULL ) {
	_DICHO_SUP_OBJECT( &Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE , (long) Var->ID );
	FDM_Free (Ref , (char*)DTx);
}
_DICHO_ADD_OBJECT( &Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE , &E );
return 0 ;

erreur:
	FDM_Signal_Warning ( Ref ,  _CONFLICT_CONFIGURATION , 1 ) ;

abort :
	FDM_Free (Ref , (char*)DT);
	return 0xffff;

}


/*=====================================================================*/


#if		(  FDM_WITH_DIAG == YES )
#define LENGTH_VAR_TEST 126

static Ushort _FIP_CRE_DVAR_TEST_RAM ( FDM_REF * Ref )
{
	Ushort K;
	DESC_VAR_TYPE D;
	Ushort CR;

	memset ( &D , 0 , sizeof(DESC_VAR_TYPE));
	K =  dem_cle( Ref );
	if ( K == 0xffff ) {
		FDM_Signal_Error ( Ref , _VARIABLE_DESCRIPTOR_TABLE_OVERFLOW , 0 ) ;
		return FDM_NOK;
	}
	Ref->FDM_Internal_Use.fipdiag.ram_test_key = K;

	D.C_Value_Ptr = dem_mem ( Ref );
	if ( D.C_Value_Ptr == 0  ) {
		lib_cle ( Ref , K ) ;
		FDM_Signal_Error ( Ref , _VARIABLE_TABLE_OVERFLOW, 0) ;
		return FDM_NOK;
	}
	D.Type_Length  =TypeLength_C | TypeLength_P | LENGTH_VAR_TEST;
	D.Access_Key   = K;
	D.Var_Type     = Var_Type_P | Var_Type_VLG;
	D.PDU_Type_Length  = LENGTH_VAR_TEST  |  0x5000 ;

#if  (  FDM_WITH_FREE_ACCESS == YES )
	D.C_Value_User  = dem_mem ( Ref  );
	if ( D.C_Value_User  == 0  ) {
		FDM_Signal_Error ( Ref , _PRIVATE_FULLFIP_RAM_OVERFLOW,  0) ;
		return FDM_NOK;
	}

#endif
        CR = _FIP_WRITE_DESCRIPTEUR ( Ref , Ref->FDM_Internal_Use.fipgen.BASE_1
				      + ( K * 16) ,(Ushort *) &D );

	return (Ushort)(( CR == 0 ) ? FDM_OK : FDM_NOK );

}


#endif
/*=====================================================================*/


void FDM_Change_Base_0 ( FDM_REF *Ref)
{
	Ref->FDM_Internal_Use.Page_2000.Base_0  =
		(Ushort)((Ref->FDM_Internal_Use.fipgen.BASE_0[ Ref->FDM_Internal_Use.fipgen.image ]>> 10 ) |
					   ((Ref->FDM_Internal_Use.Frame_Par - 1) << 12)) ;
	_FIP_WRITE_PHY (Ref , FIP_CONF_RESEAU  , &Ref->FDM_Internal_Use.Page_2000.Base_0 , 2);

}

void fdm_stop_network( FDM_REF * Fip )
{/* lirere toutes la memoire allouee , reset FIP */

	int i,nb;
	_TABLE *x;

	if ( Fip == NULL ) return ;

 	_TIMER_STOP ( (TEMPO_DEF*) &Fip->FDM_Internal_Use.Medium.Timer_tst.Timer );
	_TIMER_STOP ( (TEMPO_DEF*) &Fip->FDM_Internal_Use.fipdiag.Timer_tst.Timer );
#if			( FDM_WITH_BA == YES ) && ( FDM_WITH_FIPIO == YES )
 	_TIMER_STOP ( &Fip->FDM_Internal_Use.fipio.SM_MPS_Seg_Par.Var[0].Timer);
#endif
	Fip->H.User_Reset_Component (  &Fip->H );

	if( Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE != NULL ) {
		x = Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE;
		nb = x->nombre;
		for (i = 0 ; i < nb ; i++ ) {
			if ( x->k[i].objet_num < 0 ) continue;
			FDM_Free( Fip , (char*)  x->k[i].ptr ) ;
		}
		FDM_Free ( Fip , (char*)Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE );
	}
	if ( Fip->FDM_Internal_Use.fipgen.ptr_MAP != NULL )
		FDM_Free ( Fip , (char*)Fip->FDM_Internal_Use.fipgen.ptr_MAP );
	if ( Fip->FDM_Internal_Use.fipgen.ptr_MAP_v != NULL )
		FDM_Free ( Fip ,(char*) Fip->FDM_Internal_Use.fipgen.ptr_MAP_v );

#if							(  FDM_WITH_BA == YES )
	if ( Fip->FDM_Internal_Use.fipgen.N_PAGE_BA != NULL )
		FDM_Free ( Fip ,(char*)Fip->FDM_Internal_Use.fipgen.N_PAGE_BA );
#endif

	if ( Fip->FDM_Internal_Use.fipgen.Xrf != NULL )
		FDM_Free (Fip ,(char*) Fip->FDM_Internal_Use.fipgen.Xrf ) ;

#if							(  FDM_WITH_MESSAGING == YES )
	{
     	_DICHO_REMOVE ( &(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local)) ;

    	if ( ( Fip->S.Type & MESSAGE_RECEPTION_AUTHORIZED ) != 0 )
		_DELETE_USER(Fip->FDM_Internal_Use.Messagerie.Reveive_pool );

     	}

#endif
#if					( FDM_WITH_SEMAPHORE == YES )
	FDM_ressources_delete ( Fip );
#endif
	FDM_Free ( Fip ,(char*) Fip );
}


int FDM_STOP_DB_MOD ( FDM_REF * Ref )
{
	int i,k,nb,max;
	_TABLE *x;
	DESC_TRAME_TYPE *pD;
	DESC_TRAME_TYPE *pD_Valid;
	Ushort Mask;

	pD_Valid = (DESC_TRAME_TYPE *)0;
	x = Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE;
	nb = x->nombre;
	max =  nb / 2 ;

	/* Image 1*/
	Mask = _position_I1 | _position_CH_I1 | _position_MSG_I1 ;
	k = 1 ;
	for (i = 0 ; i < nb ; i++ ) {
		if ( x->k[i].objet_num < 0 ) continue;
		pD = ( DESC_TRAME_TYPE *) x->k[i].ptr;
		if (( pD->position & Mask  ) == 0 ) continue;

		pD_Valid = pD;
 	        if (pD_Valid == (DESC_TRAME_TYPE *)0){
  	            FDM_Signal_Error( Ref , _ILLEGAL_POINTER  , 10 );
  	            return -1;
	        }

		if (( pD->position & _position_VaD ) != 0 ) {
			if ((pD->position & _position_Ip1) != 0 ) {
				pD->Var_Desc_Ptr   = pD->Var_Desc_Ptr_Produite;
			} else {
				pD->Var_Desc_Ptr   = pD->Var_Desc_Ptr_Consommee;
			}
		}
	    	if (( pD->position & _position_MSG_I1) != 0 ){
	            pD->Type_Descriptor  =  MSGr_autorisee;
		} else {
	            pD->Type_Descriptor  =  0;
		}

		_FIP_WRITE_DESCRIPTEUR (Ref ,
			Ref->FDM_Internal_Use.fipgen.BASE_0[2]+(k*16) ,
			(Ushort * ) &pD->Identifier );
		k ++;

	}


	for ( i = k  ; i < max ; i++ ) {
		_FIP_WRITE_DESCRIPTEUR (Ref ,
			Ref->FDM_Internal_Use.fipgen.BASE_0[2]+(k*16) ,
			(Ushort * ) &pD_Valid->Identifier );
		k ++;
	}

	/**/
		OS_Enter_Region();
	Ref->FDM_Internal_Use.fipgen.BASE_0[0] ^= Ref->FDM_Internal_Use.fipgen.BASE_0[2];
	Ref->FDM_Internal_Use.fipgen.BASE_0[2]  =
		Ref->FDM_Internal_Use.fipgen.BASE_0[0] ^ Ref->FDM_Internal_Use.fipgen.BASE_0[2];
	Ref->FDM_Internal_Use.fipgen.BASE_0[0] ^= Ref->FDM_Internal_Use.fipgen.BASE_0[2];
		OS_Leave_Region();

	OS_fdm_sm_p_t(Ref);
	if ( Ref->FDM_Internal_Use.fipgen.image == 0 ) FDM_Change_Base_0 ( Ref );
	OS_fdm_sm_v_t(Ref);

	/**/
	if ( ( Ref->S.Type & TWO_IMAGE_MODE ) != 0 ) {
		Mask = _position_I2 | _position_CH_I2 | _position_MSG_I2 ;

		k = 1 ;
		for (i = 0 ; i < nb ; i++ ) {
			if ( x->k[i].objet_num < 0 ) continue;
			pD = ( DESC_TRAME_TYPE *) x->k[i].ptr;
			if (( pD->position & Mask ) == 0 ) continue;

			pD_Valid = pD;
	 	        if (pD_Valid == (DESC_TRAME_TYPE *)0){
	  	            FDM_Signal_Error( Ref , _ILLEGAL_POINTER  , 10 );
	  	            return -1;
		        }

			if (( pD->position & _position_VaD ) != 0 ) {
				if ((pD->position & _position_Ip2) != 0 ) {
					pD->Var_Desc_Ptr   = pD->Var_Desc_Ptr_Produite;
				} else {
					pD->Var_Desc_Ptr   = pD->Var_Desc_Ptr_Consommee;
				}
			}
		    	if (( pD->position & _position_MSG_I2) != 0 ){
		            pD->Type_Descriptor  =  MSGr_autorisee;
			} else {
		            pD->Type_Descriptor  =  0;
			}

			_FIP_WRITE_DESCRIPTEUR (Ref ,
				Ref->FDM_Internal_Use.fipgen.BASE_0[2]+(k*16) ,
				 (Ushort * ) &pD->Identifier );
			k ++;
		}


		for ( i = k  ; i < max ; i++ ) {
			_FIP_WRITE_DESCRIPTEUR (Ref ,
				Ref->FDM_Internal_Use.fipgen.BASE_0[2]+(k*16) ,
				(Ushort * ) &pD_Valid->Identifier );
			k ++;
		}

			OS_Enter_Region();
		Ref->FDM_Internal_Use.fipgen.BASE_0[1] ^= Ref->FDM_Internal_Use.fipgen.BASE_0[2];
		Ref->FDM_Internal_Use.fipgen.BASE_0[2]  =
			Ref->FDM_Internal_Use.fipgen.BASE_0[1] ^ Ref->FDM_Internal_Use.fipgen.BASE_0[2];
		Ref->FDM_Internal_Use.fipgen.BASE_0[1] ^= Ref->FDM_Internal_Use.fipgen.BASE_0[2];
			OS_Leave_Region();

		OS_fdm_sm_p_t(Ref);
		if ( Ref->FDM_Internal_Use.fipgen.image == 1 ) FDM_Change_Base_0 ( Ref );
		OS_fdm_sm_v_t(Ref);

	}

	DELETE_DTRAME ( Ref  ) ;

	return 0;
}









static const Uchar Report_Variable_init[] =
	{0x50,0,0,0x51,0,0,0x52,0,0,0x53,0,0,0x54,0,0} ;


/* AL_LE MS_MPS */
/****************
Liste des cle predefinies :
	3 variable test des presents
	4 variable de presence       ( id 14xx )
	5 variable liste des presents( id 9002 ) consomme
	6 ......... idem ....................... produite (fonction test_p du ba)
 	7 variable de rapport        ( id 11xx )
	8 variable d'identification  ( id 10xx )
	9 variable Synchro ba ( id 9003 ) consomme
	10 ......... idem .................produite

*****************/

static const FDM_XAE	SM_MPS[5] =
{
/* r p c l t r e p r m     i         c      p    r     P       */
/* e o o o i a v r q s     d         d      e    a     r       */
/* s s m c m f e i a g     e         o      r    n     o       */
/* e i m a   e f n o       n                i    g     c       */

#if	( FDM_WITH_BA == YES ) & ( FDM_WITH_REDONDANCY_MGNT == YES )
{ {0,3,2,0,0,0,0,0,1,1},0x1400,    0x05,    0,0, 0,
{
  (void (*) ( struct _FDM_REF *)) 0 ,
  _FIP_SMAP_PRESENCE_MOI ,
  _FIP_SMAP_PRESENCE_AUTRES ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
},
#else
{ {0,3,2,0,0,0,0,0,1,1},0x1400,    0x05,    0,0, 0,
{
  (void (*) ( struct _FDM_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
},
#endif
{ {0,3,1,0,0,0,0,0,0,0},0x1000,    0x00,    0,0, 0,
{
  (void (*) ( struct _FDM_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
},
{ {1,3,3,0,0,0,0,0,0,0},0x9002,    0x44,    0,0, 0,
{
  (void (*) ( struct _FDM_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
},
{ {0,3,1,0,0,0,0,0,0,0},0x1100,    0x0f,    0,0, 0,
{
  (void (*) ( struct _FDM_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
},
{ {0,3,3,0,0,0,0,0,0,0},0x9003,    0x04,    0,0, 0,
{
  (void (*) ( struct _FDM_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
}
};

#if			( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
static const FDM_XAE	SM_MPS_SEG_PARAM =
{ {0,3,3,0,0,0,0,0,0,0},0x9008,    0x06,    0,0, 0,
{
  (void (*) ( struct _FDM_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *)) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,Ushort  )) 0 ,
  (void (*) ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  , FDM_MPS_VAR_DATA *)) 0
},
};
#endif

#if			( FDM_WITH_BA == YES )
static const Ushort SP_TSTP[64] = {
	BA_SEND_ID_DAT , ID_BOURRAGE, 0x0,
	BA_SEND_ID_DAT , 0x1400     , 0x80,
	BA_SEND_ID_DAT , 0x9002     , 0x30,
	BA_SUSPEND , 0 , EVT_TESTP_FIN,
	0,0,0,0,
	0x1400 , 0 ,( CLE_PRESENCE_C * 0x10 ) , 0,0,0,0,0,0,0,0,0,0,0,0,0,
	0x9002 , 0 ,( CLE_LIST_PRE_P * 0x10 ) , 0,0,0,0,0,0,0,0,0,0,0,0,0,
	0x9002 , 0 ,0x00, 0,0,0,0,0,0,0,0,0,0,0,0,0

 };
#endif
static int TAb_TSLOT[4][4] =
{
	{50,200,500,2000},
	{62,250,625,2500},
	{0,0,0,0},
	{100,400,1000,4000},
};
FDM_REF *fdm_initialize_network (
	const FDM_CONFIGURATION_SOFT *  U_soft,
	      FDM_CONFIGURATION_HARD *  U_hard,
	const FDM_IDENTIFICATION     *  U_ident )
{

	Ushort CR;
	Uchar Local_Buffer[256];
	FDM_REF *P;

	if ( U_hard->User_Reset_Component    ==
		(void (*)(struct _FDM_CONFIGURATION_HARD *))0 ) return NULL;
	if ( U_hard->User_Signal_Fatal_Error ==
		(void (*)(struct _FDM_REF *Ref, FDM_ERROR_CODE ))0 ) return NULL;
	if ( U_hard->User_Signal_Warning     ==
		(void (*)(struct _FDM_REF *Ref, FDM_ERROR_CODE ))0 ) return NULL;

	OS_Allocate( U_hard->Memory_Management_Ref , FDM_REF*, P , sizeof(FDM_REF) );

#if			( FDM_WITH_PSOS == YES )
	if ( U_hard->Memory_Management_Ref->Cr != 0 ) {
		FDM_ERROR_CODE CR;
		CR.Fdm_Default		  		= _RN_FAILED ;
		CR.Information.Additional_Report	=  U_hard->Memory_Management_Ref->Cr ;
		U_hard->User_Signal_Fatal_Error ( NULL, CR ) ;
		goto abort;
	}
#else

	if ( P == (FDM_REF *) 0 ) {
		FDM_ERROR_CODE CR;
		CR.Fdm_Default		  		= _ALLOCATE_MEMORY_FAULT ;
		CR.Information.Additional_Report	=  0 ;
		U_hard->User_Signal_Fatal_Error ( NULL , CR ) ;
		goto abort;
	}
#endif
	memset ((char*)P , 0 , sizeof(FDM_REF));

	P->H = *U_hard;

	P->S = *U_soft;

 	U_hard->Ptr_Autotests = P ;

	P->H.User_Reset_Component ( &P->H  );

#if					( FDM_WITH_SEMAPHORE == YES )
	if ( FDM_ressources_create( P ) != FDM_OK ) goto abort;
#endif

/*+++++++++++++++++++++     Fipdiag   ++++++++++++++++++++++++++++++++++*/
        if ( P->S.Type & TEST_RAM_STARTUP){
	    CR= _FIP_RAM_TEST ( P );
            if (CR !=0 ) goto abort;
         }
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
		printf("avant _FIP_RAZ_RAM\n");
 	CR = _FIP_RAZ_RAM ( P );
	if ( CR  != 0 ) {
		printf("_FIP_RAZ_RAM\n");
		 goto abort1;
	}

		printf("avant _FIP_DOWNLOAD_PRG\n");

	CR = _FIP_DOWNLOAD_PRG( P );
	if ( CR  != 0 ) 
	{
		printf("_FIP_DOWNLOAD_PRG\n");
		goto abort1;
	}

		printf("avant _FIP_TEST_PRG\n");
/*+++++++++++++++++++++     Fipdiag   +++++++++++++++++++++++++++++++++++*/
	/* si la ram n' a pas ete testee ,verif que fipcode en ram est ok*/

        if (( P->S.Type & TEST_RAM_STARTUP) ==0 ){
	    CR= _FIP_TEST_PRG ( P );
            if (CR !=0 ) goto abort;
         }

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


	/*******************************/
	/* initialisation de la BD Fip */
	/*******************************/
	{ /* init table d'allocation descripteur de variable */
	int i,j;

	P->FDM_Internal_Use.Desc_Par = P->S.NB_OF_USER_MPS_VARIABLE ;
	P->FDM_Internal_Use.Desc_Par += 13 ; /* gestion */
#if		( FDM_WITH_MESSAGING == YES)

    	if ( ( P->S.Type & MESSAGE_RECEPTION_AUTHORIZED ) != 0 )
		P->FDM_Internal_Use.Desc_Par += 64;
	{
		int num_canal;
		for ( num_canal = 0 ; num_canal < 9; num_canal++){
	       		P->FDM_Internal_Use.Desc_Par +=
				P->S.Nr_Of_Tx_Buffer[num_canal] ;
		}
	}

#endif
	if (( P->FDM_Internal_Use.Desc_Par % 16 ) != 0 )
		P->FDM_Internal_Use.Desc_Par += 16 ;

	P->FDM_Internal_Use.Desc_Par /= 16 ;

	i =  P->FDM_Internal_Use.Desc_Par<<4  ;
	if ( i > 4096 ) {
		FDM_Signal_Error( P , _USER_NB_OF_USER_MPS_VARIABLE  , 0 );
		goto abort;
		}

	P->FDM_Internal_Use.Page_2000.Last_Var_Time = (Ushort) i ;
	j =   i /8  ;


    	P->FDM_Internal_Use.fipgen.ptr_MAP = (BM *) FDM_Malloc ( P , j );
	if (P->FDM_Internal_Use.fipgen.ptr_MAP == (BM *)0 ) /*NULLPTR*/ {
		FDM_Signal_Error( P ,  _ALLOCATE_MEMORY_FAULT , 0 ) ;
		goto abort;
		}

	memset(P->FDM_Internal_Use.fipgen.ptr_MAP , 0 , j   );

	}

	{/***/
	switch ( P->S.Mode ){
#if ( FDM_WITH_FIPCODE_31_25 == YES )
	case  	WORLD_FIP_31:
	case  	FIP_31:
		P->FDM_Internal_Use.Medium.duree_invalidation  =  50  ;
		break ;
#endif
#if ( FDM_WITH_FIPCODE_1000 == YES )

	case	WORLD_FIP_1000:
	case	FAST_FIP_1000:
	case 	SLOW_FIP_1000:
		P->FDM_Internal_Use.Medium.duree_invalidation  =  120  ;
		break ;

	case	FIP_2500:
	case	WORLD_FIP_2500:
		P->FDM_Internal_Use.Medium.duree_invalidation  =  300 ;
		break;
#endif
#if ( FDM_WITH_FIPCODE_5000 == YES )

	case	FIP_5000:
	case	WORLD_FIP_5000:
		P->FDM_Internal_Use.Medium.duree_invalidation  =  600 ;
		break;
#endif
	case FDM_OTHER:
		P->FDM_Internal_Use.Medium.duree_invalidation = 300 ;
		break;

	default :
		FDM_Signal_Error( P ,  _USER_MODE_FAULT  , 0 ) ;
		goto abort;
	}

	}/**/

	{ /*********** Config CONFIG SYSTEM **************/
	switch ( P->S.Mode ){

	case  	WORLD_FIP_31:
	case	WORLD_FIP_1000:
	case	WORLD_FIP_2500:
	case	WORLD_FIP_5000:
		P->FDM_Internal_Use.Page_2000.Extend_Par  =  TYPE_WORLDFIP  ;
		break ;

	case  	FIP_31:
	case	FAST_FIP_1000:
	case	FIP_2500:
	case	FIP_5000:
		P->FDM_Internal_Use.Page_2000.Extend_Par  =  TYPE_FIP  ;
		break ;

	case 	SLOW_FIP_1000:
		P->FDM_Internal_Use.Page_2000.Extend_Par  =  TYPE_SLOWFIP ;
		break;

	case 	FDM_OTHER:
/*    	case 	FDM_OTHER_1000 :
    	case 	FDM_OTHER_2500 :
    	case 	FDM_OTHER_5000 :*/

		P->FDM_Internal_Use.Page_2000.Extend_Par  = P->S.User_responsability.MODE_REGISTER  ;
		break;

	default :
		FDM_Signal_Error( P ,  _USER_MODE_FAULT  , 0 ) ;
		goto abort;
	}


 	/* init timer CNT */

	switch (P->S.Mode) {
#if ( FDM_WITH_FIPCODE_31_25 == YES )
    	case WORLD_FIP_31   :
    	case FIP_31    :

		P->FDM_Internal_Use.Page_2000.Timer_Cnt = 3;
		switch(P->S.TSlot){
		case 100:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 0<<14 ;
			break;
		case 400:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 1<<14 ;
			break;
		case 1000:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 2<<14 ;
			break;
		case 4000:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 3<<14 ;
			break;
		default:
			FDM_Signal_Error( P ,  _USER_MODE_FAULT , 0) ;
			goto abort;
 		 }

 		 /* silence = 4096 */
		 P->FDM_Internal_Use.Page_2000.Timer_Cnt |= (4096/1024)<<8 ;
 		 /* PRtime = 424 */
		 P->FDM_Internal_Use.Page_2000.Timer_Cnt |= ((424-40)/128)<<2 ;

    		 break;
#endif
#if ( FDM_WITH_FIPCODE_1000 == YES )

    	case WORLD_FIP_1000    :
    	case FAST_FIP_1000     :
    	case SLOW_FIP_1000   :

		P->FDM_Internal_Use.Page_2000.Timer_Cnt = 1;
		switch(P->S.TSlot){
		case 62:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 0<<14 ;
			break;
		case 250:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 1<<14 ;
			break;
		case 625:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 2<<14 ;
			break;
		case 2500:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 3<<14 ;
			break;
		default:
			FDM_Signal_Error( P ,  _USER_MODE_FAULT  , 0 ) ;
			goto abort;
 		}
		if  ( P->S.Mode == SLOW_FIP_1000 ) {
 		    	/* silence = 240 */
		    	P->FDM_Internal_Use.Page_2000.Timer_Cnt |= (290/5)<<8 ;
 		   	/* PRtime = 42.00 ( on met le max !!!!!!*/
		   	P->FDM_Internal_Use.Page_2000.Timer_Cnt |= ((4031-125)/62)<<2 ;
		} else {
 		    	/* silence = 150 */
		    	P->FDM_Internal_Use.Page_2000.Timer_Cnt |= (150/5)<<8 ;
 		   	/* PRtime = 10,00 */
		   	P->FDM_Internal_Use.Page_2000.Timer_Cnt |= ((1000-125)/62)<<2 ;
     		}

    		break;

    	case WORLD_FIP_2500   :
    	case FIP_2500    :

		P->FDM_Internal_Use.Page_2000.Timer_Cnt = 0;
		switch(P->S.TSlot){
		case 50:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 0<<14 ;
			break;
		case 200:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 1<<14 ;
			break;
		case 500:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 2<<14 ;
			break;
		case 2000:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 3<<14 ;
			break;
		default:
			FDM_Signal_Error( P ,  _USER_MODE_FAULT , 0) ;
			goto abort;
 		 }

 		 /* silence = 96 */
		 P->FDM_Internal_Use.Page_2000.Timer_Cnt |= (96/4)<<8 ;
 		 /* PRtime = 13.5 */
		 P->FDM_Internal_Use.Page_2000.Timer_Cnt |= (((135-5)*2)/10)<<2 ;

    		 break;
#endif
#if ( FDM_WITH_FIPCODE_5000 == YES )
    	case WORLD_FIP_5000   :
    	case FIP_5000    :

		P->FDM_Internal_Use.Page_2000.Timer_Cnt = 0;
		switch(P->S.TSlot){
		case 50:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 0<<14 ;
			break;
		case 200:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 1<<14 ;
			break;
		case 500:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 2<<14 ;
			break;
		case 2000:
			P->FDM_Internal_Use.Page_2000.Timer_Cnt |= 3<<14 ;
			break;
		default:
			FDM_Signal_Error( P ,  _USER_MODE_FAULT , 0) ;
			goto abort;
 		 }

 		 /* silence = 96 */
		 P->FDM_Internal_Use.Page_2000.Timer_Cnt |= (96/4)<<8 ;
 		 /* PRtime = 32 */
		 P->FDM_Internal_Use.Page_2000.Timer_Cnt |= (((320-5)*2)/10)<<2 ;

    		 break;

#endif

		case FDM_OTHER: /*_31:
   		case FDM_OTHER_1000 :
    	case FDM_OTHER_2500 :
    	case FDM_OTHER_5000 :
		*/

    		P->FDM_Internal_Use.Page_2000.Timer_Cnt =
    		        P->S.User_responsability.TIMER_CNT_REGISTER;
		P->S.TSlot = TAb_TSLOT
			[P->FDM_Internal_Use.Page_2000.Timer_Cnt & 3]
			[P->FDM_Internal_Use.Page_2000.Timer_Cnt >> 14];
		if (P->S.TSlot == 0 ){
			FDM_Signal_Error( P ,  _USER_MODE_FAULT , 0) ;
			goto abort;
                }
         	break;
	default:
		break;
           }
	}




	{
	/* Reference croisee Cle_Var <=> MPS_Var */
 	int i;
	i = (sizeof (void*) * (P->FDM_Internal_Use.Desc_Par<<4)) ;
	P->FDM_Internal_Use.fipgen.Xrf = (Xref_Key_Var*) FDM_Malloc  ( P , i );
	if ( P == (FDM_REF *) 0 ) {
		FDM_Signal_Error( P  , _ALLOCATE_MEMORY_FAULT , 0 ) ;
		goto abort;
	}
	memset( P->FDM_Internal_Use.fipgen.Xrf , 0 , i );
#if ( FDM_WITH_APER	==	YES )
	/* init timers time out aperiodique */
	P->FDM_Internal_Use.T_aper_N.Timer.T_init    = P->S.Time_Out_Ticks;
	P->FDM_Internal_Use.T_aper_N.Timer.User_Prog =
		( void (*) ( struct _TEMPO_DEF *) ) _FIP_APER_FLUSHOUT_N ;
	P->FDM_Internal_Use.T_aper_N.Fip      	= P ;

	P->FDM_Internal_Use.T_aper_U.Timer.T_init    = P->S.Time_Out_Ticks;
	P->FDM_Internal_Use.T_aper_U.Timer.User_Prog =
		( void (*) ( struct _TEMPO_DEF *) ) _FIP_APER_FLUSHOUT_U ;
	P->FDM_Internal_Use.T_aper_U.Fip      	= P ;
#endif
	}


	P->FDM_Internal_Use.fipgen.BASE_0[0]   = FDM_ADR_USER;

#if			( FDM_WITH_BA == YES )

if ( P->S.BA_Dim != 0 ) {
	int	i,j;
	Ushort	Adr;
	Ushort 	B[64];

	if ( ((int)(P->S.BA_Dim) % (1<<10)) != 0 )
	 	P->S.BA_Dim = (Ushort)((((P->S.BA_Dim>>10)+1)<<10)) ;
	P->FDM_Internal_Use.fipgen.BASE_0[0] += (long) P->S.BA_Dim ;

	P->FDM_Internal_Use.Page_2000.BA_Num_Abonne = (Ushort) P->H.K_PHYADR;
	P->FDM_Internal_Use.Page_2000.BA_Num_Max_Abonne = 255 ;


#if 				( FDM_WITH_OPTIMIZED_BA == YES )
	if (P->H.GreatestNumberOfBusArbitrator != 0) {
		int V,R;
		P->FDM_Internal_Use.Page_2000.BA_Nr         =  P->H.NumberOfThisBusArbitrator ;
		P->FDM_Internal_Use.Page_2000.BA_Nr_Max     =  P->H.GreatestNumberOfBusArbitrator ;
		V = ( P->FDM_Internal_Use.Page_2000.Timer_Cnt >> 8 ) & 0x3f;
		switch ( P->FDM_Internal_Use.Page_2000.Timer_Cnt & 3 ) {
		case 0 : /*2.5Mhz*/
			V *= 4;
			R =   TEST_P_DURATION / V;
			break;
		case 1 : /*1Mhz*/
			V *= 5;
			R =   TEST_P_DURATION / V;
			break;

		case 3 : /*31.25MKhz*/
			V *= 1024;
			R =   TEST_P_DURATION *5/ V;
			break;
 		}

 		P->FDM_Internal_Use.Ba.seuil_TSTP = (Ushort) R;
	} else {
		P->FDM_Internal_Use.Page_2000.BA_Nr         =  P->H.K_PHYADR  ;
		P->FDM_Internal_Use.Page_2000.BA_Nr_Max     =  255 ;
		P->FDM_Internal_Use.Ba.seuil_TSTP 	    =  257 ;
	}
#else
	P->FDM_Internal_Use.Page_2000.BA_Nr         =  P->H.K_PHYADR  ;
	P->FDM_Internal_Use.Page_2000.BA_Nr_Max     =  255 ;
	P->FDM_Internal_Use.Ba.seuil_TSTP 	    =  257 ;
#endif

	P->FDM_Internal_Use.Page_2000.BA_End_Macro_Time_Value = Calcul_tempo_31_25 ( P  ) ;

	/* Rappel:  P->FDM_Internal_Use.Page_2000.BA_Priority = 0 ;*/
   	P->FDM_Internal_Use.Page_2000.BA_Start_Time_Par =
		(Ushort) ( 2 * ( ((P->FDM_Internal_Use.Page_2000.BA_Nr_Max+1)*16) + P->FDM_Internal_Use.Ba.seuil_TSTP + 3 ) );
   	P->FDM_Internal_Use.Page_2000.BA_Election_Time_Par =
		(Ushort) ( 2 * ( ((P->FDM_Internal_Use.Page_2000.BA_Nr_Max+1)*0 ) + P->FDM_Internal_Use.Page_2000.BA_Nr  + P->FDM_Internal_Use.Ba.seuil_TSTP + 3 ) );

        if (P->S.Mode == SLOW_FIP_1000 ){
	  	Ulong T;
	  	/* bidouille corrective de tempo de start BA */

	  	T =( Ulong )P->FDM_Internal_Use.Page_2000.BA_Election_Time_Par  ;
	  	T = (T * 100)/121;
	  	P->FDM_Internal_Use.Page_2000.BA_Election_Time_Par =( Ushort ) T;

	  	T =( Ulong )P->FDM_Internal_Use.Page_2000.BA_Start_Time_Par ;
	  	T = (T * 100)/121;
	  	P->FDM_Internal_Use.Page_2000.BA_Start_Time_Par =( Ushort ) T;
	}

	{
		int 		lg;
		lg =(int)(P->S.BA_Dim / 32) /16 ;
		P->FDM_Internal_Use.fipgen.N_PAGE_BA =
                       	(BMc *)FDM_Malloc( P , lg);
    		if (P->FDM_Internal_Use.fipgen.N_PAGE_BA == NULL )  {
         		FDM_Signal_Error (P,_ALLOCATE_MEMORY_FAULT,0);
			goto abort;
	     	}

    		memset((void *)P->FDM_Internal_Use.fipgen.N_PAGE_BA ,0 , lg);

	}
	{/* init Anti dup des ID de presence */

#if ( FDM_WITH_FREE_ACCESS == NO )

		P->FDM_Internal_Use.Ba.Prog_Ba_TestP =  &P->FDM_Internal_Use.SProg ;
		memcpy ( P->FDM_Internal_Use.Ba.Prog_Ba_TestP , SP_TSTP , sizeof ( SP_TSTP ));
		P->FDM_Internal_Use.Ba.Prog_Ba_TestP->X[10] = SUSPEND_DELAI / P->S.TSlot ;
	 	_FIP_WRITE_PHY(P, FDM_ADR_USER   , (Ushort*) P->FDM_Internal_Use.Ba.Prog_Ba_TestP , 128 );

#else
		volatile Ushort * Destination ;

		Destination = P->H.FREE_ACCES_ADDRESS;
		Destination +=  FDM_ADR_USER ;
		P->FDM_Internal_Use.Ba.Prog_Ba_TestP = (SProg_TST_P*) Destination ;
		for ( i = 0 ; i < 64 ; i ++ ) {
			*Destination++ = SP_TSTP[i] ;
		}
		P->FDM_Internal_Use.Ba.Prog_Ba_TestP->X[10] = SUSPEND_DELAI / P->S.TSlot  ;

#endif
		Adr = dem_mem_MC( P ,2 );
   		memset((void *)&B[0] , 0 ,sizeof(B));

		for (j=0 ; j<8 ; j++){
			for (i=0 ; i<32 ; i++){
	     			B[2*i] = (Ushort)(0x1400 + i + (j*32)) ;
	 		}
			Adr = dem_mem_MC( P ,1  ) ;
	 		_FIP_WRITE_PHY(P, FDM_ADR_USER + (Ulong) Adr  , &B[0] , 128 );
    		}
	}
	{/*init fifos */
      		int TRQ_Dim ;
		int loop;

		TRQ_Dim = P->S.NB_OF_DIFFERENT_ID_PROG_BA << 1 ;

		TRQ_Dim =  ( TRQ_Dim < 0x40  ) ? 0x40 : TRQ_Dim ;
		if (( TRQ_Dim % 0x40 ) != 0 ) TRQ_Dim = (Ushort)((((TRQ_Dim>>6)+1)<<6)) ;

            for ( loop = 0; loop < 3 ; loop++ ) {

		for (j=0 ; j < TRQ_Dim ; j += 64 ){
   			memset((void *)&B[0] , 0 ,sizeof(B));
			Adr = dem_mem_MC( P ,1 );
			if ( j == 0) P->FDM_Internal_Use.Page_2000.BA_Adr_Fifo_TRQ[loop] = Adr;
			for (i=0 ; i<64 ; i = i+2) {
				if ( (j+i) == (TRQ_Dim - 2))
					B[i+1] = P->FDM_Internal_Use.Page_2000.BA_Adr_Fifo_TRQ[loop] ;
				else
					B[ i+1 ] = Adr + 2  + i;
	 		}
	 		_FIP_WRITE_PHY(P,  FDM_ADR_USER + (Ulong) Adr  , &B[0] , 128 );
		}
	    }
	}
	{/* init reste info  */
		P->FDM_Internal_Use.Ba.Max_priority = 15;
		P->FDM_Internal_Use.Page_2000.BA_Base_B = FDM_ADR_USER >> 10;
	}

	}


#endif

	{ /* Calcul des adresses des differentes Bases  */
	unsigned int Dim_base,T;
	int Dicho_Cr;
	Ushort k;

	T = P->S.NB_OF_USER_MPS_VARIABLE +6;

	for (  k = 0; T != 0; T >>= 1 ){ k++;}
	if ( k < 6 ) k = 6;
	P->FDM_Internal_Use.Frame_Par = k;

	if ( P->FDM_Internal_Use.Frame_Par > 12 ) {
		FDM_Signal_Error( P  , _USER_NB_OF_USER_MPS_VARIABLE , 0 ) ;
		goto abort;
	}


	Dicho_Cr = _DICHO_CREATE_FIX ( U_hard->Memory_Management_Ref ,  &P->FDM_Internal_Use.fipgen.D_Trame_TABLE  , P->FDM_Internal_Use.Frame_Par+1) ;
	if ( Dicho_Cr != 0 ) {
		FDM_Signal_Error( P  , _ALLOCATE_MEMORY_FAULT , 0 ) ;
		goto abort;
	}

	Dim_base = 1 << ( P->FDM_Internal_Use.Frame_Par + 4 ) ;

	P->FDM_Internal_Use.fipgen.BASE_0[1] = P->FDM_Internal_Use.fipgen.BASE_0[0] + Dim_base;

	if ( ( P->S.Type & TWO_IMAGE_MODE ) != 0 ) {
		P->FDM_Internal_Use.fipgen.BASE_0[2] = P->FDM_Internal_Use.fipgen.BASE_0[1] + Dim_base ;
	}
	else {
		P->FDM_Internal_Use.fipgen.BASE_0[2] = P->FDM_Internal_Use.fipgen.BASE_0[1] ;
	}
	P->FDM_Internal_Use.fipgen.BASE_1 = P->FDM_Internal_Use.fipgen.BASE_0[2] + Dim_base;
	P->FDM_Internal_Use.fipgen.BASE_2 = P->FDM_Internal_Use.fipgen.BASE_1 + ((P->FDM_Internal_Use.Desc_Par<<4)*16) ;

	P->FDM_Internal_Use.Page_2000.Base_1 = (unsigned short)( P->FDM_Internal_Use.fipgen.BASE_1 >> 10 );

	}



	{ /* Initialisation  Bit Map buffer de data */
	Ulong ll,RESTE;
	unsigned int i,j,k ;
	int idx ;

	ll = RESTE = P->S.FULLFIP_RAM_Dim - P->FDM_Internal_Use.fipgen.BASE_2 ;
	if ((RESTE & 0x80000000L ) != 0 ) {
		FDM_Signal_Error( P  , _PRIVATE_FULLFIP_RAM_OVERFLOW , 0 );
		goto abort;
	}

	ll = (ll>>(6+3))<<(6+3);
	j = (unsigned int)( RESTE - ll );         /* place perdu */
	if ( j != 0 ) RESTE = ll + 0x200;
	if ( RESTE != 0 ) {
		idx = (int)( RESTE >> (6+3) ) ;
		P->FDM_Internal_Use.fipgen.ptr_MAP_v =
			(BM *) FDM_Malloc ( P ,  ( unsigned short ) idx );
		if (P->FDM_Internal_Use.fipgen.ptr_MAP_v == (BM *)0 ) /*NULLPTR*/ {
			FDM_Signal_Error( P  , _ALLOCATE_MEMORY_FAULT , 0 );
			goto abort;
		}

		memset(P->FDM_Internal_Use.fipgen.ptr_MAP_v , 0 , idx  );
		k = 0;
		j = j>>6;
		for (i = 0 ; i < j  ; i++) {
			k = (k >> 1)+0x8000;
		}
		/* on supprime les pages qui debordent */
		P->FDM_Internal_Use.fipgen.ptr_MAP_v->bit_map[ (idx/2) - 1 ] = (unsigned short ) k;
	}
	P->FDM_Internal_Use.fipgen.Dim_Map_V = (int) (RESTE >> 6 );
	}


	dem_cle( P );/* EOC*/
	dem_cle( P );/*Msg Prod*/
	dem_cle( P );/*Msg Cons*/



	{/******* formatage de la variable d'identification *****************/
	Uchar *cp ;
	unsigned int k0;

	cp = &Local_Buffer[0];
/* codage  de la variable d'identification au format en ASN-1*/
/*80:=vendeur*/
	if ( U_ident->Vendor_Name == NULL ) {
		FDM_Signal_Error( P  , _ILLEGAL_IDENTIFICATION_PARAMETER , 0 );
		goto abort;
	}
	cp += ASNS(( unsigned char ) 0x80,(unsigned char *)cp, U_ident->Vendor_Name );
	k0 = (unsigned int) (cp - &Local_Buffer[0]);
	if (k0 > 126)  {
		FDM_Signal_Error( P  , _ILLEGAL_IDENTIFICATION_LENGTH , 0);
		goto abort;
	}

/*81:=modele*/
	if ( U_ident->Model_Name == NULL ) {
		FDM_Signal_Error( P ,  _ILLEGAL_IDENTIFICATION_PARAMETER , 0 );
		goto abort;
	}
	cp += ASNS(( unsigned char ) 0x81,(unsigned char *)cp, U_ident->Model_Name );
	k0 = (unsigned int) (cp - &Local_Buffer[0]);
	if (k0 > 126)  {
		FDM_Signal_Error( P  , _ILLEGAL_IDENTIFICATION_LENGTH  , 0 );
		goto abort;
	}

/*82*/
	*cp++ = 0x82;
	*cp++ = 1;
	if ( U_ident->Revision == NULL ){
		FDM_Signal_Error( P ,  _ILLEGAL_IDENTIFICATION_PARAMETER , 0 );
		goto abort;
	}
	*cp++ = * (unsigned char *) U_ident->Revision ;

/*83:=repere*/
	if ( U_ident->Tag_Name != NULL ) {
		cp += ASNS(( unsigned char ) 0x83,(unsigned char *)cp, U_ident->Tag_Name );
		k0 = (unsigned int) (cp - &Local_Buffer[0]);
		if (k0 > (126 - 4 ) )  {
			FDM_Signal_Error( P  , _ILLEGAL_IDENTIFICATION_LENGTH  , 0);
			goto abort;
		}
	}

/*84:F.sec SM_MPS  La variable de rapport est imposee !!! */
	*cp++ = 0x84;
	*cp++ = 1;
	*cp   = 0x10;
	if ( U_ident->SM_MPS_Conform != NULL )
		*cp |= * (unsigned char *) U_ident->SM_MPS_Conform ;
	cp++;

/*85:F.SMS_Conform*/
	if ( U_ident->SMS_Conform != NULL ) {
		cp += ASNT(( unsigned char ) 0x85,(unsigned char *)cp, U_ident->SMS_Conform );
		k0 = (unsigned int) (cp - &Local_Buffer[0]);
		if (k0 > 126)  {
			FDM_Signal_Error( P , _ILLEGAL_IDENTIFICATION_LENGTH , 0 );
			goto abort;
		}
	}
/*86:F.MPS_Conform ; MMS_Conform ; DLL_Conform , PHY_Conform*/
	if ( U_ident->PMDP_Conform != NULL ) {
		cp += ASNT(( unsigned char ) 0x86,(unsigned char *)cp, U_ident->PMDP_Conform );
		k0 = (unsigned int) (cp - &Local_Buffer[0]);
		if (k0 > 126)  {
			FDM_Signal_Error( P , _ILLEGAL_IDENTIFICATION_LENGTH , 0  );
			goto abort;
		}
	}

/*8A:F.Vendor_Field ; libre constructeur */
	if ( U_ident->Vendor_Field != NULL ) {
		cp += ASNS(( unsigned char ) 0x8A,(unsigned char *)cp, U_ident->Vendor_Field );
		k0 = (unsigned int) (cp - &Local_Buffer[0]);
		if (k0 > 126)  {
			FDM_Signal_Error( P , _ILLEGAL_IDENTIFICATION_LENGTH , 0  );
			goto abort;
		}
	}

/*fin*/
	k0 = (unsigned int) (cp - &Local_Buffer[0]);
	if (k0 > 126)  {
		FDM_Signal_Error( P , _ILLEGAL_IDENTIFICATION_LENGTH , 0  );
		goto abort;
	}
	P->FDM_Internal_Use.Presence.lg_ident = ( Uchar ) k0 ;
	P->FDM_Internal_Use.Ident_buffer[0] = 0x50;
	P->FDM_Internal_Use.Ident_buffer[1] = (Uchar) k0;
	memcpy ( &P->FDM_Internal_Use.Ident_buffer[2] , Local_Buffer , k0 );

	}

	{ /* SM-MPS  presence */

	P->FDM_Internal_Use.Ba.AE_LE_TSTP.Fip = P ;
	P->FDM_Internal_Use.Ba.AE_LE_TSTP.Etat = 3;
	P->FDM_Internal_Use.Ba.AE_LE_TSTP.Var[0].definition = SM_MPS[0];
	P->FDM_Internal_Use.Ba.AE_LE_TSTP.Var[0].Ref  = &P->FDM_Internal_Use.Ba.AE_LE_TSTP ;

	P->FDM_Internal_Use.Presence.PDU = 0x80 ;
	P->FDM_Internal_Use.Presence.lg  = 0x03 ;
#if				( FDM_WITH_BA == YES )
	P->FDM_Internal_Use.Presence.Status  = 0x10 ;
#endif
	CR = CRE_DVAR  ( P,
		 &P->FDM_Internal_Use.Ba.AE_LE_TSTP.Var[0].definition , 0x5000 ,  NULL) ;
	if ( CR == 0xffff )	{
		printf("_AE_LE_TSTP\n");
	 	goto abort1 ;
	}

	P->FDM_Internal_Use.Ba.AE_LE_TSTP.Var[0].definition.Type.Communication = 1;
	P->FDM_Internal_Use.Ba.AE_LE_TSTP.Var[0].definition.Type.Position = ( P->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;
	P->FDM_Internal_Use.Ba.AE_LE_TSTP.Var[0].definition.ID |= P->H.K_PHYADR;

	CR =  CRE_DTRAME ( P ,
		 &P->FDM_Internal_Use.Ba.AE_LE_TSTP.Var[0].definition , 0x5000 ,
		(Uchar *)&P->FDM_Internal_Use.Presence  ) ;
	if ( CR == 0xffff )
	{
		printf("_CRE_DTRAME\n");
		 goto abort1 ;
	}

#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
	P->FDM_Internal_Use.fipgen.Xrf->Var[CLE_PRESENCE_C] =
	P->FDM_Internal_Use.fipgen.Xrf->Var[CLE_PRESENCE] =
          (FDM_MPS_VAR_REF*) &P->FDM_Internal_Use.Ba.AE_LE_TSTP.Var[0].definition ;
#endif

	}


	{ /* SM-MPS  Liste des presents  */
	FDM_XAE     SM_MPS_VAR ;

	SM_MPS_VAR = SM_MPS[2];
	SM_MPS_VAR.Type.Position = ( P->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;
#if						(  FDM_WITH_FIPIO == YES )
	SM_MPS_VAR.Var_Length =
		( (P->S.Type &  FIPIO_MONO  ) ==  FIPIO_MONO  ) ?  34 : 68  ;
#endif

	P->FDM_Internal_Use.Liste_present.PDU0 = 0x80;
	P->FDM_Internal_Use.Liste_present.lg0  = 0x20;
	P->FDM_Internal_Use.Liste_present.PDU1 = 0x81;
	P->FDM_Internal_Use.Liste_present.lg1  = 0x20;

	CR =  CRE_DTRAME ( P, &SM_MPS_VAR , 0x5000 ,
		(Uchar*)&P->FDM_Internal_Use.Liste_present.PDU0 ) ;

	if ( CR != 0 )
	{
		printf("CRE_TRAME_BIS\n");
		 goto abort1 ;
	}

	}


	{ /* SM-MPS  Variable de Rapport */
	FDM_XAE     SM_MPS_VAR ;

	SM_MPS_VAR = SM_MPS[3];
	SM_MPS_VAR.ID |=  P->H.K_PHYADR;
	SM_MPS_VAR.Type.Position = ( P->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

	memcpy( &P->FDM_Internal_Use.Report_Variable , Report_Variable_init , sizeof(Report_Variable_init) );
	CR =  CRE_DTRAME ( P, &SM_MPS_VAR , 0x5000 ,
		(Uchar*)&P->FDM_Internal_Use.Report_Variable ) ;
	if ( CR != 0 ) 
	{
		printf("CRE_TRAME_TER\n");
		goto abort1 ;
	}

	}


	{ /* create SM-MPS  identification variable */
	FDM_XAE     SM_MPS_VAR ;

	SM_MPS_VAR = SM_MPS[1];
	SM_MPS_VAR.ID |=  P->H.K_PHYADR;
	SM_MPS_VAR.Var_Length  = P->FDM_Internal_Use.Presence.lg_ident ;
	SM_MPS_VAR.Type.Position = ( P->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

	CR =  CRE_DTRAME ( P, &SM_MPS_VAR , 0x5000 , Local_Buffer ) ;
	if ( CR != 0 )
	{
		printf("CRE_TRAME_QUATRE\n");
		 goto abort1 ;
		 }

	}



	{ /* Initialisation FIFOS inter-tache */
#if		( FDM_WITH_APER == YES )
	if ( P->S.User_Signal_Mps_Aper !=
			(void (*)(struct _FDM_REF *))0 )
		FDM_INITIALIZE_TASK_MPS    ( P ,   P->S.User_Signal_Mps_Aper ) ;
#endif

#if		( FDM_WITH_SM_MPS == YES )
	if ( P->S.User_Signal_Smmps !=
			(void (*)(struct _FDM_REF *))0 )
		FDM_INITIALIZE_TASK_SM_MPS ( P ,   P->S.User_Signal_Smmps ) ;
#endif
	}


	{ /* SM-MPS  Synchro BA  */
	FDM_XAE     SM_MPS_VAR ;

	_FDM_XAE_Tempo_Ref *T;
	FDM_AE_LE_REF *Synch_ba;
	FDM_MPS_VAR_REF *El ;

	SM_MPS_VAR = SM_MPS[4];
	SM_MPS_VAR.Type.Position = ( P->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

	CR =  CRE_DTRAME ( P, &SM_MPS_VAR , 0x5000 , NULL ) ;
	if ( CR != 0 ) 
	{
		printf("CRE_TRAME_CINQ\n");
		goto abort1 ;
		}
	P->FDM_Internal_Use.Synchro_BA.PDU      = 0x80;
	P->FDM_Internal_Use.Synchro_BA.lg       = 0x02;
	P->FDM_Internal_Use.Synchro_BA.Station  = P->H.K_PHYADR;

	T = (_FDM_XAE_Tempo_Ref *) &P->FDM_Internal_Use.SM_MPS_Synchro_BA.Var[0].Timer;
	T->Compte_rendu = 2;
	Synch_ba  = &P->FDM_Internal_Use.SM_MPS_Synchro_BA ;
	Synch_ba->Etat = AE_LE_CONFIG;
	Synch_ba->dim  = 1;
	Synch_ba->Fip  = P ;

	El = fdm_mps_var_create (  Synch_ba,  &SM_MPS_VAR );
	El->Ref->Etat =  AE_LE_RUNNING ;
	El->cle_Con   = CLE_SYNCHRO_BA ;
	El->cle_Pro   = CLE_SYNCHRO_BA_PROD;
	El->Timer.File    = P->FDM_Internal_Use.FIFO_SM_MPS ;
	El->Timer._.Type_out	= 1;

	}

#if			( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
	{ /* SM-MPS  Segment_Param */
	FDM_XAE     SM_MPS_VAR ;

	_FDM_XAE_Tempo_Ref *T;
	FDM_AE_LE_REF *Segment_Param;
	FDM_MPS_VAR_REF *El ;

	if ( ( P->S.Type & (  FIPIO_MONO | FIPIO_BI )) !=  0  ) {


		Local_Buffer[0] = 0x80 ;
		Local_Buffer[1] = 0x04 ;
		Local_Buffer[3] = 0; /* P->H.MySegment */;
		Local_Buffer[4] = P->S.Nr_Of_Repeat ;

		switch (P->S.Mode) {
	    	case SLOW_FIP_1000 :
			Local_Buffer[2] = 255 ; /*silence */
			Local_Buffer[5] = 70 ;	/* RpTime */
			break;
	    	case WORLD_FIP_1000    :
	    	case FAST_FIP_1000     :
			Local_Buffer[2] = 150 ; /*silence */
			Local_Buffer[5] = 10 ; /* RpTime */
			break;

		default :
			FDM_Signal_Error ( P  , _CONFLICT_CONFIGURATION, 2 ) ;
			printf("CONFICT CONFIGURATION\n");
			goto abort1 ;
		}
		SM_MPS_VAR =  SM_MPS_SEG_PARAM;
		SM_MPS_VAR.Type.Position = ( P->S.Type & TWO_IMAGE_MODE ) ? 3 : 1 ;

		CR =  CRE_DTRAME ( P, &SM_MPS_VAR , 0x5000 , Local_Buffer ) ;
		if ( CR != 0 ) 
		{
			printf("Encore CDE_TRAME\n");
			goto abort1 ;
		}

		Segment_Param = & P->FDM_Internal_Use.fipio.SM_MPS_Seg_Par;
		Segment_Param->Fip = P;
		Segment_Param->dim  = 1;
		Segment_Param->Etat = AE_LE_CONFIG;

		El = fdm_mps_var_create (   Segment_Param ,  &SM_MPS_VAR );
		El->Ref->Etat =  AE_LE_RUNNING ;
		El->cle_Con   = CLE_SEGPAR_CONS  ;
		El->cle_Pro   = CLE_SEGPAR_PROD  ;

		El->Timer.T_init        =  P->S.Segment_Paramers_Ticks;
		El->Timer._.Periodique  = 1;
		El->Timer._.Mode  	= 0;
		El->Timer._.Type_out  	= 0;
		El->Timer.User_Prog =
			( void (*) (  TEMPO_DEF* ) )  _UPDATE_SEG_PARAM_VAR ;

		}
	}
#endif



/*+++++++++++++++++++++     Fipdiag   +++++++++++++++++++++++++++++++++++*/

#if 		( FDM_WITH_DIAG == YES )

	{ /* Variable de Test Ram on line  */
	CR=  _FIP_CRE_DVAR_TEST_RAM( P );
	if ( CR != 0 )
		{
		printf("_FIP_CRE_DVAR_TEST_RAM\n");
		 goto abort1 ;
	}

	}
#endif




#if		( FDM_WITH_MESSAGING == YES)


   {



    /* Initialisation Messagerie  Reception*/


    _DICHO_CREATE_VAR (P->H.Memory_Management_Ref ,
 			&(P->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local)) ;

    if ( ( P->S.Type & MESSAGE_RECEPTION_AUTHORIZED ) != 0 ) {
	P->FDM_Internal_Use.Messagerie.Reveive_pool    =
      			_NEW_USER ( P->H.Memory_Management_Ref  ,
    				sizeof ( FDM_MSG_R_DESC)  , 1 ) ;

       /* ecriture de la table des segments */
       /*-----------------------------------*/
       { unsigned int j;

	 P->FDM_Internal_Use.Messagerie.TAB_SEGMENT[0]= SEGMENT_CONFIG2;
	 for (j=1; j <256 ;j++){
	     P->FDM_Internal_Use.Messagerie.TAB_SEGMENT[j]= SEGMENT_IGORED;
	 }
	 P->FDM_Internal_Use.Messagerie.TAB_SEGMENT[(int)P->H.MySegment]= SEGMENT_CONFIG2;
	 P->FDM_Internal_Use.Messagerie.TAB_SEGMENT[255]= SEGMENT_CONFIG1;

	 for ( j = 0 ; j < 4 ; j++ ) {
   		_FIP_WRITE_PHY(P, (Absolute_ADR)( MSG_TAB_SEGMENT + (j*64) ),
	         (Ushort *)&P->FDM_Internal_Use.Messagerie.TAB_SEGMENT[j*64] ,128);
             }
       }



       {
        unsigned int     i;
	unsigned int     nb_pages;

 	Ushort	adr_bloc_debut;
	Ushort	adr_bloc_suivant;
 	Ulong   adr_rgt_msg;
	Ushort	acces_cle;
	Ushort	page;


	DESC_FIFO_MSG_REC D_fifo;

        memset ( &D_fifo , 0 , sizeof(DESC_FIFO_MSG_REC));

        if ( P->S.User_Signal_Rec_Msg == (void (*)(struct _FDM_REF *))0 ) {
			FDM_Signal_Error ( P  , _MSG_PROG_MISSING , 0 ) ;
			goto abort ;
        }

        /*init  adr buffer rgt msg */
       /*------------------------*/
	adr_rgt_msg= 0;
        for ( i=0 ; i <3 ; i++){ /* demande 3 pages : message =266 octets */
          page = dem_mem(P);
          if ( page == 0  ) {
	     FDM_Signal_Error ( P , _VARIABLE_TABLE_OVERFLOW , 0) ;
	     goto abort ;
	  }
	  if (i==0) {
                       /* adresse du 1er buffer */
 	     adr_rgt_msg  =(Ulong )page;
             adr_rgt_msg  = adr_rgt_msg << 6 ;
          }
        }

	P->FDM_Internal_Use.Page_2000.Msg_Buffer_PForts=
					(Ushort)( adr_rgt_msg >> 10 );
	P->FDM_Internal_Use.Page_2000.Msg_Buffer_Pfaibles=
					(Ushort)(adr_rgt_msg & 0x3ff );




       /* init descripteur fifo + x pages de messages recus */
       /* 1) demande de x pages pour stocker les messages recus */

       /* nombre de pages de 64 mots a demander=
       (nombre de buffer * taille 1 message (mot) )/ 64 mots */
       i  =   MAX_MSG_REC * SIZE_1_MSG ;
       nb_pages = i >>6;
       if (( i % 0x40 ) !=0)
    	 nb_pages +=1;

      /* demande des nb_pages*/
       for ( i=0 ; i <=  nb_pages ; i++){
          page = dem_mem(P);
          if ( page == 0  ) {
	     FDM_Signal_Error ( P , _VARIABLE_TABLE_OVERFLOW , 0) ;
	     goto abort ;
	  }

          if (i==0) {
                       /* adresse du 1er buffer */
             adr_rgt_msg  =(Ulong )page;
             adr_rgt_msg  = adr_rgt_msg << 6 ;
          }
       }

      /* 2) demande de cles de descripteur de fifo de reception de message*/


        acces_cle = dem_cle_bar( P ) ;
	if (acces_cle ==0xffff) {
   	  FDM_Signal_Error ( P ,_VARIABLE_DESCRIPTOR_TABLE_OVERFLOW , 0) ;
	  goto abort ;
        }

       adr_bloc_debut= acces_cle * 0x10;
       P->FDM_Internal_Use.Page_2000.Msg_Adr_Fifo_Rec =adr_bloc_debut;


      for ( i=1 ; i <=  MAX_MSG_REC ; i++){

        if (i != MAX_MSG_REC){
           acces_cle = dem_cle_bar( P ) ;
           if (acces_cle ==0xffff) {
   	     FDM_Signal_Error ( P ,_VARIABLE_DESCRIPTOR_TABLE_OVERFLOW , 0) ;
	     goto abort ;
           }
           adr_bloc_suivant 	=acces_cle * 0x10;
        }else{
          adr_bloc_suivant     = P->FDM_Internal_Use.Page_2000.Msg_Adr_Fifo_Rec ;
        }

        D_fifo.Indic		=0;
	D_fifo.Adr_data_pforts  =(Ushort)(adr_rgt_msg >> 10);
	D_fifo.Adr_data_pfaibles=(Ushort)(adr_rgt_msg & 0x3ff);
	D_fifo.Adr_suivant	= adr_bloc_suivant;

	CR = _FIP_WRITE_DESCRIPTEUR ( P ,
		P->FDM_Internal_Use.fipgen.BASE_1 + adr_bloc_debut ,
 		(Ushort *) &D_fifo );
	if ( CR != 0 ) 
	{
		printf("FIP_WRITE_DESCRIPOTEUR\n");
		goto abort1 ;
	}

        /* adresse dans la page du buffer de messagerie suivant */
        adr_rgt_msg     = adr_rgt_msg + SIZE_1_MSG;

        adr_bloc_debut = adr_bloc_suivant;
      }

      P->FDM_Internal_Use.Page_2000.Last_Var_Time  -= MAX_MSG_REC ;


    }

   }else{
        /* pas de  MESSAGE_RECEPTION_AUTHORIZED */

        P ->FDM_Internal_Use.Page_2000.Msg_Adr_Fifo_Rec =0;
   }

} /* fin init messagerie reception */




	{ /* Initialisation Messagerie  Emission */


	unsigned int     i;
	unsigned int     nb_pages;

	Ushort	adr_bloc_debut;
	Ushort	adr_bloc_suivant;
 	Ulong   adr_rgt_msg;
  	Ushort	acces_cle;
  	Ushort  page;
	Ushort	num_canal;
	Ushort  Nr_Of_Tx_Buffer;


	DESC_FIFO_MSG_EMI D_fifo;


	for ( num_canal = 1 ; num_canal < 9; num_canal++){
		P->FDM_Internal_Use.Messagerie.CANAL[num_canal] = 0xFFFFFFFFL;
	}

        memset ( &D_fifo , 0 , sizeof(DESC_FIFO_MSG_EMI));

      if ( P->S.User_Signal_Send_Msg ==
		(void (*)(struct _FDM_REF *))0 ) {
	FDM_Signal_Error ( P  , _MSG_PROG_MISSING , 0 ) ;
	goto abort ;
      }

     for ( num_canal =0 ; num_canal < 9; num_canal++){

        Nr_Of_Tx_Buffer =P->S.Nr_Of_Tx_Buffer[num_canal] ;
        if ( Nr_Of_Tx_Buffer == 0){
          P->FDM_Internal_Use.Page_2000.Msg_Adr_Fifo_Emi[num_canal] =0;
        }
        else {

	P->FDM_Internal_Use.Page_2000.Last_Var_Time  -= Nr_Of_Tx_Buffer ;
     	/* pour chaque canal */
     	/*-------------------*/
        /* controle que pour chaque canal , on n'a pas plus de 32 buffers configures */
        if ( P->S.Nr_Of_Tx_Buffer[num_canal] >MAX_TX_BUFFER_EMI ){
   	    FDM_Signal_Error ( P , _USER_NR_OF_TX_BUFFER_FAULT , 0 );
	    goto abort ;
	}

         /* 1) demande de x pages pour stocker les messages a emettre */

          /* nombre de pages de 64 mots a demander */
         i  =   Nr_Of_Tx_Buffer * SIZE_1_MSG ;
         nb_pages = i >>6;
         if (( i % 0x40 ) !=0)
    	   nb_pages +=1;

         /* demande des nb_pages*/
	adr_rgt_msg = 0;
         for ( i=0 ; i <=  nb_pages ; i++){
           page = dem_mem(P);
           if ( page == 0  ) {
	     FDM_Signal_Error ( P , _VARIABLE_TABLE_OVERFLOW , 0) ;
	     goto abort ;
	   }
           if (i==0){
          		   /* adresse du 1er buffer */
            adr_rgt_msg		    =(Ulong )page;
            adr_rgt_msg		    = adr_rgt_msg << 6;
           }
         }

           /* 2) demande de cles de descripteur de fifo d'emission de message*/
         acces_cle = dem_cle_bar( P ) ;
     	 if (acces_cle ==0xffff) {
   	   FDM_Signal_Error ( P ,_VARIABLE_DESCRIPTOR_TABLE_OVERFLOW , 0) ;
	   goto abort ;
     	 }
     	 adr_bloc_debut =  acces_cle * 0x10;
       	 P->FDM_Internal_Use.Page_2000.Msg_Adr_Fifo_Emi[num_canal] = adr_bloc_debut;

     	 for ( i=1 ; i <=  Nr_Of_Tx_Buffer ; i++){

 	    if (i != Nr_Of_Tx_Buffer){
               acces_cle = dem_cle_bar( P ) ;
               if (acces_cle ==0xffff) {
   	         FDM_Signal_Error ( P ,_VARIABLE_DESCRIPTOR_TABLE_OVERFLOW , 0) ;
	         goto abort ;
               }
               adr_bloc_suivant 	=acces_cle * 0x10;
            }
            else{
               adr_bloc_suivant=
                   P->FDM_Internal_Use.Page_2000.Msg_Adr_Fifo_Emi[num_canal] ;
            }


            D_fifo.Indic	    =0;
	    D_fifo.Adr_data_pforts  =(Ushort)(adr_rgt_msg >> 10 );
	    D_fifo.Adr_data_pfaibles=(Ushort)(adr_rgt_msg & 0x3ff );

  	    D_fifo.Desc_fifo         =((Ushort)((i-1) |
	                                     ( (num_canal<<8) & 0xf7f)));
	    D_fifo.Adr_suivant       = adr_bloc_suivant;

	    CR = _FIP_WRITE_DESCRIPTEUR ( P ,
		   P->FDM_Internal_Use.fipgen.BASE_1 + adr_bloc_debut ,
 		  (Ushort *) &D_fifo );
	    if ( CR != 0 )
	    {
		printf("FIP_WRITE_DESCRIPTEUR\n");
	    	 goto abort1 ;
	    }

       	    /* adresse dans la page du buffer de messagerie suivant */
            adr_rgt_msg     = adr_rgt_msg + SIZE_1_MSG;

            adr_bloc_debut = adr_bloc_suivant;
         }/* endfor nr_of_tx_buffer*/
        } /* endif  nr_of_tx_buffer !=0*/
      } /* endfor num_canal */


     FDM_INITIALIZE_TASK_MSG_T ( P ,   P->S.User_Signal_Send_Msg ) ;

     P->FDM_Internal_Use.Page_2000.Msg_Repeat=P->S.Nr_Of_Repeat;
 }


#else
	/* pas de messagerie */
#endif





	{ /* fin d'initialisation  */

	P->FDM_Internal_Use.Page_2000.First_Var_Time = ( CLE_SEGPAR_CONS * 0x10 ) +2;
	P->FDM_Internal_Use.Page_2000.Last_Var_Time  =
		((P->FDM_Internal_Use.Page_2000.Last_Var_Time-1) *16) +2;

	P->FDM_Internal_Use.Page_2000.Page_Lib_User = dem_mem ( P );
	P->FDM_Internal_Use.Page_2000.Page_Lib_Rec  = dem_mem ( P );
	P->FDM_Internal_Use.Page_2000.Page_Lib_Emi  = dem_mem ( P );

	P->FDM_Internal_Use.Page_2000.TSlot_Value   =  ( U_soft->TSlot == 62 ) ? 125 : ( U_soft->TSlot*2) ;
	P->FDM_Internal_Use.Page_2000.Type_Station  = User_Type_Station ;

#if 							( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == YES )
	P->FDM_Internal_Use.Page_2000.EOC_Signal  = EOC_NIVEAU ;
#else
	P->FDM_Internal_Use.Page_2000.EOC_Signal  =
		(Ushort)(( ( P->S.Type & EOC_PULSE_MODE ) != 0 ) ?
			 EOC_PULSE : EOC_NIVEAU );
#endif

	P->FDM_Internal_Use.Page_2000.Mode_Fonct = User_Mode_Fonct ;

	P->FDM_Internal_Use.Page_2000.Redondancy = User_Redondancy ;

#if  				( FDM_WITH_FIPIO == YES )
	P->FDM_Internal_Use.Page_2000.Redondancy |=
		(Ushort)(( ( P->S.Type &  FIPIO_MONO ) == FIPIO_MONO )  ?
				RDD_FIP_MONO  : RDD_FIP_BIMED );
	P->FDM_Internal_Use.Page_2000.Redondancy |= RDD_LISTE_NON_PRETE ;
#endif

	P->FDM_Internal_Use.Page_2000.TXCK_Mode =
		(Ushort)(( ( P->S.Type &  EXTERNAL_TXCK ) == 0 ) ?
			 	MODE_TXCK : MODE_EXTERNE );

	P->FDM_Internal_Use.fipgen.image =
		((P->S.Type & IMAGE_2_STARTUP ) == 0 ) ?
				IMAGE_1: IMAGE_2 ;

	FDM_STOP_DB_MOD ( P );

	CR = _FIP_WRITE_PHY ( P , FIP_CONF_RESEAU , (Ushort *) &P->FDM_Internal_Use.Page_2000 , 128 );
	if (CR != 0 )
		{
		printf("_FIP_WRITE_PHY\n");
	 goto abort1;
		}


	CR |= _FIP_TEST_PRG ( P ); /* verif que fipcode est ok */
	if (CR != 0 ) 
	{
		printf("_FIP_TEST_PRG\n");
		goto abort1;
	}


	CR |= _FIP_START_CIRCUIT ( P );
	if (CR != 0 ) 
	{
		printf("_FIP_START_CIRCUIT\n");
		goto abort1;
	}
	}

/*+++++++++++++++++++++     Fipdiag   +++++++++++++++++++++++++++++++++++*/

#if ( FDM_WITH_DIAG == YES )

	{
           CR= _FIP_DIAG_OFF_LINE ( P );
           if (CR !=0 ) goto abort;
           _FIP_INIT_DIAG_ON_LINE ( P );
	}

#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

	{ /* vrai demarrage */
	Ulong tempo ;

	CR = _FIP_END_DIAG ( P );
	if (CR != 0 )
	{
		printf("_FIP_END_DIAG\n");
		 goto abort1;
	}

	P->FDM_Internal_Use.Medium.Timer_tst.Timer.T_init        =  P->S.Test_Medium_Ticks;
	P->FDM_Internal_Use.Medium.Timer_tst.Timer._.Periodique  = 1;
	P->FDM_Internal_Use.Medium.Timer_tst.Timer.User_Prog =
		( void (*) ( struct _TEMPO_DEF *) ) _FIP_TEST_MEDIUM ;
	P->FDM_Internal_Use.Medium.Timer_tst.Fip = P;
	tempo = _TIMER_START( (TEMPO_DEF*) &P->FDM_Internal_Use.Medium.Timer_tst.Timer );
	if ( tempo != 0 ) {
		FDM_Signal_Error ( P ,_INIT_TIMER_FAIL , 0 );
		goto abort ;
	}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#if		( FDM_WITH_FREE_ACCESS == YES )
	{
	volatile Ushort *AL;
	AL   = P->H.FREE_ACCES_ADDRESS;
	AL  += P->FDM_Internal_Use.fipgen.BASE_1;
	P->FDM_Internal_Use.AL.BASE_1 = (Ushort *) AL;

	AL  = P->H.FREE_ACCES_ADDRESS ;
	AL += FIP_R_FIFO_DDE_NOR ;
	P->FDM_Internal_Use.AL.FifoUser = (Ushort *) AL;


	AL  = P->H.FREE_ACCES_ADDRESS ;
	AL += FIP_V_FIFO_DDE_NOR ;
	P->FDM_Internal_Use.AL.FifoFip = (Ushort *) AL;

	AL  = P->H.FREE_ACCES_ADDRESS ;
	AL += FIP_FIFO_DDE_URG ;
	P->FDM_Internal_Use.AL.FifoUrgent =  (Ushort *)AL;
	}
#endif


#if (FDM_WITH_DIAG==YES)
	if ( P->S.Online_Tests_Ticks != 0 ) {
	  P->FDM_Internal_Use.fipdiag.Timer_tst.Timer.T_init =
						P->S.Online_Tests_Ticks;
   	  P->FDM_Internal_Use.fipdiag.Timer_tst.Timer._.Periodique  = 1;
	  P->FDM_Internal_Use.fipdiag.Timer_tst.Timer.User_Prog =
		( void (*) ( struct _TEMPO_DEF *) )_FIP_DIAG_ON_LINE_TICKS  ;
	  P->FDM_Internal_Use.fipdiag.Timer_tst.Fip = P;
	  tempo = _TIMER_START( (TEMPO_DEF*)
				&P->FDM_Internal_Use.fipdiag.Timer_tst.Timer );
  	  if ( tempo != 0 ) {
		FDM_Signal_Error ( P ,_INIT_TIMER_FAIL , 0 );
		goto abort ;
	   }
       }
#endif
#if			( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
	{
	   if ( ( P->S.Type &  ( FIPIO_MONO | FIPIO_BI )) != 0 ) {
		tempo = _TIMER_START( (TEMPO_DEF*)
			&P->FDM_Internal_Use.fipio.SM_MPS_Seg_Par.Var[0].Timer );
		if ( tempo != 0 ) {
			FDM_Signal_Error ( P ,_INIT_TIMER_FAIL , 0 );
			goto abort ;
		}
	   }
	}
#endif
	return P;
}



abort1:
	FDM_Signal_Error( P , _INIT_ABORTED , 0 );

abort :
	if ( P != NULL )  fdm_stop_network ( P ) ;
	return NULL;
}

