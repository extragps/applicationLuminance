/*************************************************************************************************
 *
 *  Source file : monitor.c
 *
 *  Project     : Application example running with FIP DEVICE MANAGER V4.2 
 *                on a CC121 PC board under DOS environment at 1Mbps
 *
 *  Version     : 1.0
 *
 *  Hardware & Software environment : FULLFIP2 / CC121 / DOS 6.22
 *
 *  Author      : WorldFIP  - Jorge de AZEVEDO - 
 *  Date        : March 1998
 *
 *  Description: RealTime DOS monitor simulator
 *  
 *
 *************************************************************************************************
 *
 *  Revision history:
 *
 *************************************************************************************************
 *
 *  Copyright (c) 1998 WorldFIP
 *  1 rue de Bene
 *  92160 ANTONY - FRANCE -
 *
 *  e-mail : wftechno@worldfip.imaginet.fr
 *
 ************************************************************************************************/
 
#include "fdm.h"
#include "fdmtime.h"
#include "usr_fip.h"
#include "man_olga.h"

static unsigned int ticks_TOP;//debug flag
static int polling=1;
int debugOn=0;


void usr_fip_periodic(void)
{
	while(1)
	{
	int nbIts;
	int nbEoc;
	
		nbIts=0;
		nbEoc=0;
		if(polling)
		{
			while(fdm_process_it_irq(usr_fip_ref)==IRQ_TO_PROCESS) nbIts++;
			while(fdm_process_it_eoc(usr_fip_ref)==IRQ_TO_PROCESS) nbEoc++;
		}

/*		if((nbIts+nbEoc)!=0)
			printf("Ticks counter Its %03d Eoc %03d\n",nbIts,nbEoc);
		if(debugOn)
			printf("Ticks counter Its %03d Eoc %03d\n",nbIts,nbEoc);
			*/
  		fdm_ticks_counter();

  		if (network_MPS_Aperiodic_event > 0)
  		{
		if(debugOn)
			printf("Aperiodic\n");
		    fdm_mps_fifo_empty( usr_fip_ref );
		    network_MPS_Aperiodic_event = 0;
		}
		
		if (network_received_MSG_event > 0)
		{
			if(debugOn)
				printf("Received\n");
		    fdm_msg_rec_fifo_empty( usr_fip_ref );
		    network_received_MSG_event=0;
		}
		
		if (network_transmitted_MSG_event > 0)
		{
			if(debugOn)
				printf("Send\n");
		    fdm_msg_send_fifo_empty( usr_fip_ref );
		    network_transmitted_MSG_event=0;
		}           
										/* Traitement des evenements MCS	*/
		usr_fip_mcs_send();				/* Retransmission eventuelle du 
										   message recu par MCS	*/

		usr_fip_mcs_process();

		ticks_TOP++;
		ksleep(500);
	}
}
