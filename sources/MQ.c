
#include "MQ.h"
#include "standard.h"
#include "Superviseur.h"
#ifndef VXWORKS
#include "portage.h"
#endif

MSG_Q_ID SysMQCreer (void)
{
  MSG_Q_ID MqId;

    /*---------------------------------------------------------------
      Create a message queue - fill in a mq_attr structure with the 
      size and no. of messages required, and pass it to mq_open(). 
    ---------------------------------------------------------------*/
  MqId = msgQCreate (2, MSG_SIZE, MSG_Q_FIFO);
  return MqId;
}

int SysMQDetruire (MSG_Q_ID MqId)
{
  int retour = msgQDelete (MqId);
  return retour;
}

int SysMQEmettre (MSG_Q_ID MqId, char *pMess, unsigned short Longueur)
{
  int retour = OK;
  retour = msgQSend (MqId, pMess, Longueur + 1, 0, MSG_PRI_NORMAL);
  return retour;
}

int SysMqLire (MSG_Q_ID MqId, char *pMess)
{
  int nbLu = 0;
  int len;
  len = msgQReceive (MqId, pMess, MSG_SIZE, 0);
  if (ERROR != len)
    {
      nbLu++;
    }
  return nbLu;
}
