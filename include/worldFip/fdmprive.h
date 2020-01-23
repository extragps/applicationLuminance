/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD

      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmprive.h
  Description     : Define et structures PRIVEES generales



  Date de creation: Fri Mar 15 15:11:44 MET 1996
  Historique des evolutions/modifications
  	Modification : 96.07.26
  	Reference Ra : RA xx
  	But de la modification :
  		modif Buffer--> FBuffer
  		(nom Buffer utilise dans <stdio.h> compilateur METAWARE 16bits)

***********************************************************/

#ifndef __fdmprive
#define __fdmprive

#include "fdm_os.h"

#if ((FIP_ADR_USER & 0x3ff ) == 0 )
#define FDM_ADR_USER FIP_ADR_USER
#else
#define FDM_ADR_USER ((FIP_ADR_USER>>10)+1)<<10
#endif
/************** FIPHANDLER DEFINITIONS *********************************/
#define ID_BOURRAGE				0x9080
/****
les cles ci-dessous sont les cle des VAR SM-MPS  par l'abonne
0 = EOC
1=Msg Prod
2=Msg Cons
*/

enum SM_MPS_Var_Key {
	CLE_PRESENCE_C		= 3	,
	CLE_PRESENCE	  	,
	CLE_LIST_PRE_C		,
	CLE_LIST_PRE_P		,
	CLE_REPORT		,
	CLE_IDENT		,
	CLE_SYNCHRO_BA		,
	CLE_SYNCHRO_BA_PROD	,
	CLE_SEGPAR_CONS		,
	CLE_SEGPAR_PROD
}  ;

typedef  unsigned long	Absolute_ADR ;
typedef  unsigned short Acces_Key ;

/******************* FIPHANDLER function prototypes ********************/
#define Add_Repport_MSG	 2
#define Add_Repport_Diag 3


/* Initialisations */
extern void  _FIP__petite_tempo   ( struct _FDM_REF * );

extern Ushort _FIP_RAZ_RAM   ( struct _FDM_REF * );

extern void _FIP_SMAP_PRESENCE_MOI (struct _FDM_MPS_VAR_REF *) ;

extern Ushort _FIP_START_BA  ( struct _FDM_REF *Ref , Ushort  Adr_next_macrocycle ) ;

extern void _FIP_SMAP_PRESENCE_AUTRES (struct _FDM_MPS_VAR_REF *) ;

extern Ushort _FIP_DOWNLOAD_PRG   ( struct _FDM_REF * );

extern Ushort _FIP_TEST_PRG   ( struct _FDM_REF * );

extern Ushort _FIP_DOWNLOAD_FILE  ( struct _FDM_REF * );

extern Ushort _FIP_TEST_FILE  ( struct _FDM_REF * );

extern Ushort _FIP_START_CIRCUIT  ( struct _FDM_REF * );

extern void _FIP__tres_petite_tempo  ( struct _FDM_REF * );

extern Ushort _FIP_END_DIAG       ( struct _FDM_REF * );
#if		( FDM_WITH_FREE_ACCESS == YES )
extern void  _FIP__petite_tempo   ( struct _FDM_REF *  );
#endif

extern Ushort  Calcul_tempo_31_25 ( struct _FDM_REF * ) ;

extern int FDM_place_dans_base_x ( struct _FDM_REF *  , Ushort V );

/*   utilisables en  Acces libre ou nornal */

extern Ushort _FIP_WRITE_PHY_BYTE  ( struct _FDM_REF * ,Absolute_ADR, const Uchar  *, int Nb_byte  );

extern Ushort _FIP_WRITE_PHY       ( struct _FDM_REF * ,Absolute_ADR, const Ushort *, int Nb_byte );

extern Ushort _FIP_READ_PHY_BYTE   ( struct _FDM_REF * ,Absolute_ADR, Uchar  *, int Nb_byte );

extern Ushort Internal_fdm_ae_le_start  ( struct _FDM_AE_LE_REF *AE ,Ushort PDU ) ;

#ifdef __cplusplus
extern "C" {
#endif
 	Ushort _FIP_READ_PHY        ( struct _FDM_REF * ,Absolute_ADR, Ushort *, int Nb_byte );

 	void * FDM_Malloc (  struct _FDM_REF *Ref , Ulong nombre_d_octet );

	 void FDM_Free( struct _FDM_REF *Ref,void * segment_adr);

	Ulong SWAB_ONE_LONG ( Ulong mot_long );
 	void FDM_Signal_Warning ( struct _FDM_REF *Ref ,
		 enum CODE_ERROR  , Ulong Default_Type );

	void FDM_Signal_Error ( struct _FDM_REF *Ref ,
		 enum CODE_ERROR , Ulong Default_Type ) ;

	Ushort _FIP_READ_WORD        ( struct _FDM_REF * ,Absolute_ADR, Ushort *);


#ifdef __cplusplus
}
#endif

extern int  FDM_ressources_create( struct _FDM_REF * ) ;

extern void  FDM_ressources_delete( struct _FDM_REF *);

extern Ushort _FIP_WRITE_DESCRIPTEUR ( struct _FDM_REF * ,Absolute_ADR, const Ushort * );

extern Ushort _FIP_WRITE_WORD      ( struct _FDM_REF * ,Absolute_ADR, Ushort Val );

/*  Variables */

extern Ushort _FIP_WRITE_VAR       ( struct _FDM_REF * ,Acces_Key ,  const void * );

extern Ushort _FIP_WRITE_VAR_TIME  ( struct _FDM_REF * ,Acces_Key ,
	Uchar* (*User_Get_Value) ( void  ) , const Ulong * );

extern Ushort _FIP_READ_VAR        ( struct _FDM_REF * ,Acces_Key , void * );

extern FDM_MPS_READ_STATUS _FIP_READ_VAR_TIME        ( struct _FDM_REF * ,
			Acces_Key ,
			void (*User_Store_Value) ( FDM_MPS_VAR_TIME_DATA *),
			FDM_MPS_VAR_TIME_DATA * );

extern Ushort _FIP_UPDATE          ( struct _FDM_REF * , Acces_Key  , short Prio );

/* structure appelee par GDT sur time out */
typedef struct __FDM_XAE_Tempo_Ref {
	TEMPO_DEF			Timer;
	int 				Compte_rendu;
	void /* struct __FDM_XAE_Ref *	*/ * Ref_XAE;
   	struct _FDM_REF 		* Fip; /*pointe sur FIP */
} _FDM_XAE_Tempo_Ref ;
extern Ushort _FIP_APER_FLUSHOUT_N   ( struct __FDM_XAE_Tempo_Ref * ) ;
extern Ushort _FIP_APER_FLUSHOUT_U   ( struct __FDM_XAE_Tempo_Ref * ) ;


/* BA */
typedef struct _FDM_BA_REF {
    struct _FDM_REF	*Fip;
    Ushort    		Macro_Cycle_Adr;
    Ushort    		Label;
    int    		Nbre_page;
} FDM_BA_REF ;



/* Divers */

extern Ushort _FIP_ACK_EOC       ( struct _FDM_REF *  );


extern Ushort _FIP_DEF_EVT       ( struct _FDM_REF * ,Acces_Key , Ushort  Event_Definition );


/************* Its Ba ******************************/
extern void  _FIP_SYNCH_WAIT( struct _FDM_REF*, FIP_EVT_Type * );

/************* Its messagerie reception******************************/

extern void _FIP_SMAP_ACQ_EMIS (struct _FDM_REF *, Ushort);


/* taille d'un buffer de message en mots : en emission ou en reception */
#define SIZE_1_MSG 	     133

#define MAX_DATA_MSG         256  /* taille max donnees utiles du message */


/*Messagerie Emission */
/**********************/

/* on limite  32 buffers emission message par canal */
#define MAX_TX_BUFFER_EMI    32

typedef struct {
	Ushort Indic;
	Ushort Adr_data_pforts;
	Ushort Adr_data_pfaibles;
	Ushort Desc_fifo;
	Ushort Adr_suivant;
	Ushort non_u[11];
} DESC_FIFO_MSG_EMI;


typedef struct {
	Uchar   Mode[2] ;          /* 0 pF 1 pf MSG_SDN  ou MSG_SDA */
	Uchar   Length[2] ; 	   /* lg totale , entete + donnees*/
	Uchar   Destination[3];
	Uchar   Source[3];
	} MSG_HEADER_Send_Type ;



/* Reception */
/*************/


/* on definit 64 buffers de reception message */
#define MAX_MSG_REC          64


typedef struct {
	Ushort Indic;
	Ushort Adr_data_pforts;
	Ushort Adr_data_pfaibles;
	Ushort Adr_suivant;
	Ushort non_u[12];
} DESC_FIFO_MSG_REC;



typedef struct {
	Uchar  Recept_Time[2];
	Uchar  Length[2] ; 	   /* lg totale , entete + donnees*/
	Uchar  dest[3];
	Uchar  sour[3];
} MSG_HEADER_Receive_Type;

typedef struct {
     Ushort nr_of_elem;     	/* nombre d'elements a purger */
     struct{
      Uchar Num_fifo;
      Uchar Num_bloc;
     }Elem[128];
} PURGE_type ;



/******************* FIPDIAG function prototypes ********************/

extern Ushort _FIP_RAM_TEST ( struct _FDM_REF *);

extern Ushort _FIP_DIAG_OFF_LINE     (struct _FDM_REF *);

extern void _FIP_INIT_DIAG_ON_LINE (struct _FDM_REF *);

extern Ushort _FIP_DIAG_ON_LINE_TICKS(_FDM_XAE_Tempo_Ref *);

extern Ushort dem_mem_MC( struct _FDM_REF * ,int Nbre_page );

extern void _FIP_SMAP_TEST_P_DEBUT ( struct _FDM_REF * , Ushort );

extern void  _FIP_SMAP_TEST_P_FIN ( struct _FDM_REF *  );


/***********************************************************/
/********************* FIPCODE 6 USER_PARAMETRAGE **********/
/***********************************************************/
#if ( FDM_WITH_APER == NO )
#if ( FDM_WITH_BA == YES )
#error Illegal Compilation Options !!!
#endif
#endif

#if ( FDM_WITH_APER == NO )
#if ( FDM_WITH_MESSAGING == NO )
#define User_Type_Station  1
#else
#define User_Type_Station  2
#endif
#else
#if ( FDM_WITH_BA == NO )
#define User_Type_Station  3
#else
#define User_Type_Station  4
#endif
#endif


#if ( FDM_WITH_FREE_ACCESS  == NO )
#define User_Mode_Fonct 0
#else
#define User_Mode_Fonct 1
#endif

#if  ( FDM_WITH_REDONDANCY_MGNT	== YES )
#define User_Redondancy	RDD_USER
#else	/* redondance par FIPCODE */
#if ( FDM_WITH_BI_MEDIUM == YES )
#define User_Redondancy	  RDD_AVEC_FIELDUAL  
#else
#define User_Redondancy   RDD_SANS_FIELDUAL  
#endif
#endif


/***********************************************************/
/********************* FIPCODE 6 DEFINITION ****************/
/***********************************************************/

/*aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa Adresse aaaaaaaaaaaaa*/
#define _Ad_FIP_STATUS			0x0089

/*cccccccccccccccccccccccccccccccc constantes cccccccccccc*/

/* position type */
#define _position_I1			0x1	/* var mps sur image 1 */
#define _position_I2			0x2	/* var mps sur image 2*/
#define _position_MSGa			0x4	/* var mps avec MSGa */
#define _position_Cons			0x8	/* var mps consommee */
#define _position_Var			0x10
#define _position_SAP			0x20
#define _position_SAPx			0x40
#define _position_CH_I1			0x0100	/* canal sur image 1 */
#define _position_CH_I2			0x0200	/* canal sur image 2*/
#define _position_MSG_I1		0x0400	/* msg sur image 1 */
#define _position_MSG_I2		0x0800  /* msg sur image 2 */
#define _position_VaD			0x1000  /* var cons-prod */
#define _position_Ip1			0x4000  /*var mps cons_prod image 1 */
#define _position_Ip2			0x8000  /*var mps cons_prod image 2 */



#define Synchro_Id_Desc			0
#define Msg_Id_Desc_Prod		0x10
#define Msg_Id_Desc_Cons		0x20

#define MSGr_autorisee			1
#define Msg_Channel_UNUSED		0x0000
#define Msg_Channel_Aper		0x0000
#define Msg_Channel_i			0x0000


#define	_MSG_IMAGE_1	1
#define	_MSG_IMAGE_2	2
#define	_MSG_IMAGE_1_2	3


/*ttttttttttttttttttttttttttttttttttttt types tttttttttttt*/

typedef struct _DESC_TRAME_TYPE {
	struct _DESC_TRAME_TYPE *Prec;
	struct _DESC_TRAME_TYPE *Next;
	Ushort Var_Desc_Ptr_Produite;
	Ushort Var_Desc_Ptr_Consommee;
	/**/
	 Ushort Identifier;
	 Ushort Type_Descriptor;	/*<= MSGr_autorisee*/
	 Ushort Var_Desc_Ptr;		/*<= [ @DVar | Synchro_Id_Desc | Msg_Id_Desc*/
	 Ushort	rien3;
	 Ushort	rien4;
	 Ushort	reserved_FIPCODE;
	 Ushort	Production_Period;
	 Ushort	rien7;
	 Ushort	Msg_Channel;
	 Ushort	Reserved[6];
	 Ushort	position;		/*<= [_SAP_position]|[_CH_position]|[_Va_position]*/
}  DESC_TRAME_TYPE ;


/*eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee enum eeeeeeeeeeee*/

/* desc_Type_Length */
#define TypeLength_C			0x0200
#define TypeLength_Promptness		0x0800

#define TypeLength_P			0x0100

/* desc_Var_Type */
#define Var_Type_C			0x0000
#define Var_Type_VLG			0x0004

#define Var_Type_P			0x0040
#define Var_Type_MSGa			0x0020
#define Var_Type_RQa			0x0010
#define Var_Type_SR			0x0008
#define Var_Type_SRD			0x0004
#define Var_Type_VLG			0x0004


/* Receive_Evt */
#define Receive_Evt			0x0200
#define Receive_Evt_permanent		0x8200

#define Send_Evt			0x0100
#define Send_Evt_permanent		0x8100

typedef struct  {
	 Ushort Type_Length;	/*<= [[|]desc_Type_Length] | Data_length */
	 Ushort	Access_Key;
	 Ushort	C_Value_Time;
	 Ushort C_Value_Ptr;
	 Ushort C_Value_User;
	 Ushort Var_Type;	/*<= [[|]desc_Var_Type] */
	 Ushort	C_Value_LstPresent;
	 Ushort	rien7;
	 Ushort	Evenement;	/*<= [Receive_Evt]|[Send_Evt]   */
	 Ushort PDU_Type_Length;
	 Ushort Consumption_Period;
	 Ushort	rienB;
	 Ushort rienC;
	 Ushort	rienD;
	 Ushort Trans_Delai ;
	 Ushort ID;
} DESC_VAR_TYPE ;

typedef struct  {
		DESC_VAR_TYPE Desc[1] ;
} T_Desc_Ptr ;

typedef struct  {
		 Ushort P_Type_Descripteur ;	/*<= [[|]desc_Type_Length] | Data_length */
		 Ushort Access_Key;
		 Ushort Res2 		   ;
	 volatile Ushort P_Value_Ptr	   ;
	 volatile Ushort P_Value_User	   ;
	 volatile Ushort P_Var_Type 	   ;	/*<= [[|]desc_Var_Type] */
	 volatile Ushort Cpt_Var_User      ;
	 volatile Ushort	Var_Status ;
		 Ushort	Config_Evt 	   ;	/*<= [Receive_Evt]|[Send_Evt]   */
		 Ushort PDU_Type_Length    ;
		 Ushort rienA		   ;
		 Ushort	reinB		   ;
		 Ushort ResC 		   ;
		 Ushort	ResD		   ;
		 Ushort rienE		   ;
		 Ushort ID 		   ;
} AE_P_DESC_VAR_TYPE ;

typedef struct  {
	 	Ushort C_Type_Descripteur  ;	/*<= [[|]desc_Type_Length] | Data_length */
		 Ushort Access_Key;
	 	Ushort   Res2 		   ;
	 volatile Ushort C_Value_Ptr	   ;
	 volatile Ushort C_Value_User	   ;
	 	Ushort C_Var_Type 	   ;	/*<= [[|]desc_Var_Type] */
	 volatile Ushort Cpt_Var_User      ;
	 volatile Ushort Var_Status 	   ;
	 	Ushort	 Config_Evt 	   ;	/*<= [Receive_Evt]|[Send_Evt]   */
	 	Ushort  PDU_Type_Length    ;
	 	Ushort  Consumption_Period ;
	volatile Ushort Reception_Delay	   ;
		Ushort  ResC 		   ;
	 	Ushort	ResD		   ;
	 	Ushort	Trans_Delai 	   ;/* ou divers autres valeurs */
	 	Ushort  ID 		   ;
} AE_C_DESC_VAR_TYPE ;

/********************** Fonctions Privees de FDM *********************/
extern int FDM_STOP_DB_MOD ( struct _FDM_REF * Ref ) ;

extern Ushort CRE_DTRAME( struct _FDM_REF * Ref ,
	 FDM_XAE * Var , Ushort PDU , const Uchar * FBuffer) ;

extern Ushort CRE_DVAR( struct _FDM_REF * Ref , const FDM_XAE * Var , Ushort PDU , const Uchar * FBuffer) ;

extern void SUP_DVAR ( struct _FDM_REF *Ref , DESC_TRAME_TYPE *DT );

extern void SUP_DTRAME ( struct _FDM_REF *Ref , Ushort  Identifieur );

extern void DELETE_DTRAME ( struct _FDM_REF *Ref   ) ;

extern int _FIP_Create_DB_BA ( struct _FDM_REF *Ref   ) ;

extern Ushort Convert_User_time ( struct _FDM_REF *Ref, Ulong Tempo );

extern void FDM_Change_Base_0 ( struct _FDM_REF *Ref);

extern Absolute_ADR _SCAN_DTRAME_IN_FIP ( struct _FDM_REF * , int image , Ushort ID );

/***BA definition******/


/*etat du Ba dans [7..4] de status dans variable presence*/
#define _BA_NON		(Uchar ) 0x00
#define _BA_STOP	(Uchar ) 0x10
#define _BA_VEILLE	(Uchar ) 0x20
#define _BA_RUN  	(Uchar ) 0x30



typedef struct _FIP_MSG_EVT {
	struct 		_FIP_MSG_EVT *Next;
	struct 		_FIP_MSG_EVT *Prev;

	Ushort		Key;
	struct _FDM_REF		*Fip;

} FIP_MSG_EVT ;


typedef struct _FDM_AE_LE_REF {
	struct _FDM_REF *Fip;
	int dim ;
	enum  _FDM_AE_LE_STATE Etat ;
	enum  _FDM_CHANGE_ALLOWED  Flg;	/* change allowed */
	FDM_MPS_VAR_REF Var[1];
} FDM_AE_LE_REF ;




/***etat reseau definition******/

typedef  struct {
      Ushort Nb_Of_Transaction_Ok_1; /* Nbre transactions sans err sur voie 1 */
      Ushort Nb_Of_Transaction_Ok_2; /* Nbre transactions sans err sur voie 2 */
      Ushort Nb_Of_Frames_Nok_1;     /* Nbre trames recues avec err sur voie 1*/
      Ushort Nb_Of_Frames_Nok_2;     /* Nbre trames recues avec err sur voie 2*/
      Ushort Nb_No_Echo_Transmit_Error;	/* Nbre trames emises avec err
						( sans echo ligne) */
      Ushort Error_Testp_Channel1;/* Nbre erreurs sur voie 1  lors du TESTP */
      Ushort Error_Testp_Channel2;/* Nbre erreurs sur voie 2 lors du TESTP  */

    }CHANNEL_COUNTERS;

typedef struct {
	Ushort BA_Current_Macrocycle; /* Adresse Macrocycle en cours execution*/
	Ushort BA_Status;	      /* Etat de fonctionnement du BA */
        Ushort FIELDUAL_Status;       /* Bit0	: Etat voie 1
                                         Bit1	: Etat voie 2
 					 Bit2	: Erreur transmission sur voie 1
					 Bit3	: Erreur transmission sur voie 2
					 Bit4	: Etat watchdog ligne 1
					 Bit5	: Etat watchdog ligne 2
					 Bit6 a Bit15 : reserve */
        }ETAT_RESEAU;



/*<BA_Status>

#define	BA_STOPPED
#define	BA_STARTING
#define	BA_IDLE
#define	BA_SENDING
#define	BA_PENDING
#define	BA_WAITING_TIME
#define	BA_WAITING_SYNC
#define	BA_MSG_WINDOW
#define	BA_APER_WINDOW
*/

/* <FIP_Status>

#define FIP_INITIALISATION	// Etat Initialisation en cours
#define	FIP_OPERATIONNEL	// Etat Operationnel
*/

/*<Val1/2>
#define	VALID
#define INVALID
*/

/*<Et1/2> et <WD1/2>
#define	ERROR
#define NO_ERROR
*/


/******* ETAT RESEAU   ***************/

typedef struct {

       CHANNEL_COUNTERS   Channel_Counters;
       ETAT_RESEAU  	  Etat_Reseau;
      }FIP_ETAT_RESEAU ;


typedef struct {
     Ushort X[64];
	} SProg_TST_P ;

#define NO_EVT_TESTP_DEBUT    01
#define NO_EVT_TESTP_FIN      02
#define EVT_BA_SUSPEND     0x500

#define EVT_TESTP_DEBUT   EVT_BA_SUSPEND +NO_EVT_TESTP_DEBUT
#define EVT_TESTP_FIN     EVT_BA_SUSPEND +NO_EVT_TESTP_FIN

#define Nb_Page_Max_PREVU   400

#if ( Nb_Page_Max > Nb_Page_Max_PREVU )
#error  debordement Nb_Page_Max
#endif

/***FIPGEN definition******/

/* Bit map :
   Table de bit utilise pour la gestion des pages memoires
*/

typedef struct { Ushort bit_map[1];} BM;

typedef struct { Uchar bit_map[1];} BMc;

typedef struct { FDM_MPS_VAR_REF *Var[1];} Xref_Key_Var;


/***FIPDIAG definition******/
enum TYPE_TEST_DIAG{
	NO_TEST_PRG =1,
	NO_TEST_RAM,
	NO_TEST_FIPCODE,
	NO_TEST_CTRL_RAM,
	NO_TEST_COMPOSANT
};
enum TYPE_TEST_COMPOSANT{
        NO_TEST_FULLFIP_PRET=1,
	NO_TEST_IC,
	NO_TEST_FE,
	NO_TEST_AE,
	NO_TEST_SV,
	NO_TEST_PREPAR_WRITE_FIFO,
	NO_TEST_WRITE_FIFO,
        NO_TEST_PREPAR_READ_FIFO,
	NO_TEST_READ_FIFO,
	NO_TEST_IRQ,
	NO_TEST_REGS,
        NO_TEST_PREPARE_EOC_TIMER,
        NO_TEST_TIMER,
        NO_TEST_EOC,
        NO_FIN_TEST
};

typedef struct {
	  unsigned int   base;
	  unsigned short checksum;
        }BASE_CHECKS;



typedef struct {

      Ushort 	Frame_Par ;
      Ushort 	Desc_Par ;


/******* BA ****************/
	struct{
		struct  _FDM_BA_REF *BA_in_use;
		Ushort  Current_Macrocycle;   /*Adr Macrocycle en cours
						  execution*/
		Ushort  Status;	              /* Etat de fonctionnement du BA */

		Ushort  Last_MC_Adr;
		int     Max_priority;
		int     Current_priority;
		int     Mode_degrade;

		int	Synchro_recu_user;
		int	Synchro_recu_BA;

		int	BA_actif;
		int	BA_running;

		int	cpt_timeout_suspend;

		int	tst_p_en_cours;
		int	cpt_nbre_tst_p;
		int	no_abonne_teste;
		int	Flag_voie_test;
		int	Flag_traitement;
		int	id_presence_recu;
		Ushort 	B0_tst_p;
		SProg_TST_P *Prog_Ba_TestP;
		FDM_AE_LE_REF AE_LE_TSTP ;
		int 	seuil_TSTP;
		int 	Prio_Stop;
	}Ba;

#if ( FDM_WITH_FREE_ACCESS == NO )
	SProg_TST_P  SProg;
#endif
/******* MEDIUM  ***************/
	struct MediumDef {

		int 	Voie_autorisee;
		int	Voie_Valide;
		int 	Voie_en_cours;
		int	double_medium;

		int	etat_precedant_des_voies;
		int	memo_drive;

		int	memo_EWD;
		int	memo_EWD_CDE ;

                int 	traffic[2] ;
		int	defaut_emission[2];
		int	defaut_reception[2];
		int     defaut_test_p[2];
		int	defaut_voie[2];
		int	defaut_tst_p[2];
		int	nb_tst_p;

		Ushort  FIELDUAL_Status;
		CHANNEL_COUNTERS  defaut_fip;
		CHANNEL_COUNTERS  defaut_fip_valeur_courante;

		Ushort	Seuil_detection_en_reception;

		_FDM_XAE_Tempo_Ref	Timer_tst;

		Ushort Histo;

		int	duree_invalidation;
	}Medium;

/******* SMMPS ****************/
  int aaa;
	struct {
		Uchar  PDU0;		/* 0x80 		*/
		Uchar  lg0;		/* 0x20			*/
		Uchar  V0[32];
		Uchar  PDU1;		/* 0x81 		*/
		Uchar  lg1;		/* 0x20			*/
		Uchar  V1[32];
	} Liste_present;
  int bbb;
	struct _Report_Variable {
		Uchar  PDU_0;		/* 0x50 		*/
		Uchar  V0_ok[2];	/* defaut_fip[0]    	*/
		Uchar  PDU_1;		/* 0x51 		*/
		Uchar  V1_ok[2];	/* defaut_fip[1]    	*/
		Uchar  PDU_2;		/* 0x52 		*/
		Uchar  V0_hs[2];	/* defaut_fip[2]    	*/
		Uchar  PDU_3;		/* 0x53 		*/
		Uchar  V1_hs[2];	/* defaut_fip[3]    	*/
		Uchar  PDU_4;		/* 0x54 		*/
		Uchar  bilan[2];	/* 0    		*/
	} Report_Variable;
  int ccc;
	struct {
		Uchar  PDU;	/* 0x80 */
		Uchar  lg;	/* 03   */
		Uchar  lg_ident;
		Uchar  k;	/* 0    */
		Uchar  Status;	/* [7..4]= etat BA ; [3..0] priorite BA */
	} Presence;
  int ddd;
	struct {
		Uchar  PDU;	/* 0x80 */
		Uchar  lg;	/* 02   */
		Uchar  MC_High;	/* poids fort num Macrocycle    */
		Uchar  Station;	/* Num de station */
	} Synchro_BA  ;


/*********** Ressources FIPGEN ************************/
	struct {
		Ushort  Fip_run;

		enum    IMAGE_NR image;

		Ulong	BASE_0[3];
		Ulong	BASE_1;
		Ulong	BASE_2;

 		BM  *ptr_MAP;		 /* Bit Map { Cle descripteur de variables }	*/
 		int Dim_Map_V;

 		BM  *ptr_MAP_v;		 /* Bit Map { page de donnees 		   }    */
 		BMc *N_PAGE_BA ;	 /* Bit Map { page BA programme 	   }    */

 		_TABLE  *D_Trame_TABLE ;
 		FIFO_INT_TYPE Del_encours;
		Xref_Key_Var  *Xrf;
	} fipgen ;


	struct {		/* Table de configuration systeme  */

		Ushort 	Base_0;			/* Taille et adresse debut de table des descripteurs de trames. */
		Ushort 	Base_1;			/* Adresse debut de table des descripteurs de variables */
		Ushort 	First_Var_Time;		/* Premier descripteur configure avec status  */
		Ushort 	Last_Var_Time;		/* Dernier descripteur configure avec status  */
		Ushort 	Timer_Cnt;
		Ushort 	Page_Lib_User;		/* Numero page memoire pour interface utilisateur (R/W var) */
		Ushort 	Page_Lib_Rec;		/* Numero page memoire pour interface ligne (reception var) */
		Ushort 	Page_Lib_Emi;		/* Numero page memoire pour interface ligne  (emission var) */
		Ushort 	TSlot_Value;		/* Valeur de TSlot en ms				    */
		Ushort 	Extend_Par;
		Ushort 	Type_Station;
		Ushort 	EOC_Signal;
		Ushort 	Mode_Fonct;		/* acces libre */
		Ushort 	Redondancy;
		Ushort 	TXCK_Mode;
		Ushort 	Reserved_Word0;

	        Ushort  Msg_Adr_Fifo_Rec;     /* fifo messagerie reception*/
	 	Ushort  Msg_Buffer_PForts;
	 	Ushort  Msg_Buffer_Pfaibles;
	 	Ushort  Msg_Repeat;
		Ushort	Msg_Adr_Fifo_Emi[9]; /* fifo messagerie emission*/
	        Ushort 	Reserved_Word1[3];

			/* Table de configuration pour arbitre de bus */
		Ushort  BA_Base_B ;
		Ushort  BA_Adr_Fifo_TRQ[3];
		Ushort  BA_Num_Abonne;
		Ushort  BA_Num_Max_Abonne;
		Ushort  BA_Priority;
		Ushort  BA_Start_Time_Par;
		Ushort  BA_Election_Time_Par;
		Ushort  BA_End_Macro_Time_Value; /* Max Ab*sil/Tslot */

		Ushort  BA_Nr;
		Ushort  BA_Nr_Max;

		Ushort  FIP_Reserve[20]	;	/*  */

	} Page_2000 ;


/*********** Ressources MPS ************************/

	_FDM_XAE_Tempo_Ref T_aper_N;/* timer demande apper Normal*/
	_FDM_XAE_Tempo_Ref T_aper_U;/* timer demande apper Urgent*/

	 INTEG_FILE_TYPE *FIFO_MPS ;
/*********** Ressources SM_MPS *********************/
	 INTEG_FILE_TYPE *FIFO_SM_MPS ;
	FDM_AE_LE_REF SM_MPS_Presence;
	FDM_AE_LE_REF SM_MPS_Identification;
	FDM_AE_LE_REF SM_MPS_Repport;
	FDM_AE_LE_REF SM_MPS_Liste_Present;
	FDM_AE_LE_REF SM_MPS_Synchro_BA;

	Uchar Ident_buffer[128];


/*********** Ressources LAI *********************/

	TASK_REFERENCE_TYPE Task_Ref_MPS;
	SOFT_REFERENCE_TYPE Soft_Ref_MPS ;
	INTEG_FILE_TYPE	    Files_MPS[1];

	TASK_REFERENCE_TYPE Task_Ref_SM_MPS;
	SOFT_REFERENCE_TYPE Soft_Ref_SM_MPS ;
	INTEG_FILE_TYPE	    Files_SM_MPS[1];


	TASK_REFERENCE_TYPE Task_Ref_MSG_T;
	SOFT_REFERENCE_TYPE Soft_Ref_MSG_T;
	INTEG_FILE_TYPE	    Files_MSG_T[11];



/*********** Ressources MESSAGERIE  ************************/
	struct {
	/* Emission */
		Ushort TAB_SEGMENT[256] ; /* image de la table des segments  */
		Ulong  		CANAL[9] ;/* image de la table des segments  */
 		Ushort          Image_canal[9];	/*_position_CH_I1
						   et/ou _position_CH_I2 */
		TEMPO_DEF_C	Timer_T[9];

		Ushort 		Cpt_canal_T[9]; /* [0..32 ] */
                FDM_MSG_TO_SEND *tab_wait_cnf[9][MAX_TX_BUFFER_EMI];
                Ushort		Desc_fifo_T[9];     /* adresse descr fifo */

                FIFO_INT_TYPE 	fifo_CNF_T[9];

     	       /* Reception */
		int 		Cpt_Receive ;

  		_TABLE          *Adr_TAB_DLL_Local ;

		T_File 		*Reveive_pool;
	} Messagerie;

/*********** Ressources FIPDIAG ************************/
	struct {
	         Fip_code_Programme_Type *fipcode_prg;
		 enum TYPE_TEST_DIAG 	 type_test;
		 int			 test_number;
		 int			 page_number;
		 Acces_Key		 ram_test_key;
		 Ushort			 Write_Data[64];
		 Ushort			 Read_Data[64];
		 Uchar			 Flag_it_eoc;
		 Uchar 			 Flag_it_irq;
		 _FDM_XAE_Tempo_Ref	 Timer_tst;
		Ushort (*TST_ON_LINE)(struct _FDM_REF *Ref);
		}fipdiag;

/****** fipcode ************/
	int			Nb_Page_Vrai;
	BASE_CHECKS   tab_BC[ Nb_Page_Max_PREVU ]; /* Nb_Page_Max_PREVU >= Nb_Page_Max Nb_Page defini dans fipcode6.h*/


/*********** Monotor Ressources  ************************/
/* Semaphores */
#if ( FDM_WITH_NT == YES  )
    __Object_Mutex_Type__	Modif_BD_Semaphore; /* sm modif DB en cours */
    __Object_Mutex_Type__  	Task_Semaphore ;    /* sm autre que l'qcces qu composant FIP */
    __Object_Mutex_Type__   	Semaphore ;	     /* sm composant FIP */
    __Object_Semaphore_Type__   Vcom ;	    	     /* sm Vcom (acces libre ) */
#else
    __Object_Semaphore_Type__	Modif_BD_Semaphore; /* sm modif DB en cours */
    __Object_Semaphore_Type__   Task_Semaphore ;    /* sm autre que l'qcces qu composant FIP */
    __Object_Semaphore_Type__   Semaphore ;	     /* sm composant FIP */
    __Object_Semaphore_Type__   Vcom ;	    	     /* sm Vcom (acces libre ) */
#endif

/* Memory region */
    Ulong	 rnid  ;
    Ulong	 asize ;
#if		( FDM_WITH_FREE_ACCESS == YES )
struct {
	Ushort *BASE_1;
	Ushort *FifoUser;
	Ushort *FifoFip;
	Ushort *FifoUrgent;
	} AL;
#endif
#if			( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
	struct {
		FDM_AE_LE_REF 			SM_MPS_Seg_Par ;
	} fipio;
#endif


} _FDM_private_variables ;


/* utilitaires en tous genres */

typedef struct _FDM_REF {
    FDM_CONFIGURATION_HARD H;
    FDM_CONFIGURATION_SOFT S;
    _FDM_private_variables  FDM_Internal_Use ;
} FDM_REF ;




void FDM_INITIALIZE_TASK_MPS    ( FDM_REF *Ref, void (*) ( struct _FDM_REF* )  );
void FDM_INITIALIZE_TASK_SM_MPS ( FDM_REF *Ref, void (*) ( struct _FDM_REF* )  );
void FDM_INITIALIZE_TASK_MSG_T  ( FDM_REF *Ref, void (*) ( struct _FDM_REF* )  );

extern void _FIP_TEST_MEDIUM(_FDM_XAE_Tempo_Ref *T);

extern void _UPDATE_SEG_PARAM_VAR (  _FDM_XAE_Tempo_Ref* T);

#endif
