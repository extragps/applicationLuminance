
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 14 Juillet 2001
 * Fichier	: stAlLib.h
 * Objet	: Fonctions utilisees dans le cadre du traitement des alertes.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,14Jui01, xag Creation du fichier.
 * ************************************************************************	*/

/* ********************************
 * LISTE DES INCLUDES
 * ********************************	*/

#ifdef VXWORKS
#include "vxWorks.h"
#include "vxworks.h"
#include "semLib.h"
#endif
#include "stdio.h"
#include "time.h"
#include "define.h"
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "mon_debug.h"
#include "mon_str.h"
#include "x01_vcth.h"
#ifndef SANS_WORLDFIP
#include "fdm.h"
#include "fdmtime.h"
#include "man_olga.h"
#include "man_fdmi.h"
#include "usr_fip.h"
#include "mcs.h"
#include "usr_fip_var.h"
#include "usr_fip_init.h"
#include "usr_fip_mess.h"
#endif
#include "stAlLib.h"
#include "eriLib.h"

/* ********************************
 * PROTOTYPE DES TYPES
 * ********************************	*/


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
 * VARIABLES LOCALES
 * ********************************	*/

T_stAlData stAlBloc;
T_stAlData *stAlData;


/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * stAlInit
 * ========
 * Initialisation de la structure 
 * utilisee dans le cadre des alertes.
 * --------------------------------	*/

void stAlInit (void)
{
  stAlData = &stAlBloc;
  stAlData->neutre = E_stAlDisponible;
  stAlData->etat = E_stAlLibre;
  stAlData->test = 0;
  stAlData->testEnCours = 0;
}

/* --------------------------------
 * stAlLancer
 * ==========
 * Demande de demarrage du mecanisme 
 * des alertes (sur arrivee d'une
 * alerte).
 * Retour de la fonction :
 * - la fonction retourne -1 si le
 * mecanisme d'alerte est neutralise.
 * --------------------------------	*/

int stAlLancer (void)
{
  int retour = -1;
  printDebug ("Dans stAlLancer\n");
  if ((vct_st_al.act == TRUE) && (stAlData->neutre != E_stAlNeutralise) &&
      (stAlData->etat == E_stAlLibre))
    {
      printDebug ("Alerte initialisee\n");
      stAlData->port = vct_st_al.port;
      stAlData->cptNeutre = vct_st_al.neut * 10;
      stAlData->neutre = E_stAlNeutralise;
      stAlData->etat = E_stAlLancer;
      stAlData->numRepet = 0;
      stAlData->numMess = 0;
      vct_ST_TR |= STATUS_TR_ALERTE;

      /* Par defaut, on considere que le
       * traitement de l'alerte est   
       * correct.     */
      stAlData->abouti = 1;

      retour = 0;
    }
  /* ---------------------------------
   * FIN DE stAlLancer
   * --------------------------------     */
  return (retour);
}

/* -------------------------------
 * stAlDemarrerTest
 * ================
 * --------------------------------	*/

void stAlDemarrerTest (void)
{
  printDebug ("Dans stAlDemarrerTest\n");
  stAlData->cptTest = 300;
  stAlData->test = 1;
  /* Positionner ici le ST ERI    */
  eriAjouter (E_eriTest, ERI_TST_ALERTE);
}

/* --------------------------------
 * stAlForcer
 * ==========
 * Demande de demarrage du mecanisme 
 * des alertes en mode test.
 * --------------------------------	*/

int stAlForcer (void)
{
  int retour;
  printDebug ("Dans stAlForcer\n");
  printDebug ("Alerte initialisee\n");
  stAlData->port = vct_st_al.port;
  stAlData->cptNeutre = vct_st_al.neut * 10;
  stAlData->neutre = E_stAlNeutralise;
  stAlData->etat = E_stAlLancer;
  stAlData->numRepet = 0;
  stAlData->numMess = 0;
  stAlData->testEnCours = 1;
  vct_ST_TR |= STATUS_TR_ALERTE;

  /* Par defaut, on considere que le
   * traitement de l'alerte est   
   * correct.     */
  stAlData->abouti = 1;

  retour = 0;
  /* ---------------------------------
   * FIN DE stAlLancer
   * --------------------------------     */
  return (retour);
}

/* ------------------------------------
 * stAlEmettreMessage
 * ==================
 * On passe au message suivant.
 * ------------------------------------	*/

void stAlEmettreMessage (T_stAlData * data)
{
#ifndef SANS_WORLDFIP
  T_usr_fip_mess *mess = usr_fip_mess_lire_st_al ();
  printDebug ("Dans stAlEmettreSuivant\n");
  /* Emission du message */
  if (NULL != mess)
    {
      T_vct_st_al_mess *bloc = &vct_st_al.mess[data->numMess];
      char buffer[200];
      int lgBuffer = 0;
      int indCour;
      data->numRepet++;
      printDebug ("Numero du message %d\n", data->numMess);
      usr_fip_mess_set_dest (mess, bloc->adFip);
      usr_fip_mess_set_ident (mess, bloc->idFip);
      usr_fip_mess_init_debut (mess);
      printDebug ("Nombre de caracteres a emettre %d\n", bloc->nb_em);
      for (indCour = 0; indCour < bloc->nb_em; indCour++)
        {
          if (bloc->txt_em[indCour] == '*')
            {
              memcpy (&buffer[lgBuffer], "TST ", 4);
              lgBuffer += 4;
              buffer[lgBuffer++] = x01_status1.cod[0];
              buffer[lgBuffer++] = x01_status1.cod[1];
              buffer[lgBuffer++] = x01_status1.cod[2];
              buffer[lgBuffer++] = x01_status1.cod[3];
              buffer[lgBuffer++] = x01_status1.cod[4];
              buffer[lgBuffer++] = x01_status1.cod[5];
              buffer[lgBuffer++] = x01_status1.cod[6];
            }
          else
            {
              buffer[lgBuffer++] = bloc->txt_em[indCour];
            }
        }
      buffer[lgBuffer] = 0;
      printDebug ("\nEmission du message %s", buffer);
      buffer[lgBuffer++] = 0x0d;
      usr_fip_mess_ecrire_rep (mess, buffer, lgBuffer);
      usr_fip_mess_send_message_mcs (mess, 0);
      if (0 != vct_st_al.mess[data->numMess].tp_rep)
        {
          data->etat = E_stAlAcquit;
          data->cptAcquit = vct_st_al.mess[data->numMess].tp_rep;
        }
      else
        {
          /* Pas d'acquitement attendu, on
           * passe au message suivant.            */
          stAlMessageSuivant (data);
        }
    }
  else
    {
      /* Increment d'un compteur pour signaler que le message n'a pu
       * etre emis. Au bout de 100, on fait une erreur */
      printDebug ("Le message n'a pu etre emis\n");
    }
  /* ---------------------------------
   * FIN DE stAlEnvoyerMessage
   * --------------------------------     */
#endif
}

/* --------------------------------
 * stAlMessageSuivant
 * ==================
 * --------------------------------	*/

void stAlMessageSuivant (T_stAlData * data)
{
  printDebug ("Dans stAlMessageSuivant\n");
  data->numMess++;
  if ((data->numMess >= 0) && (data->numMess < MAX_MESS_AL))
    {
      stAlData->numRepet = 0;
      data->etat = E_stAlLancer;
    }
  else
    {
      stAlData->etat = E_stAlLibre;
      if (stAlData->abouti == 1)
        {
          vct_ST_TR &= ~STATUS_TR_ALERTE;
        }
      if (data->testEnCours)
        {
          data->testEnCours = 0;
          eriSupprimer (E_eriTest, ERI_TST_ALERTE);
        }
      /* Si l'alerte a abouti, positionnment du 
       * status temps reel. */
    }                                  /* endif((data->numMess>=0)&&           */
}

/* --------------------------------
 * stAlTeste********************************r
 * ==========
 * Mecanisme de test des alertes.
 * Cette fonction est appelee par
 * la tache WorldFip.
 * Entree :
 * 	- le numero de port sur lequel
 * 	doit etre realise l'alerte.
 * --------------------------------	*/

void stAlTester (int port)
{
  T_stAlData *data = stAlData;
  if (port == data->port)
    {
      switch (data->etat)
        {
          /* Lancemenent d'une alerte     */
        case E_stAlLancer:
          printDebug ("stAlLancer\n");
          /* Verifire que le numero du message a lancer est correct */
          if ((data->numMess >= 0) && (data->numMess < MAX_MESS_AL))
            {
              if (TRUE == vct_st_al.mess[data->numMess].flag)
                {
                  /* Si le flag pour le numero de 
                   * message considere est a vrai, 
                   * on traite le message */
                  if (0 != vct_st_al.mess[data->numMess].tp_em)
                    {
                      data->etat = E_stAlAttente;
                      data->cptAttente = vct_st_al.mess[data->numMess].tp_em;
                    }
                  else
                    {
                      /* Emission du message */
                      stAlEmettreMessage (data);
                      /* Traitemement de la suite     */
                      if (0 != vct_st_al.mess[data->numMess].tp_rep)
                        {
                          data->etat = E_stAlAcquit;
                          data->cptAcquit =
                            vct_st_al.mess[data->numMess].tp_rep;

                        }
                      else
                        {
                          /* Pas d'acquitement attendu, on
                           * passe au message suivant.            */
                          stAlMessageSuivant (data);
                        }              /* endif(0!=vct_st_al.mess[data->nu     */
                    }                  /* endif(0!=vct_st_al.mess[data->nu     */
                }
              else
                {
                  /* Sinon, on passe au message           
                   * suivant et on sort direct de 
                   * la procedure, on traitera au 
                   * message suivant.     */
                  stAlMessageSuivant (data);
                }
            }
          else
            {
              /* Pas d'aboutissement de l'alerte, on se trouve dans un
               * cas impossible theoriquement */
            }
          break;
        case E_stAlAttente:
          printDebug ("+");
          if (data->cptAttente < 0)
            {
              stAlEmettreMessage (data);
            }
          else
            {
              /* C'est pour la prochaine      */
            }
          break;
        case E_stAlAcquit:
          printDebug ("-");
          if (data->cptAcquit < 0)
            {
              if (data->numRepet < vct_st_al.rep)
                {
                  stAlEmettreMessage (data);
                }
              else
                {
                  /* Suivant le contenu de la reponse attendue, il
                   * faut positionner le bit du str.                                      */
                  if (((strcmp
                        ((char *) vct_st_al.mess[data->numMess].txt_rep,
                         "*") == 0)
                       && (vct_st_al.mess[data->numMess].prot == 'N'))
                      || (vct_st_al.mess[data->numMess].nb_rep == 0))
                    {
                    }
                  else
                    {
                      data->abouti = 0;
                    }                  /* endif                                                        */
                  stAlMessageSuivant (data);
                }
            }
          else
            {
              /* Ce n'est pas fini, on attend.        */
            }                          /* endif(stAlData->                                     */
          break;
        case E_stAlLibre:
          printDebug (":");
          break;
        default:
          /* Il y a defaut...                                                                                             */
          break;
        }                              /* endswitch(prot==stAlData                     */
    }
}

/* --------------------------------
 * stAlDecompter
 * =============
 * --------------------------------	*/

void stAlDecompter (void)
{
  T_stAlData *data = stAlData;
  printDebug (".");
  if (data->neutre == E_stAlNeutralise)
    {
      if (--data->cptNeutre < 0)
        {
          data->neutre = E_stAlDisponible;
        }
    }                                  /* endif(data->neutre                           */
  switch (data->etat)
    {
    case E_stAlAttente:
      data->cptAttente--;
      break;
    case E_stAlAcquit:
      data->cptAcquit--;
      break;
    case E_stAlLancer:
    case E_stAlLibre:
      break;
    default:
      break;
    }                                  /* endswitch(data->etat                         */
  if (data->test)
    {
      if (--data->cptTest == 0)
        {
          data->test = 0;
          stAlForcer ();
        }
    }
  /* --------------------------------     */
  /* FIN DE stAlDecompter                         */
  /* --------------------------------     */
}

/* --------------------------------
 * stAlAcquiterFip
 * ===============
 * On regarde si il s'agit d'un 
 * acquittement d'alerte.
 * --------------------------------	*/

int stAlAcquitFip (int port, char *message, int lg)
{
  int retour = 0;
#ifndef SANS_WORLDFIP
  T_stAlData *data = stAlData;
  T_usr_fip_mess *mess = usr_fip_mess_lire_courant ();
  if (((port == data->port) && (NULL != mess))
      && ((data->numMess >= 0) && (data->numMess < MAX_MESS_AL)))
    {
      T_vct_st_al_mess *bloc = &vct_st_al.mess[data->numMess];
      if (data->etat == E_stAlAcquit)
        {
          /* Controler l'adresse du message */
          /* Par defaut, on ne reconnait pas
           * l'identifiant ni l'adresse du 
           * message.             */
          if (bloc->adFip == usr_fip_mess_get_dest (mess))
            {
              if (bloc->idFip == usr_fip_mess_get_ident (mess))
                {
                  /* Controler le contenu du message */
                  /* Recuperation du message dans le buffer       */
                  if ((bloc->nb_rep)
                      && (('*' != bloc->txt_rep[0]) || (1 != bloc->nb_rep)))
                    {
                      /* Dans le cas ou le message est
                       * correct, on passe au message         
                       * suivant.     */
                      if (strncmp (message, (char *) bloc->txt_rep, lg) == 0)
                        {
                          printDebug ("Reponse OK");
                          stAlMessageSuivant (data);
                          retour = 1;
                        }
                      else
                        {
                          printDebug ("Reponse incorrecte %d %s pour %d %s\n",
                                      lg, message, bloc->nb_rep,
                                      bloc->txt_rep);
                        }
                    }
                  else
                    {
                      /* Si aucun acquittement n'est attendu
                       * ou si on veut un acquit court, il 
                       * faut passer au message suivant.      */
                      printDebug ("Reponse OK");
                      stAlMessageSuivant (data);
                      retour = 1;
                    }
                }
              else
                {
                  printDebug
                    ("Identification incorrecte attentu %#x pour %#x\n",
                     bloc->idFip, usr_fip_mess_get_ident (mess));
                }
            }
          else
            {
              printDebug ("Adresse incorrecte attentu %#x pour %#x\n",
                          bloc->adFip, usr_fip_mess_get_dest (mess));
            }
        }                              /* endif(data->etat==E_stAlAcquit)      */
    }
#endif
  return (retour);

}

/* --------------------------------
 * stAlAcquitter
 * =============
 * Si la fonction retourne 1, il 
 * s'agit d'un acquittement sinon c'est
 * un message normal.
 * --------------------------------	*/

int stAlAcquit (int port)
{
  T_stAlData *data = stAlData;
  int retour = 0;
  if (port == data->port)
    {
      /* Controler l'adresse du message */
      /* Par defaut, on ne reconnait pas
       * l'identifiant ni l'adresse du 
       * message.             */
      /* Controler le contenu du message */
      /* Recuperation du message dans le buffer       */
      /* Dans le cas ou le message est
       * correct, on passe au message         
       * suivant.     */
      /*
         if(FALSE)
         {
         retour=1;
         if(FALSE)
         {
         stAlMessageSuivant(data);
         }
         }
         else
         {
         }                                            
       */
    }
  return (retour);
}

/* ********************************
 * FIN DE stAlLib.c
 * ********************************	*/
