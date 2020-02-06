/*
 * portMsgQueue
 * ===================
 * Objet : Le module defini les methodes pour la gestion
 * 	d'une file de messages de taille variable.
 *  Chaque element de la liste comporte une information sur
 *  la taille du message en un pointeur sur le message lui
 *  meme. Attention, la zone de memoire doit avoir ete allouee
 *  avec kmmAlloc.
 *
 *  TODO: Reformer le fonctionnement de la chose :
 *  - Utiliser comme identifiant le pointeur sur la file de
 *  message ce qui permettrait de simplifier grandement la
 *  gestion de la liste. Cela doit se faire sans douleur compte
 *  tenu du typage existant.
 *
 *  Created on: 20 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: portMsgQueue.c,v $
 * Revision 1.2  2018/06/04 08:38:38  xg
 * Passage en version V122
 *
 * Revision 1.2  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *****************************
 * LISTE DES INCLUDES
 * ***************************** */

#include "portage.h"
#include "define.h"
#include "semLib.h"
#include "errno.h"
#include "string.h"
#ifdef LINUX
#include "time.h"
#endif
#include <mq.h>
#include "kcommon.h"

typedef struct QUEUE_ELEM
{
	int taille;
	char *info;
} QUEUE_ELEM;

typedef struct QUEUE
{
	MSG_Q_ID id;
	int tailleMax;
	int nbMax;
	Klist *elements;
} QUEUE;

/* ********************************
 * VARIABLES LOCALES
 * ******************************** */

/* Une liste pour permettre la liberation
 * des ressourses lors de l'arret de l'application
 * et un semaphore pour synchroniser les acces aux
 * files de message.
 * On pourrait prevoir d'integrer un mutex par file de
 * messages en plus.
 */
static Klist *msgQListe = NULL;
static Kmutex *msgQMutex = NULL;

/* ********************************
 * PROTOTYPES DES FONCTIONS LOCALES
 * ******************************** */
static void queueDelete(void *);
static void queueElemDelete(void *);

/* *********************************
 * FONCTIONS DU MLDULE
 * ********************************* */

/*
 * queueElemDelete
 * ===============
 * Liberation des ressources allouees pour
 * la gestion des messages.
 */

static void queueElemDelete(void *queue_pt)
{
	if (NULL != queue_pt)
	{
		kmmFree(NULL, ((QUEUE_ELEM *)queue_pt)->info);
	}
	kmmFree(NULL, queue_pt);
}

/*
 * queueElemCreate
 * ===============
 * Creation d'un element pour la file des messages. Les infor-
 * mations communiquées sont recopiees dans un buffer alloue
 * pour la circonstance.
 */

static QUEUE_ELEM *queueElemCreate(int tailleMax, char *info)
{
	QUEUE_ELEM *elem_pt = NULL;
	elem_pt = kmmAlloc(NULL, sizeof(QUEUE));
	if (NULL != elem_pt)
	{
		elem_pt->taille = tailleMax;
		elem_pt->info = kmmAlloc(NULL, tailleMax);
		if (NULL != elem_pt->info)
		{
			memcpy(elem_pt->info, info, tailleMax);
		} else
		{
			kmmFree(NULL, elem_pt);
			elem_pt = NULL;
		}
	}
	return elem_pt;
}

/*
 * queueCreate
 * ===========
 * Creation d'une file de messages. Il s'agit
 * d'une classe derive de la classe klist. Elle
 * integre en plus des informatios habituelles un
 * id qui permet d'acceder a la file des messages
 * par un identifiant plutot qu'un pointeur sur
 * la liste.
 * Pour completer, deux attributs permettent de
 * contraindre la taille des message et le
 * nombre maximal de messages. Peut etre que le
 * second element n'est pas indispensable compte tenu
 * des caracteristiques des files de message.
 */
static QUEUE *queueCreate(int id, int nbMax, int tailleMax )
{
	QUEUE *retour_pt = NULL;
	retour_pt = kmmAlloc(NULL, sizeof(QUEUE));
	if (NULL != retour_pt)
	{
		retour_pt->id = id;
		retour_pt->elements = klistNew(nbMax, KTSL_DEFAULT, NULL, queueElemDelete);
		retour_pt->tailleMax = tailleMax;
		retour_pt->nbMax = nbMax;
	}
	return retour_pt;
}

/*
 * queueDelete
 * ===========
 * Desctruction d'une file de message et de tous les
 * elements qui la compose.
 *
 */

static void queueDelete(void *queue_pt)
{
	if (NULL != queue_pt)
	{
		klistDelete(((QUEUE *) queue_pt)->elements);
		((QUEUE *) queue_pt)->elements = NULL;
	}
	kmmFree(NULL, queue_pt);
}

/*
 * msgQInit
 * =========
 * Initialisation de la file des files de messages.
 * Toutes les files de messages sont en effet regroupees
 * dans une liste afin de permettre de conserver l'ancien
 * acces.
 */

void msgQInit(void)
{
	msgQListe = klistNew(0, KTSL_DEFAULT, NULL, queueDelete);
	msgQMutex = kmutexNew();
}

/*
 * msgQTerm
 * =========
 * Liberation des ressources allouees pour la gestion de la
 * file.
 */
void msgQTerm(void)
{
	kmutexLock(msgQMutex);
	if (NULL != msgQListe)
	{
		klistDelete(msgQListe);
		msgQListe = NULL;
	}
	kmutexUnlock(msgQMutex);
	kmutexDelete(msgQMutex);
	msgQMutex = NULL;
}

/*
 * msgQCreate
 * ==========
 * Creation d'une nouvelle file de message. La file est ajoutee
 * a la liste des files de messages et surtout, un nouvel index
 * est cree. Attention, il est important de constater que
 * l'identifiant est calcule a partir de l'identifiant du
 * dernier element ajoute. Il faudrait peut etre un peu plus de
 * vigilance.
 */
MSG_Q_ID msgQCreate(int maxMsgs, int maxLength, int options)
{
	MSG_Q_ID id_dw = 0;
	if (NULL != msgQListe)
	{
		id_dw = 1;
		kmutexLock(msgQMutex);
		{
			QUEUE *queue_pt = NULL;
			int nbElems = klistGetSize(msgQListe);
			/* On récupére le dernier element si il existe, on prend l'identifiant et on ajoute 1 */
			if (0 != nbElems)
			{
				queue_pt = klistLast(msgQListe);
				id_dw = queue_pt->id + 1;
			}
			queue_pt = queueCreate(id_dw, maxMsgs, maxLength);
			if (NULL == queue_pt)
			{
				id_dw = 0;
			} else
			{
				if(false==klistAdd(msgQListe, queue_pt))
				{
					printf("Erreur d'insertion dans la liste des files \n");
				}
			}
		}
		kmutexUnlock(msgQMutex);
	}
	return id_dw;
}

/*
 * msgQGet
 * =======
 * Recuperation d'une file a partir de son identifiant.
 */
static QUEUE * msgQGet(MSG_Q_ID msgQId)
{
	QUEUE *queue_pt = NULL;
	for (queue_pt = klistFirst(msgQListe); queue_pt != NULL; queue_pt = klistNext(msgQListe))
	{
		if (msgQId == queue_pt->id)
		{
			break;
		}
	}
	return queue_pt;
}

/*
 * msgQDelete
 * ==========
 * Destruction d'une file de message a partir de son identifiant
 * et suppression de la liste des files de messsages.
 */
STATUS msgQDelete(MSG_Q_ID msgQId)
{
	STATUS retour = ERROR;
	if (NULL != msgQListe)
	{
		QUEUE *queue_pt = NULL;
		kmutexLock(msgQMutex);
		/* Recherche de la queue dans la liste */
		queue_pt = msgQGet(msgQId);
		/* Extraction de la liste */
		if (NULL != queue_pt)
		{
			klistRemove(msgQListe, queue_pt);
			queueDelete(queue_pt);
		}
		kmutexUnlock(msgQMutex);
	}
	return retour;
}

/*
 * msqQReceive
 * ===========
 * Recuperation d'un message dans une file.
 */
int msgQReceive(MSG_Q_ID msgQId, char *buffer, UINT nbBytes, int timeout)
{
	int nbCarLus = -1;
	if (NULL != msgQListe)
	{
		QUEUE *queue_pt = NULL;
		kmutexLock(msgQMutex);
		/* Recheche de la queue dans la liste */
		queue_pt = msgQGet(msgQId);
		/* Recuperation du premier message de la liste */
		if (NULL != queue_pt)
		{
			QUEUE_ELEM *elem_pt = (QUEUE_ELEM *) klistOutFifo(queue_pt->elements);
			/* Recopie des informations dans le buffer */
			if (NULL != elem_pt)
			{
				nbCarLus = MIN(nbBytes, elem_pt->taille);
				memcpy(buffer, elem_pt->info, nbCarLus);
				/* Suppression du message */
				queueElemDelete(elem_pt);
			}

		}
		kmutexUnlock(msgQMutex);
	}
	return nbCarLus;
}

/*
 * msgQSend
 * ========
 * Envoi d'un message.
 */
int msgQSend(MSG_Q_ID msgQId, char *buffer, UINT nbBytes, int timeout, int priority)
{
	int nbCarEcrits = -1;
	if (NULL != msgQListe)
	{
		QUEUE *queue_pt = NULL;
		kmutexLock(msgQMutex);
		/* Recheche de la queue dans la liste */
		queue_pt = msgQGet(msgQId);
		/* Recuperation du premier message de la liste */
		if (NULL != queue_pt)
		{
			if(klistGetSize(queue_pt->elements)<queue_pt->nbMax)
			{
			QUEUE_ELEM *elem_pt = queueElemCreate(nbBytes, buffer);
			/* Recopie des informations dans le buffer */
			if (NULL != elem_pt)
			{
				nbCarEcrits = nbBytes;
				if(false==klistAdd(queue_pt->elements, elem_pt))
				{
					printf("Erreur d'insertion dans la file %ld\n",queue_pt->id);
					queueElemDelete(elem_pt);
				}
			}
			}

		}
		kmutexUnlock(msgQMutex);
	}
	return nbCarEcrits;
}

/* *********************************
 * FIN DE portMsgQUeue.c
 * ********************************* */
