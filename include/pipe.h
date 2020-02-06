/*
 * pipe.h
 * ===================
 *
 *  Created on: 24 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: pipe.h,v $
 * Revision 1.2  2018/06/04 08:38:40  xg
 * Passage en version V122
 *
 * Revision 1.4  2008/07/02 15:07:16  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
#ifndef PIPE_H
#define PIPE_H

/* ********************************
 * DEFINITION DES INCLUDES
 * ********************************	*/

#ifdef VXWORKS
#include "vxworks.h"
#include "io/iolib.h"
#include "pipeDrv.h"
#endif
#include "stdio.h"
#include "errno.h"
#include "standard.h"

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define PIPE_NB_MESS	5

/* ********************************
 * PROTOTYPE DES FONCTIONS
 * ********************************	*/

int SysPipeCreer (char *Nom, INT maxMsgs, INT maxLength);
void SysPipeDetruire (int fd);
int SysPipeEmettre (int fd, char *pMess, unsigned short Longueur);
int SysPipeEmettreAvecSusp (int, int, char *, unsigned short);
int SysPipeEmettreSansSusp (int, int, char *, unsigned short);
int SysPipeLire (int fd, char *ptMess, int Longueur);

/* ********************************
 * FIN DE pipe.h
 * ********************************	*/

#endif
