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

#ifndef __INCusr_fip_varh
#define __INCusr_fip_varh

void User_Signal_Asent_usr_var_rapport (struct _FDM_MPS_VAR_REF * );
void User_Signal_Var_Prod_usr_var_rapport (struct _FDM_MPS_VAR_REF *,unsigned short);
void User_Signal_Var_Cons_usr_var_rapport (struct _FDM_MPS_VAR_REF * ,
		FDM_MPS_READ_STATUS,FDM_MPS_VAR_DATA *);
void User_Signal_Areceived_usr_var_dateEtHeure (struct _FDM_MPS_VAR_REF * );
void User_Signal_Areceived_usr_var_dummy (struct _FDM_MPS_VAR_REF * );
void User_Signal_Areceived_usr_var_date (struct _FDM_MPS_VAR_REF * );

/* ********************************	*/
/* DEFINITION DES TYPES				*/
/* ********************************	*/

/* --------------------------------	*/
/* T_VarComm_rapport				*/
/* =================				*/
/* Structure utilisee pour la trans-*/
/* mission de la variable de 		*/
/* rapport.							*/
/* --------------------------------	*/

typedef struct T_varComm_rapport
{
	unsigned long	nbMsgLongEmisOk;
	unsigned long	nbMsgLongEmisNok;
	unsigned long	nbMsgCourtEmisOk;
	unsigned long	nbMsgCourtEmisNok;
	unsigned long	nbMsgLongRecOk;
	unsigned long	nbMsgLongRecNok;
	unsigned long	nbMsgCourtRecOk;
	unsigned long	nbMsgCourtRecNok;
	unsigned long	nbTransactionOk;
	unsigned long	nbTransactionNok;
	unsigned long   reserved_V24[8];
	unsigned char 	rafMPS;
}	T_varComm_rapport;

/* --------------------------------	*/
/* T_VarComm_dateEtHeure			*/
/* =====================			*/
/* --------------------------------	*/

typedef struct T_VarComm_dateEtHeure
{
	unsigned char 	longueur;
	unsigned char 	heurePosix;
	unsigned char 	longueurPosix;
	unsigned char 	signification;
	unsigned long 	dateSec;
	unsigned long 	dateUSec;
	unsigned long 	rafDyn;
	unsigned char 	rafMPS;
}	T_VarComm_dateEtHeure;

extern T_varComm_rapport usr_varComm_rapport;
extern int usr_fip_date_recue;

void usr_varComm_rapport_init(T_varComm_rapport *rapport);


#endif
