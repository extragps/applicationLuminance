
/*
 * standard.h
 * ===================
 *
 *  Created on: 24 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: standard.h,v $
 * Revision 1.2  2018/06/04 08:38:40  xg
 * Passage en version V122
 *
 * Revision 1.7  2009/04/16 12:49:50  xag
 * Ajout du klog.h pour eviter les erreurs de compilation.
 *
 * Revision 1.6  2008/07/02 15:07:15  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
#ifndef STANDARD_H
#define STANDARD_H

#include "kcommon.h"
#include "klog.h"
#include <sys/time.h>

/* redefinition des types par soucis d'homogeneite et de portabilite */

#define GLOBALREF      extern
#define LOCAL          static
#define REGISTRE       register

typedef void VOID;

typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;

#ifndef __INCntcontexth
typedef unsigned char BYTE;     /* XG : Ajout pour eviter le conflit */
                                        /* avec ntdetect.                   */
#endif
typedef char CR8;

typedef char CHAR;

typedef short SHORT;
typedef short CR16;

typedef int INT;

typedef long LONG;
typedef float FLOAT;
typedef char *STRING;
typedef char *POINTEUR;
typedef double DOUBLE;

/* types particuliers */
typedef int FICHIER;
#ifdef LINUX
#define min(a,b) ( ((a) < (b)) ? (a) : (b) )
#define max(a,b) ( ((a) > (b)) ? (a) : (b) )
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef enum { oui,non } BOOL;
typedef void * FUNCPTR;
typedef int STATUS;
typedef Kthread *TASK_ID;
#else
typedef int TASK_ID;
#endif

#endif
/* *********************************
 * FIN DE standard.h
 * ********************************* */
