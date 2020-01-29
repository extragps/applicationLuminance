/**********************************************************************************************************************
 * SIAT - Copyright SIAT
 * --------------------------------------------------------------------------------------------------------------------
 * fichier     : 
 * description : 
 * prefixe     : 
 * --------------------------------------------------------------------------------------------------------------------
 * Auteur      : 
 * Date        : 
 * OS          : Linux/uClinux
 * --------------------------------------------------------------------------------------------------------------------
 * $Log: watchdog.c,v $
 * Revision 1.2  2018/06/04 08:38:44  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/10/03 16:21:48  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/03/07 17:25:35  xag
 * *** empty log message ***
 *
 * Revision 1.1  2007/12/05 20:01:54  sgronchi
 * Adaptation a la gestion avec le moduleManager, creation de fichiers annexes (cpu432.c/h, watchdog.c/h) pour alleger la lecture du fichier cpuRdtMain.c
 *
 *********************************************************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <kcommon.h>
#include <klog.h>

#include "watchdog.h"


/**--------------------------------------------------------------------------------------------------------------------
 * 
 * Trois fonctions sont utilisée pour la gestion du watchdog :
 * - l'initialisation qui crée la socket,
 * - l'entretien du watchdog par envoi d'un message de vie,
 * - l'arrêt de l'entretien sur sortie normale (TST QUIT).
 */
bool watchdogStart(Watchdog *wdInfo_pt,int32 ident,int port)
{
	bool retour_b=false;
	wdInfo_pt->ident_dw=ident;
	wdInfo_pt->cptErr_dw=0;
	wdInfo_pt->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (wdInfo_pt->sock_fd < 0)
	{
		perror("socket");
	}
	else
	{
		struct hostent *hp = gethostbyname("127.0.0.1");
		/* adresse du destinataire */
		if(NULL!=hp)
		{
			wdInfo_pt->name.sin_family = AF_INET;
			wdInfo_pt->name.sin_addr.s_addr = ( ( struct in_addr * ) ( hp->h_addr ) )->s_addr; /* 0x7f000001 */
			wdInfo_pt->name.sin_port = ntohs(port);
			klogPut(NULL,LOG_DEBUG,"_mainWdInit : gethostbyname OK ");
			retour_b=true;
		}
		else
		{
			klogPut(NULL,LOG_INFO,"_mainWdInit : probleme avec gethostbyname  ");
			close(wdInfo_pt->sock_fd);
			wdInfo_pt->sock_fd=-1;
		}
	}
	return retour_b;
}

/**--------------------------------------------------------------------------------------------------------------------
 * 
 */
static bool _mainWdEnvoi(Watchdog *wdInfo_pt,char commande)
{
	bool retour_b=false;
	char message[10];
	sprintf(message,"%c%02ld",commande,wdInfo_pt->ident_dw);
	if (sendto(wdInfo_pt->sock_fd, message, strlen(message)+1, 0,
			(const struct sockaddr *)&wdInfo_pt->name, sizeof(wdInfo_pt->name))< 0)
	{
		wdInfo_pt->cptErr_dw++;
		if(wdInfo_pt->cptErr_dw<10)
		{
			perror("sendto");
		}
	}
	else
	{
		retour_b=true;
	}
	return retour_b;
}

/**--------------------------------------------------------------------------------------------------------------------
 * 
 */
bool watchdogSignal(Watchdog *wdInfo_pt)
{
	return _mainWdEnvoi(wdInfo_pt,'v');
}

/**--------------------------------------------------------------------------------------------------------------------
 * 
 */
bool watchdogStop(Watchdog *wdInfo_pt)
{
	bool retour_b=false;
	retour_b=_mainWdEnvoi(wdInfo_pt,'d');
	/* Fermeture de la socket utilisee */
	close(wdInfo_pt->sock_fd);
	wdInfo_pt->sock_fd=-1;
	return retour_b;
}
