/*
 * lcr_temp.h
 * ===================
 *
 *  Created on: 3 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_temp.h,v $
 * Revision 1.2  2018/06/04 08:38:41  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/04 16:51:41  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 * Creation car il n'y avait pas le prototype de la fonction.
 *
 */
#ifndef _INCLUDE_LCR_TEMP
#define _INCLUDE_LCR_TEMP
int lcr_temp(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess);

#endif

/* *********************************
 * FIN DE lcr_temp.h
 * ********************************* */
