/*
 * file_name
 * ===================
 * Objet	: Gestion du bouton poussoir pour le test du panneau.
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: bpTestLib.c,v $
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.7  2009/01/09 16:23:04  xag
 * Correction du probleme de voyant affiche sur timeout.
 *
 * Revision 1.6  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.5  2008/10/03 16:21:47  xag
 * *** empty log message ***
 *
 * Revision 1.4  2008/09/29 07:58:52  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.3  2008/09/22 07:53:42  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/09/12 14:57:10  xag
 * *** empty log message ***
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <stdio.h>
#include "define.h"
#include "vct_str.h"
#include "mon_def.h"
#include "mon_debug.h"
#include "mon_str.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "bpTestLib.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "portage.h"
#include "pipTest.h"
#include "lcr_p.h"
#include "mgpLib.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define BP_TEST_NB_MESS 4

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static int bpEtatPrec = 0;
static int bpCpt = 0;
static int bpCptPrec = 0;       /* Compteur precedent */
static int bpMin = 10;         /* Nombre de cycle a etat stable pour valider. */
static int bpTestFrequence = 10000;
static int bpVoyantFrequence = 500;
static int bpMessTimeout = 5000; /* Exprime en tick systeme */
static int bpVoyantTimeout = 500; /* Exprime en tick systeme */
static int bpVoyantIndex = 0;
static int bpMessIndex = 0;
static int bpTimeoutValue = 600000;
static int bpTimeout = 1000;
static int bpMessTable[BP_TEST_NB_MESS] = {
  PIP_TEST_BP1, PIP_TEST_BP2, PIP_TEST_BP3, PIP_TEST_BP4
};
static ULONG bpMessTick = 0;
static ULONG bpVoyantTick = 0;
static ULONG bpTestTick = 0;
static ULONG bpEtatTick = 0;
static ULONG bpTestMode = 0;

/* ********************************
 * FONCTIONS DU MODULE.
 * ********************************	*/

static int _bpTestChangementMessage (ULONG tickCour);
static int _bpTestTimeout (ULONG tickCour);
static int _bpTestChangementVoyant (ULONG tickCour);
static int _bpTestIncrementer (void);
static int etatCour=0;



/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

int setEtatCour(int val)
{
	etatCour=val;
	return etatCour;
}

int getEtatCour(void)
{
	return etatCour;
}

static void bpTestSetMode(int mode)
{
		if(mode!=bpTestMode)
		{
			if(0==mode)
			{
				eriSupprimer(E_eriTest,ERI_TST_TEST_EN_COURS);
			}
			else
			{
				eriAjouter(E_eriTest,ERI_TST_TEST_EN_COURS);
			}
			bpTestMode=mode;
		}
		if(-1!=pip_cf_tst_sc.tc_bp_led)
		{
		   mgpPositionnerSortie(pip_cf_tst_sc.tc_bp_led,mode!=0);
		}
}

int bpGetTimeout ()
{
  return bpTimeout;
}

void printBp ()
{
  printDebug ("Etat %d, Cpt %d/%d, Timeout %d/%d\n",
              bpEtatPrec, bpCpt, bpMin, bpTimeout, bpTimeoutValue);
}

/* --------------------------------
 * bpTestInit
 * ==========
 * Initialisation de la scrutation
 * test.
 * --------------------------------	*/

int bpTestInit (void)
{
int etat=0;
ULONG tickCour;
	if(-1!=pip_cf_tst_sc.ts_bp_test)
	{
		etat=mgpLireEntree(pip_cf_tst_sc.ts_bp_test);
		if(-1!=etat)
		{
			bpEtatPrec=etat;
		}
	}
	tickCour=tickGet();
	/* Initialisation des ticks. */
	bpTestTick=tickCour;
	bpVoyantTick=tickCour;
	bpMessTick=tickCour;
	bpEtatTick=tickCour;

	bpTestSetMode(0);
	return etat;
}

/* --------------------------------
 * bpTest
 * ======
 * Test du bouton poussoir. La methode
 * doit etre appelee periodiquement pour
 * controler l'activation ou la desactivation
 * du bouton. On considere qu'il y a activation si
 * le bouton reste dans le meme etat pendant
 * bpMin cycles.
 * --------------------------------	*/

int bpTest ()
{
int retour = 0;
int etat=0;
	/* Attention, pas de mode lorsque le panneau est deleste. */
	if(0==(pipGetEtat()&PIP_DELESTE))
	{
	if(-1!=pip_cf_tst_sc.ts_bp_test)
	{
		etat=mgpLireEntree(pip_cf_tst_sc.ts_bp_test);
		if(-1==etat)
		{
			etat=0;
		}
	}
  if (etat != bpEtatPrec)
    {
      /* On regarde a ce que l'etat precedent soit valide e savoir que
       * le nombre de cycles dans l'etat precedent soit suffisant pour$
       * valider le cycle. */
      /* Il y a activation du bouton, si le temps ecoule depuis le
       * precedent appui n'est pas ecoule, on repart avec la valeur
       * precedent du compteur */
      if (bpCpt < bpMin)
        {
          bpCpt = bpCptPrec;
        }
      else
        {
          bpCptPrec = bpCpt;
          bpCpt = 0;
          printDebug ("bpTest, bpCpt annule\n");
        }
      bpEtatPrec = etat;
    }
  else
    {
      ULONG tickCour = tickGet ();
      if (bpEtatTick != tickCour)
        {
          if (bpEtatTick < tickCour)
            {
              int deltaTick = tickCour - bpEtatTick;
              if ((bpCpt < bpMin) && ((bpCpt + deltaTick) >= bpMin)
                  && (etat == 1))
                {
                  /* L'action est validee, il faut proceder au changement
				   * d'etat */
                  retour = _bpTestIncrementer ();
                }
              bpCpt += deltaTick;
            }
          bpEtatTick = tickCour;
        }
    }
	} /* endif(0==(pipGetEtat()&PIP_DELESTE)) */
  /* Controle du changement eventuel de message */
  if (0 != bpTestMode)
    {
      ULONG tickCour = tickGet ();
      if (bpTestMode == 2)
        {
          retour |= _bpTestChangementMessage (tickCour);
        }
      _bpTestChangementVoyant(tickCour);
      retour |= _bpTestTimeout (tickCour);
    }


  return retour;
}

/* ---------------------------------
 * bpTestChangementMessage
 * =======================
 * La methode a pour objet de changer
 * de message de test periodiquement.
 * La fonction retourne 1 si il est
 * necessaire de reafficher.
 * --------------------------------	*/

static int _bpTestChangementMessage (ULONG tickCour)
{
  int retour = 0;

  if (2 == bpTestMode)
    {
      if (tickCour > bpMessTick)
        {
          bpMessTimeout -= tickCour - bpMessTick;
          if (bpMessTimeout < 0)
            {
              bpMessTimeout = bpTestFrequence;
              bpMessIndex = (bpMessIndex + 1) % BP_TEST_NB_MESS;
              retour = 1;
            }
        }
    }
  bpMessTick = tickCour;
  return retour;
}

static int _bpTestChangementVoyant (ULONG tickCour)
{
  int retour = 0;

  if ( 0!= bpTestMode)
    {
      if (tickCour > bpVoyantTick)
        {
          bpVoyantTimeout -= tickCour - bpVoyantTick;
          if (bpVoyantTimeout < 0)
            {
              bpVoyantTimeout = bpVoyantFrequence;
              /* Incrementer ici le numero du test */
              /* Eteindre le voyant précedent */
              mgpPositionnerSortie(bpVoyantIndex,false);
              bpVoyantIndex=(bpVoyantIndex+1)%6;
              mgpPositionnerSortie(bpVoyantIndex,true);
              retour = 1;
            }
        }
    }
  bpVoyantTick = tickCour;
  return retour;
}

/* --------------------------------
 * bpTestTimeout
 * =============
 * La fonction a pour objet de controler
 * le timeout en cas de passage en
 * mode test.
 * Sur ecoulement de la duree de timeout,
 * il faut repasser dans l'etat
 * d'affichage normal.
 * --------------------------------	*/

static int _bpTestTimeout (ULONG tickCour)
{
  int retour = 0;
  /* Le test n'est a realiser que lorsque le mode test
   * est active. */
  if (0 != bpTestMode)
    {
      if (tickCour > bpTestTick)
        {
          bpTimeout -= tickCour - bpTestTick;
          if (bpTimeout < 0)
            {
				printDebug("bpTestTimeout, le timeout a pete\n");
              bpTestSetMode(0);
              pipSetEtat (pipGetEtat()&(~PIP_TEST_BP));
      		  mgpPositionnerSortie(bpVoyantIndex,false);
 	  		  afficheursSetSortieDefauts(0);
              retour = 1;
            }
        }
    }
  bpTestTick = tickCour;
  return retour;
}

/* ---------------------------------
 * bpTestIncrementer
 * =================
 * Changement de phase de test.
 * --------------------------------	*/

static int _bpTestIncrementer (void)
{
  int retour = 0;
  bpTestSetMode( (bpTestMode + 1) % 3);
  printDebug ("Dans bpTestIncrementer : %d\n",bpTestMode);
  switch (bpTestMode)
    {
    case 1:
      /* Lancement du test sur les diodes. */
  	  printDebug ("Dans bpTestIncrementer : test des diodes\n");
      bpTimeout = bpTimeoutValue;      /* Relance du timeout */
	  pipLonTestDiodes();
      pipSetEtat (pipGetEtat () | PIP_TEST_BP);
      retour = 1;
      break;
    case 2:
      /* Lancement du test d'affichage . */
  	  printDebug ("Dans bpTestIncrementer : test affichage\n");
      pipSetEtat (pipGetEtat () | PIP_TEST_BP);
      bpTimeout = bpTimeoutValue;      /* Relance du timeout */
      bpMessTimeout = bpTestFrequence;
      bpMessIndex = 0;
      retour = 1;
      break;
    default:                          /* Retour e l'affichage normal */
      pipSetEtat (pipGetEtat () & (~PIP_TEST_BP));
      /* On force la sortie a 0 des defauts... */
      mgpPositionnerSortie(bpVoyantIndex,false);
 	  afficheursSetSortieDefauts(0);
      retour = 1;
      break;
    }                                  /* endswitch(bpTestMode) */
  return retour;
}

/* --------------------------------
 * bpTestGetMessage()
 * ==================
 * La fonction retourne le message
 * a afficher en fonction de l'index
 * courant du message.
 * --------------------------------	*/

int bpTestGetMessage ()
{
int val=PIP_TEST_DIODES;
	if(2==bpTestMode)
	{
  		val=bpMessTable[bpMessIndex];
	}
	return val;
}

/* ********************************
 * FIN DE bpTestLib.c
 * ********************************	*/
