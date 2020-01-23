/*
 * supRun.c
 * ===================
 *
 *  Created on: 23 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: supRun.c,v $
 * Revision 1.2  2018/06/04 08:38:38  xg
 * Passage en version V122
 *
 * Revision 1.6  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.5  2008/10/03 16:21:45  xag
 * *** empty log message ***
 *
 * Revision 1.4  2008/09/29 07:58:51  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.3  2008/09/22 07:53:42  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/09/12 14:57:09  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:03  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* **********************************
 * FICHIERS D'INCLUSION
 * ********************************** */

#include "Superviseur.h"
#include "Tache.h"
#include "standard.h"
#include "supRun.h"
#include "stAlLib.h"
#include "perfLib.h"
#include "mon_debug.h"
#include "mon_str.h"
#include "mon_pro.h"
#include "semLib.h"
#include "spcLib.h"
#include "etaSyst.h"
#include "pip_def.h"

/* ********************************	*/
/* DEFINITION DES CONSTANTES		*/
/* ********************************	*/

#define DEBUG 0
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* *********************************
 * VARIABLES LOCALES
 * ********************************	*/

static SEM_ID supRunExclusion = NULL;

static T_perfElem table[SUP_MAX_TACHE];
static T_perfElem total;
static T_perfElem reste;


/* *********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * supRunInit
 * ==========
 * Initialisation des semaphores qui
 * sont utilises pour demarrer et
 * arreter les taches applicatives.
 * -------------------------------- */

void supRunInit (void)
{
  supGestion.appli = 0;
  supRunExclusion = semBCreateNamed (SEM_Q_FIFO, SEM_EMPTY,"Exclusion");
}

/* --------------------------------
 * supRunDemarrer
 * ==============
 * Lancement des taches applicatives
 * --------------------------------	*/

int supRunDemarrer (void)
{
  int retour = semTake (supRunExclusion);
  if(ERROR!=retour)
  {
      supGestion.appli = 1;
      pipSetEtat(PIP_AFFICHAGE);
      semGive (supRunExclusion);
    }
  return retour;
}

int supRunArreter (void)
{
  int retour = semTake (supRunExclusion);
  if(ERROR!=retour)
  {
      supGestion.appli = 0;
      pipSetEtat(PIP_VEILLE);
#ifndef SANS_WORLDFIP
		if (fipMediumGetOk())
		{
			pipSetEtat(pipGetEtat() | PIP_COM);
		} else
		{
			pipSetEtat(pipGetEtat() & (~PIP_COM));
		}
#endif
      semGive (supRunExclusion);
    }
  return retour;
}


/* ---------------------------------
 * supRunLireEtat
 * =============
 * Lecture de l'etat du flag appli.
 * --------------------------------	*/

int supRunLireEtat (void)
{
  int retour = semTake (supRunExclusion);
  if (ERROR != retour)
    {
      /* On vide eventuellement le sema-
       * phore signal.        */
      retour = supGestion.appli;
      semGive (supRunExclusion);
    }
  return retour;
}

/* --------------------------------
 * supIncrement
 * ============
 * Increment periodique des compteurs
 * rqtemp.
 * --------------------------------	*/

void supIncrement (int numero)
{
  int notFin = 1;
  int nbMsCour=1000;
  int nbMsDixCour=100;

  while (notFin)
    {
    	/* La tache de traitement periodique etait calee pour etre effectuee tous
    	 * les 60 millisecondes, ainsi, on bricole pour realiser des traitements tous les
    	 * 100 millis et toutes les secondes. */
	  mnTraitPeriodique ();
	  ksleep(62);
	  nbMsCour-=62;
	  nbMsDixCour-=62;
      if(nbMsDixCour<0)
      {
      	nbMsDixCour+=100;
         stAlDecompter ();
      }
      if(nbMsCour<0)
      {
          spcTester ();
          spcTrmTester ();
          nbMsCour+=1000;
      }
      etaSystIncrCompteur (numero);
    }
  /* -----------------------------
   * FIN DE supIncrement
   * --------------------------------     */
}

/* --------------------------------
 * supRun
 * ======
 * Fonction assurant le sequecement
 * des taches de l'application puis
 * le calcul de l'etat de fonction-
 * nement qui est communique e la
 * tache de gestion du chien de garde.
 * --------------------------------	*/

int supRun (T_supGestion * pGes, int typeBoot)
{                                      /* --------------------------------
                                        * DECLARATION DES VARIABLES
                                        * --------------------------------     */
  int indice;
  int retour = OK;
  int notFin = 1;
  unsigned long tickCour=ktickGetCurrent();
  unsigned long tickLast=tickCour;
  for (indice = 0; indice < SUP_MAX_TACHE; indice++)
    {
      perfInit (&table[indice]);
    }
  perfInit (&total);
  perfInit (&reste);

  /* Attention de ne pas declencher le
   * WatchDog.
   */
  perfDemarrer (&total);
  while (notFin)
    {
      /* si les taches sont en fonctionne-
       * ment, on les lance les unes apres
       * les autres.  */
      perfArreter (&total);
      perfDemarrer (&total);
      if (1 == supRunLireEtat ())
        {
          for (indice = 0; indice < pGes->nbTache; indice++)
            {
              T_supDescTache *tache = &pGes->Tache[indice];
              if (tache->sequence)
                {
                  perfDemarrer (&table[indice]);
                  retour = semSignal (tache->monVersTac);
                  if (retour == ERROR)
                    {
                      if (tache->monVersTac == NULL)
                        {
                          printDebug ("Le semaphore a ete supprime\n");
                        }
                      else
                        {
                          return ERROR;
                        }
                    }
#ifndef CPU432
				  /* Sur la CPU 432, il n'y a pas besoin de realiser cette temporisation. */
                  ksleep(1);
#endif
                  retour = semWait (tache->tacVersMon);
                  if (retour == ERROR)
                    {
                      if (tache->tacVersMon == NULL)
                        {
                          printDebug ("Le semaphore a ete supprime\n");
                        }
                      else
                        {
                          return ERROR;
                        }
                    }
                  perfArreter (&table[indice]);
                }                      /* endif(tache->sequence)         */
//              ksleep(100);
            }                          /* endfor(i=0                     */
        }
      else
        {
        	ksleep(50);
        }                              /* endif(1==supRunLireEtat()      */
      /* --------------------------------     */
      /* SIGNALEMENT POUR WATCH DOG           */
      /* --------------------------------     */
      perfDemarrer (&reste);
      /*------------------------------------
	* TRAITEMENT DES ALERTES
	* --------------------------------	*/
      if (1 == supRunLireEtat ())
        {
          trt_alt ();
        }                              /* endif(pGes->appli==1                         */
      perfArreter (&reste);


      if (monDebugGet (MON_DEBUG_PERF_SUP))
        {
      tickCour=ktickGetCurrent();
      if((tickCour-tickLast)>10000)
      {
      	tickLast=tickCour;
              if (1 == supRunLireEtat ())
                {
                  for (indice = 0; indice < pGes->nbTache; indice++)
                    {
                      T_supDescTache *tache = &pGes->Tache[indice];
                      if (tache->sequence)
                        {
                          printf ("\nTache %2d et nom %-20s (%7d/%d) : ",
                                  indice, tache->nomTache,tache->bidon,tache->position);
                          perfImprimer (&table[indice]);
                          perfInit (&table[indice]);
                        }
                        else
                        {
                          printf ("\nTache %2d et nom %-20s (%7d/%d)",
                                  indice, tache->nomTache,tache->bidon,tache->position);
                        }
                    }
                  printf ("\nReste --------------- ");
                  perfImprimer (&reste);
                  perfInit (&reste);
                  printf ("\nTotal --------------- ");
                  perfImprimer (&total);
                  perfInit (&total);
                  printf ("\n");
                }
                else
                {
                    printf("Mode systeme debut ***************\n");
                   for (indice = 0; indice < pGes->nbTache; indice++)
                    {
                      T_supDescTache *tache = &pGes->Tache[indice];
                      if (!tache->sequence)
                        {
                          printf ("\nTache %2d et nom %10s (%7d/%d)",
                                  indice, tache->nomTache,tache->bidon,tache->position);
                        }
                    }
                    printf("\nMode systeme fin ***************\n");
                }
                /* Trace des allocations memoire */
                {
                int32 allocSize_dw,allocNb_dw;
                int32 maxSize_dw,maxNb_dw;
                	kmmGetStatus(NULL,&allocSize_dw,&allocNb_dw,&maxSize_dw,&maxNb_dw);
                    printf("Allocations **** %8ld/%8ld **** MAX %8ld/%8ld ******\n",
                    	allocSize_dw,allocNb_dw,maxSize_dw,maxNb_dw);
                    printf("Err 485 **** %8ld/%8ld **** \n",
                    	lumMsgCreateurGetErr(),lumTraiteurGetErr());
                }
            }
        }

    }                                  /* endwhile(notFin                                      */
  /* --------------------------------     */
  /* FIN DE supRun                                        */
  /* --------------------------------     */
  return retour;
}
