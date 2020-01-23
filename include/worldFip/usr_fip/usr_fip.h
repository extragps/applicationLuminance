/***************************************************************************/
/*       FICHIER: %M%      REL: %I% DATE: %G%                              */
/*DOC***********************************************************************/
/*                                       SIAT                              */
/*                     8, parc technologique de la Sainte Victoire         */
/*                                    Le Cannet                            */
/*                              13590 Meyreuil - FRANCE                    */
/*                                   Tel. 04 42 58 63 71                   */
/***************************************************************************/
/*  Fichier : usr_fip.c                                                    */  
/*  MODULE  : usr_fip                                                      */
/***************************************************************************/
/* Auteur :   Xavier GAILLARD                                              */
/* Date de creation : O6 octobre 2000                                      */
/***************************************************************************/
/* OBJET DU MODULE      :                                                  */
/*		Inplementation des fonctions utilisateur de WorldFip			   */
/*                                                                         */
/***********************************************************************DOC*/
/* Liste des fonctions du fichier :                                        */
/***************************************************************************/
/*                           Modifications                                 */
/*                      ***********************                            */
/* Auteur :                                                                */
/* Date de la modification :                                               */
/* Description :                                                           */
/*                      ***********************                            */
/***************************************************************************/

#ifndef __INCfip_usrh
#define __INCfip_usrh

/* ******************************** */
/* DEFINITION DES CONSTANTES		*/
/* ******************************** */

#define USR_FIP_MAX_MESSAGE	5

/* ******************************** */
/* DEFINITION DES TYPES				*/
/* ******************************** */

extern FDM_REF *usr_fip_ref;
extern int	network_MPS_Aperiodic_event;
extern int	network_smmps_event;
extern int	network_received_MSG_event;
extern int	network_transmitted_MSG_event;

/* ******************************** */
/* PROTOTYPE DES FONCTIONS          */
/* ******************************** */

										/* Fonctions specifiques FIP		*/
void User_Reset_Component         (struct _FDM_CONFIGURATION_HARD *config);
void User_Signal_Fatal_Error(struct _FDM_REF *ref,FDM_ERROR_CODE code);
void User_Signal_Warning(struct _FDM_REF *ref,FDM_ERROR_CODE code);
void User_Present_List_Prog(struct _FDM_REF *ref,FDM_PRESENT_LIST *presents) ;
Ushort User_Presence_Prog(struct _FDM_REF *ref,FDM_PRESENCE_VAR *variables);
Ushort User_Identification_Prog(struct _FDM_REF *ref,FDM_IDENT_VAR *) ;
Ushort User_Report_Prog(struct _FDM_REF *ref,FDM_REPORT_VAR *variables);
void User_Synchro_BA_Prog(struct _FDM_REF *ref, FDM_SYNCHRO_BA_VAR *);
void User_Signal_Mps_Aper(struct _FDM_REF *ref);
void User_Signal_Smmps(struct _FDM_REF *ref);
void User_Signal_Rec_Msg(struct _FDM_REF *ref);
void User_Signal_Send_Msg(struct _FDM_REF *ref);
void User_Signal_Synchro(struct _FDM_REF *ref);
void User_Signal_Var_Prod(struct _FDM_MPS_VAR_REF *var_ref,Ushort);
void User_Signal_Var_Cons(struct _FDM_MPS_VAR_REF *var_ref,FDM_MPS_READ_STATUS status,FDM_MPS_VAR_DATA *donnee);
void User_Signal_Asent(struct _FDM_MPS_VAR_REF *var_ref);
void User_Signal_Areceived(struct _FDM_MPS_VAR_REF *var_ref);
int User_Signal_Mode(const int mode);
Uchar *User_Get_Value(void);
void User_Set_Value(FDM_MPS_VAR_TIME_DATA *donnee);
void User_Msg_Rec_Proc(FDM_MESSAGING_REF *msg_ref,FDM_MSG_TO_SEND *msg_sent);
void User_Msg_Ack_Proc(FDM_MESSAGING_REF *msg_ref,FDM_MSG_RECEIVED *msg_recv);
void User_Msg_Ack_Type(struct _FDM_REF *ref,unsigned long Remote_Adr);
void fdm_generic_time_give_value(FDM_GENERIC_TIME_VALUE *);
FDM_GENERIC_TIME_VALUE fdm_generic_time_get_value(void);
										/* Mes fonctions du module.			*/
unsigned short usr_fip_network_stop (unsigned short ErrorCode);
unsigned short usr_fip_network_start (int);
unsigned long usr_fip_lire_adresse(void);
int usr_fip_process(void);	
void usr_fip_periodic(void);
void usr_fip_set_fd(int);
int  usr_fip_get_fd(void);
int usr_fip_get_cpt_warning(void);
void usr_fip_reset_cpt_warning(void);

#endif
