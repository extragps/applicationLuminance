/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4
  Nom du fichier  : fdmbamed.c
  Description     : Procedures de gestion du BA et du Medium

  Date de creation: Fri Mar 15 15:11:21 MET 1996
Version:4.1.4
	a/-filtrage different pour presence trafic sur voie
		seuil max si pas TST_P sur voie
Version:4.2.1
	a/-possibilite arret BA par FIPCODE pendant TST_P
	   alors pas de remise etat de Base_0 actif
Version:4.2.1
	a/- gestion medium nouveau mode ne pas faire de raz de WD sur FIPDUAL
	b/- gestion medium nouveau mode remonte des defauts  FIPDUAL
Version:4.3.10
	a/- Redondance medium nouveau mode :
		variable de rapport erreurs emission
Version:4.3.18
	a/- Modif seuils pour arret BA

Version:4.5.0
	a/- Tempo 31.25
	b/ modifs pour portage sur cible DSP texas

Version:4.8.0
	a/- invalidation voie 1 minute sur panne
	b/- option de compil FDM_WITH_LEAVE_BA_ON_DEFAULT

Version:4.9.0
	a/- option de compil FDM_WITH_OPTIMIZED_BA

***********************************************************/
#include "user_opt.h"
#include "fdm.h"
#include "fdmmacro.h"

/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
&&&&&&&&&&&&& Procedures decrites concernant le BA  &&&&&&&&&&&&&&&&&&&&&&&
services RefHandler
static Ushort _FIP_STOP_BA     ( FDM_REF *);
Ushort _FIP_START_BA    ( FDM_REF *, Ushort  Adr_next_macrocycle );
services de la classe Bus_Arbitrator
Ushort   fdm_ba_commute_macrocycle( FDM_BA_REF * );
Ushort   fdm_ba_start             ( FDM_BA_REF *  );
services de la classe Ref_Device_Manager
Ushort   fdm_ba_external_resync( FDM_REF *);
Ushort   fdm_ba_set_priority   ( FDM_REF * , Uchar );
Ushort   fdm_ba_set_parameters ( FDM_REF * , int, Uchar,  Uchar );
Ushort   fdm_ba_status         ( FDM_REF * ,BA_INF_STATUS * );
Ushort   fdm_ba_stop           ( FDM_REF *  );
&&&&&&&&&&&   Procedures decrites concernant le medium  &&&&&&&&&&&&&&&&&&&&&&&
 procedures internes
static Ushort FIP_GET_MEDIUM(FDM_REF *,FIP_ETAT_RESEAU *);
static Ushort _FIP_VALID_MEDIUM         ( FDM_REF *  );
static void _FDM_WRITE_REPORT_VARIABLE ( FDM_REF *   , int );
static void FIPDRIVE(FDM_REF *);
static Ushort internal_ba_set_priority  ( FDM_REF *Fip  , Uchar Priority_level )
&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
#if							(  FDM_WITH_BA == YES )

#if ( FDM_WITH_LEAVE_BA_ON_DEFAULT == YES )
static Ushort internal_ba_set_priority  ( FDM_REF *Fip  , Uchar Priority_level );
#endif

static Ushort
_FIP_SET_BA_PARAMS  ( FDM_REF *Ref  )
{
#define CMDE_CHANGE_PARAM_TAILLE 8
#if		( FDM_WITH_FREE_ACCESS == NO )
	int tempo;
	FDM_ERROR_CODE CR ;
	unsigned int taille;
	Ushort *src ;
	FIFO_Reg;
	Debut_procedure ;
	WAIT_Busy;
	/************ ecriture de la commande */
	ECRITE(UCom ,  CMDE_CHANGE_PARAM);
	taille = CMDE_CHANGE_PARAM_TAILLE ;
	FIFO_port = FIFO;
	src = &Ref->FDM_Internal_Use.Page_2000.BA_Num_Abonne;

	/************ attente bit SV */
	WAIT_SV;
	/************ boucle d'ecriture */
	BLOCKOUTWORD(FIFO_port,src,taille);
	/************ compte rendu */
	CR_UState;
	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;
	Fin_procedure;
#else
	Ushort volatile *FifoUser;
	Ushort volatile *FifoFip ;
	int  tempo;
	Ushort Tmp ,CR;
	Ushort volatile * S;
	Ushort volatile * D;
	int i;
	Dial_AE *PDial_AE;

	CR = 0;
	S = &Ref->FDM_Internal_Use.Page_2000.BA_Num_Abonne;
    	D =  Ref->H.FREE_ACCES_ADDRESS ;
	D +=0x2024;
	for (i = CMDE_CHANGE_PARAM_TAILLE ; i!=0 ; i--){
		*D++ = *S++;
	}
	MAC__INIT_REG;
	MAC__WAIT_PLACE;
	MAC__ECRIRE_DEMANDE( CHANGE_PARAM_USER , 0 );
	OS_Leave_Region();
abortd:
	return CR;
#endif
}
/****************************************************************/
/*                                                              */
/*		   SERVICES   FIPHANDLER			*/
/*                                                              */
/****************************************************************/
static Ushort
_FIP_STOP_BA          ( FDM_REF *Ref  )
{
#if		( FDM_WITH_FREE_ACCESS == NO )
	int tempo;
	FDM_ERROR_CODE CR ;
	Debut_procedure ;
	WAIT_Busy;
	/************ ecriture de la commande */
	ECRITE(UCom , CMDE_STOP_BA );
	/************ attente bit SV */
	WAIT_SV;
	_FIP__tres_petite_tempo ( Ref ) ;
	CR_Var_State( LIRE(Var_State) ) ;
	/************ compte rendu */
	CR_UState;
	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;
	Fin_procedure;
#else
	volatile Ushort  *FifoUser, *FifoFip ;
	int  tempo;
	Ushort Tmp ,CR;
	Dial_AE *PDial_AE;

	CR =0;
	MAC__INIT_REG;
	MAC__WAIT_PLACE;
	MAC__ECRIRE_DEMANDE( STOP_BA_USER , 0 );
	OS_Leave_Region();

abortd:
	return CR;
#endif
}

Ushort
_FIP_START_BA  ( FDM_REF *Ref , Ushort  Adr_next_macrocycle )
{
#if		( FDM_WITH_FREE_ACCESS == NO )
	int tempo ;
	FDM_ERROR_CODE CR ;
	Debut_procedure ;
	WAIT_Busy;
	ECRIRE_Key( Adr_next_macrocycle );
	/************ ecriture de la commande */
	ECRITE(UCom , CMDE_START_BA );
	/************ attente bit SV */
	WAIT_SV;
	_FIP__tres_petite_tempo ( Ref ) ;
	CR_Var_State( LIRE(Var_State) ) ;
	/************ compte rendu */
	CR_UState;
	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;
	Fin_procedure;
#else
	Ushort  volatile *FifoUser, *FifoFip ;
	Dial_AE *PDial_AE;
	int  tempo;
	Ushort Tmp ,CR;

	CR =0;
	MAC__INIT_REG;
	MAC__WAIT_PLACE;
	MAC__ECRIRE_DEMANDE( START_BA_USER , Adr_next_macrocycle );
	OS_Leave_Region();
abortd:
	return CR;
#endif
}


/*---------------------------------------------------
		Synchronisation du BA -
		 acces classiques uniquement
----------------------------------------------------*/
#if  (  FDM_WITH_FREE_ACCESS == NO )
/*====================================================================*/
/*=================== fdm_ba_external_resync==========================*/

Ushort
fdm_ba_external_resync( FDM_REF *Ref)
{
	int tempo ;
	FDM_ERROR_CODE CR ;
	if ( Ref->FDM_Internal_Use.Ba.BA_in_use == 0) return FDM_NOK;
	Debut_procedure ;
	WAIT_Busy;
	/************ ecriture de la commande */
	ECRITE(UCom ,CMDE_SYNC_BA );
	/************ attente bit SV */
	WAIT_SV;
	/************ compte rendu */
	CR_UState;
	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;
	Fin_procedure;
}
#endif

/****************************************************************/
/*                                                              */
/*	  SERVICES   FIPMANAGER CONCERNANT le BA		*/
/*                                                              */
/****************************************************************/
Ushort
fdm_ba_commute_macrocycle          ( FDM_BA_REF *BA )
{
	FDM_REF *Ref;
#if  (  FDM_WITH_FREE_ACCESS == NO )
	int tempo ;
	FDM_ERROR_CODE CR ;
	int A;
	Ref = BA->Fip;
	Ref->FDM_Internal_Use.Synchro_BA.MC_High =  (Uchar)(BA->Label>>8);
   	_FIP_WRITE_VAR(Ref,  CLE_SYNCHRO_BA_PROD , &(Ref->FDM_Internal_Use.Synchro_BA) );
	Debut_procedure ;
	WAIT_Busy;
	ECRIRE_Key( BA->Macro_Cycle_Adr );
	/************ ecriture de la commande */
	ECRITE(UCom , CMDE_CHANGE_MAC );
	/************ attente bit SV */
	WAIT_SV;
	/************ compte rendu */
	CR_UState;
	_FIP__tres_petite_tempo ( Ref ) ;
	A= LIRE(Var_State);
	if (A ==0)
	   Ref->FDM_Internal_Use.Ba.BA_in_use =BA;
	CR_Var_State(A) ;
	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;
#else
	volatile Ushort  *FifoUser, *FifoFip ;
	Dial_AE *PDial_AE ;
	int  tempo;
	Ushort Tmp ,CR;

	CR = 0;
	Ref = BA->Fip;
	Ref->FDM_Internal_Use.Synchro_BA.MC_High =  (Uchar)(BA->Label>>8);
   	_FIP_WRITE_VAR(Ref,  CLE_SYNCHRO_BA_PROD , &(Ref->FDM_Internal_Use.Synchro_BA) );
	MAC__INIT_REG;
	MAC__WAIT_PLACE;
	MAC__ECRIRE_DEMANDE(  CHANGE_MACRO_USER ,   BA->Macro_Cycle_Adr );
	Ref->FDM_Internal_Use.Ba.BA_in_use =BA;
	OS_Leave_Region();

abortd:
	return CR;
#endif
}


/*====================================================================*/
/*=================== fdm_ba_start          ==========================*/

Ushort
fdm_ba_start   ( FDM_BA_REF *Ba_ref  )
{
	FDM_REF *Fip;
	Ushort  CR=0;
	Fip = Ba_ref->Fip ;
	Fip->FDM_Internal_Use.Ba.cpt_timeout_suspend = 0;
	Fip->FDM_Internal_Use.Synchro_BA.MC_High =  (Uchar)(Ba_ref->Label>>8);
   	_FIP_WRITE_VAR( Fip ,  CLE_SYNCHRO_BA_PROD , &(Fip->FDM_Internal_Use.Synchro_BA) );
#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
	CR  = _FIP_DEF_EVT( Fip , CLE_PRESENCE_C, Receive_Evt_permanent);
	CR |= _FIP_DEF_EVT( Fip , CLE_PRESENCE, Send_Evt_permanent);
	Fip->FDM_Internal_Use.Ba.Flag_voie_test   = 1;
	Fip->FDM_Internal_Use.Ba.Flag_traitement  = 1;
	/* invalidation de la var Liste des Presents */
	Fip->FDM_Internal_Use.Ba.cpt_nbre_tst_p = 0 ;
#endif

	Fip->FDM_Internal_Use.Ba.B0_tst_p  = ( FDM_ADR_USER>> 10 );
#if								(FDM_WITH_FIPIO == YES )
	Fip->FDM_Internal_Use.Ba.B0_tst_p  |= 0x1000;
#endif
	CR |=    _FIP_START_BA (Fip , Ba_ref->Macro_Cycle_Adr);
	if ((CR & 0x8000 ) == 0)
		Fip->FDM_Internal_Use.Ba.BA_in_use = Ba_ref;

	return  ( (Ushort)((CR==0) ? FDM_OK :  FDM_NOK)) ;
}
/*====================================================================*/
/*===================     fdm_ba_stop       ==========================*/
Ushort fdm_ba_stop ( FDM_REF *Ref  )
{
        struct MediumDef *MEDIUM;
	Ushort  CR=0;
	MEDIUM = &Ref->FDM_Internal_Use.Medium;
	Ref->FDM_Internal_Use.Ba.BA_actif       = 0;
	Ref->FDM_Internal_Use.Ba.BA_running     = 0;
#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
	CR  = _FIP_DEF_EVT(Ref, CLE_PRESENCE_C, 0 );
	CR |= _FIP_DEF_EVT(Ref, CLE_PRESENCE, 0 );
#endif
	CR |= _FIP_STOP_BA(Ref);
	 _FIP_WRITE_PHY(Ref, FIP_CONF_RESEAU  ,
			&Ref->FDM_Internal_Use.Page_2000.Base_0 ,2);
	Ref->FDM_Internal_Use.Ba.tst_p_en_cours    = 0;
	MEDIUM->Voie_en_cours = 3 ;
	Ref->FDM_Internal_Use.Ba.BA_in_use        =  NULL;
	if (CR==0) return(FDM_OK);
	else return (FDM_NOK);
}

/*====================================================================*/
/*===================     fdm_ba_status    ====================*/
/* return FDM_NOK  si defaut procedure fiphandler
 adr_inf ->actif_running = 0 si inexistant
 			 = 1 si actif
 			 = 3 si running
*/
Ushort
fdm_ba_status  ( FDM_REF *Ref ,BA_INF_STATUS *adr_inf )
{

      if (Ref->FDM_Internal_Use.Ba.BA_in_use == NULL){
    		/* le ba n'existe pas */
       adr_inf->Actif_Running     = NOT_BA;
       adr_inf->Status_Fiphandler= 0;
       adr_inf->Label            = 0;
       return(FDM_OK);
    }
   adr_inf->Actif_Running    = LOADED_BA ;
   adr_inf->Label            = Ref->FDM_Internal_Use.Ba.BA_in_use->Label;
   adr_inf->Status_Fiphandler= Ref->FDM_Internal_Use.Ba.Status ;
   if (Ref->FDM_Internal_Use.Ba.BA_actif)
  	 adr_inf->Actif_Running   = ACTIF;
   if (Ref->FDM_Internal_Use.Ba.BA_running)
         adr_inf->Actif_Running  =  RUNNING;
    return (FDM_OK);
}

#endif


/**************************************************************************/
/*************************** Procedures Internes **************************/
/**************************************************************************/
/*------------------------------------------------------------*/
#if             (  FDM_WITH_REDONDANCY_MGNT == NO )
static Ushort _FIP_VALID_MEDIUM  ( FDM_REF *Ref , Ushort  cmde_fieldual )
{
#if 	(  FDM_WITH_FREE_ACCESS == NO )
	int tempo ;
	FDM_ERROR_CODE CR ;
	Debut_procedure ;
	WAIT_Busy;
	ECRIRE_Key(cmde_fieldual);
	/************ ecriture de la commande */
	ECRITE(UCom , CMDE_VALID_MEDIUM );
	/************ attente bit SV */
	WAIT_SV;
	_FIP__tres_petite_tempo ( Ref ) ;
	CR_Var_State( LIRE(Var_State) ) ;
	/************ compte rendu */
	CR_UState;
	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;
	Fin_procedure;
#else
	Ushort   *FifoUser, *FifoFip ;
	int  tempo;
	Ushort Tmp ,CR;
	Dial_AE *PDial_AE;

	CR =0;
	MAC__INIT_REG;
	MAC__WAIT_PLACE;
	MAC__ECRIRE_DEMANDE(  VALID_MEDIUM_USER ,  cmde_fieldual  );
	OS_Leave_Region();

abortd:
	return CR;
#endif
}
#endif

/*------------------------------------------------------------*/
static Ushort
FIP_GET_MEDIUM(FDM_REF *Ref,FIP_ETAT_RESEAU *tab_fip_etat_reseau)
{
		/* procedure de lecture de l'etat du reseau */
#if 	(  FDM_WITH_FREE_ACCESS == NO )

	int tempo ;
	FDM_ERROR_CODE CR ;
   	FIFO_Reg;
	register unsigned int taille;
 	register Ushort *dst;
	Debut_procedure;
	WAIT_Busy;
	/************ ecriture de la commande */
	ECRITE(UCom , CMDE_GET_MEDIUM);
	/* ici pour occuper utilement le micro */
	FIFO_port = FIFO;
	dst = (Ushort *)tab_fip_etat_reseau ;
        taille = sizeof( FIP_ETAT_RESEAU ) /sizeof(Ushort) ;
	/************ attente bit SV */
	WAIT_SV;
	/************ boucle de lecture */
	BLOCKINWORD(FIFO_port,dst,taille );
	/************ compte rendu */
	CR_UState;
	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;
	Fin_procedure ;
#else
	Ushort volatile *D ;
	Ushort volatile *S ;
	Ushort volatile *FifoUser;
	Ushort volatile *FifoFip ;
	int i;
	int  tempo;
	Ushort CR  ;
	Ushort Tmp;
	Dial_AE *PDial_AE;

	CR =   0 ;
	MAC__INIT_REG;

	MAC__WAIT_PLACE	;

	MAC__ECRIRE_DEMANDE(  GET_MEDIUM_USER,0);
	D = (Ushort*)tab_fip_etat_reseau	;
    	S =  Ref->H.FREE_ACCES_ADDRESS ;
	S +=0x80;
	MAC__WAIT_LECTURE_PRETE;
	OS_Leave_Region();
	for (i = sizeof( FIP_ETAT_RESEAU )/2 ; i != 0 ; i--){
		*D++ = *S++;
	}
	return CR ;
abort:
	OS_Leave_Region();
abortd:
	return CR ;
#endif
}

/*-----------------------------------------------------------------------*/
#define NB_COUNTERS 7
typedef struct {
	Ushort V[NB_COUNTERS];	/*.....  modif V4.0 .....*/
	}ECART;
/*--------------------------------------------------------------------*/
/*--------------   _FDM_WRITE_REPORT_VARIABLE      -------------------*/
/*--------------------------------------------------------------------*/
/* ecriture de la variable de rapport
     descripteur de la variable de rapport = CLE_REPORT
*/
static void
_FDM_WRITE_REPORT_VARIABLE ( FDM_REF * Ref  , int Bilan)
{
   unsigned short K;
   struct MediumDef *MEDIUM;
   MEDIUM = &Ref->FDM_Internal_Use.Medium;
   K=  MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1;
   Ref->FDM_Internal_Use.Report_Variable.V0_ok[0] =(unsigned char)( K>>8 );
   Ref->FDM_Internal_Use.Report_Variable.V0_ok[1] =(unsigned char) K;
   K=  MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2;
   Ref->FDM_Internal_Use.Report_Variable.V1_ok[0] =(unsigned char)( K>>8 );
   Ref->FDM_Internal_Use.Report_Variable.V1_ok[1] =(unsigned char) K;
   K=  MEDIUM->defaut_fip.Nb_Of_Frames_Nok_1;
   Ref->FDM_Internal_Use.Report_Variable.V0_hs[0] =(unsigned char)( K>>8 );
   Ref->FDM_Internal_Use.Report_Variable.V0_hs[1] =(unsigned char) K;
   K=  MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2;
   Ref->FDM_Internal_Use.Report_Variable.V1_hs[0] =(unsigned char)( K>>8 );
   Ref->FDM_Internal_Use.Report_Variable.V1_hs[1] =(unsigned char) K;
   Bilan |= MEDIUM->memo_EWD ;
   Ref->FDM_Internal_Use.Report_Variable.bilan[0] = (unsigned char) (Bilan>>8) ;
   Ref->FDM_Internal_Use.Report_Variable.bilan[1] = (unsigned char)Bilan ;
   _FIP_WRITE_VAR(Ref, CLE_REPORT , &(Ref->FDM_Internal_Use.Report_Variable) );
}


#if		(  FDM_WITH_BI_MEDIUM == YES )
#if             (  FDM_WITH_REDONDANCY_MGNT == YES )
/*-----------------------------------------------------------------------*/
/*definition des registers et des modes d'acces  utilises pour FIPDRIVE
  option FDM_WITH_REDONDANCY_MGNT =YES uniquement */
#if 		 ( FDM_WITH_CHAMP_IO == YES )
#define AD_LOC0   Ref->H.LOC_FIPDRIVE[0]
#define AD_LOC1   Ref->H.LOC_FIPDRIVE[1]
#define AD_LOC2   Ref->H.LOC_FIPDRIVE[2]
#define AD_LOC3   Ref->H.LOC_FIPDRIVE[3]
#else
#define  AD_LOC0   * Ref->H.LOC_FIPDRIVE[0]
#define  AD_LOC1   * Ref->H.LOC_FIPDRIVE[1]
#define  AD_LOC2   * Ref->H.LOC_FIPDRIVE[2]
#define  AD_LOC3   * Ref->H.LOC_FIPDRIVE[3]
#endif
#endif

Ushort
fdm_ack_ewd_medium (FDM_REF *Ref, enum _MEDIUM_DEF  Medium)
{
struct MediumDef *MEDIUM;
MEDIUM = &Ref->FDM_Internal_Use.Medium;
if (MEDIUM->double_medium ==0 ) return 0xffff;
MEDIUM->memo_EWD_CDE |= Medium << 4;
return 0x0;
}
/*---------------------------------------------------------------------*/
/*		         FIPDRIVE				       */
/*---------------------------------------------------------------------*/
/* Fipdrive est appele dans TEST_MEDIUM  ( redondance gere par user ou non)
	               dans TESTP_DEBUT  ( redondance gere par user uniquement)
	               dans TESTP_FIN    ( redondance gere par user uniquement)
pour commande fiepdual : b1b0 =	01	-> voie 1 valide
				10	-> voie 1 invalide
			 b3b2 =	01	-> voie 2 valide
				10	-> voie 2 invalide
			 b4   = 1	-> raz erreurs transmission
			 b5   = 1	-> raz defauts WD ligne 1
			 b6   = 1	-> raz defauts WD ligne 2
*/
static void
FIPDRIVE(FDM_REF *Ref)
{
struct MediumDef *MEDIUM;
MEDIUM = &Ref->FDM_Internal_Use.Medium;
if (MEDIUM->double_medium ==0 ) return;
#if             (  FDM_WITH_REDONDANCY_MGNT == YES )
{
	int V;
	int T;
	volatile int Tx;
	if (MEDIUM->Voie_en_cours ==3){
		V = MEDIUM->Voie_autorisee ;
	}else{
		V = MEDIUM->Voie_en_cours;
	}
	T = LIRE (AD_LOC0);
	OS_Enter_Region();
	MEDIUM->memo_drive |=( T >>2)  & 3 ;
	OS_Leave_Region();
	if ((T & 0x10) !=0 ) {
		if ( ( MEDIUM->memo_EWD & 0x400 ) == 0)
			FDM_Signal_Warning ( Ref  , _MEDIUM_EWD_FAILED  , 1 ) ;
		MEDIUM->memo_EWD |= 0x400 ;
	}
        if ((T & 0x20) !=0 ) {
		if ( ( MEDIUM->memo_EWD & 0x800 ) == 0)
			FDM_Signal_Warning ( Ref  , _MEDIUM_EWD_FAILED  , 2 ) ;
		MEDIUM->memo_EWD |= 0x800 ;
	}
	if ( MEDIUM->memo_EWD_CDE != 0 ) {
		if (( MEDIUM->memo_EWD_CDE & 0x10) !=0 )
		        Tx = LIRE(AD_LOC2);   /*raz erreur ewd1*/
		if (( MEDIUM->memo_EWD_CDE & 0x20) !=0 )
		        Tx = LIRE(AD_LOC3);   /*raz erreur ewd2*/
		MEDIUM->memo_EWD &= ~(MEDIUM->memo_EWD_CDE << 6 );
        	MEDIUM->memo_EWD_CDE = 0 ;
	}
    	if ((T & 0xc ) !=0 ) Tx = LIRE(AD_LOC1);   /*raz erreur */
    	switch ( V ){
     	case 0 :
     		ECRITE(AD_LOC0, 0 );	/* mode rebouclage */
      		break;
     	case 1 :
     		ECRITE(AD_LOC1, 1 );
     	     	break;
      	case 2 :
     		ECRITE(AD_LOC2, 2 );
     	     	break;
     	case 3 :
     		ECRITE(AD_LOC3, 3 );
     	     	break;
     	}
}
#else
{/* cas redondance par fipcode : */
  Ushort cmd_voie ;
  Ushort defaut ;

	if ((MEDIUM->FIELDUAL_Status & 0x10) !=0 ) {
		if ( ( MEDIUM->memo_EWD & 0x400 ) == 0)
			FDM_Signal_Warning ( Ref  , _MEDIUM_EWD_FAILED  , 1 ) ;
		MEDIUM->memo_EWD |= 0x400 ;
	}
        if ((MEDIUM->FIELDUAL_Status & 0x20) !=0 ) {
		if ( ( MEDIUM->memo_EWD & 0x800 ) == 0)
			FDM_Signal_Warning ( Ref  , _MEDIUM_EWD_FAILED  , 2 ) ;
		MEDIUM->memo_EWD |= 0x800 ;
	}

	defaut = MEDIUM->memo_EWD_CDE | 0x100;
 	MEDIUM->memo_EWD_CDE = 0 ;


	switch (  MEDIUM->Voie_autorisee ){
     	case 0 :
     		break;
     	case 1 :
		cmd_voie =0x9 | defaut ;
     		 _FIP_VALID_MEDIUM  ( Ref , cmd_voie );
     	     	break;
      	case 2 :
 		cmd_voie =0x6 | defaut ;
        	 _FIP_VALID_MEDIUM  ( Ref , cmd_voie);
       	     	break;
    	case 3 :
 		cmd_voie =0x5 | defaut ;
       		 _FIP_VALID_MEDIUM  ( Ref , cmd_voie);
    	     	break;
     }
  }
#endif
}
#endif

/****************************************************************/
/*                                                              */
/*	  SERVICES    CONCERNANT le MEDIUM		        */
/*                                                              */
/****************************************************************/
/* Nota : si l'option FDM_WITH_BI_MEDIUM n'est pas positionne , ou
          si dans configuration hard bit TWO_BUS_MODE =0
          on est donc mono medium , le 2 ieme parametre est non significatif
*/
Ushort
fdm_valid_medium ( FDM_REF *Ref ,enum _MEDIUM_DEF Voie_desiree )
{
  struct MediumDef *MEDIUM;
  MEDIUM = &Ref->FDM_Internal_Use.Medium;
  MEDIUM->defaut_test_p[0] 	= 0 ;
  MEDIUM->defaut_test_p[1] 	= 0 ;
  MEDIUM->defaut_emission[0] 	= 0 ;
  MEDIUM->defaut_emission[1] 	= 0 ;
  MEDIUM->defaut_reception[0] 	= 1 ;
  MEDIUM->defaut_reception[1] 	= 1 ;
  MEDIUM->defaut_voie[0] 	= 14 ;
  MEDIUM->defaut_voie[1] 	= 14 ;

#if		(  FDM_WITH_BI_MEDIUM == YES )
  MEDIUM->double_medium  = ((Ref->S.Type & TWO_BUS_MODE ) == 0) ? 0 : 1 ;
  if  ( MEDIUM->double_medium  ==0){
	/* cas mono medium */
   	MEDIUM->Voie_en_cours  = 1;
	MEDIUM->Voie_Valide    = 1;
 	MEDIUM->Voie_autorisee = 1;
  }else{
	/* cas Bi medium */
       /* on demarre avec les 2 voies (TST_P)*/
 	 MEDIUM->Voie_en_cours  = 3;
	 MEDIUM->Voie_Valide    = (int)Voie_desiree;
 	 MEDIUM->Voie_autorisee = (int)Voie_desiree;
  }

#if             (  FDM_WITH_REDONDANCY_MGNT == YES )
  FIPDRIVE(Ref);
#else
    switch ( Voie_desiree ){
     	case _MEDIUM_1 :
		/* 001 1001 = voie 1 valide , voie 2 invalide */
     		 _FIP_VALID_MEDIUM  ( Ref , (Ushort)0x19 );
     	     	break;
      	case _MEDIUM_2 :
		/* 111 0110 = voie 1 invalide , voie 2 valide */
      		 _FIP_VALID_MEDIUM  ( Ref ,  (Ushort)0x16 );
       	     	break;
    	case _MEDIUM_1_2 :
 		/* 111 0101 = voie 1 valide , voie 2 valide */
     		 _FIP_VALID_MEDIUM  ( Ref ,  (Ushort)0x15 );
    	     	break;
    }
#endif
#else
	/* cas OPTION FDM_WITH_BI_MEDIUM=NO */
  MEDIUM->Voie_en_cours  = 1;
  MEDIUM->Voie_Valide    = 1;
  MEDIUM->Voie_autorisee = 1;
#endif
  return 0;
}

#define BruitOff 	0
#define BruitOn		1

Ushort _FIP_BRUIT          ( FDM_REF *Ref , short BruitOnOff )
{
	switch (Ref->S.Mode ){
		case  	WORLD_FIP_31:
		case  	FIP_31:
			return 0;
		default : 
			break;
	}

#if  (  FDM_WITH_FREE_ACCESS == NO )
	{
		int tempo ;
		FDM_ERROR_CODE CR ;
	
		Debut_procedure ;

		WAIT_Busy;

		/************ ecriture de la cle d'acces */
		ECRIRE_Key( BruitOnOff );

		/************ ecriture de la commande */
		ECRITE(UCom , CMDE_GES_BRUIT    );

		/************ attente bit SV */
		WAIT_SV;

		/************ compte rendu */
		CR_UState;

		/************ close */
  	      ECRITE(UCom,CMDE_CLOSE) ;

		Fin_procedure;
	}
#else
	/***********************************************/
	/**************** Acces libre ******************/
	/**************** _FIP_BRUIT   *****************/
	/***********************************************/
	{
		volatile Ushort   *FifoUser, *FifoFip ;
		Dial_AE *PDial_AE;
		int  tempo;
		Ushort CR ;
		Ushort Tmp ;


		CR = 0;

		MAC__INIT_REG;

		MAC__WAIT_PLACE;
		MAC__ECRIRE_DEMANDE( GESTION_BRUIT_USER , BruitOnOff );

		OS_Leave_Region();
abortd:
		return CR ;
	}
#endif
}



/*---------------------------------------------------------------------*/
/*=----------------	_FIP_TEST_MEDIUM :                 -----------*/
/*---------------------------------------------------------------------*/
/* cette procedure doit etre appelee periodiquement
1 fois / 200 ms a 2.5  Mhz
1 fois / 500 ms a 1    Mhz
1 fois /1600 ms a 31.25Khz
*/

void
_FIP_TEST_MEDIUM(_FDM_XAE_Tempo_Ref *T)
{
   /* lecture statistiques
       - appel de la procedure FIP_GET_MEDIUM
       - mise a jour des infos BA en fonction etat_reseau lu
       - mise a jour de memo_drive
    .........................................................*/
   int i;
   int conclusion_voies;
   int Voie;
   int M5;
   FDM_REF *Ref;
   FIP_ETAT_RESEAU  tab_fip_etat_reseau;
   int Flag_arret_BA ;
   int threshold ;
   struct MediumDef *MEDIUM;
   int Div;
#if		(  FDM_WITH_BI_MEDIUM == YES )
   int st_drive;
#endif
Ref = T->Fip;
MEDIUM = &Ref->FDM_Internal_Use.Medium;

{  /**[**/
    ECART            *adr;
    ECART            *adr_old;
    ECART            *tab;
  	FIP_GET_MEDIUM  (Ref,&tab_fip_etat_reseau);
	tab  =(ECART*)&tab_fip_etat_reseau ;
      	adr  =(ECART*)&(MEDIUM->defaut_fip);
      	adr_old =
		(ECART*)&(MEDIUM->defaut_fip_valeur_courante);
      	for (i =0 ; i < NB_COUNTERS ; i++){
             	adr->V[i]      = (tab->V[i] -adr_old->V[i]);
        	adr_old->V[i]  = tab->V[i] ;
    	}
#if		(  FDM_WITH_BA == YES )
	Ref->FDM_Internal_Use.Ba.Status =
		tab_fip_etat_reseau.Etat_Reseau.BA_Status;
 	Ref->FDM_Internal_Use.Ba.Current_Macrocycle =
		tab_fip_etat_reseau.Etat_Reseau.BA_Current_Macrocycle;
	Ref->FDM_Internal_Use.Ba.BA_actif =
		( tab_fip_etat_reseau.Etat_Reseau.BA_Status == 0 ) ? 0 : 1 ;
	{
		int Nouveau_Status ;

		if ( Ref->FDM_Internal_Use.Ba.BA_in_use  != NULL ) {
 	 	    	if (( tab_fip_etat_reseau.Etat_Reseau.BA_Status  & 0x8000 ) != 0 ) {
				Ref->FDM_Internal_Use.Ba.BA_running = 1;
  		    		Nouveau_Status = _BA_RUN ;
 			}
			else {
				Ref->FDM_Internal_Use.Ba.BA_running = 0 ;
  		    		Nouveau_Status = _BA_VEILLE ;
  		  	}
		} else
   		   	 Nouveau_Status = _BA_STOP ;
   	   	if ( Nouveau_Status != ( Ref->FDM_Internal_Use.Presence.Status & 0xf0 )) {
			Ref->FDM_Internal_Use.Presence.Status = (Uchar)
				(( Ref->FDM_Internal_Use.Presence.Status & 0x0f ) | Nouveau_Status );
			_FIP_WRITE_VAR(Ref,CLE_PRESENCE,(void*)&Ref->FDM_Internal_Use.Presence);
		}
		if (( Nouveau_Status == _BA_STOP ) || ( Nouveau_Status == _BA_VEILLE )) {
			/* on force base0 = base_user */
		        _FIP_WRITE_PHY(Ref, FIP_CONF_RESEAU  , &Ref->FDM_Internal_Use.Page_2000.Base_0 ,2);
		}
	}
#endif
} /**]**/

#if (FDM_WITH_REDONDANCY_MGNT == NO )
{ /**[**/
	int i;
	MEDIUM->memo_drive = (int)tab_fip_etat_reseau.Etat_Reseau.FIELDUAL_Status ;
	MEDIUM->memo_drive = ( MEDIUM->memo_drive >> 2 ) & 3;
        i   =   MEDIUM->defaut_tst_p[0];
        MEDIUM->defaut_tst_p[0] = i + ((MEDIUM->defaut_fip.Error_Testp_Channel1 - i )>>1);
        i   =   MEDIUM->defaut_tst_p[1];
        MEDIUM->defaut_tst_p[1] = i + ((MEDIUM->defaut_fip.Error_Testp_Channel2 - i )>>1);
 	i  = (MEDIUM->defaut_tst_p[0] > 2 ) ? 0x01 : 0;
	i |= (MEDIUM->defaut_tst_p[1] > 2 ) ? 0x02 : 0;
	MEDIUM->memo_drive |= i;

        MEDIUM->FIELDUAL_Status = tab_fip_etat_reseau.Etat_Reseau.FIELDUAL_Status;
}/**]**/
#endif

#if		(  FDM_WITH_BI_MEDIUM == NO )
{/**[**/
   MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 += MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 ;
   MEDIUM->defaut_fip.Nb_Of_Frames_Nok_1     += MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2  ;
   MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 = 0 ;
   MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2 = 0 ;
   MEDIUM->Voie_autorisee = 1 ;
}/**]**/
#else			/*  FDM_WITH_BI_MEDIUM =YES */
{/**[**/
   if ( MEDIUM->double_medium == 0 )    {
      	MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 += MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 ;
      	MEDIUM->defaut_fip.Nb_Of_Frames_Nok_1     += MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2  ;
      	MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 	= 0 ;
      	MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2  		= 0 ;
      	MEDIUM->Voie_autorisee 				= 1 ;
      	st_drive  = 1 ;
   } else {
       	if (MEDIUM->Voie_Valide != 3 ) {
          	if ( MEDIUM->Voie_Valide == 2 ){
	  	 	 MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 	= 0 ;
	  	 	 MEDIUM->defaut_fip.Nb_Of_Frames_Nok_1		= 0 ;
          	} else {
	   		 MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 	= 0 ;
	   		 MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2 		= 0 ;
          	}
        }
	{
		int i,v,Z;
      		/* defaut emission */
		OS_Enter_Region();
		Z = MEDIUM->memo_drive;
		MEDIUM->memo_drive = 0;
		OS_Leave_Region();
		v = MEDIUM->defaut_emission[0];
      		i = (Z  & 1) <<7;
      		v = v + ( i-v)/4 ;
      		MEDIUM->defaut_emission[0] = v;
		v = MEDIUM->defaut_emission[1];
      		i = (Z  & 2) <<6;
      		v = v + ( i-v)/4 ;
      		MEDIUM->defaut_emission[1] = v;
      	}
	      	st_drive = 3;
		if (MEDIUM->defaut_emission[0] > 0x40 ) st_drive &= ~1 ;
		if (MEDIUM->defaut_emission[1] > 0x40 ) st_drive &= ~2 ;
}/**]**/
#endif
  /* defaut reception */
  	Div  = ( MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 == 0  ) ? 1 : MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 ;
	threshold = MEDIUM->defaut_fip.Nb_Of_Frames_Nok_1 / Div  ;
	if (  threshold > (int)Ref->S.Default_Medium_threshold ) {
		if (MEDIUM->defaut_reception[0] < 3 )  MEDIUM->defaut_reception[0]++;
		if (MEDIUM->defaut_reception[0] == 3 ) {
	 		_FIP_BRUIT (  Ref ,  BruitOff );
			MEDIUM->defaut_reception[0] = MEDIUM->duree_invalidation ;
		}
	}else{
		if (MEDIUM->defaut_reception[0] != 0 )  MEDIUM->defaut_reception[0]--;
	}

#if		(  FDM_WITH_BI_MEDIUM == NO )
   MEDIUM->defaut_reception[1] = 4 ;
   Voie = ((MEDIUM->defaut_reception[0] > 3 ) ? 0 : 1 ) ;
   conclusion_voies = MEDIUM->Voie_autorisee << 4;

#else
{/**[**/
  int  k=3;
  if ( MEDIUM->double_medium == 0 )    {
    	MEDIUM->defaut_reception[1] = 4 ;
    	Voie = ((MEDIUM->defaut_reception[0] > 3 ) ? 0 : 1) ;
    	conclusion_voies = MEDIUM->Voie_autorisee << 4 ;
  } else {
  	Div  = ( MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 == 0  ) ? 1 : MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 ;
	threshold = MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2 / Div  ;
	if (  threshold > (int)Ref->S.Default_Medium_threshold ) {
		if (MEDIUM->defaut_reception[1] < 3 )  MEDIUM->defaut_reception[1]++;
		if (MEDIUM->defaut_reception[1] == 3 ) {
	 		_FIP_BRUIT (  Ref ,  BruitOff );
			MEDIUM->defaut_reception[1] = MEDIUM->duree_invalidation ;
		}
	}else{
		if (MEDIUM->defaut_reception[1] != 0 )  MEDIUM->defaut_reception[1]--;
	}
  }

/* Controle du nombre de trame en erreur trop grand ( fipcode 6.8 ) */
  if (( MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 + MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2  +
        MEDIUM->defaut_fip.Nb_Of_Frames_Nok_1 + MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2 ) == 0 ) {
			MEDIUM->defaut_reception[0] = 0;
			MEDIUM->defaut_reception[1] = 0;
   } else {
	if (( MEDIUM->defaut_fip.Nb_Of_Frames_Nok_1 + MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2 ) > 50 ){
 		_FIP_BRUIT (  Ref ,  BruitOff );
		if (MEDIUM->defaut_fip.Nb_Of_Frames_Nok_2 > MEDIUM->defaut_fip.Nb_Of_Frames_Nok_1 ){
			MEDIUM->defaut_reception[0] = 0;
			MEDIUM->defaut_reception[1] = MEDIUM->duree_invalidation ;
		} else {
			MEDIUM->defaut_reception[0] = MEDIUM->duree_invalidation ;
			MEDIUM->defaut_reception[1] = 0;
		}
	}
   	if ((MEDIUM->defaut_reception[0] + MEDIUM->defaut_reception[1]) == 0){
		if ((MEDIUM->defaut_emission[0]> 0x40) || (MEDIUM->defaut_emission[1] > 0x40) ){
	 		_FIP_BRUIT (  Ref ,  BruitOff );
			if (MEDIUM->defaut_emission[0] < MEDIUM->defaut_emission[1] ){
				MEDIUM->defaut_reception[0] = 0;
				MEDIUM->defaut_reception[1] = MEDIUM->duree_invalidation ;
			} else {
				MEDIUM->defaut_reception[0] = MEDIUM->duree_invalidation ;
				MEDIUM->defaut_reception[1] = 0;
			}
		}
	}
   }


  Voie = ((MEDIUM->defaut_reception[0] > 3 ) ? 0 : 1 ) | ((MEDIUM->defaut_reception[1] > 3 ) ? 0 : 2 );
  {/**[[**/
    Flag_arret_BA  = 0;
    /* quelle voie choisir ? */
    switch (MEDIUM->Voie_Valide ) {
    case 0:
	k = 0 ;
	break;
    case 1:
	k = 1 ;
	break;
    case 2:
	k = 2 ;
	break;
    case 3:
	/* on ne peut invalider une voie que si l autre est correct */
	k = Voie & st_drive;
	switch ( MEDIUM->etat_precedant_des_voies ^ k ) {
	case 1:
	     if (( k & 1 ) == 0 ) {
		/* voie 1: bon -> hs */
 		if (MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 == 0)
	   		/* on refuse car pas de trafic voie 2*/
	   		 k = 1;
	   		 MEDIUM->defaut_reception[1] = 0;
	         }
	      break;
	case 2:
	     if (( k & 2 ) == 0 ) {
		/* voie 2: bon -> hs */
		 if (MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 == 0)
		  	 /* on refuse car pas de trafic voie 1*/
		  	  k = 2;
		  	  MEDIUM->defaut_reception[0] = 0;
	         }
	      break;
	 case 3:
	      /* les voies  : bon -> hs on remet les voies qui ont du trafic !!! */
		if (MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 != 0) k |= 1;
		if (MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 != 0) k |= 2;
	       break;
	}/* switch */
	break;
    } /* switch */
    } /*]]*/

    if (k == 0 ) k = 3; /* on ne sait plus rien !! */

    if ((k == 3) && (MEDIUM->etat_precedant_des_voies != 3) ){
	_FIP_BRUIT (  Ref ,  BruitOn );
	}

    MEDIUM->Voie_autorisee = k ;
    if ( MEDIUM->Voie_autorisee == 0 )
	MEDIUM->Voie_autorisee 		= MEDIUM->etat_precedant_des_voies;
    else
        MEDIUM->etat_precedant_des_voies= MEDIUM->Voie_autorisee ;


    { /*[[*/
	    int i;
	    i = (MEDIUM->etat_precedant_des_voies  & 1)  << 2;
	    MEDIUM->defaut_voie[0] =
                (MEDIUM->defaut_voie[0] + (i - (MEDIUM->defaut_voie[0]>>2)));
	    i = (MEDIUM->etat_precedant_des_voies & 2 )  << 1;
	    MEDIUM->defaut_voie[1] =
                (MEDIUM->defaut_voie[1] +  (i - (MEDIUM->defaut_voie[1]>>2)));
    }
    conclusion_voies =
		((MEDIUM->defaut_voie[0] > 10 ) ? 0x10 : 0 ) |
    		((MEDIUM->defaut_voie[1] > 10 ) ? 0x20 : 0 );
    } /*]]*/
} /*]*/
FIPDRIVE(Ref);
#endif
{ /*[*/
	/* eriture de la variable de rapport */
	{
	int i;
        i  =  ( MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_1 > 1 ) ? 120 : 0 ;
        MEDIUM->traffic[0] +=  ((i- MEDIUM->traffic[0] )/4);
        i  =  ( MEDIUM->defaut_fip.Nb_Of_Transaction_Ok_2 > 1 ) ? 120 : 0 ;
        MEDIUM->traffic[1] += ((i- MEDIUM->traffic[1] )/4);
	M5 = 3;
	if (MEDIUM->traffic[0] > 40 ) M5 |=  0x40;
	if (MEDIUM->traffic[1] > 40 ) M5 |=  0x80;
    if ( MEDIUM->defaut_emission[0] > 0x20 ) M5 &= ~1;
    if ( MEDIUM->defaut_emission[1] > 0x20 ) M5 &= ~2;
	}
	M5 |= conclusion_voies | ( Voie  << 2  );
	/* bits 8 et 9 indique si voie (i) == ok */
	{ int M6;
	M6 = ( M5 << 4 ) & ( M5 << 6 ) & ( M5 << 8 ) ;
/* if (FDM_WITH_REDONDANCY_MGNT == NO ) */
	if (  Ref->FDM_Internal_Use.Ba.BA_running == 0 )
/* #endif*/
		M6 &= ( M5 << 2 ) ;
	M6 &= 0x300 ;
	M5 |= M6;
	}
	MEDIUM->nb_tst_p = 0;
	_FDM_WRITE_REPORT_VARIABLE (Ref , M5 );
} /*]*/

#if			( FDM_WITH_BA == YES)
{ /*[*/
int M6;
if (Ref->FDM_Internal_Use.Ba.BA_in_use == NULL ) goto suite;

M6 = 3;
if ( MEDIUM->defaut_emission[0] > 0x60 ) M6 &= ~1;
if ( MEDIUM->defaut_emission[1] > 0x60 ) M6 &= ~2;
M5 = (M5 >> 2 ) & (M5 >> 4) & M6 & 3;

switch (M5)	{
case 0 :Flag_arret_BA = 1; break;
case 1 :
case 2 :
	{
#if ( FDM_WITH_LEAVE_BA_ON_DEFAULT == YES )
		if (Ref->FDM_Internal_Use.Page_2000.BA_Priority != Ref->FDM_Internal_Use.Ba.Max_priority )
		{
		   internal_ba_set_priority(Ref, (unsigned char)Ref->FDM_Internal_Use.Ba.Max_priority);
		   Ref->FDM_Internal_Use.Ba.Mode_degrade = 1;
		}
#endif
		Flag_arret_BA = 0;
		break;
	}
case 3 :
	{
#if ( FDM_WITH_LEAVE_BA_ON_DEFAULT == YES )
		if (Ref->FDM_Internal_Use.Page_2000.BA_Priority != Ref->FDM_Internal_Use.Ba.Current_priority )
		{
		   internal_ba_set_priority(Ref, (unsigned char)Ref->FDM_Internal_Use.Ba.Current_priority);
		   Ref->FDM_Internal_Use.Ba.Mode_degrade = 0;
		}
#endif
		Flag_arret_BA = 0;
		break;
	}
}

if ( Ref->FDM_Internal_Use.Ba.cpt_timeout_suspend != 0  ) Ref->FDM_Internal_Use.Ba.cpt_timeout_suspend--;

if ( Ref->FDM_Internal_Use.Ba.cpt_timeout_suspend > 200 ) {
	/* cette station ne merite pas d'etre BA : trop lente !!! */
	Ref->FDM_Internal_Use.Ba.cpt_timeout_suspend = 0;
	FDM_Signal_Warning ( Ref  , _BA_STOPPED_ON_TIME_OUT , 0) ;
	Ref->FDM_Internal_Use.Ba.BA_running = 0;
	Ref->FDM_Internal_Use.Ba.BA_actif   = 0;
	Ref->FDM_Internal_Use.Ba.BA_in_use  = NULL;
	goto arret_du_ba;
}
if ( Ref->FDM_Internal_Use.Ba.BA_actif == 1 ) {
	if (Flag_arret_BA == 1 ) {
		if (Ref->FDM_Internal_Use.Ba.BA_running == 1)
			goto arret_du_ba ;
	}
} else {
	if (Flag_arret_BA == 0 ) {
		if (Ref->FDM_Internal_Use.Ba.BA_running == 0)
			goto reprise_du_ba;
	}
}
goto suite;

arret_du_ba:
	_FIP_STOP_BA (Ref);
#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
	_FIP_DEF_EVT( Ref , CLE_PRESENCE_C, 0 );
 	_FIP_DEF_EVT( Ref , CLE_PRESENCE, 0 );
#endif
     _FIP_WRITE_PHY(Ref, FIP_CONF_RESEAU  , &Ref->FDM_Internal_Use.Page_2000.Base_0 ,2);
	 Ref->FDM_Internal_Use.Ba.tst_p_en_cours     = 0;
       	 Ref->FDM_Internal_Use.Ba.cpt_nbre_tst_p     = 0;
	 Ref->FDM_Internal_Use.Ba.B0_tst_p           = ( FDM_ADR_USER>> 10 );
         MEDIUM->Voie_en_cours  = 3 ;
	 goto suite;
reprise_du_ba:
#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
	_FIP_DEF_EVT( Ref , CLE_PRESENCE_C, Receive_Evt_permanent);
	_FIP_DEF_EVT( Ref , CLE_PRESENCE, Send_Evt_permanent);
#endif
       	_FIP_START_BA(Ref,Ref->FDM_Internal_Use.Ba.BA_in_use->Macro_Cycle_Adr);
suite:
;
} /*]*/

#endif

}



/*====================================================================*/
/*     traitement des evenements ba :
		-test presence ,
		-test_p debut
		-test_p fin
*/
/*====================================================================*/
#if							(  FDM_WITH_BA == YES )
/*====================================================================*/
/*===================     _fdm_ba_set_priority    ====================*/
Ushort
fdm_ba_set_priority  ( FDM_REF *Fip  , Uchar Priority_level )
{
    Ushort       CR;
    if (Priority_level >Fip->FDM_Internal_Use.Ba.Max_priority ) {
 	FDM_Signal_Warning ( Fip ,  _LEVEL_PRIORITY_BA,   0 ) ;
    	return FDM_NOK;
    }
    Fip->FDM_Internal_Use.Presence.Status =
    	       (Uchar)((Fip->FDM_Internal_Use.Presence.Status & 0xf0 ) | Priority_level);

    Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par =
             (Ushort) ( 2 * ( ((Fip->FDM_Internal_Use.Page_2000.BA_Nr_Max+1)*(Priority_level))
                  + Fip->FDM_Internal_Use.Ba.seuil_TSTP  + Fip->FDM_Internal_Use.Page_2000.BA_Nr + 3)) ;
    Fip->FDM_Internal_Use.Page_2000.BA_Priority = Priority_level;

   if (Fip->S.Mode == SLOW_FIP_1000 ){
	  Ulong T;
	  /* bidouille corrective de tempo de start BA */
	  T =( Ulong )Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par  ;
	  T = (T * 100)/121;
	  Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par =( Ushort ) T;
   }

   CR = _FIP_SET_BA_PARAMS( Fip );
   CR |= _FIP_WRITE_VAR(Fip,CLE_PRESENCE,&(Fip->FDM_Internal_Use.Presence));
   Fip->FDM_Internal_Use.Ba.Current_priority  = Priority_level ;
   return  ( (Ushort)((CR==0) ? FDM_OK :  FDM_NOK)) ;
}

#if ( FDM_WITH_LEAVE_BA_ON_DEFAULT == YES )

static Ushort
internal_ba_set_priority  ( FDM_REF *Fip  , Uchar Priority_level )
{
    Ushort       CR;

    Fip->FDM_Internal_Use.Presence.Status =
    	       (Uchar)((Fip->FDM_Internal_Use.Presence.Status & 0xf0 ) | Priority_level);
    Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par =
             (Ushort) ( 2 * ( ((Fip->FDM_Internal_Use.Page_2000.BA_Nr_Max+1)*(Priority_level))
                  + Fip->FDM_Internal_Use.Ba.seuil_TSTP  + Fip->FDM_Internal_Use.Page_2000.BA_Nr + 3)) ;
    Fip->FDM_Internal_Use.Page_2000.BA_Priority = Priority_level;

   if (Fip->S.Mode == SLOW_FIP_1000 ){
	  Ulong T;
	  /* bidouille corrective de tempo de start BA */
	  T =( Ulong )Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par  ;
	  T = (T * 100)/121;
	  Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par =( Ushort ) T;
   }

   CR = _FIP_SET_BA_PARAMS( Fip );
   CR |= _FIP_WRITE_VAR(Fip,CLE_PRESENCE,&(Fip->FDM_Internal_Use.Presence));
   return  ( (Ushort)((CR==0) ? FDM_OK :  FDM_NOK)) ;
}

#endif

/*====================================================================*/
/*===================     _fdm_ba_set_parameters    ==================*/
/*
	parametre d entree : MAX_Subscriber n0 max d'abonne a tester par TST_P :
		compris entre  1 et 255
		Si 1 	on a    2 abonnes -> 0 et 1
		Si 255  on a  256 abonnes
FDM_WITH_OPTIMIZED_BA 
	=> cette primivite devient un peut bizarre en R4.9
	MAX_Subscriber uniquement pour Liste Present
*/
Ushort
fdm_ba_set_parameters( FDM_REF *Fip ,
		enum _BA_SET_MODE  standard ,
		Uchar MAX_Subscriber,
		Uchar MAX_Priority )
{
    int prio_actuel;
    int V,R;
    Ushort       CR;

  prio_actuel = Fip->FDM_Internal_Use.Page_2000.BA_Priority;
#if 				( FDM_WITH_OPTIMIZED_BA == YES )
 if (Fip->H.GreatestNumberOfBusArbitrator != 0) {
  	V = ( Fip->FDM_Internal_Use.Page_2000.Timer_Cnt >> 8 ) & 0x3f;
  	switch ( Fip->FDM_Internal_Use.Page_2000.Timer_Cnt & 3) {
  	case 0 : /*2.5Mhz*/
		V *= 4;
        	R =   TEST_P_DURATION/V;
		break;
  	case 1 : /*1Mhz*/
		V *= 5;
        	R =   TEST_P_DURATION/V;
		break;
   
  	case 3 : /*31.25MKhz*/
		V *= 1024;
        	R =   TEST_P_DURATION*5/V;
		break;
  	}
        Fip->FDM_Internal_Use.Ba.seuil_TSTP = (Ushort) R;
  } else {
  	Fip->FDM_Internal_Use.Ba.seuil_TSTP = 257 ;
  }

#else
  Fip->FDM_Internal_Use.Ba.seuil_TSTP = 257 ;
#endif

  switch  ( standard ) {
     case STANDARD:
        /* MAX_Priority	MAX_Subscriber n ont pas d'influence */
        Fip->FDM_Internal_Use.Page_2000.BA_Num_Max_Abonne = 255 ;
 	Fip->FDM_Internal_Use.Ba.Max_priority 		  = 15;
     break;
     case OPTIMIZE_1:
        if (MAX_Subscriber ==0 ) {
     	     /* erreur sur le parametre Max_Subscriber =nbre max d'abonnes  */
 		FDM_Signal_Warning ( Fip , _MAX_SUBSCRIBER_BA , 0 ) ;
		return 0xffff;
         }
         Fip->FDM_Internal_Use.Page_2000.BA_Num_Max_Abonne = MAX_Subscriber ;
 	 Fip->FDM_Internal_Use.Ba.Max_priority 		   = 15;
     	 break;
     case OPTIMIZE_2:
        if (MAX_Subscriber ==0 ) {
 		FDM_Signal_Warning ( Fip , _MAX_SUBSCRIBER_BA , 0 ) ;
		return 0xffff;
        }
        if ((MAX_Priority > 15 ) || ( MAX_Priority < prio_actuel)){
 		FDM_Signal_Warning ( Fip , _LEVEL_PRIORITY_BA  , 0 ) ;
		return 0xffff;
        }
        Fip->FDM_Internal_Use.Ba.Max_priority             = MAX_Priority;
        Fip->FDM_Internal_Use.Page_2000.BA_Num_Max_Abonne = MAX_Subscriber ;
        break;
     case OPTIMIZE_3:
         if (MAX_Subscriber ==0 ) {
 		FDM_Signal_Warning ( Fip , _MAX_SUBSCRIBER_BA , 0 ) ;
		return 0xffff;
         }
         if ((MAX_Priority > 15 ) || ( MAX_Priority < prio_actuel)){
 		FDM_Signal_Warning ( Fip , _LEVEL_PRIORITY_BA  , 0 ) ;
		return 0xffff;
         }
         Fip->FDM_Internal_Use.Ba.Max_priority      	   = MAX_Priority ;
         Fip->FDM_Internal_Use.Page_2000.BA_Num_Max_Abonne = MAX_Subscriber ;
	 Fip->FDM_Internal_Use.Ba.seuil_TSTP 		   = 0 ;

         break;

  }
#if 				( FDM_WITH_OPTIMIZED_BA == NO )
         Fip->FDM_Internal_Use.Page_2000.BA_Nr_Max = Fip->FDM_Internal_Use.Page_2000.BA_Num_Max_Abonne  ;
#endif
  
  Fip->FDM_Internal_Use.Page_2000.BA_End_Macro_Time_Value = Calcul_tempo_31_25 ( Fip ) ;
  
  Fip->FDM_Internal_Use.Page_2000.BA_Start_Time_Par =
	     (Ushort)(2 *
		(((Fip->FDM_Internal_Use.Page_2000.BA_Nr_Max+1)*(Fip->FDM_Internal_Use.Ba.Max_priority+1) ) 
			+ (Fip->FDM_Internal_Use.Ba.seuil_TSTP) + 3 ));


  Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par =
             (Ushort) ( 2 * ( ((Fip->FDM_Internal_Use.Page_2000.BA_Nr_Max+1)*(prio_actuel))
                      + Fip->FDM_Internal_Use.Ba.seuil_TSTP + Fip->FDM_Internal_Use.Page_2000.BA_Nr + 3)) ;

  if (Fip->S.Mode ==SLOW_FIP_1000 ){
	  Ulong T;
	  /* bidouille corrective de tempo de start BA */
	  T =( Ulong ) Fip->FDM_Internal_Use.Page_2000.BA_Start_Time_Par ;
	  T = (T * 100)/121;
	  Fip->FDM_Internal_Use.Page_2000.BA_Start_Time_Par = ( Ushort ) T;
	  T =( Ulong ) Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par ;
	  T = (T * 107)/100;
	  Fip->FDM_Internal_Use.Page_2000.BA_Election_Time_Par =( Ushort ) T;
    }
   CR = _FIP_SET_BA_PARAMS( Fip );
   return  ( (Ushort)((CR==0) ? FDM_OK :  FDM_NOK)) ;
}

/***************************************/
/* BA TST_P Realise par le processeur */
/**************************************/
#if 	( FDM_WITH_REDONDANCY_MGNT ==YES )
static Ushort _FIP_CONTINUE_BA ( FDM_REF *Ref,Ushort Adr_Continue  )
{
#if		( FDM_WITH_FREE_ACCESS == NO )
	int tempo ;
	FDM_ERROR_CODE CR ;
	Debut_procedure ;
	WAIT_Busy;
	ECRIRE_Key(Adr_Continue);
	/************ ecriture de la commande */
	ECRITE(UCom , CMDE_CONTINUE_BA );
	/************ attente bit SV */
	WAIT_SV;
	/************ compte rendu */
	CR_UState;
	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;
	Fin_procedure;
#else
	volatile Ushort   *FifoUser, *FifoFip ;
	int  tempo;
	Ushort Tmp ,CR;
	Dial_AE *PDial_AE;
	CR =0;
	MAC__INIT_REG;
	MAC__WAIT_PLACE;
	MAC__ECRIRE_DEMANDE( CONTINUE_USER ,  Adr_Continue );
abort:
	OS_Leave_Region();
abortd:
	return CR;
#endif
}

/*---------------------------------------------------------------------*/
/*=----------	_FIP_SMAP_PRESENCE_MOI: test id presence    -----------*/
/*---------------------------------------------------------------------*/
void
_FIP_SMAP_PRESENCE_MOI ( struct _FDM_MPS_VAR_REF *Var )
{
/*
fonction:
	cette procedure realise le traitement de l'evenement
	reception de l'id de presence moi
		_ note la presence de l'abonne
*/
	FDM_REF *Ref;
	Ref = Var->Ref->Fip;
       if ( Ref->FDM_Internal_Use.Ba.BA_actif == 0 ) return;
       Ref->FDM_Internal_Use.Ba.id_presence_recu = 1;
}
/*---------------------------------------------------------------------*/
/*=-----     _FIP_SMAP_PRESENCE_AUTRES: test id presence    -----------*/
/*---------------------------------------------------------------------*/
void
_FIP_SMAP_PRESENCE_AUTRES ( struct _FDM_MPS_VAR_REF *Var  )
{
/*
fonction:
	cette procedure realise le traitement de l'evenement
	reception de l'id de presence moi
		_ note la presence de l'abonne
*/
    Uchar B[7] ;
    Ushort CR;
    FDM_REF *Ref;
    struct MediumDef *MEDIUM;
	Ref = Var->Ref->Fip;
   	MEDIUM = &Ref->FDM_Internal_Use.Medium;
       if ( Ref->FDM_Internal_Use.Ba.BA_actif == 0 ) return;
       Ref->FDM_Internal_Use.Ba.id_presence_recu = 1;
       if ((Ref->FDM_Internal_Use.Presence.Status & 0xf) == 0 ) return;
       CR =_FIP_READ_VAR(Ref, CLE_PRESENCE_C ,(void *) B);
       if ( (CR & 0x8000) != 0 ) return;
       if ((B[6] & 0xf0) == _BA_VEILLE )
	{
	 	if ((Uchar)(B[6] & 0xf) < (Uchar)(Ref->FDM_Internal_Use.Presence.Status & 0xf))
		{
/*888888		   _FIP_STOP_BA(Ref);
	           _FIP_WRITE_PHY(Ref, FIP_CONF_RESEAU  ,
			&Ref->FDM_Internal_Use.Page_2000.Base_0 ,2);
		   Ref->FDM_Internal_Use.Ba.tst_p_en_cours     = 0;
	       	   Ref->FDM_Internal_Use.Ba.cpt_nbre_tst_p     = 0;
		   Ref->FDM_Internal_Use.Ba.B0_tst_p           = ( FDM_ADR_USER>> 10 );
	           MEDIUM->Voie_en_cours  = 3 ;
8888*/
			/* set flag pour relancer BA qd EVT EVT_BA_STOP3 */
		   Ref->FDM_Internal_Use.Ba.Prio_Stop = 0x5a5a;
		 }
	}
}
/*---------------------------------------------------------------------*/
/*=-----------    _FIP_SMAP_TEST_P_DEBUT :debut test_p      -----------*/
/*---------------------------------------------------------------------*/
/*
Rappel :
Prog_ba_test_p[ 0 a 0x0f]    = instructions programme BA
Prog_ba_test_p[ 0x10 a 0x1f] = table de recherche dicho a 1 variable ID 14xx
si on est en acces classique le pointeur , pointe sur une table intermediaire ,
	il faut donc faire un FIP_WRITE_PHY pour transfert dans FULLFIP
Si acces libre , la table pointe diretement sur table prog_ba_test

traitement de l'evenement SUSPEND associe a evt debut test_p
	sauvegarde adresse en cours
	modification du programme Prog_ba_test_p :
	- modif
		Prog_ba_test_p[ 4] = id suivant =14xx
		Prog_ba_test_p[ 5] =	adresse de l'id dans table antidup testP
		si on est pas id abonne alors
			Prog_ba_test_p[ 0x10 ]= 14xx
	 commute sur base tst_p ->lancement du macro cycle
*/
void  _FIP_SMAP_TEST_P_DEBUT ( FDM_REF *Ref, Ushort Ba_adr_prog)
{
   int no_abonne_teste;
   struct MediumDef *MEDIUM;
   MEDIUM = &Ref->FDM_Internal_Use.Medium;
  Ref->FDM_Internal_Use.Ba.Last_MC_Adr =Ba_adr_prog ;
  Ref->FDM_Internal_Use.Ba.id_presence_recu = 0;
  MEDIUM->nb_tst_p++;
  if (MEDIUM->double_medium == 1) {
	/* set voie pour le test */
       MEDIUM->Voie_en_cours=
					Ref->FDM_Internal_Use.Ba.Flag_voie_test;
#if (  FDM_WITH_BI_MEDIUM == YES )
	       FIPDRIVE(Ref);
#endif
   }
   if ( Ref->FDM_Internal_Use.Ba.Flag_traitement == 1 ) {
	   Ref->FDM_Internal_Use.Ba.no_abonne_teste++;
	   Ref->FDM_Internal_Use.Ba.no_abonne_teste %= Ref->FDM_Internal_Use.Page_2000.BA_Num_Max_Abonne+1;
   }
   no_abonne_teste =  Ref->FDM_Internal_Use.Ba.no_abonne_teste;
   Ref->FDM_Internal_Use.Ba.Prog_Ba_TestP->X[4]=(short)(0x1400 | no_abonne_teste);
	/* adresse id en table ba_testp_antidup*/
   Ref->FDM_Internal_Use.Ba.Prog_Ba_TestP->X[5]= (short)(0x80 + (no_abonne_teste*2));

   Ref->FDM_Internal_Use.Ba.Prog_Ba_TestP->X[0x12]=
	   (Ref->H.K_PHYADR == no_abonne_teste) ? 0x40 : 0x30 ;
   Ref->FDM_Internal_Use.Ba.Prog_Ba_TestP->X[0x10]=
					 (short)(0x1400 | no_abonne_teste);
#if  (  FDM_WITH_FREE_ACCESS == NO )
	_FIP_WRITE_PHY( Ref, FDM_ADR_USER,
			Ref->FDM_Internal_Use.Ba.Prog_Ba_TestP->X ,40);
	/* commute sur base tst_p */
	Ref->FDM_Internal_Use.Ba.tst_p_en_cours = 1;
	_FIP_WRITE_PHY(Ref,FIP_CONF_RESEAU ,
			&Ref->FDM_Internal_Use.Ba.B0_tst_p,2);
#else
	{
	Ushort  *buffer;
	if (Ref->H.K_PHYADR !=(Uchar)no_abonne_teste) {
		/* commute sur base tst_p */
		Ref->FDM_Internal_Use.Ba.tst_p_en_cours = 1;
		buffer  = Ref->H.FREE_ACCES_ADDRESS;
		buffer += 0x2000;
		*buffer = Ref->FDM_Internal_Use.Ba.B0_tst_p;
	}
	}
#endif
   /* continue sur programme ba */
   _FIP_CONTINUE_BA(Ref, 0  );
}
/*---------------------------------------------------------------------*/
/*=-----------    _FIP_SMAP_TEST_P_FIN :fin test_p          -----------*/
/*---------------------------------------------------------------------*/
/*
fonction:
	cette procedure realise le traitement de l'evenement
	fin test
	_ restitue BASE_0
	_ met a jour la table des presents
	_ reprend le cycle du BA
*/
void
_FIP_SMAP_TEST_P_FIN ( FDM_REF *Ref )
{
    int k;
    int abonne_teste;
    struct MediumDef *MEDIUM;

	MEDIUM = &Ref->FDM_Internal_Use.Medium;
    k = Ref->FDM_Internal_Use.Ba.Flag_voie_test - 1;
    if (MEDIUM->double_medium == 1) {
        Ref->FDM_Internal_Use.Ba.Flag_voie_test ^= 3;
        if (( Ref->FDM_Internal_Use.Ba.Flag_voie_test &
              MEDIUM->Voie_Valide ) == 0 ) {
       		Ref->FDM_Internal_Use.Ba.Flag_voie_test ^= 3;
       		Ref->FDM_Internal_Use.Ba.Flag_traitement = 1;
        }
        else   Ref->FDM_Internal_Use.Ba.Flag_traitement ^=1;
        MEDIUM->Voie_en_cours  = 3 ;
#if		(  FDM_WITH_BI_MEDIUM == YES )
        FIPDRIVE(Ref);
#endif
    }
    abonne_teste =Ref->FDM_Internal_Use.Ba.no_abonne_teste;
    if (Ref->FDM_Internal_Use.Ba.id_presence_recu == 1) {
	    /* abonne present */
	 if (k == 0)
	    Ref->FDM_Internal_Use.Liste_present.V0[ abonne_teste>>3] =
	            (Uchar)( Ref->FDM_Internal_Use.Liste_present.V0[abonne_teste >>3 ] |
	                   (1 << (abonne_teste  & 0x7)) );
	 else
	    Ref->FDM_Internal_Use.Liste_present.V1[ abonne_teste>>3] =
	           (Uchar)( Ref->FDM_Internal_Use.Liste_present.V1[ abonne_teste >>3 ] |
	                  (1 << (abonne_teste  & 0x7)) );
    }else
    {
            /* abonne absent */
      if ( MEDIUM->Voie_Valide == 3 ) {
        if (k == 0)
	      Ref->FDM_Internal_Use.Liste_present.V0[ abonne_teste >>3] =
	             (Uchar)( Ref->FDM_Internal_Use.Liste_present.V0[abonne_teste>>3 ] &
	                     ~(1 << (abonne_teste  & 0x7)) );
        else
	      Ref->FDM_Internal_Use.Liste_present.V1[abonne_teste >>3] =
	             (Uchar)( Ref->FDM_Internal_Use.Liste_present.V1[abonne_teste >>3 ] &
	                     ~(1 << (abonne_teste  & 0x7)) );
      } else {
    	  Ref->FDM_Internal_Use.Liste_present.V0[abonne_teste >>3] =
	             (Uchar)( Ref->FDM_Internal_Use.Liste_present.V0[abonne_teste >>3 ] &
	                     ~(1 << (abonne_teste  & 0x7)) );
	      Ref->FDM_Internal_Use.Liste_present.V1[abonne_teste >>3] =
	             (Uchar)( Ref->FDM_Internal_Use.Liste_present.V1[abonne_teste >>3 ] &
	                     ~(1 << (abonne_teste  & 0x7)) );
      }
    }
    Ref->FDM_Internal_Use.Ba.cpt_nbre_tst_p++ ;
    /* MAJ variable liste present */
    if ((Ref->FDM_Internal_Use.Ba.cpt_nbre_tst_p %
        ( (Ref->FDM_Internal_Use.Page_2000.BA_Num_Max_Abonne+1) << MEDIUM->double_medium))== 0){
	  Ref->FDM_Internal_Use.Ba.B0_tst_p |= 0x1000;
	  _FIP_WRITE_VAR(Ref, CLE_LIST_PRE_P , (void *)&Ref->FDM_Internal_Use.Liste_present) ;
    }
	/* on remet la base USER */
	 _FIP_WRITE_PHY(Ref, FIP_CONF_RESEAU  ,
			&Ref->FDM_Internal_Use.Page_2000.Base_0 ,2);
	 Ref->FDM_Internal_Use.Ba.tst_p_en_cours = 0;
    /* on reprend le programme USER */
    if ( Ref->FDM_Internal_Use.Ba.Prio_Stop == 0x5a5a ){
	Ref->FDM_Internal_Use.Ba.Prio_Stop = 0x55aa;
 	_FIP_STOP_BA(Ref);
    } 
    else {
    	_FIP_CONTINUE_BA(Ref,Ref->FDM_Internal_Use.Ba.Last_MC_Adr);
    }

}
#endif

#endif

