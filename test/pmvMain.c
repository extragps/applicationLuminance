/*
 * pmvMain.c
 * ===================
 *
 *  Created on: 24 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: pmvMain.c,v $
 * Revision 1.2  2018/06/04 08:38:42  xg
 * Passage en version V122
 *
 * Revision 1.4  2008/09/22 13:41:07  xag
 * Ajout d'un parametre pour les traces des performances.
 *
 * Revision 1.3  2008/09/22 07:53:44  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/07/02 15:30:21  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */
#include <string.h>
#include <stdio.h>
#include "kcommon.h"
#include "portage.h"

/*
 * main
 * =====
 * Fonction de lancement....
 */
int main(int argc, char * argv[])
{
int indice=0;
int typeBoot=1;
bool debug=false;
bool perf=false;

	while(indice<argc)
	{
		if(0==strcmp(argv[indice],"-appli"))
		{
			typeBoot=1;
		}
		else if(0==strcmp(argv[indice],"-sommeil"))
		{
			typeBoot=0;
		}
		else if (0==strcmp(argv[indice],"-debug"))
		{
			debug=true;
		}
		else if(0==strcmp(argv[indice],"-perf"))
		{
			perf=true;
		}
		else if((0==strcmp(argv[indice],"-help"))|| (0==strcmp(argv[indice],"-?")))
		{
			printf("Syntaxe :  pmvMain [-appli] [-sommeil] [-perf] [-debug] [-help]\n");
		}
		indice++;
	}
	kcommonInit(debug);
	msgQInit();
	superviseurMain(typeBoot,perf);
	msgQTerm();
	kcommonEnd();

	return 0;
}

/* *********************************
 * FIN DE pmvMain.c
 * ********************************* */
