/*
 * lcr_tst.h
 * ===================
 *
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_tst.h,v $
 * Revision 1.2  2018/06/04 08:38:42  xg
 * Passage en version V122
 *
 * Revision 1.4  2008/10/03 16:21:45  xag
 * *** empty log message ***
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

#ifndef INCLUDE_LCR_TST
#define INCLUDE_LCR_TST
void lcr_tst_tst (INT, INT, INT, STRING, INT, struct usr_ztf *);
int lcr_tst_eth(INT las,INT mode,INT lg_mess,STRING buffer,INT position, struct usr_ztf *pt_mess);
int lcr_tst_debug(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess);
void lcr_tst_cnf(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess);
void lcr_cmd_tst_print_version(void);

#endif

/* *********************************
 * FIN DE lcr_tst.h
 * ********************************* */
