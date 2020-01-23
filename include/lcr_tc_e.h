/*
 * lcr_tc_e.h
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_tc_e.h,v $
 * Revision 1.2  2018/06/04 08:38:40  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:07:16  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

#ifndef _LCR_TC_E
#define _LCR_TC_E
int lcr_tc_e(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess, INT flg_fin,
		INT * bloc);
#endif
/* *********************************
 * FIN DE lcr_tc_e.h
 * ********************************* */
