/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm.h
  Description     : Interface  utilisateur ( procedures et Classes )


  Date de creation: Fri Mar 15 15:10:57 MET 1996

Version:4.0.2
	a/- modif Buffer--> FBuffer
  	   ( Buffer est utilise dans <stdio.h> du compilateur METAWARE 16bits)

Version:4.1.2
	a/-simplification FDM_CONFIGURATION_SOFT->NB_OF_DIFFERENT_ID_PROG_BA
  	   remplace :
		FDM_CONFIGURATION_SOFT->AMTRQ_Dim
		FDM_CONFIGURATION_SOFT->UATRQ_Dim
		FDM_CONFIGURATION_SOFT->NATRQ_Dim
Version:4.2.1
	a/- correction des Macros
		fdm_ba_loaded
		fdm_get_image
	b/- Class MPS_... on force le type correct a l'appel du constructeur.
	c/- procedure User_Signal devinent de type ( struct _FDM_REF*)
	d/-suppression des champs Frame_Par et Desc_Par remplace par
		NB_OF_USER_MPS_VARIABLE
	e/-EXTERNAL_TXCK

Version:4.3.1
	a/- FDM_WITH_WIN32
	b/- Configuration pour FIPIO

Version:4.5.1
	a/ Nouveau : Default_Medium_threshold => FDM_CONFIGURATION_SOFT
            ( rapport mauvais / bon du nbre de trame en % )
	   pour calcul des defauts dans "TEST_MEDIUM"
	   fdm_valid_medium ( FDM_REF * , enum  _MEDIUM_DEF  );
		supp 3eme  param
	b/ sup inline SMMPS ,MSG_SEND et MPS_Fifo_Fifo_Empty

Version:4.7.0
	a/ Nouveau : instruction BA SYN_WAIT_SILENT

***********************************************************/

#ifndef __fdm_h
#define __fdm_h

#include "user_opt.h"

#ifdef __cplusplus
/**** User FDM V4 class ***/
class Fdm_Initialize ;
class Fip_Device_Manager ;
class Bus_Arbitrator ;
class AE_LE ;
class MPS_Var_Prod ;
class MPS_Var_Cons ;
class MPS_Var_Sync ;
class MPS_Areceive ;
class MPS_Var_Cons_and_Prod;
class MsgDataLinkFullduolex ;
class MsgDataLinkToSend ;
class MsgDataLinkToReceive ;
class GenericTime  ;
#endif

typedef unsigned long  Ulong ;
typedef unsigned short Ushort;
typedef unsigned char  Uchar;

typedef struct {
    char Version;
    char Revision ;
    char Indice ;
} FDM_VERSION_ELEMENT ;

typedef struct {
    FDM_VERSION_ELEMENT fdmV4;
    FDM_VERSION_ELEMENT fipcodeV6 ;
} FDM_VERSION ;






/*******************************************************************/
/******************  valeur des CR de fonctions    *****************/
/*******************************************************************/
#define FDM_OK	0
#define FDM_NOK	0xffff




/*******************************************************************/
/*******************     liste des code erreur     *****************/
/*******************************************************************/



enum CODE_ERROR {
    _NO_ERROR				= 0x000,
    _FIPCODE_ERROR			= 0x100,
    _TIME_OUT	 			= 0x201,
    _FIPCODE_RESPONSE_IMPOSSIBLE ,
    _FIPCODE_FAIL,
    _INIT_TIMER_FAIL,
    _INIT_ABORTED,
    _RN_FAILED,
    _SM_FAILED,
    _CIRCUIT_ACCES_FAILED ,
    _ILLEGAL_POINTER ,
    _MEDIUM_EWD_FAILED ,
    _FIFO_ACCES_FAILED ,
	_RTX_REPORT_ERROR ,

    _ALLOCATE_MEMORY_FAULT	 	= 0x401,
    _PRIVATE_FULLFIP_RAM_OVERFLOW,
    _FRAME_DESCRIPTOR_TABLE_OVERFLOW,
    _VARIABLE_DESCRIPTOR_TABLE_OVERFLOW,
    _VARIABLE_TABLE_OVERFLOW,
    _VARIABLE_ALREADY_EXIST,
    _ILLEGAL_IDENTIFICATION_PARAMETER,
    _ILLEGAL_IDENTIFICATION_LENGTH,
    _INTEGRITY_DATA_BASE_FAULT,
    _SYNCHRO_VAR_ALREADY_EXIST,
    _INDICATION_PROG_MISSING,
    _CONFLICT_CONFIGURATION,

    _GEN_AE_LE_ILLEGAL_LENGTH	 	= 0x501,
    _GEN_AE_LE_ILLEGAL_STATE,
    _GEN_AE_LE_ILLEGAL_RANG,
    _GEN_AE_LE_ILLEGAL_POSITION,
    _GEN_AE_LE_ILLEGAL_MODIFICATION ,
    _GEN_AE_LE_START_NOT_POSSIBLE ,
    _GEN_AE_LE_TIME_PROG_MISSING,
    _GEN_AE_LE_SYNCHRO_PROG_MISSING,
    _GEN_AE_LE_SYNCHRO_PROG_ILLEGAL,
    _GEN_AE_LE_ILLEGAL_CONFIGURATION ,

    _GEN_BA_CHEKSUM_ERROR 		= 0x601,
    _GEN_BA_LIST_ERROR,
    _GEN_BA_END_BA_NOT_FOUND,
    _GEN_BA_NOT_EXISTED_INSTRUCTION ,
    _GEN_BA_NOT_VALID_TIME,
    _GEN_BA_NO_ID_IN_LIST,
    _GEN_BA_DEF_DICHO,
    _GEN_BA_DEF_DEM_MEM_MC,
    _GEN_BA_DEF_DIM_FILES,

    _LEVEL_PRIORITY_BA 		= 0x901,
    _MAX_SUBSCRIBER_BA,
    _DELETE_MC_IN_USE,
    _USER_NB_OF_USER_MPS_VARIABLE ,
    _USER_MODE_FAULT ,
    _VAR_PERIODE_FAULT ,
    _LENGTH_VAR_FAULT ,
    _USER_NR_OF_TX_BUFFER_FAULT ,
    _USER_SWITCH_FAULT,
    _NON_CONSUMING_VARIABLE ,
    _NON_PRODUCING_VARIABLE ,
    _IMAGE_FAULT_ON_REQUEST ,
    _REQUEST_RAN_ALWAYS,
    _NON_TIME_VARIABLE,
    _INDICATION_VARIABLE_IN_USE,
    _BA_STOPPED_ON_TIME_OUT,

     _CHANNEL_ALREADY_EXIST	=0xA01,
     _INVALID_NO_CHANNEL,
     _CHANNEL_NOT_CONFIGURED,
     _NOT_TX_BUFFER_ON_CHANNEL,
     _NOT_CREATED_CHANNEL_ON_THIS_VARIABLE,
     _MSG_PROG_MISSING,
     _EXCEEDED_SEGMENT_NUMBER,
     _MISSING_MEMORY_DELETED_MESSAGE,
     _IMAGE_NOT_CONFIGURED,

     _GEN_MESSAGING_USER_ACK_EMIS = 0xB01,
     _GEN_MESSAGING_USER_ACK_RECEP,
     _GEN_MESSAGING_BLOCK_DESC_ERROR,
     _MESSAGING_CONTEXT_NOT_TO_SEND,
     _MESSAGING_CONTEXT_NOT_ALLOWED,
     _GEN_MESSAGING_ALREADY_CONFIGURED,
     _GEN_MESSAGING_ILLEGAL_SEGMENT,


     _TEST_RAM_FAIL	       	=0xC01 ,

     _DIAG_FAIL_ON_BUSY_PIN,
     _DIAG_FAIL_ON_EOC_PIN,
     _DIAG_FAIL_ON_IRQ_PIN,
     _DIAG_FAIL_ON_SV_PIN,
     _DIAG_FAIL_ON_FR_PIN,
     _DIAG_FAIL_ON_AE_PIN,
     _DIAG_FAIL_ON_FE_PIN,
     _DIAG_FAIL_ON_IC_PIN,
     _DIAG_FAIL_ON_INTERNAL_FIFO,
     _DIAG_FAIL_ON_INTERNAL_REGISTERS,
     _DIAG_FAIL_ON_INTERNAL_TIMERS,

     _ON_LINE_DIAG_FIPCODE_FAIL ,
     _ON_LINE_DIAG_CIRCUIT_ACCES_FAILED ,
     _ON_LINE_DIAG_TEST_RAM_FAIL ,
     _TEST_ON_TICKS_ON_LINE,

     _ERR_FIP_DOWLOAD_FILE = 0xD01


};


typedef struct {
	enum   CODE_ERROR Fdm_Default;
      	union {
 		struct {
  		      	unsigned _Ustate           		: 3 ;
  		      	unsigned _Var_State       		: 8 ;
  	      	} Fipcode_Report ;
		Ulong Additional_Report ;
	} Information ;
      } FDM_ERROR_CODE ;



/*=============== AE.LE definition =============================*/

typedef  const void * USER_BUFFER_TO_READ ;
typedef  struct {
	Uchar Pdu_Type;
	Uchar Pdu_Length;
	Uchar FBuffer[126];
	} FDM_MPS_VAR_DATA ;

typedef  struct {
	Ulong Network_Delay;
	Uchar Pdu_Type;
	Uchar Pdu_Length;
	Uchar FBuffer[126];
	} FDM_MPS_VAR_TIME_DATA ;

enum _FDM_MPS_READ_CNF {
	_FDM_MPS_READ_OK			= 0,
	_FDM_MPS_USER_ERROR	,
	_FDM_MPS_STATUS_FAULT   ,
	_FDM_MPS_STATUS_TO
};

typedef struct {
	enum _FDM_MPS_READ_CNF	Report;

	unsigned Non_Significant 		:1 ;
	unsigned Promptness_false 		:1 ;
	unsigned Signifiance_status_false 	:1 ;
	unsigned Refresment_false 		:1 ;

} FDM_MPS_READ_STATUS ;

enum  IMAGE_NR {
	IMAGE_1 =0,
	IMAGE_2
	};

enum  FDM_FLAGS {
	FLAG_OFF =0,
	FLAG_ON  =1
	};

enum  _FDM_CHANGE_ALLOWED {
	CHANGE_NOT_ALLOWED =0,
	CHANGE_ALLOWED
	};

enum _FDM_CHANGE_TYPE {
	_PRODUCED=1,
	_CONSUMED
};

enum _FDM_TYPE_COMMUNICATION {
	VAR_SYNCHRO	= 0,
	VAR_PRODUCED,
	VAR_CONSUMED,
	VAR_CONS_PROD
};

struct _FDM_MPS_VAR_REF;	/* User SM-MPS var reference */
struct _FDM_REF ;

typedef struct  {
      struct {
            unsigned Reserved    : 5;  /* must be set to 0 */
            unsigned Position    : 2;  /* 0 or 1 if  mono image
					1= image 1 ;
					2= image 2 ;
					3= image 1 and 2 ;
					*/
            unsigned Communication : 2;/* 0 : synchro ;
            				  1 : produced ;
            				  2 : consumed;
            				  3 : consumed becomming produced
            				*/
            unsigned Scope       	: 1;  /* 0 = remote; 1 = local	*/

            unsigned With_Time_Var    	: 1;  /* optional */
            unsigned Refreshment 	: 1;  /* optional */
            unsigned Indication  	: 1;  /* optional */
            unsigned Priority    	: 1;  /* optional */
            unsigned RQa         	: 1;  /* optional */
            unsigned MSGa        	: 1;  /* optional */
            } Type ;
       Ushort ID ;
       Ushort Var_Length ;
       Ulong  Refreshment_Period ;
       Ulong  Promptness_Period ;
       int    Rank;			/* AELE Rank */
       struct {				/* optional  */
		void (*User_Signal_Synchro)   ( struct _FDM_REF *);
		void (*User_Signal_Asent)     ( struct _FDM_MPS_VAR_REF *);
		void (*User_Signal_Areceive)  ( struct _FDM_MPS_VAR_REF *);
		void (*User_Signal_Var_Prod)  ( struct _FDM_MPS_VAR_REF *,Ushort  Report );
		void (*User_Signal_Var_Cons)  ( struct _FDM_MPS_VAR_REF *,FDM_MPS_READ_STATUS  Report ,
						FDM_MPS_VAR_DATA *);
	} Signals;
      } FDM_XAE;

#include "fdmtype.h"
#include "fipcode6.h"


enum _FDM_AE_LE_STATE  {
 	AE_LE_NOT_EXIST		= 0 ,
 	AE_LE_CONFIG 		= 1 ,
 	AE_LE_RUNNING		= 3
};


/*=======================================================*/
/*=======================================================*/
/*================== SM_MPS Typedef =====================*/
/*=======================================================*/
/*===================Presence============================*/

typedef struct {
	Uchar  Report;
	Uchar  Subscriber;
	Uchar 	Ident_Length;
	struct {
		Uchar BA_Status;
		Uchar BA_Priority;
	} BA_Information;
} FDM_PRESENCE_VAR;



/*===================Identification=============================*/
typedef struct {
	Uchar  Report;
	Uchar  Subscriber;
	Ushort Identification_length;
	Uchar  Identification [128];
} FDM_IDENT_VAR;

/*===================Synchro BA =============================*/
typedef struct {
	Uchar  Report;
	Uchar  Hi_MC_Nr;
	Uchar  Subscriber;
} FDM_SYNCHRO_BA_VAR;


/*===================Liste present=============================*/
typedef struct {
	Uchar  Report;
	Uchar  Nop;
	Uchar Bus1_P[32];
	Uchar Bus2_P[32];
	} FDM_PRESENT_LIST ;


/*===================Repport Variable==========================*/
typedef struct {
	Uchar  Report;
	Uchar  Subscriber;
	Ushort Nb_Of_Transaction_Ok_1;
	Ushort Nb_Of_Transaction_Ok_2;
	Ushort Nb_Of_Frames_Nok_1;
	Ushort Nb_Of_Frames_Nok_2;
	Ushort Activity_Status;
	} FDM_REPORT_VAR ;


/* CR fonction user sur demande de la valeur d'une variable gestion reseau */
/*-------------------------------------------------------------------------*/
	/*0 =pas  de destruction de variable reseau demande , 1 = destruction*/
#define NO_VAR_DELETE	0
#define VAR_DELETE	1

/* CR des demande de lecture de variable gestion reseau */
/*-------------------------------------------------------------------------*/
#define VAR_TRANSFERT_OK	0
#define VAR_TRANSFERT_NOK	0xff





/*******************************************************************/
/*=============  Initialisation generale de FDM ===================*/
/*******************************************************************/

enum _FULLFIP_Mode_List {
    WORLD_FIP_31	= 0,
    WORLD_FIP_1000,
    WORLD_FIP_2500,
    WORLD_FIP_5000,
    FIP_31		= 0x10,
    FAST_FIP_1000,
    FIP_2500,
    FIP_5000,
    SLOW_FIP_1000	= 0x21,
    FDM_OTHER		= 0x30
    };


typedef struct {
    char * Vendor_Name;
    char * Model_Name;
    char * Revision;
    char * Tag_Name;
    char * SM_MPS_Conform;
    char * SMS_Conform;
    char * PMDP_Conform;
    char * Vendor_Field;
    } FDM_IDENTIFICATION;

/* FDM_CONFIGURATION_SOFT.Type  definition */
#if 						(  FDM_WITH_BI_MEDIUM == YES )
#define TWO_BUS_MODE			0x0020
#endif
#define MESSAGE_RECEPTION_AUTHORIZED	0x0004
#define TWO_IMAGE_MODE			0x0040

#if 						(  FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == NO )
#define EOC_PULSE_MODE			0x0100
#endif

#define IMAGE_2_STARTUP 		0x0200
#define TEST_RAM_STARTUP		0x0400
#define EXTERNAL_TXCK			0x0800

#if						(  FDM_WITH_FIPIO == YES )
#define FIPIO_MONO		 		0x1000
#define FIPIO_BI		 		0x2000
#endif

#if 						(  FDM_WITH_DIAG == YES )
#define IRQ_CONNECTED			0x4000
#define EOC_CONNECTED			0x8000
#endif

typedef struct {
      Ushort 	Type;
      enum   	_FULLFIP_Mode_List Mode;
      Ushort 	TSlot ;
      Ushort 	NB_OF_USER_MPS_VARIABLE ;
      Ushort 	BA_Dim ;
      Ulong  	FULLFIP_RAM_Dim ;
      Ushort 	NB_OF_DIFFERENT_ID_PROG_BA ;
      Ushort	Nr_Of_Repeat ;
      Ushort	Nr_Of_Tx_Buffer[9] ; 	/* [0..32] 0 = unused channel */
      /* User SM-MPS call back Procedures */
      void   (*User_Present_List_Prog )   ( struct _FDM_REF* , FDM_PRESENT_LIST   * ) ;
      Ushort (*User_Identification_Prog ) ( struct _FDM_REF* , FDM_IDENT_VAR      * ) ;
      Ushort (*User_Report_Prog )   	  ( struct _FDM_REF* , FDM_REPORT_VAR     * ) ;
      Ushort (*User_Presence_Prog ) 	  ( struct _FDM_REF* , FDM_PRESENCE_VAR   * ) ;
      void   (*User_Synchro_BA_Prog ) 	  ( struct _FDM_REF* , FDM_SYNCHRO_BA_VAR * ) ;

      Ushort	Test_Medium_Ticks;
      Ushort	Time_Out_Ticks;		/* for MPS and SM-MPS */
      Ushort	Time_Out_Msg_Ticks;
      Ushort	Online_Tests_Ticks;

      Ushort	Default_Medium_threshold;	/* bad compared with good in % */

#if			( FDM_WITH_BA == YES ) && (FDM_WITH_FIPIO == YES )
      Ushort	Segment_Paramers_Ticks;
#endif
      struct _User_responsability {		/* if _FULLFIP_Mode_list.OTHER */
      			/* else not use */
      	 Ushort TIMER_CNT_REGISTER ;
      	 Ushort MODE_REGISTER ;
	} User_responsability ;

      void (*User_Signal_Mps_Aper)  (  struct _FDM_REF*  ) ;
      void (*User_Signal_Smmps )    (  struct _FDM_REF*  ) ;
      void (*User_Signal_Send_Msg)  (  struct _FDM_REF*  ) ;
      void (*User_Signal_Rec_Msg )  (  struct _FDM_REF*  ) ;

 	   void * User_Ctxt;

    } FDM_CONFIGURATION_SOFT;

typedef struct _FDM_CONFIGURATION_HARD {
      Uchar 	K_PHYADR ;
      char 	MySegment ;
#if 				( FDM_WITH_OPTIMIZED_BA == YES )
      Uchar 	NumberOfThisBusArbitrator ;
      Uchar 	GreatestNumberOfBusArbitrator  ;
#else
      Uchar 	Reserved[2] ;
#endif
#if		(  FDM_WITH_CHAMP_IO == YES )
     __Port_Type__ LOC_FIP[8];
     __Port_Type__ LOC_FIPDRIVE[4];
#else
      Uchar volatile *  LOC_FIP[8];
      Uchar volatile *  LOC_FIPDRIVE[4];
#endif
      Ushort volatile *  FREE_ACCES_ADDRESS;

     void (*User_Reset_Component)( struct _FDM_CONFIGURATION_HARD * );

     void (*User_Signal_Fatal_Error)( struct _FDM_REF * Ref ,FDM_ERROR_CODE );
     void (*User_Signal_Warning )   ( struct _FDM_REF * Ref ,FDM_ERROR_CODE );

     MEMORY_RN  *Memory_Management_Ref   ;/* options for memory management */

     struct _FDM_REF *Ptr_Autotests ; /* internal use for fipdiag reference */

   } FDM_CONFIGURATION_HARD ;







/********************************************************************/
/*=============== BA - Meduim definitions   ========================*/
/********************************************************************/
enum _MEDIUM_DEF {
	_MEDIUM_1    = 1,
	_MEDIUM_2    ,
	_MEDIUM_1_2
	  } ;

enum _BA_SET_MODE {
	STANDARD  = 0,
	OPTIMIZE_1 ,
 	OPTIMIZE_2 ,
	OPTIMIZE_3
};


/* CR fdm_macrocycle_loaded */
/*--------------------------*/
#define BA_LOADED     0
#define BA_NOT_LOADED 1


#define ID_DAT    	0x03
#define ID_MSG    	0x05
 

#define NEXT_MACRO      	2
#define SEND_MSG        	4
#define SEND_APER       	5
#define BA_WAIT        	 	6
#define SEND_LIST       	100
#define TEST_P          	103
#define SYN_WAIT        	104
#define SYN_WAIT_SILENT        	105

enum _BA_INF {
 	NOT_BA	  =0 ,
 	LOADED_BA ,
 	ACTIF,			/* ba  idle or starting */
  	RUNNING			/* ba running */
 };

typedef struct {
	enum _BA_INF  	Actif_Running;
	Ushort  	Status_Fiphandler;/* see fipcode6.h */
	Ushort  	Label;
	} BA_INF_STATUS ;


typedef struct {
	Ushort Op_Code;
	Ulong Param1;
	} PTR_INSTRUCTIONS ;

typedef struct {
	Ushort ID;
	Ushort Frame_Code;
	} LIST_ELEMENT;

typedef struct{
	Ushort List_Size; /*taille de la liste en byte */
	LIST_ELEMENT *Ptr_List_Element;
       } PTR_LISTS ;

/*  USER definition list macro   */
#define User_Liste(P)  		(Ushort)sizeof(P),P




/*=======================================================*/
/*=============== Messaging Services Type ===============*/
/*=======================================================*/

typedef enum _FDM_MSG_IMAGE {
	_FDM_MSG_IMAGE_1	= 1,
	_FDM_MSG_IMAGE_2 	,
	_FDM_MSG_IMAGE_1_AND_2

} _FDM_MSG_IMAGE;



typedef struct {
        enum _FDM_MSG_IMAGE 	Position    ;
	Ushort Channel_Nr; 			/*  [ 1 .. 8 ] */
	Ushort Identifier;

} FDM_CHANNEL_PARAM;


typedef struct _FDM_MSG_T_DESC {
    Ushort                	Nr_Of_Bytes;
    Uchar*             		Ptr_Data;
    struct _FDM_MSG_T_DESC     *Next_Block;
} FDM_MSG_T_DESC;

typedef struct _FDM_MSG_R_DESC {
	Ushort               	Nr_Of_Bytes;
	Uchar*             	Ptr_Data;
	struct _FDM_MSG_R_DESC	*Next_Block;
#if	( FDM_WITH_SPECIAL_USAGE == YES )
	_USER_BUFFER_TO_DEFINE
#endif
	Uchar			Data_Buffer[256];
} FDM_MSG_R_DESC;

enum _FIP_MSG_SND_REP {
	_FIP_NOACK_BAD_RP_MSG		= 0x4,

	_FIP_ACK_NO_REC_AFTER_RETRY	= 0x2,
	_FIP_ACK_NEG_AFTER_RETRY	= 0x5,
	_FIP_ACK_NEG_NO_RETRY		= 0x7,
	_FDM_TIME_OUT			= 0x8
};

enum _FDM_MSG_SND_CNF {
	_FDM_MSG_SEND_OK			= 0,
	_FDM_MSG_USER_ERROR	,
	_FDM_DATA_LINK_ERROR
};

enum _FDM_MSG_USER_ERROR_LIST {
        _FDM_MSG_REPORT_OK      		 = 0,
	_FDM_MSG_REPORT_MSG_NOT_ALLOWED  ,
	_FDM_MSG_REPORT_CHANNEL_NOT_ALLOWED	 ,
        _FDM_MSG_REPORT_ERR_LG_MSG	 ,
        _FDM_MSG_REPORT_ERR_MSG_INFOS    ,
        _FDM_MSG_REPORT_INTERNAL_ERROR

 };

typedef struct {
	enum		_FDM_MSG_SND_CNF 	  Valid ;
	enum  		_FDM_MSG_USER_ERROR_LIST  msg_user_soft_report 	;
 	enum		_FIP_MSG_SND_REP          Way	;
	} FDM_MSG_SEND_SERVICE_REPORT ;


enum _FDM_MSG_ACK_TYPE {
	_FDM_MSG_ACK_TYPE_SDN = MSG_SDN,
	_FDM_MSG_ACK_TYPE_SDA = MSG_SDA

};

/**** User Read Only  ****/
#define TYPE_MDL_EMIS	0x0001
#define TYPE_MDL_RECEP	0x0002

struct  _FDM_MSG_RECEIVED;
struct  _FDM_MSG_TO_SEND;
struct  _FDM_MESSAGING_REF;

typedef struct _FDM_MESSAGING_REF  {
     struct _FDM_REF   *Fip;
     struct {
        unsigned Communication ;
            		/*	1 =sending,
            			2 =receiving
            			3 =emis+recept	*/
    	enum _FDM_MSG_IMAGE 	Position    ;

     } Type ;

     struct{
		void	         (*User_Msg_Rec_Proc)
			(struct  _FDM_MESSAGING_REF * , struct _FDM_MSG_RECEIVED * );
		void*	          User_Qid;	/* optional user info */
		void*	          User_Ctxt;	/* optional user info */
		PRIVATE_FDM_DLI_R Private;
     }Receive_infos;

     struct{
		void	         (*User_Msg_Ack)
			( struct _FDM_MESSAGING_REF * ,struct _FDM_MSG_TO_SEND * );
		void*	          User_Qid;	/* optional user info */
		void*	          User_Ctxt;	/* optional user info */
		Ushort	          Channel_Nr;
		enum _FDM_MSG_ACK_TYPE	 (*User_Msg_Ack_Type)
			(struct  _FDM_REF * , Ulong Remote_Adr );
		int Cpt_Occup;
     }Send_infos;

     Ulong	Local_DLL_Address;
     Ulong	Remote_DLL_Address;

   } FDM_MESSAGING_REF;


/* message a emettre
----------------------*/
typedef struct  _FDM_MSG_TO_SEND {
	struct 		_FDM_MSG_TO_SEND *Next;
	struct 		_FDM_MSG_TO_SEND *Prev;
	int  		Nr_Of_Blocks;
	FDM_MSG_T_DESC  *Ptr_Block ;
	Ulong		Local_DLL_Address;	/* optional ,Bridge bit30 */
	Ulong		Remote_DLL_Address;	/* optional ,fct bit31	*/
	FDM_MSG_SEND_SERVICE_REPORT Service_Report ;

	FDM_PRIVATE_DLI_T Private ;
} FDM_MSG_TO_SEND ;



/* message a recevoir
----------------------*/
typedef struct  _FDM_MSG_RECEIVED {
	struct 		_FDM_MSG_RECEIVED *Next;
	struct 		_FDM_MSG_RECEIVED *Prev;
	int		Nr_Of_Blocks;
	FDM_MSG_R_DESC  *Ptr_Block ;
	Ulong		Local_DLL_Address;
	Ulong		Remote_DLL_Address;

	void*		User_Qid;	/* optional user info */
	void*		User_Ctxt;	/* optional user info */
	FDM_MESSAGING_REF *Ref;

} FDM_MSG_RECEIVED ;



/* Data Link Interface
----------------------*/

typedef struct  {
     enum _FDM_MSG_IMAGE 	Position    ;
     struct{
	void		(*User_Msg_Ack)
				(FDM_MESSAGING_REF*, FDM_MSG_TO_SEND * );
	void*	 	User_Qid;	/* optional user info */
	void*	 	User_Ctxt;	/* optional user info */
	Ushort	 	Channel_Nr;
      }sending;

      Ulong	Local_DLL_Address;
      Ulong	Remote_DLL_Address;

   } FDM_MESSAGING_TO_SEND;



typedef struct  {
     enum _FDM_MSG_IMAGE 	Position    ;
     struct{
	void	(*User_Msg_Rec_Proc)
			(FDM_MESSAGING_REF*,FDM_MSG_RECEIVED * );
	void*		     User_Qid;	/* optional user info */
	void*		     User_Ctxt;	/* optional user info */
	int 		     Number_Of_Msg_Desc;
	int 		     Number_Of_Msg_Block;
     }receiving ;

      Ulong	Local_DLL_Address;
      Ulong	Remote_DLL_Address;

   } FDM_MESSAGING_TO_REC;


typedef struct  {
     enum _FDM_MSG_IMAGE 	Position    ;
     struct{
	void	(*User_Msg_Ack)
			(FDM_MESSAGING_REF*, FDM_MSG_TO_SEND * );
	void*		User_Qid;	/* optional user info */
	void*		User_Ctxt;	/* optional user info */
	Ushort		Channel_Nr;
     }sending;

     struct{
	void	(*User_Msg_Rec_Proc)
		(FDM_MESSAGING_REF*,FDM_MSG_RECEIVED * );
	void*		  User_Qid;	/* optional user info */
	void*		  User_Ctxt;	/* optional user info */
	int 		  Number_Of_Msg_Desc;
	int 		  Number_Of_Msg_Block;
     }receiving ;

      Ulong	Local_DLL_Address;
      Ulong	Remote_DLL_Address;

   } FDM_MESSAGING_FULLDUPLEX;




#include "fdmprive.h"
#include "fdm_os.h"

/********************************************************************/
/******************** IMMEDIATE REPORT*** ***************************/
/********************************************************************/
#define EOC_IRQ_TO_PROCESS	1
#define NO_EOC_IRQ_TO_PROCESS	0
#define IRQ_TO_PROCESS		1
#define NO_IRQ_TO_PROCESS	0
#define EOC_TO_PROCESS		1
#define NO_EOC_TO_PROCESS	0

/********************************************************************/
/******************** PUBLIC FDM FUNCTIONS ***************************/
/********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

FDM_REF *fdm_initialize_network (
	const FDM_CONFIGURATION_SOFT *  U_soft,
	      FDM_CONFIGURATION_HARD *  U_hard,
	const FDM_IDENTIFICATION     *  U_ident );

void fdm_stop_network( FDM_REF * Ref ) ;

/**IT**/
Ushort fdm_process_its_fip (  FDM_REF * );

Ushort fdm_process_it_eoc  (  FDM_REF * );

Ushort fdm_process_it_irq  (  FDM_REF * );

/**BA**/
FDM_BA_REF * fdm_ba_load_macrocycle_fipconfb ( FDM_REF * ,const Ushort*);

Ushort  fdm_ba_delete_macrocycle(FDM_BA_REF *);

Ushort  fdm_ba_external_resync (  FDM_REF *  );

Ushort  fdm_ba_start  (  FDM_BA_REF *  );

Ushort  fdm_ba_commute_macrocycle(  FDM_BA_REF *  );

FDM_BA_REF* fdm_ba_load_macrocycle_manual ( FDM_REF*,
	int Nb_of_Liste, int Nb_of_Instruction ,Ushort Label,
	const PTR_LISTS* ,const PTR_INSTRUCTIONS * );

Ushort fdm_ba_set_priority( FDM_REF *  , Uchar );

Ushort fdm_ba_set_parameters( FDM_REF * ,
		enum _BA_SET_MODE  ,
		Uchar MAX_Subscriber,
		Uchar MAX_Priority );

Ushort fdm_ba_status 	( FDM_REF * ,BA_INF_STATUS * );

Ushort fdm_ba_stop    	(  FDM_REF *  );

/****/
Ushort fdm_valid_medium   ( FDM_REF * , enum  _MEDIUM_DEF  );

Ushort fdm_ack_ewd_medium ( FDM_REF * , enum _MEDIUM_DEF  ) ;

Ushort fdm_switch_image 	( FDM_REF * ,  enum  IMAGE_NR ) ;

Ushort fdm_read_report  	( FDM_REF * ,Uchar Subscriber );

Ushort fdm_read_present_list  	( FDM_REF *  );

Ushort fdm_read_identification  ( FDM_REF * , Uchar Subscriber );

Ushort fdm_read_presence  	( FDM_REF * , Uchar Subscriber );

Ushort fdm_read_ba_synchronize	( FDM_REF *  );

Ushort fdm_get_local_report 	( FDM_REF * , FDM_REPORT_VAR     * ) ;

/**AELE**/

FDM_AE_LE_REF *fdm_ae_le_create ( FDM_REF * ,int  Nb_Vcom , enum  _FDM_CHANGE_ALLOWED );

Ushort fdm_ae_le_start  	( FDM_AE_LE_REF * );

Ushort fdm_ae_le_stop   	( FDM_AE_LE_REF * );

Ushort fdm_ae_le_delete 	( FDM_AE_LE_REF * );

FDM_MPS_VAR_REF * fdm_mps_var_create (	FDM_AE_LE_REF * , const FDM_XAE * );

Ushort fdm_mps_var_change_periods ( FDM_MPS_VAR_REF *  ,
		Ulong Refreshment_Period ,
		Ulong Promptness_Period ) ;

Ushort fdm_mps_var_change_id	  ( FDM_MPS_VAR_REF *  , Ushort New_ID ) ;

Ushort fdm_mps_var_change_RQa	  ( FDM_MPS_VAR_REF *  , enum  FDM_FLAGS ) ;

Ushort fdm_mps_var_change_MSGa	  ( FDM_MPS_VAR_REF *  , enum  FDM_FLAGS ) ;

Ushort fdm_mps_var_change_priority( FDM_MPS_VAR_REF *  , unsigned New_Priority);

Ushort fdm_mps_var_change_prod_cons ( FDM_MPS_VAR_REF * ,
		enum _FDM_CHANGE_TYPE  New_Image_1_Var_type ,
	 	enum _FDM_CHANGE_TYPE  New_Image_2_Var_type );

Ushort fdm_mps_var_write_loc ( FDM_MPS_VAR_REF  * , USER_BUFFER_TO_READ ) ;

FDM_MPS_READ_STATUS fdm_mps_var_read_loc  ( FDM_MPS_VAR_REF *  , FDM_MPS_VAR_DATA *) ;

Ushort fdm_mps_var_time_write_loc ( FDM_MPS_VAR_REF  *  , 
		Uchar * (*User_Get_Value)(void),
		const Ulong * ) ;

FDM_MPS_READ_STATUS fdm_mps_var_time_read_loc  ( FDM_MPS_VAR_REF *  , 
		void (*User_Set_Value)(FDM_MPS_VAR_TIME_DATA *),		
		FDM_MPS_VAR_TIME_DATA *) ;


Ushort fdm_mps_var_write_universal ( FDM_MPS_VAR_REF  * , USER_BUFFER_TO_READ ) ;

Ushort fdm_mps_var_read_universal  ( FDM_MPS_VAR_REF *   ) ;

/**MSG**/

Ushort fdm_channel_create ( FDM_REF *  , FDM_CHANNEL_PARAM * );

Ushort fdm_channel_delete ( FDM_REF *  , Ushort Channel_nr );

	/* messagerie configuration */
FDM_MESSAGING_REF* fdm_messaging_fullduplex_create(FDM_REF*,
					FDM_MESSAGING_FULLDUPLEX*);

FDM_MESSAGING_REF* fdm_messaging_to_send_create(FDM_REF*,
					FDM_MESSAGING_TO_SEND*);

FDM_MESSAGING_REF* fdm_messaging_to_rec_create(FDM_REF*,
					FDM_MESSAGING_TO_REC*);

Ushort fdm_messaging_delete(FDM_MESSAGING_REF*);

/* messagerie emission */

Ushort fdm_send_message(FDM_MESSAGING_REF*, FDM_MSG_TO_SEND*);

Ushort fdm_change_channel_nr(FDM_MESSAGING_REF*, Ushort Channel_NR);

Ushort fdm_change_messaging_acknowledge_type(
	FDM_MESSAGING_REF *,
	enum _FDM_MSG_ACK_TYPE (*User_Msg_Ack_Type )(struct  _FDM_REF * , Ulong Remote_Adr )  );

/* messagerie reception  */


Ushort fdm_msg_ref_buffer_free(FDM_MSG_RECEIVED* Memory_Block);

Ushort fdm_msg_data_buffer_free(FDM_MSG_R_DESC*  Memory_Block);

void fdm_msg_rec_fifo_empty (FDM_REF *  );



/**DIVERS***/

#define fdm_ticks_counter	_FIP_TICKS_COUNTER

const FDM_VERSION *fdm_get_version ( void ) ;

#if						( FDM_WITH_DIAG == YES )

#if 				( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == YES )

void fdm_process_its_fip_test(struct _FDM_REF *);

#define TEST_IRQ_EOC(H) fdm_process_its_fip_test(H.Ptr_Autotests);

#else

void fdm_process_it_eoc_test(struct _FDM_REF *);

void fdm_process_it_irq_test(struct _FDM_REF * );

#define TEST_EOC(H)  	fdm_process_it_eoc_test(H.Ptr_Autotests);

#define TEST_IRQ(H)	fdm_process_it_irq_test(H.Ptr_Autotests);

#endif

Ushort  fdm_online_test ( FDM_REF *  );

#endif


#ifndef __cplusplus

/******** MACROS *****/
#define fdm_initialize 	FDM_INITIALISATIONS

#define fdm_ticks_counter	_FIP_TICKS_COUNTER

#define fdm_change_test_medium_ticks(ref,Value) (ref)->FDM_Internal_Use.Medium.Timer_tst.Timer.T_init = Value

#define fdm_mps_fifo_empty(ref)      _FIP_fifo_empty(&(ref->FDM_Internal_Use.Task_Ref_MPS))

#define fdm_smmps_fifo_empty(ref)    _FIP_fifo_empty(&(ref->FDM_Internal_Use.Task_Ref_SM_MPS))

#define fdm_msg_send_fifo_empty(ref) _FIP_fifo_empty(&(ref->FDM_Internal_Use.Task_Ref_MSG_T))

#define fdm_ae_le_get_state(Ptr_AE) 	(Ptr_AE)->Etat

#define fdm_get_image(ref) (((ref)->FDM_Internal_Use.fipgen.image == 0)? IMAGE_1 : IMAGE_2)

#define fdm_ba_loaded(BA) (((BA) == NULL ) ? BA_NOT_LOADED : BA_LOADED)

#endif

#ifdef __cplusplus
}
#endif


/************************************************************/
/************ Traitement des Evenements *********************/
/************************************************************/

#define BEGIN_SMAP_FIP_EVT { int i; do { i=0;

#define END_SMAP_FIP_EVT   } while ( i );}

#ifdef __cplusplus
#define SMAP_FIP_EVT(Fip_Ref) i |= Fip_Ref;
#else

#if 				( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == YES )
#define SMAP_ON_IRQ_EOC(Fip_Ref) i |= fdm_process_its_fip(Fip_Ref);
#else
#define SMAP_ON_IRQ(Fip_Ref)     i |= fdm_process_it_irq(Fip_Ref);
#define SMAP_ON_EOC(Fip_Ref)     i |= fdm_process_it_eoc(Fip_Ref);
#endif
#endif


#ifdef __cplusplus

/************************************************************/
/**************** INITIALISATION GENERALE********************/
/************************************************************/


class  Fdm_Initialize {
	public :
	Fdm_Initialize()  {FDM_INITIALISATIONS();}
};



/************************************************************/
/******************** Fip Device Manager ********************/
/************************************************************/

class Fip_Device_Manager {
 
 private :
	FDM_REF 	*Fip;
	friend class Bus_Arbitrator ;
	friend class AE_LE ;
	friend class MsgDataLink ;
	friend class MsgDataLinkFullduplex;
	friend class MsgDataLinkToSend ;
	friend class MsgDataLinkToReceive ;
	friend class GenericTime  ;

  public :


  /* constructeur */
  Fip_Device_Manager (
	  FDM_CONFIGURATION_SOFT &U_soft,
	  FDM_CONFIGURATION_HARD &U_hard,
	  FDM_IDENTIFICATION     &U_ident
	) : Fip ( fdm_initialize_network ( &U_soft , &U_hard , &U_ident )) {}

  Fip_Device_Manager (
	  FDM_CONFIGURATION_SOFT *U_soft,
	  FDM_CONFIGURATION_HARD *U_hard,
	  FDM_IDENTIFICATION     *U_ident
	) : Fip ( fdm_initialize_network ( U_soft , U_hard , U_ident )) {}

  Fip_Device_Manager (FDM_REF 	*Ref) : Fip ( Ref) {}

   ~Fip_Device_Manager(){ fdm_stop_network ( Fip );}

  inline FDM_REF* My_Reference_Is () { return  ( Fip ); }

  inline Ushort IS_running ()
	{ return (Ushort) ( ( Fip == NULL ) ? 0 : 1 ); }

  inline Ushort
     Valid_Medium ( enum  _MEDIUM_DEF Voie_desiree )
  	{ return fdm_valid_medium ( Fip , Voie_desiree  ); }

  inline Ushort
     Ack_Ewd_Medium ( enum  _MEDIUM_DEF Voie_desiree )
  	{ return  fdm_ack_ewd_medium( Fip , Voie_desiree  ); }

  inline Ushort Set_Image ( enum  IMAGE_NR  Image)
	{ return  fdm_switch_image ( Fip , Image ); }

  inline Ushort Get_Image ()
	{return (Fip->FDM_Internal_Use.fipgen.image == 0)? IMAGE_1 : IMAGE_2 ;}

  inline Ushort Get_Report ( FDM_REPORT_VAR &Value)
	{ return  fdm_get_local_report 	( Fip , &Value ) ; }

  inline Ushort Get_EocCount ( Ushort *Value)
	{ return   _FIP_READ_WORD ( Fip  ,0xb0,  Value ) ; }

#if 				( FDM_WITH_BA == YES )

  inline Ushort Set_BA_Priority ( Uchar Priority_level )
  	{ return fdm_ba_set_priority( Fip , Priority_level); }

  inline Ushort Set_BA_Parameters (
	enum _BA_SET_MODE  Standard , Uchar MAX_Subscriber, Uchar  MAX_Priority )
  	{ return fdm_ba_set_parameters( Fip ,
		Standard, MAX_Subscriber, MAX_Priority ); }

  inline void BA_Status (BA_INF_STATUS &adr_inf)
    	{  fdm_ba_status( Fip ,&adr_inf); }

  inline Ushort  BA_Stop ( )
  	{ return  fdm_ba_stop  (  Fip  ); }

#if  (  FDM_WITH_FREE_ACCESS == NO )

  inline Ushort BA_Resync ( )
  	{   return fdm_ba_external_resync ( Fip ); }

#endif

#endif

#if 				( FDM_WITH_SM_MPS  == YES )

  inline Ushort  Read_Report(Uchar Subscriber)
	{return fdm_read_report  ( Fip, Subscriber );}

  inline Ushort Read_Present_List( void )
	{return fdm_read_present_list  ( Fip );}

  inline Ushort Read_Ident(Uchar Subscriber)
	{return fdm_read_identification  ( Fip ,  Subscriber );}

  inline Ushort Read_Presence(Uchar Subscriber)
	{return fdm_read_presence  ( Fip,  Subscriber );}

  inline Ushort Read_BA_Synchronize_var ( void )
	{return  fdm_read_ba_synchronize  ( Fip  );}

   void SMMPS_Fifo_Empty();


#endif

#if 				( FDM_WITH_MESSAGING  == YES )

  inline Ushort Create_Channel(FDM_CHANNEL_PARAM* C)
  	{return fdm_channel_create(Fip,C ) ;}

  inline Ushort Delete_Channel(Ushort Channel_Nr)
  	{return fdm_channel_delete(Fip,Channel_Nr ) ;}

  inline void  MSG_REC_Fifo_Empty()
              { fdm_msg_rec_fifo_empty(Fip);}

  void MSG_SEND_Fifo_Empty();


#endif

#if 				(  FDM_WITH_APER == YES )

  void  MPS_Fifo_Empty();

#endif

#if 				( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == YES )

  inline int   On_EocIrq_Pin ( )
	{ return (fdm_process_its_fip(Fip)); }
#else
  inline int   On_Eoc_Pin ( )
	{ return ( fdm_process_it_eoc (Fip)); }

  inline int   On_Irq_Pin ( )
	{ return ( fdm_process_it_irq (Fip)); }

#endif

  Ushort  Physical_Read ( Ulong Addr , Ushort *buffer );

};


#endif

/*=================================================================*/
/*=================================================================*/
/*====================   BA definition ============================*/
/*=================================================================*/
/*=================================================================*/

#ifdef __cplusplus

#define BA_LISTE_REF(Listes) \
	sizeof(Listes)/sizeof(PTR_LISTS),Listes

#define BA_PROG_REF(Programme_BA) \
	sizeof(Programme_BA)/sizeof(PTR_INSTRUCTIONS),Programme_BA


class Bus_Arbitrator {
  private :
	  FDM_BA_REF * Ptr_BA;
  public :
  /* constructeur */
  Bus_Arbitrator (
	  Fip_Device_Manager 	*Fip ,
	   const Ushort		& Programme
	) : Ptr_BA ( fdm_ba_load_macrocycle_fipconfb ( Fip->Fip  , &Programme )) { }


  Bus_Arbitrator (
	Fip_Device_Manager *Fip ,
	  Ushort	                Label,
	  int                   	Nb_of_Liste,
	  const PTR_LISTS      		* Listes,
	  int                   	Nb_of_Instruction,
	  const PTR_INSTRUCTIONS        * Instructions
	) : Ptr_BA ( fdm_ba_load_macrocycle_manual(  Fip->Fip  ,
                 Nb_of_Liste, Nb_of_Instruction , Label,
 	         Listes, Instructions)) { }

  /* modifieur */

  inline Ushort Delete()
        { return  fdm_ba_delete_macrocycle (Ptr_BA);}

  inline Ushort Start ( )
  	{ return  fdm_ba_start ( Ptr_BA  ); }

  inline Ushort Commute ( )
  	{ return  fdm_ba_commute_macrocycle (  Ptr_BA  ); }


  /* consulteur */
  inline int IS_Loaded () {return  (Ptr_BA == NULL ) ? 0 : 1 ; }


};

#else		/******** C+ *********/

#define  Bus_Arbitrator__CREATE_M(Fip,Label,Listes,Programme_BA) \
	fdm_ba_load_macrocycle_manual(Fip,\
		sizeof(Listes)/sizeof(PTR_LISTS),sizeof(Programme_BA)/sizeof(PTR_INSTRUCTIONS),\
		Label,Listes,Programme_BA)

#define  Bus_Arbitrator__Start(BA) 	fdm_ba_start(BA)

#define  Bus_Arbitrator__Commute(BA) 	fdm_ba_commute_macrocycle(BA)


#endif


/*=======================================================*/
/*=======================================================*/
/*==================== MPS AELE =========================*/
/*=======================================================*/
/*=======================================================*/


#ifdef __cplusplus

class AE_LE {
  private :
	FDM_AE_LE_REF * Ptr_AE;
	friend class MPS_Variable;
	friend class MPS_Var_Prod;
	friend class MPS_Var_Cons ;
	friend class MPS_Var_Sync ;
	friend class MPS_Var_Cons_and_Prod;
	friend class FDM ;
  public :

  AE_LE	(  Fip_Device_Manager &Fip, int Nombre_Var) :
  	Ptr_AE (fdm_ae_le_create( Fip.Fip , Nombre_Var , CHANGE_ALLOWED )) {}

  AE_LE ( Fip_Device_Manager *Fip, int Nombre_Var) :
  	Ptr_AE (fdm_ae_le_create( (FDM_REF *) Fip->Fip , Nombre_Var , CHANGE_ALLOWED)) {}

  ~AE_LE(){ fdm_ae_le_delete ( Ptr_AE);}

  inline Ushort Start ( )
  	{ return  fdm_ae_le_start ( Ptr_AE ); }

  inline Ushort Stop ( )
  	{ return  fdm_ae_le_stop ( Ptr_AE ); }

 /* consulteur */

  inline _FDM_AE_LE_STATE State ()
	{ if( Ptr_AE == (FDM_AE_LE_REF *) 0 ) return AE_LE_NOT_EXIST ;
	  else return Ptr_AE->Etat  ;
        }

  inline int IS_Created () {return  (  Ptr_AE == NULL ) ? 0 : 1 ; }



};
#else	/************** C+ ****************/

#define AE_LE__CREATE(Fip,Nombre_Var) 	\
	fdm_ae_le_create(Fip , Nombre_Var, CHANGE_ALLOWED  )

#define AE_LE__Start(Ptr_AE) 	fdm_ae_le_start( Ptr_AE )

#define AE_LE__Stop(Ptr_AE)  	fdm_ae_le_stop( Ptr_AE )

#define AE_LE__Delete(Ptr_AE)  	fdm_ae_le_delete( Ptr_AE )

#define AE_LE__State(Ptr_AE) 	fdm_ae_le_get_state(Ptr_AE)

#endif



/*=======================================================*/
/*=======================================================*/
/*==================== MPS VARIABLE =====================*/
/*=======================================================*/
/*=======================================================*/


#ifdef __cplusplus


class MPS_Variable {
  private :
	FDM_MPS_VAR_REF  *MPS;
  public :
  MPS_Variable ( AE_LE &Xdef ,  FDM_XAE &Def)
	{
	MPS = fdm_mps_var_create( Xdef.Ptr_AE,  &Def);
  	}

  MPS_Variable ( AE_LE *Xdef ,  FDM_XAE &Def)
	{
	MPS = fdm_mps_var_create( Xdef->Ptr_AE,  &Def);
  	}

/* modifieurs de config */
  inline Ushort Change_ID ( Ushort ID )
	{ return  fdm_mps_var_change_id ( MPS  , ID ); }

  inline Ushort Change_Period ( Ulong Refreshment , Ulong Promptness )
	{ return  fdm_mps_var_change_periods ( MPS  , Refreshment , Promptness ); }

  inline Ushort Change_Priority ( unsigned Priority )
	{ return  fdm_mps_var_change_priority ( MPS  , Priority ); }

  inline Ushort Change_Var_type
	(
	  enum _FDM_CHANGE_TYPE Image_1_Var_Type ,
	  enum _FDM_CHANGE_TYPE Image_2_Var_Type )
	{ return  fdm_mps_var_change_prod_cons (
			MPS  , Image_1_Var_Type ,Image_2_Var_Type ); }

  inline Ushort Change_RQa (  enum  FDM_FLAGS RQA )
	{ return  fdm_mps_var_change_RQa ( MPS  , RQA ); }

  inline Ushort Change_MSGa (  enum  FDM_FLAGS MSGA )
	{ return  fdm_mps_var_change_MSGa ( MPS  , MSGA ); }


/****/

  inline Ushort Local_Write (  USER_BUFFER_TO_READ  D)
	{ return  fdm_mps_var_write_loc ( MPS , D); }

  inline FDM_MPS_READ_STATUS Local_Read (  FDM_MPS_VAR_DATA &D)
	{ return  fdm_mps_var_read_loc ( MPS  , &D); }


  inline Ushort Universal_Write (  USER_BUFFER_TO_READ D )
	{ return  fdm_mps_var_write_universal ( MPS  ,	D ); }

  inline Ushort Universal_Read  (  )
	{ return  fdm_mps_var_read_universal ( MPS ); }

};

class MPS_Var_Prod {
  private :
	FDM_MPS_VAR_REF  *MPS;
  public :
  /* constructeur */
  MPS_Var_Prod ( AE_LE &Xdef ,  FDM_XAE &Def)
	{
	Def.Type.Communication = 1;
	MPS = fdm_mps_var_create( Xdef.Ptr_AE,  &Def);
  	}

  MPS_Var_Prod ( AE_LE *Xdef ,  FDM_XAE &Def)
	{
	Def.Type.Communication = 1;
	MPS = fdm_mps_var_create( Xdef->Ptr_AE,  &Def);
  	}

  inline Ushort Change_ID ( Ushort ID )
	{ return  fdm_mps_var_change_id ( MPS  , ID ); }

  inline Ushort Change_RQa (  enum  FDM_FLAGS RQA )
	{ return  fdm_mps_var_change_RQa ( MPS  , RQA ); }

  inline Ushort Change_MSGa (  enum  FDM_FLAGS MSGA )
	{ return  fdm_mps_var_change_MSGa ( MPS  , MSGA ); }

  inline Ushort Change_Priority ( unsigned Priority )
	{ return  fdm_mps_var_change_priority ( MPS  , Priority ); }

   inline Ushort Change_Period ( Ulong Refreshment  )
	{ return  fdm_mps_var_change_periods ( MPS  , Refreshment , 0 ); }


  inline Ushort Local_Write (  USER_BUFFER_TO_READ  D)
	{ return  fdm_mps_var_write_loc ( MPS , D); }

  inline Ushort Universal_Write (  USER_BUFFER_TO_READ D )
	{ return  fdm_mps_var_write_universal ( MPS  ,	D ); }


};


class MPS_Var_Cons {
  private :
	FDM_MPS_VAR_REF  *MPS;
  public :
  /* constructeur */
  MPS_Var_Cons ( AE_LE &Xdef ,  FDM_XAE &Def)
	{
	Def.Type.Communication = 2;
	MPS = fdm_mps_var_create( Xdef.Ptr_AE,  &Def);
  	}

  MPS_Var_Cons ( AE_LE *Xdef ,  FDM_XAE &Def)
	{
	Def.Type.Communication = 2;
	MPS = fdm_mps_var_create( Xdef->Ptr_AE,  &Def);
  	}
  inline Ushort Change_ID ( Ushort ID )
	{ return  fdm_mps_var_change_id ( MPS  , ID ); }


  inline Ushort Change_Priority ( unsigned Priority )
	{ return  fdm_mps_var_change_priority ( MPS  , Priority ); }


   inline Ushort Change_Period (  Ulong Promptness )
	{ return  fdm_mps_var_change_periods ( MPS  , 0 , Promptness ); }


  inline FDM_MPS_READ_STATUS Local_Read (  FDM_MPS_VAR_DATA &D)
	{ return  fdm_mps_var_read_loc ( MPS  , &D); }

  inline Ushort Universal_Read  (  )
	{ return  fdm_mps_var_read_universal ( MPS ); }



};

class MPS_Areceive  {
  private :
	  FDM_MPS_VAR_REF * MPS;
  public :
  /* constructeur */
  MPS_Areceive ( struct _FDM_MPS_VAR_REF *Var )
	{ MPS = (FDM_MPS_VAR_REF *)Var;}

   /*  AE_LE etat running  */
  inline FDM_MPS_READ_STATUS Local_Read (  FDM_MPS_VAR_DATA &D)
	{ return  fdm_mps_var_read_loc ( MPS  , &D); }


};

class MPS_Var_Sync  {
  private :
	  FDM_MPS_VAR_REF * MPS;
  public :
  /* constructeur */
  MPS_Var_Sync ( AE_LE &Xdef ,  FDM_XAE &Def)
	{
	Def.Type.Communication = 0;
	MPS = fdm_mps_var_create( Xdef.Ptr_AE,  &Def);
  	}

  MPS_Var_Sync ( AE_LE *Xdef ,  FDM_XAE &Def)
	{
	Def.Type.Communication = 0;
	MPS = fdm_mps_var_create( Xdef->Ptr_AE,  &Def);
  	}

   /*  AE_LE etat stopped */
  inline Ushort Change_ID ( Ushort ID )
	{ return  fdm_mps_var_change_id ( MPS  , ID ); }


};


class MPS_Var_Cons_and_Prod {
	  FDM_MPS_VAR_REF * MPS;
  public :
  /* constructeur */
  MPS_Var_Cons_and_Prod ( AE_LE &Xdef ,  FDM_XAE &Def)
	{
	Def.Type.Communication = 3;
	MPS = fdm_mps_var_create( Xdef.Ptr_AE,  &Def);
  	}

  MPS_Var_Cons_and_Prod ( AE_LE *Xdef ,  FDM_XAE &Def)
	{
	Def.Type.Communication = 3;
	MPS = fdm_mps_var_create( Xdef->Ptr_AE,  &Def);
  	}

   /*  AE_LE etat stopped */
  inline Ushort Change_ID ( Ushort ID )
	{ return  fdm_mps_var_change_id ( MPS  , ID ); }

  inline Ushort Change_RQa (  enum  FDM_FLAGS RQA )
	{ return  fdm_mps_var_change_RQa ( MPS  , RQA ); }

  inline Ushort Change_MSGa (  enum  FDM_FLAGS MSGA )
	{ return  fdm_mps_var_change_MSGa ( MPS  , MSGA ); }

   /*  AE_LE etat running  */
  inline Ushort Change_Priority ( unsigned Priority )
	{ return  fdm_mps_var_change_priority ( MPS  , Priority ); }

  inline Ushort Change_Period ( Ulong Refreshment , Ulong Promptness )
	{ return  fdm_mps_var_change_periods ( MPS  , Refreshment , Promptness ); }

  inline Ushort Change_Var_type
	(
	  enum _FDM_CHANGE_TYPE Image_1_Var_Type ,
	  enum _FDM_CHANGE_TYPE Image_2_Var_Type )
	{ return  fdm_mps_var_change_prod_cons (
			MPS  , Image_1_Var_Type ,Image_2_Var_Type ); }


  inline Ushort Local_Write (  USER_BUFFER_TO_READ  D)
	{ return  fdm_mps_var_write_loc ( MPS , D); }

  inline Ushort Universal_Write (  USER_BUFFER_TO_READ D )
	{ return  fdm_mps_var_write_universal ( MPS  ,	D ); }


  inline FDM_MPS_READ_STATUS Local_Read (  FDM_MPS_VAR_DATA &D)
	{ return  fdm_mps_var_read_loc ( MPS  , &D); }

  inline Ushort Universal_Read  (  )
	{ return  fdm_mps_var_read_universal ( MPS ); }


};


#endif



/*=======================================================*/
/*=======================================================*/
/*==================== MESSAGING SERVICES ===============*/
/*=======================================================*/
/*=======================================================*/

#ifdef __cplusplus

class MsgDataLinkFullduplex  {
   private :
	 FDM_MESSAGING_REF  * Ref_MDL;

   public :

        /*constructeurs */
    MsgDataLinkFullduplex (Fip_Device_Manager *Fip_DM,
     		FDM_MESSAGING_FULLDUPLEX &Cfg) :
       Ref_MDL(fdm_messaging_fullduplex_create(Fip_DM->Fip,&Cfg)){ }

     ~ MsgDataLinkFullduplex() {fdm_messaging_delete( Ref_MDL);}

    inline Ushort IS_Created() { return (Ushort)( ( Ref_MDL == NULL) ? 0 : 1 ); }

    inline Ushort SendMsg(FDM_MSG_TO_SEND* Message)
      			{return fdm_send_message(Ref_MDL , Message);}

   } ;

class  MsgDataLinkToSend  {
 private :
	 FDM_MESSAGING_REF  * Ref_MDL;

 public :

    MsgDataLinkToSend (Fip_Device_Manager *Fip_DM,
     		 FDM_MESSAGING_TO_SEND &Cfg) :
        Ref_MDL(fdm_messaging_to_send_create(Fip_DM->Fip,&Cfg)){}


     ~ MsgDataLinkToSend() {fdm_messaging_delete( Ref_MDL);}

    inline Ushort IS_Created() { return (Ushort)( ( Ref_MDL == NULL) ? 0 : 1 ); }

    inline Ushort SendMsg(FDM_MSG_TO_SEND* Message)
      			{return fdm_send_message(Ref_MDL , Message);}

   } ;

class  MsgDataLinkToReceive  {
   private :
	 FDM_MESSAGING_REF  * Ref_MDL;

   public :

     MsgDataLinkToReceive (Fip_Device_Manager *Fip_DM,
     		FDM_MESSAGING_TO_REC &Cfg) :
        Ref_MDL(fdm_messaging_to_rec_create(Fip_DM->Fip,&Cfg)){}


    ~ MsgDataLinkToReceive() {fdm_messaging_delete( Ref_MDL);}

    inline Ushort IS_Created() { return (Ushort)( ( Ref_MDL == NULL) ? 0 : 1 ); }

   } ;

#endif
#endif
