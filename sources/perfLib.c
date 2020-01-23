/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 06/02/01
 * Fichier	: simuLib.c
 * Objet	: Le fichier defini toutes les procedures necessaires e la
 * 		simulation de vehicules...
 * 		Pour l'heure, la librairie n'est pas reentrante et donc ne constitue
 * 		pas un objet instanciable.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,01Dec00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include "stdio.h"
#include "portage.h"
#include "time.h"
#include "limits.h"
#include "standard.h"
#include "perfLib.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

void perfInit (T_perfElem * elem)
{
  elem->nbFois = 0;
  elem->moyenne = 0;
  elem->min = ULONG_MAX;
  elem->max = 0;
  elem->cour = 0;
}

void perfDemarrer (T_perfElem * elem)
{
  elem->tickDebut = tickGet ();
}

void perfArreter (T_perfElem * elem)
{
  ULONG current = tickGet ();
  if (current < elem->tickDebut)
    {
      elem->cour = current + (ULONG_MAX - elem->tickDebut);
    }
  else
    {
      elem->cour = current - elem->tickDebut;
    }
  if (PERF_NB_FOIS_MAX < elem->nbFois)
    {
      elem->nbFois++;
      elem->moyenne =
        ((elem->moyenne) * (PERF_NB_FOIS_MAX - 1) +
         elem->cour) / PERF_NB_FOIS_MAX;
    }
  else
    {
      elem->nbFois++;
      elem->moyenne = ((elem->moyenne) * (elem->nbFois - 1) + elem->cour) /
        elem->nbFois;
    }
  if (elem->cour > elem->max)
    {
      elem->max = elem->cour;
    }
  if (elem->cour < elem->min)
    {
      elem->min = elem->cour;
    }
}

void perfImprimer (T_perfElem * elem)
{
  printf ("NbFois= %d moyenne= %ld min= %ld max= %ld cour= %ld",
          elem->nbFois, elem->moyenne, elem->min, elem->max, elem->cour);
}
