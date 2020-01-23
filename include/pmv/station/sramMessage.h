#ifndef CONFIGURATION_MESSAGE_H_
#define CONFIGURATION_MESSAGE_H_

/* ********************************	*
 * DEFINITION DES CONSTANTES
 * ********************************	*/

/* ************************************
 * DEFINITION DES TYPES LOCAUX
 * ************************************	*/

/* ************************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ************************************	*/
int32 sramMessageInit(void *adresseMessages_p);
bool sramMessageEcrireCaisson(uint32 numCaisson_udw,void *caisson_p);
bool sramMessageLireCaisson(uint32 numCaisson_udw,void *caisson_p);
void sramMessageLireTout(void *caisson_p);
void sramMessageEcrireTout(void *caisson_p);

#endif /*CONFIGURATION_H_*/
