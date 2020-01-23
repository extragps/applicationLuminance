/*
 * lcr_st1.h
 * ===================
 *
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_st1.h,v $
 * Revision 1.2  2018/06/04 08:38:40  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:07:18  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

#ifndef LCR_ST1_H_
#define LCR_ST1_H_

extern INT st1_st (INT, INT, INT, char *, INT, struct usr_ztf *);
void lcr_st1_evt (char * evt);

/* ********************************
 * FIN DE lcr_st1.h
 * ********************************	*/

#endif /* LCR_ST1_H_ */
