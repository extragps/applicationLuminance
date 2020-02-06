/*
 * LCR_CF6.c
 * ===================
 * FONCTION DU SOUS-MODULE : Regroupe les fonctions de traitement alertes
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: LCR_CF6.c,v $
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.4  2008/10/13 08:18:15  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.3  2008/09/12 14:57:09  xag
 * *** empty log message ***
 *
 * Revision 1.2  2008/07/04 17:05:06  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:04:44  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

#ifndef SANS_WORLDFIP
#include "fdmtime.h"
#include "man_olga.h"
#include "man_fdmi.h"
#include "usr_fip.h"
#include "mcs.h"
#include "usr_fip_var.h"
#include "usr_fip_init.h"
#include "usr_fip_mess.h"
#endif /* SANS_WORLDFIP */
#include "string.h"
#include "time.h"
#include "standard.h"                  /* redefinition des types                         */
#include "mon_inc.h"
#include "define.h"                    /* definition des constantes                      */
#include "xdg_def.h"                   /* definition des structures globales             */
#include "x01_str.h"                   /* definition des structures globales             */
#include "x01_var.h"                   /* definition des variables globales               */
#include "vct_str.h"                   /* definition des structures globales             */
#include "x01_vcth.h"                  /* definition des variables globales               */
#include "sir_dv1h.h"
#include "sir_dv2h.h"
#include "mon_debug.h"


#include "lcr_cf3h.h"
#include "lcr_cf6h.h"
#include "lcr_idfh.h"
#include "ted_prot.h"
#include "identLib.h"
#include "stAlLib.h"

#define DEBUG 0

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif


/* *************************************
 * PROTOTYPE DES FONCTIONS LOCALES
 * ************************************* */
LOCAL INT par_al (STRING, INT, INT *, INT *);
LOCAL INT par_st_al (STRING, INT, INT *);

/* *************************************
 * FONCTIONS DU MODULE
 * ************************************* */
/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_al                                                /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 10/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf6.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : modification des parametres alertes individuelles      /
/-------------------------------------------------------------------------DOC*/

LOCAL INT par_al (STRING buffer, INT taille, INT * init, INT * consomme)
{
  INT flag_err = 0;             /* flag d'erreur ds l'analyse de la question */
#ifndef SANS_WORLDFIP
  unsigned int i = 0;           /* variable de boucle */
  INT val;
  LONG lg_val;
  if (taille < 3)
    {
      printDebug ("Taille trop petite\n");
      flag_err = 1;
    }
  else
    {
      if (dv1_scmp (&buffer[i], (STRING) "ACT", 3))
        {
          i += 4;
          switch (buffer[i])
            {
            case 'O':
              vct_st_al.act = TRUE;
              break;
            case 'N':
              vct_st_al.act = FALSE;
              break;
            default:
              printDebug ("Valeur incorrecte pour ACT\n");
              flag_err = 6;
              break;
            }
          *init = TRUE;
        }
      else if (dv1_scmp (&buffer[i], (STRING) "NEUT", 4))
        {
          int nbCar;
          i += 5;
          nbCar = dv1_car (&buffer[i], taille - i, (INT) (' '));
          if (!dv1_atol (&buffer[i], nbCar, &lg_val))
            {
              printDebug ("Valeur incorrecte pour NEUT %d %s\n",
                          taille - i, &buffer[i]);
              flag_err = 6;
            }
          else
            {
              if (lg_val > 65535)
                {
                  flag_err = 6;
                }
              else
                {
                  i += nbCar;
                  vct_st_al.neut = (UINT) lg_val;
                }
            }
        }
      else if (dv1_scmp (&buffer[i], (STRING) "REP", 3))
        {
          int nbCar;
          i += 4;
          nbCar = dv1_car (&buffer[i], taille - i, (INT) (' '));
          if (!dv1_atoi (&buffer[i], nbCar, &val))
            {
              printDebug ("Valeur incorrecte pour REP\n");
              flag_err = 6;
            }
          else
            {
              if ((val < 1) || (val > 10))
                {
                  printDebug ("Pas dans les clous pour REP\n");
                  flag_err = 6;
                }
              else
                {
                  i += nbCar;
                  vct_st_al.rep = (unsigned char) val;
                }
            }
        }
      else if (dv1_scmp (&buffer[i], (STRING) "PORT", 4))
        {
          i += 5;
          switch (buffer[i])
            {
            case '1':
              vct_st_al.port = 0;
              flag_err = 4;
              break;
            case '2':
              vct_st_al.port = PORT_FIP;
              break;
            case '3':
              break;
            default:
              printDebug ("Valeur incorrecte pour PORT %c\n", buffer[i]);
              flag_err = 6;
              break;
            }                          /* endswitch (buffer[i])                        */
        }
      else if (dv1_scmp (&buffer[i], (STRING) "PROT", 4))
        {
          i += 4;
          if ((buffer[i] < '1') || (buffer[i] > ('0' + MAX_MESS_AL)))
            {
              printDebug ("Valeur incorrecte pour PROT %c\n", buffer[i]);
              flag_err = 1;
            }
          else
            {
              int k = buffer[i] - '1';
              i += 2;
              switch (buffer[i])
                {
                case '0':
                  if (2 != vct_st_al.port)
                    {
                      vct_st_al.mess[k].prot = MODE_TERMINAL;
                    }
                  else
                    {
                      printDebug ("Mauvaise valeur de prot 0\n"),
                        flag_err = 6;
                    }
                  break;
                case '1':
                  if (2 != vct_st_al.port)
                    {
                      vct_st_al.mess[k].prot = MODE_PROTEGE;
                      if (i + 1 < taille)
                        {
                          /* pour l'adresse */
                          i += 2;
                          if (taille - i == 3)
                            {
                              memcpy (vct_st_al.mess[k].adresse, &buffer[i],
                                      3);
                            }
                          else
                            flag_err = 1;
                        }
                    }
                  else
                    {
                      printDebug ("Mauvaise valeur de prot 1\n"),
                        flag_err = 6;
                    }
                  break;
                case '2':
                  if (2 != vct_st_al.port)
                    {
                      vct_st_al.mess[k].prot = MODE_TEST;
                      if (i + 1 < taille)
                        {
                          /* pour l'adresse */
                          i += 2;
                          if (taille - i == 3)
                            {
                              memcpy (vct_st_al.mess[i].adresse, &buffer[i],
                                      3);
                            }
                          else
                            flag_err = 1;
                        }
                    }
                  else
                    {
                      flag_err = 6;
                    }
                  break;
                case 'N':
                  vct_st_al.mess[k].prot = MODE_AUCUN;
#ifndef SANS_WORLDFIP
                  if (vct_st_al.port == PORT_FIP)
                    {
                      /* decodage de l'adresse WorldFip */
                      /* Regarder e partir de i+1 */
                      if (((strncmp (&buffer[i + 1], "/0", strlen ("/0")) ==
                            0) && (buffer[i + 1 + strlen ("/0")] != '.'))
                          ||
                          (strncmp
                           (&buffer[i + 1], "/0.0.0.0/0/0",
                            strlen ("/0.0.0.0/0/0")) == 0)
                          || (i + 1 == taille) || (buffer[i + 1] == ' '))
                        {
                          T_usr_fip_mess *mess = usr_fip_mess_lire_courant ();
                          if (NULL != mess)
                            {
                              vct_st_al.mess[k].adFip =
                                usr_fip_mess_get_dest (mess);
                              vct_st_al.mess[k].idFip =
                                usr_fip_mess_get_ident (mess);
                            }
                          else
                            {
                              flag_err = 6;
                            }
                        }
                      else
                        {
                          int ad1, ad2, ad3, ad4, mod;
                          unsigned long id;
                          int nbPar;
                          printDebug ("Lecture de l'adresse %s\n",
                                      &buffer[i + 1]);
                          nbPar =
                            sscanf (&buffer[i + 1], "/%d.%d.%d.%d/%d/%x",
                                    &ad1, &ad2, &ad3, &ad4, &mod, (unsigned int *)&id);
                          if (6 == nbPar)
                            {
                              if ((((ad1 < 0) || (ad1 > 255))
                                   || ((ad2 < 0) || (ad2 > 255))
                                   || ((ad3 < 0) || (ad3 > 255))
                                   || ((ad4 < 0) || (ad4 > 255)))
                                  || ((mod < 0) || (mod > 7))
                                  || ((id > 0xFFFFFF)))
                                {
                                  printDebug ("Adresse incorrecte %d\n",
                                              nbPar);
                                  flag_err = 6;
                                }
                              else
                                {
                                  ((unsigned char *) &vct_st_al.mess[k].
                                   adFip)[3] = (unsigned char) ad4;
                                  ((unsigned char *) &vct_st_al.mess[k].
                                   adFip)[2] = (unsigned char) ad3;
                                  ((unsigned char *) &vct_st_al.mess[k].
                                   adFip)[1] = (unsigned char) ad2;
                                  ((unsigned char *) &vct_st_al.mess[k].
                                   adFip)[0] = (unsigned char) ad1;
                                  vct_st_al.mess[k].idFip = id;
                                  ((unsigned char *) &vct_st_al.mess[k].
                                   idFip)[0] = (unsigned char) (mod << 5);
                                }
                            }
                          else
                            {
                              printDebug ("Adresse incorrecte %d\n", nbPar);
                              flag_err = 6;
                            }
                        }
                    }
#endif
                  break;

                default:
                  printDebug ("Protocole incorrect \n");
                  flag_err = 6;
                  break;
                }
            }
        }
    }                                  /* endif (taille < 3)                           */
  *consomme = i;
  /* ---------------------------------
   * FIN DE par_al
   * --------------------------------     */
#endif /* SANS_WORLDFIP */
  return (flag_err);
}


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : par_cf_al_ind                                          /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 12/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cf6.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : traitement des parametres de la commande lcr_cf_al     /
/-------------------------------------------------------------------------DOC*/
LOCAL INT par_st_al (STRING buffer, INT lg_par, INT * init)
{
  INT i = 0;                    /* indice de lecture de buffer               */
  INT fini;                     /* variable de controle de boucle            */
  INT flag_err = 0;             /* drapeau de detection d'erreur             */
  INT taille;                   /* taille d'un parametre de detecteur        */
  /* le premier caractere de buffer
   * correspond au debut du 1er parame-
   * tre si on est pas au bout du
   * buffer, on continue l'analyse */
  if (i < lg_par)
    {
      fini = FALSE;
      while (!fini)
        {
          int consomme;
          /* on recherche la fin du parametre
           * et de l'operateur logique */
          taille = lg_par - i;         /* dv1_car (&buffer[i],lg_par-i,(INT)(' ')); */
          flag_err = par_al (&buffer[i], taille, init, &consomme);
          /* si on a trouve une erreur, on ter-
           * mine le decodage */
          if (flag_err != 0)
            {
              fini = TRUE;
            }
          else
            fini = FALSE;
          /* on passe au parametre suivant */
          i += consomme;
          if (i < lg_par)
            {
              i += dv1_car (&buffer[i], lg_par - i, (INT) (' '));
            }
          /* on elimine les separateurs */
          if ((i += dv1_separateur (&buffer[i], lg_par - i)) >= lg_par)
            {
              fini = TRUE;
            }
        }                              /* endwhile (! fini)                            */
    }
  /* -------------------------------
   *       FIN DE par_st_al
   * --------------------------------     */
  return (flag_err);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : cf6_st_al                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 10/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_cfal.c                                           /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : execution de la commande lcr_cfal                      /
/-------------------------------------------------------------------------DOC*/
INT
cf6_st_al (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
           struct usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
  UINT8 acquit = FALSE;
  INT init = FALSE;
  INT i, j, fini;
  INT flag_err = 0;
  struct vct_st_al sv_vct_st_al;        /* sauvegarde du status des alertes */
  INT config = FALSE;
  /* on se place apres le nom de la
   * commande */
  printDebug ("La valeur de position est %d\n", position);
  i = position + 5;
  /* on elimine les separateurs */
  i += dv1_separateur (&buffer[i], lg_mess - i);
  /* si on est pas en fin de message,
   * on continue l'analyse */
  if (i < lg_mess)
    {
      /* teste si l'operateur est habilite */
      if (identLireAutorisationAvecNiveau (las, 0))
        {
          config = TRUE;
          /* on reinitialise le module de
           * gestion de l'alerte */
          if (((buffer[i] == 'S') || (buffer[i] == 'Z'))
              && ((lg_mess - i) <= 2))
            {
              init = TRUE;
              vct_st_al.act = FALSE;
              vct_st_al.neut = 400;
              /* Valeur par defaut pour la reponse
               * utilisee pour Fip.   */
              vct_st_al.rep = 1;
              vct_st_al.seq = 'B';
              vct_st_al.port = 3;      /* Changer par XG, valeur par de-       */
              /* faut precisee par la norme.          */
              for (j = 0; j < MAX_MESS_AL; j++)
                {
                  T_vct_st_al_mess *mess = &vct_st_al.mess[j];
                  mess->adresse[0] = x01_status1.cod[1];
                  mess->adresse[1] = x01_status1.cod[2];
                  mess->adresse[2] = x01_status1.cod[6];
                  mess->flag = FALSE;
                  mess->prot = MODE_AUCUN;
                  mess->adFip = 0;
                  mess->idFip = 0;
                  mess->nb_em = 0;
                  mess->nb_rep = 0;
                  mess->tp_em = 0;
                  mess->tp_rep = 0;
                }
              vct_st_al.mess[0].flag = TRUE;
              memcpy (vct_st_al.mess[0].txt_em, (STRING) "*", 1);
              vct_st_al.mess[0].nb_em = 1;
            }
          else
            {
              if (dv1_scmp (&buffer[i], (STRING) "TST", 3))
                {
                  acquit = TRUE;
                }
              else
                {
                  /* on sauvegarde le status des
                   * alertes  */
                  memcpy ((void *) &sv_vct_st_al,
                          (const void *) &vct_st_al,
                          sizeof (struct vct_st_al));
                  /* on elimine les separateurs */
                  i += dv1_separateur (&buffer[i], lg_mess - i);
                  flag_err = par_st_al (&buffer[i], lg_mess - i, &init);
                  if (flag_err != 0)
                    {
                      /* on restaure le status des alertes */
                      memcpy ((void *) &vct_st_al,
                              (const void *) &sv_vct_st_al,
                              sizeof (struct vct_st_al));
                    }
                }                      /* endif (dv1_scmp (&buffer[i],         */
            }                          /* endif (((buffer[i] == 'S') ||        */
        }
      else
        {
          flag_err = 3;
        }
    }
  if (acquit == FALSE)
    {
      /* si on a pas trouve d'erreur */
      if (flag_err == 0)
        {
          /* on renvoie le status des alertes */
          j = 0;
          fini = FALSE;
          /* configuration des alertes indivi-
           * duelles */
          memcpy (&buffer[j], "ST AL ", 6);
          j += 6;
          switch (vct_st_al.act)
            {
            case TRUE:
              memcpy (&buffer[j], "ACT=O ", 6);
              j += 6;
              break;
            case FALSE:
              memcpy (&buffer[j], "ACT=N ", 6);
              j += 6;
              break;
            }
          memcpy (&buffer[j], "NEUT=", 5);
          j += 5;
          j += dv1_itoa (&buffer[j], vct_st_al.neut, 0);
          buffer[j++] = ' ';
          memcpy (&buffer[j], "REP=", 4);
          j += 4;
          j += dv1_itoa (&buffer[j], vct_st_al.rep, 0);
          buffer[j++] = ' ';
          memcpy (&buffer[j], (STRING) "SEQ=", 4);
          j += 4;
          buffer[j++] = vct_st_al.seq;
          buffer[j++] = ' ';
          switch (vct_st_al.port)
            {
            case 0:
              memcpy (&buffer[j], (STRING) "PORT=1 ", 7);
              j += 7;
              break;
            case PORT_FIP:
              memcpy (&buffer[j], (STRING) "PORT=2 ", 7);
              j += 7;
              break;
            case 3:
              memcpy (&buffer[j], (STRING) "PORT=3 ", 7);
              j += 7;
              break;
            }
          for (i = 0; (i < MAX_MESS_AL) && (fini == FALSE); i++)
            {
              if (vct_st_al.mess[i].flag == TRUE)
                {
                  if (j >= MAXLCR)
                    {
                      tedi_send_bloc (las, mode, buffer, j, *bloc, FALSE,
                                      pt_mess);
                      if (++(*bloc) >= 10)
                        {
                          *bloc = 0;
                        }
                      j = 0;
                      if (vct_IT[las])
                        fini = TRUE;
                    }
                  if ((i == 0) || ((vct_st_al.mess[i].prot !=
                                    vct_st_al.mess[i - 1].prot)
                                   || (vct_st_al.mess[i].adresse[0] !=
                                       vct_st_al.mess[i - 1].adresse[0])
                                   || (vct_st_al.mess[i].adresse[1] !=
                                       vct_st_al.mess[i - 1].adresse[1])
                                   || (vct_st_al.mess[i].adresse[2] !=
                                       vct_st_al.mess[i - 1].adresse[2])
                                   || (vct_st_al.mess[i].idFip !=
                                       vct_st_al.mess[i - 1].idFip)
                                   || (vct_st_al.mess[i].adFip !=
                                       vct_st_al.mess[i - 1].adFip)))
                    {
                      switch (vct_st_al.mess[i].prot)
                        {
                        case MODE_AUCUN:
                          if (PORT_FIP == vct_st_al.port)
                            {
                              T_vct_st_al_mess *mess = &vct_st_al.mess[i];
                              unsigned long adFip = mess->adFip;
                              unsigned long idFip = mess->idFip;
                              j +=
                                sprintf (&buffer[j],
                                             "PROT=N/%d.%d.%d.%d/%d/%06X ",
                                             ((unsigned char *) &adFip)[0],
                                             ((unsigned char *) &adFip)[1],
                                             ((unsigned char *) &adFip)[2],
                                             ((unsigned char *) &adFip)[3],
                                             ((((unsigned char *) &idFip)[0])
                                              >> 5),
                                             (unsigned int) (idFip &
                                                              (0x00FFFFFF)));
                            }
                          else
                            {
                              memcpy (&buffer[j], "PROT=N ", 7);
                              j += 7;
                            }
                          break;
                        case MODE_TERMINAL:
                          memcpy (&buffer[j], "PROT=0 ", 7);
                          j += 7;
                          break;
                        case MODE_PROTEGE:
                          memcpy (&buffer[j], "PROT=1/", 7);
                          j += 7;
                          memcpy (&buffer[j], vct_st_al.mess[i].adresse, 3);
                          j += 3;
                          buffer[j++] = ' ';
                          break;
                        case MODE_TEST:
                          memcpy (&buffer[j], "PROT=2/", 7);
                          j += 7;
                          memcpy (&buffer[j], vct_st_al.mess[i].adresse, 3);
                          j += 3;
                          buffer[j++] = ' ';
                          break;
                        }
                    }
                  buffer[j++] = 'M';
                  buffer[j++] = '1' + (unsigned char) i;
                  buffer[j++] = '=';
                  memcpy (&buffer[j], vct_st_al.mess[i].txt_em,
                          vct_st_al.mess[i].nb_em);
                  j += vct_st_al.mess[i].nb_em;
                  buffer[j++] = '/';
                  memcpy (&buffer[j], vct_st_al.mess[i].txt_rep,
                          vct_st_al.mess[i].nb_rep);
                  j += vct_st_al.mess[i].nb_rep;
                  buffer[j++] = '/';
                  if (vct_st_al.mess[i].tp_em >= 0)
                    {
                      j += dv1_itoa (&buffer[j], vct_st_al.mess[i].tp_em, 0);
                    }
                  buffer[j++] = '/';
                  if (vct_st_al.mess[i].tp_rep >= 0)
                    {
                      j += dv1_itoa (&buffer[j], vct_st_al.mess[i].tp_rep, 0);
                    }
                  buffer[j++] = ' ';
                }                      /* endif (vct_st_al.mess[i].flag =      */
            }
          j--;
          if (!vct_IT[las])
            {
              if (flg_fin == TRUE)
                {
                  buffer[j++] = '\n';
                  buffer[j++] = '\r';
                  /* apres le dernier capteur on envoie
                   * la reponse */
                  tedi_send_bloc (las, mode, buffer, j, *bloc, FALSE,
                                  pt_mess);
                }
              else
                {
                  /* apres le dernier capteur on envoie
                   * la reponse */
                  tedi_send_bloc (las, mode, buffer, j, *bloc, TRUE, pt_mess);
                }
              if (++(*bloc) >= 10)
                {
                  *bloc = 0;
                }
            }
        }
      else
        {
          x01_cptr.erreur = (unsigned char) flag_err;
          tedi_erreur (las, mode);
          config = FALSE;
        }
    }
  else
    {
      /* on envoie un acquit a la question */
      /* Sauf pour l'heure si le port est
       * different de 2       */
      if (vct_st_al.port != 2)
        {
          x01_cptr.erreur = 4;
          tedi_erreur (las, mode);
          config = FALSE;
        }
      else
        {
          tedi_ctrl (las, mode, TRUE);
          /* on lance le test des alertes */
          stAlDemarrerTest ();
        }
    }
  if (init)
    {
      /* on reinitialise */
      idf_reinit (FALSE);
#ifdef RAD
      cf3_init_fic_mes (TRUE);
#endif
    }
  /* ---------------------------------
   * FIN DE cf6_st_al
   * --------------------------------     */
  return (config);
}
