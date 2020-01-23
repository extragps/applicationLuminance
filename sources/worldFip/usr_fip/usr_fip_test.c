
/***************************************************************************/
/*       FICHIER: %M%      REL: %I% DATE: %G%                              */
/*DOC***********************************************************************/
/*                                       SIAT                              */
/*                     8, parc technologique de la Sainte Victoire         */
/*                                    Le Cannet                            */
/*                              13590 Meyreuil - FRANCE                    */
/*                                   Tel. 04 42 58 63 71                   */
/***************************************************************************/
/*  Fichier : usr_fip_test.c                                               */  
/*  MODULE  : usr_fip                                                      */
/***************************************************************************/
/* Auteur :   Xavier GAILLARD                                              */
/* Date de creation : 11 octobre 2000                                      */
/***************************************************************************/
/* OBJET DU MODULE      :                                                  */
/* 	Il s'agit d'ecrire un programme de test pour WorldFip				   */
/*	Le programme realise les chose suivantes :							   */
/*	- initialisation de WorldFip,										   */
/*	- initialisation des structures d'echange avec la tache de cadencement */
/*	- lancement de la tache periodique de cadencement de WorldFip		   */
/*	- attente des messages en provenance de la tache de cadencement. Sur   */
/*    reception, une reponse est composee et renvoyee par le meme chemin.  */
/*	- mise e jour periodique de la variable de rapport.					   */
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


#include "vxWorks.h"
#include "string.h"
#include "pipeDrv.h"
#include "io/iolib.h"
#include "selectLib.h"
#include "Tache.h"
#include "fdm.h"
#include "mcs.h"
#include "fdmtime.h"
#include "usr_fip.h"
#include "usr_fip_var.h"
#include "man_fdmi.h"
#include "man_olga.h"
#include "usr_fip_mcs_init.h"


extern int rapportEmis;
extern int rapportCompteRendu;
extern int dummyPresente;
extern TS_InfoMsgCtxFD usr_fip_contexte_msg;
extern TS_InfoMsgCtxS usr_fip_send_msg;
extern TS_InfoVar usr_var_rapport;

int usr_fip_pipe=-1;

int numero=1;

int usr_fip_test()
{
int retour;
										/* --------------------------------	*/
										/* INIT DE WORLD FIP				*/
										/* --------------------------------	*/
	retour=usr_fip_network_start(0x04);
	usr_varComm_rapport_init(&usr_varComm_rapport);

	if(retour==0)
	{
	int compteur=0;
	int notFin=1;						/* Pour sortir de la boucle.		*/
/*		retour=usr_fip_mcs_init(usr_fip_ref,0x04);
										/* --------------------------------	*/
										/* CREATION DU PIPE					*/
										/* --------------------------------	*/
		pipeDrv();						/* Init du driver					*/
		pipeDevCreate("usr_fip_pipe",5,256);
										/* --------------------------------	*/
										/* INIT DE LA FIFO DE MESSAGE		*/
										/* --------------------------------	*/
		usr_fip_pipe=open("usr_fip_pipe",O_RDWR,0);
		usr_fip_set_fd(usr_fip_pipe);
										/* --------------------------------	*/
										/* INIT DE LA STRUCTURE DE RAPPORT	*/
										/* --------------------------------	*/
		if(usr_fip_pipe!=-1)
		{
		
										/* --------------------------------	*/
										/* LANCEMENT DE LA TACHE PERIODIQUE	*/
										/* --------------------------------	*/
  			taskSpawn("usr_fip_periodic",95,0,2000,(FUNCPTR)usr_fip_periodic,
				0,0,0,0,0,0,0,0,0,0); 
/*		
			usr_fip_periodic();
				*/
										/* --------------------------------	*/
										/* BOUCLE DE TRAITEMENT				*/
										/* --------------------------------	*/
/*			notFin=0;
 *			*/
			while(notFin)
			{
			struct timeval timeout={0,100000};
			fd_set masque;
			int result;
				FD_ZERO(&masque);
				FD_SET(usr_fip_pipe,&masque);
				result=select(usr_fip_pipe+1,&masque,NULL,NULL,&timeout);
										/* Si j'ai recu quelquechose sur 	*/
										/* mon pipe.						*/
				if(FD_ISSET(usr_fip_pipe,&masque))
				{
				char valeur[256];
				int  nbCar;
					nbCar=read(usr_fip_pipe,valeur,256);
					printf("J'ai recu sur le pipe : %s\n",valeur);
					if(nbCar!=-1)
					{
					unsigned char *buffer;
					FDM_MSG_TO_SEND *ptMsg;
					FDM_MSG_T_DESC  *ptBlock;
					int indice;
										/* --------------------------------	*/
										/* ALLOCATION MEMOIRE				*/
										/* --------------------------------	*/
										/* La memoire est liberee dans la 
										 * fonction d'acquitement du message
										 */
						buffer=(unsigned char *)kmmAlloc(NULL,256);
						ptMsg=(FDM_MSG_TO_SEND*)kmmAlloc(NULL,sizeof(FDM_MSG_TO_SEND));
						ptBlock=(FDM_MSG_T_DESC *)
								kmmAlloc(NULL,sizeof(FDM_MSG_T_DESC));
						ptMsg->Next=NULL;
						ptMsg->Prev=NULL;
						ptMsg->Ptr_Block=ptBlock;
						ptMsg->Nr_Of_Blocks=1;
						ptBlock->Next_Block=NULL;
						ptBlock->Ptr_Data=buffer;
						ptBlock->Nr_Of_Bytes=strlen("J'ai bien recu")+6;
										/* --------------------------------	*/
										/* INVERSION DU CONTENU				*/
										/* --------------------------------	*/
						sprintf((char *)buffer,"J'ai bien recu %04d",numero++);
						printf("Compte rendu du fdm_send : %d\n",
							fdm_send_message(usr_fip_contexte_msg.MsgRef,ptMsg));
					}
					else
					{
						printf("Erreur de lecture sur le pipe\n");
					}					/* endif(nbCar!=-1					*/
				}
				else					/* --------------------------------	*/
				{						/* MISE A JOUR VARIABLE RAPPORT		*/	
										/* --------------------------------	*/
					compteur++;
					if((compteur%10)==0)
					{
						/* printf("Modification de la variable de rapport\n");*/
						usr_varComm_rapport.nbMsgLongEmisOk++;
						fdm_mps_var_write_loc(usr_var_rapport.VarRef,
							(void *)&usr_varComm_rapport);
					}
					if(dummyPresente)
					{
						printf("Reception de la variable dummy\n");
						dummyPresente=0;
					}
					if(rapportEmis)
					{
						printf("Emission de la variable rapport : %d\n",
								rapportCompteRendu);
						rapportEmis=0;
					}
				}						/* endif(FD_ISSET					*/
				
			}							/* endwhile(notFin					*/
		}								/* endif(usr_fip_pipe!=-1			*/
	}									/* endif(retour==0					*/
										/* --------------------------------	*/
										/* FIN DE usr_fip_test				*/
										/* --------------------------------	*/
	return retour;


}



