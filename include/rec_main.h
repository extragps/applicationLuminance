/*
 * rec_main.h
 * ===================
 *
 *  Created on: 23 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: rec_main.h,v $
 * Revision 1.2  2018/06/04 08:38:39  xg
 * Passage en version V122
 *
 * Revision 1.5  2008/07/02 15:07:16  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
#ifndef _REC_MAIN_H
#define _REC_MAIN_H
VOID rec_main_init (int);
VOID rec_main (int);
VOID rec_alarme (VOID);
VOID rec_main_valid_config (VOID);
VOID rec_initsimul (VOID);
VOID rec_simul (VOID);
#endif
/* *********************************
 * FIN DE rec_main.h
 * ********************************* */
