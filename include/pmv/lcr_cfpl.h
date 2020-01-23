/*
 * lcr_cfpl.h
 * ===================
 *
 * Objet : Prototypes des fonctions de gestion de police.
 *
 * --------------------------------------------------------
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_cfpl.h,v $
 * Revision 1.2  2018/06/04 08:38:41  xg
 * Passage en version V122
 *
 * Revision 1.7  2008/07/04 17:05:59  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.6  2008/07/02 15:29:16  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

#ifndef INCLUDE_LCR_CFPL
#define INCLUDE_LCR_CFPL
INT lcr_cfpl (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
void lcr_cfpl_init ();
#endif

/* *********************************
 * FIN DE lcr_cfpl.h
 * ********************************* */
