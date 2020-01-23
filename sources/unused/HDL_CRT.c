/*
 * hdl_crt.c
 * ===================
 *
 *  Created on: 23 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: HDL_CRT.c,v $
 * Revision 1.2  2018/06/04 08:38:44  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/09/12 15:02:48  xag
 * Archivage de printemps
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* ******************************
 * LISTE DES INCLUDES
 * ******************************* */
#include "standard.h"
#include "portage.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"

/* ******************************
 * DEFINTION DES CONSTANTES
 * ******************************* */

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* ******************************
 * VARIABLES LOCALES
 * ******************************* */

//static int valLon=0xAAAA5555;
//static int lgLon=2;

/* ******************************
 * FONCTIONS DU MODULE
 * ******************************* */

/*
 * TODO : La fonction hdl_crt_init doit changer car la gestion des cartes est realisee
 * differement sur linux.
 */

VOID hdl_crt_init (VOID)
{
  BYTE fini;
  INT i, j;
  UINT *pt_adr;
  ULONG *pt_long;
//  BYTE *pt_byte;
  UINT val;
  LONG adresse;
  xdg_ctrl_rack.flag_carte = TRUE;
  fini = FALSE;
  if (xdg_ctrl_rack.bcc != 0x12345678)
    {
	  mgpInit();
      xdg_ctrl_rack.nb_crt_MEO116 = 0;
      xdg_ctrl_rack.nb_crt_MSO116 = 0;
      xdg_ctrl_rack.nb_crt_LON100 = 0;
      xdg_ctrl_rack.nb_crt_MSR116 = 0;
      for (j = 0; j < NB_CRT_LON100; j++)
        {
          xdg_ctrl_rack.crt_LON100[j] = FALSE;
        }

      for (j = 0; j < NB_CRT_MEO116; j++)
        {
          xdg_ctrl_rack.crt_MEO116[j] = FALSE;
        }

      for (j = 0; j < NB_CRT_MSO116; j++)
        {
          xdg_ctrl_rack.crt_MSO116[j] = FALSE;
        }

      for (j = 0; j < NB_CRT_MSR116; j++)
        {
          xdg_ctrl_rack.crt_MSR116[j] = FALSE;
          xdg_ctrl_rack.outputMSR116[j] = 0;
        }
      xdg_ctrl_rack.crt_COM104 = FALSE;
      xdg_ctrl_rack.crt_DSR111 = FALSE;
      /* on initialise la zone de detection des cartes */
      xdg_ctrl_rack.phase = 1;

      for (j = 0; j < NB_CRT_MSO116; j++)
        {
          xdg_ctrl_rack.inputMSO116[j] = 0;
          xdg_ctrl_rack.outputMSO116[j] = 0;
        }
    }
  /* Branchement de la routine de traitement des erreurs */
  /* Positionnemement de la routine de traitement du bus erreur */
  while (!fini)
    {
		printDebug("hdl_crt_init : phase %d\n",xdg_ctrl_rack.phase);
      switch (xdg_ctrl_rack.phase)
        {
        case 0:
          xdg_ctrl_rack.phase = 1;
          break;

        case 1:
          /* presence des cartes entrees TOR MEO116 */
          xdg_ctrl_rack.phase = 2;

          adresse = AD_MEO116;

          for (i = 0; i < NB_CRT_MEO116; i++)
            {
//			int retour=0;
//			int valeur;
//	/* TODO : Portage */
//#ifdef VXWORKS
//				retour=vxMemProbe((char *)adresse,VX_READ,4,&valeur);
//              if (OK == retour)
//                {
//                  /* si on a pas eu de bus error,la carte
//                     entree tor est presente */
//                  xdg_ctrl_rack.crt_MEO116[i] = TRUE;
//                  xdg_ctrl_rack.nb_crt_MEO116++;
//                }
//              /* carte suivante */
//#endif
              adresse += TAILLE_MEO116;
            }
          break;
        case 2:
          /* presence des cartes sorties TOR MSO116 */
          xdg_ctrl_rack.phase = 3;
          adresse = AD_MSO116;
          for (i = 0; i < NB_CRT_MSO116; i++)
            {
//			int retour=0;
//			int valeur;
//	/* TODO : Portage */
//#ifdef VXWORKS
//				retour=vxMemProbe((char *)adresse,VX_WRITE,4,&valeur);
//              if (OK == retour)
//                {
//                  xdg_ctrl_rack.crt_MSO116[i] = TRUE;
//                  xdg_ctrl_rack.nb_crt_MSO116++;
//                }
//              /* carte suivante */
//              adresse += TAILLE_MSO116;
//#endif
            }
          break;
        case 3:
          /* presence des cartes TC TS FAV */
          xdg_ctrl_rack.phase = 4;
	  mgpTesterCartes();
          break;
        case 4:
          xdg_ctrl_rack.phase = 5;
          /* presence des cartes COM */
          if (0)
            {
              adresse = AD_COM104;
              pt_adr = (UINT *) adresse;
              val = *pt_adr;
              /* si on a pas eu de bus error,la carte com104 est presente */
              xdg_ctrl_rack.crt_COM104++;
            }
          break;
        case 5:
          /* presence carte DSRAM  */
          xdg_ctrl_rack.phase = 6;
          if (0)
            {
              adresse = 0x400000;
              pt_long = (ULONG *) adresse;
              *pt_long = 0x55555555;
              /* si on arrive ici la carte ram est presente */
              if (*pt_long == 0x55555555)
                {
                  xdg_ctrl_rack.crt_DSR111++;
                }
            }
          break;
        case 6:
          /* presence carte LON100  */
          xdg_ctrl_rack.phase = 7;
          adresse = AD_LON100;
          for (i = 0; i < NB_CRT_LON100; i++)
            {
//			int retour=0;
//			int valeur=valLon;
//			int ok=FALSE;
//	/* TODO : Portage */
//#ifdef VXWORKS
//				retour=vxMemProbe((char *)adresse,VX_WRITE,lgLon,&valeur);
//              if (OK == retour)
//			  {
//				taskDelay(1);
//				retour=vxMemProbe((char *)adresse,VX_READ,lgLon,&valeur);
//              	if (OK == retour)
//			  	{
//              	if (valeur == valLon)
//                	{
//						printDebug("Carte LON trouvee %#0x\n",adresse);
//                  		ok = TRUE;
//                  /* si on arrive ici la carte lon100 est presente */
//                	}
//			  		else
//			  		{
//						printDebug("Probleme de lect de valeur %#0x\n",valeur);
//						retour=vxMemProbe((char *)adresse,VX_READ,4,&valeur);
//						printDebug("1robleme de lect de valeur %#0x\n",valeur);
//						retour=vxMemProbe((char *)adresse,VX_READ,4,&valeur);
//						printDebug("2robleme de lect de valeur %#0x\n",valeur);
//						valeur=0;
//						retour=vxMemProbe((char *)adresse,VX_READ,2,&valeur);
//						printDebug("3robleme de lect de valeur %#0x\n",valeur);
//
//			  		}
//			  	}
//                if (TRUE == ok)
//                {
//                    xdg_ctrl_rack.nb_crt_LON100++;
//                    xdg_ctrl_rack.crt_LON100[i] = TRUE;
//                }
//			  }
//#endif
              /* carte suivante */
              adresse += TAILLE_LON100;
            }
          break;
        case 7:
          /* presence des cartes sorties TOR MSR116 */
          xdg_ctrl_rack.phase = 8;
          adresse = AD_MSR116;
          for (i = 0; i < NB_CRT_MSR116; i++)
            {
//			int retour=0;
//			int valeur;
//	/* TODO : Portage */
//#ifdef VXWORKS
//				retour=vxMemProbe((char *)adresse,VX_READ,4,&valeur);
//              if (OK == retour)
//			  {
//                  xdg_ctrl_rack.crt_MSR116[i] = TRUE;
//                  xdg_ctrl_rack.nb_crt_MSR116++;
//                }
//#endif
//              /* carte suivante */
              adresse += TAILLE_MSR116;
            }
          break;
        case 8:
          fini = TRUE;
          break;
        }
    }
  xdg_ctrl_rack.bcc = 0L;
  xdg_ctrl_rack.flag_carte = FALSE;
}
