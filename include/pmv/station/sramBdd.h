#ifndef SRAM_BDD_H
#define SRAM_BDD_H

/* ********************************	*
 * DEFINITION DES CONSTANTES
 * ********************************	*/

/* ************************************
 * DEFINITION DES TYPES LOCAUX
 * ************************************	*/

/* ************************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ************************************	*/
int32 sramBddInit(void *adresseMessages_p,int32 tailleMax);
bool sramBddEcrireFichier(uint32 numFichier_udw,void *fichier_p);
bool sramBddLireFichier(uint32 numFichier_udw,void *fichier_p);
void sramBddLireTout(void *fichier_p);
void sramBddEcrireTout(void *fichier_p);
int32 sramBddTailleGet(void);
void sramBddTailleSet(int32 taille_dw);
void sramBddLireEnreg( int32 dest,int taille,void *info_p);
void sramBddEcrireEnreg( int32 dest,int taille,void *info_p);

#endif /*SRAM_BDD_H*/
