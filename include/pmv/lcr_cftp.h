/*
 * lcr_cftp.h
 * 		Le fichier contient les definitions et prototypes utilisés pour la commande lcr CFTP
 *
 *  Created on: 17 juin 2008
 *      Author: xavier
 */


#ifndef INCLUDE_LCR_CFTP
#define INCLUDE_LCR_CFTP

/* *************************************
 * DEFINITIONS DU MODULE
 * ************************************* */

#define LCR_CFTP_MODULE "AM="
#define LCR_CFTP_TYPE "TYP="
#define LCR_CFTP_LUMINOSITE "LUM="
#define LCR_CFTP_SABORDAGE "SAB="
#define LCR_CFTP_CARTE "CRT="
#define LCR_CFTP_VALEUR "VAL="
#define LCR_CFTP_INFO "INF="


/* *************************************
 * PROTOTYPE DES FONCTIONS
 * ************************************* */
INT lcr_cftp (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
void lcr_cftp_init (bool);
void lcr_cftp_arborescence (void);


/* Fonctions du fichier LCRCFTP1.c */

int caisChercher (int, int);
int caisChercherIndex (int indexModule, int numCaisson);
int caisGetIndex (int numModule, int numCaisson);
int modChercherIndex (int numModule);
int modGetIndex (int numModule);
void lcr_cftp_cmd_exec(struct cmd_cftp * cmd_cftp);
void lcr_cftp_cmd_init(struct cmd_cftp * cmd_cftp);

/* *********************************
 * FIN DE lcr_cftp.h
 * ********************************* */

#endif
