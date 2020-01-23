#include <stdio.h>
#include <string.h>
#include <kcommon.h>
#include <klog.h>
#include <tokenizerDirif.h>
#include "configuration.h"
#include "sramBdd.h"
#include "cpu432/sram.h"

#include "standard.h"
#include "eriLib.h"

#include "pip_def.h"
#include "pip_str.h"

/* ************************************
 * DEFINITION DES TYPES
 * ************************************	*/

/* Donnees stockées en SRAM */
typedef struct _SramBdd
{
	T_zdf fichier[MAXFIC];
	int32 checksum_dw;
} SramBdd;

/* ************************************
 * PROTOTYPAGE DES FONCTIONS LOCALES
 * ************************************	*/

static void _sramBddInit(void);
static int32 _sramBddCalculerChecksum(void);
static void _sramBddEcrireChecksum(void);
static bool _sramBddControlerChecksum(void);

/* ************************************
 * DECLARATION DES VARIABLES LOCALES
 * ************************************	*/

static Sram *sramBdd_pt=NULL;
static Sram *sramEnreg_pt=NULL;
static int32 _sramBddTaille_dw=0;

/* ************************************
 * FONCTIONS DU MODULE
 * ************************************ */

int32 sramBddTailleGet(void)
{
	return _sramBddTaille_dw;
}

void sramBddTailleSet(int32 taille_dw)
{
	_sramBddTaille_dw=taille_dw;
}
int32 sramBddInit(void *adresseBase_p,int32 tailleMax_dw)
{
int32 tailleDescripteur_dw=(sizeof(SramBdd)/1024+1)*1024;
int32 offsetDescripteur_dw = tailleDescripteur_dw + 1024;
int32 tailleBdd_dw=0;


	sramBdd_pt=sramNew(adresseBase_p,tailleDescripteur_dw);

	offsetDescripteur_dw=(c4sramGetTotalSize(sramBdd_pt)/sizeof(long)+1)*sizeof(long)+1024;

	if( ( tailleMax_dw - offsetDescripteur_dw ) > 0 ) {
		void *adresseEnreg_p = (void *) ( ( (char *) adresseBase_p ) + offsetDescripteur_dw );
		int32 tailleEnreg_dw = tailleMax_dw - offsetDescripteur_dw-1024;

		sramEnreg_pt = sramNew( adresseEnreg_p, tailleEnreg_dw );
		sramBddTailleSet( tailleEnreg_dw );
		tailleBdd_dw=(c4sramGetTotalSize(sramEnreg_pt)/sizeof(long)+1)*sizeof(long);
		if( null == sramEnreg_pt ) {
			printf( "sramBddInit : Probleme d'initialisation de la base... " );
		}

	}
	if(false==_sramBddControlerChecksum())
	{
		_sramBddInit();
	}

	return offsetDescripteur_dw+tailleBdd_dw;
}


void sramBddTerm()
{
	sramDelete(sramEnreg_pt);
	sramDelete(sramBdd_pt);
}

/* _configSramInit
 * ===============
 * Dans le cas ou le checksum de la sram est defectueux, on
 * realise une reinitialisatio de la sram.
 * */

static void _sramBddInit()
{
SramBdd message_t;
int numFichier;
	memset(&message_t,0,sizeof(SramBdd));
	for(numFichier=0;numFichier<MAXFIC;numFichier++)
	{
		sramBddEcrireFichier(numFichier,&message_t.fichier[numFichier]);
	}
	_sramBddEcrireChecksum();
}
static int32 _sramBddCalculerChecksum()
{
	SramBdd message_t;
	int32 checksum=0;
	sramRead(sramBdd_pt,0,sizeof(SramBdd),1,&message_t);
	{
		int offsetDeb=(int)(&((SramBdd *)0)->fichier);
		int offsetFin=(int)(&((SramBdd *)0)->checksum_dw);
		int offset;
		for(offset=offsetDeb;offset<offsetFin;offset++)
		{
			checksum+=((uint8 *)&message_t)[offset];
		}
	}
	return checksum;
}

static void _sramBddEcrireChecksum()
{
	int32 checksum=_sramBddCalculerChecksum();
	/* Le checksum est situé en début de bloc. */
	sramWrite(sramBdd_pt,(int32)&(((SramBdd *)0)->checksum_dw),sizeof(int32),1,&checksum);
}

static bool _sramBddControlerChecksum()
{
	int32 checksumCalc_dw=_sramBddCalculerChecksum();
	int32 checksumEnreg_dw;
	bool retour_b=false;
	if(sramRead(sramBdd_pt,(int32)&(((SramBdd *)0)->checksum_dw),sizeof(int32),1,&checksumEnreg_dw)==sizeof(int32))
	{
		retour_b=(checksumCalc_dw==checksumEnreg_dw);
	}
	return retour_b;
}

bool sramBddEcrireFichier(uint32 numFichier_udw,void *fichier_p)
{
bool retour_b=false;

	if(numFichier_udw<MAXFIC)
	{
	sramWrite(sramBdd_pt,(int32)&(((SramBdd *)0)->fichier[numFichier_udw]),sizeof(T_zdf),1,fichier_p);
	_sramBddEcrireChecksum();
	retour_b=true;
	}
	return retour_b;
}

bool sramBddLireFichier(uint32 numFichier_udw,void *fichier_p)
{
	bool retour_b=false;

		if(numFichier_udw<MAXFIC)
		{
		sramRead(sramBdd_pt,(int32)&(((SramBdd *)0)->fichier[numFichier_udw]),sizeof(T_zdf),1,fichier_p);
		retour_b=true;
		}
		return retour_b;

}

void sramBddLireTout(void *fichier_p)
{
		sramRead(sramBdd_pt,(int32)(((SramBdd *)0)->fichier),sizeof(T_zdf),MAXFIC,fichier_p);
}

void sramBddEcrireTout(void *fichier_p)
{
		sramWrite(sramBdd_pt,(int32)(((SramBdd *)0)->fichier),sizeof(T_zdf),MAXFIC,fichier_p);
	_sramBddEcrireChecksum();
}

void sramBddLireEnreg( int32 dest,int taille,void *info_p)
{
	if(NULL!=sramEnreg_pt)
	{
		if(taille!=sramRead(sramEnreg_pt,dest,sizeof(char),taille,info_p))
		{
			printf("Il y une erreur de lecture %d\n",errno);
		}
	}
}

void sramBddEcrireEnreg( int32 dest,int taille,void *info_p)
{
	if(NULL!=sramEnreg_pt)
	{
	int tailleEcrite=sramWrite(sramEnreg_pt,dest,sizeof(char),taille,info_p);
		if(taille!=tailleEcrite)
		{
			printf("Il y une erreur d ecriture %d\n",errno);
		}
	}
}
