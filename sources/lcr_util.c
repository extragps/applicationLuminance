/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 12/12/00
 * Fichier	: lcr_st_lcom.c
 * Objet	: Analyse et traitement de la commande ST,LCOM....
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,30Nov00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "portage.h"
#include "define.h"
#include "time.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "ted_prot.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * lcr_util_buffer_init
 * ====================
 * Initialisation d'un buffer.
 * Entree :
 * - un pointeur sur le buffer e
 *   initialiser,
 * - ne nombre max de caracteres.
 * Valeur de retour :
 * - la fonction retourne -1 si la
 * 	 valeur max est trop grande.
 * --------------------------------	*/

int lcr_util_buffer_init(T_lcr_util_buffer * buf, int max)
{
	int retour;
	if (LCR_UTIL_MAX_BUFFER < max)
	{
		retour = -1;
		buf->nbCarMax = LCR_UTIL_MAX_BUFFER;
	} else
	{
		retour = 0;
		buf->nbCarMax = max;
	}
	buf->guillemet=0;
	buf->nbCar = 0;
	/* ----------------------------------
	 * FIN DE lcr_util_buffer_init
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * lcr_util_buffer_recopier
 * =======================
 * Recopie sans censure dans le
 * buffer.
 * --------------------------------	*/

int lcr_util_buffer_tester_fin(T_lcr_util_buffer * buf)
{
	int retour = 0;
	if ((buf->nbCar == 1) && (buf->buffer[0] == '\\'))
	{
		retour = 1;
	} /* endwhile((buf->nbCar                         */
	/* --------------------------------
	 * FIN DE lcr_util_buffer_recopier
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * lcr_util_buffer_recopier
 * =======================
 * Recopie sans censure dans le
 * buffer.
 * --------------------------------	*/
int lcr_util_buffer_recopier(T_lcr_util_buffer * buf, char *mess, int nbCar)
{
	int retour = 0;
	int carCour = 0;
	while ((buf->nbCar < buf->nbCarMax) && (carCour < nbCar))
	{
		buf->buffer[buf->nbCar++] = mess[carCour++];
	} /* endwhile((buf->nbCar                         */
	/* --------------------------------
	 * FIN DE lcr_util_buffer_recopier
	 * --------------------------------     */
	return retour;
}

/* --------------------------------
 * lcr_util_buffer_ajouter
 * =======================
 * Ajout de caracteres dans un buffer
 * Les caracteres de retour de ligne
 * sont remplaces par des espaces.
 * --------------------------------	*/
int lcr_util_buffer_ajouter(T_lcr_util_buffer * buf, char *mess, int nbCar)
{
	int retour = 0;
	int carCour = 0;
	/* La variable guillemet permet de considerer le cas ou le
	 * point d'exclamation est dans un message.
	 */
	while ((buf->nbCar < buf->nbCarMax) && (carCour < nbCar))
	{
		if ('!' == mess[carCour])
		{
			/* L'acquit court positif n'est
			 * pas trace.   */
			/* Attention, il ne faut considere l'acquit que si l'on n'est
			 * pas dans un message. */
			if(0==buf->guillemet) {
				printDebug("J'ai un acquit positif\n");
				retour = 1;
				carCour++;
				break;
			} else  {
				buf->buffer[buf->nbCar++] = mess[carCour++];
			}
		} else if ('"' == mess[carCour]) /* Traitement du ENQ                            */
		{
			if( 0 == buf->guillemet )
			{
				/* On se trouve a l'interieur d'un message pmv */
				buf->guillemet=1;
			}
			else
			{
				/* Sortie du message pmv */
				buf->guillemet=0;
			}
			buf->buffer[buf->nbCar++] = mess[carCour++];
		} else if (5 == mess[carCour]) /* Traitement du ENQ                            */
		{
			char *enq = "[ENQ]";
			int indice = 0;
			while ((enq[indice] != 0) && (buf->nbCar < buf->nbCarMax))
			{
				buf->buffer[buf->nbCar++] = enq[indice++];
			}
			carCour++;
		} else if (2 == mess[carCour]) /* Traitement du STX                            */
		{
			char *stx = "[STX]";
			int indice = 0;
			while ((stx[indice] != 0) && (buf->nbCar < buf->nbCarMax))
			{
				buf->buffer[buf->nbCar++] = stx[indice++];
			}
			carCour++;
		} else if (3 == mess[carCour]) /* Traitement du ETX                            */
		{
			char *etx = "[ETX]";
			int indice = 0;
			while ((etx[indice] != 0) && (buf->nbCar < buf->nbCarMax))
			{
				buf->buffer[buf->nbCar++] = etx[indice++];
			}
			carCour++;
			retour = 1;
		} else if (0x17 == mess[carCour]) /* Traitement du ETB                            */
		{
			char *etx = "[ETB]";
			int indice = 0;
			while ((etx[indice] != 0) && (buf->nbCar < buf->nbCarMax))
			{
				buf->buffer[buf->nbCar++] = etx[indice++];
			}
			carCour++;
		} else if (('\n' == mess[carCour]) || ('\r' == mess[carCour]))
		{
			buf->buffer[buf->nbCar++] = ' ';
			carCour++;
		} else
		{
			if ((('?' == mess[carCour]) || ('%' == mess[carCour])) && (0 == buf->nbCar))
			{
				if(0==buf->guillemet) {
					printDebug("J'ai un acquit negatif\n");
					retour = 2;
				}
			}
			buf->buffer[buf->nbCar++] = mess[carCour++];
		}
	} /* endwhile((                                           */
	/* --- RECHERCHE DE L'ACQUIT            */
	while ((carCour < nbCar) && (0 == retour))
	{
		if ('!' == mess[carCour]) {
			/* L'acquit court positif n'est
			 * pas trace.   */
			if(0==buf->guillemet) {
				printDebug("J'ai un acquit positif\n");
				retour = 1;
			}
		} else if ('"' == mess[carCour]) {
			if( 0 == buf->guillemet ) {
				/* On se trouve a l'interieur d'un message pmv */
				buf->guillemet=1;
			} else {
				/* Sortie du message pmv */
				buf->guillemet=0;
			}
			carCour++;
		} else if ((('?' == mess[carCour]) || ('%' == mess[carCour])) && (0 == buf->nbCar))
		{
			if(0==buf->guillemet) {
				printDebug("J'ai un acquit negatif\n");
				retour = 2;
			}
		}
		carCour++;
	} /* endwhile((carCour<nbCar                      */
	/* --------------------------------
	 * FIN DE lcr_util_buffer_ajouter
	 * --------------------------------     */
	return retour;
}

/* ---------------------------------
 * lcr_util_buffer_lire_buffer
 * ===========================
 * La fonction retourne le pointeur
 * sur le debut du buffer.
 * --------------------------------	*/

char *lcr_util_buffer_lire_buffer(T_lcr_util_buffer * buf)
{
	return (buf->buffer);
}

/* ---------------------------------
 * lcr_util_buffer_lire_nb_car
 * ======================
 * Retourne le nombre de caracteres
 * utiles dans le buffer.
 * --------------------------------	*/

int lcr_util_buffer_lire_nb_car(T_lcr_util_buffer * buf)
{
	return (buf->nbCar);
}

/* --------------------------------
 * tst_send_bloc
 * =============
 * Ajout dans le buffer des infos
 * necessaires pour le retour.
 * --------------------------------	*/
void tst_send_bloc(INT las, INT mode, STRING pt_buff, INT * nbCar, INT * bloc, INT der_blk, struct usr_ztf *pt_mess,
		char *format, ...)
{
	va_list liste;
	char donnees[MAXLCR];
	int taille;
	int carBuf = *nbCar;
	int carDebut = 0;
	/* Formattage des donneees                      */
	va_start(liste, format);
	taille = vsprintf(donnees, format, liste);
	while ((carBuf + taille) >= MAXLCR)
	{
		int aCopier = MIN (taille, MAXLCR - carBuf);
		memcpy(&pt_buff[carBuf], &donnees[carDebut], aCopier);
		/* Envoi du buffer                                      */
		tedi_send_bloc(las, mode, pt_buff, carBuf + aCopier, *bloc, FALSE, pt_mess);
		if (++(*bloc) >= 10)
		{
			*bloc = 0;
		}
		carDebut += aCopier;
		taille -= aCopier;
		carBuf = 0;
	}
	/* Copie du reste du message            */
	memcpy(&pt_buff[carBuf], &donnees[carDebut], taille);
	carBuf += taille;
	/* Si il s'agit du dernier block.       */
	if (der_blk == TRUE)
	{
		tedi_send_bloc(las, mode, pt_buff, carBuf, *bloc, TRUE, pt_mess);
		if (++(*bloc) >= 10)
		{
			*bloc = 0;
		}
		carBuf = 0;
	}
	/* Combien reste t-il de caracteres     */
	*nbCar = carBuf;
}

char *tst_passe_blanc(char *buffer, int lg_mess, int *reste)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	while (lgCour > 0)
	{
		if ((*ptCour == ' ') || (*ptCour == '\t') || (*ptCour == '\n'))
		{
			ptCour++;
			lgCour--;
		} else
		{
			break;
		}
	}
	*reste = lgCour;
	return ptCour;
}

char *tst_passe_slash(char *buffer, int lg_mess, int *reste)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	printDebug("Dans tst passe slash\n");
	if (*ptCour == '/')
	{
		ptCour++;
		lgCour--;
	} else
	{
		lgCour = 0;
	}
	*reste = lgCour;
	return ptCour;
}

char *tst_passe_egale(char *buffer, int lg_mess, int *reste)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	printDebug("Dans tst passe egale\n");
	if (*ptCour == '=')
	{
		ptCour++;
		lgCour--;
	} else
	{
		lgCour = 0;
	}
	*reste = lgCour;
	return ptCour;
}

char *tst_lit_char(char *buffer, int lg_mess, int *reste, char *val)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	char valeur = 0;

	printDebug("Dans tst lit char\n");
	while ((*ptCour >= '0') && (*ptCour <= '9'))
	{
		valeur = (char) (valeur * 10 + ((*ptCour) - '0'));
		ptCour++;
		lgCour--;
	}
	*val = valeur;
	*reste = lgCour;
	return ptCour;
}

char *tst_lit_short(char *buffer, int lg_mess, int *reste, short *val)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	short valeur = 0;

	printDebug("Dans tst lit short\n");
	while ((*ptCour >= '0') && (*ptCour <= '9'))
	{
		valeur = (short) (valeur * 10 + ((*ptCour) - '0'));
		ptCour++;
		lgCour--;
	}
	*val = valeur;
	*reste = lgCour;
	return ptCour;
}

char *tst_lit_entier(char *buffer, int lg_mess, int *reste, int *val)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	int valeur = 0;

	printDebug("Dans tst lit entier\n");
	if (0 < lg_mess)
	{
		while ((*ptCour >= '0') && (*ptCour <= '9'))
		{
			valeur = valeur * 10 + ((*ptCour) - '0');
			ptCour++;
			lgCour--;
		}
		if (lgCour != lg_mess)
		{
			*val = valeur;
		} else
		{
			*val = -1;
		}
	} else
	{
		*val = -1;
	}

	*reste = lgCour;
	return ptCour;
}

/* --------------------------------
 * tst_lit_date
 * ============
 * Lecture d'une date composee de
 * l'indication de l'heure precedee
 * du jour dans l'annee.
 * Entrees :
 * -	un pointeur sur le buffer
 * 		contenant le message a
 * 		decoder,
 * - 	longueur du buffer en
 * 		nombre de caracteres.
 * Sorties :
 * -	le nombre de caracteres
 *      restant e traiter dans le
 *      buffer,
 * -	la date decodee,
 * -	l'indicateur de dechiffrage de
 * 		la date,
 * - 	l'indicateur de dechiffrage de
 * 		l'heure,
 *  Valeur de retour :
 *  -	le pointeur sur le debut du
 *  	buffer e decoder.
 * --------------------------------	*/

char *tst_lit_date(char *buffer, int lg_mess, int *reste, struct tm *date, int *dateOk, int *heureOk)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;

	ptCour = tst_lit_jour(ptCour, lgCour, &lgCour, date, dateOk);
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	ptCour = tst_lit_heure(ptCour, lgCour, &lgCour, date, heureOk);
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	*reste = lgCour;
	return ptCour;
}

/* ----------------------------------
 * tst_lit_jour
 * ============
 * Lecture du jour dans l'annee suivant
 * le format jj/mm/aa ou jj/mm/aaaa.
 * -------------------------------- */

char *tst_lit_jour(char *buffer, int lg_mess, int *reste, struct tm *date, int *dateOk)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	char *ptSuiv;
	int lgSuiv;
	int jour, mois, annee;

	*dateOk = 0;
	ptSuiv = tst_lit_entier(ptCour, lgCour, &lgSuiv, &jour);
	if (-1 != jour)
	{
		printDebug("Le jour est OK : %d\n", jour);
		if ('/' == *ptSuiv)
		{
			ptSuiv++;
			lgSuiv--;
			ptSuiv = tst_lit_entier(ptSuiv, lgSuiv, &lgSuiv, &mois);
			if (-1 != mois)
			{
				printDebug("Le mois est OK : %d\n", mois);
				if ('/' == *ptSuiv)
				{
					ptSuiv++;
					lgSuiv--;
					ptSuiv = tst_lit_entier(ptSuiv, lgSuiv, &lgSuiv, &annee);
					if (-1 != annee)
					{
						if (1900 < annee)
						{
							annee -= 1900;
						} else
						{
							/* Si la date est codee sur deux
							 * chiffres, et pour respecter le
							 * codage de la structure time, on
							 * suppose que l'on debute en 1900
							 */
							annee += 100;
						}
						printDebug("L'annee est OK : %d\n", annee);
						date->tm_mday = jour;
						date->tm_mon = mois - 1;
						date->tm_year = annee;
						ptCour = ptSuiv;
						lgCour = lgSuiv;
						*dateOk = 1;
					}
				}
			}
		}
	}
	*reste = lgCour;
	return ptCour;
}

char *tst_lit_heure(char *buffer, int lg_mess, int *reste, struct tm *date, int *heureOk)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	char *ptSuiv;
	int lgSuiv;
	int heure, minute, seconde;

	*heureOk = 0;
	ptSuiv = tst_lit_entier(ptCour, lgCour, &lgSuiv, &heure);
	if (-1 != heure)
	{
		printDebug("L'heure est OK : %d\n", heure);
		if (':' == *ptSuiv)
		{
			ptSuiv++;
			lgSuiv--;
			ptSuiv = tst_lit_entier(ptSuiv, lgSuiv, &lgSuiv, &minute);
			if (-1 != minute)
			{
				printDebug("Minute est OK : %d\n", minute);
				if (':' == *ptSuiv)
				{
					ptSuiv++;
					lgSuiv--;
					ptSuiv = tst_lit_entier(ptSuiv, lgSuiv, &lgSuiv, &seconde);
					if (-1 != seconde)
					{
						printDebug("Les secondes sont OK : %d\n", seconde);
						ptCour = ptSuiv;
						lgCour = lgSuiv;
						date->tm_hour = heure;
						date->tm_min = minute;
						date->tm_sec = seconde;
						*heureOk = 1;
					}
				}
			}
		}
	}
	*reste = lgCour;
	return ptCour;
}
/* ------------------------------
 * tst_lit_param
 * =============
 * Lecture d'un parametre dans la
 * ligne de commande LCR.
 * Ajout pour le traitement des
 * parametres d'affichage des PMV
 * d'un test complementaire de
 * comptage des guillemets.
 * ----------------------------	*/

char *tst_lit_param(char *buffer, int lg_mess, int *reste, char *param)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	while (lgCour > 0)
	{
		if ((*ptCour != ' ') && (*ptCour != '\t') && (*ptCour != '\r') && (*ptCour != '/') && (*ptCour != '\n'))
		{
			*param++ = *ptCour++;
			lgCour--;
		} else
		{
			break;
		}
	}
	*param++ = 0;
	*reste = lgCour;
	return ptCour;
}

/* ------------------------------
 * tst_lit_param_af
 * ================
 * Lecture d'un parametre dans la
 * ligne de commande LCR.
 * Ajout pour le traitement des
 * parametres d'affichage des PMV
 * d'un test complementaire de
 * comptage des guillemets.
 * ----------------------------	*/

char *tst_lit_param_af(char *buffer, int lg_mess, int *reste, char *param)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	int nbGuil = 0;
	char prec = 0;
	char *debut = param;
	while (lgCour > 0)
	{
		if (('"' == *ptCour) && (prec != 0x19))
		{
			nbGuil++;
		}
		if (((*ptCour != ' ') || (0 != (nbGuil % 2))) && (*ptCour != '\t') && (*ptCour != '\r') && ((*ptCour != '/')
				|| (0 != (nbGuil % 2))) && (*ptCour != '\n'))
		{
			prec = *ptCour++;
			if ((prec != '"') || ((nbGuil % 2) != 0))
			{
				*param++ = prec;
			}
			lgCour--;
		} else
		{
			break;
		}
	}
	if ((prec == '"') && ((nbGuil % 2) == 0))
	{
		*param++ = prec;
	}
	*param++ = 0;
	printf("La valeur du parametre est : ##%s##\n", debut);
	*reste = lgCour;
	return ptCour;
}

/* ------------------------------
 * tst_lit_param_at
 * ================
 * Lecture d'un parametre dans la
 * ligne de commande LCR pour le
 * parametrage de l'alternat.
 * Il peut y avoir de '/'.
 * ----------------------------	*/

char *tst_lit_param_at(char *buffer, int lg_mess, int *reste, char *param)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	while (lgCour > 0)
	{
		if ((*ptCour != ' ') && (*ptCour != '\t') && (*ptCour != '\r') && (*ptCour != '\n'))
		{
			*param++ = *ptCour++;
			lgCour--;
		} else
		{
			break;
		}
	}
	*param++ = 0;
	*reste = lgCour;
	return ptCour;
}
/* ----------------------------------
 * tst_lit_am
 * ============
 * Lecture de l'adresse module que
 * se soit l'adresse topologique ou
 * l'adresse constructeur.
 * Par defaut, le numero de caisson est
 * e -1.
 * -------------------------------- */

char *tst_lit_am(char *buffer, int lg_mess, int *reste, int *module, int *caisson, int *amOk)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	char *ptSuiv;
	int lgSuiv;

	*amOk = 0;
	ptSuiv = tst_lit_entier(ptCour, lgCour, &lgSuiv, module);
	if (-1 != *module)
	{
		if ('.' == *ptSuiv)
		{
			ptSuiv++;
			lgSuiv--;
			ptSuiv = tst_lit_entier(ptSuiv, lgSuiv, &lgSuiv, caisson);
			if (-1 != *caisson)
			{
				printDebug("Le caisson est OK : %d\n", *caisson);
				ptCour = ptSuiv;
				lgCour = lgSuiv;
				*amOk = 1;
			}
		} else
		{
			ptCour = ptSuiv;
			lgCour = lgSuiv;
			*amOk = 1;
			*caisson = -1;
		}
	}
	*reste = lgCour;
	return ptCour;
}

/* ----------------------------------
 * tst_lit_am_bis
 * ==============
 * Lecture de l'adresse module que
 * se soit l'adresse topologique ou
 * l'adresse constructeur.
 * Par defaut, le numero de caisson est
 * e -1.
 * Interpretation aussi du caractere '*'
 * -------------------------------- */

char *tst_lit_am_bis(char *buffer, int lg_mess, int *reste, int *module, int *caisson, int *amOk)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	char *ptSuiv;
	int lgSuiv;

	*amOk = 0;
	if (*ptCour != '*')
	{
		ptSuiv = tst_lit_entier(ptCour, lgCour, &lgSuiv, module);
		if (-1 != *module)
		{
			if ('.' == *ptSuiv)
			{
				ptSuiv++;
				lgSuiv--;
				if ('*' != *ptSuiv)
				{
					ptSuiv = tst_lit_entier(ptSuiv, lgSuiv, &lgSuiv, caisson);
					if (-1 != *caisson)
					{
						printDebug("Le caisson est OK : %d\n", *caisson);
						ptCour = ptSuiv;
						lgCour = lgSuiv;
						*amOk = 1;
					}
				} else
				{
					ptCour = ptSuiv + 1;
					lgCour = lgSuiv - 1;
					*amOk = 1;
					*caisson = -1;
				}
			} else
			{
				ptCour = ptSuiv;
				lgCour = lgSuiv;
				*amOk = 1;
				*caisson = -1;
			}
		}
	} else
	{
		ptCour++;
		lgCour--;
		*amOk = 1;
		*module = NN_AFF;
		*caisson = N_AFF;
		if (0 == strncmp(ptCour, ".*", strlen(".*")))
		{
			ptCour += strlen(".*");
			lgCour -= strlen(".*");
		}
	}
	*reste = lgCour;
	return ptCour;
}

char *tst_lit_dv(char *buffer, int lg_mess, int *reste, long *date, int *heureOk)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	char *ptSuiv;
	int lgSuiv;
	int heure = 0;
	int minute = 0;
	int seconde = 0;
	;

	*heureOk = 0;
	ptSuiv = tst_lit_entier(ptCour, lgCour, &lgSuiv, &heure);
	if (-1 != heure)
	{
		printDebug("L'heure est OK : %d\n", heure);
		if (':' == *ptSuiv)
		{
			ptSuiv++;
			lgSuiv--;
			ptSuiv = tst_lit_entier(ptSuiv, lgSuiv, &lgSuiv, &minute);
			if (-1 != minute)
			{
				printDebug("Minute est OK : %d\n", minute);
				if (':' == *ptSuiv)
				{
					ptSuiv++;
					lgSuiv--;
					ptSuiv = tst_lit_entier(ptSuiv, lgSuiv, &lgSuiv, &seconde);
					if (-1 != seconde)
					{
						printDebug("Les secondes sont OK : %d\n", seconde);
						*heureOk = 1;
					}
				} else
				{
					*heureOk = 1;
				}
			}
		} else
		{
			*heureOk = 1;
		}
	}
	if (*heureOk)
	{
		ptCour = ptSuiv;
		lgCour = lgSuiv;
		*date = (heure * 60 + minute) * 60 + seconde;
	}

	*reste = lgCour;
	return ptCour;
}

char *tst_lit_param_symb(char *buffer, int lg_mess, int *reste, char *param)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	while (lgCour > 0)
	{
		if ((*ptCour != ' ') && (*ptCour != '\t') && (*ptCour != '\r') && (*ptCour != '=') && (*ptCour != '\n'))
		{
			*param++ = *ptCour++;
			lgCour--;
		} else
		{
			break;
		}
	}
	*param++ = 0;
	*reste = lgCour;
	return ptCour;
}

int tst_j7(char carCour)
{
	int retour = 0;
	printDebug("tst_j7 %c\n", carCour);
	if ((('0' <= carCour) && ('9' >= carCour)) || (('A' <= carCour) && ('Z' >= carCour)) || (('a' <= carCour) && ('z'
			>= carCour)) || (0x7B == carCour) || (0x7C == carCour) || (' ' == carCour))
	{
		retour = 1;
	}
	return retour;
}

/* ------------------------------------
 * tst_lit_param_usr
 * =================
 * Lecture des parametres en entree
 * pour les parametres utilisateur.
 * Attention, en entree, on doit avoir
 * deux buffers de 40 caracteres.
 * ------------------------------------	*/

char *tst_lit_param_usr(char *buffer, int lg_mess, int *reste, char *usr1, char *usr2, int *ok)
{
	char *ptCour = buffer;
	int lgCour = lg_mess;
	int lgChaine = 0;
	char *ptChaine = usr1;
	usr1[0] = 0;
	usr2[0] = 0;
	*ok = TRUE;
	printDebug("tst_lit_param_usr\n");
	while (lgCour > 0)
	{
		if (tst_j7(*ptCour) && (lgChaine < MAX_USR_LCPI))
		{
			ptChaine[lgChaine++] = *ptCour++;
			lgCour--;
		} else if ((*ptCour == '/') && (ptChaine == usr1))
		{
			ptChaine[lgChaine++] = 0;
			ptChaine = usr2;
			lgChaine = 0;
			ptCour++;
			lgCour--;

		} else if ((*ptCour == '\t') || (*ptCour != '\r') || (*ptCour != '\n'))
		{
			ptChaine[lgChaine++] = 0;
			ptChaine = usr2;
			lgChaine = 0;
			ptCour++;
			lgCour--;
			break;
		} else
		{
			printDebug("tst_lit_param_NOK\n");
			*ok = FALSE;
		}
	}
	if (TRUE == *ok)
	{
		ptChaine[lgChaine++] = 0;
	}
	*reste = lgCour;
	return ptCour;
}

char *tst_lit_param_cftp(char *buffer, int lg_mess, int *reste, char *usr1, int *ok)
{
	char *ptCour = buffer;
	int indCour = 0;
	int lgCour = lg_mess;
	int lgChaine = 0;
	char *ptChaine = usr1;
	usr1[0] = 0;
	*ok = TRUE;
	printDebug("tst_lit_param_cftp\n");
	if ('"' == ptCour[indCour])
	{
		indCour++;
		lgCour--;
		printDebug("tst_lit_param_cftp, premiere etape\n");
		while ((lgCour > 0) && (FALSE != *ok))
		{
			if (tst_j7(ptCour[indCour]) && (lgChaine < MAX_USR_LCPI))
			{
				ptChaine[lgChaine++] = ptCour[indCour];
				indCour++;
				lgCour--;
			} else if (ptCour[indCour] == '"')
			{
				indCour++;
				lgCour--;
				break;
			} else
			{
				printDebug("tst_lit_param_cftp, probleme %c\n", ptCour[indCour]);
				*ok = FALSE;
			}
		}
		if (TRUE == *ok)
		{
			ptChaine[lgChaine++] = 0;
		}
	} else
	{
		*ok = FALSE;
	}
	*reste = lgCour;
	return ptCour;
}

