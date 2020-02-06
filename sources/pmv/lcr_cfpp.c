/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_pmv10                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 14/02/1991                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles pour la con- /
/ figuration et l'activation des panneaux                                     /
/                                                                             /
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

#include <string.h>
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"
#include "sir_dv1h.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "tac_conf.h"
#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "x01_vcth.h"
#include "lcr_cfpp.h"
#include "lcr_util.h"
#include "pip_util.h"
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

LOCAL VOID cmd_exec_cfpp (T_pip_cmd_cfpp *);
LOCAL INT lcr_cmd_cfpp (STRING, INT, INT, INT *, INT *, INT *);



/* ---------------------------------
 * lcr_cfpp_analyse_cl
 * =================
 * --------------------------------	*/

static int lcr_cfpp_analyse_cl (char *param, T_pip_cmd_cfpp * cmd_p)
{
  int flag_err = 0;
  int t1 = -1;
  int t2 = -2;
  /* Les formes possibles pour at sont :
   * - t,
   * - t1/t2
   * - /t2/t1     */
  printDebug ("Dans lcr_cfpp_analyse_cl : #%s#\n", param);
  if ('/' == *param)
    {
      char *ptCour = param;
      int lgCour = strlen (param);
      ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
      ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t2);
      ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
      ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t1);
      if ((0 != lgCour) || (-1 == t1) || (-1 == t2))
        {
          printDebug ("Erreru detectee %d %d %d\n", lgCour, t1, t2);
          flag_err = 2;
        }
      else
        {
          printDebug ("Clignotement Cas 3 : %d/%d\n", t1, t2);
        }
    }
  else
    {
      char *ptCour = param;
      int lgCour = strlen (param);
      ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t1);
      if (0 == lgCour)
        {
          /* Il n'y a qu'un entier, il faut faire ce qui va bien */
          printDebug ("Clignotement Cas 1 : %d/%d\n", t1, t1);
          t2 = t1;
        }
      else
        {
          printDebug ("Clignotement analyse t2 : %d et ptCour %c\n", t1,
                      *ptCour);
          if ('/' == *ptCour)
            {
              ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
              ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t2);
              if ((0 != lgCour) || (-1 == t1) || (-1 == t2))
                {
                  printDebug ("Erreru detectee %d %d %d\n", lgCour, t1, t2);
                  flag_err = 2;
                }
              else
                {
                  printDebug ("Clignotement Cas 2 : %d/%d\n", t1, t2);
                }
            }
          else
            {
              /* Presence d'une erreur */
              printDebug ("Erreur analyse du clignotement : %d/%d\n", t1, t2);
              flag_err = 2;
            }
        }

    }
  if (0 == flag_err)
    {
      pip_clig_set (&cmd_p->clig, PIP_ACTIF, t1, t2);
      cmd_p->flag = 1;
    }
  return flag_err;

}

/* ---------------------------------
 * lcr_cfpp_analyse_at
 * =================
 * --------------------------------	*/

static int lcr_cfpp_analyse_at (char *param, T_pip_cmd_cfpp * cmd_p)
{
  int flag_err = 0;
  int t1, t2, t3, t4;
  /* Les formes possibles pour at sont :
   * - t,
   * - t1/t2/t3/t4
   * - /t2/t1/t4/t3       */
  if ('/' == *param)
    {
      char *ptCour = param;
      int lgCour = strlen (param);
      ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
      ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t2);
      ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
      ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t1);
      ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
      ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t4);
      ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
      ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t3);
      if ((0 != lgCour) || (-1 == t1) || (-1 == t2) || (-1 == t3)
          || (-1 == t4))
        {
          flag_err = 2;
        }
      else
        {
          printDebug ("Alternat Cas 3 : %d/%d/%d/%d\n", t1, t2, t3, t4);
        }
    }
  else
    {
      char *ptCour = param;
      int lgCour = strlen (param);
      ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t1);
      if (0 == lgCour)
        {
          /* Il n'y a qu'un entier, il faut faire ce qui va bien */
          printDebug ("Alternat Cas 1 : %d/0/%d/0\n", t1, t1);
          t3 = t1;
          t2 = 0;
          t4 = 0;
        }
      else
        {
          if ('/' == *ptCour)
            {
              ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
              ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t2);
              ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
              ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t3);
              ptCour = tst_passe_slash (ptCour, lgCour, &lgCour);
              ptCour = tst_lit_entier (ptCour, lgCour, &lgCour, &t4);
              if ((0 != lgCour) || (-1 == t1) || (-1 == t2) || (-1 == t3)
                  || (-1 == t4))
                {
                  flag_err = 2;
                }
              else
                {
                  printDebug ("Alternat Cas 2 : %d/%d/%d/%d\n", t1, t2, t3,
                              t4);
                }
            }
          else
            {
              /* Presence d'une erreur */
              flag_err = 2;
            }
        }

    }
  if (0 == flag_err)
    {
      pip_alt_set (&cmd_p->alt, PIP_ACTIF, t1, t2, t3, t4);
      cmd_p->flag = 1;
    }
  return flag_err;

}

/* ----------------------------------
 * lcr_cfpp_at
 * =========
 * Analyse de la duree de validite.
 * --------------------------------	*/


static STRING lcr_cfpp_at (STRING ptCour, INT lgCour, INT * lgSuiv,
                           T_pip_cmd_cfpp * cmd, int *flag_err)
{
  int erreur = 0;
  if ('0' == *ptCour)
    {
      ptCour++;
      lgCour--;
      cmd->alt.type = PIP_AUCUN;
      cmd->flag = 1;
    }
  else
    {
      /* Analyse de l'alternat de la forme : t, t1/t2/t3/t4 ou /t2/t1/t4/t3 */
      /* cas : t, deux fois la meme duree */
      /* cas 2 : t2 et t4 doivent etre e 0 */
      /* cas 3 : t2 et t4 doivent etre e 0 */
      char param[MAXLCR];
      ptCour = tst_lit_param_at (ptCour, lgCour, &lgCour, param);
      erreur = lcr_cfpp_analyse_at (param, cmd);
    }
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
  *lgSuiv = lgCour;
  *flag_err = erreur;
  return ptCour;
}

/* ----------------------------------
 * lcr_cfpp_cl
 * =========
 * Analyse de la duree de validite.
 * --------------------------------	*/

static STRING lcr_cfpp_cl (STRING ptCour, INT lgCour, INT * lgSuiv,
                           T_pip_cmd_cfpp * cmd, int *flag_err)
{
  int erreur = 0;
  printDebug ("Dans lcr_cfpp_cl\n");
  if ('0' == *ptCour)
    {
      ptCour++;
      lgCour--;
      cmd->clig.type = PIP_AUCUN;
      cmd->flag = 1;
    }
  else
    {
      /* Analyse du clignotement : t, t1/t2 ou /t1/t1....     */
      char param[MAXLCR];
      printDebug ("Analyse des parametres de clignotement\n");
      ptCour = tst_lit_param_at (ptCour, lgCour, &lgCour, param);
      erreur = lcr_cfpp_analyse_cl (param, cmd);

    }
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
  *lgSuiv = lgCour;
  *flag_err = erreur;
  return ptCour;
}

/* ----------------------------------
 * lcr_cfpp_dv
 * =========
 * Analyse de la duree de validite.
 * --------------------------------	*/

static STRING lcr_cfpp_dv (STRING ptCour, INT lgCour, INT * lgSuiv,
                           T_pip_cmd_cfpp * cmd, int *flag_err)
{
  int erreur = 0;
    {
      /* Analyse de l'eclairage : symbolique ou numerique     */
      long validite;
      int heureOk;
      ptCour = tst_lit_dv (ptCour, lgCour, &lgCour, &validite, &heureOk);
      if (heureOk)
        {
          printDebug ("La duree de validite est correcte %ld\n", validite);
          cmd->validite = validite;
          cmd->flag = 1;
        }
      else
        {
  if ('0' == *ptCour)
    {
      ptCour++;
      lgCour--;
      cmd->validite = 0;
      cmd->flag = 1;

    }
  else
          erreur = 3;
        }
    }

  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
  *lgSuiv = lgCour;
  *flag_err = erreur;
  return ptCour;
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_cfpp_init                                          /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 16/08/1993                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_pmv0.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande P de positionnement           /
/-------------------------------------------------------------------------DOC*/
void lcr_cfpp_init (void)
{
  INT i;

  /* Remise a zero du compteur de scrutation SPC */
  vct_tempo_spc=0;

  /* module */
  for (i = 0; i < NB_MODULE; i++)
    {
      /* surveillance du PC */
      pip_cf_module[i].spc = 0;

      /* hysteresis sur la commutation jour nuit */
      pip_cf_module[i].hysteresis_lum = HYSTERESIS_LUM;
    }
  /* caisson */
  for (i = 0; i < NB_CAISSON; i++)
    {
	T_pip_cf_caisson *caisson=&pip_cf_caisson[i];
      /* nombre de parametre de clignotement */
	  caisson->alt.type=PIP_AUCUN;
      caisson->clig.type = PIP_AUCUN;
      caisson->validite = 0L;
      caisson->sab_def_trans = 'O';
      caisson->forceLum = '0';
    }
}
void lcr_cfpp_standard (void)
{
  INT i;

  /* Remise a zero du compteur de scrutation SPC */
  vct_tempo_spc=900;

  /* module */
  for (i = 0; i < NB_MODULE; i++)
    {
      /* surveillance du PC */
      pip_cf_module[i].spc = 0;

      /* hysteresis sur la commutation jour nuit */
      pip_cf_module[i].hysteresis_lum = HYSTERESIS_LUM;
    }
  /* caisson */
  for (i = 0; i < NB_CAISSON; i++)
    {
	T_pip_cf_caisson *caisson=&pip_cf_caisson[i];
      /* nombre de parametre de clignotement */
      if(caisson->type==TYPE_ALPHA)
      {
	  caisson->alt.type=PIP_ACTIF;
      caisson->alt.val[0]=150;
      caisson->alt.val[1]=30;
      caisson->alt.val[2]=150;
      caisson->alt.val[3]=30;
      caisson->clig.type = PIP_ACTIF;
      caisson->clig.val[0]=50;
      caisson->clig.val[1]=50;
      caisson->validite = 600L;
      caisson->sab_def_trans = 'O';
      caisson->forceLum = '0';
    }
      else
      {
 	  caisson->alt.type=PIP_AUCUN;
      caisson->clig.type = PIP_AUCUN;
      caisson->validite = 0L;
      caisson->sab_def_trans = 'O';
      caisson->forceLum = '0';
      }
    }
}

static void cmd_init_cfpp (T_pip_cmd_cfpp * cmd)
{
  cmd->flag = FALSE;
  cmd->id_module = NN_AFF;
  cmd->caisson = N_AFF;
  cmd->princ = N_AFF;
  cmd->flag_id = 0;
  cmd->clig.type = N_AFF;
  cmd->alt.type = N_AFF;
  cmd->validite = -1L;
  cmd->hysteresis_lum = -1;
  cmd->sab_def_trans = -1;
  cmd->spc = -1;
  cmd->forceLum = 0;
  cmd->no_las = N_AFF;
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_cfpp                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 16/08/1993                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_pmv0.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande P de positionnement           /
/-------------------------------------------------------------------------DOC*/
LOCAL VOID cmd_exec_cfpp (T_pip_cmd_cfpp * cmd_cfpp)
{
  BYTE erreur;

  erreur = FALSE;

  /* Rechercher ici le numero de caisson concerne..... */
  if (0 != pip_nb_topo)
    {
      int indice;
      int deb =
        pip_util_ind_deb_topo (cmd_cfpp->id_module, cmd_cfpp->caisson);
      int fin =
        pip_util_ind_fin_topo (cmd_cfpp->id_module, cmd_cfpp->caisson);
      /* Controle de l'existance des amt */
      /* parcours de la liste des am concernes */
      /* Recuperation de l'index du caisson */
      printDebug ("cmd_exec_cfpp, AM=%d.%d deb %d fin %d\n",
                  cmd_cfpp->id_module, cmd_cfpp->caisson, deb, fin);
      for (indice = deb; indice < fin; indice++)
        {
          int indCaisson = pip_cf_topo[indice].indexCaisson;
          T_pip_cf_caisson *caisson = &pip_cf_caisson[indCaisson];
          if ((cmd_cfpp->caisson == N_AFF)
              || (pip_cf_topo[indice].topoCaisson== cmd_cfpp->caisson))
            {
              /* il y a une demande de modification de la duree de validite */
              if (cmd_cfpp->validite != -1L)
                {
                  caisson->validite = cmd_cfpp->validite;
                  printDebug ("cmd_exec_cfpp, validite %ld\n", caisson->validite);
                }
              if (N_AFF != cmd_cfpp->clig.type)
                {
				  switch(cmd_cfpp->clig.type)
				  {
				 	case PIP_ACTIF :
                  printDebug ("cmd_exec_cfpp, clignotement : %d/%d\n",
								  cmd_cfpp->clig.val[0],cmd_cfpp->clig.val[1]);
				  break;
					case PIP_AUCUN :
                  printDebug ("cmd_exec_cfpp, clignotement : 0\n");
break;
				  }
                  caisson->clig = cmd_cfpp->clig;
                }
               if (N_AFF != cmd_cfpp->alt.type)
                {
                  caisson->alt = cmd_cfpp->alt;
                }
              /* modification sabordage en cas de defaut trans
                 if (cmd_cfpp->sab_def_trans != -1)
                 {
                 caisson->sab_def_trans =
                 cmd_cfpp->sab_def_trans;
                 }
                 if (cmd_cfpp->forceLum)
                 {
                 caisson->forceLum = cmd_cfpp->forceLum;
                 }
               */
            }
          if (erreur)
            {
              x01_cptr.erreur = CPTRD_PARAM;
            }
        }

      /* on initialise la structure de demande de configuration */

    }
  else
    {
      printDebug ("Aucune equivalence topo definie!!!\n");
    }
  cmd_init_cfpp (cmd_cfpp);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cmd_cfpp                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 16/08/1993                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_pmv0.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande P de positionnement           /
/-------------------------------------------------------------------------DOC*/
LOCAL INT lcr_cmd_cfpp (STRING buffer, INT nbcar, INT las,
                        INT * mod, INT * cais, INT * config)
{
  T_pip_cmd_cfpp cmd_cfpp;
  /* on initialise la structure de demande de configuration */
  STRING ptCour = (char *) &buffer[0];
  INT lgCour = nbcar;
  int flag_err = 0;
  INT valeur = 0;
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);

  cmd_init_cfpp (&cmd_cfpp);
  cmd_cfpp.no_las = las;
  if (0 < lgCour)
    {
      int amOk;
      printDebug ("cmd_cfpp, il y a des parametres\n");
      /* Recuperation eventuelle du numero de module..... */
      if (dv1_scmp_and_skip (ptCour, lgCour, "AM=Z.Z", 0, &ptCour, &lgCour)
          || dv1_scmp_and_skip (ptCour, lgCour, "AM=z.z", 0, &ptCour,
                                &lgCour))
        {
          printDebug ("cmd_cfpp, c'est pour le spc\n");
          ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
          /* Traitement du SPC... */
          *mod = 0;
          if (0 < lgCour)
            {
              if (identLireAutorisationAvecNiveau (las, 0))
                {
                  /* Lecture du SPC=...   */
                  if (dv1_scmp_and_skip
                      (ptCour, lgCour, "SPC=", 0, &ptCour, &lgCour))
                    {
                      char param[MAXLCR];
                      ptCour = tst_lit_param (ptCour, lgCour, &lgCour, param);
                      if (0 == lgCour)
                        {
                          dv1_atoi (param, strlen (param), &valeur);
                          *config = TRUE;
                        }
                      else
                        {
                          flag_err = 3;
                        }
                      /* Lecture de la valeur du SPC et positionnement */


                    }
                  else
                    {
                      flag_err = 2;
                    }                  /* endif(dv1_scmp_and_sk  ur,"SPC=" */
                  if ((*config == TRUE) && (0 == flag_err))
                    {
                      vct_tempo_spc = valeur;
                    }
                }
              else
                {
                  flag_err = 5;
                  x01_cptr.erreur = CPTRD_PROTOCOLE;
                }
            }
        }
      else if (dv1_scmp_and_skip (ptCour, lgCour, "AM=", 0, &ptCour, &lgCour))
        {
          printDebug ("cmd_cfpp, il y a une am definie\n");
          ptCour = tst_lit_am_bis (ptCour, lgCour, &lgCour, mod, cais, &amOk);
          if (amOk)
            {


			  if(pip_util_ind_ctrl_topo(*mod,*cais))
			  {
              printDebug ("cmd_cfpp, l'am est OK %d.%d\n", *mod, *cais);
              ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
              if (0 < lgCour)
                {
                  if (identLireAutorisationAvecNiveau (las, 0))
                    {
                      cmd_cfpp.id_module = *mod;
                      cmd_cfpp.caisson = *cais;
                      
                      while((0<lgCour)&&(0==flag_err))
                      {

                      if (dv1_scmp_and_skip
                          (ptCour, lgCour, "AT=", 0, &ptCour, &lgCour))
                        {
                          printDebug ("parametre AT!!\n");
                          ptCour = lcr_cfpp_at (ptCour, lgCour, &lgCour,
                                                &cmd_cfpp, &flag_err);
                        }
                      else
                        if (dv1_scmp_and_skip
                            (ptCour, lgCour, "CL=", 0, &ptCour, &lgCour))
                        {
                          printDebug ("parametre CL!!\n");
                          ptCour = lcr_cfpp_cl (ptCour, lgCour, &lgCour,
                                                &cmd_cfpp, &flag_err);
                        }
                      else
                        if (dv1_scmp_and_skip
                            (ptCour, lgCour, "DV=", 0, &ptCour, &lgCour))
                        {
                          printDebug ("parametre DV!!\n");
                          ptCour = lcr_cfpp_dv (ptCour, lgCour, &lgCour,
                                                &cmd_cfpp, &flag_err);
                        }
                      else
                        if (dv1_scmp_and_skip
                            (ptCour, lgCour, "TRB=", 0, &ptCour, &lgCour))
                        {
                          /* Non traite pour l'heure */
                        }
                      else
                        if (dv1_scmp_and_skip
                            (ptCour, lgCour, "TRA=", 0, &ptCour, &lgCour))
                        {
                          /* Non traite pour l'heure */
                        }
                      else
                        {
                          flag_err = 2;
                        }
              ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);
                      }
                    }
                  else
                    {
                      flag_err = 5;
                      x01_cptr.erreur = CPTRD_PROTOCOLE;
                    }
                }
              else
                {
                  printDebug ("Lecture seule!!!\n");
                }
			  }
			  else
			  {
              	printDebug ("cmd_cfpp, l'am est incorrect %d.%d\n", *mod, *cais);
                 flag_err = 3;
			  }
            }
          else
            {
              flag_err = 3;
            }

        }
      else if (dv1_scmp_and_skip (ptCour, lgCour, "Z", 0, &ptCour, &lgCour))
	  {
		/* Reinitialisation des parametres pour tous les modules */
		lcr_cfpp_init();
	  }
      else if (dv1_scmp_and_skip (ptCour, lgCour, "S", 0, &ptCour, &lgCour))
	  {
		/* Valeurs par defaut pour tous les modules */
		lcr_cfpp_standard();
	  }
      else
        {
          /* Le parametre n'est pas reconnu. */
          flag_err = 2;
        }
    }


  if ((x01_cptr.erreur == CPTRD_OK) && (cmd_cfpp.flag))
    {
      /* on traite la commande */
      cmd_exec_cfpp (&cmd_cfpp);
    }
  if (flag_err)
    {
      x01_cptr.erreur = CPTRD_SYNTAXE;

      /* on restaure la config */
      tac_conf_cfg_lec_conf ();

    }
  x01_cptr.config = TEMPO_CONFIG;
  return flag_err;
}

INT cfpp_imprimer_zz (INT las, INT mode, STRING buffer,
                      INT * j, INT * bloc, struct usr_ztf * pt_mess)
{
  INT retour = 0;
  int lg = *j;
  lg += sprintf (&buffer[lg], "AM=z.z SPC=%d\n\r", vct_tempo_spc);
  *j = lg;
  return retour;
}

INT cfpp_imprimer_caisson (INT las, INT mode, STRING buffer,
                           INT * j, INT * bloc, struct usr_ztf * pt_mess,
                           INT indTopo)
{
  INT retour = 0;
  int lg = *j;
  T_pip_cf_topo *topo = &pip_cf_topo[indTopo];
  int indexCaisson = topo->indexCaisson;
  T_pip_cf_caisson *caisson = &pip_cf_caisson[indexCaisson];
  
  if(caisson->type==TYPE_ALPHA)
  {

  lg +=
    sprintf (&buffer[lg], "AM=%d",topo->topoModule);
    if(-1!=topo->topoCaisson)
    {
  		lg+=sprintf(&buffer[lg],".%d",  topo->topoCaisson);
    }
  /* Imprimer ici les autres parametres */
  lg += sprintf (&buffer[lg], " AT=");
  lg += pip_alt_imprimer (&buffer[lg], &caisson->alt);
  lg += sprintf (&buffer[lg], " CL=");
  lg += pip_clig_imprimer (&buffer[lg], &caisson->clig);
  lg += sprintf (&buffer[lg], " DV=");
  lg += pip_dv_imprimer (&buffer[lg], caisson->validite, 0);
  lg += sprintf (&buffer[lg], " TRA=0 TRB=0\n\r");

  *j = lg;
  tedi_test_bloc (las, mode, buffer, j, bloc, pt_mess);
  }
  return retour;
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : lcr_p                                                  /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/12/96                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_p.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : commande de positionnement d'un panneau                /
/-------------------------------------------------------------------------DOC*/

INT lcr_cfpp (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
              struct usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
  INT i;                  /* variable de boucle lecture du buffer */
  INT mod = NN_AFF;             /* Par defaut, tout module et tout caisson */
  INT cais = N_AFF;
  INT config = FALSE;
  INT flag_err = 0;

  printDebug ("lcr_cfpp\n");
  /* on se place apres le nom de la commande */
  i = position + 4;

  x01_cptr.erreur = CPTRD_OK;

  /* si on est pas en fin de message, on continue l'analyse */
  if (i + 1 < lg_mess)
    {
      /* on traite la commande */
      flag_err =
        lcr_cmd_cfpp (&buffer[i], lg_mess - i, las, &mod, &cais, &config);
    }

  printDebug ("L'erreur est : %d et flag_err %d \n", x01_cptr.erreur,
              flag_err);
  if (x01_cptr.erreur == CPTRD_OK)
    {
      /* on traite l'etat des panneaux */
      /* on initialise le numero de bloc de reponse */
      int j = 0;
      int deb, fin;
      int indCour;

      j += dv1_scpy (&buffer[j], "CFPP\n\r", 0);
      /* Il s'agit ici de faire ici la liste des valeurs par defaut
       * pour les module topologiques. */
      /* On utilise potentiellement la liste des modules pour lesquels on a
       * passe la commande. */
	  if(0<pip_nb_topo)
	  {
      deb = pip_util_ind_deb (mod, cais);
      fin = pip_util_ind_fin (mod, cais);
      for (indCour = deb; indCour < fin; indCour++)
        {
          cfpp_imprimer_caisson (las, mode, buffer, &j, bloc, pt_mess,
                                 indCour);
        }
	  }

      /* Traiter le cas particulier du module z.z */
      /* On va considerer qu'il s'agit du numero de module 0 */
      if ((0 == mod) || (NN_AFF == mod))
        {
          cfpp_imprimer_zz (las, mode, buffer, &j, bloc, pt_mess);
        }

      tedi_send_bloc (las, mode, buffer, j, *bloc, TRUE, pt_mess);

/*
      while ((i < pip_nb_module) && (!fini))
        {
          k = pip_cf_module[i].num_caisson;
          while ((k <
                  pip_cf_module[i].num_caisson + pip_cf_module[i].nb_caisson)
                 && (!fini))
            {
              if (j >= 150)
                {
                  tedi_send_bloc (las, mode, buffer, j, bloc, FALSE, pt_mess);

                  if (++bloc >= 10)
                    bloc = 0;
                  j = 0;

                  if (vct_IT[las])
                    fini = TRUE;
                }
              j +=
                dv1_sprintf (&buffer[j], "AM=%d.%d",
                             (INT) (pip_cf_module[i].id_module),
                             (INT) (pip_cf_caisson[k].caisson));

              if (pip_cf_caisson[k].validite != 0L)
                {
                  j +=
                    dv1_sprintf (&buffer[j], " DV=%02d:%02d:%02d",
                                 (INT) (pip_cf_caisson[k].validite / 3600L),
                                 (INT) ((pip_cf_caisson[k].validite % 3600) /
                                        60),
                                 (INT) ((pip_cf_caisson[k].validite % 3600) %
                                        60));
                }
              else
                {
                  j += dv1_scpy (&buffer[j], " DV=0", 0);
                }

              j += dv1_scpy (&buffer[j], " CL=", 0);
              for (l = 0; l < pip_cf_caisson[k].nb_clig; l++)
                {
                  j +=
                    dv1_sprintf (&buffer[j], "%d/",
                                 pip_cf_caisson[k].tp_clig[l] * 10);
                }
              j--;

              j +=
                dv1_sprintf (&buffer[j], " HYS=%d",
                             pip_cf_module[i].hysteresis_lum - 1);

              j += dv1_sprintf (&buffer[j], " SPC=%d", pip_cf_module[i].spc);

              j += dv1_scpy (&buffer[j], " SDT=", 0);
              buffer[j++] = pip_cf_caisson[k].sab_def_trans;

              j += dv1_scpy (&buffer[j], " FL=", 0);
              buffer[j++] = pip_cf_caisson[k].forceLum;

              if (pip_cf_caisson[k].caisson_princ)
                {
                  j += dv1_scpy (&buffer[j], " PR=O\n\r", 0);
                }
              else
                {
                  j += dv1_scpy (&buffer[j], " PR=N\n\r", 0);
                }
              k++;
            }
          i++;
        }
*/

    }
  /* sinon, on signale l'erreur */
  else
    {
      config = FALSE;
      tedi_ctrl_ext (las, mode, flag_err);
    }
  return config;
}
