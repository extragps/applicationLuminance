

#include <stdio.h>
#include <string.h>
#include "kcommon.h"
#include "klog.h"
#include "affichage/zone.h"
#include "affichage/page.h"
#include "affichage/caractere.h"
#include "affichage/affichage.h"
#include "infos/lumInfos.h"
#include "lumAdresse.h"
#include "lumMsg.h"
#include "luminance/lumMsgCreer.h"
#include "affichage/afficheur.h"
#include "es/es.h"
#include "thl/thl.h"
#include "event.h"
#include "cartes.h"
#include "configuration.h"


void stationInit(void)
{
	thlsInit();
	essInit();
	afficheursInit();
}

void stationTerm(void)
{
	afficheursTerm();
	essTerm();
	thlsTerm();
}

bool stationIsOk(void)
{
bool retour=false;
	retour=essIsOk();
//	retour&=thlsIsOk();
	retour&=afficheursIsOk();
	retour|=configIsRebouclage();
	return retour;
}

void stationReset(void)
{
	esResetSabordage();
	thlsReset();
	afficheursReset();
}

bool stationIsDefautCom(void)
{
bool retour_b=false;
	retour_b=essIsDefautCom()||afficheursIsDefautCom();
	return retour_b;
}

