/************************************************************
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD



      Projet      : Fip Device Manager Version 4
 Nom du fichier  : fdmmsg.c
  Description     : Gestion de la messagerieconfiguration des contextes de messagerieEmission et Reception de messages

  Date de creation: Fri Mar 15 15:11:42 MET 1996
  Date de modification : Thu Jun 20 15:11:42 MET 1996
	messagerie sur image 1 et image 2
	bridge
 Historique des evolutions/modifications
  	Modification : 96.07.26
  	Reference Ra : RA xx
  	But de la modification :
  		modif Buffer--> FBuffer
  		(nom Buffer utilise dans <stdio.h> compilateur METAWARE 16bits)
   	Modification : 96.07.30
  	Reference Ra : RA 24
  	But de la modification :produre _FIP_READ_HEADER_AND_DATA
Version 4.1.1
	emission message chaine : si 1ere bloc = nb pair octet
	=>2eme blocs donnees non emises - Mode acces libre
Version 4.2.1
	a/- idem 4.1.1  cible BIG_ENDIAN
	b/- Acces libre sned msg aper.
Version 4.3.1
	a/- PURGE MSG  acces libre ( recodage)
Version 4.3.4
	a/- vide_fifo_MSG_TO acces a NULLPTR si erreur utilisateur.
Version 4.3.4
	a/- delete LSAP type bridge reception
Version 4.4.1
	a/- modif boucle de vidage emission message
Version 4.4.4
	a/- Config LSAP cons "pure" ( voir FIPCODE 6.4)
Version 4.5.1
	a/- aleas possible entre lecture EVT et son traitement.
Version 4.6.0
	a/- Acces 32 bits
Version 4.7.0
	a/- Ameliorations mineures

***********************************************************/
/* 	FDM_V4
	messagerie
*/
#include "user_opt.h"
#include "fdm.h"
#include "fdmmacro.h"


/************************************************************************/
/*	procedures de liberation des ressources du message recu 	*/
/*		-fdm_msg_ref_buffer_free				*/
/*		-fdm_msg_data_buffer_free				*/
/************************************************************************/
#if (FDM_WITH_MESSAGING ==YES)

Ushort  fdm_msg_ref_buffer_free(FDM_MSG_RECEIVED *Memory_Block)
{
	return (Ushort)
		((( _FREE_MEMORY ( (User_GDM*) Memory_Block ) == 0 ) ? 0 : 0xffff) );
}
Ushort fdm_msg_data_buffer_free(FDM_MSG_R_DESC * Memory_Block)
{
	return (Ushort)
		((( _FREE_MEMORY ( (User_GDM*) Memory_Block ) == 0 ) ? 0 : 0xffff)) ;
}

enum _MSG_REPORT_PRIVATE {
	_MSG_REPORT_NOT_INIT = 0,
	_MSG_REPORT_RECEIVED = 1,
        _MSG_REPORT_ACK_P    = 2,
        _MSG_REPORT_TO       = 4
};

/*###########################################################################*/
	/*  MESSAGERIE EMISSION*/
/*###########################################################################*/


/****************************************************************/
/*	  Determine le type d'adressage	     			*/
/****************************************************************/
/* Retourne : MSG_SDA  si adresse individuelle */
/*	      MSG_SDN  si adresse de groupe    */
static enum _FDM_MSG_ACK_TYPE
_FIP_ADDRESS_TYPE ( struct  _FDM_REF * Ref, Ulong Address )
{
    Ulong I_G ;         /* Individu_Groupe */
    Ulong S_R ;         /* Segment_Reseau */
    I_G =  Address  & 0x00800000L ;
    S_R =  Address  & 0x00000080L ;

#if  ( FDM_WITH_NT == YES)
switch ((Address >> 24) & 3 ){
case 0x2:return  _FDM_MSG_ACK_TYPE_SDN ;
case 0x3:return  _FDM_MSG_ACK_TYPE_SDA ;
case 0x00:    /* NORMALISE */
default:break;
}
#endif

    if( S_R == 0 ){                         /* Individu ou Gp_Restreint */
	    if( I_G == 0 )                  /* Individu */
	    	return  _FDM_MSG_ACK_TYPE_SDA ;
    }
    return   _FDM_MSG_ACK_TYPE_SDN ;

}

Ushort fdm_change_messaging_acknowledge_type(
	FDM_MESSAGING_REF *Dll,
	enum _FDM_MSG_ACK_TYPE (*User_Msg_Ack_Type)
		(struct  _FDM_REF * , Ulong Remote_Adr )   )
{
	if ( Dll == NULL ) return FDM_NOK;
	Dll->Send_infos.User_Msg_Ack_Type = User_Msg_Ack_Type;
	return FDM_OK;
}

/*************************************************************************
            _FIP_WRITE_MSG: ecriture d un message
retourne:
	8001h si defaut
	sinon  : bit6_0= no message alloue : no fifo
Nota : le test de la saturation du canal ; comparaison entre Cpt_Canal_T[]
            et Nr_of_Tx_Budffer[] doit etre fait avant
***************************************************************************/

static Ushort
_FIP_WRITE_MSG (
	FDM_REF * Ref ,
	MSG_HEADER_Send_Type *H ,
	FDM_MSG_TO_SEND * Message,
	Ushort Channel_Nr )
{

#if (FDM_WITH_FREE_ACCESS == NO)
   { /************ WRITE MESSAGE ************/
	int VAR_STATE ;
	FDM_ERROR_CODE CR ;
    	FIFO_Reg;
    	int            tempo,nb_block,lg_data;
  	FDM_MSG_T_DESC *src;
	Uchar          *Data_buffer;
	int	       lg_header;
	Uchar          *src_header;
	Debut_procedure ;
	WAIT_Busy;
	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Channel_Nr );
	/************ ecriture de la commande */
	ECRIRE_UCOM( CMDE_W_MSG );
	/* ici pour occuper utilement le micro */
	FIFO_port   = FIFO;
	src_header  = (Uchar *)H;
	lg_header   = sizeof( MSG_HEADER_Send_Type);
	nb_block    = Message->Nr_Of_Blocks;
	src	    = Message->Ptr_Block;
	Data_buffer = src->Ptr_Data;
	lg_data     = src->Nr_Of_Bytes ;
	/************ attente bit SV */
	WAIT_SV;
	_FIP__tres_petite_tempo ( Ref ) ;
	/************ test de validite  */
	VAR_STATE = LIRE(Var_State) ;
	if ((VAR_STATE & 0x80) != 0 ) {
                ECRIRE_UCOM(CMDE_CLOSE) ;
		OS_fdm_sm_v;
		return (0x8001);
	}
 	/************ boucle d'ecriture de l entete du message */
	 BLOCKOUTBYTE(FIFO_port,src_header,lg_header );
	/************ boucle d'ecriture des donnees utiles du message */
	for (;;) {
	    BLOCKOUTBYTE(FIFO_port,Data_buffer, lg_data);
	    src = src->Next_Block  ;
	    nb_block--;
	    if ( ( src == NULL )  || ( nb_block == 0 ) ) break;
	    Data_buffer = src->Ptr_Data ;
	    lg_data     = src->Nr_Of_Bytes ;
	}
	{
		int Cr_Fifo;

		OS_Enter_Region();

		Cr_Fifo = STORE_FIFO_INT( &(Ref->FDM_Internal_Use.Messagerie.fifo_CNF_T[Channel_Nr] ),
     			(FIFO_INT_ELT_TYPE*) Message);
		if ( Cr_Fifo == 0 ){
	      		Ref->FDM_Internal_Use.Messagerie.tab_wait_cnf[Channel_Nr][VAR_STATE] = Message ;
		        Ref->FDM_Internal_Use.Messagerie.Cpt_canal_T[Channel_Nr]++;
		} else {
			OS_Leave_Region();
	        	ECRIRE_UCOM(CMDE_CLOSE) ;
 			OS_fdm_sm_v;
			FDM_Signal_Error ( Ref , _FIFO_ACCES_FAILED  , 12) ;
  			return(0x8001);
		}

		OS_Leave_Region();
        	ECRIRE_UCOM(CMDE_CLOSE) ;
		OS_fdm_sm_v;
	        return ((Ushort)VAR_STATE);
		}
     }
#else
/****************************************************************************************/
/****************************************************************************************/
/*********************          ACCES LIBRE                 *****************************/
/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/

	volatile Ushort *buffer ,*FiFo_Adr ,*Abs_ADR;
	DESC_FIFO_MSG_EMI * BLOC_DE_DESCRIPTION;
	Ulong Ad ;
	int NBr , L ;

	buffer  = Ref->H.FREE_ACCES_ADDRESS;
	buffer += 0x21a9 + Channel_Nr;
	FiFo_Adr = Ref->H.FREE_ACCES_ADDRESS;
	FiFo_Adr += Ref->FDM_Internal_Use.fipgen.BASE_1;
	FiFo_Adr += *buffer;
	BLOC_DE_DESCRIPTION = (DESC_FIFO_MSG_EMI *) FiFo_Adr   ;
	*buffer = BLOC_DE_DESCRIPTION->Adr_suivant;
	Ad   = BLOC_DE_DESCRIPTION->Adr_data_pforts;
	Ad <<= 10;
	Ad  |= BLOC_DE_DESCRIPTION->Adr_data_pfaibles;
	Abs_ADR = Ref->H.FREE_ACCES_ADDRESS;
	Abs_ADR += Ad ;
	/* -----------------------ecrire Header */
	buffer = (Ushort * ) H;
	NBr = 5;

	do {
		L  =  *buffer++ ;
#if 			( FDM_WITH_LITTLE_ENDIAN == YES )
		L  =  ( L >> 8 ) | ( L << 8 )   ;
#endif
		*Abs_ADR++  = (Ushort) L  ;
	} while ( --NBr != 0  ) ;
#if							( FDM_WITH_PLX9050 == NO )
	/* -----------------------ecrire Message */
	/* acces 16bits*/
	{
		Ushort Tmp;
		int   nb_block , Flag ;
	  	FDM_MSG_T_DESC *src;
		nb_block    	= Message->Nr_Of_Blocks;
		src	    	= Message->Ptr_Block;
		buffer 	 	= (Ushort * ) src->Ptr_Data;

		NBr     	= (int)       src->Nr_Of_Bytes / 2 ;
		Flag	 	= 0 ;
		Tmp		= 0;
		for (;;) {
		    	if (NBr != 0 ) {
				do {
					L  =  *buffer++ ;
#if 			( FDM_WITH_LITTLE_ENDIAN == YES )
					L  =  ( L >> 8 ) | ( L << 8 )   ;
#endif
					*Abs_ADR++  = (Ushort) L  ;
				} while ( --NBr != 0  ) ;
			}
			if (( src->Nr_Of_Bytes & 1 ) != 0 ) {
				Tmp = *buffer;
#if 			( FDM_WITH_LITTLE_ENDIAN == YES )
				Tmp = Tmp << 8;
#else
				Tmp &= 0xff00 ;
#endif
				Flag |= 1;
			} ;
		    	src = src->Next_Block  ;
		    	nb_block--;
		    	if ( ( src == NULL )  || ( nb_block == 0 ) ) {
				if ( Flag ) *Abs_ADR = Tmp;
				break;
			}
			if ( src->Nr_Of_Bytes != 0 ) {
				if ( Flag ) {
					Uchar * Cbuffer;
					Flag	= 0 ;
		    			Cbuffer = src->Ptr_Data ;
			    		L  	= *Cbuffer ;
					L 	|= Tmp ;
					*Abs_ADR++  = (Ushort) L  ;
					NBr = src->Nr_Of_Bytes;
					NBr--;
					Cbuffer++;
					buffer = (Ushort*) Cbuffer;
				} else {
		    			buffer 	= (Ushort * )src->Ptr_Data ;
		    			NBr     = src->Nr_Of_Bytes / 2;
				}
			}
		}
	}
#else
	/* -----------------------ecrire Message en mode 32 bits*/
	{
	  	FDM_MSG_T_DESC 	*src;
                Ulong 		T , *Dest, * Source ;
		src	    	= Message->Ptr_Block;
		Source 	 	= (Ulong * ) src->Ptr_Data;
		Dest = (Ulong*) Abs_ADR ;
		NBr     	=  ( ( src->Nr_Of_Bytes & 3 ) == 0 ) ? 0 : 1;
		NBr     	+= ( src->Nr_Of_Bytes / 4 ) ;


		do {
			T = *Source++;
#if 						( FDM_WITH_NT == YES )
__asm mov eax , T;
__asm  xchg ah,al;
__asm  ror eax,16;
__asm  xchg ah,al;
__asm  ror eax,16;
__asm  mov T,eax ;
#else
	T = (( T << 8 ) & 0xff00ff00) | ((T >> 8) & 0x00ff00ff);
#endif

			*Dest++  = T  ;
		} while ( --NBr != 0  ) ;

	}
#endif

	{   /* -----------------------valider */
		int Cr_Fifo;

                OS_Enter_Region();
		NBr = BLOC_DE_DESCRIPTION->Desc_fifo & 0x3f ;

		if ( Ref->FDM_Internal_Use.Messagerie.tab_wait_cnf[Channel_Nr][NBr] != (FDM_MSG_TO_SEND*)0 ){
			OS_Leave_Region();
			FDM_Signal_Error ( Ref ,  _ILLEGAL_POINTER , 12) ;
   			return(1);
		}

		Cr_Fifo = STORE_FIFO_INT( &(Ref->FDM_Internal_Use.Messagerie.fifo_CNF_T[Channel_Nr] ),
     			(FIFO_INT_ELT_TYPE*) Message);
		if ( Cr_Fifo == 0 ){
	      		Ref->FDM_Internal_Use.Messagerie.tab_wait_cnf[Channel_Nr][NBr] = Message ;
		        Ref->FDM_Internal_Use.Messagerie.Cpt_canal_T[Channel_Nr] ++;
 		} else {
			OS_Leave_Region();
			FDM_Signal_Error ( Ref , _FIFO_ACCES_FAILED  , 12) ;
   			return(1);
		}
		BLOC_DE_DESCRIPTION->Indic =  MSG_BLOC_OCCUPE;
		OS_Leave_Region();

		/***********************************************/
		/********** FIP_Send_msg_aper   ****************/
		/***********************************************/
		if ( Channel_Nr == 0 ) {
			volatile Ushort   *FifoUser, *FifoFip ;
			int  tempo;
			Ushort Tmp ,CR;
			Dial_AE *PDial_AE;
			MAC__INIT_REG;

			MAC__WAIT_PLACE;
			MAC__ECRIRE_DEMANDE(SEND_MSG_APER_USER,0)

			OS_Leave_Region();
abortd:;
		}
	return ((Ushort) NBr ) ;
	}

#endif
}

/*###########################################################################*/
	/*MESSAGERIE RECEPTION*/
/*###########################################################################*/
/*********************************************/
/* lecture de l'entete  _FIP_RECEIVE_HEADER  */
/*********************************************/
#if (FDM_WITH_FREE_ACCESS==NO)
static Ushort _FIP_READ_HEADER_AND_DATA   ( FDM_REF * Ref , MSG_HEADER_Receive_Type * H , FDM_MSG_R_DESC  *destination )
{
	int   tempo;
	int VAR_STATE ;
	Uchar *dst ;
	FDM_ERROR_CODE CR ;
	Ushort  taille;
    	FIFO_Reg;
	Debut_procedure ;
	WAIT_Busy;
	/************ ecriture de la commande */
	ECRIRE_UCOM( CMDE_R_MSG );
	/* ici pour occuper utilement le micro */
	FIFO_port = FIFO;
	dst       = ( Uchar * ) H ;
        taille    = sizeof(MSG_HEADER_Receive_Type);
     	destination->Ptr_Data      = &destination->Data_Buffer[0];
      	destination->Next_Block    = NULL;
	/************ attente bit SV */
	WAIT_SV;
	_FIP__tres_petite_tempo ( Ref ) ;
	/************ test de valididite */
	VAR_STATE = LIRE(Var_State) ;
	if ((VAR_STATE & 0x80) != 0 ) {
                ECRIRE_UCOM(CMDE_CLOSE) ;
                Fin_procedure;
	}
	/********** lecture Header *******/
	BLOCKINBYTE(FIFO_port,dst,taille);
	taille	= ((Ushort)H->Length[0] <<8 ) | ((Ushort)H->Length[1]  );
        destination->Nr_Of_Bytes   = taille;
	/********** lecture Msg *******/
	{
		Uchar* Ptr_Data;
		Ptr_Data      = &destination->Data_Buffer[0];
		BLOCKINBYTE(FIFO_port,Ptr_Data,taille);
	}
	/************ compte rendu */
	CR_UState;
	/************ close */
        ECRIRE_UCOM(CMDE_CLOSE) ;
	Fin_procedure;
}
#endif

/********************************************************/
/*	procedure FIP  	:purge des cannaux		*/
/*                        _FIP_MSG_CHANNEL_FLUSHOUT	*/
/********************************************************/
/* si pas d'elements a purger  alors Buffer->nr_of_elem =0 */

static Ushort _FIP_MSG_CHANNEL_FLUSHOUT ( FDM_REF *Ref , int Knal, PURGE_type *FBuffer )
{
#if  (  FDM_WITH_FREE_ACCESS == NO)
	int 		tempo ;
	FDM_ERROR_CODE  CR ;
	int           VAR_STATE;
	FIFO_Reg;
	register  Uchar       *dst;
	register unsigned int taille;
        FBuffer->nr_of_elem =0;
	Debut_procedure ;
	WAIT_Busy;
	ECRIRE_Key( (Ushort) Knal );
	ECRIRE_UCOM( CMDE_PURGE_MSG );
	/* ici pour occuper utilement le micro */
	FIFO_port = FIFO;

	dst = (Uchar*) &FBuffer->Elem[0];
	/************ attente bit SV */
	WAIT_SV;
	_FIP__tres_petite_tempo ( Ref ) ;
	/************ test de valididite */
	VAR_STATE = LIRE(Var_State) ;
	if ((VAR_STATE & 0x80) != 0 ) {
		/*si file vide var_state =8ah , ou fifo inconnue :impossible*/
                ECRIRE_UCOM(CMDE_CLOSE) ;
               	OS_fdm_sm_v;
               	return ( 0 ) ;
	}
	taille = (unsigned int ) LIRE(Var_Size) ;
	FBuffer->nr_of_elem = (Ushort) (taille >> 1);
	if ( taille != 0 ) {
	   /************ boucle de lecture */
	   BLOCKINBYTE(FIFO_port,dst,taille);
        }
	/************ close */
	CR_UState ;
	ECRIRE_UCOM(CMDE_CLOSE) ;
	Fin_procedure;
#else
	volatile Ushort   *FifoUser, *FifoFip  ;
	Dial_AE *PDial_AE;
	int  tempo;
	Ushort Tmp;
	Ushort CR ;
        FBuffer->nr_of_elem =0;
	MAC__INIT_REG;
	MAC__WAIT_PLACE;
	MAC__ECRIRE_DEMANDE(PURGE_MSG_USER,Knal);
	MAC__WAIT_LECTURE_PRETE;
	FBuffer->nr_of_elem =  PDial_AE->Parametre  ;
	OS_Leave_Region();
	if ( FBuffer->nr_of_elem != 0) {
		int i;
		volatile Ushort *Abs_ADR  ;
		Ushort *buffer ;
		buffer = (Ushort *) &FBuffer->Elem[0];
		Abs_ADR      = Ref->H.FREE_ACCES_ADDRESS;
		Abs_ADR     += MSG_TAB_PURGE ;
		for ( i = 0 ; i < FBuffer->nr_of_elem ; i++ ){
			*buffer++ = *Abs_ADR++ ;
		}
#if 			(  FDM_WITH_LITTLE_ENDIAN == YES )
		SWAB_SHORT(
			(char*) &FBuffer->Elem[0]  ,
			(char*) &FBuffer->Elem[0]  ,
			2*FBuffer->nr_of_elem );
#endif
	}

	return 0;
abortd:
	return CR ;
abort:
	OS_Leave_Region();
	return CR ;
#endif
}

/********************************************************/
/*	procedure FIP  	:delete LSAP       		*/
/*                        _FIP_LSAP_delete       	*/
/********************************************************/
static void _FIP_LSAP_delete ( FDM_REF *Ref  , Ulong Local_DLL_Address )
{
 	DESC_TRAME_TYPE *DT;
        Ulong 		Ident;
	Uchar		segment;
	int		lsap_a_deleter =0;
       segment =(Uchar)Local_DLL_Address ;
       if ( segment ==0xff) return;
       if  ( ( segment != 0 ) && ( segment != Ref->H.MySegment  )) {
		Ushort G = 0;
		 Ref->FDM_Internal_Use.Messagerie.TAB_SEGMENT[segment]= G;
  		_FIP_WRITE_WORD( Ref , MSG_TAB_SEGMENT +segment , G);
		return;
	}
       Ident   = Local_DLL_Address >>8;
	OS_fdm_sm_p_bd( Ref ) ;
	DT = (DESC_TRAME_TYPE *)  _DICHO_SCAN_OBJECT(
		&Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE , Ident );
	if ( DT	!= (DESC_TRAME_TYPE *) 0 ) {
		if (segment ==0) {
			    /* supression du segment 0 */
	 	    if (DT->position & _position_SAPx ) {
		    	DT->position 		&= ~_position_SAP;
		    }else{
			lsap_a_deleter =1;
		    }
		}else{
		    /* supression du segment le mien */
		    if (DT->position & _position_SAP ) {
		    	DT->position 		&= ~_position_SAPx;
		    }else{
			lsap_a_deleter =1;
		    }
		}
		if (lsap_a_deleter==1){
	 		/* supression de DT que si les 2 segments 0 et  le mien sont supprimes */
		    DT->Type_Descriptor 	&= ~MSGr_autorisee;
		    DT->position 		&= ~( _position_SAP    | _position_SAPx |
						      _position_MSG_I1 | _position_MSG_I2);
		    SUP_DTRAME ( Ref , ( Ushort )Ident ) ;
		    FDM_STOP_DB_MOD ( Ref ); /* on dnload */
	 	 }
	}
        OS_fdm_sm_v_bd( Ref ) ;
}

/********************************************************/
/*           fdm_msg_rec_fifo_empty 			*/
/********************************************************/
void *_DICHO_SCAN_OBJECT_INTERNE ( _TABLE **ptr , long val )
{
/*
fonction:
    recherche dichotomique dans une table
    En fonction d'un numero d'objet
    retourne l'adresse de l'objet
*/
	int i,j;
	_TABLE *x;

	x = *ptr;
	i = j = (x->nombre)>>1;
	do {
		j = j >> 1;
		if ((x->k[i].objet_num & 0xffffff )== val ) return ( x->k[i].ptr );
		i +=  ((x->k[i].objet_num < val ) ? j : - j);
	} while (j != 0 ) ;
	if (x->k[0].objet_num == val ) return ( x->k[0].ptr );
	return( nil_ele );
	/* pas trouve ! */
}

void  fdm_msg_rec_fifo_empty ( FDM_REF *Fip )
#if (FDM_WITH_FREE_ACCESS == NO)
{
	MSG_HEADER_Receive_Type	 Hr ;
	Ushort			 Cr ;
	Ulong			 Remote_DLL_Address , Local_DLL_Address , Local_DLL_Add ;
	FDM_MESSAGING_REF        *Ref_mdl;
	_TABLE			 *tab_Remote_DLL;
	FDM_MSG_RECEIVED        *Block ;
	FDM_MSG_R_DESC 	        *Datas ;
	FDM_MSG_R_DESC 	        *Datas_interne ;
	Ulong		        remote_configured =0;
  for(;;){
     OS_Enter_Region();
     if( Fip->FDM_Internal_Use.Messagerie.Cpt_Receive == 0 ) {
      	OS_Leave_Region();
      	return;
     }
     Fip->FDM_Internal_Use.Messagerie.Cpt_Receive-- ;
     OS_Leave_Region();
	Datas = NULL;
	Block = NULL;
	Datas_interne = (FDM_MSG_R_DESC*) _ALLOCATE_MEMORY (Fip->FDM_Internal_Use.Messagerie.Reveive_pool);
	if ( Datas_interne == (FDM_MSG_R_DESC*) 0 ) {
        	FDM_Signal_Error ( Fip  , _INTEGRITY_DATA_BASE_FAULT, 0  ) ;
		return ;
	}

	Cr =  _FIP_READ_HEADER_AND_DATA ( Fip , &Hr , Datas_interne );

	if ( Cr != 0 ) 	goto poubelle1 ;
     	Local_DLL_Add = Local_DLL_Address 	= ((Ulong) Hr.dest[0] << 16 ) |
			  ((Ulong) Hr.dest[1] << 8  ) |
        		  ((Ulong) Hr.dest[2]       ) ;
        if   (( Hr.dest[2] != 0 ) && ( Hr.dest[2] != 0xFF )) {
        	if   ( Hr.dest[2] != Fip->H.MySegment ) {
			/* Bridge */
			Local_DLL_Address = 0x40000000L | Hr.dest[2] ;
        	}
       	}
     	Remote_DLL_Address = ((Ulong) Hr.sour[0] << 16 ) |
     			   ((Ulong) Hr.sour[1] << 8 ) |
     			   ((Ulong) Hr.sour[2]      ) ;

     	/* recherche du Ref_mdl associe au couple Source-Destination */
     	tab_Remote_DLL =  (_TABLE*)_DICHO_SCAN_OBJECT (
     			&(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local),
      			 Local_DLL_Address);
     	if (tab_Remote_DLL == NULL) goto poubelle1 ;
     	/* recherche destination */
     	Ref_mdl =(FDM_MESSAGING_REF*)_DICHO_SCAN_OBJECT_INTERNE
   		(&tab_Remote_DLL,Remote_DLL_Address);
     	if (Ref_mdl == NULL){
   	/* - Cas ou le remote non configure ,
   	   on regarde si Ref_mdl initialise sur ce cas */
         	Ref_mdl =(FDM_MESSAGING_REF*)_DICHO_SCAN_OBJECT
   				(&tab_Remote_DLL,0x40000000L);
         	if (Ref_mdl == NULL)	 goto poubelle1 ;
         	remote_configured = 0x80000000L ; /* positionne bit 31 */
      	}
  	/* allocation des ressources */
      	Block =(FDM_MSG_RECEIVED*) _ALLOCATE_MEMORY(Ref_mdl->Receive_infos.Private.GM1);
      	if (Block ==0 ) {
        	FDM_Signal_Warning ( Fip  , _MISSING_MEMORY_DELETED_MESSAGE , Local_DLL_Address  ) ;
		goto poubelle1 ;
	}
      	Datas = (FDM_MSG_R_DESC *)_ALLOCATE_MEMORY(Ref_mdl->Receive_infos.Private.GM2);
      	if (Datas ==0 ) {
        	FDM_Signal_Warning ( Fip  , _MISSING_MEMORY_DELETED_MESSAGE , Local_DLL_Address  ) ;
		goto poubelle1 ;
	}
      	_GDM__SWITCH_POOL ( ( User_GDM *) Datas  , ( User_GDM *) Datas_interne  );
        _FREE_MEMORY (( User_GDM *) Datas );
       /* c'est ok */
      	Block->Ptr_Block            = Datas_interne;
      	Block->Nr_Of_Blocks         = 1;
      	Block->Remote_DLL_Address   = Remote_DLL_Address |remote_configured;
        Block->Local_DLL_Address   = Local_DLL_Add ;
      	Block->User_Qid             = Ref_mdl->Receive_infos.User_Qid;
      	Block->User_Ctxt            = Ref_mdl->Receive_infos.User_Ctxt;
      	Block->Ref 	   	    = Ref_mdl ;

    	/* appel procedure reception message de l'user */
      	Ref_mdl->Receive_infos.User_Msg_Rec_Proc ( (struct _FDM_MESSAGING_REF*) Ref_mdl, Block );

      continue  ;
poubelle1 :
	if (  Datas_interne != NULL )
        	_FREE_MEMORY (( User_GDM *) Datas_interne );
	if ( Block != NULL )
           _FREE_MEMORY (( User_GDM *) Block );
	if ( Datas != NULL )
	   _FREE_MEMORY (( User_GDM *) Datas );
 }/* endfor*/

/***************************************************************/
/************************** ACCES libre ************************/
/***************** Messagerie Reception ************************/
/***************************************************************/
#else
{
	MSG_HEADER_Receive_Type	 Hr ;
	Ulong			 Remote_DLL_Address , Local_DLL_Address , Local_DLL_Add ;
	Ushort			 cdo;
	FDM_MESSAGING_REF        *Ref_mdl;
	_TABLE			 *tab_Remote_DLL;
	DESC_FIFO_MSG_REC * BLOC_DE_DESCRIPTION;
	volatile Ushort   *buffer, *Abs_ADR ;
	FDM_MSG_RECEIVED        *Block = NULL ;
	FDM_MSG_R_DESC 	        *Datas = NULL;
	Ulong		        remote_configured =0;
	if ( Fip->FDM_Internal_Use.Messagerie.Cpt_Receive == 0 ) return;
  for(;;){
   {
	volatile Ushort * FiFo_Adr ;
	Ulong Ad ;
	int NBr , L ;
	Ushort *dst ;

	/* -----------------------lecture descripteur */
	buffer  = Fip->H.FREE_ACCES_ADDRESS;
	buffer += 0x218f ;
	FiFo_Adr = Fip->H.FREE_ACCES_ADDRESS;
	FiFo_Adr += Fip->FDM_Internal_Use.fipgen.BASE_1;
	FiFo_Adr += *buffer;
	BLOC_DE_DESCRIPTION = (DESC_FIFO_MSG_REC *) FiFo_Adr   ;
	if ( BLOC_DE_DESCRIPTION->Indic == 0 ) {
		Fip->FDM_Internal_Use.Messagerie.Cpt_Receive = 0;
		return ;
	}
	/* -----------------------lecture Header */

	Ad   = BLOC_DE_DESCRIPTION->Adr_data_pforts;
	Ad <<= 10;
	Ad  |= BLOC_DE_DESCRIPTION->Adr_data_pfaibles;
	Abs_ADR = Fip->H.FREE_ACCES_ADDRESS;
	Abs_ADR += Ad ;
	/* -----------------------lire  */
	dst = (Ushort * ) &Hr;
	NBr = 5;
	do {
		L =  *Abs_ADR++ ;
#if 			( FDM_WITH_LITTLE_ENDIAN == YES )
		L  =  ( L >> 8 ) | ( L << 8 )   ;
#endif
		*dst++  = (Ushort) L  ;
	} while ( --NBr != 0  ) ;
    }
     Local_DLL_Add = Local_DLL_Address 	= ((Ulong) Hr.dest[0] << 16 ) |
			  ((Ulong) Hr.dest[1] << 8  ) |
        		  ((Ulong) Hr.dest[2]       ) ;
     if   ( Hr.dest[2] != 0 ) {
        	if   ( Hr.dest[2] == Fip->H.MySegment ) {
			/* Bridge */
			Local_DLL_Address = 0x40000000L | Hr.dest[2] ;
        	}
     }
     Remote_DLL_Address = ((Ulong) Hr.sour[0] << 16 ) |
     			   ((Ulong) Hr.sour[1] << 8 ) |
     			   ((Ulong) Hr.sour[2]      ) ;
     cdo 		= ((Ushort)Hr.Length[0] <<8 ) |
     			  ((Ushort)Hr.Length[1]     );
     /* recherche du Ref_mdl associe au couple Source-Destination */
     tab_Remote_DLL =  (_TABLE*)_DICHO_SCAN_OBJECT(
     			&(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local),
      			 Local_DLL_Address);
     if (tab_Remote_DLL ==NULL)	goto poubelle ;
     /* recherche destination */
     Ref_mdl =(FDM_MESSAGING_REF*)_DICHO_SCAN_OBJECT
   		(&tab_Remote_DLL,Remote_DLL_Address);
     if (Ref_mdl ==NULL){
   	/* cas ou le remote non configure ,
   		on regarde si Ref_mdl initialise sur ce cas*/
         Ref_mdl =(FDM_MESSAGING_REF*)_DICHO_SCAN_OBJECT
   				(&tab_Remote_DLL,0x40000000L);
         if (Ref_mdl ==NULL)	 goto poubelle ;
         remote_configured = 0x80000000L ; /* positionne bit 31 */
      }
      Datas = NULL ;
  	/* allocation des ressources */
      Block =(FDM_MSG_RECEIVED*) _ALLOCATE_MEMORY(Ref_mdl->Receive_infos.Private.GM1);
      if (Block ==0 ) goto poubelle1 ;
      Datas = (FDM_MSG_R_DESC *)_ALLOCATE_MEMORY(Ref_mdl->Receive_infos.Private.GM2);
      if (Datas ==0 ) goto poubelle1 ;
       /* c'est ok */
      Block->Ptr_Block            = Datas;
      Block->Nr_Of_Blocks         = 1;
      Block->Remote_DLL_Address   = Remote_DLL_Address |remote_configured;
      Block->Local_DLL_Address   = Local_DLL_Add ;
      Block->User_Qid             = Ref_mdl->Receive_infos.User_Qid;
      Block->User_Ctxt            = Ref_mdl->Receive_infos.User_Ctxt;
      Datas->Ptr_Data      = &Datas->Data_Buffer[0];
      Datas->Nr_Of_Bytes   = cdo;
      Datas->Next_Block    = NULL;
      Block->Ref 	   = Ref_mdl ;
#if						( FDM_WITH_PLX9050 == NO )
/*lecture du message 16 bits*/
    {
	register int NBr , L ;
	register Ushort *dst ;
	dst = ( Ushort * ) Datas->Ptr_Data;
	NBr = ( Datas->Nr_Of_Bytes/2 );
 	NBr += (( Datas->Nr_Of_Bytes & 1 ) != 0 ) ? 1 : 0 ;
	do {
		L = *Abs_ADR++ ;
#if 			( FDM_WITH_LITTLE_ENDIAN == YES )
		L  =  ( L >> 8 ) | ( L << 8 )   ;
#endif
		*dst++  = (Ushort) L  ;
	} while ( --NBr != 0  ) ;

    }
#else
/*lecture du message 32 bits*/
    {
	int NBr  ;
	Ulong *dst ,*Src , T ;
	dst = ( Ulong * ) Datas->Ptr_Data;
	NBr = ( Datas->Nr_Of_Bytes/4 );
 	NBr += (( Datas->Nr_Of_Bytes & 3 ) != 0 ) ? 1 : 0 ;
 	Src = (Ulong*) Abs_ADR ;
	do {
		T = *Src++ ;
#if 						( FDM_WITH_NT == YES )
__asm  mov eax , T;
__asm  xchg ah,al;
__asm  ror eax,16;
__asm  xchg ah,al;
__asm  ror eax,16;
__asm  mov T,eax ;
#else
	T = (( T << 8 ) & 0xff00ff00) | ((T >> 8) & 0x00ff00ff);
#endif

		*dst++  = T  ;
	} while ( --NBr != 0  ) ;

    }
#endif
    	/* appel procedure reception message de l'user */

    Ref_mdl->Receive_infos.User_Msg_Rec_Proc ( (struct _FDM_MESSAGING_REF*) Ref_mdl, Block );
    BLOC_DE_DESCRIPTION->Indic = 0;
    buffer  = Fip->H.FREE_ACCES_ADDRESS;
    buffer += 0x218f ;
    *buffer = BLOC_DE_DESCRIPTION->Adr_suivant;
    continue  ;

poubelle1 :
	if ( Block != NULL )
           _FREE_MEMORY (( User_GDM *) Block );
	if ( Datas != NULL )
	   _FREE_MEMORY (( User_GDM *) Datas );
        FDM_Signal_Warning ( Fip  , _MISSING_MEMORY_DELETED_MESSAGE ,
		Local_DLL_Address   ) ;
poubelle :
    BLOC_DE_DESCRIPTION->Indic = 0;
    buffer  = Fip->H.FREE_ACCES_ADDRESS;
    buffer += 0x218f ;
    *buffer = BLOC_DE_DESCRIPTION->Adr_suivant;

 }/* endfor*/
#endif
}


static int SEND_MSG_RQ (INTEG_FILE_TYPE *) ;  /* vide les 9 files FIFO_MSG_RQ */
static int SEND_MSG_CNF(INTEG_FILE_TYPE *);   /* vide la file de confirmation */
static int vide_fifo_MSG_TO(INTEG_FILE_TYPE *);
static void vidage_cnf_msg_maj(FIFO_INT_TYPE *);
static void _FIP_Appli_MSG_T(SOFT_REFERENCE_TYPE *);

/************************************************************************/
/*       procedures d init du LAI pour emission				*/
/*		FDM_INITIALIZE_TASK_MSG_T				*/
/************************************************************************/
void FDM_INITIALIZE_TASK_MSG_T ( FDM_REF *Ref,
			     void (*USER_FUNCT) ( struct _FDM_REF* ) )
{
	SOFT_REFERENCE_TYPE 	*Tache_MSG_S_ref ;
    	SOFT_INTERFACE_TYPE 	SIT;
	int i;
	CREATE_TASK_ENVIRONNEMENT  ( &Ref->FDM_Internal_Use.Task_Ref_MSG_T ,
			            (void (*)(void*)) USER_FUNCT,(void*)Ref );
    	SIT.Nr_Of_External_In_Files 	= 11;
    	SIT.Software_Loop_Fct           = _FIP_Appli_MSG_T ;

	Tache_MSG_S_ref = &Ref->FDM_Internal_Use.Soft_Ref_MSG_T;
    	 ATTACH_SOFT_TO_TASK( &Ref->FDM_Internal_Use.Task_Ref_MSG_T ,
			  &SIT,
			  Tache_MSG_S_ref,
			  Ref->FDM_Internal_Use.Files_MSG_T);
    /* initialisation des Timers de T.O de surveillance canaux */
    for ( i = 0 ; i < 9 ; i++ ) {
	Ref->FDM_Internal_Use.Messagerie.Timer_T[i].T.T_init   = Ref->S.Time_Out_Msg_Ticks; ;
	Ref->FDM_Internal_Use.Messagerie.Timer_T[i].T._.Type_out  = 1;
	Ref->FDM_Internal_Use.Messagerie.Timer_T[i].T.File =
	 	 Tache_MSG_S_ref->Array_Of_In_Files[ 10 ] ;
	Ref->FDM_Internal_Use.Messagerie.Timer_T[i].Knal = i;/* num canal */
	Ref->FDM_Internal_Use.Messagerie.Timer_T[i].Fip = Ref;/* num canal */
     }

}

/************************************************************************
     LOOP de  vidage des files emission -conf
            void  fdm_appli_msg_t (SOFT_REFERENCE_TYPE *Ref)
   entree :
   	    SOFT_REFERENCE_TYPE *Ref
 ************************************************************************/
static void _FIP_Appli_MSG_T(SOFT_REFERENCE_TYPE *Ref)
{
   int j;
   INTEG_FILE_TYPE  *FIFO_MSG_CNF;
   INTEG_FILE_TYPE  *FIFO_MSG_TO;
   INTEG_FILE_TYPE  *FIFO_MSG_RQ;
  OS_Leave_Region();


  for (;;){
      /* vide les 9 files FIFO_MSG_RQ */
      for (j=0 ; j<9 ;j++){
          FIFO_MSG_RQ = Ref->Array_Of_In_Files[j];
          if (FIFO_MSG_RQ->Nbr_Of_Elt !=0)  {
		if ( SEND_MSG_RQ(FIFO_MSG_RQ) == 2) {
		        OS_Enter_Region();
      			Ref->Software_State_Word &= ~(1<<j) ;
	     		OS_Leave_Region();
		}
	   }
       }
       /* vide la file de confirmation */
       FIFO_MSG_CNF = Ref->Array_Of_In_Files[9];
       SEND_MSG_CNF(FIFO_MSG_CNF);

       /* vide la file des Time-Out */
       FIFO_MSG_TO  = Ref->Array_Of_In_Files[10];
       vide_fifo_MSG_TO(FIFO_MSG_TO);
       OS_Enter_Region();
       if ( Ref->Software_State_Word == 0 )  {
		return;
       }
       OS_Leave_Region();
   }
 }


/***********************************************************************
     vide une file de message  ->SEND_MSG_RQ
          entree :
   	    FIFO_MSG_RQ  *fifo_rqx   reference dela fifo RQ a vider X =0 a 8
    sortie = 0
  		 s'il n y a plus de ressources disponibles
 ************************************************************************/
static int SEND_MSG_RQ (INTEG_FILE_TYPE* fifo_rqx)
{
	FDM_MSG_TO_SEND  	*msg_t ;
	FDM_MESSAGING_REF	*Ref_mdl;
	FDM_REF  		*Fip;
	Ushort			Channel_Nr;
	MSG_HEADER_Send_Type    msg_header;
	Ushort			lg ;
	enum  _FDM_MSG_USER_ERROR_LIST  cr_user_soft_report ;
	int    i1;

   /* consultation du 1er element Fifo */
   msg_t = (FDM_MSG_TO_SEND *) CONSULT_FIFO_INTEG((INTEG_FILE_TYPE*)fifo_rqx );
   if (msg_t == (FDM_MSG_TO_SEND*) 0) return (0); /* file vide */

   Ref_mdl    = ( FDM_MESSAGING_REF*) msg_t->Private.Ref_mdl;
   Channel_Nr = msg_t->Private.Channel_Nr;
   Fip	      = Ref_mdl->Fip;
   /* test si cannal configure ( verifie si pas eu de delete canal )*/
   if (Fip->FDM_Internal_Use.Messagerie.CANAL[Channel_Nr]==0xffffffffL){
	 cr_user_soft_report  = _FDM_MSG_REPORT_CHANNEL_NOT_ALLOWED;
         goto rejet_a;
         /*---------------------------------------------------------*/
      }
   /* Init du champ prive du  message  ====> a voir   */
  msg_t->Private.evt_addit_info       = 0;
  msg_t->Private.Service_Report       = _MSG_REPORT_NOT_INIT;
    {
      /* test validite  du message -
         ==========================
   			- test si Ptr_Block et Nr_Of_Blocks non a 0
        		- test nbre d 'octets , si < 251 et non a 0 */
        FDM_MSG_T_DESC  *ptr_block;

     if (( msg_t->Ptr_Block == NULL ) || (msg_t->Nr_Of_Blocks == 0)){
	 cr_user_soft_report  = _FDM_MSG_REPORT_ERR_MSG_INFOS;
         goto rejet_a;
         /*---------------------------------------------------------*/
      }
   	/* calcul du nombre total d'octets */
      ptr_block= msg_t->Ptr_Block;
      for ( i1 =0 , lg= 0; i1 < msg_t->Nr_Of_Blocks ;i1 ++){
          lg  +=ptr_block->Nr_Of_Bytes;
          ptr_block =ptr_block->Next_Block;
      }
      if ( (lg > MAX_DATA_MSG ) || (lg  == 0 ) ){
           cr_user_soft_report  = _FDM_MSG_REPORT_ERR_LG_MSG;
           goto rejet_a;
         /*---------------------------------------------------------*/
      }
  }

   /* test si messagerie autorisee */
   /*=============================================*/
  if (Fip->FDM_Internal_Use.fipgen.image == IMAGE_1 ){
      if ((Ref_mdl->Type.Position & _MSG_IMAGE_1) == 0){
  		/* si un FDM non AUTORISE en messagerie sur image 1-->
   		  positionne service report -> poubelle msg */
   	  cr_user_soft_report  = _FDM_MSG_REPORT_MSG_NOT_ALLOWED ;
    	  goto rejet_a;
         /*---------------------------------------------------------*/
       }
   }else {
      if ((Ref_mdl->Type.Position & _MSG_IMAGE_2) == 0 ){
  		/* si un FDM non AUTORISE en messagerie sur image 2-->
   		  positionne service report -> poubelle msg */
   	  cr_user_soft_report  = _FDM_MSG_REPORT_MSG_NOT_ALLOWED ;
    	  goto rejet_a;
         /*---------------------------------------------------------*/
   	}
   }

   /* test si ressouces disponibles  */
   /*=============================================*/
    /* test si ressource du canal <  ressource max affectee au canal */
   if (Fip->FDM_Internal_Use.Messagerie.Cpt_canal_T[Channel_Nr]  >=
          Fip->S.Nr_Of_Tx_Buffer[Channel_Nr]-1){
          return (2); /*il n y a plus de ressources disponibles pour le canal*/
         /*-----------------------------------------------------*/
  }





  msg_t = (FDM_MSG_TO_SEND *) GET_FROM_FIFO_INTEG((INTEG_FILE_TYPE*)fifo_rqx );
  {
     /* preparation du message
        		- test si lsap source et lsap dest ok
        		- positionner type acquit , suivant lsap */
    Ulong		   Remote_DLL_Address;

    if (Ref_mdl->Remote_DLL_Address & 0x80000000L ){
    	/*l'adresse du destinataire du message est dans le message */
    	  Remote_DLL_Address = msg_t->Remote_DLL_Address;
    } else {
    	   Remote_DLL_Address = Ref_mdl->Remote_DLL_Address ;
    }
    msg_header.Destination[0]= (Uchar) (Remote_DLL_Address>>16);
    msg_header.Destination[1]= (Uchar) (Remote_DLL_Address>>8);
    msg_header.Destination[2]= (Uchar) (Remote_DLL_Address );
    if (Ref_mdl->Local_DLL_Address & 0x40000000L ){
	    msg_header.Source[0]= (Uchar) (msg_t->Local_DLL_Address >>16);
	    msg_header.Source[1]= (Uchar) (msg_t->Local_DLL_Address >>8);
	    msg_header.Source[2]= (Uchar) (msg_t->Local_DLL_Address );
    } else {
	    msg_header.Source[0]= (Uchar) (Ref_mdl->Local_DLL_Address >>16);
	    msg_header.Source[1]= (Uchar) (Ref_mdl->Local_DLL_Address >>8);
	    msg_header.Source[2]= (Uchar) (Ref_mdl->Local_DLL_Address );
    }
    /* positionne mode ack pour non diffusion , noack pour diffusion */
    msg_header.Mode[0] =  0;
    msg_header.Mode[1] = Ref_mdl->Send_infos.User_Msg_Ack_Type (Fip,Remote_DLL_Address ) ;
    lg +=6 ;     /* entete trame sans mode et lg */
    msg_header.Length[0]= (Uchar)(lg>>8);
    msg_header.Length[1]= (Uchar)(lg);
 }

   /* envoi du message dans FIP */
 {
     Ushort  	no_bloc_fifo;
     TEMPO_DEF *T;

      no_bloc_fifo= _FIP_WRITE_MSG ( Fip, &msg_header , msg_t, Channel_Nr );

      if (no_bloc_fifo & 0x8000){
      	  /* pb --> message non ecrit dans Fip */
          cr_user_soft_report  = _FDM_MSG_REPORT_INTERNAL_ERROR ;
           goto rejet;
          /*---------------------------------------------------------*/
      }
      T =&(Fip->FDM_Internal_Use.Messagerie.Timer_T[Channel_Nr].T);
      _TIMER_RETRIG_ANYWAY ( T );

 }
 return(1);

 rejet_a:
      msg_t =(FDM_MSG_TO_SEND *)GET_FROM_FIFO_INTEG((INTEG_FILE_TYPE*)fifo_rqx);
 rejet :
      msg_t->Service_Report.Valid                = _FDM_MSG_USER_ERROR;
      msg_t->Service_Report.msg_user_soft_report = cr_user_soft_report;
      Ref_mdl->Send_infos.User_Msg_Ack(Ref_mdl,msg_t);
	  Ref_mdl->Send_infos.Cpt_Occup--;
      return(1);
      /*---------------------------------------------------------*/
 }


/***********************************************************************

  1) ANALYSE de l'element :
     - Suivant l'EVT , on sait :sur quel canal on a l'acquit
     		       on retrouve le message dli_t dans la table
     		       tab_wait_cnf du canal
  2) - il faut  mettre a jour du private.service_report :
	bit D0 =1 indique acquit recu
	bit d1 = ack+ ou non en fonction de l'EVT
  3) tester si on est le 1er element de la file FIFO_MSG_CNF
   	Si oui : vide tous les elements contigus acquites de la fifo
   		--> appel procedure conf User
   		-----------------------
Nota :
dans chaque service report interne:
		analyse si ack+ ou no --> si ack+  ; D1 =1
pour service report final :
si defaut : defaut dans d15 d14 d12  !!!!!! MODIF / FDM_V3
-----------------------------------------------------------------------
     sortie = 0 s il n y a plus d'element  dans la file
************************************************************************/

static int SEND_MSG_CNF(INTEG_FILE_TYPE *FIFO_MSG_CNF)
{
	    FDM_REF		  *Ref;
    	    FDM_MESSAGING_REF     *Ref_mdl;
            FDM_PRIVATE_DLI_T     *elem_fifo;
            FDM_MSG_TO_SEND	  *msg_t;
            Ushort 		   Channel_Nr ;
            Uchar		   no_bloc_fifo;
            int		          i;
	    FIFO_INT_TYPE	  *FIFO_CNF_T;

  /* depile 1 element de la file */
  elem_fifo = (FDM_PRIVATE_DLI_T *)GET_FROM_FIFO_INTEG (FIFO_MSG_CNF);
  if ( elem_fifo ==  (FDM_PRIVATE_DLI_T *)0) return (0); /* file vide */

  Ref_mdl=  elem_fifo->Ref_mdl;
  Ref    =  Ref_mdl->Fip;

 {	/* recherche du message- Analyse de l'acquit -
           positionne private service_report */
     Ushort  		  ack_recu;
    Channel_Nr   =  ( elem_fifo->evt_addit_info &  0x0f00) >>8;
    ack_recu     =  ( elem_fifo->evt_addit_info &  0xf000);
    no_bloc_fifo = (Uchar)(elem_fifo->evt_addit_info);
    msg_t = elem_fifo->User_Msg;

    switch(ack_recu >>12){
      case MSG_SDN_ACK:		/* SDN */
      case MSG_ACK_POS:		/* ack+ sans retry  SDA */
      case MSG_ACK_POS_REP: 	/* ack+ apres retry SDA  */
     	  i= _MSG_REPORT_ACK_P;
      break;
      default:
   	  i=0;
      break;
     }
    /* dans Service_Report du FDM concerne :
      positionne D0 =1 pour ack recu et D1 =1 pour ACK+ ou D1 reste 0
      bits D12 a D15 : ack  */
     msg_t->Private.Service_Report = ack_recu |_MSG_REPORT_RECEIVED | i;
   }
   /*vidage eventuel de la fifo interne des confirmation du canal considere */
    FIFO_CNF_T = &(Ref->FDM_Internal_Use.Messagerie.fifo_CNF_T[Channel_Nr]);
    vidage_cnf_msg_maj(FIFO_CNF_T);
    return (1);
}


/***********************************************************************
    vide une file des timout messagerie emission  (quelque soit le FDM)
    entree :INTEG_FILE_TYPE * pointeur sur 1 element de la FIFO
  				element de type TEMPO_DEF_C
    sortie = 0	, s'il n y a plus d elements disponibles
 ************************************************************************/
static int vide_fifo_MSG_TO(INTEG_FILE_TYPE *FIFO_MSG_TO)
{
  TEMPO_DEF_C	*T;	     /* T->Fip = reference FDM_REF
  				T->Knal=no du canal time out */
  PURGE_type     P_Elem;
  FIFO_INT_TYPE  *FIFO_CNF_T ;
  FDM_REF	    	 *Ref;
  int	i;

   /* depile 1 element de la file */
   T =  ( TEMPO_DEF_C *) GET_FROM_FIFO_INTEG ( FIFO_MSG_TO  );
   if ( T ==  (TEMPO_DEF_C *)0)   return( 0 );/* file vide */
   Ref =T->Fip;
   T->T._.In_File =0;
      /* mise a jour des elements  a purger */
   _FIP_MSG_CHANNEL_FLUSHOUT(  Ref , T->Knal , &P_Elem );
   if (P_Elem.nr_of_elem != 0 )	{
    	FDM_MSG_TO_SEND	 *msg_t;
    	Uchar   	         no_bloc_fifo;
  	for (i=0 ; i < (int) P_Elem.nr_of_elem; i++) {

      		no_bloc_fifo = P_Elem.Elem[i].Num_bloc;
       		msg_t = Ref->FDM_Internal_Use.Messagerie.tab_wait_cnf[T->Knal][no_bloc_fifo];
	        Ref->FDM_Internal_Use.Messagerie.tab_wait_cnf[T->Knal][no_bloc_fifo] = (FDM_MSG_TO_SEND*)0;
                if ( msg_t == ( FDM_MSG_TO_SEND*) 0 ){
			 FDM_Signal_Error ( Ref ,  _ILLEGAL_POINTER , 0) ;
   			 return(1);
		}
       		/* decrement du compteur de ressources du canal */
		  OS_Enter_Region() ;
       		Ref->FDM_Internal_Use.Messagerie.Cpt_canal_T[T->Knal]--;
                  OS_Leave_Region() ;
       		msg_t->Private.Service_Report = 	_MSG_REPORT_RECEIVED | _MSG_REPORT_TO ;
   	}
    }
   /*vidage eventuel de la fifo interne des confirmations du canal considere */
    FIFO_CNF_T =&(Ref->FDM_Internal_Use.Messagerie.fifo_CNF_T[ T->Knal]);
    vidage_cnf_msg_maj(FIFO_CNF_T);
    return(1);
}

/***********************************************************************
 vidage_cnf_msg_maj
   	 ---> il faut vider les messages dans l ordre ou ils ont ete emis
   	 On consulte donc la fifo , element par element
   	 Si le message est a jour : service_report positionne alors
   	   on extrait le message
   	   on envoie la conf a l'utilisateur
 entree : FIFO_CNF_T  a vider (fifo interne , depend du canal et du driver )
 ************************************************************************/
static void vidage_cnf_msg_maj(FIFO_INT_TYPE *FIFO_CNF_T)
{
  FDM_MSG_TO_SEND       *msg_t;
  FDM_MESSAGING_REF	*Ref_mdl;

 while ( FIFO_CNF_T->Nbr_Of_Elt !=0){
    msg_t =(FDM_MSG_TO_SEND *)FIFO_CNF_T->Head_Ptr;
    if ( msg_t ==  (FDM_MSG_TO_SEND *)0)
       /*bizarre */
       break;
      /*--------------------------*/
    if (msg_t->Private.Service_Report == _MSG_REPORT_NOT_INIT) 	break;
    else {
    	/*on extrait de la file ->  appel procedure user */
        msg_t = (FDM_MSG_TO_SEND*)GET_FIFO_INT(FIFO_CNF_T);
    	/* maj Service_Report du msg */
       if (msg_t->Private.Service_Report & _MSG_REPORT_ACK_P){
       	    /* ack+  --> service report OK  */
      	       msg_t->Service_Report.Valid  = _FDM_MSG_SEND_OK;
       }else {
    	     msg_t->Service_Report.Valid  = _FDM_DATA_LINK_ERROR;
    	    if (msg_t->Private.Service_Report & _MSG_REPORT_TO) {
    	           msg_t->Service_Report.Way =_FDM_TIME_OUT ;
    	    }else{
    	    	switch ((msg_t->Private.Service_Report  & 0xf000 )>>12){
    	    	   case MSG_SDN_ERROR :
    	             msg_t->Service_Report.Way= _FIP_NOACK_BAD_RP_MSG;
    	             break;
   		   case MSG_NO_ACK :
   		     msg_t->Service_Report.Way= _FIP_ACK_NO_REC_AFTER_RETRY;
    	             break;
                   case MSG_ACK_NEG	:
		     msg_t->Service_Report.Way= _FIP_ACK_NEG_AFTER_RETRY;
    	             break;
                   case MSG_ACK_NEG_REP :
                     msg_t->Service_Report.Way= _FIP_ACK_NEG_NO_RETRY;
    	             break;
    	           default:
    	             break;
                 }
            }
        }
    	Ref_mdl = msg_t->Private.Ref_mdl;
		Ref_mdl->Send_infos.User_Msg_Ack(Ref_mdl,msg_t);
  	    Ref_mdl->Send_infos.Cpt_Occup--;
     }
  }
}

 /***********************************************************************
	_FIP_SMAP_ACQ_EMIS (FDM_REF *Ref, FIP_EVT_TYPE *EVT)
	modification  / fdm_v3
 procedure appelee dans fdevt sur
 	 reception evenement EVT_SEND_MSG
 on aura Channel_NR = EVT->Additional_Information  bit 8 a 11;
 	 no_bloc    = EVT->Additional_Information  bit 0 a 7
 	 msg_ack    = EVT->Additional_Information  bit 12 a 15

 ************************************************************************/
void _FIP_SMAP_ACQ_EMIS (FDM_REF *Ref, Ushort Evt_Additional_Information)
{

  Uchar   no_bloc_fifo;
  Uchar   no_canal;
  FDM_MSG_TO_SEND     *dli_t;
  INTEG_FILE_TYPE  *FIFO_MSG_CNF;

  OS_Enter_Region() ;
  no_canal     = (Uchar) ((Evt_Additional_Information & 0x0f00) >>8);
  no_bloc_fifo = (Uchar)   Evt_Additional_Information;
  dli_t = Ref->FDM_Internal_Use.Messagerie.tab_wait_cnf[no_canal][no_bloc_fifo];
  Ref->FDM_Internal_Use.Messagerie.tab_wait_cnf[no_canal][no_bloc_fifo] = (FDM_MSG_TO_SEND*)0;
  if ( dli_t == (FDM_MSG_TO_SEND  *) 0 ) {
		OS_Leave_Region() ;
   		return ;
	}
    /* decrement du compteur de ressources du canal */
  Ref->FDM_Internal_Use.Messagerie.Cpt_canal_T[no_canal]--;
  dli_t->Private.User_Msg = dli_t ;
  dli_t->Private.evt_addit_info = Evt_Additional_Information;
  FIFO_MSG_CNF = &(Ref->FDM_Internal_Use.Files_MSG_T[9]);

  STORE_IN_FIFO_INTEG(FIFO_MSG_CNF,
         	    (INTEG_FILE_ELT *) &dli_t->Private );
  OS_Leave_Region() ;
}

/*======================================================================*/
/*======================================================================*/
/*       procedures de creation /destruction des canaux 		*/
/*		fdm_channel_create					*/
/*		fdm_channel_delete 					*/
/*======================================================================*/

/************************************************************************/
Ushort fdm_channel_delete ( FDM_REF *Ref , Ushort Channel_Nr )
{
	Ulong		 Identifier;
	DESC_TRAME_TYPE	 *DT;
	Ushort		 Cr  ;
	Ushort 		 position_ch;

	if  ( ( Channel_Nr == 0 ) || ( Channel_Nr > 8) ) 
	{
		FDM_Signal_Warning ( Ref  , _INVALID_NO_CHANNEL ,0 ) ;
		return FDM_NOK;
	}
	Identifier = Ref->FDM_Internal_Use.Messagerie.CANAL[Channel_Nr] ;
	if (Identifier == 0xffffffffL ) {
		FDM_Signal_Warning ( Ref  , _CHANNEL_NOT_CONFIGURED , 0) ;
		return FDM_NOK;
	}
	position_ch= Ref->FDM_Internal_Use.Messagerie.Image_canal[Channel_Nr] ;
    /* modification bdd Fip*/
 	OS_fdm_sm_p_bd( Ref ) ;
	DT = (DESC_TRAME_TYPE *)  _DICHO_SCAN_OBJECT(
		&Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE , Identifier );
	DT->position 		&= ~(_position_CH_I1 |_position_CH_I2 );
	if ( Msg_Id_Desc_Cons > DT->Var_Desc_Ptr ) 
	{
	   	  /* descripteur de trame msg , non associe a descripteur variable  */
	      SUP_DTRAME(  Ref ,  (Ushort) Identifier );
	      FDM_STOP_DB_MOD ( Ref );
	      DELETE_DTRAME ( Ref) ;
	} else 
	{	/* raz canal seulement */
		Absolute_ADR ADR ;
	    DT->Msg_Channel =  Msg_Channel_UNUSED ;
	    if(position_ch & _position_CH_I1)
		{
 			/* raz canal dans Image_1 */
	     	ADR = _SCAN_DTRAME_IN_FIP ( Ref , 0 , (Ushort)Identifier ) ;
	     	if ( ADR == 0xffffffffL)
			{
				OS_fdm_sm_v_bd(Ref) ;
				FDM_Signal_Error ( Ref , _INTEGRITY_DATA_BASE_FAULT, 0 );
				return (FDM_NOK);
			}
	     	if((DT->position & _position_SAP ) != 0)
			{
	     	         DT->Var_Desc_Ptr  =  Msg_Id_Desc_Cons;
	     	}

	     	Cr = _FIP_WRITE_WORD ( Ref , ADR + 8 , DT->Msg_Channel );
	     	Cr |= _FIP_WRITE_WORD ( Ref , ADR + 15 , DT->position );
	      	if (Cr !=0)
			{
	      		OS_fdm_sm_v_bd( Ref ) ;
	      	    return (FDM_NOK );
	     	}
		}
	    if(position_ch & _position_CH_I2)
		{
 			/* raz canal dans Image_2 */
	     	ADR = _SCAN_DTRAME_IN_FIP ( Ref , 1 , (Ushort)Identifier ) ;
	     	if ( ADR == 0xffffffffL)
			{
				OS_fdm_sm_v_bd(Ref) ;
				FDM_Signal_Error ( Ref , _INTEGRITY_DATA_BASE_FAULT, 0 );
				return (FDM_NOK);
	     	}
	     	Cr = _FIP_WRITE_WORD ( Ref , ADR + 8 , DT->Msg_Channel );
	     	Cr |= _FIP_WRITE_WORD ( Ref , ADR + 15 , DT->position );
	      	if (Cr !=0)
			{
	      		OS_fdm_sm_v_bd( Ref ) ;
	      	    return (FDM_NOK );
	     	}
		}
	}
 	OS_fdm_sm_v_bd( Ref ) ;
 	Ref->FDM_Internal_Use.Messagerie.CANAL[Channel_Nr]       = 0xffffffffL;
  	Ref->FDM_Internal_Use.Messagerie.Image_canal[Channel_Nr] = 0;
    return FDM_OK;
}

/************************************************************************/
Ushort fdm_channel_create ( FDM_REF *Ref , FDM_CHANNEL_PARAM *Parametres)
{
  	DESC_TRAME_TYPE *DT;
	element		E;
    Ushort          Cr = 0;
	Ushort 		position_ch;

	if (( Parametres->Channel_Nr ==0) || ( Parametres->Channel_Nr >8))
	{
		/* no de canal non valide*/
        FDM_Signal_Warning (Ref  ,_INVALID_NO_CHANNEL,0);
		return FDM_NOK;
	}
    if ( Ref->FDM_Internal_Use.Messagerie.CANAL[ Parametres->Channel_Nr ] != 0xffffffffL ) 
	{
		/*canal deja utilise */
        FDM_Signal_Warning ( Ref  , _CHANNEL_ALREADY_EXIST,0 ) ;
		return FDM_NOK;
	}
	if (Ref->S.Nr_Of_Tx_Buffer[ Parametres->Channel_Nr ] ==0 )
	{
  		FDM_Signal_Warning (Ref ,_NOT_TX_BUFFER_ON_CHANNEL,0);
  		return FDM_NOK;
	}
	if ( (Ref->S.Type & TWO_IMAGE_MODE) == 0 ) Parametres->Position = 1;
	switch(Parametres->Position)
	{
	case _FDM_MSG_IMAGE_1:
		  position_ch     = _position_CH_I1; break;
	case _FDM_MSG_IMAGE_2:
		  position_ch     = _position_CH_I2; break;
	case _FDM_MSG_IMAGE_1_AND_2:
		  position_ch     =_position_CH_I1 | _position_CH_I2 ;break;
	default:
		FDM_Signal_Warning ( Ref  , _IMAGE_NOT_CONFIGURED  , 0 );
		return FDM_NOK ;
	}

	/* creation du canal ==> modif bdd Fip */
    OS_fdm_sm_p_bd( Ref ) ;
    DT = (DESC_TRAME_TYPE *)_DICHO_SCAN_OBJECT
   		(&Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE ,
		(Ulong) Parametres->Identifier );

    if ( DT == NULL ) {
   		/* aucun descripteur associe sur cet ID   */
        DT =(DESC_TRAME_TYPE *) FDM_Malloc ( Ref, sizeof(DESC_TRAME_TYPE));
        if ( DT == ( DESC_TRAME_TYPE *) 0 )	
		{
			OS_fdm_sm_v_bd(Ref) ;
			FDM_Signal_Warning ( Ref ,   _ALLOCATE_MEMORY_FAULT , 0 ) ;
			return (FDM_NOK);
		}
        memset ( DT       , 0 , sizeof ( DESC_TRAME_TYPE ));
        DT->Identifier	  =  Parametres->Identifier ;
        DT->Var_Desc_Ptr  =  Msg_Id_Desc_Prod;
        DT->Msg_Channel   =  Parametres->Channel_Nr ;
		if ( FDM_place_dans_base_x(Ref, (Ushort) Parametres->Position)  !=0 ) 
		{
  			/* il n y  a pas  de la place dans tables Dtrames*/
			FDM_Free (Ref , (char*)DT);
       	    OS_fdm_sm_v_bd(Ref) ;
			FDM_Signal_Warning ( Ref ,   _FRAME_DESCRIPTOR_TABLE_OVERFLOW  , 0) ;
 			return (FDM_NOK);
        }
        DT->position  = position_ch ;
        E.objet_num = (Ulong) Parametres->Identifier ;
        E.ptr       = DT;
        _DICHO_ADD_OBJECT (&Ref->FDM_Internal_Use.fipgen.D_Trame_TABLE ,&E );
		FDM_STOP_DB_MOD ( Ref ); /* on dnload */
		Ref->FDM_Internal_Use.Messagerie.CANAL[Parametres->Channel_Nr] =
	                                                 Parametres->Identifier ;
		Ref->FDM_Internal_Use.Messagerie.Image_canal[Parametres->Channel_Nr] =
	                                                 position_ch;
		OS_fdm_sm_v_bd( Ref ) ;
		return (FDM_OK);
		}
	else
	{
     	  /* Cet ID est deja associe a une variable */
	  /*----------------------------------------*/
		Absolute_ADR ADR ;
		if ( DT->Msg_Channel != Msg_Channel_UNUSED ) 
		{ 	/* deja canal !! */
           OS_fdm_sm_v_bd(Ref) ;
		   FDM_Signal_Warning (Ref, _CHANNEL_ALREADY_EXIST,
			 	(Ulong)DT->Msg_Channel ) ;
		return (FDM_NOK);
		}
		if ( DT->position & (_position_VaD | _position_MSGa | _position_Cons) )
		{
	       /*Variable consommee ou produite consommee ou
			 avec demande msg aperiodique*/
			OS_fdm_sm_v_bd(Ref) ;
			FDM_Signal_Warning ( Ref , _NOT_CREATED_CHANNEL_ON_THIS_VARIABLE ,0) ;
			return (FDM_NOK);
		}

		DT->Msg_Channel  = Parametres->Channel_Nr  ;
		DT->position     |=position_ch ;
		if(position_ch & _position_CH_I1)
		{
 	     /* recherche adresse du descriteur de trame - IMAGE 1 */
  			ADR = _SCAN_DTRAME_IN_FIP( Ref, 0,(Ushort)Parametres->Identifier);
  			if ( ADR == 0xffffffffL)
			{
     	   		OS_fdm_sm_v_bd(Ref) ;
				FDM_Signal_Error ( Ref, _INTEGRITY_DATA_BASE_FAULT ,0  );
				return (FDM_NOK);
      		}
      	    Cr = _FIP_WRITE_WORD ( Ref , ADR + 8 , DT->Msg_Channel );
     	    Cr |= _FIP_WRITE_WORD ( Ref , ADR + 15 , DT->position );
      	    if (Cr !=FDM_OK)
			{
                OS_fdm_sm_v_bd(Ref) ;
				return (FDM_NOK);
            }
		}
        if(position_ch & _position_CH_I2)
		{
 			/* recherche adresse du descriteur de trame - IMAGE 1 */
  			ADR = _SCAN_DTRAME_IN_FIP( Ref, 1,(Ushort)Parametres->Identifier);
  			if ( ADR == 0xffffffffL)
			{
     	   		OS_fdm_sm_v_bd(Ref) ;
				FDM_Signal_Error ( Ref, _INTEGRITY_DATA_BASE_FAULT ,0  );
				return (FDM_NOK);
      	    }
      	    Cr = _FIP_WRITE_WORD ( Ref , ADR + 8 , DT->Msg_Channel );
     	    Cr |= _FIP_WRITE_WORD ( Ref , ADR + 15 , DT->position );

      	    if (Cr !=FDM_OK)
			{
                OS_fdm_sm_v_bd(Ref) ;
				return (FDM_NOK);
            }
		}

		Ref->FDM_Internal_Use.Messagerie.CANAL[Parametres->Channel_Nr] = Parametres->Identifier ;
		Ref->FDM_Internal_Use.Messagerie.Image_canal[Parametres->Channel_Nr] = position_ch;
	    OS_fdm_sm_v_bd( Ref ) ;
		return (FDM_OK);
    }
}

/*======================================================================*/
/*      procedures de messagerie de la classe MsgDataLink		*/
/*               	 sur  FDM_MESSAGING_REF			*/
/*=======================================================================*/
/*======================================================================*/
/************************************************************************/
/*       configurationcreation /destruction de FDM_MESSAGING_REF	*/
/*		fdm_messaging_fullduplex_create				*/
/*		fdm_messaging_to_send_create 				*/
/*		fdm_messaging_to_rec_create  				*/
/*		fdm_msg_datalink_delete 				*/
/************************************************************************/

/**************************************************************************/
static Ushort
controle_validite_datalink(FDM_MESSAGING_REF* Ref_mdl,
						int Number_Of_Msg_Desc, int Number_Of_Msg_Block )
{
  enum CODE_ERROR	cr_code_error ;
  FDM_REF 		*Fip;
  Ushort  		Cr ;
  Ulong 		Remote_DLL_Address;
  _TABLE	  	*tab_Remote_DLL;
  Ushort         	creation_tab_remote;
  Ushort 		G;
  Uchar 	  	segment ;
  element		E , *El_Remote_DLL;
  enum __dicho_result  	cr_dicho;
  Ushort 		position;

  Ulong ADD_repport = 0 ;
  cr_code_error = 0;
  position = 0 ;
  Fip=Ref_mdl->Fip;
  Ref_mdl->Receive_infos.Private.GM1  =NULL;
  Ref_mdl->Receive_infos.Private.GM2  =NULL;
	 /* controle partie position : image_1 et image_ 2 */
 	 /*------------------------------------------------*/
   if ((Ref_mdl->Type.Position & _MSG_IMAGE_1_2 )==0) {
	/*position non configuree par user */
	FDM_Signal_Warning ( Fip,_IMAGE_NOT_CONFIGURED  ,0  );
  	return (FDM_NOK);
      }
	 /* controle partie emission */
	 /*--------------------------*/
   if (Ref_mdl->Type.Communication & TYPE_MDL_EMIS ) {
      Ushort		Channel_Nr;
      if( Ref_mdl->Send_infos.User_Msg_Ack  ==
	(void (*)(struct _FDM_MESSAGING_REF *, struct _FDM_MSG_TO_SEND *)) 0 ) {
	FDM_Signal_Warning ( Fip, _GEN_MESSAGING_USER_ACK_EMIS ,0  );
  	return (FDM_NOK);
      }
     Channel_Nr = Ref_mdl->Send_infos.Channel_Nr ;
     if  ( Channel_Nr > 8)  {
	        FDM_Signal_Warning ( Fip  , _INVALID_NO_CHANNEL ,0 ) ;
		return FDM_NOK;
     }
     /* test si le canal dispose de buffer d'emission  */
     if (Fip->S.Nr_Of_Tx_Buffer[Channel_Nr] ==0 ){
  	    FDM_Signal_Warning ( Fip  , _NOT_TX_BUFFER_ON_CHANNEL,0) ;
		return FDM_NOK;
     }
  }

  if ((Ref_mdl->Type.Communication & TYPE_MDL_RECEP ) == 0) {
     return(FDM_OK);
 }else {
 		 /* controle partie reception */
 		 /*---------------------------*/
    segment = (Uchar)(Ref_mdl->Local_DLL_Address);
    if ( Ref_mdl->Local_DLL_Address  & 0x40000000L ) {
	if ( segment == Fip->H.MySegment ){
		    cr_code_error= _GEN_MESSAGING_ILLEGAL_SEGMENT ;
		   goto abort_lib0 ;
 	}
     }
     creation_tab_remote=0;
          /* -1) verifie si procedure de retour user presente */
     if ( Ref_mdl->Receive_infos.User_Msg_Rec_Proc ==
		(void (*)(struct _FDM_MESSAGING_REF *, struct _FDM_MSG_RECEIVED *))0 ){
 	   cr_code_error=  _GEN_MESSAGING_USER_ACK_RECEP  ;
	   goto abort_lib0 ;
     }
        /* -2) verifie demande ressources */
     if( (Number_Of_Msg_Desc ==0 ) ||  (Number_Of_Msg_Block == 0)){
 	   cr_code_error=  _GEN_MESSAGING_BLOCK_DESC_ERROR  ;
	   goto abort_lib0 ;
     }
      /* -3) allocation des ressources memoires */
     Ref_mdl->Receive_infos.Private.GM1 =
     		     _NEW_USER ( Fip->H.Memory_Management_Ref  ,
    				sizeof ( FDM_MSG_RECEIVED) , Number_Of_Msg_Desc) ;
     Ref_mdl->Receive_infos.Private.GM2 =
      			_NEW_USER ( Fip->H.Memory_Management_Ref  ,
    				sizeof ( FDM_MSG_R_DESC)  , Number_Of_Msg_Block ) ;
     if (( Ref_mdl->Receive_infos.Private.GM1== NULL ) ||
         ( Ref_mdl->Receive_infos.Private.GM2== NULL )){
		goto abort_lib1 ;
     }
       /* -4) creation lien_couple dans les 2 tables dicho
              en verifiant la non existance du couple
              			local_DLL_Addres-Remote_DLL_Address*/
     if ( Ref_mdl->Remote_DLL_Address & 0x80000000L ){
      	Remote_DLL_Address=0x40000000L ;	/* adresse remote non definie*/
     }else{
	Remote_DLL_Address =Ref_mdl->Remote_DLL_Address;
     }
     El_Remote_DLL = (element*)INTERNAL_SCAN_OBJECT (
     			&(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local),
      			  Ref_mdl->Local_DLL_Address);
      /* si objet local_DLL_Address existe deja ==> table remote_existe deja,
      	  sinon nouvel objet Local_DLL_Address  */
     if (  El_Remote_DLL != NULL ){
      		/*tab_Remote_DLL   existe deja,
 		  verifie que Remote_DLL_Address n' existe pas deja */
      	     if ((_TABLE*)_DICHO_SCAN_OBJECT((_TABLE**)&El_Remote_DLL->ptr,Remote_DLL_Address) != NULL){
	        cr_code_error= _GEN_MESSAGING_ALREADY_CONFIGURED;
		goto abort_lib1 ;
             }
     }else{
  	   cr_dicho = _DICHO_CREATE_VAR( Fip->H.Memory_Management_Ref ,
  	   				 &tab_Remote_DLL);
  	   if (cr_dicho  != __dicho_result_ok) {
		  ADD_repport = Add_Repport_MSG;
	          cr_code_error=_ALLOCATE_MEMORY_FAULT  ;
		  goto abort_lib1 ;
  	   }
 	   creation_tab_remote=1;
  	   E.objet_num = Ref_mdl->Local_DLL_Address;
 	   E.ptr       = tab_Remote_DLL;
 	   cr_dicho=_DICHO_ADD_OBJECT(
 	     	     &(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local),&E);
	   if (cr_dicho  != __dicho_result_ok) {
		  ADD_repport = Add_Repport_MSG;
	          cr_code_error=_ALLOCATE_MEMORY_FAULT  ;
		  goto abort_lib2 ;
  	   }
       }
       /* ajout de l'element dans la 2 ieme table dicho : table tab_Remote_DLL */
       El_Remote_DLL = (element*)INTERNAL_SCAN_OBJECT (
     			&(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local),
      			  Ref_mdl->Local_DLL_Address);
       E.objet_num = Remote_DLL_Address;
       E.ptr       = Ref_mdl;
       cr_dicho=_DICHO_ADD_OBJECT((_TABLE**)&El_Remote_DLL->ptr,&E);
       if (cr_dicho  != __dicho_result_ok) {
	        ADD_repport = Add_Repport_MSG;
 	        cr_code_error= _ALLOCATE_MEMORY_FAULT  ;
		goto abort_lib3 ;
       }

     /* - 5) creation de la BD FIP */
    switch (Ref_mdl->Type.Position ) {
	case _FDM_MSG_IMAGE_1:
		position = _position_MSG_I1;
	break;
	case _FDM_MSG_IMAGE_2:
		position = _position_MSG_I2;
	break;
	case _FDM_MSG_IMAGE_1_AND_2:
		position = _position_MSG_I1 | _position_MSG_I2;
	break;
	default:
	break;
    }

    if (( segment == 0) || ( segment == Fip->H.MySegment )) {
     /* adressage individuel ===> creation LSAP_FIP */
	DESC_TRAME_TYPE *DT;
	Ulong 		Ident;
      Ident   = Ref_mdl->Local_DLL_Address >>8;
      OS_fdm_sm_p_bd( Fip ) ;
      DT = (DESC_TRAME_TYPE *)  _DICHO_SCAN_OBJECT(
		&Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE ,(Ushort) Ident );
      if ( DT == NULL ) {
		 /* aucun ID n' est associe  */
	    DT =(DESC_TRAME_TYPE *) FDM_Malloc ( Fip, sizeof(DESC_TRAME_TYPE));
            if ( DT == ( DESC_TRAME_TYPE *) 0 ){
                OS_fdm_sm_v_bd(Fip) ;
	        cr_code_error=  _ALLOCATE_MEMORY_FAULT  ;
	        goto abort_lib4 ;
	     }
            memset ( DT       , 0 , sizeof ( DESC_TRAME_TYPE ));
            DT->Identifier      = (Ushort) Ident ;
            DT->Type_Descriptor =  MSGr_autorisee;
            DT->Var_Desc_Ptr    =  Msg_Id_Desc_Cons;
            if (segment ==0){
                DT->position    = _position_SAP | position;
            }else{
                DT->position    = _position_SAPx | position;
            }
           if (FDM_place_dans_base_x(Fip, position)  !=0 ) {
  	   /* il n y  a pas  de la place dans tables Dtrames de(s) base(s)*/
	      FDM_Free (Fip , (char*)DT);
       	      OS_fdm_sm_v_bd(Fip) ;
	      cr_code_error=  _FRAME_DESCRIPTOR_TABLE_OVERFLOW  ;
 	      goto abort_lib4 ;
           }
           E.objet_num = (Ulong) Ident ;
           E.ptr       = DT;
           _DICHO_ADD_OBJECT (&Fip->FDM_Internal_Use.fipgen.D_Trame_TABLE ,&E );
           FDM_STOP_DB_MOD ( Fip ); /* on dnload */
       }else{
             /* Cet ID est deja associe a une variable */
 		Absolute_ADR ADR ;
            if ( segment == 0 ) {
               if ( DT->position & _position_SAPx ) {
                 OS_fdm_sm_v_bd(Fip) ;
	         cr_code_error =_EXCEEDED_SEGMENT_NUMBER ;
	         goto abort_lib4 ;
	       }else {
	         DT->position        |= _position_SAP | position;
	       }
            }else {
               if ( DT->position & _position_SAP ) {
                 OS_fdm_sm_v_bd(Fip) ;
	         cr_code_error = _EXCEEDED_SEGMENT_NUMBER  ;
	         goto abort_lib4 ;
	       }else {
	         DT->position        |= _position_SAPx | position;
	       }
            }
            DT->Type_Descriptor  =  MSGr_autorisee;
              /* recherche adresse du descriteur de trame - dans base 0 */
	    if ( position & _position_MSG_I1){
            	ADR = _SCAN_DTRAME_IN_FIP( Fip, 0,(Ushort)Ident);
         	if ( ADR == 0xffffffffL){
              		OS_fdm_sm_v_bd(Fip) ;
	       		cr_code_error= _INTEGRITY_DATA_BASE_FAULT ;
	       		goto abort_lib4 ;
           	 }
            	 Cr = _FIP_WRITE_WORD ( Fip , ADR + 1 , DT->Type_Descriptor  );
           	 if (Cr !=FDM_OK){
              		OS_fdm_sm_v_bd(Fip) ;
		  	ADD_repport = Add_Repport_MSG;
               		cr_code_error=_CIRCUIT_ACCES_FAILED;
	       		goto abort_lib4 ;
                  }
    	    }
              /* recherche adresse du descriteur de trame - dans base 1 */
	    if ( position & _position_MSG_I2){
            	ADR = _SCAN_DTRAME_IN_FIP( Fip, 1,(Ushort)Ident);
         	if ( ADR == 0xffffffffL){
              		OS_fdm_sm_v_bd(Fip) ;
	       		cr_code_error= _INTEGRITY_DATA_BASE_FAULT ;
	       		goto abort_lib4 ;
           	 }
            	 Cr = _FIP_WRITE_WORD ( Fip , ADR + 1 , DT->Type_Descriptor  );
           	 if (Cr !=FDM_OK){
              		OS_fdm_sm_v_bd(Fip) ;
		  	ADD_repport = Add_Repport_MSG;
               		cr_code_error=_CIRCUIT_ACCES_FAILED;
	       		goto abort_lib4 ;
    	          }
               }
         }
         OS_fdm_sm_v_bd(Fip) ;
         /*fin de creation LSAP_FIP  et modif bdd FIP */
    } else {
	/* config segment bridge */
     	G = SEGMENT_CONFIG1;
    	Fip->FDM_Internal_Use.Messagerie.TAB_SEGMENT[segment]= G;
    	Cr = _FIP_WRITE_WORD( Fip, MSG_TAB_SEGMENT +segment , G);
    	if (Cr !=0 ){
	       ADD_repport = Add_Repport_MSG;
               cr_code_error=_CIRCUIT_ACCES_FAILED;
	      goto abort_lib5 ;
	}
     }
     return(FDM_OK);

  abort_lib5:
         _FIP_LSAP_delete (Fip, Ref_mdl->Local_DLL_Address );
  abort_lib4:
      tab_Remote_DLL =(_TABLE*)_DICHO_SCAN_OBJECT(
     			&(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local),
      			  Ref_mdl->Local_DLL_Address);
     _DICHO_SUP_OBJECT(&tab_Remote_DLL, Remote_DLL_Address);
  abort_lib3:
      if (creation_tab_remote	==1){
 	_DICHO_SUP_OBJECT(&(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local), Ref_mdl->Local_DLL_Address);
      }
  abort_lib2:
 			/*supression element en table dicho*/
      if (creation_tab_remote	==1){
		_DICHO_REMOVE(&tab_Remote_DLL);
      }
  abort_lib1:
#if					( FDM_WITH_RTX	== YES )
	cr_code_error = _RTX_REPORT_ERROR;
	ADD_repport   = GetLastError();
#endif
	if (Ref_mdl->Receive_infos.Private.GM1 !=NULL)
      		_DELETE_USER(Ref_mdl->Receive_infos.Private.GM1);
  	if (Ref_mdl->Receive_infos.Private.GM2 !=NULL)
    		_DELETE_USER(Ref_mdl->Receive_infos.Private.GM2);

     	 if (cr_code_error !=_INTEGRITY_DATA_BASE_FAULT)
 abort_lib0:
      	 {
  		FDM_Signal_Warning ( Fip, cr_code_error ,ADD_repport  );
  		return (FDM_NOK);
         }else
         {
   		FDM_Signal_Error ( Fip, _INTEGRITY_DATA_BASE_FAULT ,0  );
                return (FDM_NOK);
         }
  }

}

/**************************************************************************/
FDM_MESSAGING_REF* fdm_messaging_fullduplex_create(FDM_REF* Fip,
					FDM_MESSAGING_FULLDUPLEX* cfg)
{
    FDM_MESSAGING_REF  *Ref_mdl;
    Ushort 		cr;
   if (( Fip->S.Type & MESSAGE_RECEPTION_AUTHORIZED ) == 0 ){
  	FDM_Signal_Warning(Fip,_MESSAGING_CONTEXT_NOT_ALLOWED , 0) ;
        return (NULL);
    }
    Ref_mdl=(FDM_MESSAGING_REF *)FDM_Malloc(Fip,sizeof(FDM_MESSAGING_REF));
    if(Ref_mdl ==NULL){
  	FDM_Signal_Warning(Fip,_ALLOCATE_MEMORY_FAULT  ,Add_Repport_MSG) ;
        return (NULL);
    }
    Ref_mdl->Send_infos.User_Msg_Ack = cfg->sending.User_Msg_Ack;
    Ref_mdl->Send_infos.Channel_Nr = cfg->sending.Channel_Nr ;
    Ref_mdl->Send_infos.User_Qid   = cfg->sending.User_Qid ;
    Ref_mdl->Send_infos.User_Ctxt  = cfg->sending.User_Ctxt;
    Ref_mdl->Send_infos.User_Msg_Ack_Type  = _FIP_ADDRESS_TYPE;
    Ref_mdl->Send_infos.Cpt_Occup  = 0;

    Ref_mdl->Receive_infos.User_Msg_Rec_Proc=cfg->receiving.User_Msg_Rec_Proc;
    Ref_mdl->Receive_infos.User_Qid  = cfg->receiving.User_Qid ;
    Ref_mdl->Receive_infos.User_Ctxt = cfg->receiving.User_Ctxt;
    Ref_mdl->Type.Communication = TYPE_MDL_EMIS | TYPE_MDL_RECEP;
    Ref_mdl->Type.Position      = cfg->Position;
    Ref_mdl->Fip  = Fip;
    Ref_mdl->Local_DLL_Address  =cfg->Local_DLL_Address  ;
    Ref_mdl->Remote_DLL_Address =cfg->Remote_DLL_Address ;
    cr= controle_validite_datalink(Ref_mdl,
    					      cfg->receiving.Number_Of_Msg_Desc,
    					      cfg->receiving.Number_Of_Msg_Block);
    if (cr == FDM_OK)
  	 return(Ref_mdl);
    else {
	FDM_Free(Fip, (char *)Ref_mdl);
        return (NULL);
     }
}


 /**************************************************************************/
FDM_MESSAGING_REF* fdm_messaging_to_send_create (FDM_REF* Fip,
					FDM_MESSAGING_TO_SEND  * cfg)
{
    FDM_MESSAGING_REF  *Ref_mdl;
    Ushort 	       cr ;

    Ref_mdl=(FDM_MESSAGING_REF *)FDM_Malloc(Fip,sizeof(FDM_MESSAGING_REF));
    if(Ref_mdl ==NULL){
  	FDM_Signal_Warning(Fip,_ALLOCATE_MEMORY_FAULT  ,Add_Repport_MSG) ;
        return (NULL);
    }
    Ref_mdl->Send_infos.User_Msg_Ack = cfg->sending.User_Msg_Ack;
    Ref_mdl->Send_infos.Channel_Nr = cfg->sending.Channel_Nr ;
    Ref_mdl->Send_infos.User_Qid   = cfg->sending.User_Qid ;
    Ref_mdl->Send_infos.User_Ctxt  = cfg->sending.User_Ctxt;
    Ref_mdl->Send_infos.User_Msg_Ack_Type  = _FIP_ADDRESS_TYPE;
    Ref_mdl->Send_infos.Cpt_Occup  = 0;

    Ref_mdl->Type.Communication = TYPE_MDL_EMIS ;
    Ref_mdl->Type.Position      = cfg->Position;
    Ref_mdl->Fip  = Fip;
    Ref_mdl->Local_DLL_Address  =cfg->Local_DLL_Address  ;
    Ref_mdl->Remote_DLL_Address =cfg->Remote_DLL_Address ;
    cr= controle_validite_datalink(Ref_mdl,0,0);
    if (cr == FDM_OK)
  	 return(Ref_mdl);
    else {
	FDM_Free(Fip, (char *)Ref_mdl);
        return (NULL);
     }
}

/***************************************************************************/
FDM_MESSAGING_REF* fdm_messaging_to_rec_create  (FDM_REF* Fip,
					FDM_MESSAGING_TO_REC * cfg)
{
    FDM_MESSAGING_REF  *Ref_mdl;
    Ushort 	       cr ;
    if (( Fip->S.Type & MESSAGE_RECEPTION_AUTHORIZED ) == 0 ){
  	FDM_Signal_Warning(Fip,_MESSAGING_CONTEXT_NOT_ALLOWED , 0) ;
        return (NULL);
    }
    Ref_mdl=(FDM_MESSAGING_REF *)FDM_Malloc(Fip,sizeof(FDM_MESSAGING_REF));
    if(Ref_mdl ==NULL){
  	FDM_Signal_Warning(Fip,_ALLOCATE_MEMORY_FAULT  ,Add_Repport_MSG) ;
        return (NULL);
    }
    Ref_mdl->Receive_infos.User_Msg_Rec_Proc=cfg->receiving.User_Msg_Rec_Proc;
    Ref_mdl->Receive_infos.User_Qid  = cfg->receiving.User_Qid ;
    Ref_mdl->Receive_infos.User_Ctxt = cfg->receiving.User_Ctxt;
    Ref_mdl->Type.Communication = TYPE_MDL_RECEP;
    Ref_mdl->Type.Position      = cfg->Position;
    Ref_mdl->Fip  = Fip;
    Ref_mdl->Local_DLL_Address  =cfg->Local_DLL_Address  ;
    Ref_mdl->Remote_DLL_Address =cfg->Remote_DLL_Address ;
    cr = controle_validite_datalink(Ref_mdl,
    					      cfg->receiving.Number_Of_Msg_Desc,
    				 	      cfg->receiving.Number_Of_Msg_Block);
    if (cr == FDM_OK)
  	    return(Ref_mdl);
    else {
	FDM_Free(Fip, (char *)Ref_mdl);
        return (NULL);
     }
}
/*************************************************************/
Ushort fdm_messaging_delete(FDM_MESSAGING_REF* Ref_mdl)
{
     FDM_REF 	*Fip;
     _TABLE	*tab_Remote_DLL;
     int 	no_element;
     element	 *El_Remote_DLL;
	 Ulong L;

     if ( Ref_mdl ==NULL)                                return (FDM_NOK);

     if (Ref_mdl->Type.Communication & TYPE_MDL_EMIS ) {
		/*  Mise en commentaire par Xavier GAILLARD le 12 Juillet 2002
		 *  pour resoudre le probleme de perte memoire lors de la relance
		 *  de WorldFip.
		 *  ------------------------------------------------------------
		 * if (    Ref_mdl->Send_infos.Cpt_Occup  != 0)     
		 *	return (FDM_NOK);
		 *	*/
	 }

    Fip =Ref_mdl->Fip;

    if (Ref_mdl->Type.Communication & TYPE_MDL_RECEP) {
 	if (Ref_mdl->Receive_infos.Private.GM1 !=NULL)
      		_DELETE_USER(Ref_mdl->Receive_infos.Private.GM1);
  	if (Ref_mdl->Receive_infos.Private.GM2 !=NULL)
    		_DELETE_USER(Ref_mdl->Receive_infos.Private.GM2);
    El_Remote_DLL   = INTERNAL_SCAN_OBJECT (
     			&(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local),
      			  Ref_mdl->Local_DLL_Address);
 	L = ( Ref_mdl->Remote_DLL_Address == 0x80000000L ) ?
			0x40000000L : Ref_mdl->Remote_DLL_Address ;
	        _DICHO_SUP_OBJECT((_TABLE**)&El_Remote_DLL->ptr , L );
    /* si table remote vide ,
         		 delete table dicho remote
         	        et supression element local_DLL  dans table locale*/
    tab_Remote_DLL = El_Remote_DLL->ptr;
    no_element =tab_Remote_DLL->nombre -1 ;
    if (tab_Remote_DLL->k[no_element].objet_num ==-1 ){
         /* a voir si proc dans fdmdicho.c */
         _DICHO_REMOVE(&tab_Remote_DLL);
         _DICHO_SUP_OBJECT(
             		&(Fip->FDM_Internal_Use.Messagerie.Adr_TAB_DLL_Local),
					Ref_mdl->Local_DLL_Address);
	     _FIP_LSAP_delete (Fip, Ref_mdl->Local_DLL_Address );
         }
	}

	FDM_Free(Fip, (char *)Ref_mdl);
	return (FDM_OK);
}


/*************************************************************/
/************* demande d'emission d'un message ***************/
/*************************************************************/
/* sortie : FDM_NOK si l'utilisateur n' a pas fourni de procedure d'appel
		pour le compte rendu
	    OK pour tous les autres cas
*/



Ushort TTEb[16][8];
int    TTEbx = 0;
int    TTEbn = 0;


Ushort fdm_send_message ( FDM_MESSAGING_REF *Ref_mdl ,
			  FDM_MSG_TO_SEND   *lettre )
{
   FDM_REF *Ref;
   Ushort Channel_Nr;
   INTEG_FILE_TYPE  *FIFO_MSG_RQ;
#if ( FDM_WITH_CONTROLS	== YES)
   /* test si Ref_mdl configure en emission */
  if (( Ref_mdl->Type.Communication & TYPE_MDL_EMIS) !=TYPE_MDL_EMIS)
		return (FDM_NOK);
#endif
   Ref_mdl->Send_infos.Cpt_Occup++;
   Ref  		      = Ref_mdl->Fip;
   Channel_Nr 		      = Ref_mdl ->Send_infos.Channel_Nr;
   lettre->Private.Channel_Nr = Channel_Nr;
   lettre->Private.Ref_mdl    = Ref_mdl;
   OS_Enter_Region();
   FIFO_MSG_RQ = &Ref->FDM_Internal_Use.Files_MSG_T [Channel_Nr];
   STORE_IN_FIFO_INTEG( FIFO_MSG_RQ , (INTEG_FILE_ELT *) lettre );



   OS_Leave_Region();
   return FDM_OK;
}

/*************************************************************/
/************* demande de modif du no canal    ***************/
/*************************************************************/
Ushort fdm_change_channel_nr(FDM_MESSAGING_REF* Ref_mdl, Ushort Channel_Nr)
{
   	FDM_REF		 *Fip;
   Fip =  Ref_mdl->Fip;
#if ( FDM_WITH_CONTROLS	== YES)

   if ((Ref_mdl->Type.Communication & TYPE_MDL_EMIS) != TYPE_MDL_EMIS){
     	    FDM_Signal_Warning ( Fip  , _MESSAGING_CONTEXT_NOT_TO_SEND ,0 ) ;
	    return FDM_NOK;
   }
#endif
   if  ( Channel_Nr > 8)  {
	        FDM_Signal_Warning ( Fip  , _INVALID_NO_CHANNEL ,0 ) ;
		return FDM_NOK;
   }
   if (Channel_Nr !=0) {
         if (Fip->FDM_Internal_Use.Messagerie.CANAL[Channel_Nr]==0xffffffffL){
                FDM_Signal_Warning ( Fip  , _CHANNEL_NOT_CONFIGURED , 0) ;
		return FDM_NOK;
	 }
   } else{
  	       /*si Channel_Nr =0 on messagerie aperiodique */
   	/* test si le canal dispose de buffer d'emission  */
        if (Fip->S.Nr_Of_Tx_Buffer[0] ==0 ){
  	    FDM_Signal_Warning ( Fip  , _NOT_TX_BUFFER_ON_CHANNEL,0) ;
		return FDM_NOK;
	 }
    }
    Ref_mdl->Send_infos.Channel_Nr= Channel_Nr;
    return (FDM_OK);
}

#endif
