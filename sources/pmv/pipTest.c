/*
 * pipTest.c
 * ===================
 *
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: pipTest.c,v $
 * Revision 1.2  2018/06/04 08:38:43  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:30:15  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */


/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include "standard.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "mon_def.h"
#include "mon_ext.h"
#include "pipTest.h"

/* *************************************
 * VARIABLES LOCALES DU MODULE
 * ************************************* */

/* La variable est utilisee pour stocker le
 * numero de test courant.
 */
static int pipNumTest=0;

/* *************************************
 * FONCTIONS DU MODULE
 * ************************************* */

void 	pipLonTestDiodes(void)
{
  	rqtemp[TP_TEST_DIODES]=2;
}

void 	pipLonInitTest(void)
{
  if(pip_cf_tst_sc.autotests)
  {
	/* La tempo des autotests est exprimee en minutes. */
  	rqtemp[TP_TEST_DIODES]=pip_cf_tst_sc.autotests*60*8;
  }
  else
  {
  	rqtemp[TP_TEST_DIODES]=TP_HS;
  }
}

/**
 * pipLonGetTest
 * =============
 * Recuperation du numero de test.
 */
int 	pipLonGetTest()
{
	return pipNumTest;
}

/**
 * pipLonSetTest
 * =============
 * Positionnement du numero de test.
 * @param val le numero du test.
 */
void 	pipLonSetTest(int val)
{
	pipNumTest=val;
}

/* ********************************
 * FIN DE pipTest.c
 * ********************************	*/

