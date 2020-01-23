#ifndef CONFIGURATION_PARAM_H_
#define CONFIGURATION_PARAM_H_

#define CONFIG_SEPARATORS "=:"
#define CONFIG_SEP_PARAM '.'
#define CONFIG_TAILLE_MESSAGE 256

/* ************************************
 * DEFINITION DES TYPES LOCAUX
 * ************************************	*/

typedef enum _ConfigType
{
	CONFIG_BOOL,
	CONFIG_LONG,
	CONFIG_CHAINE,
	CONFIG_LISTE_LONG,
	CONFIG_MAX_TYPE,
} ConfigType;

typedef struct _ConfigParam
{
	char *nomParam_pc;
	ConfigType type_e;
	char *comment_pc;
	bool (*setter)();
	void (*getter)();
}	ConfigParam;


/* Methodes pour le parsing d'une ligne de parametre. */
bool configParseLigne(char *param_pc,char *value_pc, const char * ligne_pc ) ;
bool configParseLigneParam(char *param_pc,const char * ligne_pc ) ;
char *configParamGet(char *param_ac);
bool configParamTraiter(char *,char *);
void configParamFirst(char *tableau,int longueur);
void configParamNext(char *tableau,int longueur);


#endif /*CONFIGURATION_H_*/
