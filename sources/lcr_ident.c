/*
 * lcr_ident.c
 * ===================
 * Objet	: Le fichier contient les nouvelles fonctions d'analyse pour
 * 			l'identification suivant les differents modes envisages :
 * 			- le mode direct,
 * 			- le mode datagramme,
 * 			- le mode parametre.
 *
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_ident.c,v $
 * Revision 1.2  2018/06/04 08:38:36  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "define.h"
#include "standard.h"
#include "mon_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "mon_str.h"
#include "tst_simu.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "identLib.h"
#include "lcr_ident.h"
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
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * lcr_ident
 * =============
 * Analyse de la commande "ID"
 * Entree :
 * - numero de la liaison sur lequel
 *   se fait le transfert.
 * - mode : mode de transmission,
 * - nombre de caracteres dans le
 *   buffer,
 * - position dans le buffer,
 * - pointeur vers un message de
 *   transmission
 * --------------------------------	*/

int lcr_ident(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int erreur = 0;
	char *ptSuiv;
	int lgSuiv;
	int nbParam;
	char param1[MAXLCR];
	char param2[MAXLCR];
	lgCour -= 2;
	ptCour += 2;
	/* On saute les espaces.                        */
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	/* Recuperation du premier parametre */
	/* Recuperation du second parametre
	 * si il y en a un.     */
	/* Rechercher ici param1 */
	ptCour = tst_lit_param(ptCour, lgCour, &lgCour, param1);
	nbParam = 1;
	/* On saute les caracteres blancs e
	 * suivre. pour pointer eventuellement
	 * sur le debut d'une nouvelle commande
	 */
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	/* Rechercher ici param2 */
	if (lgCour > 0)
	{
		nbParam = 2;
		ptSuiv = tst_lit_param(ptCour, lgCour, &lgSuiv, param2);
	}
	if (2 == nbParam)
	{
		int utilisateur = identTesterIdentEtPassword(param1, param2);
		if (-1 != utilisateur)
		{
			/* OK pour l'identification on met
			 * les pointeurs e jour.        */
			ptCour = ptSuiv;
			lgCour = lgSuiv;
			printDebug("LgCour %d\n", lgCour);
			identAutoriser(las, utilisateur, (0 == lgCour));
			ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		} else
		{
			utilisateur = identTesterPassword(param1);
			if (-1 != utilisateur)
			{
				printDebug("LgCour %d\n", lgCour);
				identAutoriser(las, utilisateur, (0 == lgCour));
			} else
			{
				identInterdire(las, (0 == lgCour));
			}
		}
	} else
	{
		int utilisateur = identTesterPassword(param1);
		/* Il peut s'agir d'une identification
		 * avec mot de passe seulement et le
		 * parametre 2 est alors la commande
		 * e suivre pour une identification en
		 * mode datagramme. */
		if (-1 != utilisateur)
		{
			printDebug("LgCour %d\n", lgCour);
			identAutoriser(las, utilisateur, (lgCour == 0));
		} else
		{
			/* Nous sommes en presence d'une
			 * identification incorrecte.
			 * Il faut renvoyer le code qui
			 * va bien. */
			identInterdire(las, (0 == lgCour));
			erreur = 2;
		}
	}
	switch (erreur)
	{
	case 2:
		x01_cptr.erreur = erreur;
		tedi_erreur(las, mode);
		break;
	default:
		if (0 == lgCour)
		{
			tedi_ctrl(las, mode, TRUE);
		} /* endif(0==lgCour                                      */
		break;
	}
	/* Il faut retouner le nombre de
	 * caracteres traites */
	return (lg_mess + position - lgCour);
}

/* -------------------------------
 * lcr_ident_param
 * ===============
 * Analyse de la commande d'iden-
 * tification passee en parametre.
 * ------------------------------- */

int lcr_ident_param(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	char *ptSuiv;

	ptSuiv = strstr(ptCour, "ID=");
	/* As t-on trouve le mot cle????        */
	if ((NULL != ptSuiv) && ((ptSuiv - ptCour) < lg_mess))
	{
		int nbParam;
		char param1[MAXLCR];
		char param2[MAXLCR];
		lgCour -= (3 + ptSuiv - ptCour);
		ptCour = ptSuiv;
		ptCour += 3;
		/* Recuperation du premier parametre */
		/* Recuperation du second parametre
		 * si il y en a un.     */
		/* Rechercher ici param1 */
		ptCour = tst_lit_param(ptCour, lgCour, &lgCour, param1);
		nbParam = 1;
		/* On saute les caracteres blancs e
		 * suivre. pour pointer eventuellement
		 * sur le debut d'une nouvelle commande
		 */
		if (*ptCour == '/')
		{
			ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
			/* Rechercher ici param2 */
			if (lgCour > 0)
			{
				nbParam = 2;
				ptCour = tst_lit_param(ptCour, lgCour, &lgCour, param2);
			}
		}
		if (2 == nbParam)
		{
			int utilisateur = identTesterIdentEtPassword(param1, param2);
			if (-1 != utilisateur)
			{
				int indice;
				int longueur = lg_mess - position - (ptCour - ptSuiv);
				lg_mess = lg_mess - (ptCour - ptSuiv);
				for (indice = 0; indice < longueur; indice++)
				{
					*ptSuiv++ = *ptCour++;
				}
				/* OK pour l'identification on met
				 * les pointeurs e jour.        */
				printDebug("LgCour %d FALSE\n", lgCour);
				identAutoriser(las, utilisateur, FALSE);
			}
		} else
		{
			int utilisateur = identTesterPassword(param1);
			/* Il peut s'agir d'une identification
			 * avec mot de passe seulement et le
			 * parametre 2 est alors la commande
			 * e suivre pour une identification en
			 * mode datagramme. */
			if (-1 != utilisateur)
			{
				int indice;
				int longueur = lg_mess - position - (ptCour - ptSuiv);
				lg_mess = lg_mess - (ptCour - ptSuiv);
				for (indice = 0; indice < longueur; indice++)
				{
					*ptSuiv++ = *ptCour++;
				}
				identAutoriser(las, utilisateur, FALSE);
			} else
			{
				/* Nous sommes en presence d'une
				 * identification incorrecte.
				 * Il faut renvoyer le code qui
				 * va bien. */
				identInterdire(las, (0 == lgCour));
			}
		}
	}
	/* -------------------------------
	 * FIN DE lcr_ident_param
	 * --------------------------------     */
	/* Il faut retouner le nombre de
	 * caracteres traites */
	return (lg_mess);
}

/* --------------------------------
 * lcr_ident_substituer
 * ====================
 * Rechercher de la chaine ID en
 * debut de commande et substitu-
 * tion eventuelle par les infos qui
 * vont bien pour l'enregistrement
 * dans les fichiers de traces.
 * --------------------------------	*/

char *lcr_ident_substituer(char *commande, int nbCarCmd, int *nbCarRestant, T_lcr_util_buffer * rep)
{
	char *ptCour = commande;
	int lgCour = nbCarCmd;
	char *ptSuiv;
	int lgSuiv;
	int nbParam;
	char param1[MAXLCR];
	char param2[MAXLCR];

	/* On saute les espaces.                        */
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	if (0 == strncmp(ptCour, "ID", strlen("ID")))
	{
		ptCour += 2;
		lgCour -= 2;
		lcr_util_buffer_ajouter(rep, "ID ", strlen("ID "));
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		/* Recuperation du premier parametre */
		/* Recuperation du second parametre
		 * si il y en a un.     */
		/* Rechercher ici param1 */
		ptCour = tst_lit_param(ptCour, lgCour, &lgCour, param1);
		nbParam = 1;
		/* On saute les caracteres blancs e
		 * suivre. pour pointer eventuellement
		 * sur le debut d'une nouvelle commande
		 */
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
		/* Rechercher ici param2 */
		if (lgCour > 0)
		{
			nbParam = 2;
			ptSuiv = tst_lit_param(ptCour, lgCour, &lgSuiv, param2);
			ptSuiv = tst_passe_blanc(ptSuiv, lgSuiv, &lgSuiv);
		}
		/* Recopie des informations.    */
		if (2 == nbParam)
		{
			int utilisateur = identTesterIdentEtPassword(param1, param2);
			if (-1 != utilisateur)
			{
				/* OK p our l'identification on met
				 * les pointeurs e jour.        */
				ptCour = ptSuiv;
				lgCour = lgSuiv;
				/* Recuperation de l'ident pour l'ecrire dans le fichier
				 * de traces.  */
				lcr_util_buffer_ajouter(rep, param1, strlen(param1));
				lcr_util_buffer_ajouter(rep, " * ", strlen(" * "));
			} else
			{
				int utilisateur = identTesterPassword(param1);
				if (-1 != utilisateur)
				{
					/* Recuperer l'identification et l'ecrire dans le
					 * fichier de traces. */
					char *passIdent = identLireIdent(param1);
					lcr_util_buffer_ajouter(rep, passIdent, strlen(passIdent));
					lcr_util_buffer_ajouter(rep, " * ", strlen(" * "));
				} else
				{
					ptCour = ptSuiv;
					lgCour = lgSuiv;
					/* Ecrire les parametres tel quel dans le buffer. */
					lcr_util_buffer_ajouter(rep, param1, strlen(param1));
					lcr_util_buffer_ajouter(rep, " ", strlen(" "));
					lcr_util_buffer_ajouter(rep, param2, strlen(param2));
					lcr_util_buffer_ajouter(rep, " ", strlen(" "));
				}
			}
		} else
		{
			int utilisateur = identTesterPassword(param1);
			/* Il peut s'agir d'une identification
			 * avec mot de passe seulement et le
			 * parametre 2 est alors la commande
			 * e suivre pour une identification en
			 * mode datagramme. */
			if (-1 != utilisateur)
			{
				char *passIdent = identLireIdent(param1);
				lcr_util_buffer_ajouter(rep, passIdent, strlen(passIdent));
				lcr_util_buffer_ajouter(rep, " * ", strlen(" * "));
			} else
			{
				/* Ecrire le parametre tel quel dans le buffer. */
				/* Le pointeur sur le suivant ne change pas. */
				lcr_util_buffer_ajouter(rep, param1, strlen(param1));
				lcr_util_buffer_ajouter(rep, " ", strlen(" "));
			}
		}
	}
	/* Il faut retouner le nombre de
	 * caracteres traites */
	*nbCarRestant = lgCour;
	return (ptCour);
}

/* --------------------------------
 * lcr_ident_param_substituer
 * ==========================
 * Recherche de l'identintification
 * en mode parametre.
 * --------------------------------	*/

char *lcr_ident_param_substituer(char *commande, int nbCarCmd, int *nbCarRestant, T_lcr_util_buffer * rep)
{
	char *ptCour = commande;
	int lgCour = nbCarCmd;
	char *ptSuiv;
	int lgSuiv;

	ptSuiv = strstr(ptCour, "ID=");
	lgSuiv=lgCour-(ptSuiv-ptCour);
	/* As t-on trouve le mot cle????        */
	if ((NULL != ptSuiv) && ((int) (ptSuiv - ptCour) < nbCarCmd))
	{
		int nbParam;
		char param1[MAXLCR];
		char param2[MAXLCR];
		/* Recopie du debut de la commande      */
		lcr_util_buffer_ajouter(rep, ptCour, ptSuiv - ptCour);
		/* Mise e jour des pointeurs            */
		lgCour -= (3 + ptSuiv - ptCour);
		ptCour = ptSuiv;
		ptCour += 3;
		/* On saute les espaces.                        */
		lcr_util_buffer_ajouter(rep, "ID=", strlen("ID="));
		/* Recuperation du premier parametre */
		/* Recuperation du second parametre
		 * si il y en a un.     */
		/* Rechercher ici param1 */
		ptCour = tst_lit_param(ptCour, lgCour, &lgCour, param1);
		nbParam = 1;
		if (*ptCour == '/')
		{
			ptCour = tst_passe_slash(ptCour, lgCour, &lgCour);
			/* Rechercher ici param2 */
			if (lgCour > 0)
			{
				nbParam = 2;
				ptCour = tst_lit_param(ptCour, lgCour, &lgCour, param2);
			}
		}
		/* Recopie des informations.    */
		if (2 == nbParam)
		{
			int utilisateur = identTesterIdentEtPassword(param1, param2);
			if (-1 != utilisateur)
			{
				/* OK p our l'identification on met
				 * les pointeurs e jour.        */
				ptCour = ptSuiv;
				lgCour = lgSuiv;
				/* Recuperation de l'ident pour l'ecrire dans le fichier
				 * de traces.  */
				lcr_util_buffer_ajouter(rep, param1, strlen(param1));
				lcr_util_buffer_ajouter(rep, "/* ", strlen("/* "));
			} else
			{
				int utilisateur = identTesterPassword(param1);
				if (-1 != utilisateur)
				{
					/* Recuperer l'identification et l'ecrire dans le
					 * fichier de traces. */
					char *passIdent = identLireIdent(param1);
					lcr_util_buffer_ajouter(rep, passIdent, strlen(passIdent));
					lcr_util_buffer_ajouter(rep, "/* ", strlen("/* "));
				} else
				{
					/* Ecrire le parametre tel quel dans le buffer. */
					ptCour = ptSuiv;
					lgCour = lgSuiv;
					lcr_util_buffer_ajouter(rep, param1, strlen(param1));
					lcr_util_buffer_ajouter(rep, "/", strlen("/"));
					lcr_util_buffer_ajouter(rep, param2, strlen(param2));
					lcr_util_buffer_ajouter(rep, " ", strlen(" "));
				}
			}
		} else
		{
			int utilisateur = identTesterPassword(param1);
			/* Il peut s'agir d'une identification
			 * avec mot de passe seulement et le
			 * parametre 2 est alors la commande
			 * e suivre pour une identification en
			 * mode datagramme. */
			if (-1 != utilisateur)
			{
				/* Recuperer l'identification et l'ecrire dans le
				 * fichier de traces. */
				char *passIdent = identLireIdent(param1);
				lcr_util_buffer_ajouter(rep, passIdent, strlen(passIdent));
				lcr_util_buffer_ajouter(rep, "/* ", strlen("/* "));
			} else
			{
				/* Ecrire le parametre tel quel dans le buffer. */
				/* Le pointeur sur le suivant ne change pas. */
				lcr_util_buffer_ajouter(rep, "/", strlen("/"));
				lcr_util_buffer_ajouter(rep, param1, strlen(param1));
				lcr_util_buffer_ajouter(rep, " ", strlen(" "));
			}
		}
	}
	/* Recopier ici la suite...             */
	lcr_util_buffer_ajouter(rep, ptCour, lgCour);
	ptCour += lgCour;
	lgCour = 0;
	/* Il faut retouner le nombre de
	 * caracteres traites */
	*nbCarRestant = lgCour;

	/* ---------------------------------
	 * lcr_ident_param_substituer
	 * --------------------------------     */
	return (ptCour);
}
