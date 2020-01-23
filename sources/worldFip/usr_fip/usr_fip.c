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

/* ******************************** */
/* LISTE DES INCLUDES               */
/* ******************************** */

#include "fdmtime.h"
#include "usr_fip.h"
#include "time.h"
#ifdef VXWORKS
#include "timers.h"
#endif
#ifdef LINUX
#include <stdio.h>
#endif

/* ******************************** */
/* VARIABLES GLOBALES				*/
/* ******************************** */

int network_MPS_Aperiodic_event=0;
int network_smmps_event=0;
int network_received_MSG_event=0;
int network_transmitted_MSG_event=0;
static int usr_fip_pipe=-1;

static int	usr_fip_last_def=0;
static int	usr_fip_last_ustate=0;
static int	usr_fip_last_varstate=0;
static int	usr_fip_add_report=0;
static int  usr_fip_cpt_warning=0;

/* ******************************** */
/* DEFINITION DES FONCTIONS         */
/* ******************************** */

/* --------------------------------
 * usr_fip_get_cpt_warning
 * =======================
 * Recuperation du nombre de warning
 * rencontres.
 * --------------------------------	*/

int usr_fip_get_cpt_warning(void)
{
	return usr_fip_cpt_warning;
}

/* --------------------------------
 * usr_fip_reset_cpt_warning
 * =========================
 * Remise e 0 du compteur de warning
 * --------------------------------	*/

void usr_fip_reset_cpt_warning(void)
{
	usr_fip_cpt_warning=0;
}

/* --------------------------------
 * usr_fip_incr_cpt_warning
 * ========================
 * Lorsque survient une erreur.
 * --------------------------------	*/

static void usr_fip_incr_cpt_warning(void)
{
	usr_fip_cpt_warning++;
}

/* --------------------------------
 * usr_fip_set_fd
 * ================
 * Positionnement du canal utilise
 * pour la reception et l'emission
 * des messages echanges sur worldfip
 * --------------------------------	*/

void usr_fip_set_fd(int fd)
{
	usr_fip_pipe=fd;
}

/* ---------------------------------
 * usr_fip_get_fd
 * ==============
 * Recuperation du fd utilise pour
 * la reception et l'emission des
 * messages echanges sur worldfip.
 * --------------------------------	*/

int usr_fip_get_fd(void)
{
	return(usr_fip_pipe);
}

/* -------------------------------- */
/* User_Reset_Component             */
/* ====================             */
/* Pour effectuer une reinitialisa- */
/* tion du hard. La fonction est    */
/* appelee par fdm_stop_network.    */
/* -------------------------------- */

void User_Reset_Component         (struct _FDM_CONFIGURATION_HARD *config)
{
}

/* -------------------------------- */
/* User_Signal_Fatal_Error          */
/* ====================             */
/* Traitement d'une erreur hard     */
/* Dans ce cas, on force une sortie */
/* du programme. 					*/
/* -------------------------------- */

void User_Signal_Fatal_Error(struct _FDM_REF *ref,FDM_ERROR_CODE code)
{
char buffer[100];
	sprintf(buffer,"WORLD FIP : Erreur fatale %04X add %d",code.Fdm_Default,
		code.Information.Additional_Report);
	cmd_trc_tr(buffer);
	printf("Fatal error : %04X\n",code.Fdm_Default);
	printf("%s\n",buffer);
	ksleep(1000);
	exit(0);
}

void User_Signal_Warning(struct _FDM_REF *ref,FDM_ERROR_CODE code)
{

	usr_fip_incr_cpt_warning();
	printf("Warning     : %04X\n",(unsigned int)code.Fdm_Default);
	if(code.Fdm_Default==0x100)
	{
		printf("	Ustate     : %X\n",code.Information.Fipcode_Report._Ustate);
		printf("	Var_State  : %X\n",
			code.Information.Fipcode_Report._Var_State);
		if(
			(code.Fdm_Default!=usr_fip_last_def)||
			(code.Information.Fipcode_Report._Ustate!=usr_fip_last_ustate)||
			(code.Information.Fipcode_Report._Var_State!=usr_fip_last_varstate))
		{
		char buffer[100];
			sprintf(buffer,"WORLD FIP : Attention %04X etatU %X etatV %X",
				code.Fdm_Default,
				code.Information.Fipcode_Report._Ustate,
				code.Information.Fipcode_Report._Var_State);
			cmd_trc_tr(buffer);
			usr_fip_last_def=code.Fdm_Default;
			usr_fip_last_ustate=code.Information.Fipcode_Report._Ustate;
			usr_fip_last_varstate=code.Information.Fipcode_Report._Var_State;
		}

	}
	else
	{
		printf("	Additionnal: %04X\n",(unsigned int)code.Information.Additional_Report);
		if(
			(code.Fdm_Default!=usr_fip_last_def)||
			(code.Information.Additional_Report!=usr_fip_add_report))
		{
		char buffer[100];
			sprintf(buffer,"WORLD FIP : Attention %04X plus %X",
				code.Fdm_Default,(unsigned int)code.Information.Additional_Report);
			cmd_trc_tr(buffer);
			usr_fip_last_def=code.Fdm_Default;
			usr_fip_add_report=code.Information.Additional_Report;
		}
	}
}

void User_Present_List_Prog(struct _FDM_REF *ref,FDM_PRESENT_LIST *presents)
{
}
Ushort User_Presence_Prog(struct _FDM_REF *ref,FDM_PRESENCE_VAR *variables)
{
Ushort retour=0;
	return retour;
}

Ushort User_Identification_Prog(struct _FDM_REF *ref,FDM_IDENT_VAR *ident)
{
Ushort retour=0;
	return retour;
}

Ushort User_Report_Prog(struct _FDM_REF *ref,FDM_REPORT_VAR *var)
{
Ushort retour=0;
	/* Decodage de la variable report	*/
	if(VAR_TRANSFERT_OK==var->Report)
	{
		printf("Equip : %d nbOk1 : %d nbMess1 : %d Status %0x\n",
			var->Subscriber,
			var->Nb_Of_Transaction_Ok_1,
			var->Nb_Of_Frames_Nok_1,
			var->Activity_Status);
	}
	else
	{
		printf("No variable transfert\n");
	}
	return retour;
}

void User_Synchro_BA_Prog(struct _FDM_REF *ref, FDM_SYNCHRO_BA_VAR *var)
{
}

void User_Signal_Mps_Aper(struct _FDM_REF *ref)
{
	network_MPS_Aperiodic_event++;
}

void User_Signal_Smmps(struct _FDM_REF *ref)
{
	network_smmps_event++;
}

void User_Signal_Rec_Msg(struct _FDM_REF *ref)
{
	network_received_MSG_event ++;
}

void User_Signal_Send_Msg(struct _FDM_REF *ref)
{
	network_transmitted_MSG_event ++;
}

void User_Signal_Synchro(struct _FDM_REF *ref)
{
}

void User_Signal_Var_Prod(struct _FDM_MPS_VAR_REF *var_ref,Ushort var)
{
}

void User_Signal_Var_Cons(struct _FDM_MPS_VAR_REF *var_ref,FDM_MPS_READ_STATUS status,FDM_MPS_VAR_DATA *donnee)
{
}

void User_Signal_Asent(struct _FDM_MPS_VAR_REF *var_ref)
{
}

void User_Signal_Areceived(struct _FDM_MPS_VAR_REF *var_ref)
{
}

int User_Signal_Mode(const int mode)
{
int retour=0;
	return retour;
}

Uchar *User_Get_Value(void)
{
Uchar *retour=NULL;
	return retour;
}

void User_Set_Value(FDM_MPS_VAR_TIME_DATA *donnee)
{
}

void User_Msg_Rec_Proc(FDM_MESSAGING_REF *msg_ref,FDM_MSG_TO_SEND *msg_sent)
{
}

void User_Msg_Ack_Proc(FDM_MESSAGING_REF *msg_ref,FDM_MSG_RECEIVED *msg_recv)
{
}

void User_Msg_Ack_Type(struct _FDM_REF *ref, unsigned long Remote_Adr)
{
}

/* --------------------------------
 * usr_fip_process
 * ===============
 * Traitement des evenements FIP.
 * --------------------------------	*/

int usr_fip_process(void)
{
int retour=0;

	if (network_smmps_event > 0)
  	{
    	fdm_smmps_fifo_empty( usr_fip_ref );
    	network_smmps_event = 0;
		retour=1;
	}
  	if (network_MPS_Aperiodic_event > 0)
  	{
    	fdm_mps_fifo_empty( usr_fip_ref );
    	network_MPS_Aperiodic_event = 0;
		retour=1;
	}
	if (network_received_MSG_event > 0)
	{
    	fdm_msg_rec_fifo_empty( usr_fip_ref );
    	network_received_MSG_event=0;
		retour=1;
	}
	if (network_transmitted_MSG_event > 0)
	{
    	fdm_msg_send_fifo_empty( usr_fip_ref );
    	network_transmitted_MSG_event=0;
		retour=1;
	}
										/* --------------------------------
										 * FIN DE usr_fip_process
										 * --------------------------------	*/
	return retour;
}

