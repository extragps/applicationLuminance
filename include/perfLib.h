/*
 * perfLib.h
 * ===================
 *
 *  Created on: 24 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: perfLib.h,v $
 * Revision 1.2  2018/06/04 08:38:40  xg
 * Passage en version V122
 *
 * Revision 1.3  2008/07/02 15:07:17  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

#ifndef _PERF_LIB_H
#define _PERF_LIB_H

#define PERF_NB_FOIS_MAX 20

/* ********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* --------------------------------
 * T_perfElem
 * ==========
 * Structure de donnees utilisee pour
 * comptabiliser les donnees de temps
 * passe.
 * --------------------------------	*/

typedef struct T_perfElem
{
  unsigned long tickDebut;
  int nbFois;
  ULONG moyenne;
  ULONG min;
  ULONG max;
  ULONG cour;
} T_perfElem;

void perfImprimer (T_perfElem * elem);
void perfInit (T_perfElem * elem);
void perfDemarrer (T_perfElem * elem);
void perfArreter (T_perfElem * elem);

#endif

/* *********************************
 * FIN DE perfLib.h
 * ********************************* */
