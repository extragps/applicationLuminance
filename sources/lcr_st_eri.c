
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 02/05/01
 * Fichier	: lcr_st_eri.c
 * Objet	: Traitement de la commande d'initialisation du compteur
 * 			anti-chevauchement.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,02May01,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "standard.h"
#include "define.h"
#include "x01_str.h"
#include "vct_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_st_divers.h"
#include "ted_prot.h"
#include "eriLib.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 0
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define ST_ERI_CMD				"ST ERI"

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
 * lcr_st_eri
 * =============
 * Analyse de la commande "ST ERI"
 * --------------------------------	*/

int
lcr_st_eri (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
            struct usr_ztf *pt_mess)
{
  char *ptCour = (char *) &buffer[position];
  int lgCour = lg_mess - position;
  int modifConfig = 0;
  int nbErreur;
  int bloc=0;


  ptCour += strlen (ST_ERI_CMD);
  lgCour -= strlen (ST_ERI_CMD);
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
  nbErreur = eriLireNbErreur ();
  if (0 == nbErreur)
    {
	int j=0;
		j=sprintf(buffer,"STATUS ERI 0\n\r");
		tedi_send_bloc (las, mode, buffer, j, bloc, TRUE, pt_mess);
      /* tst_send_bloc (las, mode, buffer, &nbCar, bloc, TRUE, pt_mess,
                     "STATUS ERI 0\n\r"); */
    }
  else
    {
	int j=0;
      char chaine[MAX_MESSAGE_LCR] = "";
      int indCour;
		j=sprintf(buffer,"STATUS ERI\n\r");
		printDebug("Nombre d'erreurs %d\n",nbErreur);
      for (indCour = nbErreur - 1; indCour >= 0; indCour--)
        {
          char grav, nature;
          char temp[10];
		  char mod,cais;
		  int val;
		  chaine[0]=0;
          eriLireErreur (indCour, &grav, &nature);
		 	sprintf(temp, " 10%01d%02d", grav, nature);
			strcat(chaine,temp);
          eriLireErreurModule (indCour, &mod, &cais,&val);
		  if(0!=mod)
		  {
		 	sprintf(temp, "/%d", mod);
			strcat(chaine,temp);

		  	if(((char)N_AFF)!=cais)
		  	{
		 	sprintf(temp, ".%d", cais);
			strcat(chaine,temp);
		  		if(255!=val)
		  		{
		 		sprintf(temp, ".%d", val);
			strcat(chaine,temp);
		  		}
		  	}
		  }
          j+=sprintf (&buffer[j], "%s",chaine);
      	  tedi_test_bloc (las, mode, buffer, &j, &bloc, pt_mess);

		}
      	tedi_send_bloc (las, mode, buffer, j, bloc,TRUE, pt_mess);
    }
  /* ------------------------------------
   * FIN DE lcr_st_eri
   * ------------------------------------ */
  return modifConfig;
}
