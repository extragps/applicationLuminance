#include <stdio.h>
#include <FDM.H>
#include <FDMPRIVE.H>

#ifdef ST1
#include "st1.h"
#endif
#ifdef ST2
#include "st2.h"
#endif

FDM_MSG_T_DESC Msg_T_Desc[NB_MSG_CONTEXT];
FDM_MSG_TO_SEND Msg_Send [NB_MSG_CONTEXT];

FDM_MSG_RECEIVED msg_rec;

extern unsigned char Bufferr[];
extern int nb_rec;

extern int Msg_Send_Busy[];

FDM_MESSAGING_FULLDUPLEX Msg_Fulldup [NB_MSG_CONTEXT];

/* void User_Msg_Ack (FDM_MESSAGING_REF *, FDM_MSG_TO_SEND * ); */

void User_Msg_Ack_00 (FDM_MESSAGING_REF *, FDM_MSG_TO_SEND * );
void User_Msg_Ack_01 (FDM_MESSAGING_REF *, FDM_MSG_TO_SEND * );

void (*Tab_User_Msg_Ack[]) (FDM_MESSAGING_REF *, FDM_MSG_TO_SEND * )=
{
	User_Msg_Ack_00,
	User_Msg_Ack_01
};

void User_Msg_Rec (FDM_MESSAGING_REF *, FDM_MSG_RECEIVED *);

int QidS [NB_MSG_CONTEXT];
int CtxtS [NB_MSG_CONTEXT];
int QidR [NB_MSG_CONTEXT];
int CtxtR [NB_MSG_CONTEXT];
int local_adr;
int remote_adr;

FDM_MESSAGING_REF * init_msg ( FDM_REF *Net_Ref,int lsap,int myadr,int segment,int distadr,int No_Context)
{

	/* Init No de contextes qui seront recuperes dans User_Msg_Rec
	   (Voir si la Remote addr recuperee n'est pas suffisante)
	*/

	QidR[0] = 0;
	QidR[1] = 1;

	CtxtR[0] = 0;
	CtxtR[1] = 1;

	Msg_Fulldup[No_Context].Position = _FDM_MSG_IMAGE_1;
	Msg_Fulldup[No_Context].sending.User_Msg_Ack = Tab_User_Msg_Ack[No_Context];
	Msg_Fulldup[No_Context].sending.User_Qid = &QidS[No_Context];
	Msg_Fulldup[No_Context].sending.User_Ctxt = &CtxtS[No_Context];
/*	Msg_Fulldup[No_Context].sending.Channel_Nr = 0;	/* Messagerie aperiodique */
	Msg_Fulldup[No_Context].sending.Channel_Nr = 2;	/* Messagerie periodique */

	Msg_Fulldup[No_Context].receiving.User_Msg_Rec_Proc = User_Msg_Rec;
	Msg_Fulldup[No_Context].receiving.User_Qid = &QidR[No_Context];
	Msg_Fulldup[No_Context].receiving.User_Ctxt = &CtxtR[No_Context];
	Msg_Fulldup[No_Context].receiving.Number_Of_Msg_Desc = 1;
	Msg_Fulldup[No_Context].receiving.Number_Of_Msg_Block = 1;

	
	/*
	local_adr=(((lsap+No_Context)<<16)&0xff0000)+((myadr<<8)&0xff00)+(segment&0xff);
	remote_adr=((lsap+No_Context<<16)&0xff0000)+((distadr<<8)&0xff00)+(segment&0xff);
	*/
	
	/* Adresses pour Sirius */
	
	#ifdef ST1 /* Passerelle */
	local_adr = 0x100600;
	remote_adr = 0x060000 + (((distadr+No_Context)<<8)&0xff00);
	#endif

	#ifdef ST2
	local_adr = 0x060000 + (((myadr+No_Context)<<8)&0xff00);
	remote_adr = 0x100600;
	#endif
		
	printf ("Contexte %d: local adr: %08x  remote_adr: %08x\n", No_Context, local_adr, remote_adr);

	Msg_Fulldup[No_Context].Local_DLL_Address = local_adr;
	Msg_Fulldup[No_Context].Remote_DLL_Address = remote_adr;

	return fdm_messaging_fullduplex_create (Net_Ref, &Msg_Fulldup[No_Context]);
}


/*
void User_Msg_Ack (FDM_MESSAGING_REF *Msg_Ref,
				   FDM_MSG_TO_SEND *Msg_To_Send)
{
	 printf ("User_Msg_Ack Function: Valid: %d soft_report: %d Way: %d\n",
	 		  Msg_To_Send->Service_Report.Valid,
	 		  Msg_To_Send->Service_Report.msg_user_soft_report,
	 		  Msg_To_Send->Service_Report.Way
	 		  );
	Msg_Send_Busy = 0;
}
*/

void User_Msg_Ack_00 (FDM_MESSAGING_REF *Msg_Ref,
				      FDM_MSG_TO_SEND *Msg_To_Send)
{
	/*
	printf ("User_Msg_Ack_00 Function: LocA:%x RemA:%x Valid:%d soft_report:%d Way:%d\n",
	 		 Msg_To_Send->Local_DLL_Address,
	 		 Msg_To_Send->Remote_DLL_Address,
	 		 Msg_To_Send->Service_Report.Valid,
	 		 Msg_To_Send->Service_Report.msg_user_soft_report,
	 		 Msg_To_Send->Service_Report.Way
	 		 );
	*/
	Msg_Send_Busy[0] = 0;
}

void User_Msg_Ack_01 (FDM_MESSAGING_REF *Msg_Ref,
				      FDM_MSG_TO_SEND *Msg_To_Send)
{
	/*
	printf ("User_Msg_Ack_01 Function: LocA:%x RemA:%x Valid:%d soft_report:%d Way:%d\n",
	 		 Msg_To_Send->Local_DLL_Address,
	 		 Msg_To_Send->Remote_DLL_Address,
	 		 Msg_To_Send->Service_Report.Valid,
	 		 Msg_To_Send->Service_Report.msg_user_soft_report,
	 		 Msg_To_Send->Service_Report.Way
	 		 );
	*/
	Msg_Send_Busy[1] = 0;
}


void User_Msg_Rec (FDM_MESSAGING_REF *Msg_Ref,
				   FDM_MSG_RECEIVED *Msg_Rec)
{
	int i;
		
	nb_rec = Msg_Rec->Ptr_Block->Nr_Of_Bytes;
	
	strncpy((char *)Bufferr,(const char *) Msg_Rec->Ptr_Block->Ptr_Data,nb_rec);

	printf("Msg recu Ctxt %d LocA:%x RemA:%x : %s\n",
			* (int *)Msg_Rec->User_Ctxt,
			Msg_Rec->Local_DLL_Address,
			Msg_Rec->Remote_DLL_Address,
			Bufferr
			);


	fdm_msg_data_buffer_free (Msg_Rec->Ptr_Block);
	fdm_msg_ref_buffer_free (Msg_Rec);			  
}

send_mes(FDM_MESSAGING_REF *Msg_Ref, int No_Context, unsigned char *message )
{
	FDM_MSG_TO_SEND *Pt_Msg_To_Send;

	/* Utilisation d'1 structure FDM_MSG_TO_SEND globale */
	Pt_Msg_To_Send = &Msg_Send[No_Context];	

	Pt_Msg_To_Send->Nr_Of_Blocks = 1;
	Pt_Msg_To_Send->Ptr_Block = &Msg_T_Desc[No_Context];
	Pt_Msg_To_Send->Ptr_Block->Next_Block =  0;
	Pt_Msg_To_Send->Ptr_Block->Nr_Of_Bytes = strlen ((char *)message);
	Pt_Msg_To_Send->Ptr_Block->Ptr_Data = message;


	printf ("Em msg FIP contexte %d de %x a %x\n",
			No_Context,
			Msg_Fulldup[No_Context].Local_DLL_Address,
	 		Msg_Fulldup[No_Context].Remote_DLL_Address);

	if (fdm_send_message (Msg_Ref, Pt_Msg_To_Send) != FDM_OK)
		return(-1);
}
							  
