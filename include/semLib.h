/*
 * semLib.h
 * ===================
 *
 *  Created on: 24 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: semLib.h,v $
 * Revision 1.2  2018/06/04 08:38:41  xg
 * Passage en version V122
 *
 * Revision 1.2  2008/07/02 15:07:17  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
#ifndef SEM_LIB
#define SEM_LIB
#include "standard.h"
#include "portage.h"

typedef int SEM_B_STATE;

SEM_ID semBCreate(int options,SEM_B_STATE initialState);
SEM_ID semBCreateNamed(int options,SEM_B_STATE initialState,char *nom);
STATUS semDelete(SEM_ID sem);
STATUS semTake(SEM_ID sem);
STATUS semGive(SEM_ID sem);
STATUS semTakeTimeout(SEM_ID sem,int timeout);
EVENT_ID semEventCreate(void);
STATUS semEventDelete(EVENT_ID event);
STATUS semWait(EVENT_ID event);
STATUS semSignal(EVENT_ID event);

#endif

/* *********************************
 * FIN DE semLib.h
 * ********************************* */


