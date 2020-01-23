/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cfet                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/11/2005                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Configuration des equivalences topologiques.	  /
/                                                                             /
/-----------------------------------------------------------------------------/
/                                HISTORIQUE                                   /
/-----------------------------------------------------------------------------/
/ DATE   |   AUTEUR    | VERSION | No de fiche de modification                /
/-----------------------------------------------------------------------------/
/        |             |         |                                            /
/-----------------------------------------------------------------------------/
/        |             |         |                                            /
/-----------------------------------------------------------------------------/
/                    INTERFACES EXTERNES DU SOUS-MODULE                       /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES :                                                         /
/-----------------------------------------------------------------------------/
/ DONNEES EXPORTEES :                                                         /
/-----------------------------------------------------------------------------/
/ FONCTIONS IMPORTEES :                                                       /
/-----------------------------------------------------------------------------/
/ FONCTIONS EXPORTEES :                                                       /
/-----------------------------------------------------------------------------/
/               DONNEES ET FONCTIONS INTERNES DU SOUS-MODULE                  /
/-----------------------------------------------------------------------------/
/ DONNEES INTERNES :                                                          /
/-----------------------------------------------------------------------------/
/ FONCTIONS INTERNES :                                                        /
/-------------------------------------------------------------------------DOC*/

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include <string.h>
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "pip_def.h"
#include "pip_str.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "tac_conf.h"
#include "pip_str.h"
#include "pip_var.h"
#include "lcr_cfes.h"
#include "lcr_cfet.h"
#include "lcr_util.h"
#include "lcr_cftp.h"
#include "lcr_cfpp.h"
#include "identLib.h"


#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif


/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

static int cfet_nouveau (int mod, int cais, int index)
{
  T_pip_cf_caisson *caisson = &pip_cf_caisson[index];
  int indexTopo = caisson->indexTopo;
  if (-1 != indexTopo)
    {
      int indCour;
      /* Supprimer l'equivalence topologique afin
       * que la nouvelle soit correctement affectee */
      for (indCour = indexTopo; indCour < (pip_nb_topo - 1); indCour++)
        {
          pip_cf_topo[indCour] = pip_cf_topo[indCour + 1];
          pip_cf_caisson[pip_cf_topo[indCour].indexCaisson].indexTopo =
            indCour;
        }
      indexTopo = -1;
    }
  /* Recherche de l'index d'insertion de l'equivalence topologique
   * dans la table */
  {
    int index2;
    for (indexTopo = 0;
         (indexTopo < pip_nb_topo) &&
         ((pip_cf_topo[indexTopo].topoModule < mod) ||
	((pip_cf_topo[indexTopo].topoModule == mod)
             &&(pip_cf_topo[indexTopo].topoCaisson < cais)));
         indexTopo++);
    /* A priori pas la peine de tester que l'on
     * depasse le nombre max car on a le meme nombre
     * au max que de caisson configures. */
    for (index2 = pip_nb_topo; index2 > indexTopo; index2--)
      {
        pip_cf_topo[index2] = pip_cf_topo[index2 - 1];
        pip_cf_caisson[pip_cf_topo[index2].indexCaisson].indexTopo = index2;
      }
    pip_nb_topo++;
    caisson->indexTopo = indexTopo;
  }
  /* Traitement de l'equivalence topologique... */
  {
    T_pip_cf_topo *topo = &pip_cf_topo[indexTopo];
    topo->topoModule = mod;
    topo->topoCaisson = cais;
    topo->indexCaisson = index;
  }
  return indexTopo;
}

static int cfet_chercher (int module, int caisson)
{
  int index = -1;
  int indice;
  for (indice = 0; (indice < pip_nb_topo) && (-1 == index); indice++)
    {
      T_pip_cf_topo *topo = &pip_cf_topo[indice];
      if ((topo->topoModule == module) && (topo->topoCaisson == caisson))
        {
          index = indice;
        }
    }
  return index;
}

/* --------------------------------
 * cfet_ajouter
 * ============
 * Ajouter une equivalence topologique
 * --------------------------------	*/

static int cfet_ajouter (int modT, int caisT, int modC, int caisC)
{
  int flag_err = 1;
  if ((-1 != modC) && (-1 != caisC))
    {
      int indexCaisson = caisChercher (modC, caisC);
      if (-1 != indexCaisson)
        {
          int indexTopo = cfet_chercher (modT, caisT);
          if (-1 == indexTopo)
            {
              indexTopo = cfet_nouveau (modT, caisT, indexCaisson);
              flag_err = 0;
            }
          else
            {
              /* Si il y a incompatibilite, il y a erreur. On est dans le
               * cas ou l'equivalence est deje utilisee mais pour un
               * autre module.        */
              flag_err =
                (pip_cf_caisson[indexCaisson].indexTopo != indexTopo);
            }
        }
    }
  return flag_err;
}

/* --------------------------------
 * lcr_cfet_init
 * =========
 * Reinitialisation des equivalences
 * topologiques.
 * --------------------------------	*/

int lcr_cfet_init ()
{
  int indice;
  lcr_cfes_init ();
  lcr_cfpp_init ();
  pip_nb_topo = 0;
  for (indice = 0; indice < NB_CAISSON; indice++)
    {
      pip_cf_caisson[indice].indexTopo = -1;
    }
  return TRUE;
}


/* --------------------------------
 * lcr_cfet
 * ========
 * Fonction de lecture et de confi-
 * guration des equivalences topolo-
 * giques.
 * --------------------------------	*/

INT lcr_cfet (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
              T_usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{

  INT i;
  INT j;
  BYTE fini;
  INT config = FALSE;
  /* on se place apres le nom de la commande */
  char *ptCour = (char *) &buffer[position + strlen ("CFET")];
  int lgCour = lg_mess - position - strlen ("CFET");
  int flag_err = 0;
  int standard=0;
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);

  if (0 < lgCour)
    {
      /* on teste si l'operateur est habilite */
      if (identLireAutorisationAvecNiveau (las, 0))
        {
          T_pip_cf_topo sav_topo[NB_CAISSON];
          INT sav_index[NB_CAISSON];
          int indice;
          int sav_nb_topo = pip_nb_topo;
          for (indice = 0; indice < NB_CAISSON; indice++)
            {
              sav_topo[indice] = pip_cf_topo[indice];
              sav_index[indice] = pip_cf_caisson[indice].indexTopo;
            }
          printDebug ("Autorisation OK\n");
          /* XG : TODO : traitement de la demande de configuration des
           * equivalences topologiques. Pour l'instant, il y a erreur*/
          if (strncmp (ptCour, "Z", strlen ("Z")) == 0)
            {
              printDebug ("Traitement du Z\n");
              ptCour++;
              lgCour--;
              config = lcr_cfet_init ();
            }
          else if (strncmp (ptCour, "S", strlen ("S")) == 0)
            {
              config = TRUE;
              printDebug ("Traitement du S\n");
              ptCour++;
              lgCour--;
              config = lcr_cfet_init ();
              cfet_ajouter (1,1,1,1);
              cfet_ajouter (1,2,1,2);
              cfet_ajouter (9,-1,9,0);
              standard=1;
            }
          else if (strncmp (ptCour, "AM", strlen ("AM")) == 0)
            {
              printDebug ("Traitement du AM\n");
              ptCour += strlen ("AM");
              lgCour -= strlen ("AM");
              ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
              /* Si la longueur courante est nulle, il s'agit d'une
               * commande de lecture. */
              while ((0 < lgCour) && (0 == flag_err))
                {
                  /* Traitement des equivalences. Il s'agit de couples
                   * module/caisson = module/caisson.
                   * Pour qu'il n'y ait pas erreur il faut que la designation
                   * du caisson soit correcte et ensuite que l'equivalence soit
                   * syntaxiquement correcte. */
                  /* Lecture de l'am topologique */
                  int mod, cais, amOk;
                  config = TRUE;
                  ptCour =
                    tst_lit_am (ptCour, lgCour, &lgCour, &mod, &cais, &amOk);
                  if (amOk)
                    {
                      printDebug ("Am topo %d.%d", mod, cais);
                      /* Controle de validite d'analyse */
                      /* On regarde par exemple si l'amt n'est pas
                       * deje utilisee. */
                      if ('=' == *ptCour)
                        {
                          int modC, caisC;
                          ptCour++;
                          lgCour--;
                          ptCour = tst_lit_am (ptCour, lgCour, &lgCour,
                                               &modC, &caisC, &amOk);
                          if (amOk)
                            {
                              printDebug ("Am cons %d.%d", mod, cais);
                              flag_err =
                                cfet_ajouter (mod, cais, modC, caisC);
                            }
                          else
                            {
                              flag_err = 1;
                            }
                        }
                      else
                        {
                          flag_err = 1;                                                                                                                                                                                                                                                                                                                                                                                                                /* Erreur d'analyse. l'amt doit etre
                                                                                                                                                                                                                                                                                                                                                                                                                                                          suivie du =. */
                        }
                    }
                  else
                    {
                      printDebug ("Am incorrecte\n");
                      flag_err = 1;                                                                                                                                                                                                                                                                                                                                                                                                                    /* Erreur de lecture de l'AM    */
                    }                                                                                                                                                                                                                                                                                                                                                                                                                                  /* endif(amOk) */
                  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
                }                                                                                                                                                                                                                                                                                                                                                                                                                                      /* endwhile((0<lgCour)&&(0==fl */
              if (flag_err && (TRUE == config))
                {
                  /* Recuperation de la config */
                  pip_nb_topo = sav_nb_topo;
                  for (indice = 0; indice < NB_CAISSON; indice++)
                    {
                      pip_cf_topo[indice] = sav_topo[indice];
                      pip_cf_caisson[indice].indexTopo = sav_index[indice];
                    }

                }
            }
          else
            {
              flag_err = 1;                                                                                                                                                                                                                                                                                                                                                                                                                            /* Erreur parametre non reconnu. */
            }
          ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
          flag_err = (0 != lgCour) || flag_err;
        }
      else
        {
          x01_cptr.erreur = CPTRD_PROTOCOLE;
        }
    }

  /* si on a pas detecte d'erreur */
  if ((x01_cptr.erreur == CPTRD_OK) && (0 == flag_err))
    {
		if((FALSE==config)||(1==standard))
		{
      int indice = 0;
      i = 0;
      j = 0;
      fini = FALSE;
      j = sprintf (buffer, "CFET AM\n\r");
      for (indice = 0; indice < pip_nb_caisson; indice++)
        {
          T_pip_cf_caisson *caisson = &pip_cf_caisson[indice];
          /* Affichage de l'equivalence topologique */
          if (-1 != caisson->indexTopo)
            {
              int indexTopo = caisson->indexTopo;
              if (indexTopo < pip_nb_topo)
                {
                  T_pip_cf_topo *topo = &pip_cf_topo[indexTopo];
                  j += sprintf (&buffer[j], "%d", topo->topoModule);
                  if (-1 != topo->topoCaisson)
                    {
                      j += sprintf (&buffer[j], ".%d", topo->topoCaisson);
                    }
                }
            }
          j += sprintf (&buffer[j], "=%d.%d\n\r", caisson->id_module,
                            caisson->caisson);
          if (j >= MAXLCR)
            {
              tedi_send_bloc (las, mode, buffer, j, *bloc, FALSE, pt_mess);

              if (++(*bloc) >= 10)
                *bloc = 0;
              j = 0;

              if (vct_IT[las])
                fini = TRUE;
            }
        }
      if (!fini)
        {
//          if (TRUE == flg_fin)
//            {
//              buffer[j++] = '\n';
//              buffer[j++] = '\r';
//            }
          tedi_send_bloc (las, mode, buffer, j, *bloc,
                          (TRUE == flg_fin ? FALSE : TRUE), pt_mess);

          if (++(*bloc) >= 10)
            *bloc = 0;
          j = 0;

          if (vct_IT[las])
            fini = TRUE;
        }
		}
		else
		{
			tedi_send_bloc (las, mode, buffer, 0, *bloc,
                          (TRUE == flg_fin ? FALSE : TRUE), pt_mess);

		}
    }
  else
    {
      config = FALSE;
      tedi_ctrl_ext (las, mode, ((x01_cptr.erreur == CPTRD_PROTOCOLE) ? 5 : 2));
    }
  return config;

}
