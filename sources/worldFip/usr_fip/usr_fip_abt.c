
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 18/12/00
 * Fichier	: usr_fip_abt
 * Objet	: Dans le cadre de tests production d'un message d'abonnement.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,18Dec00,xag  creation.
 * ************************************************************************	*/

/* ********************************	
 * FICHIERS D'INCLUDE
 * ********************************	*/

#ifdef VXWORKS
#include "vxworks.h"
#include "tasklib.h"
#include "wdLib.h"
#include "timers.h"
#include "tickLib.h"
#include "portage.h"
#endif
#include "stdio.h"
#include "limits.h"
#include "time.h"
#include "define.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "fdm.h"
#include "mcs.h"
#include "fdmtime.h"
#include "man_fdmi.h"
#include "man_olga.h"
#include "usr_fip.h"
#include "usr_fip_init.h"
#include "usr_fip_mess.h"
#include "usr_fip_var.h"
#include "usr_fip_msg.h"
#include "usr_fip_abt.h"

/* ********************************		
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define USR_FIP_ABT			 "ABONNEMENT"
#define USR_FIP_ABT_DEMARRE  "DEMARRE"
#define USR_FIP_ABT_TERMINE  "TERMINE"
#define USR_FIP_ABT_LG  	 "LG="
#define USR_FIP_ABT_PERIODE  "PERIODE="


/* ********************************	
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************	
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static BOOL usr_fip_abt_actif=FALSE;
static int  usr_fip_abt_periode=20;
static int  usr_fip_abt_lg=100;
static int 	usr_fip_abt_tick_cour;
static int  usr_fip_abt_reste=0;
static int 	usr_fip_abt_ident;
static int 	usr_fip_abt_dest;

/* ********************************	
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ********************************	*/

static int usr_fip_abt_interroger(char *buffer);
static void usr_fip_abt_terminer(void);
static void usr_fip_abt_demarrer(int periode,int longueur);

/* ********************************	
 * FONCTIONS DU MODULE
 * ********************************	*/


static char *usr_fip_passe_blanc(char *buffer,int lg_mess,int *reste)
{
char *ptCour=buffer;
int   lgCour=lg_mess;
	while(lgCour>0)
	{
		if((*ptCour==' ')||(*ptCour=='\t')||(*ptCour=='\n'))
		{
			ptCour++;
			lgCour--;
		}
		else
		{ 
			break;
		}
	}
	*reste=lgCour;
	return ptCour;
}

static char *usr_fip_lit_entier(char *buffer,int lg_mess,int *reste,int *val)
{
char *ptCour=buffer;
int   lgCour=lg_mess;
int   valeur=0;

	printDebug("Dans tst lit entier\n");
	while((*ptCour>='0')&&(*ptCour<='9'))
	{
		valeur=valeur*10+((*ptCour)-'0');
		ptCour++;
		lgCour--;
	}
	if(lgCour!=lg_mess)
	{
		*val=valeur;
	}
	else
	{
		*val=-1;
	}
	*reste=lgCour;
	return ptCour;
}

static void usr_fip_abt_envoyer_message(char *buffer)
{
int longueur;
T_usr_fip_mess *mess=usr_fip_mess_obtenir_periodique();
	if(NULL!=mess)
	{
		memcpy(&mess->buffer[0],(char *)&usr_fip_abt_ident,4);
		memcpy(&mess->buffer[4],(char *)&usr_fip_abt_dest,4);
		longueur=sprintf((char*)&mess->buffer[8],buffer);
		mess->block.Nr_Of_Bytes=(unsigned short)longueur+8; 
		fdm_send_message(usr_fip_contexte_msg.MsgRef,&mess->mess);
	}
	else
	{
		printDebug("Message periodique non disponible\n");
	}
}

void usr_fip_abt(void)
{
T_usr_fip_mess *mess=usr_fip_mess_obtenir_periodique();
	if(NULL!=mess)
	{
	int longueur;
		longueur=usr_fip_abt_interroger((char *)&mess->buffer[8]);
		if(0!=longueur)
		{
		int indice;
			memcpy(&mess->buffer[0],(char *)&usr_fip_abt_ident,4);
			memcpy(&mess->buffer[4],(char *)&usr_fip_abt_dest,4);
			mess->block.Nr_Of_Bytes=(unsigned short)longueur+8; 
			for(indice=0;indice<longueur;indice++)
			{
				printf("%c",mess->buffer[indice+8]);
			}
			printf("\n");
			fdm_send_message(usr_fip_contexte_msg.MsgRef,&mess->mess);
		}

	}
	else
	{
		printf("Le message n'est pas libre\n");
	}

}


static int usr_fip_abt_interroger(char *buffer)
{
int longueur=0;

	if(TRUE==usr_fip_abt_actif)
	{
	unsigned long 	tickCour=tickGet();
	unsigned long	nbTicks;
		if(tickCour<usr_fip_abt_tick_cour)
		{
			nbTicks=(ULONG_MAX-usr_fip_abt_tick_cour)+tickCour;
		}
		else
		{
			nbTicks=tickCour-usr_fip_abt_tick_cour;
		}
		usr_fip_abt_reste-=nbTicks;		
		usr_fip_abt_tick_cour=tickCour;
		/* Si l'heure est venue, on genere le message */
		if(usr_fip_abt_reste<0)
		{
			usr_fip_abt_reste+=usr_fip_abt_periode*sysClkRateGet();
			longueur=sprintf(buffer,"REPONSE ABONNEMENT ");
			while(longueur<usr_fip_abt_lg)
			{
				buffer[longueur]='0'+(longueur%10);
				longueur++;
			}						/* endwhile(longueur<usr_fip_abt	*/
			/* printDebug("Envoi du message %s\n",buffer);  */
		}
	}
	return(longueur);
}

/* --------------------------------
 * usr_fip_abt_terminer
 * ====================
 * Terminer les reponses sur les 
 * demandes d'abonnement.
 * --------------------------------	*/

static void usr_fip_abt_terminer(void)
{
	usr_fip_abt_actif=FALSE;
	printDebug("Arret de l'abonnement");
	usr_fip_abt_envoyer_message("ABONNEMENT TERMINE OK");
}

/* --------------------------------
 * usr_fip_abt_demarrer
 * ====================
 * Terminer les reponses sur les 
 * demandes d'abonnement.
 * --------------------------------	*/

static void usr_fip_abt_demarrer(int periode,int longueur)
{
char buffer[200];
	usr_fip_abt_actif=TRUE;
	usr_fip_abt_periode	=periode;
	usr_fip_abt_lg		=longueur;
	usr_fip_abt_tick_cour=tickGet();
	usr_fip_abt_reste=periode*sysClkRateGet();
	printDebug(
		"Demarrage de l'abonnement avec une periode de %d "
		"et %d caracteres\n",
		periode,longueur);
										/* Envoyer ici un message comme 
										 * quoi tout va bien.	*/
	sprintf(&buffer[0],"%s LG=%d PERIODE=%s",
			   USR_FIP_ABT,usr_fip_abt_lg,usr_fip_abt_periode);
	usr_fip_abt_envoyer_message(&buffer[0]);
										/* --------------------------------
										 * FIN DE usr_fip_abt_demarrer
										 * --------------------------------	*/
}

int usr_fip_abt_analyser(char *buffer,int lg_mess,int position)
{
char *ptCour=(char *)&buffer[position];
int   lgCour=lg_mess-position;
int 	nbCar=0;
int 	bloc=0;
int 	erreur=0;
										/* Lecture des identificateurs et 
										 * destinataires du message periodique
										 */
	usr_fip_abt_ident=*(int *)&buffer[0];
	usr_fip_abt_dest =*(int *)&buffer[4];
	printf("L'identifiant est %d et la dest %d\n",
			usr_fip_abt_ident,usr_fip_abt_dest);
	buffer[lg_mess]=0;
												

	ptCour+=strlen(USR_FIP_ABT);
	lgCour-=strlen(USR_FIP_ABT);
	ptCour=usr_fip_passe_blanc(ptCour,lgCour,&lgCour);
	if(strncmp(ptCour,USR_FIP_ABT_DEMARRE,strlen(USR_FIP_ABT_DEMARRE))==0)
	{
	int longueur=100;					/* Longueur par defaut des trames	*/
	int	periode =20;					/* Periode par defaut.				*/
		ptCour+=strlen(USR_FIP_ABT_DEMARRE);
		lgCour-=strlen(USR_FIP_ABT_DEMARRE);
		ptCour=usr_fip_passe_blanc(ptCour,lgCour,&lgCour);
		printDebug("On a trouve DEMARRE\n");
		while((lgCour!=0)&&(0==erreur))
		{
			if(strncmp(ptCour,USR_FIP_ABT_PERIODE,
						strlen(USR_FIP_ABT_PERIODE))==0)
			{
				ptCour+=strlen(USR_FIP_ABT_PERIODE);
				lgCour-=strlen(USR_FIP_ABT_PERIODE);
				ptCour=usr_fip_lit_entier(ptCour,lgCour,&lgCour,&periode);
				printDebug("On a trouve PERIODE %d\n",periode);
				ptCour=usr_fip_passe_blanc(ptCour,lgCour,&lgCour);
			}
			else if(strncmp(ptCour,USR_FIP_ABT_LG,strlen(USR_FIP_ABT_LG))==0)
			{
				ptCour+=strlen(USR_FIP_ABT_LG);
				lgCour-=strlen(USR_FIP_ABT_LG);
				ptCour=usr_fip_lit_entier(ptCour,lgCour,&lgCour,&longueur);
				printDebug("On a trouve LG %d\n",longueur);
				ptCour=usr_fip_passe_blanc(ptCour,lgCour,&lgCour);
			}
			else if(lgCour!=0)
			{
				erreur=2;
			}
		}
		if(erreur==2)
		{
			usr_fip_abt_envoyer_message("ABONNEMENT DEMARRE NOK");
		}
		usr_fip_abt_demarrer(periode,longueur);
	}
	else if(strncmp(ptCour,USR_FIP_ABT_TERMINE,strlen(USR_FIP_ABT_TERMINE))==0)
	{
		printDebug("On a trouve TERMINE\n");
		ptCour+=strlen(USR_FIP_ABT_TERMINE);
		lgCour-=strlen(USR_FIP_ABT_TERMINE);
		usr_fip_abt_terminer();
		ptCour=usr_fip_passe_blanc(ptCour,lgCour,&lgCour);
	}
	else
	{
		if(0==lgCour)
		{
		char buffer[200];
			sprintf(&buffer[0],"%s LG=%d PERIODE=%s",
			   USR_FIP_ABT,usr_fip_abt_lg,usr_fip_abt_periode);
		}
		else
		{
			erreur=2;
			printf("Trame non reconnue %s\n",ptCour);
		}
	}
	return((erreur==0));
}

