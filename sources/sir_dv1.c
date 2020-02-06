/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 24/01/1991                                               /
/-----------------------------------------------------------------------------/
/ FONCTION DU SOUS-MODULE : Regroupe toutes les fonctions utiles a toutes les /
/ taches de la station SIREDO.                                                /
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

/* *************************************
 * LISTE DES INCLUDES
 * ************************************* */
#include <memory.h>
#include "standard.h"
#include "mon_inc.h"
#include "define.h"
#include "xdg_def.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "sir_dv1h.h"
#include "pip_str.h"
#include "pip_var.h"

 /**********************************************************/
 /*                                                        */
 /*            prototypage des fonctions                   */
 /*                                                        */
 /**********************************************************/

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_format_date                                        /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 15/01/97                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : dv1_fcnt.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : test de la date                                        /
/-------------------------------------------------------------------------DOC*/

INT dv1_format_date (STRING buff, STRING date, INT format)
{
  INT nbcar;
  BYTE car;

  switch (format)
    {
    case TYPE_HR:
      /* on sauvegarde le caractere de fin de chaine */
      car = buff[5];
      nbcar = sprintf (buff, "%02d:%02d", date[H_HOU], date[H_MIN]);
      buff[5] = car;
      break;

    case TYPE_DT0:
      car = buff[8];
      nbcar =
        sprintf (buff, "%02d/%02d/%02d", date[H_DAT], date[H_MON],
                     date[H_YEA]);
      buff[8] = car;
      break;

    case TYPE_DT1:
      car = buff[12];
      if (date[H_YEA] > 70)
        nbcar =
          sprintf (buff, "%02d      19%02d", date[H_DAT], date[H_YEA]);
      else
        nbcar =
          sprintf (buff, "%02d      20%02d", date[H_DAT], date[H_YEA]);

      switch (date[H_MON])
        {
        case 1:                       /* janvier  */
          sprintf (&buff[3], "jan.");
          break;
        case 2:                       /* fevrier */
          sprintf (&buff[3], "fev.");
          break;
        case 3:                       /* mars     */
          sprintf (&buff[3], "mars");
          break;
        case 4:                       /* avril     */
          sprintf (&buff[3], "avr.");
          break;
        case 5:                       /* mai      */
          sprintf (&buff[3], "mai ");
          break;
        case 6:                       /* juin      */
          sprintf (&buff[3], "juin");
          break;
        case 7:                       /* juillet  */
          sprintf (&buff[3], "juil");
          break;
        case 8:                       /* aout     */
          sprintf (&buff[3], "aout");
          break;
        case 9:                       /* septembre */
          sprintf (&buff[3], "sep.");
          break;
        case 10:                      /* octobre  */
          sprintf (&buff[3], "oct.");
          break;
        case 11:                      /* novembre  */
          sprintf (&buff[3], "nov.");
          break;
        case 12:                      /* decembre */
          sprintf (&buff[3], "dec.");
          break;
        }
      break;
    }
  return (nbcar);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_sprintf                                            /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 24/03/1996                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : dv1_fcnt.c                                           /
/-------------------------------------------------------------------------DOC*/
INT dv1_sprintf (STRING dest, STRING format, INT param)
{
  STRING pt_deb;
  INT *pt_int;
  UINT *pt_uint;
  LONG *pt_long;
  ULONG *pt_ulong;
  STRING *pt_string;
  char *pt_car;

  char *pt_format;
  INT *pt_param;

  UINT8 phase;



  struct form
  {
    UINT8 car;
    UINT8 nb_car;
    UINT8 unsig;
    UINT8 type;
  } form[20];

  UINT8 nb_par;

  pt_deb = dest;

  memset ((STRING) form, 0, 20 * sizeof (struct form));

  pt_format = format;

  phase = 0;
  nb_par = 0;

  while (*pt_format)
    {
      switch (*pt_format)
        {
        case '%':
          phase = 1;
          break;

        case '0':
        case ' ':
          if (phase == 1)
            {
              phase = 2;
              form[nb_par].car = *pt_format;
            }
          break;

        case 'c':
        case 'd':
        case 'l':
        case 'L':
        case 's':
        case 'x':
        case 'X':
        case 'I':
          if (phase > 0)
            {
              phase = 0;
              form[nb_par].type = *pt_format;
              nb_par++;
            }
          break;

        case 'u':
          if (phase)
            {
              phase = 3;
              form[nb_par].unsig = TRUE;
            }
          break;

        default:
          if (phase > 0)
            {
              if ((*pt_format > '0') && (*pt_format <= '9'))
                {
                  form[nb_par].nb_car = *pt_format - '0';
                  phase = 3;
                }
            }
          break;
        }
      pt_format++;
    }

  pt_param = &param;
  pt_format = format;

  nb_par = 0;
  phase = 0;

  while (*pt_format)
    {
      switch (*pt_format)
        {
        case '%':
          phase = 1;
          /* on copie le format */
          switch (form[nb_par].type)
            {
            case 'c':
              pt_car = (STRING) (pt_param);

              *dest++ = *pt_car;

              pt_car++;
              pt_param = (INT *) pt_car;
              break;
            case 'd':
              if (form[nb_par].unsig)
                {
                  pt_uint = (UINT *) pt_param;
                  if (form[nb_par].nb_car)
                    {
                      memset (dest, form[nb_par].car, form[nb_par].nb_car);
                    }
                  dest +=
                    dv1_ltoa (dest, (LONG) * pt_uint, form[nb_par].nb_car);
                  pt_uint++;
                  pt_param = (INT *) pt_uint;
                }
              else
                {
                  pt_int = (INT *) pt_param;
                  if (form[nb_par].nb_car)
                    {
                      memset (dest, form[nb_par].car, form[nb_par].nb_car);
                    }
                  dest += dv1_itoa (dest, *pt_int, form[nb_par].nb_car);
                  pt_int++;
                  pt_param = (INT *) pt_int;
                }
              break;

            case 'x':
            case 'X':
              pt_uint = (UINT *) pt_param;
              if (form[nb_par].nb_car)
                {
                  memset (dest, form[nb_par].car, form[nb_par].nb_car);
                }
              dest += dv1_utoh (dest, (UINT) * pt_uint, form[nb_par].nb_car);
              pt_uint++;
              pt_param = (INT *) pt_uint;
              break;

            case 'l':
            case 'L':
              if (form[nb_par].unsig)
                {
                  pt_ulong = (ULONG *) pt_param;
                  if (form[nb_par].nb_car)
                    {
                      memset (dest, form[nb_par].car, form[nb_par].nb_car);
                    }
                  dest += dv1_ltoa (dest, *pt_ulong, form[nb_par].nb_car);
                  pt_ulong++;
                  pt_param = (INT *) pt_ulong;
                }
              else
                {
                  pt_long = (LONG *) pt_param;
                  if (form[nb_par].nb_car)
                    {
                      memset (dest, form[nb_par].car, form[nb_par].nb_car);
                    }
                  dest += dv1_ltoa (dest, *pt_long, form[nb_par].nb_car);
                  pt_long++;
                  pt_param = (INT *) pt_long;
                }
              break;
            case 's':
              pt_string = (STRING *) (pt_param);
              while (**pt_string)
                {
                  *dest++ = *(*pt_string)++;
                }
              pt_string++;
              pt_param = (INT *) pt_string;
              break;
            }
          /* on passe au format suivant */
          nb_par++;
          break;

        case 'c':
        case 'd':
        case 'l':
        case 'L':
        case 's':
        case 'x':
        case 'X':
        case 'I':
          if (phase > 0)
            phase = 0;
          else
            *dest++ = *pt_format;
          break;

        default:
          if (phase == 0)
            {
              *dest++ = *pt_format;
            }
          break;
        }
      pt_format++;

    }
  *dest = *pt_format;

  return ((INT) (dest - pt_deb));
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_atoi                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 11/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------*/

INT dv1_atoi (STRING buffer, INT lgbuff, INT * valeur)
{

  INT i;                        /* index de lecture dans le buffer  */
  INT fin;                      /* variable de controle de boucle   */
  INT erreur;                   /* erreur retournee                 */
 /* on initialise i a zero */
  i = 0;

 /* on initialise erreur a 0 */
  erreur = FALSE;

 /* on initialise valeur a 0 */
  *valeur = 0;

 /* on initialise fin a faux */
  fin = FALSE;

  while (!fin)
    {
      /* on teste si le caractere suivant est bon */
      if ((buffer[i] >= '0') && (buffer[i] <= '9'))
        {
          *valeur = (*valeur * 10) + buffer[i] - '0';
        }
      else
        {
          /* si le caractere n'est pas bon */
          erreur = TRUE;
        }

      if (++i >= lgbuff)
        {
          if (lgbuff)
            {
              /* on force la fin de la boucle while */
              fin = TRUE;
            }
          else
            {
              if (buffer[i] == 0)
                {
                  /* on force la fin de la boucle while */
                  fin = TRUE;
                }
            }
        }
    }
 /* on retourne le code de l'erreur */
  return (!erreur);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_atoi                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 11/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------*/

INT dv1_htoi (STRING buffer, INT lgbuff, INT * valeur)
{

  INT i;                        /* index de lecture dans le buffer                   */
  INT fin;                      /* variable de controle de boucle                    */
  INT erreur;                   /* erreur retournee                                  */

 /* on initialise i a zero */
  i = 0;

 /* on initialise erreur a 0 */
  erreur = FALSE;

 /* on initialise valeur a 0 */
  *valeur = 0;

 /* on initialise fin a faux */
  fin = FALSE;

  while (!fin)
    {
      /* on teste si le caractere suivant est bon */
      if ((buffer[i] >= '0') && (buffer[i] <= '9'))
        {
          *valeur = (*valeur * 16) + buffer[i] - '0';
        }
      else if ((buffer[i] >= 'A') && (buffer[i] <= 'F'))
        {
          *valeur = (*valeur * 16) + buffer[i] - 0x37;
        }
      else
        {
          /* si le caractere n'est pas bon */
          erreur = TRUE;
        }

      if (++i >= lgbuff)
        {
          if (lgbuff)
            {
              /* on force la fin de la boucle while */
              fin = TRUE;
            }
          else
            {
              if (buffer[i] == 0)
                {
                  /* on force la fin de la boucle while */
                  fin = TRUE;
                }
            }
        }
    }
 /* on retourne le code de l'erreur */
  return (!erreur);
}


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_scpy                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 24/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : copie d'une chaine dans une autre                      /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : ch_s : chaine source de la copie                        /
/                     ch_d : chaine destination de la copie                   /
/                     nbcar: nombre de caracteres a copier                    /
/-------------------------------------------------------------------------DOC*/
INT dv1_scpy (STRING ch_d, STRING ch_s, INT nbcar)
{
  INT nb;
  if (nbcar == 0)
    {
      nb = 0;
      while (*ch_s)
        {
          *ch_d++ = *ch_s++;
          nb++;
        }
      *ch_d++ = 0;
    }
  else
    {
      nb = nbcar;
      while (nbcar--)
        {
          *ch_d++ = *ch_s++;
        }
    }
  return (nb);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_set                                                /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 24/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : initialise une chaine                                  /
/-------------------------------------------------------------------------DOC*/
INT dv1_set (STRING ch_d, INT car, INT nbcar)
{
  INT nb;
  nb = nbcar;
  while (nbcar--)
    {
      *ch_d++ = (UINT8) car;
    }
  return (nb);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_scmp                                               / /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 24/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : compare deux chaines                                   /
/-------------------------------------------------------------------------DOC*/
INT dv1_scmp (STRING ch_1, STRING ch_2, INT nbcar)
{

  INT retour;
  INT fini;
  INT i;

 /* on suppose les deux chaines egales */
  retour = TRUE;

 /* fin de boucle a faux */
  fini = FALSE;

  i = 0;

  while (!fini)
    {
      if (ch_1[i] != ch_2[i])
        {
          retour = FALSE;              /* on signale une inegalite */

          fini = TRUE;                 /* on provoque la sortie de la boucle */
        }
      else
        {
          /* on passe au caractere suivant */
          i++;

          /* fin de chaine ? */
          if (i >= nbcar)
            {
              if (nbcar)
                fini = TRUE;
              else
                {
                  if ((ch_1[i] == 0) || (ch_2[i] == 0))
                    {
                      fini = TRUE;
                    }
                }
            }
        }
    }
  return (retour);
}

INT dv1_scmp_and_skip (STRING ch_1,INT lg1, STRING ch_2, INT nbcar,STRING *fin,INT *lg)
{
INT retour;
INT fini;
INT i;
	if(0==nbcar)
	{
		nbcar=strlen(ch_2);
	}
  retour = ((lg1>=nbcar)?TRUE:FALSE);
  fini   = ((lg1>=nbcar)?FALSE:TRUE);
  i = 0;
  while ((!fini)&&(i<lg1)&&(i<nbcar))
  {
      if (ch_1[i] != ch_2[i])
        {
          retour = FALSE;              /* on signale une inegalite */
          fini = TRUE;                 /* on provoque la sortie de la boucle */
        }
      else
        {
          /* on passe au caractere suivant */
          i++;

          /* fin de chaine ? */
          if (i >= nbcar)
            {
              if (nbcar)
                fini = TRUE;
              else
                {
                  if ((ch_1[i] == 0) || (ch_2[i] == 0))
                    {
                      fini = TRUE;
                    }
                }
            }
        }
    }
  if(TRUE==retour)
  {
	*fin=ch_1+nbcar;
	*lg=lg1-nbcar;
  }
  return (retour);

}



/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_egal                                               / /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 11/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : cherche un signe egal dans la chaine specifie          /
/-------------------------------------------------------------------------DOC*/
INT dv1_car (STRING chaine, INT lg_mess, INT car)
{                                      /* debut de dv1_egal */

  INT i;                        /* indice de lecture des chaines a comparer            */

  i = 0;

  while ((chaine[i] != car) && (i < lg_mess))
    {                                  /* W1 */

      /* on passe au caractere suivant */
      i++;

    }                                  /* FW1 */

  return (i);

}                                      /* fin de dv1_egal   */

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_num                                                /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 10/10/93                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : cherche un num                                         /
/-------------------------------------------------------------------------DOC*/
INT dv1_num (STRING chaine, INT lg_mess)
{

  INT i;                        /* indice de lecture des chaines a comparer            */

  i = 0;
  while ((chaine[i] >= '0') && (chaine[i] <= '9') && (i < lg_mess))
    {
      i++;
    }
  return (i);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_cherchefin                                         /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 11/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_ge1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : recherche de la fin d'une portion de chaine terminee   /
/ par un blanc ou par une virgule.                                            /
/-------------------------------------------------------------------------DOC*/

INT dv1_cherchefin (STRING buffer, INT lgbuff)
{
  INT i = 0;
  INT taille;
  INT fin = FALSE;
  while (!fin)
    {
      /* si on est arrive au bout du buffer */
      if (i == lgbuff)
        {
          taille = lgbuff;
          fin = TRUE;
        }                              /* FIF1 */
      else
        {                              /* EIF1 */
          if ((buffer[i] == ' ') || (buffer[i] == ','))
            {                          /* IF2 */
              taille = i;
              fin = TRUE;
            }                          /* FIF2 */
          i++;
        }                              /* FEIF1 */
    }                                  /* FW1 */
  return (taille);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_separateur                                         /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 11/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : elimine tous les caracteres de type separateur         /
/-------------------------------------------------------------------------DOC*/
INT dv1_separateur (STRING buffer, INT lg_mess)
{                                      /* debut de dv1_separateur */

  INT i;                        /* indice de lecture des chaines a comparer            */

  i = 0;

  while ((i < lg_mess) && ((buffer[i] == ',') || (buffer[i] == ' ')))
    {                                  /* W1 */

      /* on passe au caractere suivant */
      i++;

    }                                  /* FW1 */

  return (i);

}                                      /* fin de dv1_separateur */


/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_itoa                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 24/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : transforme un entier en une chaine                     /
/-------------------------------------------------------------------------DOC*/

INT dv1_itoa (STRING chaine, INT ent, INT taille)
{                                      /* debut de dv1_itoa */

  INT i;                        /* compteur de caracteres de la chaine a creer         */
  INT nbcar;                    /* nombre de caracteres de la chaine                   */
  UINT entier;

  if (ent < 0)
    entier = 0;
  else
    entier = (UINT) ent;

 /* on calcule le nombre de carateres de chaine */
  if (taille == 0)
    {

      if (entier > 9999)
        nbcar = 5;
      else if (entier > 999)
        nbcar = 4;
      else if (entier > 99)
        nbcar = 3;
      else if (entier > 9)
        nbcar = 2;
      else
        nbcar = 1;

    }
  else
    {
      nbcar = taille;
      switch (taille)
        {
        case 1:
          if (entier > 9)
            entier = 9;
          break;
        case 2:
          if (entier > 99)
            entier = 99;
          break;
        case 3:
          if (entier > 999)
            entier = 999;
          break;
        case 4:
          if (entier > 9999)
            entier = 9999;
          break;
        }
    }

  i = nbcar;                           /* on initialise i a 1   */

  do
    {
      chaine[--i] = (entier % 10) + '0';
      entier = entier / 10;
    }
  while (i);


  return (nbcar);

}

INT dv1_ltoa (STRING chaine, LONG entier, INT taille)
{
  INT i;                        /* compteur de caracteres de la chaine a creer         */
  INT nbcar;                    /* nombre de caracteres de la chaine                   */
  /* on calcule le nombre de carateres
   * de chaine */
  if (taille == 0)
    {                                  /* IF0 */
      if (entier > 9999999)
        nbcar = 8;
      else if (entier > 999999)
        nbcar = 7;
      else if (entier > 99999)
        nbcar = 6;
      else if (entier > 9999)
        nbcar = 5;
      else if (entier > 999)
        nbcar = 4;
      else if (entier > 99)
        nbcar = 3;
      else if (entier > 9)
        nbcar = 2;
      else
        nbcar = 1;
    }                                  /* FIF0 */
  else
    {
      nbcar = taille;
      switch (taille)
        {
        case 1:
          if (entier > 9)
            entier = 9;
          break;
        case 2:
          if (entier > 99)
            entier = 99;
          break;
        case 3:
          if (entier > 999)
            entier = 999;
          break;
        case 4:
          if (entier > 9999)
            entier = 9999;
          break;
        case 5:
          if (entier > 99999)
            entier = 99999;
          break;
        case 6:
          if (entier > 999999)
            entier = 999999;
          break;
        case 7:
          if (entier > 9999999)
            entier = 9999999;
          break;
        case 8:
          if (entier > 99999999)
            entier = 99999999;
          break;
        }
    }
  i = nbcar;                           /* on initialise i a 1   */
  do
    {
      chaine[--i] = (INT) (entier % 10L) + '0';
      entier = entier / 10;
    }
  while (i);

  return (nbcar);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_itoh                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 29/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : transforme un entier en une chaine hexa                /
/-------------------------------------------------------------------------DOC*/
VOID dv1_itoh (STRING chaine, INT entier, INT nbcar)
{                                      /* debut de dv1_itoh */

  INT i;                        /* compteur de caracteres de la chaine a creer         */
  INT reste;                    /* reste de la division entiere de entier par 16       */
 /* conversion de chiffre de 0 a 5 en caractere         */


  i = 1;                               /* on initialise i a 1   */

  while (i <= nbcar)
    {                                  /* W1 */

      /* division entiere de entier par 16 */
      reste = entier % 16;

      /* codage de reste sous forme d'un caractere */
      if (reste < 10)
        {                              /* IF1 */

          chaine[nbcar - i] = reste + '0';

        }                              /* FIF1 */
      else
        {                              /* EIF1 */

          chaine[nbcar - i] = reste - 10 + 'A';

        }                              /* FEIF1 */

      entier = entier / 16;

      i++;

    }                                  /* FW1 */

}                                      /* fin de dv1_itoh   */

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_str_atoh                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/12/96                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : transforme une chaine en antier                        /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : chaine : chaine e generer                               /
/                     entier : valeur e transformer en chaine                 /
/-------------------------------------------------------------------------DOC*/

INT dv1_str_atoh (STRING * ptr, UINT * ent)
{
  INT retour;
  BYTE fini;
  *ent = 0;
  fini = FALSE;

  retour = FALSE;

  while (!fini)
    {
      if ((**ptr >= '0') && (**ptr <= '9'))
        {
          *ent *= 16;
          *ent += (**ptr) - '0';
        }
      else if ((**ptr >= 'A') && (**ptr <= 'F'))
        {
          *ent *= 16;
          *ent += (**ptr) - 0x37;
        }
      else
        {
          switch (**ptr)
            {
            case 0:
            case '.':
            case '/':
            case '=':
            case ',':
              fini = TRUE;
              retour = TRUE;
              break;
            default:
              fini = TRUE;
              break;
            }
        }
      /* on passe au caractere suivant */
      (*ptr)++;
    }
  return (retour);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_utoh                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 29/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : transforme un entier en une chaine hexa                /
/-------------------------------------------------------------------------DOC*/
INT dv1_utoh (STRING chaine, UINT entier, INT nbcar)
{

  INT i;                        /* compteur de caracteres de la chaine a creer         */
  UINT reste;                   /* reste de la division entiere de entier par 16       */

  i = 1;
  if (nbcar == 0)
    {
      if (entier < 10)
        nbcar = 1;
      else if (entier < 100)
        nbcar = 2;
      else if (entier < 1000)
        nbcar = 3;
      else if (entier < 10000)
        nbcar = 4;
      else
        nbcar = 5;
    }

  while (i <= nbcar)
    {
      /* division entiere de entier par 16 */
      reste = entier & 0x000F;

      /* codage de reste sous forme d'un caractere */
      if (reste < 10)
        {
          chaine[nbcar - i] = reste + '0';
        }
      else
        {
          chaine[nbcar - i] = reste + '7';
        }

      entier >>= 4;

      i++;
    }
  return (nbcar);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_ltoh                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 29/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-------------------------------------------------------------------------DOC*/
INT dv1_ltoh (STRING chaine, LONG lon_g, INT nbcar)
{

  INT i;                        /* compteur de caracteres de la chaine a creer         */
  LONG reste;                   /* reste de la division entiere de entier par 16       */

  i = 1;

  if (nbcar == 0)
    {
      if (lon_g < 0x10)
        nbcar = 1;
      else if (lon_g < 0x100)
        nbcar = 2;
      else if (lon_g < 0x1000)
        nbcar = 3;
      else if (lon_g < 0x10000)
        nbcar = 4;
      else if (lon_g < 0x100000)
        nbcar = 5;
      else if (lon_g < 0x1000000)
        nbcar = 6;
      else if (lon_g < 0x10000000)
        nbcar = 7;
      else
        nbcar = 8;
    }

  while (i <= nbcar)
    {
      /* division entiere de entier par 16 */
      reste = lon_g & 0x0000000F;

      /* codage de reste sous forme d'un caractere */
      if (reste < 10)
        {
          chaine[nbcar - i] = reste + '0';
        }
      else
        {
          chaine[nbcar - i] = reste + '7';
        }

      lon_g >>= 4;

      i++;
    }
  return (nbcar);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_atol                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 30/01/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-------------------------------------------------------------------------DOC*/

INT dv1_atol (STRING buffer, INT lgbuff, LONG * valeur)
{

  INT i;                        /* index de lecture dans le buffer                   */
  INT fin;                      /* variable de controle de boucle                    */
  INT erreur;                   /* erreur retournee                                  */

 /* on initialise i a zero */
  i = 0;

 /* on initialise erreur a 0 */
  erreur = FALSE;

 /* on initialise valeur a 0 */
  *valeur = 0L;

 /* on initialise fin a faux */
  fin = FALSE;

  while (!fin)
    {
      /* on teste si le caractere suivant est bon */
      if ((buffer[i] >= '0') && (buffer[i] <= '9'))
        {
          *valeur = (*valeur * 10L) + buffer[i] - '0';
        }
      else
        {
          /* si le caractere n'est pas bon */
          erreur = TRUE;
        }

      /* si on est arrive au bout du buffer */
      if (++i >= lgbuff)
        {
          if (lgbuff)
            {
              /* on force la fin de la boucle while */
              fin = TRUE;
            }
          else
            {
              if (buffer[i] == 0)
                {
                  /* on force la fin de la boucle while */
                  fin = TRUE;
                }
            }
        }
    }
 /* on retourne le code de l'erreur */
  return (!erreur);

}

INT dv1_htol (STRING buffer, INT lgbuff, LONG * valeur)
{

  INT i;                        /* index de lecture dans le buffer                   */
  INT fin;                      /* variable de controle de boucle                    */
  INT erreur;                   /* erreur retournee                                  */

 /* on initialise i a zero */
  i = 0;

 /* on initialise erreur a 0 */
  erreur = FALSE;

 /* on initialise valeur a 0 */
  *valeur = 0L;

 /* on initialise fin a faux */
  fin = FALSE;

  while (!fin)
    {
      /* on teste si le caractere suivant est bon */
      if ((buffer[i] >= '0') && (buffer[i] <= '9'))
        {
          *valeur = (*valeur * 16L) + buffer[i] - '0';
        }
      else if ((buffer[i] >= 'A') && (buffer[i] <= 'F'))
        {
          *valeur = (*valeur * 16L) + buffer[i] - 0x37;
        }
      else
        {
          /* si le caractere n'est pas bon */
          erreur = TRUE;
        }

      /* si on est arrive au bout du buffer */
      if (++i >= lgbuff)
        {
          if (lgbuff)
            {
              /* on force la fin de la boucle while */
              fin = TRUE;
            }
          else
            {
              if (buffer[i] == 0)
                {
                  /* on force la fin de la boucle while */
                  fin = TRUE;
                }
            }
        }
    }
 /* on retourne le code de l'erreur */
  return (!erreur);

}

 /* FICHIER: %M%      REL: %I% DATE: %G% */
/*DOC***************************************************************************
*  SIAT.  *   FICHE DE  FONCTION:  dv1_chk
********************************************************************************
* OBJET DE LA FONCTION : calcul un cheksum entre deux adresses memoire
* PARAMETRES           : adresse de debut, adresse de fin
* VALEURS RENDUES      : le checksum
* FONCTIONS APPELEES   :
* FONCTIONS APPELANTES :
***************************************************************************DOC*/
ULONG dv1_chk (STRING debut, STRING fin)
{
  char *ptr;
  ULONG checksum;

  checksum = 0L;
  ptr = debut;
  while (ptr < fin)
    {
      if (((unsigned char)*ptr) != 0xFF)
        checksum += (ULONG) * ptr;
      ptr += 50;
    }
  return (checksum);
}

 /* FICHIER: %M%      REL: %I% DATE: %G% */
/*DOC***************************************************************************
*  SIAT.  *   FICHE DE  FONCTION:  dv1_chk
********************************************************************************
* OBJET DE LA FONCTION : calcul un cheksum entre deux adresses memoire
* PARAMETRES           : adresse de debut, adresse de fin
* VALEURS RENDUES      : le checksum
* FONCTIONS APPELEES   :
* FONCTIONS APPELANTES :
***************************************************************************DOC*/
ULONG dv1_chk_prom (STRING debut, STRING fin)
{
  char *ptr;
  ULONG checksum;

  checksum = 0L;
  ptr = debut;
  while (ptr < fin)
    {
      if (((unsigned char)*ptr) != 0xFF)
        checksum += (ULONG) * ptr;
      ptr++;
    }
  return (checksum);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_test_date                                              /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 28/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_idf.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : test de la date                                        /
/-------------------------------------------------------------------------DOC*/

INT dv1_test_date (STRING pt_date)
{
  INT flag_err;

  flag_err = FALSE;

  switch (pt_date[H_MON])
    {
    case 1:                           /* janvier  */
    case 3:                           /* mars     */
    case 5:                           /* mai      */
    case 7:                           /* juillet  */
    case 8:                           /* aout     */
    case 10:                          /* octobre  */
    case 12:                          /* decembre */
      if ((pt_date[H_DAT] < 1) || (pt_date[H_DAT] > 31))
        {
          flag_err = TRUE;
        }
      break;

    case 4:                           /* avril     */
    case 6:                           /* juin      */
    case 9:                           /* septembre */
    case 11:                          /* novembre  */
      if ((pt_date[H_DAT] < 1) || (pt_date[H_DAT] > 30))
        {
          flag_err = TRUE;
        }
      break;

    case 2:                           /* fevrier */
      if ((pt_date[H_YEA] % 4) == 0)   /* annee bissextile     */
        {
          if ((pt_date[H_DAT] < 1) || (pt_date[H_DAT] > 29))
            {
              flag_err = TRUE;
            }
        }
      else                             /* annee non bissextile */
        {
          if ((pt_date[H_DAT] < 1) || (pt_date[H_DAT] > 28))
            {
              flag_err = TRUE;
            }
        }
      break;

    default:
      flag_err = TRUE;
      break;
    }

  if ((pt_date[H_YEA] > 99) ||
      (pt_date[H_HOU] > 23) || (pt_date[H_MIN] > 59) || (pt_date[H_SEC] > 59))
    {
      flag_err = TRUE;
    }

  return (!flag_err);
}

 /* FICHIER: %M%      REL: %I% DATE: %G% */
/*DOC***************************************************************************
*   SIAT.  *   FICHE DE  FONCTION:   dv1_init_date
********************************************************************************
* OBJET DE LA FONCTION : reinitialisation de la date
* PARAMETRES           :
* VALEURS RENDUES      :
* FONCTIONS APPELEES   :
* FONCTIONS APPELANTES :
***************************************************************************DOC*/
VOID dv1_init_date (VOID)
{
  char date[SZ_DATE];

 /* date incorrecte, on initialise une date */

  date[H_DAT] = 2;
  date[H_MON] = 4;
  date[H_YEA] = 99;
  date[H_HOU] = 10;
  date[H_MIN] = 0;
  date[H_SEC] = 0;
  mnecal (date);

};

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_str_atoi                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/12/96                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : transforme une chaine en antier                        /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : chaine : chaine e generer                               /
/                     entier : valeur e transformer en chaine                 /
/-------------------------------------------------------------------------DOC*/

INT dv1_str_atoi (STRING * ptr, INT * ent)
{
  INT retour;
  UINT8 fini;
  *ent = 0;
  fini = FALSE;

  retour = FALSE;

  while (!fini)
    {
      if ((**ptr >= '0') && (**ptr <= '9'))
        {
          *ent *= 10;
          *ent += (**ptr) - '0';
        }
      else
        {
          switch (**ptr)
            {
            case 0:
            case '.':
            case '/':
            case '=':
            case ',':
              fini = TRUE;
              retour = TRUE;
              break;
            default:
              fini = TRUE;
              break;
            }
        }
      /* on passe au caractere suivant */
      (*ptr)++;
    }
  return (retour);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_str_atol                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/12/96                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : transforme une chaine en long                        /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : chaine : chaine e generer                               /
/                     entier : valeur e transformer en chaine                 /
/-------------------------------------------------------------------------DOC*/

INT dv1_str_atol (STRING * ptr, LONG * l_ent)
{
  INT retour;
  UINT8 fini;
  *l_ent = 0L;
  fini = FALSE;

  retour = FALSE;

  while (!fini)
    {
      if ((**ptr >= '0') && (**ptr <= '9'))
        {
          *l_ent *= 10L;
          *l_ent += (**ptr) - '0';
        }
      else
        {
          switch (**ptr)
            {
            case 0:
            case '.':
            case '=':
            case '/':
            case ',':
              fini = TRUE;
              retour = TRUE;
              break;
            default:
              fini = TRUE;
              break;
            }
        }
      /* on passe au caractere suivant */
      (*ptr)++;
    }
  return (retour);
}

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_str_litt                                               /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/12/96                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : transforme une chaine en antier                        /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : chaine : chaine e generer                               /
/                     entier : valeur e transformer en chaine                 /
/-------------------------------------------------------------------------DOC*/

INT dv1_str_litt (STRING * ptr, STRING dest, INT nbcarmax)
{
  INT nbcar;
  BYTE fini;
  BYTE flag;
  BYTE flag_cote;
  fini = FALSE;
  flag = FALSE;

  nbcar = 0;

  flag_cote = FALSE;

  while ((!fini) && (nbcar < nbcarmax))
    {
      switch (**ptr)
        {
        case 0:
        case '/':
          fini = TRUE;
          break;

        case '"':
          /* on supprime les cotes */
          if (flag)
            {
              /* pour separer les caissons */
              *dest++ = CR;
              flag = FALSE;
            }
          else
            flag = TRUE;
          flag_cote = TRUE;
          break;
        default:
          *dest++ = **ptr;
          nbcar++;
          flag = FALSE;
          break;
        }
      /* on passe au caractere suivant */
      (*ptr)++;
    }
  if (nbcar)
    {
      /* fin de la commande */
      if (flag_cote)
        {
          *dest++ = LF;
          /* on passe au caractere suivant */
          (*ptr)++;
        }
      else
        *dest++ = 0;
      nbcar++;
    }
  return (nbcar);
}
#ifdef PMV
/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : dv1_str_symb                                           /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 03/12/96                                                 /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : sir_dv1.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : recherche un chaine dans la table des symbole et       /
/                      retourne l'entier correspondant                        /
/                                                                             /
/-----------------------------------------------------------------------------/
/ DONNEES IMPORTEES : chaine : chaine e generer                               /
/                     entier : valeur e transformer en chaine                 /
/-------------------------------------------------------------------------DOC*/

INT dv1_str_symb (STRING * ptr, INT * ent)
{

  INT retour;
  BYTE fini;
  STRING ptr_deb;
  STRING ptr_fin;

 /* on memorise le debut de la chaine */
  ptr_deb = *ptr;

  *ent = 0;
  fini = FALSE;

  if (**ptr)
    {
      /* c'est peut etre du numerique */
      retour = TRUE;
      while (!fini)
        {
          if ((**ptr >= '0') && (**ptr <= '9'))
            {
              *ent *= 10;
              *ent += (**ptr) - '0';
            }
          else
            {
              switch (**ptr)
                {
                case 0:
                case '.':
                case '/':
                case ',':
                  fini = TRUE;
                  /* on met le caractere de fin de chaine */
                  **ptr = 0;
                  ptr_fin = *ptr;
                  ptr_fin++;
                  break;

                default:
                  retour = FALSE;
                  break;
                }
            }
          /* on passe au caractere suivant */
          (*ptr)++;
        }

      *ptr = ptr_fin;
    }
  else
    {
      retour = FALSE;
      (*ptr)++;
    }
  return (retour);
}
#endif

/*DOC-------------------------------------------------------------------------/
/ S.I.A.T.    |     FICHE DE FONCTION                                         /
/-----------------------------------------------------------------------------/
/ NOM DE LA FONCTION : calcul_sec                                             /
/-----------------------------------------------------------------------------/
/ DATE DE CREATION : 28/02/1991                                               /
/-----------------------------------------------------------------------------/
/ UNITE DE COMPILATION : lcr_idf.c                                            /
/-----------------------------------------------------------------------------/
/ BUT DE LA FONCTION : calcul une date en seconde depuis 01/01/70             /
/-------------------------------------------------------------------------DOC*/

ULONG dv1_sec (STRING pt_date)
{
  INT co_mois;                  /* compteur de mois                                       */
  ULONG nb_jours;               /* nombre de jours ecoules depuis la date de reference    */
  ULONG annee;                  /* l'annee complete dans le siecle                        */
  ULONG seconde;

 /* on positionne l'annee dans le siecle courant */
  annee =
    (pt_date[H_YEA] <
     90 ? (ULONG) pt_date[H_YEA] + 2000L : (ULONG) pt_date[H_YEA] + 1900L);

 /* on initialise le nombre de jours avec les annees ecoulees */
  nb_jours = 365L * (annee - 1970L) +  /* nombre de jour dans les annees */
    ((annee - 1970L) + 1L) / 4L;       /* nombre de jours bissextiles    */

 /* pour tous les mois ecoulees depuis le debut de l'annee */
  for (co_mois = 1; co_mois < pt_date[H_MON]; co_mois++)
    {
      /* on ajoute le nombre de jours correspondant au mois */
      switch (co_mois)
        {
        case 1:                       /* janvier  */
        case 3:                       /* mars     */
        case 5:                       /* mai      */
        case 7:                       /* juillet  */
        case 8:                       /* aout     */
        case 10:                      /* octobre  */
        case 12:                      /* decembre */
          nb_jours += 31L;
          break;

        case 4:                       /* avril     */
        case 6:                       /* juin      */
        case 9:                       /* septembre */
        case 11:                      /* novembre  */
          nb_jours += 30L;
          break;

        case 2:                       /* fevrier */
          if ((pt_date[H_YEA] % 4) == 0)
            nb_jours += 29L;           /* annee bissextile     */
          else
            nb_jours += 28L;           /* annee non bissextile */
          break;
        }
    }

 /* on ajoute la date */
  nb_jours += (ULONG) pt_date[H_DAT] - 1L;

 /* on calcule le nombre de secondes ecoulees depuis la date de reference */
  seconde = nb_jours * 24L * 3600L +   /* nombre de jours */
    (ULONG) pt_date[H_HOU] * 3600L +   /* heure courante  */
    (ULONG) pt_date[H_MIN] * 60L +     /* minute courante */
    (ULONG) pt_date[H_SEC];            /* seconde courante */

 /* on retourne les secondes */
  return (seconde);
}
