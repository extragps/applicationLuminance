/*
 * tache.c
 * ===================
 *
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: Tache.c,v $
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* ********************************
 * LISTE DES INCLUDES
 * ******************************** */

#include <superviseur.h>
#include "standard.h"
#include "Tache.h"
#include "kcommon.h"
#include "portage.h"
#include "Tache.h"

/* ********************************
 * TYPES UTILISES LOCALEMENT
 * ******************************** */

typedef struct ThreadParam {
	FUNCPTR entryPt;
	int arg1;
	int arg2;
	int arg3;
	int arg4;
	int arg5;
	int arg6;
	int arg7;
	int arg8;
	int arg9;
	int arg10;
} ThreadParam;

/* ********************************
 * FONCTIONS DU MODULE
 * ******************************** */

void tacheSetId(T_supDescTache *tache, TASK_ID tid)
{
	tache->tid = tid;
}

unsigned long taskGetId(T_supDescTache *tache)
{
	return kthreadGetIdThread(tache->tid);
}

static int _tacheCreer(char *nom, FUNCPTR ptEntree, short priorite, int arg1, int arg2, int arg3, int arg4, int arg5)
{
	int status;
	status = (int)tacheSpawn(nom, priorite, 0, 12000, ptEntree, arg1, arg2, arg3, arg4, arg5, 0, 0, 0, 0, 0);
	return status;
}

int tacheCreerBis(T_supDescTache *tache, FUNCPTR entree, char *nom, int port)
{
	strcpy(tache->nomTache, nom);
	tache->ptEntree = entree;
	return _tacheCreer(tache->nomTache, tache->ptEntree, 98, tache->numero, port, 0, 0, 0);
}

/*
 * _taskExecute
 * ============
 * Fonction de lancement d'une tache qui permet d'initialiser correctement le
 * taskId pour un fonctionnement optimal du mnsusp.
 * */

static void _tacheExecute(FUNCPTR entryPt,int arg1,int arg2,int arg3,int arg4,int arg5,
		int arg6,int arg7,int arg8,int arg9,int arg10)

{
	supInitTaskId(arg1);
	((void (*)())entryPt)(arg1,arg2,arg3,arg4,arg4,arg5,arg6,arg7,arg8,arg9,arg10);

}

static void _tacheLanceur(ThreadParam *param)
{
	if(NULL!=param)
	{
		if(NULL!=param->entryPt)
		{
			_tacheExecute(param->entryPt, param->arg1, param->arg2, param->arg3, param->arg4, param->arg5,
					param->arg6, param->arg7, param->arg8, param->arg9, param->arg10);
			kmmFree(NULL,param);
		}
	}
}

TASK_ID tacheSpawn(char *name,int priority,int options,int stackSize,FUNCPTR entryPt,
		int numeroTache,int arg2,int arg3,int arg4,int arg5,
		int arg6,int arg7,int arg8,int arg9,int arg10)
{
Kthread *threadPt=NULL;
ThreadParam *param=kmmAlloc(NULL,sizeof(ThreadParam));
	if(NULL!=param)
	{
		param->entryPt=entryPt;
		param->arg1=numeroTache;
		param->arg2=arg2;
		param->arg3=arg3;
		param->arg4=arg4;
		param->arg5=arg5;
		param->arg6=arg6;
		param->arg7=arg7;
		param->arg8=arg8;
		param->arg9=arg9;
		param->arg10=arg10;
		threadPt=kthreadNew((void *(*)(void *))_tacheLanceur,param,name,true);
		if(NULL!=threadPt)
		{
			supInitThreadId(numeroTache,threadPt);
			kthreadStart(threadPt);
		}
	}
	return (FUNCPTR)threadPt;
}

STATUS tacheDelete(TASK_ID threadPt)
{
STATUS retour=ERROR;
	if(NULL!=threadPt)
	{
		retour=OK;
		kthreadDelete((Kthread *)threadPt);
	}
	return retour;
}

unsigned long tacheGetId()
{
	return kthreadGetId();
}

/* *********************************
 * FIN DE Tache.c
 * ********************************* */
