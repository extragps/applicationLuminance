/*
 * portage.h
 * =========
 *	Le fichier contient les definitions liées au portage depuis vxWorks de l'application
 *	luminance.
 *
 *  Created on: 19 juin 2008
 *      Author: xavier
 */
#ifndef PORTAGE_H
#define PORTAGE_H

#include <time.h>
#include "standard.h"
#include "kcommon.h"

/* **********************************
 * DEFINITION DES CONSTANTE
 * ********************************* */
#define FOREVER for(;;);
#define WAIT_FOREVER 0
#define NO_WAIT -1
#define SEM_FULL 1
#define SEM_EMPTY 0
#define SEM_Q_PRIORITY 12
#define SEM_Q_FIFO 1
#define MSG_Q_FIFO 1

#define MSG_PRI_NORMAL 1

#define SIM_BASE 40000
//#ifdef CPU432
#define SRAM_EXT_BASE 0x90010000
//#else
//#define SRAM_EXT_BASE 0x0
//#endif
#define SRAM_EXT_SIZE 0x1F0000
//#define FDP_BASE 8000000
#define FIP_BASE 0xD00F0000
#define LATTICE_BASE 0xC000000
#define IOP_UART1_ADRS 3
#define IOP_UART2_ADRS 3
#define IOP_REG_OFFSET 9

/* ***************************************
 * DEFINITION DES TYPES
 * *************************************** */

typedef Kmutex *SEM_ID;
typedef Kevent *EVENT_ID;
typedef long WDOG_ID;
typedef struct _TASK_DESC
{
	char *td_name;
} TASK_DESC ;

/* ***************************************
 * PROTOTYPE DES FONCTIONS POUR LE PORTAGE
 * *************************************** */

void msgQInit(void);
void msgQTerm(void);
int versionLireMajeure(const char *name);
int versionLireMineure(const char *name);
void calGetDate(struct tm *date);
void calSetDate(struct tm *date);
void msgQInit(void);
void msgQTerm(void);
void * superviseurMain(int,bool);
int sysClkRateGet(void);
int taskDelay(int);
ULONG tickGet(void);
void sysToMonitor(int val);


#endif
/* *********************************
 * FIN DE portage.h
 * ********************************* */

