/*
 * configurationEtat.c
 * ===================
 *
 *  Created on: 8 sept. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: configurationEtat.c,v $
 * Revision 1.2  2018/06/04 08:38:42  xg
 * Passage en version V122
 *
 * Revision 1.3  2008/10/13 08:18:17  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.2  2008/09/29 07:58:54  xag
 * Archivage seconde semaine à Tours.
 *
 * Revision 1.1  2008/09/12 15:01:00  xag
 * Archivage de printemps
 *
 */
#include <stdio.h>
#include <string.h>
#include <kcommon.h>
#include <klog.h>
#include <tokenizerDirif.h>
#include "rdtComm.h"
#include "pip_def.h"
#include "ecouteur.h"
#include "event.h"
#include "configuration.h"
#include "sequenceur.h"
#include "sequenceurTraiter.h"
#include "configurationPriv.h"
#include "configurationSram.h"
#include "cpu432/sram.h"
#include "lumAdresse.h"
#include "lumMsg.h"
#include "luminance/lumMsgCreer.h"
#include "lumTraiter.h"
#include "infos/lumInfos.h"
#include "affichage/zone.h"
#include "affichage/page.h"
#include "affichage/caractere.h"
#include "affichage/affichage.h"
#include "affichage/afficheur.h"
#include "thl/thl.h"
#include "es/es.h"
#include  "station/cartesAfficheur.h"

#include "pip_str.h"
#include "pip_var.h"

int32 configEtatCaisson(int32 numCaisson_dw)
{
int32 retour_b=0;
	/* Pour connaitre l'etat du caisson, il faut lire l'etat de tous les afficheurs... */
	return retour_b;
}

int32 configEtatAffGetNbDefautsPixel(int32 numAff_dw)
{
	int32 retour_b = 0;
	if ((0 <= numAff_dw) && (numAff_dw < configGetNbAfficheurs()))
	{
		Afficheur *aff_pt = afficheursChercherParIndex(numAff_dw);
		if (NULL != aff_pt)
		{
			retour_b=afficheurGetDefautPixel(aff_pt);
		}
	}
	return retour_b;
}
/* ********************************
 * FIN DE configurationEtat.c
 * ********************************	*/

