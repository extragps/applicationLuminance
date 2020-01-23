

#ifndef __MAN_OLGA_H
#define __MAN_OLGA_H

#include <fdm.h>

#define __Port_Type__ unsigned


/*-----------------------------------------------------------------------------*/
/* Logical Node functions                                                      */
/*-----------------------------------------------------------------------------*/

extern void LN_LN1 (void);

/*-----------------------------------------------------------------------------*/
/* Function Bloc functions                                                     */
/*-----------------------------------------------------------------------------*/

/*------- LN1 */

extern void LN1_FB0 (void);

/*-----------------------------------------------------------------------------*/
/* User functions                                                              */
/*-----------------------------------------------------------------------------*/

extern void User_Present_List_Prog (struct _FDM_REF *, FDM_PRESENT_LIST *);
extern unsigned short User_Identification_Prog (struct _FDM_REF *, FDM_IDENT_VAR *);
extern unsigned short User_Report_Prog (struct _FDM_REF *, FDM_REPORT_VAR *);
extern unsigned short User_Presence_Prog (struct _FDM_REF *, FDM_PRESENCE_VAR *);
extern void User_Synchro_BA_Prog (struct _FDM_REF *, FDM_SYNCHRO_BA_VAR *);
extern void User_Signal_Mps_Aper (struct _FDM_REF *);
extern void User_Signal_Smmps (struct _FDM_REF *);
extern void User_Signal_Send_Msg (struct _FDM_REF *);
extern void User_Signal_Rec_Msg (struct _FDM_REF *);
extern void User_Reset_Component (struct _FDM_CONFIGURATION_HARD *);
extern void User_Signal_Fatal_Error (struct _FDM_REF *, FDM_ERROR_CODE);
extern void User_Signal_Warning (struct _FDM_REF *, FDM_ERROR_CODE);

/*-----------------------------------------------------------------------------*/
/* Variable user functions                                                     */
/*-----------------------------------------------------------------------------*/

extern void User_Signal_Areceived_VarCom_LN1_0x0001 (struct _FDM_MPS_VAR_REF *);

extern void User_Signal_Areceived_VarCom_LN1_0x0002 (struct _FDM_MPS_VAR_REF *);

extern void User_Signal_Asent_VarCom_LN1_0x3000 (struct _FDM_MPS_VAR_REF *);

extern void User_Signal_Asent_VarCom_LN1_0x3001 (struct _FDM_MPS_VAR_REF *);

extern void User_Signal_Asent_VarCom_LN1_0x3002 (struct _FDM_MPS_VAR_REF *);

extern void User_Signal_Var_Prod_VarCom_LN1_0x4000 (struct _FDM_MPS_VAR_REF *, unsigned short);

extern void User_Signal_Var_Prod_VarCom_LN1_0x4100 (struct _FDM_MPS_VAR_REF *, unsigned short);

extern void User_Signal_Areceived_VarCom_LN1_0x4101 (struct _FDM_MPS_VAR_REF *);

extern void User_Signal_Areceived_VarCom_LN1_0x4102 (struct _FDM_MPS_VAR_REF *);

extern void User_Signal_Asent_VarCom_LN1_0x4200 (struct _FDM_MPS_VAR_REF *);

extern void User_Signal_Var_Cons_VarCom_LN1_0x4201 (struct _FDM_MPS_VAR_REF *, unsigned short, FDM_MPS_VAR_DATA *);

extern void User_Signal_Var_Cons_VarCom_LN1_0x4202 (struct _FDM_MPS_VAR_REF *, unsigned short, FDM_MPS_VAR_DATA *);

/*-----------------------------------------------------------------------------*/
/* Messaging Context user functions                                            */
/*-----------------------------------------------------------------------------*/

extern void User_Msg_Ack_Proc_MSGCTX_FD_MsgFDaperAgt1Ctxt (FDM_MESSAGING_REF *, FDM_MSG_TO_SEND *);
extern void User_Msg_Rec_Proc_MSGCTX_FD_MsgFDaperAgt1Ctxt (FDM_MESSAGING_REF *, FDM_MSG_RECEIVED *);
extern void User_Msg_Ack_Proc_MSGCTX_FD_MsgFDaperAgt2Ctxt (FDM_MESSAGING_REF *, FDM_MSG_TO_SEND *);
extern void User_Msg_Rec_Proc_MSGCTX_FD_MsgFDaperAgt2Ctxt (FDM_MESSAGING_REF *, FDM_MSG_RECEIVED *);
extern void User_Msg_Rec_Proc_MSGCTX_R_MsgRECperAgt1Ctxt (FDM_MESSAGING_REF *, FDM_MSG_RECEIVED *);
extern void User_Msg_Rec_Proc_MSGCTX_R_MsgRECperAgt2Ctxt (FDM_MESSAGING_REF *, FDM_MSG_RECEIVED *);
extern void User_Msg_Ack_Proc_MSGCTX_S_MsgSENDperCtxt (FDM_MESSAGING_REF *, FDM_MSG_TO_SEND *);







extern FDM_REF * usr_fip_ref;

extern const FDM_CONFIGURATION_SOFT VS_UserSoftDefinition;

extern FDM_CONFIGURATION_HARD VS_UserHardDefinition;

extern const FDM_IDENTIFICATION VS_UserIdentParam;




extern FDM_BA_REF	*VS_Ba;   
extern PTR_LISTS Listes_BA[];
extern const PTR_INSTRUCTIONS Prg_BA_A[];

extern unsigned short nb_of_lists_CC121_A_BA_Macrocycle ;
extern unsigned short nb_of_instructions_CC121_A_BA_program ;

#endif /* MANAGER_H */
