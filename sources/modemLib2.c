
/* ********************************
 * DECLARATION DES INCLUDES
 * ********************************	*/

#include "vxWorks.h"
#include "stdio.h"
#include "time.h"
#include "timers.h"
#include "string.h"
#include "selectLib.h"
#include "io/iolib.h"
#include "sioLib.h"
#include "fcntl.h"
#include "modemLib.h"

/* ********************************
 * DECLARATION VARIABLES LOCALES
 * ********************************	*/

static T_modemTable modemMotsClefs[] = {
  {E_modemOk, "OK"},
  {E_modemDtr, "ATZ\r\n"},
  {E_modemEcho, "ATE0\r\n"},
  {E_modemFlux, "AT\r\n"},
  {E_modemVitesse, "AT&M0&N6\r\n"},
  {E_modemReponse, "ATS0=1\r\n"},
  {E_modemError, "ERROR"},
  {E_modemAppel, "RING"},
  {E_modemConnexion, "CONNECT 9600"},
  {E_modemConnexion1, "CONNECT 19200"},
  {E_modemConnexion2, "CONNECT 33600"},
  {E_modemDeconnexion, "NO CARRIER"},
  {E_modemDeconnecter, "+++"},
  {E_modemRaccrocher, "ATH\r\n"},
  {E_modemOk, NULL}
};

/* ********************************
 * DECLARATION DES VARIABLES LOCALES
 * ********************************	*/

static char *modemNomEntree = "/tyCo/3";
static int modemNotFin = 1;
static int modemEntree = -1;
static int modemNbCarCour = 0;
static char modemData[MODEM_MAX_DATA + 1] = "";
static E_modemCas modemCasCourant = E_modemDtr;

/* ********************************
 * DECLARATION DES FONCTIONS
 * ********************************	*/

void modemSetCas (E_modemCas cas)
{
  modemCasCourant = cas;
}

E_modemCas modemGetCas (void)
{
  return modemCasCourant;
}

int modemLectureArreter (void)
{
  modemNotFin = 0;
}

void modemSetFd (int fd)
{
  printf ("Le descripteur est %d\n");
  modemEntree = fd;
}

void modemAjouterData (char *data, int nbCar)
{
  int indice;
  data[nbCar] = 0;
  printf ("Dans modemAjouterData car %d chaine %s\n", nbCar, data);
  for (indice = 0; indice < nbCar; indice++)
    {
      modemData[modemNbCarCour + indice] = data[indice];
    }
  modemNbCarCour += indice;
  modemData[modemNbCarCour] = 0;
}

int modemLire (void)
{
  modemEntree = open (modemNomEntree, 2, 0);
  if (-1 != modemEntree)
    {
      int width;
      struct fd_set readFds;
      struct timeval attente = { 10, 0 };

      printf ("Le descripteur est %d\n", modemEntree);
      ioctl (modemEntree, FIOBAUDRATE, 9600);
      /* ioctl (modemEntree,FIOSETOPTIONS, OPT_TERMINAL); */
      ioctl (modemEntree, SIO_HW_OPTS_SET, CREAD | CS8);
      ksleep(500);
      modemEnvoyerMots (modemCasCourant);
      ksleep(500);
      while (modemNotFin)
        {
          FD_ZERO (&readFds);
          width = 0;
          if (-1 != modemEntree)
            {
              FD_SET (modemEntree, &readFds);
              if (modemEntree > width)
                {
                  width = modemEntree;
                }
            }
          width++;
          switch (select (width, &readFds, NULL, NULL, &attente))
            {
            case ERROR:
            	if(EINTR!=errno)
            	{
              printf ("Erreur de select : %#x\n", errno);
              notFin=0;
            	}
              break;
            case 0:
              printf ("Timeout sur reception. liaison \n");
              break;
            default:
              if (FD_ISSET (modemEntree, &readFds))
                {
                  int nbCar;
                  printf ("Avant lecture\n");
                  ksleep(500);
                  nbCar = read (modemEntree, &modemData[modemNbCarCour],
                                MODEM_MAX_DATA - modemNbCarCour);
                  if (nbCar == ERROR)
                    {
                      return 0;
                    }
                  else
                    {
                      int indice;
                      printf ("DONNEES nbCarLus %d *****", nbCar);
                      for (indice = 0; indice < nbCar; indice++)
                        {
                          if (0xd == modemData[modemNbCarCour + indice])
                            {
                              printf ("<0x0d>");
                            }
                          else if (0xa == modemData[modemNbCarCour + indice])
                            {
                              printf ("<0x0a>");
                            }
                          else
                            {
                              printf ("%c %i",
                                      modemData[modemNbCarCour + indice],
                                      modemData[modemNbCarCour + indice]);
                            }
                        }
                      printf (" *****\n", modemData[modemNbCarCour + indice]);
                      modemNbCarCour += nbCar;
                      modemData[modemNbCarCour] = 0;
                      modemTraiter (modemAnalyser ());
                      /* printf("EN HEXA *****",modemData[indice]);
                         for(indice=0;indice<nbCar;indice++)
                         {
                         printf("%#02x ",modemData[indice]);
                         }
                         printf("*****\n",modemData[indice]);
                       */
                    }
                }
              break;
            }
        }
    }
  else
    {
      printf ("Erreur d'ouverture de %s\n", modemNomEntree);
    }

}

int modemEcrire (char *chaine)
{
  if (-1 != modemEntree)
    {
      int nbCar;
      nbCar = write (modemEntree, chaine, strlen (chaine));
      if (-1 == nbCar)
        {
          printf ("Erreur d'ecriture dans %d\n", modemEntree);
        }
    }
}

int modemConnecter (void)
{
  modemEcrire ("AT+CBST=7,0,1\n");
  ksleep(1000);
  modemEcrire ("AT+IFC=0,0\n");
  ksleep(1000);
  modemEcrire ("ATS0=1\n");
  ksleep(1000);
  modemEcrire ("AT+CRC=1\n");
  ksleep(1000);
  modemEcrire ("AT+CREG=1\n");
  ksleep(1000);
  modemEcrire ("AT+CPIN=1707\n");
  ksleep(1000);
}


E_modemCas modemAnalyser (void)
{
  E_modemCas casSuivant = E_modemPasDeClef;
  int motClef = 0;
  int notFin = 1;
  printf ("Dans modemAnalyser %s\n", modemData);
  /* Rechercher du OxOd OxOa */
  while (notFin
         && (NULL != modemMotsClefs[motClef].clef) & (modemNbCarCour > 0))
    {
      char *ptDebut;
      ptDebut = strstr (modemData, modemMotsClefs[motClef].clef);
      if (NULL == ptDebut)
        {
          /* La chaine n'est pas trouvee, on passe au suivant     */
          motClef++;
        }
      else
        {
          int indice;
          int nbCarDeb = (int) (ptDebut - modemData)
            + strlen (modemMotsClefs[motClef].clef);
          printf (" code trouve %s\n", modemMotsClefs[motClef].clef);
          /* Recopier les caracteres qui reste en debut de buffer */
          for (indice = 0; indice < (modemNbCarCour - nbCarDeb + 1); indice++)
            {
              modemData[indice] = modemData[indice + nbCarDeb];
            }
          modemNbCarCour -= nbCarDeb;
          switch (modemMotsClefs[motClef].cas)
            {
            case E_modemOk:
              switch (modemCasCourant)
                {
                case E_modemDtr:
                case E_modemDtr2:
                  casSuivant = E_modemEcho;
                  break;
                case E_modemEcho:
                case E_modemEcho2:
                  casSuivant = E_modemReponse;
                  break;
                case E_modemFlux:
                  casSuivant = E_modemAppel;
                  break;
                case E_modemVitesse:
                  casSuivant = E_modemFlux;
                  break;
                case E_modemReponse:
                  casSuivant = E_modemVitesse;
                  break;
                case E_modemDeconnecter:
                  casSuivant = E_modemRaccrocher;
                  break;
                case E_modemRaccrocher:
                  casSuivant = E_modemDtr;
                  break;
                default:
                  casSuivant = E_modemRien;
                  break;
                }
              break;
            case E_modemDtr:
              casSuivant = E_modemDtr2;
              break;
            case E_modemEcho:
              casSuivant = E_modemEcho2;
              break;

            case E_modemError:
              switch (modemCasCourant)
                {
                default:
                  casSuivant = E_modemDtr;
                  break;
                }
              break;
            case E_modemPinDefaut:
              casSuivant = E_modemRien;
              break;
            case E_modemAppel:
              casSuivant = E_modemAppel;
              break;
            case E_modemConnexion:
            case E_modemConnexion1:
            case E_modemConnexion2:
              casSuivant = E_modemConnexion;
              break;
            case E_modemDeconnexion:
              if (modemCasCourant == E_modemConnexion)
                {
                  casSuivant = E_modemDtr;
                }
              else
                {
                  casSuivant = E_modemDeconnexion1;
                }
              break;
            case E_modemDeconnexion1:
              casSuivant = E_modemDeconnexion2;
              break;
            case E_modemDeconnexion2:
              casSuivant = E_modemFlux;
              break;
            default:
              casSuivant = E_modemRien;
              break;
            }
          motClef = 0;
          notFin = 0;
        }
    }
  /* Action si trouve.    */
  return casSuivant;
}

int modemRechercher (E_modemCas cas, char *chaine, int nbCar)
{
  int motClef = 0;
  int retour = 0;
  printf ("Dans modemRechercher %d\n", cas);
  if (0 != nbCar)
    {
      chaine[nbCar] = 0;
      while (NULL != modemMotsClefs[motClef].clef)
        {
          if (cas == modemMotsClefs[motClef].cas)
            {
              char *ptDebut;
              ptDebut = strstr (chaine, modemMotsClefs[motClef].clef);
              if (NULL != ptDebut)
                {
                  /* La chaine est trouvee        */
                  retour = 1;
                  break;
                }
            }
          motClef++;
        }
    }
  return retour;
}

void modemTraiter (E_modemCas cas)
{
  if (E_modemPasDeClef == cas)
    {
      printf ("Dans modemTraiter : pas de clef\n");
    }
  else
    {
      printf ("Dans modemTraiter %d\n", cas);
      modemCasCourant = cas;
      switch (cas)
        {
        case E_modemFlux:
        case E_modemEcho:
        case E_modemDtr:
        case E_modemReponse:
        case E_modemVitesse:
        case E_modemEtat:
        case E_modemActivation:
        case E_modemRaccrocher:
          modemEnvoyerMots (cas);
          break;
        }
    }
}

void modemEnvoyerMots (E_modemCas cas)
{
  int motClef = 0;
  printf ("Dans modemEnvoyerMots %d\n", cas);
  while (NULL != modemMotsClefs[motClef].clef)
    {
      if (cas == modemMotsClefs[motClef].cas)
        {
          printf ("Dans modemEnvoyerMots, Clef trouvee %s\n",
                  modemMotsClefs[motClef].clef);
          modemEcrire (modemMotsClefs[motClef].clef);
          break;
        }
      else
        {
          motClef++;
        }
    }
}
