/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmevt.c
  Description     : Traitement des evenements Fip




  Date de creation: Fri Mar 15 15:11:33 MET 1996

Version:4.1.1
	a/-  Suppression de warning GNU

Version:4.3.1
	a/- Gestion de var param segment
	b/- ACK EOC et IRQ  acces libre ( recodage)
Version:4.3.11
	a/- EOC mode PULSE station non BA -> FIPCODE 6.3 du 26/6/97
Version:4.5.0
	a/ modifs pour portage sur cible DSP texas

***********************************************************/


#include "fdm.h"
#include "fipcode6.h"
#include "fdmmacro.h"


#if  (  FDM_WITH_FREE_ACCESS == NO )

static Ushort _FIP_READ_IRQ_FILE (  FDM_REF * Ref, FIP_EVT_Type *EVT )
{

	int tempo ;
	FDM_ERROR_CODE CR ;
	FIFO_Reg;
	register Ushort *dst;
	register unsigned int taille;

	Debut_procedure ;

	WAIT_Busy;

	ECRITE(UCom , CMDE_R_EVT );

	/* ici pour occuper utilement le micro */
	FIFO_port = FIFO;
    	dst = (Ushort *)EVT ;
	taille = 2 ;

	/************ attente bit SV */
	WAIT_SV;
 
	_FIP__tres_petite_tempo ( Ref ) ;

	if ( ( LIRE(Var_State) & Busy) != 0 ) {
		ECRITE(UCom,CMDE_CLOSE ) ;
		Fin_procedure;
		}

	/************ boucle de lecture */
	BLOCKINWORD(FIFO_port,dst,taille );

	/************ close */
	CR_UState ;

	ECRITE(UCom,CMDE_CLOSE ) ;

	Fin_procedure;
}

#else

static Ushort _FIP_READ_TYPE_EVT (  FDM_REF * Ref, Ushort *Type_Evt )

{
	volatile Ushort   *FifoUser, *FifoFip  ;
	Dial_AE *PDial_AE;
	int  tempo;
	Ushort Tmp;
	Ushort CR ;

	MAC__INIT_REG;

	MAC__WAIT_PLACE;

	MAC__ECRIRE_DEMANDE(READ_EVT_USER,0);

	MAC__WAIT_LECTURE_PRETE;

	*Type_Evt =  PDial_AE->Parametre  ;
abort:
	OS_Leave_Region();
	return 0;

abortd:
	return CR ;
}

#endif




Ushort _FIP_ACK_EOC ( FDM_REF * Ref  )
{
#if  (  FDM_WITH_FREE_ACCESS == NO)

	int tempo ;
	FDM_ERROR_CODE CR ;
	Acces_Key Key ;

	Debut_procedure ;

	WAIT_Busy;

	Key = 1;
	ECRIRE_Key( Key );

	ECRITE(UCom ,  CMDE_ACQ_EOC );

	/************ attente bit SV */
	WAIT_SV;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE ) ;

	Fin_procedure;
#else
	volatile Ushort   *FifoUser, *FifoFip  ;
	Dial_AE *PDial_AE;
	int  tempo;
	Ushort Tmp;

	Ushort CR = 0 ;
	MAC__INIT_REG;
	MAC__WAIT_PLACE;
	MAC__ECRIRE_DEMANDE(ACK_EOC_USER,1);

	tempo = NbLoop;
	while ( (LIRE(UState) & EOC) == 0) {
		_FIP__petite_tempo(Ref);
		tempo --;
		if ( tempo == 0 ) {
			OS_Leave_Region();
			FDM_Signal_Warning ( Ref  , _CIRCUIT_ACCES_FAILED , free_Acc_time_out  ) ;
			CR = free_Acc_time_out ;
			goto abortd;
		}
	}

	OS_Leave_Region();
	return CR ;

abortd:
	return CR ;

#endif

}


#if  (  FDM_WITH_FREE_ACCESS == NO)

static void traitement_de_l_IRQ (FDM_REF * Ref )
{
	FIP_EVT_Type EVT ;
	Ushort CR ;

	CR = _FIP_READ_IRQ_FILE ( Ref , &EVT );
	if ( CR != 0 ) return  ;


	switch (EVT.Event_Definition ) {

#if 		( FDM_WITH_APER	== YES )

		case 0x0100 : /* read-Write far */
		case 0x0200 :
			{
			FDM_MPS_VAR_REF *El ;

			El = Ref->FDM_Internal_Use.fipgen.Xrf->Var[ EVT.Additional_Information & 0xff ] ;
			if ( El != ( FDM_MPS_VAR_REF *) 0 ) _TIMER_STOP( &El->Timer );
			}
			break;

		case 0x130 : /* file aper urgent vide */
			_TIMER_STOP ( &Ref->FDM_Internal_Use.T_aper_U.Timer);
			break;
		case 0x131 : /* file aper normaux vide */
			_TIMER_STOP ( &Ref->FDM_Internal_Use.T_aper_N.Timer);
			break;

#endif

#if		( FDM_WITH_MESSAGING == YES)

		case 0x140 :
			_FIP_SMAP_ACQ_EMIS (Ref, EVT.Additional_Information);
			break;
		case 0x240 :
  		  	OS_Enter_Region();
  		  	Ref->FDM_Internal_Use.Messagerie.Cpt_Receive++;
   		  	if (Ref->FDM_Internal_Use.Messagerie.Cpt_Receive == 1 ) {
   		 		OS_Leave_Region();
				Ref->S.User_Signal_Rec_Msg(Ref);
			}
   		 	else OS_Leave_Region();
			break;


#endif

#if(  FDM_WITH_BA == YES )

	case EVT_BA_ACTIVITY :
		{
			FDM_MPS_VAR_REF *El ;
			_FDM_XAE_Tempo_Ref *T;

			T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Synchro_BA.Var[0].Timer;
			El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
 			fdm_mps_var_change_prod_cons ( El , _PRODUCED, _PRODUCED );

#if			( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
			T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.fipio.SM_MPS_Seg_Par.Var[0].Timer;
			El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
 			fdm_mps_var_change_prod_cons ( El , _PRODUCED, _PRODUCED );
#endif

		}
		break;
#if (FDM_WITH_REDONDANCY_MGNT ==YES )

	case EVT_BA_STOP1:      /*passage ba stopped sur time out suspend*/
 	   Ref->FDM_Internal_Use.Ba.cpt_timeout_suspend +=10 ;
#endif

	case EVT_BA_STOP2:	/*passage ba stopped sur anomalie reseau */
	case EVT_BA_STOP3:	/*passage ba stopped sur demande   user */
	case EVT_BA_IDLE:

#if (FDM_WITH_REDONDANCY_MGNT ==YES )

           _FIP_WRITE_PHY(Ref, FIP_CONF_RESEAU  ,
			&Ref->FDM_Internal_Use.Page_2000.Base_0 ,2);
	   Ref->FDM_Internal_Use.Ba.tst_p_en_cours    = 0;
       	   Ref->FDM_Internal_Use.Ba.cpt_nbre_tst_p    = 0;
	   Ref->FDM_Internal_Use.Ba.B0_tst_p         &=0x7fff ;
           Ref->FDM_Internal_Use.Medium.Voie_en_cours = 3 ;
#endif

	  {
		FDM_MPS_VAR_REF *El ;
		_FDM_XAE_Tempo_Ref *T;

		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Synchro_BA.Var[0].Timer;
		El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
		fdm_mps_var_change_prod_cons ( El , _CONSUMED, _CONSUMED );

#if	( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.fipio.SM_MPS_Seg_Par.Var[0].Timer;
		El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
 		fdm_mps_var_change_prod_cons ( El , _CONSUMED, _CONSUMED  );
#endif
	   }
#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
	   if ( EVT.Event_Definition == EVT_BA_STOP3) {
		if ( Ref->FDM_Internal_Use.Ba.Prio_Stop == 0x55aa ) {
			Ref->FDM_Internal_Use.Ba.Prio_Stop = 0;
		       	_FIP_START_BA(Ref,
				 Ref->FDM_Internal_Use.Ba.BA_in_use->Macro_Cycle_Adr);
		}
	    }
#endif
   	break;

#if (FDM_WITH_REDONDANCY_MGNT ==YES )

	case  EVT_TESTP_DEBUT  :
	    _FIP_SMAP_TEST_P_DEBUT ( Ref, EVT.Additional_Information+3);
	break;

	case  EVT_TESTP_FIN  :
	    _FIP_SMAP_TEST_P_FIN ( Ref );
	break;

#endif
#endif

		case 0x8100 : /* trt de l'indication A-send */
			{
			FDM_MPS_VAR_REF *Var_ref;
			FDM_AE_LE_REF *AE_LE;

			Var_ref  = Ref->FDM_Internal_Use.fipgen.Xrf->Var[ EVT.Additional_Information ] ;
			if ( Var_ref != NULL ) {
				AE_LE = Var_ref->Ref;
				if ( AE_LE->Etat == AE_LE_RUNNING ) {
					Var_ref->definition.Signals.User_Signal_Asent (Var_ref);
				}
			}
			break;
			}

		case 0x8200 : /* trt de l'indication A-receive */
			{
			FDM_MPS_VAR_REF *Var_ref;
			FDM_AE_LE_REF *AE_LE;


			Var_ref  = Ref->FDM_Internal_Use.fipgen.Xrf->Var[ EVT.Additional_Information ] ;
			if ( Var_ref != NULL ) {
				AE_LE = Var_ref->Ref;
				if ( AE_LE->Etat == AE_LE_RUNNING ) {
					Var_ref->definition.Signals.User_Signal_Areceive(Var_ref);
				}
			}
			break;
			}



		default :
			break;
		}

}

#else

/************  option *******************/
/******* FDM_WITH_FREE_ACCESS = YES******/

static void traitement_de_l_IRQ (FDM_REF * Ref )
{
	Ushort  Type_Evt ;
	Ushort  CR ;

     CR = _FIP_READ_TYPE_EVT( Ref , &Type_Evt );
     if ( CR != 0 ) return  ;

     if ( Type_Evt & EVT_FLAG_VAR )  {
	/*traitement var MPS */

	Ushort volatile *ptr_rempli;
	Ushort volatile *ptr_vidage;
	Ushort volatile *ptr_acces_key;

	Ushort volatile *Base_1 ;
	struct _Desc_Ptr {
		DESC_VAR_TYPE Desc[1] ;
	} *Desc_Ptr ;
	Ushort  var_Evenement;

	FDM_MPS_VAR_REF *Var_ref ;

		/*traitement var MPS */
	unsigned UTmp ;

	ptr_rempli  = Ref->H.FREE_ACCES_ADDRESS;
	ptr_rempli += EVT_R_FIFO_VAR ;
	ptr_vidage  = Ref->H.FREE_ACCES_ADDRESS;
	ptr_vidage += EVT_V_FIFO_VAR ;

	Base_1   = Ref->H.FREE_ACCES_ADDRESS;
	Base_1  += Ref->FDM_Internal_Use.fipgen.BASE_1;
	Desc_Ptr = (struct _Desc_Ptr *) Base_1 ;

        for ( UTmp = *ptr_vidage; UTmp != *ptr_rempli ;   ) {
	    int Cle;
	    /* lecture  evenement en fifo = cle acces  */
	    ptr_acces_key  = Ref->H.FREE_ACCES_ADDRESS;
	    ptr_acces_key += *ptr_vidage ;
	    Cle = *ptr_acces_key ;
	    Var_ref = Ref->FDM_Internal_Use.fipgen.Xrf->Var[ Cle  ] ;
	    if ( Var_ref != NULL ) {

		var_Evenement   = Desc_Ptr->Desc[ Cle ].Evenement ;

		switch (var_Evenement) {
#if 		( FDM_WITH_APER	== YES )
			case EVT_SEND_VAR_T: /* read-Write far */
		      	case EVT_RECEIVE_VAR_T :
			case 0:
				_TIMER_STOP( &Var_ref->Timer );

	      		break;
#endif

 	  	    	case EVT_SEND_VAR_P : /* trt de l'indication A-send */
			{
		   		FDM_AE_LE_REF *AE_LE;
		 		AE_LE = Var_ref->Ref;
				if ( AE_LE->Etat == AE_LE_RUNNING ) {
					Var_ref->definition.Signals.User_Signal_Asent (Var_ref);
		    		}
		   	}
		  	break;


	      		case EVT_RECEIVE_VAR_P : /* trt de l'indication A-receive */
		    	{
		  	    	FDM_AE_LE_REF *AE_LE;
				AE_LE = Var_ref->Ref;
				if ( AE_LE->Etat == AE_LE_RUNNING){
		       			    Var_ref->definition.Signals.User_Signal_Areceive(Var_ref);
		      		}
		     	}
		     	break;

	      		default:
			break;

        	}/*switch*/
	    }/*Var_ref != NULL*/

	     UTmp = ( UTmp + 1 ) & 0xff7f;
	    *ptr_vidage = ( Ushort )UTmp;
       }/*for*/
   }/* EVT_FLAG_VAR*/

#if(  FDM_WITH_BA == YES )


	if ((  Type_Evt & EVT_FLAG_BA_ACTIVITY ) != 0 ) {
		FDM_MPS_VAR_REF *El ;
		_FDM_XAE_Tempo_Ref *T;

		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Synchro_BA.Var[0].Timer;
		El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
 		fdm_mps_var_change_prod_cons ( El , _PRODUCED, _PRODUCED );

#if		( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.fipio.SM_MPS_Seg_Par.Var[0].Timer;
		El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
 		fdm_mps_var_change_prod_cons ( El ,_PRODUCED, _PRODUCED ) ;
#endif


	}

#if (FDM_WITH_REDONDANCY_MGNT ==YES )

	if ((  Type_Evt & EVT_FLAG_BA_STOP1 ) != 0 ) {
		 /*passage ba stopped sur time out suspend*/
		 Ref->FDM_Internal_Use.Ba.cpt_timeout_suspend +=10 ;
	}

	if ((  Type_Evt &  (EVT_FLAG_BA_STOP1 | EVT_FLAG_BA_STOP2 | EVT_FLAG_BA_STOP3 | EVT_FLAG_BA_IDLE) ) != 0 ){
		/*passage ba stopped sur anomalie reseau */
		/*passage ba stopped sur demande   user */
		_FIP_WRITE_PHY(Ref, FIP_CONF_RESEAU  ,
			&Ref->FDM_Internal_Use.Page_2000.Base_0 ,2);
		Ref->FDM_Internal_Use.Ba.tst_p_en_cours    = 0;
		Ref->FDM_Internal_Use.Ba.cpt_nbre_tst_p    = 0;
	   	Ref->FDM_Internal_Use.Ba.B0_tst_p         &=0x7fff ;
          	Ref->FDM_Internal_Use.Medium.Voie_en_cours = 3 ;

	}
#endif
	if ((  Type_Evt &  ( EVT_FLAG_BA_STOP1 | EVT_FLAG_BA_STOP2 | EVT_FLAG_BA_STOP3 | EVT_FLAG_BA_IDLE) ) != 0 ) {
		FDM_MPS_VAR_REF *El ;
		_FDM_XAE_Tempo_Ref *T;

		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.SM_MPS_Synchro_BA.Var[0].Timer;
		El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
		fdm_mps_var_change_prod_cons ( El , _CONSUMED, _CONSUMED );

#if		( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
		T = (_FDM_XAE_Tempo_Ref *) &Ref->FDM_Internal_Use.fipio.SM_MPS_Seg_Par.Var[0].Timer;
		El = ( FDM_MPS_VAR_REF *) T->Ref_XAE ;
 		fdm_mps_var_change_prod_cons ( El ,_CONSUMED, _CONSUMED );
#endif
#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
	   if ( ( Type_Evt & EVT_FLAG_BA_STOP3  ) != 0 ) {
		if ( Ref->FDM_Internal_Use.Ba.Prio_Stop == 0x55aa ) {
			Ref->FDM_Internal_Use.Ba.Prio_Stop = 0;
		       	_FIP_START_BA(Ref,
				 Ref->FDM_Internal_Use.Ba.BA_in_use->Macro_Cycle_Adr);
		}
	    }
#endif
	}

#if (FDM_WITH_REDONDANCY_MGNT ==YES )

	if ((  Type_Evt & EVT_FLAG_BA_SUSPEND ) != 0 ){
		Ushort * Source ;
		Ushort  no_evt_suspend ;
		Ushort  ad_ba;

			/*lecture fifo ba EVT_FIFO_BA */

		Source 		= Ref->H.FREE_ACCES_ADDRESS;
		Source 		+= EVT_FIFO_BA ;
		no_evt_suspend	= *Source++;
		ad_ba		= *Source;  /* adresse */

	        if (no_evt_suspend == EVT_TESTP_DEBUT){
	    	   _FIP_SMAP_TEST_P_DEBUT ( Ref, (Ushort)(ad_ba + 3));
		} else {
		   /* NO_EVT_TESTP_FIN */
	   	    _FIP_SMAP_TEST_P_FIN ( Ref );
		}
        }

#endif
#endif



#if		( FDM_WITH_MESSAGING == YES)

{	 Ushort volatile *ptr_rempli;
 	 Ushort volatile *ptr_vidage;
	 Ushort *Evt_Additional_Information;



	if (Type_Evt & EVT_FLAG_MSG_E ) {
	  	unsigned UTmp ;

	 	ptr_rempli  = Ref->H.FREE_ACCES_ADDRESS;
 	 	ptr_rempli += EVT_R_FIFO_MSG ;
 	 	ptr_vidage  = Ref->H.FREE_ACCES_ADDRESS;
 	 	ptr_vidage += EVT_V_FIFO_MSG ;

                for ( UTmp = *ptr_vidage; UTmp != *ptr_rempli ;   ) { 
	  		/* lecture  evenement en fifo msg = MSG_CR_EMIS */
   			 Evt_Additional_Information  = (Ushort*) Ref->H.FREE_ACCES_ADDRESS;
    	 		 Evt_Additional_Information += *ptr_vidage ;

			_FIP_SMAP_ACQ_EMIS (Ref,*Evt_Additional_Information);

		    	UTmp = ( UTmp + 1 ) & 0xff7f ;
		    	*ptr_vidage = ( Ushort )UTmp;
    	 	 }
	}

	if ( Type_Evt & EVT_FLAG_MSG_R ){
		OS_Enter_Region();
		Ref->FDM_Internal_Use.Messagerie.Cpt_Receive++;
   		if (Ref->FDM_Internal_Use.Messagerie.Cpt_Receive == 1 ) {
			OS_Leave_Region();
			Ref->S.User_Signal_Rec_Msg(Ref);
		} else
			OS_Leave_Region();
	}
}
#endif

#if 		( FDM_WITH_APER	== YES )
	if ( Type_Evt & EVT_FLAG_APU){
		/* file aper urgent vide */
		_TIMER_STOP ( &Ref->FDM_Internal_Use.T_aper_U.Timer);
	}
	if ( Type_Evt & EVT_FLAG_APN){
		 /* file aper normaux vide */
		_TIMER_STOP ( &Ref->FDM_Internal_Use.T_aper_N.Timer);
	}
#endif


}
#endif

#if ( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == NO )

Ushort fdm_process_it_eoc(FDM_REF * Ref)
{

	if ( (LIRE(UState) & EOC) == 0) { /* EOC */
		FDM_MPS_VAR_REF *Var_ref;
		Var_ref  = Ref->FDM_Internal_Use.fipgen.Xrf->Var[ 0 ] ;
		if ( Var_ref != (FDM_MPS_VAR_REF *) 0 )
			Var_ref->definition.Signals.User_Signal_Synchro(Ref);
		_FIP_ACK_EOC (  Ref  ) ;
	}
	return  ( Ushort)
		( ( ( LIRE(UState) & EOC )   == 0  ) ? 1 : 0 ) ;

}

Ushort fdm_process_it_irq( FDM_REF * Ref)
{
	if ( (LIRE(UState) & IRQ) != 0)
		traitement_de_l_IRQ ( Ref ) ;

	return  ( Ushort)
		( ( ( LIRE(UState) & IRQ  ) != 0  ) ? 1 : 0 );

}

#else

/********************************************************************
Cette procedure  traite un hard dans lequel IRQ et EOC sont en "ou cable"

1 / elle acquite les evenement EOC
2 / vide la file des EVT FIP puis realise le traitement associe

ps     :  EOC est prioritaire
rappel : pour ce type de hard , EOC est en mode etat obligatoirement
*********************************************************************/

Ushort fdm_process_its_fip (  FDM_REF * Ref )
{

	if ( (LIRE(UState) & EOC) == 0) { /* EOC */
		FDM_MPS_VAR_REF *Var_ref;
		Var_ref  = Ref->FDM_Internal_Use.fipgen.Xrf->Var[ 0 ] ;
		if ( Var_ref != (FDM_MPS_VAR_REF *) 0 )
			Var_ref->definition.Signals.User_Signal_Synchro(Ref);
		_FIP_ACK_EOC (  Ref  ) ;
	}

	if ( (LIRE(UState) & IRQ) != 0) {
		traitement_de_l_IRQ ( Ref ) ;
	}
	return  ( Ushort)
		( ( ( (LIRE(UState) ^ EOC) & (IRQ | EOC)  ) != 0  ) ? 1 : 0 );

}

#endif
