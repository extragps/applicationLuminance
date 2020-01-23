/*
 * lcr_cf3.c
 * ===================
 *
 *  Created on: 20 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_cf3.c,v $
 * Revision 1.2  2018/06/04 08:38:38  xg
 * Passage en version V122
 *
 * Revision 1.5  2008/09/22 13:38:13  xag
 * Remplacement de la variable sur la taille memoire par un appel fonctionnel.
 *
 * Revision 1.4  2008/09/22 07:53:43  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.3  2008/09/12 14:57:11  xag
 * *** empty log message ***
 *
 * Revision 1.2  2008/07/04 17:05:06  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:06:04  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

#include <string.h>

#include "standard.h"
#include "mon_inc.h"
#include "define.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "sir_dv1h.h"
#include "sir_dv2h.h"

#include "lcr_idfh.h"
#include "ted_prot.h"
#include "lcr_cf3h.h"
#include "seqLib.h"
#include "x01_str.h"
#include "x01_var.h"

#include "sramBdd.h"
/* De maniere temporaire pour tester les mesures */

/* ********************************************
 * DEFINITION DES CONSTANTES
 * ******************************************** */

#define SIZEOF_INT 2

/* ********************************************
 * DEFINITION DES TYPES
 * ******************************************** */

typedef struct T_cf3Seq
{
	T_ficListe seq;
	char       car;
} T_cf3Seq;

/* ********************************************
 * VARIABLES LOCALES DU MODULE
 * ******************************************** */


static T_cf3Seq cf3Seq[]={

  {F_IC,'C'},
  {F_IQ,'Q'},
  {F_IR,'R'},
  {F_IP,'P'},
  {F_IM,'M'},
  {F_IA,'A'},
  {MAXFIC,'Z'}};
//
//static Sram *sramBdd_pt=NULL;
//extern Sram *sram_pt;

/* ********************************************
 * FONCTIONS DU MODULE
 * ******************************************** */

/*
 * cf3_get_char_seq
 * ================
 * Récuperation du caractere associe a la sequence.
 */
char cf3_get_char_seq(int seq)
{
char retour='Z';
int  indice=0;
	while(('Z'==retour)&&(MAXFIC!=cf3Seq[indice].seq))
	{
		if(seq==cf3Seq[indice].seq)
		{
			retour=cf3Seq[indice].car;
		}
		indice++;
	}
	return retour;
}

VOID cf3_conf_memoire (VOID)
{
  int ficCour;
  /* Initialisation de la structure de
   * donnees. */
  for (ficCour = 0; ficCour < MAXFIC; ficCour++)
    {
	T_zdf tmp;
      tmp.max_seq = ficInfo[ficCour].min;
      tmp.pt_ecr = (STRING) 0;
      tmp.pt_prem = (STRING) 0;
      tmp.nb_ecr = 0;
      tmp.num_ecr = 0;
      tmp.sz_enr = 1024;
	  tmp.ty_seq=cf3_get_char_seq(ficCour);

	  sramBddEcrireFichier(ficCour,&tmp);

    }
  /* --------------------------------
   * FIN DE cf3_conf_memoire
   * --------------------------------     */
}

/* --------------------------------
 * cf3_init_fic_tra
 * ================
 * Initialisation des fichiers de
 * traces.
 * Celle ci ne peut maintenant se
 * faire que si la taille d'un
 * fichier de trace change.
 * A l'avenir, on pourra meme prevoir
 * de ne pas perdre les fichiers
 * de traces meme dans ce cas la.
 * --------------------------------	*/

VOID cf3_init_fic_tra (void)
{
INT ficCour;                        /* indice de boucle sur les fichiers */
STRING prec_pt=NULL;

   /* pour chaque fichier */
  for (ficCour = 0; ficCour < MAXFIC; ficCour++)
    {
	T_zdf tmp;
    int portCour;
	  sramBddLireFichier(ficCour,&tmp);
      tmp.sz_enr = cf3_szenrfic (ficCour, FALSE);
 	  tmp.ty_seq=cf3_get_char_seq(ficCour);

 	  if (ficCour == 0)
        {
    	  /* TODO : Remplacer sram_add_mem par 0 car a priori, on part de 0 */
          tmp.pt_deb = (STRING)
            ((int)sramBddTailleGet() -
             (tmp.max_seq * tmp.sz_enr));
        }
      else
        {
          tmp.pt_deb = prec_pt - (tmp.max_seq * tmp.sz_enr);
        }
      /* le nombre d'enregistrement
       * disponible est mis a zero */
      tmp.nb_ecr = 0;
      /* le numero courant est le numero 0 */
      tmp.num_ecr = 0;
      /* le pointeur d'ecriture                       */
      tmp.pt_ecr = tmp.pt_deb;
      tmp.pt_prem = tmp.pt_deb;
      tmp.pad = 'X';
      for (portCour = 0; portCour < NBPORT; portCour++)
        {
          tmp.nb_dispo[portCour] = 0;
          tmp.sav_num_ecr[portCour] = tmp.num_ecr;
          tmp.sav_pt_ecr[portCour] = tmp.pt_ecr;
        }
      /* Sauvegarde du fichier                        */
	  sramBddEcrireFichier(ficCour,&tmp);
    	prec_pt = tmp.pt_deb;
  }

  /* --------------------------------
   * FIN DE cf3_init_fic_tra
   * --------------------------------     */
}

VOID cf3_fic_tra_print ()
{
  INT i;                        /* indice de boucle sur les fichiers */

  /* pour chaque fichier */
  for (i = 0; i < MAXFIC; i++)
    {
	T_zdf vct_bdd;
	  sramBddLireFichier(i,&vct_bdd);
 		printf("Type de sequencement : %c\n",vct_bdd.ty_seq);
		printf("--Taille enreg : %ld par max enreg : %ld\n",vct_bdd.sz_enr,
						vct_bdd.max_seq);
		printf("--Nb ecr enreg : %ld ecr cour enreg : %ld\n",vct_bdd.nb_ecr,
						vct_bdd.num_ecr);
		printf("--Ad deb : %#0x cour %#0x prem %#0x\n",
						(unsigned int)vct_bdd.pt_deb,
						(unsigned int)vct_bdd.pt_ecr,
						(unsigned int)vct_bdd.pt_prem);
	}
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cf3_szenrfic                                           /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 08/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf3.c                                            /
/-------------------------------------------------------------------------DOC*/

INT cf3_szenrfic (INT fich, INT remplissage)
{
INT i,k;
  struct sz_enreg sz_enreg;

/* on initialise la taille des enregistrements */
  sz_enreg.sz_rec = 0;
  sz_enreg.sz_tete = 0;

  for (i = 0; i < NBCANAL; i++)
    {
      for (k = 0; k < MAXDATA; k++)
        {
          sz_enreg.canal[i].sz_donn_rec[k] = 0;
          sz_enreg.canal[i].sz_donn_em[k] = 0;
        }
      sz_enreg.canal[i].sz_canal = 0;
    }

  for (k = 0; k < MAXDATA; k++)
    {
      sz_enreg.nb_donn[k] = 0;
    }
  /* suivant le numero du fichier */
  switch (fich)
    {
    case F_IC:
    case F_IM:
    case F_IP:
    case F_IQ:
    case F_IR:
    case F_IA:
      sz_enreg.sz_rec = 1024;
      break;
    }                                  /* endswitch (fich)                                     */
  /* pour le nombre de donnees par
   * ligne */

  return (sz_enreg.sz_rec);
}
