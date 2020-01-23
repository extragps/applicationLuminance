/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
 
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmtime.h
  Description     : multi-producteur de l'heure -
		    Interface  utilisateur ( procedures et Classes )


Version:4.3.1
	a/- compilation sur 16 bits 1000"L"

***********************************************************/
/*****************************************************/
/******* Gestion de la variable MPS d'heuree *********/
/*****************************************************/

#ifndef __fdm_time_h
#define __fdm_time_h


#ifdef __cplusplus
class GenericTime  ;
#endif

typedef enum FDM_BOOLEAN  { fdm_false=0 , fdm_true=1  } FDM_BOOLEAN ; 

typedef enum SubscriberStates
{
	unavailable	 = 0x0 ,
	not_eligible 	 = 0x1 ,
	active		 = 0x2 ,
	standby		 = 0x4
}  SubscriberStates;

#include "fdm.h"

typedef struct _EtatAbonnes {
	unsigned short         	abonne  : 8 ;
	unsigned short			State   : 4 ;
	unsigned short		   Prio 	: 4 ;
	 } EtatAbonnes ;

typedef struct _GT_PRIORITY { unsigned Val : 4; } GT_PRIORITY ;

typedef struct _FDM_GENERIC_TIME_VALUE{
	Ulong	 		Signifiance ; 		/* 0 global ; 2 local */
	Ulong			Number_of_second;
	Ulong			Number_of_nanoseconde;
	 } FDM_GENERIC_TIME_VALUE;

typedef struct _FDM_GENERIC_TIME_DEFINITION {
	FDM_BOOLEAN	 	With_Choice_Producer;
	FDM_BOOLEAN	 	With_MPS_Var_Produced;
	FDM_BOOLEAN	 	With_MPS_Var_Consumed;
	_FDM_MSG_IMAGE 	Image	;

	Ulong 			Refreshment;			/*if Prod        */
	Ulong 			Promptness;			/*if Cons        */
	const Ulong		*Delta_Time_Location  ;		/*if Prod        */
	void			(*User_Signal_Mode) ( const int sens );	/*if Choice Prod */
	Ushort 			Ticks_Election ;		/*if Choice Prod */
	Ushort	 		Channel_Nr;			/*if Choice Prod */

}  FDM_GENERIC_TIME_DEFINITION;

typedef struct _FDM_GENERIC_TIME_REFERENCE {
	FDM_REF				*Fip;

	FDM_GENERIC_TIME_DEFINITION 	user_def;

	FDM_AE_LE_REF 			* Generic_AELE;
	FDM_MPS_VAR_REF 		* Generic_var ;

	TEMPO_DEF			Ticks_Election;
	struct _FDM_GENERIC_TIME_REFERENCE *P1;
	TEMPO_DEF			ticks_etat ;
	struct _FDM_GENERIC_TIME_REFERENCE *P2;
 	T_File				* MemPool;
	FDM_MESSAGING_REF 		*EmettreOrdre;
	FDM_MESSAGING_REF 		*RecevoirOrdre;

	FDM_MESSAGING_REF		*EmettreEtat;
	FDM_MESSAGING_REF		*RecevoirEtat;

	Ushort 				Status;
	int 				StationActive;

	int				Priorite;
        int etat_emission_etat;
	/*Eligible:
	=================
	true => le mecanisme est autorise par l'utilisateur
	*/

	FDM_BOOLEAN	 	Eligible ;

	FDM_BOOLEAN	 	Stopped ;

	/*Election_State:
	=================
	true => le mecanisme fonctionne
	false =>le mecanisme d'election est HS ( TO sur envoi message ...  )
	*/
	FDM_BOOLEAN	 	Election_State ;


	EtatAbonnes			Old[256];
	EtatAbonnes			Actual[256];

} FDM_GENERIC_TIME_REFERENCE;




/*********************   P U B L I C   F U N C T I O N S *********************/
#ifdef __cplusplus
extern "C" {
#endif

FDM_GENERIC_TIME_REFERENCE *
fdm_generic_time_initialize (
	FDM_REF			 *Fip,
	const FDM_GENERIC_TIME_DEFINITION  *User_param
);


Ushort    fdm_generic_time_delete ( FDM_GENERIC_TIME_REFERENCE * );

#if	 ( FDM_WITH_GT == YES )
Ushort    fdm_generic_time_set_priority
		( FDM_GENERIC_TIME_REFERENCE * , GT_PRIORITY  prio );
Ushort    fdm_generic_time_set_candidate_for_election
		( FDM_GENERIC_TIME_REFERENCE * , FDM_BOOLEAN  );
#endif

#if (  FDM_WITH_GT_ONLY_PRODUCED == YES ) || ( FDM_WITH_GT == YES )

extern FDM_GENERIC_TIME_VALUE fdm_generic_time_get_value ( void ) ;


Ushort
fdm_generic_time_write_loc  ( const FDM_GENERIC_TIME_REFERENCE *  );
#endif

#if	( FDM_WITH_GT_ONLY_CONSUMED == YES ) || ( FDM_WITH_GT == YES )

extern void fdm_generic_time_give_value ( FDM_GENERIC_TIME_VALUE *) ;

FDM_MPS_READ_STATUS
fdm_generic_time_read_loc  (
	const FDM_GENERIC_TIME_REFERENCE *
	);

#endif

#ifdef __cplusplus
}
#endif
/*****************************************************************************/



#ifdef __cplusplus


class GenericTime  {
  private  :
	 FDM_GENERIC_TIME_REFERENCE   * Ptr_TIME ;

  public :
  // constructeur

  GenericTime 	(  Fip_Device_Manager &Fip ,FDM_GENERIC_TIME_DEFINITION &Def ) :
  	Ptr_TIME (  fdm_generic_time_initialize ( Fip.Fip , &Def  ) ) {}

  GenericTime 	( Fip_Device_Manager  *Fip ,FDM_GENERIC_TIME_DEFINITION &Def ) :
  	Ptr_TIME (  fdm_generic_time_initialize ( Fip->Fip , &Def  ) ) {}

  ~GenericTime(){fdm_generic_time_delete (Ptr_TIME);}

  inline Ushort IS_Created() { return (Ushort)( (  Ptr_TIME == NULL) ? 0 : 1 ); }

#if (  FDM_WITH_GT_ONLY_PRODUCED == YES ) || ( FDM_WITH_GT == YES )

  inline Ushort Local_Write (  )
	{ return  fdm_generic_time_write_loc  ( Ptr_TIME  ); }

#endif

#if	( FDM_WITH_GT_ONLY_CONSUMED == YES ) || ( FDM_WITH_GT == YES )

  inline FDM_MPS_READ_STATUS Local_Read ( )
	{ return  fdm_generic_time_read_loc  ( Ptr_TIME  ); }
#endif

#if	 ( FDM_WITH_GT == YES )

  inline Ushort Set_Priority ( GT_PRIORITY priority )
	{ return  fdm_generic_time_set_priority ( Ptr_TIME  ,priority   ); }

  inline Ushort Set_Candidate_For_Election ( FDM_BOOLEAN S )
	{ return  fdm_generic_time_set_candidate_for_election  (Ptr_TIME  , S  ); }

#endif

};


#endif

#endif

