/*
 * ypc_las.h
 * ===================
 *
 *  Created on: 23 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: ypc_las.h,v $
 * Revision 1.2  2018/06/04 08:38:41  xg
 * Passage en version V122
 *
 * Revision 1.5  2008/07/02 15:07:16  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
#ifndef _YPC_LAS_H
#define _YPC_LAS_H

void tac_las_ctrl (void);
void tac_las (int);
void tac_las_init_cfg (void);
void tac_las_init_cfg_las (INT);
void monMessSendLas (INT las, mqd_t pt_anc, void *pt_mes, unsigned Longueur);
void tac_las_init(void);
void tac_las_demarre(int port);
int tac_las_send_direct(int i, char *buffer, int nbCar);


#endif

/* *********************************
 * FIN DE yps_las.h
 * ********************************* */
