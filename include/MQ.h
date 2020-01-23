
#ifndef _MQ_INC
#define _MQ_INC

#include "stdio.h"
#include "signal.h"
#include "fcntl.h"
#include "errno.h"
#include "standard.h"



#define MSG_SIZE    1024               /* limit on message sizes */

#ifdef LINUX
typedef long MSG_Q_ID;
#endif

typedef MSG_Q_ID mqd_t;

/*******************************************************************/
/*		Prototypes des fonctions du modules                */
/*******************************************************************/

MSG_Q_ID SysMQCreer (void);
int SysMQDetruire (MSG_Q_ID MqId);
int SysMQEmettre (MSG_Q_ID MqId, char *pMess, unsigned short Longueur);
int SysMqLire (MSG_Q_ID MqId, char *pMess);

MSG_Q_ID msgQCreate(int maxMsgs, int maxLength, int options) ;
STATUS msgQDelete(MSG_Q_ID msgQId) ;
int msgQSend(MSG_Q_ID msgQId, char *buffer, UINT nbBytes, int timeout, int priority);
int msgQReceive(MSG_Q_ID msgQId, char *buffer, UINT nbBytes, int timeout);


#endif
