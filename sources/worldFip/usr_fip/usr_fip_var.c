#include "fdm.h"
#include "usr_fip_var.h"
#include "man_olga.h"
#include "man_fdmi.h"

T_varComm_rapport usr_varComm_rapport;
extern TS_InfoVar usr_var_dummy ;
int dummyPresente=0;
int rapportCompteRendu=0;
int rapportEmis=0;
int usr_fip_date_recue=0;

void usr_varComm_rapport_init(T_varComm_rapport *rapport)
{
	rapport->nbMsgLongEmisOk=0;
	rapport->nbMsgLongEmisNok=0;
	rapport->nbMsgCourtEmisOk=0;
	rapport->nbMsgCourtEmisNok=0;
	rapport->nbMsgLongRecOk=0;
	rapport->nbMsgLongRecNok=0;
	rapport->nbMsgCourtRecOk=0;
	rapport->nbMsgCourtRecNok=0;
	rapport->nbTransactionOk=0;
	rapport->nbTransactionNok=0;
	rapport->rafMPS=0;
}

void User_Signal_Asent_usr_var_rapport (struct _FDM_MPS_VAR_REF * ref)
{
	rapportCompteRendu=ref->Compte_rendu;
	rapportEmis=1;
}

void User_Signal_Var_Prod_usr_var_rapport (struct _FDM_MPS_VAR_REF * ref,unsigned short code)
{
	/* printf("Variable rapport produite code %d\n",code); */
}

void User_Signal_Var_Cons_usr_var_rapport (struct _FDM_MPS_VAR_REF * ref,
		FDM_MPS_READ_STATUS code, FDM_MPS_VAR_DATA *data)
{
	printf("Variable rapport consommee\n");
}

void User_Signal_Areceived_usr_var_dateEtHeure (struct _FDM_MPS_VAR_REF *ref)
{
	/* printf("La variable date et heure a ete reeue\n"); */
}

void User_Signal_Areceived_usr_var_date (struct _FDM_MPS_VAR_REF *ref)
{
	/* printf("La variable date et heure a ete reeue\n"); */
	usr_fip_date_recue=1;
}

void User_Signal_Areceived_usr_var_dummy (struct _FDM_MPS_VAR_REF *ref)
{
	/*	printf("J'ai recu la variable dummy\n"); */
}
