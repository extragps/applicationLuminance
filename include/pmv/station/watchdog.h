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
 * $Log: watchdog.h,v $
 * Revision 1.2  2018/06/04 08:38:44  xg
 * Passage en version V122
 *
 * Revision 1.2  2008/10/03 16:21:48  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/03/07 17:25:35  xag
 * *** empty log message ***
 *
 * Revision 1.1  2007/12/05 20:01:54  sgronchi
 * Adaptation a la gestion avec le moduleManager, creation de fichiers annexes (cpu432.c/h, watchdog.c/h) pour alleger la lecture du fichier cpuRdtMain.c
 *
 *********************************************************************************************************************/
#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#define CPU_WD_PORT		8001	/* Numéro de port IP utilise pour la communication entre appli et wd */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
/**--------------------------------------------------------------------------------------------------------------------
 * Structure utilisee pour la gestion du watchdog. Toutes les informations
 * utilise ont été regroupées dans la structure.
 * Reste qu'il faudra peut être faire un include commun entre le watchdog
 * et l'application pour partager les définitions communes. */
struct _Watchdog {
	int32 ident_dw;
	int sock_fd;
	int32 cptErr_dw;
	struct sockaddr_in name;
};

typedef struct _Watchdog Watchdog;

bool watchdogStart( Watchdog *wdInfo_pt, int32 ident,int port );

bool watchdogSignal( Watchdog *wdInfo_pt );

bool watchdogStop( Watchdog *wdInfo_pt );

#endif /*WATCHDOG_H_*/
