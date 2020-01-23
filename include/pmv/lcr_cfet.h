/*
 * lcr_cfet.h
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_cfet.h,v $
 * Revision 1.2  2018/06/04 08:38:41  xg
 * Passage en version V122
 *
 * Revision 1.3  2008/07/04 17:05:59  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.2  2008/07/02 15:29:15  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
#ifndef INCLUDE_LCR_CFET
#define INCLUDE_LCR_CFET
int lcr_cfet_init (void);
INT lcr_cfet (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
#endif
/* *********************************
 * FIN DE lcr_cfet.h
 * ********************************* */
