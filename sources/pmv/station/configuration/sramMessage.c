#include <stdio.h>
#include <string.h>
#include <kcommon.h>
#include <klog.h>
#include <tokenizerDirif.h>
#include "configuration.h"
#include "sramMessage.h"
#include "cpu432/sram.h"

#include "standard.h"
#include "eriLib.h"

#include "pip_def.h"
#include "pip_str.h"

/* ************************************
 * DEFINITION DES TYPES
 * ************************************	*/

/* Donnees stockées en SRAM */
typedef struct _SramMessage
{
	T_pip_sv_act_caisson caisson[NB_CAISSON];
	int32 checksum_dw;
} SramMessage;

/* ************************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ************************************	*/

static void _sramMessageInit(void);
static int32 _sramMessageCalculerChecksum(void);
static void _sramMessageEcrireChecksum(void);
static bool _sramMessageControlerChecksum(void);

/* ************************************
 * DECLARATION DES VARIABLES LOCALES
 * ************************************	*/

static Sram *sramMessage_pt=NULL;

/* ************************************
 * FONCTIONS DU MODULE
 * ************************************ */


int32 sramMessageInit(void *adresseMessages_p)
{
int32 taille_dw=(sizeof(SramMessage)/1024+1)*1024;


	sramMessage_pt=sramNew(adresseMessages_p,taille_dw);
	if(false==_sramMessageControlerChecksum())
	{
		_sramMessageInit();
	}
	return (c4sramGetTotalSize(sramMessage_pt)/sizeof(long)+1)*sizeof(long);;
}


void sramMessageTerm()
{
	sramDelete(sramMessage_pt);
}

/* _configSramInit
 * ===============
 * Dans le cas ou le checksum de la sram est defectueux, on
 * realise une reinitialisatio de la sram.
 * */

static void _sramMessageInit()
{
SramMessage message_t;
int numCaisson;
	memset(&message_t,0,sizeof(SramMessage));
	for(numCaisson=0;numCaisson<NB_CAISSON;numCaisson++)
	{
		sramMessageEcrireCaisson(numCaisson,&message_t.caisson[numCaisson]);
	}
	_sramMessageEcrireChecksum();
}
static int32 _sramMessageCalculerChecksum()
{
	SramMessage message_t;
	int32 checksum=0;
	sramRead(sramMessage_pt,0,sizeof(SramMessage),1,&message_t);
	{
		int offsetDeb=(int)(&((SramMessage *)0)->caisson);
		int offsetFin=(int)(&((SramMessage *)0)->checksum_dw);
		int offset;
		for(offset=offsetDeb;offset<offsetFin;offset++)
		{
			checksum+=((uint8 *)&message_t)[offset];
		}
	}
	return checksum;
}

static void _sramMessageEcrireChecksum()
{
	int32 checksum=_sramMessageCalculerChecksum();
	/* Le checksum est situé en début de bloc. */
	sramWrite(sramMessage_pt,(int32)&(((SramMessage *)0)->checksum_dw),sizeof(int32),1,&checksum);
}

static bool _sramMessageControlerChecksum()
{
	int32 checksumCalc_dw=_sramMessageCalculerChecksum();
	int32 checksumEnreg_dw;
	bool retour_b=false;
	if(sramRead(sramMessage_pt,(int32)&(((SramMessage *)0)->checksum_dw),sizeof(int32),1,&checksumEnreg_dw)==sizeof(int32))
	{
		retour_b=(checksumCalc_dw==checksumEnreg_dw);
	}
	return retour_b;
}

bool sramMessageEcrireCaisson(uint32 numCaisson_udw,void *caisson_p)
{
bool retour_b=false;

	if(numCaisson_udw<NB_CAISSON)
	{
	sramWrite(sramMessage_pt,(int32)&(((SramMessage *)0)->caisson[numCaisson_udw]),sizeof(T_pip_sv_act_caisson),1,caisson_p);
	_sramMessageEcrireChecksum();
	retour_b=true;
	}
	return retour_b;
}

bool sramMessageLireCaisson(uint32 numCaisson_udw,void *caisson_p)
{
	bool retour_b=false;

		if(numCaisson_udw<NB_CAISSON)
		{
		sramRead(sramMessage_pt,(int32)&(((SramMessage *)0)->caisson[numCaisson_udw]),sizeof(T_pip_sv_act_caisson),1,caisson_p);
		retour_b=true;
		}
		return retour_b;

}

void sramMessageLireTout(void *caisson_p)
{
		sramRead(sramMessage_pt,(int32)(((SramMessage *)0)->caisson),sizeof(T_pip_sv_act_caisson),NB_CAISSON,caisson_p);
}

void sramMessageEcrireTout(void *caisson_p)
{
		sramWrite(sramMessage_pt,(int32)(((SramMessage *)0)->caisson),sizeof(T_pip_sv_act_caisson),NB_CAISSON,caisson_p);
	_sramMessageEcrireChecksum();
}
