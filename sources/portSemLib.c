/*
 * portSemLib.c
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: portSemLib.c,v $
 * Revision 1.2  2018/06/04 08:38:36  xg
 * Passage en version V122
 *
 * Revision 1.16  2009/08/24 08:24:34  xgaillard
 * Suppression des fonctions de lecture de la version.
 *
 * Revision 1.15  2009/06/30 12:11:27  xgaillard
 * Version 1.15
 *
 * Revision 1.14  2009/06/23 15:49:25  xgaillard
 * Version 1.14
 *
 * Revision 1.13  2009/06/04 16:00:23  xag
 * Increment du numero de version
 *
 * Revision 1.12  2009/04/16 13:38:15  xag
 * Increment du numero de version.
 *
 * Revision 1.11  2009/03/12 09:14:46  xag
 * Increment de la version
 *
 * Revision 1.10  2009/02/27 16:27:33  xag
 * Version 1.10
 *
 * Revision 1.9  2009/01/19 15:58:36  xag
 * Increment du numero de version
 *
 * Revision 1.8  2009/01/09 16:23:21  xag
 * Increment du numero de version
 *
 * Revision 1.7  2008/12/02 14:26:30  xag
 * Passage a la version 1.5
 *
 * Revision 1.6  2008/11/26 08:50:57  xag
 * Changement du numero de version
 *
 * Revision 1.5  2008/11/19 10:17:21  xag
 * Changement du numero de version de l'application.
 *
 * Revision 1.4  2008/11/05 08:59:13  xag
 * Changement du numero de version de l'application.
 *
 * Revision 1.3  2008/10/27 11:02:14  xag
 * Changement du numero de version de l'application.
 *
 * Revision 1.2  2008/09/22 07:53:42  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include "portage.h"
#include "define.h"
#include "semLib.h"
#include "errno.h"
#include "time.h"
#include <unistd.h>
#include <sys/reboot.h>

/* *************************************
 * FONCTIONS DU MODULE
 * ************************************* */

SEM_ID semBCreate(int options, SEM_B_STATE initialState)
{
	return semBCreateNamed(options, initialState, "Pas de nom");
}

SEM_ID semBCreateNamed(int options, SEM_B_STATE initialState, char *nom)
{
	SEM_ID sem = NULL;
	sem = kmutexNewExt(nom, false);
	if (NULL != sem)
	{
		if (SEM_FULL == initialState)
		{
			kmutexLock(sem);
		}
	}
	return sem;
}

EVENT_ID semEventCreate()
{
	EVENT_ID event = NULL;
	event = keventNew(false);
	return event;
}

STATUS semEventDelete(EVENT_ID event)
{
	STATUS retour = ERROR;
	if (NULL != event)
	{
		keventDelete(event);
		retour = OK;
	}
	return retour;
}

STATUS semDelete(SEM_ID sem)
{
	STATUS retour = ERROR;
	if (NULL != sem)
	{
		kmutexDelete(sem);
		retour = OK;
	}
	return retour;
}

STATUS semSignal(EVENT_ID event)
{
	STATUS retour = ERROR;
	if (NULL != event)
	{
		//		printf("Signal %d\n",event);
		keventSignal(event);
		retour = OK;
	}
	return retour;
}

STATUS semTakeTimeout(SEM_ID sem, int val)
{
	STATUS retour = ERROR;
	if (NULL != sem)
	{
		switch (val)
		{
		case WAIT_FOREVER:
			kmutexLock(sem);
			retour = OK;
			break;
		case NO_WAIT:
			if (true == kmutexTryLock(sem,val))
			{
				retour = OK;
			}
			break;
		default:
			if (true == kmutexTryLock(sem,val))
			{
				retour = OK;
			}
			break;
		}
		retour = OK;
	}
	return retour;
}

STATUS semTake(SEM_ID sem)
{
	STATUS retour = ERROR;
	if (NULL != sem)
	{
		kmutexLock(sem);
		retour = OK;
	}
	return retour;
}

STATUS semWait(EVENT_ID sem)
{
	STATUS retour = ERROR;
	if (NULL != sem)
	{
		//		printf("Wait %d\n",sem);
		keventWait(sem);
		retour = OK;
	}
	return retour;
}

STATUS semGive(SEM_ID sem)
{
	STATUS retour = ERROR;
	if (NULL != sem)
	{
		kmutexUnlock(sem);
		retour = OK;
	}
	return retour;
}

ULONG tickGet()
{
	return ktickGetCurrent();
}

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	struct timeval heure;
	struct timezone zone;
	int retour = -1;
	if (NULL != tp)
	{
		retour = gettimeofday(&heure, &zone);
		if (-1 != retour)
		{
			tp->tv_sec = heure.tv_sec;
			tp->tv_nsec = heure.tv_usec * 1000;
		}
	}
	return retour;
}

int intLock(void)
{
	return 4;
}

void intUnlock(int val)
{
}

void calGetDate(struct tm *date)
{
	time_t heure;
	time(&heure);
	localtime_r(&heure, date);
}

void calSetDate(struct tm *date)
{
	time_t heure = mktime(date);
	struct timeval nDate;
	nDate.tv_sec = heure;
	nDate.tv_usec = 0;
	settimeofday(&nDate, NULL);
}

void sysToMonitor(int val)
{
	reboot(RB_AUTOBOOT);
}

/* *********************************
 * FIN DE portSemLib.c
 * ********************************* */
