/*******************************************************************************/
/* MAN_FDMI.H                                                                  */
/*-----------------------------------------------------------------------------*/
/* Creation date : Friday, April 10, 1998 - 11:49:48                           */
/*******************************************************************************/

#ifndef ___INCLUDE_MAN_FDMI__
#define ___INCLUDE_MAN_FDMI__

/*-----------------------------------------------------------------------------*/
/* AELE structure                                                              */
/*-----------------------------------------------------------------------------*/

typedef struct
    {
    int                         NrOfVar;
    enum _FDM_CHANGE_ALLOWED    Type;  /* CHANGE_ALLOWED or CHANGE_NOT_ALLOWED */
    FDM_AE_LE_REF *             AELEref;
    } TS_InfoAELE;

/*-----------------------------------------------------------------------------*/
/* AELE reference                                                              */
/*-----------------------------------------------------------------------------*/

extern TS_InfoAELE AELE_LN1;

/*-----------------------------------------------------------------------------*/
/* Communication Variable structure                                            */
/*-----------------------------------------------------------------------------*/

typedef struct
    {
    FDM_XAE                     VarDesc;
    FDM_MPS_VAR_REF *           VarRef;
    } TS_InfoVar;

/*-----------------------------------------------------------------------------*/
/* Communication Variable reference (variables)                                */
/*-----------------------------------------------------------------------------*/

extern TS_InfoVar VarCom_LN1_0x0001;
extern TS_InfoVar VarCom_LN1_0x0002;
extern TS_InfoVar VarCom_LN1_0x3000;
extern TS_InfoVar VarCom_LN1_0x3001;
extern TS_InfoVar VarCom_LN1_0x3002;
extern TS_InfoVar VarCom_LN1_0x4000;
extern TS_InfoVar VarCom_LN1_0x4100;
extern TS_InfoVar VarCom_LN1_0x4101;
extern TS_InfoVar VarCom_LN1_0x4102;
extern TS_InfoVar VarCom_LN1_0x4200;
extern TS_InfoVar VarCom_LN1_0x4201;
extern TS_InfoVar VarCom_LN1_0x4202;

/*-----------------------------------------------------------------------------*/
/* Communication Variable reference (ranks)                                    */
/*-----------------------------------------------------------------------------*/

#define RANK_VarCom_LN1_0x0001                         0
#define RANK_VarCom_LN1_0x0002                         1
#define RANK_VarCom_LN1_0x3000                         2
#define RANK_VarCom_LN1_0x3001                         3
#define RANK_VarCom_LN1_0x3002                         4
#define RANK_VarCom_LN1_0x4000                         5
#define RANK_VarCom_LN1_0x4100                         6
#define RANK_VarCom_LN1_0x4101                         7
#define RANK_VarCom_LN1_0x4102                         8
#define RANK_VarCom_LN1_0x4200                         9
#define RANK_VarCom_LN1_0x4201                         10
#define RANK_VarCom_LN1_0x4202                         11

/*-----------------------------------------------------------------------------*/
/* Messaging Context Type constants                                            */
/*-----------------------------------------------------------------------------*/

#define TYPE_MSG_FULLDUP 0
#define TYPE_MSG_TO_SEND 1
#define TYPE_MSG_TO_REC  2

/*-----------------------------------------------------------------------------*/
/* Messaging Context structure                                                 */
/*-----------------------------------------------------------------------------*/

typedef struct
    {
    int                         Type;
    FDM_MESSAGING_FULLDUPLEX    Msg;
    FDM_MESSAGING_REF *         MsgRef;
    } TS_InfoMsgCtxFD; /* FullDuplex */

typedef struct
    {
    int                         Type;
    FDM_MESSAGING_TO_SEND       Msg;
    FDM_MESSAGING_REF *         MsgRef;
    } TS_InfoMsgCtxS; /* To Send */

typedef struct
    {
    int                         Type;
    FDM_MESSAGING_TO_REC        Msg;
    FDM_MESSAGING_REF *         MsgRef;
    } TS_InfoMsgCtxR; /* To Receive */

/*-----------------------------------------------------------------------------*/
/* Messaging Context reference                                                 */
/*-----------------------------------------------------------------------------*/

extern TS_InfoMsgCtxFD MSGCTX_FD_MsgFDaperAgt1Ctxt;
extern TS_InfoMsgCtxFD MSGCTX_FD_MsgFDaperAgt2Ctxt;
extern TS_InfoMsgCtxR MSGCTX_R_MsgRECperAgt1Ctxt;
extern TS_InfoMsgCtxR MSGCTX_R_MsgRECperAgt2Ctxt;
extern TS_InfoMsgCtxS MSGCTX_S_MsgSENDperCtxt;

/*-----------------------------------------------------------------------------*/
/* Channel structure                                                           */
/*-----------------------------------------------------------------------------*/

typedef struct
    {
    FDM_CHANNEL_PARAM           Param;
    unsigned short              Status;
    } TS_InfoChannel;

/*-----------------------------------------------------------------------------*/
/* Channel reference                                                           */
/*-----------------------------------------------------------------------------*/

extern TS_InfoChannel CHANNEL_CHANNEL1;

/*-----------------------------------------------------------------------------*/
/* F_Start & F_Stop functions return values                                    */
/*-----------------------------------------------------------------------------*/

typedef enum
    {
    C_NoError = 0,
    C_ErrorCreateContextFdmNT,
    C_ErrorInstanceCreation,
    C_ErrorValidMedium,
    C_ErrorBAMacroCycleNotLoaded,
    C_ErrorBANotStarted,
    C_ErrorAELECreation,
    C_ErrorMPSVariableCreation,
    C_ErrorAELEStart,
    C_ErrorMessagingContextCreation,
    C_ErrorPeriodicChannelCreation,
    C_ErrorGenericTimeInitialisation,
    } TE_ErrorCode;

/*-----------------------------------------------------------------------------*/
/* F_Start & F_Stop functions                                                  */
/*-----------------------------------------------------------------------------*/

unsigned short F_Start (void);
unsigned short F_Stop (unsigned short);


/*-----------------------------------------------------------------------------*/
/* Global variables                                                            */
/*-----------------------------------------------------------------------------*/
#endif /* ___INCLUDE_MAN_FDMI__ */
