#ifndef INCLUDE_TEMP_LIB
#define INCLUDE_TEMP_LIB

/* **********************************
 * DEFINITION DES TYPES
 * ********************************	*/

/* --------------------------------
 * T_tempInfos
 * ===========
 * Informations relatives au controle
 * de la temperature.
 * --------------------------------	*/

typedef struct T_tempInfos
{
	int alerte;
	int hyster;
}	T_tempInfos;

/* **********************************
 * PROTOTYPAGE DES FONCTIONS
 * ********************************	*/

void tempInit ();
int tempGetAlerte();
void tempSetAlerte(int val);
int tempDecTempo();
void tempSetTempo(int val);
void tempSetValeur(int val);
int tempGetValeur(void);

/* **********************************
 * FIN DE tempLib.h
 * ********************************	*/

#endif
