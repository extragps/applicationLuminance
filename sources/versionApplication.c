/*
 * versionApplication.c
 * ===================
 *
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: versionApplication.c,v $
 * Revision 1.3  2018/06/14 08:37:10  xg
 * Passage en version 1.23
 *
 * Revision 1.2  2018/06/04 08:38:39  xg
 * Passage en version V122
 *
 * Revision 1.7  2015/06/09 07:56:43  xg
 * Passage en version 1.21
 *
 * Revision 1.6  2015/06/04 15:20:50  xg
 * Passage en version 1.20
 *
 * Revision 1.5  2010/09/15 16:31:42  xgaillard
 * Version 1.19 suite aux adaptation pour la reponse su l'etat d'affichage en cas de sabordage.
 *
 * Revision 1.4  2010/08/05 14:37:09  xgaillard
 * Passage en version 1.18
 *
 * Revision 1.3  2009/09/28 08:07:18  xgaillard
 * Passage en version 1.17 pour intégrer le soucis de comm
 *
 * Revision 1.2  2009/08/24 08:24:20  xgaillard
 * Deplacement des fonctions de lecture de la version dans versionApplication.c
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include "portage.h"



/* Informations a modifier pour changer les parametres de version.... */

static int majeure = 1;
static int mineure = 23;

int versionLireMajeure(const char *name)
{
	return majeure;
}
int versionLireMineure(const char *name)
{
	return mineure;
}

/* *********************************
 * FIN DE versionApplication.c
 * ********************************* */
