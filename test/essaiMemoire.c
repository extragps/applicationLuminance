#include "kcommon.h"

#include "stdio.h"

int main(int argc,char *argv[])
{
int indice=0;
void *tableau[10];
	for(indice=0;indice<10;indice++)
	{
		printf("Allocation de %d octets \n",1000*indice+30000);
		tableau[indice]=kmmAlloc(NULL,1000*indice+30000);
		if(NULL!=tableau[indice])
		{
			printf("Alloc correcte\n");
		}
	}
		for(indice=0;indice<10;indice++)
	{
		printf("Liberation %d octets \n",indice*1000+30000);
		if(NULL!=tableau[indice])
		{
		kmmFree(NULL, tableau[indice]);
			printf("Liberation correcte\n");
		}
	}
		for(indice=3;indice<5;indice++)
	{
		printf("Allocation de %d octets \n",indice*10000);
		tableau[indice]=kmmAlloc(NULL,10000*indice);
		if(NULL!=tableau[indice])
		{
			printf("Alloc correcte\n");
		}
	}
		for(indice=3;indice<5;indice++)
	{
		printf("Liberation %d octets \n",indice*10000);
		if(NULL!=tableau[indice])
		{
		kmmFree(NULL, tableau[indice]);
			printf("Liberation correcte\n");
		}
	}
	return 0;
}
