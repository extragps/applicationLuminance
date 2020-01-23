

#include "stdio.h"
#include "mon_debug.h"
#include "stdarg.h"

/* ********************************
 * VARIABLES LOCALES
 * ********************************	*/

static int monDebugEtatModule[MON_DEBUG_MAX_MODULE];

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

void monDebugInit (void)
{
  int module;
  for (module = 0; module < MON_DEBUG_MAX_MODULE; module++)
    {
      monDebugEtatModule[module] = 1 ;
    }                                  /* endfor(module=0;module<                      */
  /* --------------------------------
   * FIN DE monDebugInit
   * --------------------------------     */
}

void monDebugSet (E_monDebugModule module, int valeur)
{
  if ((0 <= module) && (MON_DEBUG_MAX_MODULE > module))
    {
      monDebugEtatModule[module] = valeur;
    }
}

int monDebugGet (E_monDebugModule module)
{
  int retour = 0;
  if ((0 <= module) && (MON_DEBUG_MAX_MODULE > module))
    {
      retour = monDebugEtatModule[module];
    }
  return retour;
}


int monPrintDebug (const char *chaine, ...)
{
/*
 * va_list liste;
	va_start(liste,chaine);
	return(vprintf(chaine,liste));
 */
  return 0;
}

int monPrintDebugBis (int imp,const char *chaine, ...)
{
	if(0!=imp)
	{
 	va_list liste;
		va_start(liste,chaine);
		return(vprintf(chaine,liste));
	}
	else
	{
  		return 0;
	}
}

