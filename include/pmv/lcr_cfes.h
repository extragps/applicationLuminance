/*
 * lcr_cfes.h
 *
 *  Created on: 16 juin 2008
 *      Author: xavier
 */

#ifndef LCR_CFES_H_
#define LCR_CFES_H_

int lcr_cfes_init (void);

INT lcr_cfes (INT, INT, INT, STRING, INT, struct usr_ztf *, INT, INT *);
int cfes_chercher_type(int,int,STRING,int);
int cfes_chercher(int,int,STRING);
int cfes_chercher_valeur(int module, int caisson, BYTE typeSymb, BYTE typeEquiv, UINT valeur);
void cfes_imprimer(int);
void cfes_imprimer_tout(void);
int cfes_trouver_type (int module, int caisson, STRING equiv, int type);


#endif /* LCR_CFES_H_ */
