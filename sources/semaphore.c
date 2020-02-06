
/*
 * Semaphore.c
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: Semaphore.c,v $
 * Revision 1.2  2018/06/04 08:38:39  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include "stdio.h"
#include "semaphore.h"
#include "mon_debug.h"
#include "define.h"
#include "portage.h"
#include "semLib.h"

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif





/*DOC****************************************************************************/
/*  FICHE DE  FONCTION  :     SysSemCreer                                  */
/********************************************************************************/
/* OBJET DE LA FONCTION :                                                       */
/* PARAMETRES           :                                                       */
/* VALEURS RENDUES      :                                                       */
/* FONCTIONS APPELEES   :                                                       */
/* FONCTIONS APPELANTES :                                                       */
/****************************************************************************DOC*/
SEM_ID SysSemCreerNamed (char *nom)
{
  SEM_ID Status;


  Status = semBCreateNamed (SEM_Q_FIFO, SEM_FULL,nom);
  if (Status == NULL)
    {
      return NULL;
    }


  return Status;
}


/*DOC****************************************************************************/
/*  FICHE DE  FONCTION  :                                                       */
/********************************************************************************/
/* OBJET DE LA FONCTION :                                                       */
/* PARAMETRES           :                                                       */
/* VALEURS RENDUES      :                                                       */
/* FONCTIONS APPELEES   :                                                       */
/* FONCTIONS APPELANTES :                                                       */
/****************************************************************************DOC*/
void SysSemDetruire (SEM_ID semId)
{
  if (NULL != semId)
    {
      semDelete (semId);
    }
  return;
}

int SysSemPrendre (SEM_ID semId, int tempo)
{
  int Status;
  if (semId == NULL)
    {
      printDebug ("Semaphore incorrect\n");
      return ERROR;
    }
  if (tempo == -1)
    {
      Status = semTakeTimeout (semId, WAIT_FOREVER);
      if (Status == ERROR)
        {
          return ERROR;
        }
    }
  else if (tempo == 0)
    {
      Status = semTakeTimeout (semId, NO_WAIT);
      if (Status == ERROR)
        {
          return ERROR;
        }

    }
  else
    {
      Status = semTakeTimeout (semId, tempo);
      if (Status == ERROR)
        {
          return ERROR;
        }

    }

  return OK;
}

int SysSemDonner (SEM_ID semId)
{
  int retour = ERROR;
  if (NULL != semId)
    {
      retour = semGive (semId);
    }
  return retour;
}
