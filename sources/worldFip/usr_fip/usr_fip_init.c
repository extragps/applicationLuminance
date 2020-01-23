/****************************************************************************/
/* MAN_FDMI.C                                                               */
/*--------------------------------------------------------------------------*/
/* Creation date : Friday, April 10, 1998 - 11:49:48                        */
/****************************************************************************/

#ifdef LINUX
#include "portage.h"
#endif
#include "config.h"
#include "fdmtime.h"
#include "man_fdmi.h"
#include "man_olga.h"
#include "usr_fip.h"
#include "usr_fip_var.h"
#include "usr_fip_msg.h"
#include "usr_fip_init.h"

/* Numero d'equipement definit comme constante pour l'heure et e recuperer dans
 * un registre e terme.
 */
/* ********************************	*/
/* DECLARATIONS VARIABLES GLOBALES	*/
/* ********************************	*/

FDM_REF * usr_fip_ref;					/* Reference WorldFip				*/


/*--------------------------------------------------------------------------*/
/* FDM_CONFIGURATION_SOFT variable                                          */
/*--------------------------------------------------------------------------*/

static FDM_CONFIGURATION_SOFT usr_config_soft =
{
	/*TEST_RAM_STARTUP| */
	MESSAGE_RECEPTION_AUTHORIZED,		/* Type								*/
	WORLD_FIP_31,
	400,								/* Vitesse							*/
	2, 									/* Nombre de variables MPS			*/
	0x1800,								/* Dimension arbitre de bus ?		*/
	0x080000,							/* Taille de la ram FullFip			*/
	0x40,								/* Nombre de Id Prog BA?			*/
	1,									/* Nombre de repetitions.			*/
	{31,31,0,0,0,0,0,0,0},				/* Nombre de buffers de transmission*/
	User_Present_List_Prog,
	User_Identification_Prog,
	User_Report_Prog,
	User_Presence_Prog,
	User_Synchro_BA_Prog,
	1000,								/* Test medium ticks				*/
	1000,								/* Time out ticks.					*/
	1000,								/* Time out msg ticks.				*/
	0,									/* Online tests ticks.				*/
	99, 								/* Default threshold.				*/
	{0,0},								/* User_Responsability				*/
	User_Signal_Mps_Aper,
	User_Signal_Smmps,
	User_Signal_Send_Msg,
	User_Signal_Rec_Msg,
	NULL};

static FDM_CONFIGURATION_HARD usr_config_hard=
{
	0,									/* adresse du souscripteur			*/
	0,									/* Numero de segment utilise par la */
	{0,0},								/* Reserved							*/
	{(unsigned char volatile *)FIP_BASE,
	 (unsigned char volatile *)FIP_BASE+1,
	 (unsigned char volatile *)FIP_BASE+2,
	 (unsigned char volatile *)FIP_BASE+3,
	 (unsigned char volatile *)FIP_BASE+4,
	 (unsigned char volatile *)FIP_BASE+5,
	 (unsigned char volatile *)FIP_BASE+6,
	 (unsigned char volatile *)FIP_BASE+7},
	{NULL,
	 NULL,
	 NULL,
	 NULL},
	 NULL,
	 User_Reset_Component,
	 User_Signal_Fatal_Error,
	 User_Signal_Warning,
	 NULL,
	 NULL
};
										/* messagerie.						*/
static char usr_mps_conform=0x10;

static const FDM_IDENTIFICATION usr_ident=
{
	"Siat",								/* Nom de l'equipementier			*/
	"PMV01",							/* Nom du modele.					*/
	"0",
	NULL,								/* Tag name							*/
	&usr_mps_conform,					/* Conformite MPS					*/
	NULL,								/* Conformite SMS					*/
	NULL,								/* Conformite PMDM					*/
	NULL								/* Champ reserve a l'equipementier	*/
};

/*--------------------------------------------------------------------------*/
/* AELE variable : usr_table_aele                                           */
/* and its communication variables                                          */
/*--------------------------------------------------------------------------*/

TS_InfoAELE usr_table_aele =
    {
    /* NrOfVar          */ 5,
    /* Type             */ CHANGE_NOT_ALLOWED,
    /* AELEref          */ NULL
    };


/* --------------------------------	*/
/* usr_var_date						*/
/* ===================				*/
/* Variable de date.				*/
/* --------------------------------	*/

TS_InfoVar usr_var_date =
    {
    	{
										/* --------------------------------	*/
    									/* Type               				*/
										/* --------------------------------	*/
			{
				/* Reserved      */ 0,
                /* Position      */ 1,
                /* Communication */ VAR_CONSUMED,
                /* Scope         */ 1,
                /* With_Time_Var */ 1,
                /* Refreshment   */ 1,
                /* Indication    */ 1,
                /* Priority      */ 0,
                /* RQa           */ 0,
                /* MSGa          */ 0
			},
    		/* ID                 */ 0x9802,
    		/* Var_Length         */ /* 0x11, */ 12,
    		/* Refreshment_Period */ 1000000 /* es */,
    		/* Promptness_Period  */ 150000 /* es */,
    		/* Rank               */ 2,
    		/* Signals            */
			{
				NULL,
                NULL,
                User_Signal_Areceived_usr_var_date,
                NULL,
                NULL
		 	},
	 		/* User_Get_Value     */
    	} /* VarDesc */,
    	/* VarRef             */ NULL
    };

/* --------------------------------	*/
/* usr_var_dummy					*/
/* ===================				*/
/* Variable de date.				*/
/* --------------------------------	*/

TS_InfoVar usr_var_dummy =
    {
    	{
										/* --------------------------------	*/
    									/* Type               				*/
										/* --------------------------------	*/
			{
				/* Reserved      */ 0,
                /* Position      */ 1,
                /* Communication */ VAR_CONSUMED,
                /* Scope         */ 1,
                /* With_Time_Var */ 0,
                /* Refreshment   */ 1,
                /* Indication    */ 1,
                /* Priority      */ 0,
                /* RQa           */ 1,
                /* MSGa          */ 1
			},
    		/* ID                 */ 0x0501,
    		/* Var_Length         */ 8,
    		/* Refreshment_Period */ 0 /* es */,
    		/* Promptness_Period  */ 150000 /* es */,
    		/* Rank               */ 0,
    		/* Signals            */
			{
				NULL,
                NULL,
                User_Signal_Areceived_usr_var_dummy,
                NULL,
                NULL
		 	},
	 		/* User_Get_Value     */
    	} /* VarDesc */,
    	/* VarRef             */ NULL
    };

/* --------------------------------	*/
/* usr_var_rapport   				*/
/* ===============    				*/
/* Variable de rapport mise a jour	*/
/* periodiquement par l'applicatif.	*/
/* --------------------------------	*/

TS_InfoVar usr_var_rapport =
    {
    	{
										/* --------------------------------	*/
    									/* Type               				*/
										/* --------------------------------	*/
			{
				/* Reserved      */ 0,
                /* Position      */ 1,
                /* Communication */ VAR_PRODUCED,
                /* Scope         */ 1,
                /* With_Time_Var */ 0,
                /* Refreshment   */ 0,
                /* Indication    */ 1,
                /* Priority      */ 0,
                /* RQa           */ 0,
                /* MSGa          */ 1
			},
    		/* ID                 */ 0x0000,
    		/* Var_Length         */ 0x48,
	 		/* Refreshment_Period */ 2000000 /* es */,
    		/* Promptness_Period  */ 2000000 /* es */,
    		/* Rank               */ 1,
	 		/* Signals            */
			{
				NULL,
                User_Signal_Asent_usr_var_rapport,
				NULL,
                User_Signal_Var_Prod_usr_var_rapport,
                User_Signal_Var_Cons_usr_var_rapport
			},
	 	} /* VarDesc */,
	 	/* VarRef             */ NULL
	 };

/*--------------------------------------------------------------------------*/
/* FullDuplex Messaging Context variable : usr_fip_contexte_msg						*/
/*--------------------------------------------------------------------------*/

TS_InfoMsgCtxFD usr_fip_contexte_msg =
    {
    TYPE_MSG_FULLDUP,
        {
        /* Position            */ _FDM_MSG_IMAGE_1,
        {
        /* User_Msg_Ack_Proc   */ usr_fip_contexte_msg_ack,
        /* User_Qid            */ NULL,
        /* User_Ctxt           */ NULL,
        /* Channel_Nr          */ 1,
        } /* sending */,
        {
        /* User_Msg_Rec_Proc   */ usr_fip_contexte_msg_recv,
        /* User_Qid            */ NULL,
        /* User_Ctxt           */ NULL,
        /* Number_Of_Msg_Desc  */ 5,
        /* Number_Of_Msg_Block */ 5,
        } /* receiving */,
        /* Local_DLL_Address   */ (0x0600)<<8+0x0000,
        /* Remote_DLL_Address  */ (0x100600)
        },
    /* FDM_MESSAGING_REF   */ NULL
    };

/* Channel variable : CHANNEL_CHANNEL1                                         */
/*-----------------------------------------------------------------------------*/

/* --------------------------------
 *  * usr_fip_mcs_fdm_channel
 *   * =======================
 *    * --------------------------------	*/
/*
TS_InfoChannel usr_fip_mcs_fdm_channel =
{

    {
						_FDM_MSG_IMAGE_1,
						1,
						0x1000
                       },
    FDM_NOK
};
*/
TS_InfoChannel usr_fip_canal =
    {
    /* Param        */ {
                       /* Position            */ _FDM_MSG_IMAGE_1,
                       /* Channel_Nr          */ 1,
                       /* Identifier          */ 0x0600
                       },
    /* CreateStatus */ FDM_NOK
    };


/* --------------------------------
 * usr_fip_lire_adresse
 * ====================
 * la fonction retourne l'adresse
 * WorldFip de la station.
 * --------------------------------	*/

unsigned long usr_fip_lire_adresse(void)
{
	return usr_config_hard.K_PHYADR;
}

/* --------------------------------	*/
/* usr_fip_network_start			*/
/* ====================				*/
/* Demarrage de WorldFip			*/
/* --------------------------------	*/

unsigned short usr_fip_network_start (int adresseFip)

{
unsigned short V_status;
unsigned char unused;
unsigned char volatile *ptUnused=(unsigned char volatile *)&unused;

	printf("Demarrage de WorldFip avec l'adresse %d\n",adresseFip);
	/* ajouterModuleFip();
	ajouterModuleFdm();
	ajouterModuleMcs(); */
	/* printf("La val %02X\n",*(unsigned char volatile *)0x30010000); */
	/* printf("La val %02X\n",*(unsigned char volatile *)0xE00E4000); */
	*ptUnused=*(unsigned char volatile *)(FIP_BASE+0x40);
	ksleep(1000);
	printf("Reset worldFip OK %d %#0x\n",adresseFip,FIP_BASE+0x40);
										/* INITIALISATION, A RECUPERER BIENTOT
										 * DANS UN REGISTRE DE LA CARTE.
										 */
										/* INITIALISER ICI TOUTES LES VARIABLES
										 * POUR PLACER LE BON NUMERO DE
										 * VARIABLE.
										 */
	 usr_config_hard.K_PHYADR					 =adresseFip;
	 									/* Corrections.						*/
	 usr_fip_contexte_msg.Msg.Local_DLL_Address	=
        /* Local_DLL_Address   */ ((0x0600)<<8+0x0000)+(adresseFip<<8);
	 usr_var_rapport.VarDesc.ID					=	adresseFip;
	 usr_fip_canal.Param.Identifier				=	0x600+adresseFip;
	 // usr_fip_mcs_fdm_channel.Param.Identifier	+=adresseFip;
    /*-------FIP DEVICE MANAGER library initialization----------------------*/
    
	 etaSystSetPos(12,1100);

    fdm_initialize ();

    /*-------FIP DEVICE MANAGER instance creation---------------------------*/
	 etaSystSetPos(12,1110);

    usr_fip_ref = fdm_initialize_network (&usr_config_soft,
                                           &usr_config_hard,
                                           &usr_ident);
    if (usr_fip_ref == NULL)
        return (usr_fip_network_stop (C_ErrorInstanceCreation));

    /*-------Medium redundancy management function initialization-----------*/

	 etaSystSetPos(12,1120);
	 
	 V_status = fdm_valid_medium (usr_fip_ref, _MEDIUM_1);
    if (V_status != FDM_OK)
        return (usr_fip_network_stop (C_ErrorValidMedium));

    /*-------BA function start----------------------------------------------*/
	/*
    BA_Start (usr_fip_ref); */

	 etaSystSetPos(12,1130);
    /*-------AELE creation--------------------------------------------------*/
    usr_table_aele.AELEref = fdm_ae_le_create (
                                        usr_fip_ref,
                                        usr_table_aele.NrOfVar,
                                        usr_table_aele.Type);
    if (usr_table_aele.AELEref == NULL)
        return (usr_fip_network_stop (C_ErrorAELECreation));

	 etaSystSetPos(12,1140);
    /*-------MPS variables creation-----------------------------------------*/
    usr_var_rapport.VarRef = fdm_mps_var_create (
                            usr_table_aele.AELEref,
                            &usr_var_rapport.VarDesc);
    if (usr_var_rapport.VarRef == NULL)
        return (usr_fip_network_stop (C_ErrorMPSVariableCreation));

	 etaSystSetPos(12,1150);
    usr_var_date.VarRef = fdm_mps_var_create (
                            usr_table_aele.AELEref,
                            &usr_var_date.VarDesc);
    if (usr_var_date.VarRef == NULL)
        return (usr_fip_network_stop (C_ErrorMPSVariableCreation));

	 etaSystSetPos(12,1160);
    usr_var_dummy.VarRef = fdm_mps_var_create (
                            usr_table_aele.AELEref,
                            &usr_var_dummy.VarDesc);
    if (usr_var_dummy.VarRef == NULL)
        return (usr_fip_network_stop (C_ErrorMPSVariableCreation));

	 etaSystSetPos(12,1170);
    /*-------AELE start-----------------------------------------------------*/
	 fdm_ae_le_start(usr_table_aele.AELEref);

	 etaSystSetPos(12,1190);
    /*-------Channels creation----------------------------------------------*/
    usr_fip_canal.Status = fdm_channel_create (
                            usr_fip_ref,
                            &usr_fip_canal.Param);
    if (usr_fip_canal.Status == FDM_NOK)
        return (usr_fip_network_stop (C_ErrorPeriodicChannelCreation));


	 etaSystSetPos(12,1200);
	usr_fip_mcs_init(usr_fip_ref,adresseFip);
	/*
	 printf("Creation du premier canal\n");
    usr_fip_mcs_fdm_channel.Status = fdm_channel_create (
                            usr_fip_ref,
                            &usr_fip_mcs_fdm_channel.Param);
    if (usr_fip_mcs_fdm_channel.Status == FDM_NOK)
        return (usr_fip_network_stop (C_ErrorPeriodicChannelCreation));
	*/
    /*-------Messaging Contexts creation------------------------------------*/
	 etaSystSetPos(12,1210);
    usr_fip_contexte_msg.MsgRef = fdm_messaging_fullduplex_create (
                                                   usr_fip_ref,
                                                   &usr_fip_contexte_msg.Msg);
	 etaSystSetPos(12,1220);
    if (usr_fip_contexte_msg.MsgRef == NULL)
        return (usr_fip_network_stop (C_ErrorMessagingContextCreation));
    /*-------Time processing. ---------------------------------------------	*/
	/* usr_fip_time_ref=fdm_generic_time_initialize(
	 * usr_fip_ref,&usr_fip_time_def);
	if(usr_fip_time_ref==NULL)
	{
		return(usr_fip_network_stop(C_ErrorGenericTimeInitialisation));
	} */
	 etaSystSetPos(12,1230);
										/* --------------------------------
										 * FIN DE usr_fip_network_start
										 * --------------------------------	*/
    return (C_NoError);
}

/* --------------------------------	*/
/* usr_fip_network_stop				*/
/* ====================				*/
/* Procedure d'arret sur erreur de  */
/* la fonction de lancement. Le ni-	*/
/* veau d'erreur est passe en para-	*/
/* metre.							*/
/* --------------------------------	*/

unsigned short usr_fip_network_stop (unsigned short ErrorCode)

{
	printf("Il y a une erreur %d\n",ErrorCode);
    switch (ErrorCode)
        {
        /*------Complete stop-----------------------------------------------*/
        case C_NoError:
            {
            }
        /*------Periodic channel creation error-----------------------------*/
        case C_ErrorPeriodicChannelCreation:
            {
            if (usr_fip_canal.Status == FDM_OK)
                {
		 etaSystSetPos(12,2100);
                fdm_channel_delete (usr_fip_ref, usr_fip_canal.Param.Channel_Nr);
                usr_fip_canal.Status = FDM_NOK;
                }
            }
		case C_ErrorGenericTimeInitialisation:


        /*------Messaging context creation error----------------------------*/
        case C_ErrorMessagingContextCreation:
            {
		 etaSystSetPos(12,2200);
				usr_fip_mcs_stop();
	            if (usr_fip_contexte_msg.MsgRef != NULL)
                {
		 etaSystSetPos(12,2300);
   	             	fdm_messaging_delete (usr_fip_contexte_msg.MsgRef);
   	             	usr_fip_contexte_msg.MsgRef = NULL;
                }
            }
        /*------AE/LE start error-------------------------------------------*/
        case C_ErrorAELEStart:
            {
		 etaSystSetPos(12,2310);
            if (fdm_ae_le_get_state (usr_table_aele.AELEref) == AE_LE_RUNNING)
                {
		 etaSystSetPos(12,2320);
                fdm_ae_le_stop (usr_table_aele.AELEref);
                }
            }
        /*------MPS variable creation error---------------------------------*/
        case C_ErrorMPSVariableCreation:
            {
            /* Nothing to do */
            }
        /*------AE/LE creation error----------------------------------------*/
        case C_ErrorAELECreation:
            {
            if (usr_table_aele.AELEref != NULL)
                {
		 etaSystSetPos(12,2330);
                fdm_ae_le_delete (usr_table_aele.AELEref);
                usr_table_aele.AELEref = NULL;
                }
            }
        /*------Medium validation error-------------------------------------*/
        case C_ErrorValidMedium:
            {
		 etaSystSetPos(12,2340);
            fdm_stop_network (usr_fip_ref);
            }
        /*------FDM NT context creation error-------------------------------*/
        case C_ErrorCreateContextFdmNT:
            {
            /* Nothing to do */
            }
        /*------FDM instance creation error---------------------------------*/
        case C_ErrorInstanceCreation:
            {
            /* Nothing to do */
            }
        default:
            break;
        }
		 etaSystSetPos(12,2350);
    return (ErrorCode);
}
