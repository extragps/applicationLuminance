/*
 * PipeLinux.c
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: PipeLinux.c,v $
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "standard.h"
#include "define.h"
#include "mon_debug.h"
#include "Pipe.h"


#define DEBUG 0

#if DEBUG >0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DECLARATION DES CONSTANTES
 * ******************************** */

#define PIPE_MAX_MSG	5

/* ********************************
 * DECLARATION DES TYPES LOCAUX
 * ******************************** */

/* ********************************
 * DECLARATION DES VARIABLES LOCALES
 * ******************************** */

                                                                                /* Nombre de pipes utilises.    */
/* ********************************
 * DECLARATION DES FONCTIONS
 * ******************************** */

/* --------------------------------
 * SysPipeCreer
 * ============
 * --------------------------------	*/
int SysPipeCreer (char *Nom, INT maxMsgs, INT maxLength)
{
  int retour;
  char nomPipe[200];

  sprintf(nomPipe,"/tmp/%s",Nom);

  retour = mkfifo (nomPipe, 0777);
  if((0==retour)||(17==errno))
  {
      retour = open (nomPipe, O_RDWR);
  }
  else
  {
	  printf("Erreur de creation de %s\n",nomPipe);
	  printf("SysPipCreer : erreur : %d\n",errno);
	  perror("Blup");
  }
  /* --------------------------------
   * FIN DE SysPipeCreer
   * --------------------------------     */
  return retour;
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
void SysPipeDetruire (int fd)
{


  close (fd);

  return;
}

/* --------------------------------
 * SysPipeEmettre
 * ==============
 * Emission d'un nouveau message.
 * --------------------------------	*/

int SysPipeEmettre (int fd, char *pMess, unsigned short Longueur)
{
  int retour = 0;
  unsigned int nbCarAEcrire = (unsigned int) Longueur;
  int nbCarEcrits = 0;
  /* ---------------------------------
   * COMBIEN DE MESSAGES DANS LA FILE
   * --------------------------------     */
  while ((nbCarAEcrire > 0) && (ERROR != retour))
    {
      retour = write (fd, &pMess[nbCarEcrits], nbCarAEcrire);
      if (ERROR != retour)
        {
          nbCarAEcrire -= retour;
          nbCarEcrits += retour;
        }
      else
        {
          nbCarEcrits = ERROR;
          printDebug ("Il y a erreur\n");
        }                              /* endif(ERROR!=retour)                         */
    }                                  /* while ((nbCarAEcrire > 0)&&(ER       */
  printDebug ("J'ai fini d'ecrire %d\n", fd);
  /* --------------------------------
   * FIN DE SysPipeEmettre
   * --------------------------------     */
  return nbCarEcrits;
}

/* --------------------------------
 * SysPipeEmettreAvecSusp
 * ======================
 * Emission d'un message avec controle
 * a priori du nombre de messages dans
 * le pipe.
 * Pas de bloquage a l'emission pour
 * l'instant.
 * --------------------------------	*/

int
SysPipeEmettreAvecSusp (int fd, int maxMess, char *pMess,
                        unsigned short Longueur)
{
  int retour = 0;
  unsigned int nbCarAEcrire = (unsigned int) Longueur;
  int nbCarEcrits = 0;
  /* ---------------------------------
   * COMBIEN DE MESSAGES DANS LA FILE
   * --------------------------------     */
  while ((nbCarAEcrire > 0) && (ERROR != retour))
    {
       retour = write (fd, &pMess[nbCarEcrits], nbCarAEcrire);
          if (ERROR != retour)
            {
              nbCarAEcrire -= retour;
              nbCarEcrits += retour;
            }
          else
            {
              nbCarEcrits = ERROR;
              printDebug ("Il y a erreur\n");
            }                          /* endif(ERROR!=retour)                         */
        }                              /* endif(ERROR!=totalEcrit                      */
  printDebug ("J'ai fini d'ecrire %d\n", fd);
  /* --------------------------------
   * FIN DE SysPipeEmettre
   * --------------------------------     */
  return nbCarEcrits;
}

/* --------------------------------
 * SysPipeEmettreSansSusp
 * ======================
 * Emission d'un message avec controle
 * a priori du nombre de messages dans
 * le pipe et sortie sans attente.
 * --------------------------------	*/

int SysPipeEmettreSansSusp (int fd, int maxMess, char *pMess,
                        unsigned short Longueur)
{
	return SysPipeEmettreAvecSusp(fd,maxMess,pMess,Longueur);
}

/* --------------------------------
 * SysPipeLire
 * ===========
 *
 * --------------------------------	*/

int SysPipeLire (int fd, char *ptMess, int Longueur)
{
  int Status = read (fd, ptMess, Longueur);
  printDebug ("J'ai lu %d message %s longueur %d\n", fd, ptMess, Status);
  return Status;
}
