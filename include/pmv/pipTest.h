/*
 * pipTest.h
 * ===================
 *
 *  Created on: 25 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: pipTest.h,v $
 * Revision 1.2  2018/06/04 08:38:41  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:29:14  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

#ifndef PIPTEST_H_
#define PIPTEST_H_


void 	pipLonTestDiodes(void);
void 	pipLonInitTest(void);
int 	pipLonGetTest();
void 	pipLonSetTest(int val);

/* ********************************
 * FIN DE pipTest.h
 * ********************************	*/

#endif /* PIPTEST_H_ */
