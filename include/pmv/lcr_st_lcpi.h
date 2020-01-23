/*
 * lcr_st_lcpi.h
 *
 * 	Prototypes des methodes du fichier lcr_st_lcpi.c. Contient les fonctions de gestion des
 * donnees utilisateur dont je ne sais pas trop a quoi elles peuvent etre utilisees.
 *
 *  Created on: 19 juin 2008
 *      Author: xavier
 */

#ifndef LCR_ST_LCPI_H_
#define LCR_ST_LCPI_H_

/* **********************************
 * PROTOTYPE DES FONCTIONS DU MODULE
 * ********************************** */

void lcr_st_lcpi_init();
int lcr_st_lcpi_modif(char *usr1,char *usr2);
INT lcr_st_lcpi (INT las, INT mode, INT lg_mess, STRING buffer, INT position,T_usr_ztf * pt_mess);

#endif /* LCR_ST_LCPI_H_ */
