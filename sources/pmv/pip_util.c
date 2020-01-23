/* **************************************************************************
 * Module : pip_util.c
 * Createur :  X.Gaillard
 * Date : Le 22 Novembre 2005
 * Objet : fonctions utilitaires pour la gestion des modules et caissons du
 * PIP.
 * ************************************************************************	*/

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */

#include <string.h>
#include "standard.h"

#include "mon_def.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "pip_util.h"
#include "lcr_cftp.h"
#include "lcr_cfes.h"
#include "lcr_cfet.h"
#include "lcr_cfpp.h"
#include "lcr_cfpl.h"
#include "lcr_cfsl.h"


#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

static char *pipSymbolesInterdits[]=
	{"AM","AF","CL","AT","EC","DV","R","ID",
	 "HR0","HR1","DT0","DT1","TP0",
	 "HR0/B","HR1/B","DT0/B","DT1/B","TP0/B",
	  NULL};


/* --------------------------------
 * pip_car_set_def
 * ===============
 * Positionnment de la valeur du
 * defaut caractere.
 * --------------------------------	*/

void pip_car_set_def(int index,BYTE valeur)
{
	pip_act_car[index].def_car=valeur;
}

/* --------------------------------
 * pip_dv_imprimer
 * ===============
 * Impression de la duree de validite
 * a afficher.
 * --------------------------------	*/

int pip_dv_imprimer(STRING buffer,long val,int fmtLong)
{
int j=0;
	if(0==val&&(0==fmtLong))
	{
		buffer[j++]='0';
	}
	else
	{
		j += sprintf (&buffer[j], "%02ld:%02ld:%02ld",
             val/3600, (val % 3600) / 60, (val % 3600) % 60);
	}
	return j;
}

/* --------------------------------
 * pip_clig_set
 * ============
 * Positionnement des parametres de
 * clignotement.
 * --------------------------------	*/

void pip_clig_set(T_pip_clig *clig,UINT8 type,int val1,int val2)
{
	clig->type=type;
	clig->val[0]=(val1);
	clig->val[1]=(val2);
}

int pip_clig_imprimer(STRING buffer,T_pip_clig *clig)
{
int nbCar=0;
	switch(clig->type)
	{
	case PIP_AUCUN:
		buffer[nbCar++]='0';
		break;
	case PIP_DEFAUT:
		buffer[nbCar++]='D';
		break;
	case PIP_ACTIF:
		nbCar=sprintf(buffer,"%d/%d",clig->val[0],clig->val[1]);
		break;
	}
	return nbCar;
}

/* --------------------------------
 * pip_alt_set
 * ============
 * Positionnement des parametres de
 * clignotement.
 * --------------------------------	*/

void pip_alt_set(T_pip_alt *alt,UINT8 type,int val1,int val2,int val3,int val4)
{
	alt->type=type;
	alt->val[0]=(val1);
	alt->val[1]=(val2);
	alt->val[2]=(val3);
	alt->val[3]=(val4);
}

int pip_alt_imprimer(STRING buffer,T_pip_alt *alt)
{
int nbCar=0;
	switch(alt->type)
	{
	default :
	case PIP_AUCUN:
		buffer[nbCar++]='0';
		break;
	case PIP_DEFAUT:
		buffer[nbCar++]='D';
		break;
	case PIP_ACTIF:
		nbCar=sprintf(buffer,"%d/%d/%d/%d",
						alt->val[0],alt->val[1],alt->val[2],alt->val[3]);
		break;
	}
	return nbCar;
}

/* --------------------------------
 * pip_init
 * ========
 * Reinitialisation des parametres
 * de configuration du pip.
 * --------------------------------	*/

void pip_init(bool raz)
{
	lcr_cfpl_init ();
 	lcr_cftp_init (raz);
	lcr_cfet_init ();
   	lcr_cfpp_init ();
    lcr_cfsl_init ();
}

/* --------------------------------
 * pip_util_ind_ctrl
 * =================
 * La fonction controle qu'au moins
 * un caisson correspond e la
 * description donnee.
 * --------------------------------	*/

int pip_util_ind_ctrl (int mod, int cai)
{
  int val = 0;
  if (0 != pip_nb_topo)
    {
      val =
        (pip_util_ind_deb_topo (mod, cai) < pip_util_ind_fin_topo (mod, cai) ? 1 : 0);
    }
  else
    {
      val = (pip_util_ind_deb_const (mod, cai) <
             pip_util_ind_fin_const (mod, cai) ? 1 : 0);
    }                                  /* endif(0!=pip_nb_topo) */
  return val;
}

int pip_util_ind_ctrl_topo (int mod, int cai)
{
  int val = 0;
  if (0 != pip_nb_topo)
    {
      val =
        (pip_util_ind_deb_topo (mod, cai) < pip_util_ind_fin_topo (mod, cai) ? 1 : 0);
    }
  return val;
}

int pip_util_ind_deb(int mod,int cais)
{
int index;
		if(pip_nb_topo)
		{
			index=pip_util_ind_deb_topo(mod,cais);
		}
		else
		{
			index=pip_util_ind_deb_const(mod,cais);
		}
		return index;
	}

int pip_util_ind_fin(int mod,int cais)
{
int index;
		if(pip_nb_topo)
		{
			index=pip_util_ind_fin_topo(mod,cais);
		}
		else
		{
			index=pip_util_ind_fin_const(mod,cais);
		}
		return index;
	}

int pip_util_ind_caisson(int ind)
{
int index=ind;
	if(pip_nb_topo)
	{
		index=pip_cf_topo[ind].indexCaisson;
	}
	return index;
}


/* --------------------------------
 * pip_util_ind_deb_const
 * ====================
 * Recuperation du premier caisson
 * --------------------------------	*/

int pip_util_ind_deb_const (int mod, int cais)
{
  int index = 0;
  if (mod != NN_AFF)
    {
      int indModule = 0;
      while ((pip_cf_module[indModule].id_module != mod)
             && (indModule < pip_nb_module))
        {
          indModule++;
        }
      if (indModule < pip_nb_module)
        {
          index = pip_cf_module[indModule].num_caisson;
          if ((N_AFF != cais)&&(-1!=cais))
            {
              while ((pip_cf_caisson[index].caisson != cais)
                     && (index < pip_nb_caisson))
                {
                  index++;
                }
            }
        }
    }
  return index;
}

int pip_util_ind_fin_const (int mod, int cais)
{
  int index = 0;
  if (mod != NN_AFF)
    {
      int indModule = 0;
      while ((pip_cf_module[indModule].id_module != mod)
             && (indModule < pip_nb_module))
        {
          indModule++;
        }
      if (indModule < pip_nb_module)
        {
          index = pip_cf_module[indModule].num_caisson +
            pip_cf_module[indModule].nb_caisson;
          if ((N_AFF != cais)&&(-1!=cais))
            {
              while ((pip_cf_caisson[index - 1].caisson != cais)
                     && (index > 0))
                {
                  index--;
                }
            }
        }
    }
  else
    {
      index = pip_nb_caisson;
    }
  return index;
}

int pip_util_ind_deb_topo (int mod, int cais)
{
  int index = 0;
  if (mod != NN_AFF)
    {
      while ((pip_cf_topo[index].topoModule < mod) && (index < pip_nb_topo))
        {
          index++;
        }
      if (index < pip_nb_topo)
        {
          if ((N_AFF != cais)&&(-1!=cais))
            {
              while ((pip_cf_topo[index].topoCaisson < cais)
                     && (index < pip_nb_topo))
                {
                  index++;
                }
            }
        }
    }
  return index;
}

/* --------------------------------
 * pip_util_ind_fin_topo
 * ================
 * Recherche de l'index de fin d'une
 * plage de modules dans la table
 * des equivalences topologiques.
 * --------------------------------	*/

int pip_util_ind_fin_topo (int mod, int cais)
{
  int index = pip_nb_topo;
  if (mod != NN_AFF)
    {
      while ((pip_cf_topo[index - 1].topoModule > mod) && (index > 0))
        {
          index--;
        }
      if (index > 0)
        {
          if ((N_AFF != cais)&&(-1!=cais))
            {
              while ((pip_cf_topo[index - 1].topoCaisson > cais)
                     && (index > 0))
                {
                  index--;
                }
            }
        }
    }
  return index;
}

/* --------------------------------
 * pip_util_test_nom_symbole
 * =========================
 * Controle de la validite du nom
 * d'un symbole.
 * --------------------------------	*/

int pip_util_test_nom_symbole(char *nom)
{
int retour=0;
int longueur=strlen(nom);
  /* Controle du nombre de caracteres, de 1 e LG_SYMB max */
	if((0<longueur)&&(LG_SYMB>=longueur))
	{
	int indice;
		retour=1;
	for(indice=0;(indice<longueur)&&retour;indice++)
	{
	char carCour=nom[indice];
		if(pip_util_j3(carCour)==0)
		{
			retour=0;
		}
	}
	if(retour)
	{
	int indice=0;
	while((pipSymbolesInterdits[indice]!=NULL)&&(retour))
	{
			if(strncmp(nom,pipSymbolesInterdits[indice],LG_SYMB)==0)
			{
				printDebug("Le symbole %s est interdit\n",nom);
				retour=0;
			}
			indice++;

	}
	}
  /* Controle de la nature des caracteres, ils doivent
   * appartenir e J8. */
	}
	return retour;
}

/* --------------------------------
 * pip_util_test_symbole
 * =====================
 * La methode permet de controler que
 * le parametre est effectivement un
 * symbole defini pour les modules
 * designes.
 * TODO : Ajouter le parametre sur
 * type de symbole recherche.
 * --------------------------------	*/

int pip_util_test_symbole (int mod, int cais, char *param)
{
  int retour = 0;
  /* Controle du nombre de caracteres, de 1 e 8 max */
  /* Controle de la nature des caracteres, ils doivent
   * appartenir e J8. */
  	retour=pip_util_test_nom_symbole(param);
	if(retour)
	{
  /* TODO : Enfin, on verifie que le symbole est defini pour
   * les caissons designes :
   * - Recherche pour le couple module/caisson designe,
   * - Sinon recherche pour les caissons du module,
   * - Sinon recherche pour tous les modules. */

  /* Recherche du premier et du dernier index et verification
   * de la presence du symbole pour chacun des caissons definis */
			int indCour;
			int indexDeb=pip_util_ind_deb_topo(mod,cais);
			int indexFin=pip_util_ind_fin_topo(mod,cais);
			if(indexDeb<indexFin)
			{
			for(indCour=indexDeb;(indCour<indexFin)&&(0!=retour);indCour++)
			{
				if(-1==cfes_chercher_type(pip_cf_topo[indCour].topoModule,
										pip_cf_topo[indCour].topoCaisson,
										param,PIP_EQUIV_AFF))
				{
						printDebug("Pas d'equivalence AFF %s pour %d.%d\n",
										param,pip_cf_topo[indCour].topoModule,
										pip_cf_topo[indCour].topoCaisson);
						retour=0;
				}
			}
			}
			else
			{
				/* Cas ou l'equivalence topo n'est pas definie. */
				retour=0;
			}
	}
  return retour;
}

/* --------------------------------
 * pip_util_test_symbole_ec
 * ========================
 * La methode permet de controler que
 * le parametre est effectivement un
 * symbole defini pour les modules
 * designes.
 * --------------------------------	*/

int pip_util_test_symbole_ec (int mod, int cais, char *param)
{
  int retour = 0;
  /* Controle du nombre de caracteres, de 1 e 8 max */
  /* Controle de la nature des caracteres, ils doivent
   * appartenir e J8. */
  	retour=pip_util_test_nom_symbole(param);
	if(retour)
	{
	/* Enfin, on verifie que le symbole est defini pour
   * les caissons designes pour l'eclairage. */
  /* Recherche du premier et du dernier index et verification
   * de la presence du symbole pour chacun des caissons definis */
	int indCour;
	int indexDeb=pip_util_ind_deb_topo(mod,cais);
	int indexFin=pip_util_ind_fin_topo(mod,cais);
		if(indexDeb<indexFin)
		{
			for(indCour=indexDeb;(indCour<indexFin)&&(0!=retour);indCour++)
			{
			int tMod=pip_cf_topo[indCour].topoModule;
			int tCai=pip_cf_topo[indCour].topoCaisson;
				if(-1==cfes_trouver_type(tMod,tCai,param,PIP_EQUIV_EC))
				{
						printDebug("Pas d'equivalence EC %s pour %d.%d\n",
										param,tMod,tCai);
						retour=0;
				}
			}
		}
		else
		{
			retour=0;
		}
	}
  return retour;
}

/* --------------------------------
 * pip_util_test_numerique
 * =====================
 * La methode permet de controler que
 * le parametre est effectivement un
 * symbole defini pour les modules
 * designes.
 * --------------------------------	*/

int pip_util_test_numerique (int mod, int cais, char *param)
{
  int retour = 0;
  int longueur=strlen(param);
  /* Controle de la nature des caracteres, on ne doit avoir
   * que des chiffres. */
  if(0<longueur)
  {
  int indCour;
  	retour=1;
  	for(indCour=0;(indCour<longueur)&&(retour);indCour++)
	{
	char carCour=param[indCour];
		if(('0'>carCour)||('9'<carCour))
		{
			retour=0;
		}
	}
  }

  if(retour)
  {
  /* TODO : Enfin, on verifie que le symbole est defini pour
   * les caissons designes. */
  /* Recherche du premier et du dernier index et verification
   * de la presence du symbole pour chacun des caissons definis */
  }
  return retour;
}
/* ----------------------------------
 * pip_util_g2_accent
 * ==================
 * Le caractere est un caractere
 * accentue du jeu g2.
 * --------------------------------	*/

int pip_util_g2_accent (char carCour)
{
  int retour = 0;
  /* if ((0x40 <= carCour) && (0x4F >= carCour))
    {
      retour = 1;
    } */
  return retour;
}

/* ----------------------------------
 * pip_util_g2
 * ===========
 * Le caractere est un caractere
 * du jeu g2.
 * --------------------------------	*/

int pip_util_g2 (char carCour)
{
  int retour = 0;
  if ((0x21 <= carCour) && (0x7E >= carCour))
    {
      retour = 1;
    }
  return retour;
}

int pip_util_j3 (char carCour)
{
  int retour = 0;
  if (((0x40 <= carCour) && (0x7E >= carCour))
      || (('0' <= carCour) && ('9' >= carCour)))
    {
      retour = 1;
    }
 	return retour;
}
int pip_util_j6 (char carCour)
{
  int retour = 0;
  /*
  if (((0x40 <= carCour) && ('Z' >= carCour))
      || (('a' <= carCour) && (0x7E >= carCour))
      || (('0' <= carCour) && ('9' >= carCour)))
	  */
  if ((0x20 <= carCour) && (0x7F >= (unsigned char)carCour))
    {
      retour = 1;
    }
  else
    {
      switch (carCour)
        {
        case '>':
        case '<':
        case 0x5c:
        case 0x5e:
        case ' ':
        case '(':
        case ')':
        case '-':
        case '.':
        case '/':
          retour = 1;
          break;
        default:
          break;
        }
    }
  return retour;
}

/* --------------------------------
 * pip_util_test_litteral
 * ======================
 * La methode permet de controler que
 * l'on est bien en presence d'un texte
 * litteral que l'on va pouvoir afficher
 * sur un caisson de type alphanumerique.
 * --------------------------------	*/

int pip_util_test_litteral (int mod, int cais, char *param)
{
  int retour = 0;
  int longueur = strlen (param);
  /* Pour le litteral, le controle doit etre le suivant :
   * - verifier que l'on a bien des guillemets en debut et
   *   en fin de message,
   * - controler que les caracteres definis dans le texte
   *   sont corrects,  c'est e dire qu'ils appartiennent au
   *   jeu J6 au au jeu G2 (avec pour prefixe le EM). */
  if (2 <= longueur)
    {
      if (('"' == param[0]) && ('"' == param[longueur - 1]))
        {
          int indice = 1;
          int nbCar = 0;        /* Nb car a afficher... */
          retour = 1;
          while ((indice < (longueur - 1)) && retour)
            {
              char carCour = param[indice++];
              if (0x19 == carCour)
                {
                  carCour = param[indice++];
                  if (pip_util_g2 (carCour))
                    {
                      if (!pip_util_g2_accent (carCour))
                        {
                          nbCar++;
                        }
                    }
                  else
                    {
                      printDebug ("Erreur le caractere %#0x"
                                  "	n'appartient pas a G2\n", carCour);
                      retour = 0;
                    }
                }
              else
                {
                  if (pip_util_j6 (carCour))
                    {
                      nbCar++;
                    }
                  else
                    {
                      printDebug ("Erreur le caractere %#0x"
                                  "n'appartient pas a J6\n", carCour);
                      retour = 0;
                    }
                }


            }
		  if(pip_nb_topo)
		  {
		  int indexDeb=pip_util_ind_deb_topo(mod,cais);
		  int indexFin=pip_util_ind_fin_topo(mod,cais);
		  int indCour;
		  	for(indCour=indexDeb;(indCour<indexFin)&&(0!=retour);indCour++)
			{
				if(nbCar>pip_cf_caisson[pip_cf_topo[indCour].indexCaisson].nb_car)
				{
             		printDebug ("Erreur trop de caracteres %d pour %d\n",
						nbCar,
				pip_cf_caisson[pip_cf_topo[indCour].indexCaisson].nb_car);
					retour=0;
				}
			}
		  }
		  else
		  {
		  int indexDeb=pip_util_ind_deb_const(mod,cais);
		  int indexFin=pip_util_ind_fin_const(mod,cais);
		  int indCour;
		  	for(indCour=indexDeb;(indCour<indexFin)&&(0!=retour);indCour++)
			{
		  	int nbCarCais=pip_cf_caisson[indCour].nb_car;
				/* Cas particulier pour les modules de moins de 6 caracteres
				 * comme la maquette SISER. Il n'y a pas d'erreur si le
				 * nombre de caracteres est trop grand*/
				if((nbCar>nbCarCais)&&(nbCarCais>6))
				{
             		printDebug ("Erreur trop de caracteres %d pour %d\n",
						nbCar,pip_cf_caisson[indCour].nb_car);
					retour=0;
				}
			}
		  }
        }
	  	else
		{
				printDebug("pip_util_test_litteral, pb de guillemets\n");
			}
    }
  if (retour)
    {
      /* En meme temps, on peut controler que le nombre de caracteres
       * est conforme, c'est e dire qu'il n'est pas superieur au nombre
       * attentu par le caisson. */
      /* On controle que tous les caissons designes par la selection sont
       * de type alpha et qu'ils ont un nombre suffisant de caracteres. */
    }
  return retour;
}
