/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |    FICHE DE SOUS-MODULE                                       /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cfpl                                              /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/03/1999                                               /
 /-----------------------------------------------------------------------------/
 / FONCTION DU SOUS-MODULE :
 / Configuration des polices de caracteres                                     /
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
#include "standard.h"

#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "ted_prot.h"

#include "sir_dv1h.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"
#include "tac_conf.h"

#include "sir_dv1h.h"
#include "x01_str.h"
#include "x01_var.h"
#include "lcr_cfpl.h"

#include "affichage/caractere.h"

/**********************************************************/
/*                                                        */
/*            prototypage des fonctions                   */
/*                                                        */
/**********************************************************/

static VOID cmd_exec_cfpl(struct cmd_cfpl *);
static INT lcr_cmd_cfpl(STRING, INT, INT, UINT *, UINT *);

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cfpl_init                                          /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/08/1999                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cfpl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : initialisation des polices de caractere                /
 /-------------------------------------------------------------------------DOC*/
//static void pip_car_init(BYTE *pol, BYTE car1, BYTE car2, BYTE car3, BYTE car4, BYTE car5)
//{
//	int indice = 0;
//	pol[indice++] = car1;
//	pol[indice++] = car2;
//	pol[indice++] = car3;
//	pol[indice++] = car4;
//	pol[indice++] = car5;
//}
static void _lcr_cfpl_remplace(int car,int i,int j,int k, int l, int m)
{
Caractere *car_pt = caractereNew(car,i,j,k,l,m);
Caractere *ancCar_pt = caractereTableRechercher(car);
if (NULL != ancCar_pt)
{
	*ancCar_pt = *car_pt;
	caractereDelete(car_pt);
} else
{
	caractereAdd(car_pt);
}
}

VOID lcr_cfpl_init(void)
{
	int indCour=0x21;
	/* table de configuration des polices de caracteres */
	caracteresDeleteTable();
	caracteresInitTableVierge();
	//int indice;
	//		for(indice=0;indice<MAX_CAR_POLICE;indice++)
	//		{
	//		int indCol;
	//			for(indCol=0;indCol<5;indCol++)
	//			{
	//				pip_police[indice[indCol]=0;
	//			}
	//		}
	//		/* INIT CARACTERES DU JEU J6 */
			_lcr_cfpl_remplace(indCour++,0,0,0xAF,0,0);
			_lcr_cfpl_remplace(indCour++,0,0x07,0,0x07,0); /* " */
			_lcr_cfpl_remplace(indCour++,0x50,0x7E,0x51,0x41,0x22);
			_lcr_cfpl_remplace(indCour++,0x46,0x49,0x7F,0x49,0x31); /* $ */
			_lcr_cfpl_remplace(indCour++,0x23,0x13,0x08,0x64,0x62); /* % */
			_lcr_cfpl_remplace(indCour++,0x36,0x49,0x55,0x22,0x50); /* & */
			_lcr_cfpl_remplace(indCour++,0,0x5,0x3,0,0); /*'*/
			_lcr_cfpl_remplace(indCour++,0,0x1C,0x22,0x41,0); /* ( */
			_lcr_cfpl_remplace(indCour++,0,0x41,0x22,0x1C,0); /* ) */
			_lcr_cfpl_remplace(indCour++,0x12,0x0C,0x3F,0x0C,0x12); /* * */
			_lcr_cfpl_remplace(indCour++,0x10,0x10,0x7C,0x10,0x10); /* + */
			_lcr_cfpl_remplace(indCour++,0,0x50,0x30,0,0); /* , */
			_lcr_cfpl_remplace(indCour++,0x10,0x10,0x10,0x10,0x10);
			_lcr_cfpl_remplace(indCour++,0,0x60,0x60,0,0); /* . */
			_lcr_cfpl_remplace(indCour++,0x20,0x10,0x08,0x04,0x02); /* / */
			_lcr_cfpl_remplace(indCour++,0x3E,0x41,0x41,0x3E,0); /* 0 */
			_lcr_cfpl_remplace(indCour++,0,0x42,0x7F,0x40,0); /* 1 */
			_lcr_cfpl_remplace(indCour++,0x62,0x51,0x49,0x49,0x46); /* 2 */
			_lcr_cfpl_remplace(indCour++,0x22,0x41,0x49,0x49,0x36);
			_lcr_cfpl_remplace(indCour++,0x18,0x14,0x12,0x7F,0x10);
			_lcr_cfpl_remplace(indCour++,0x27,0x45,0x45,0x45,0x39);
			_lcr_cfpl_remplace(indCour++,0x3C,0x4A,0x49,0x49,0x30); /* 6 */
			_lcr_cfpl_remplace(indCour++,0x1,0x71,0x9,0x5,0x3); /* 7 */
			_lcr_cfpl_remplace(indCour++,0x36,0x49,0x49,0x49,0x36);
			_lcr_cfpl_remplace(indCour++,0x6,0x49,0x49,0x29,0x1E);
			_lcr_cfpl_remplace(indCour++,0,0x36,0x36,0,0);
			_lcr_cfpl_remplace(indCour++,0,0x56,0x36,0,0); /* ; */
			_lcr_cfpl_remplace(indCour++,0x8,0x14,0x22,0x41,0);
			_lcr_cfpl_remplace(indCour++,0x28,0x28,0x28,0x28,0x28);
			_lcr_cfpl_remplace(indCour++,0,0x41,0x22,0x14,0x8);
			_lcr_cfpl_remplace(indCour++,0x2,0x1,0x51,0x9,0x6);
			_lcr_cfpl_remplace(indCour++,0x20,0x54,0x55,0x56,0x78);
			_lcr_cfpl_remplace(indCour++,0x7C,0x12,0x11,0x12,0x7C);
			_lcr_cfpl_remplace(indCour++,0x41,0x7F,0x49,0x49,0x36);
			_lcr_cfpl_remplace(indCour++,0x3E,0x41,0x41,0x41,0x22); /* C */
			_lcr_cfpl_remplace(indCour++,0x41,0x7F,0x41,0x41,0x3E);
			_lcr_cfpl_remplace(indCour++,0x7F,0x49,0x49,0x41,0x41);
			_lcr_cfpl_remplace(indCour++,0x7F,0x9,0x9,0x1,0x1);
			_lcr_cfpl_remplace(indCour++,0x3E,0x41,0x41,0x51,0x71);
			_lcr_cfpl_remplace(indCour++,0x7F,0x8,0x8,0x8,0x7F);
			_lcr_cfpl_remplace(indCour++,0,0x41,0x7F,0x41,0);
			_lcr_cfpl_remplace(indCour++,0x20,0x40,0x40,0x40,0x3F); /* J */
			_lcr_cfpl_remplace(indCour++,0x7F,0x8,0x14,0x22,0x41);
			_lcr_cfpl_remplace(indCour++,0x7F,0x40,0x40,0x40,0x40);
			_lcr_cfpl_remplace(indCour++,0x7F,0x2,0xC,0x2,0x7F); /* M*/
			_lcr_cfpl_remplace(indCour++,0x7F,0x4,0x8,0x10,0x7F);
			_lcr_cfpl_remplace(indCour++,0x3E,0x41,0x41,0x41,0x3E);
			_lcr_cfpl_remplace(indCour++,0x7F,0x9,0x9,0x9,0x6);
			_lcr_cfpl_remplace(indCour++,0x3E,0x41,0x51,0x21,0x5E);
			_lcr_cfpl_remplace(indCour++,0x7F,0x9,0x19,0x29,0x46); /* R */
			_lcr_cfpl_remplace(indCour++,0x26,0x49,0x49,0x49,0x32);
			_lcr_cfpl_remplace(indCour++,0x1,0x1,0x7F,0x1,0x1);
			_lcr_cfpl_remplace(indCour++,0x3F,0x40,0x40,0x40,0x3F);
			_lcr_cfpl_remplace(indCour++,0x7,0x18,0x60,0x18,0x07);
			_lcr_cfpl_remplace(indCour++,0x7F,0x20,0x18,0x20,0x7F);
			_lcr_cfpl_remplace(indCour++,0x63,0x14,0x08,0x14,0x63);
			_lcr_cfpl_remplace(indCour++,0x3,0x4,0x78,0x4,0x3);
			_lcr_cfpl_remplace(indCour++,0x61,0x51,0x49,0x45,0x43);
			_lcr_cfpl_remplace(indCour++,0x6,0x9,0x9,0x6,0); /* e */
			_lcr_cfpl_remplace(indCour++,0xC,0x12,0x52,0x32,0x12);
			_lcr_cfpl_remplace(indCour++,0xA,0x55,0x55,0x55,0x28); /* e */
			_lcr_cfpl_remplace(indCour++,0x4,0x2,0x1,0x2,0x4);
			_lcr_cfpl_remplace(indCour++,0x40,0x40,0x40,0x40,0x40);
			_lcr_cfpl_remplace(indCour++,0x7F,0x10,0x10,0xF,0x10);
			_lcr_cfpl_remplace(indCour++,0x20,0x54,0x54,0x54,0x78); /* a */
			_lcr_cfpl_remplace(indCour++,0x7F,0x44,0x44,0x44,0x38);
			_lcr_cfpl_remplace(indCour++,0x30,0x48,0x48,0x48,0x48);
			_lcr_cfpl_remplace(indCour++,0x30,0x48,0x48,0x48,0x7F);
			_lcr_cfpl_remplace(indCour++,0x38,0x54,0x54,0x54,0x18);
			_lcr_cfpl_remplace(indCour++,0x8,0x7E,0x9,0x1,0x2);
			_lcr_cfpl_remplace(indCour++,0x8,0x54,0x54,0x54,0x3C); /* g */
			_lcr_cfpl_remplace(indCour++,0x7F,0x8,0x4,0x4,0x78);
			_lcr_cfpl_remplace(indCour++,0x0,0x44,0x7D,0x40,0x0);
			_lcr_cfpl_remplace(indCour++,0x20,0x40,0x44,0x3D,0x0);
			_lcr_cfpl_remplace(indCour++,0x7F,0x10,0x28,0x44,0);
			_lcr_cfpl_remplace(indCour++,0x0,0x41,0x7F,0x40,0);
			_lcr_cfpl_remplace(indCour++,0x78,0x4,0x1C,0x4,0x78);
			_lcr_cfpl_remplace(indCour++,0x7C,0x8,0x4,0x4,0x78);
			_lcr_cfpl_remplace(indCour++,0x38,0x44,0x44,0x44,0x38); /* o */
			_lcr_cfpl_remplace(indCour++,0x7E,0x12,0x12,0xC,0);
			_lcr_cfpl_remplace(indCour++,0x0,0xC,0x12,0x12,0x7E); /* q */
			_lcr_cfpl_remplace(indCour++,0x7C,0x8,0x4,0x4,0x8);
			_lcr_cfpl_remplace(indCour++,0x48,0x54,0x54,0x54,0x20);
			_lcr_cfpl_remplace(indCour++,0x4,0x7F,0x44,0x44,0x20);
			_lcr_cfpl_remplace(indCour++,0x3C,0x40,0x40,0x3C,0x40);
			_lcr_cfpl_remplace(indCour++,0x1C,0x20,0x40,0x20,0x1C); /* v */
			_lcr_cfpl_remplace(indCour++,0x3C,0x40,0x30,0x40,0x3C);
			_lcr_cfpl_remplace(indCour++,0x44,0x28,0x10,0x28,0x44);
			_lcr_cfpl_remplace(indCour++,0x4C,0x50,0x50,0x3C,0);
			_lcr_cfpl_remplace(indCour++,0x44,0x64,0x54,0x4C,0x44);
			_lcr_cfpl_remplace(indCour++,0x38,0x56,0x55,0x54,0x18);
			_lcr_cfpl_remplace(indCour++,0x3C,0x41,0x42,0x20,0x7C);
			_lcr_cfpl_remplace(indCour++,0x38,0x54,0x55,0x56,0x18);
			_lcr_cfpl_remplace(indCour++,0x0,1,0,1,0);
			/* INIT CARACTERES DU JEU G2 */
			indCour=0x80+0x21;
			_lcr_cfpl_remplace(indCour++,0,0,0,0,0);
			_lcr_cfpl_remplace(indCour++,0x1C,0x22,0x7F,0x22,0x22);
			_lcr_cfpl_remplace(indCour++,0,0,0,0,0);
			_lcr_cfpl_remplace(indCour++,0,0,0,0,0);
			_lcr_cfpl_remplace(indCour++,0x2B,0x2C,0x78,0x2C,0x2B);
			_lcr_cfpl_remplace(indCour++,0x14,0x77,0,0x77,0x14);
			_lcr_cfpl_remplace(indCour++,0x1C,0x36,0x55,0x55,0x41);
			_lcr_cfpl_remplace(indCour++,0xF,3,5,9,0x10);
			_lcr_cfpl_remplace(indCour++,0x10,9,5,3,0xF);
			_lcr_cfpl_remplace(indCour++,0x4,0x48,0x50,0x60,0x78);
			_lcr_cfpl_remplace(indCour++,0x78,0x60,0x50,0x48,0x4);
			_lcr_cfpl_remplace(indCour++,0x8,0x1C,0x2A,0x8,0x8);
			_lcr_cfpl_remplace(indCour++,0x4,0x2,0x7F,0x2,0x4);
			_lcr_cfpl_remplace(indCour++,0x08,0x8,0x2A,0x1C,0x8);
			_lcr_cfpl_remplace(indCour++,0x10,0x20,0x7F,0x20,0x10);
			_lcr_cfpl_remplace(indCour++,0,0,0,0,0);
			_lcr_cfpl_remplace(indCour++,0x44,0x44,0x5F,0x44,0x44);
			_lcr_cfpl_remplace(indCour++,0x8,0x1C,0x36,0x63,0x41);
			_lcr_cfpl_remplace(indCour++,0x41,0x63,0x36,0x1C,0x8);
			_lcr_cfpl_remplace(indCour++,0x49,0x24,0x12,0x49,0x24);
			_lcr_cfpl_remplace(indCour++,0x12,0x49,0x24,0x12,0x49);
			_lcr_cfpl_remplace(indCour++,0x24,0x12,0x49,0x24,0x12);
			_lcr_cfpl_remplace(indCour++,0x32,0x49,0x79,0x41,0x7E);
			_lcr_cfpl_remplace(indCour++,0x8,0x8,0x2A,0x8,0x8);
			_lcr_cfpl_remplace(indCour++,0x14,0x14,0x7F,0x14,0x14);
			_lcr_cfpl_remplace(indCour++,0x20,0,0x20,0,0x20);
			_lcr_cfpl_remplace(indCour++,0x55,0,0x55,0,0x55);
			_lcr_cfpl_remplace(indCour++,0x2,0xF,0x30,0x28,0x7C);
			_lcr_cfpl_remplace(indCour++,0xF,0x0,0x58,0x68,0x58);
			_lcr_cfpl_remplace(indCour++,0x15,0xE,0x30,0x28,0x7C);
			_lcr_cfpl_remplace(indCour++,0x55,0x2A,0x55,0x2A,0x55);
			_lcr_cfpl_remplace(indCour++,0x2A,0x55,0x2A,0x55,0x2A);
			_lcr_cfpl_remplace(indCour++,0,3,5,0,0);
			_lcr_cfpl_remplace(indCour++,0,0,5,3,0);
			indCour=0x80+0x45;
			_lcr_cfpl_remplace(indCour++,0x8,0x1C,0x36,0x63,0x41);
			indCour=0x80+0x47;
			_lcr_cfpl_remplace(indCour++,4,2,2,2,1);
			indCour=0x80+0x4C;
			_lcr_cfpl_remplace(indCour++,0x7F,0x7F,3,3,3);
			_lcr_cfpl_remplace(indCour++,3,3,3,0x7F,0x7F);
			_lcr_cfpl_remplace(indCour++,0x60,0x60,0x60,0x7F,0x7F);
			_lcr_cfpl_remplace(indCour++,0x7F,0x7F,0x60,0x60,0x60);
			_lcr_cfpl_remplace(indCour++,0x7F,0x7F,0x7F,0x7F,0x7F);
			_lcr_cfpl_remplace(indCour++,0x1C,0x3E,0x3E,0x3E,0x1C);
			_lcr_cfpl_remplace(indCour++,0x1C,0x22,0x22,0x22,0x1C);
			_lcr_cfpl_remplace(indCour++,0,8,0x1C,0x3E,0x7F);
			_lcr_cfpl_remplace(indCour++,0x7F,0x3E,0x1C,8,0);
			_lcr_cfpl_remplace(indCour++,0x49,0x49,0x49,0x49,0x49);
			_lcr_cfpl_remplace(indCour++,0x1C,0x1C,0x1C,0x1C,0x1C);
			_lcr_cfpl_remplace(indCour++,0x7,7,7,0,0);
			_lcr_cfpl_remplace(indCour++,0,0,7,7,7);
			_lcr_cfpl_remplace(indCour++,0,0,0x70,0x70,0x70);
			_lcr_cfpl_remplace(indCour++,0x70,0x70,0x70,0,0);
			_lcr_cfpl_remplace(indCour++,0x7F,0x7F,0,0,0);
			_lcr_cfpl_remplace(indCour++,0,0,0,0x7F,0x7F);
			_lcr_cfpl_remplace(indCour++,0x60,0x60,0x60,0x60,0x60);
			_lcr_cfpl_remplace(indCour++,3,3,3,3,3);
			_lcr_cfpl_remplace(indCour++,0x55,0x55,0x55,0x55,0x55);
			_lcr_cfpl_remplace(indCour++,0x2A,0x2A,0x2A,0x2A,0x2A);
			_lcr_cfpl_remplace(indCour++,0x7F,0,0x7F,0,0x7F);
			_lcr_cfpl_remplace(indCour++,0,0x7F,0,0x7F,0);
			indCour=0x80+0x69;
			_lcr_cfpl_remplace(indCour++,0x7E,9,0x7E,0x49,0x49);
			_lcr_cfpl_remplace(indCour++,0x3E,0x41,0x3E,0x49,0x49);
			indCour=0x80+0x7A;
			_lcr_cfpl_remplace(indCour++,0x38,0x44,0x38,0x54,0x58);

}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : cmd_exec_cfpl                                              /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/03/1999                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cfpl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : execution de la commande CFPL                    /
 /-------------------------------------------------------------------------DOC*/
static VOID cmd_exec_cfpl(struct cmd_cfpl *cmd_cfpl)
{
	INT j, k;
	BYTE car;
	BYTE erreur;

	erreur = FALSE;

	if (cmd_cfpl->nb_car % 6 == 0)
	{
		/* le code ascii */
		for (j = 0; j < cmd_cfpl->nb_car; j += 6)
		{
			if (((cmd_cfpl->car[j]) > 1) && ((cmd_cfpl->car[j]) <= 0xFF))
			{
				/* on stocke le nouveau caractere */
				Caractere *car_pt = NULL;
				Caractere *ancCar_pt = NULL;
				car = cmd_cfpl->car[j];
				car_pt = caractereNewFromData(car, &cmd_cfpl->car[j + 1]);
				ancCar_pt=caractereTableRechercher(car);
				if (NULL != ancCar_pt)
				{
					*ancCar_pt = *car_pt;
					caractereDelete(car_pt);
				} else
				{
					caractereAdd(car_pt);
				}
			}
		}
	}

}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cmd_cfpl                                           /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 16/03/1999                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cfpl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION :                                                        /
 /              analyse de la commande CFPL : configuration de la police de car/
 /-------------------------------------------------------------------------DOC*/
static INT lcr_cmd_cfpl(STRING buffer, INT nbcar, INT las, UINT * car_min, UINT * car_max)
{
	INT config = FALSE;
	UINT val;
	BYTE erreur;
	STRING ptr_deb;
	STRING ptr_fin;

	struct cmd_cfpl cmd_cfpl;

	/* on initialise la structure de demande de configuration */
	cmd_cfpl.flag = FALSE;
	cmd_cfpl.nb_car = 0;

	/* le dernier caractere est nul */
	buffer[nbcar] = 0;

	ptr_deb = buffer;
	ptr_fin = buffer;

	erreur = FALSE;

	while (((INT) (ptr_fin - buffer) <= nbcar) && (!erreur))
	{
		config = TRUE;
		switch (*ptr_fin)
		{

		case 0:
		case ' ':
			/* si deux separateur ne se suivent pas */
			if (ptr_deb != ptr_fin)
			{
				if (*ptr_deb == 'S')
				{
					if (++ptr_deb >= ptr_fin)
					{
						lcr_cfpl_init();
					} else
					{
						erreur = TRUE;
					}
				} else if (*ptr_deb == 'Z')
				{
					if (++ptr_deb >= ptr_fin)
					{
						lcr_cfpl_init();
					} else
						erreur = TRUE;
				} else if (dv1_scmp(ptr_deb, "PL=", 0))
				{
					cmd_cfpl.flag = TRUE;
					cmd_cfpl.nb_car = 0;
					*car_max = 0;
					*car_min = 0;
					ptr_deb += 3;
					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;
					while (ptr_deb < ptr_fin)
					{
						/* on recherche la liste des parametres */
						if (dv1_str_atoh(&ptr_deb, &val))
						{
							/* le numero d'entree/sortie */
							cmd_cfpl.car[cmd_cfpl.nb_car++] = (BYTE) val;
						} else
							erreur = TRUE;

						if (cmd_cfpl.nb_car > 50)
							erreur = TRUE;
					}

					/* si il y a une erreur */
					if (erreur)
						x01_cptr.erreur = CPTRD_SYNTAXE;
				} else if (dv1_scmp(ptr_deb, "CAR=", 0))
				{
					ptr_deb += 4;
					/* a priori il n'y a pas d'erreur */
					erreur = FALSE;
					while (ptr_deb < ptr_fin)
					{
						/* on recherche la liste des parametres */
						if (dv1_str_atoh(&ptr_deb, &val))
						{
							/* le numero d'entree/sortie */
							if (val < ' ')
								erreur = TRUE;
							*car_min = val - ' ';
						} else
							erreur = TRUE;

						/* on recherche la liste des parametres */
						if (dv1_str_atoh(&ptr_deb, &val))
						{
							/* le numero d'entree/sortie */
							if (val < ' ')
								erreur = TRUE;
							*car_max = val - ' ';
							if (*car_max > MAX_CAR_POLICE)
								erreur = TRUE;
						} else
							erreur = TRUE;
					}

					/* si il y a une erreur */
					if (erreur)
						x01_cptr.erreur = CPTRD_SYNTAXE;
				}
			}
			/* on passe au parametre suivant */
			ptr_fin++;

			/* on supprime tous les blancs */

			while (*ptr_fin == ' ')
				ptr_fin++;

			/* sur le parametre suivant */
			ptr_deb = ptr_fin;
			break;

		default:
			ptr_fin++;
			break;
		}
	}
	if (erreur)
	{
		x01_cptr.erreur = CPTRD_SYNTAXE;
	}

	/* si on a deja une commande */
	if ((x01_cptr.erreur == CPTRD_OK) && (cmd_cfpl.flag))
	{
		/* on traite la commande */
		cmd_exec_cfpl(&cmd_cfpl);
	}

	if (erreur)
	{
		config = FALSE;
		x01_cptr.erreur = CPTRD_SYNTAXE;

		/* on restaure la config */
		tac_conf_cfg_lec_conf();

	}

	/* on signale que la config a change */
	x01_cptr.config = TEMPO_CONFIG;
	return config;

}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : lcr_cfpl                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 03/12/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_cfpl.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : configuration de la police de caractere                /
 /-------------------------------------------------------------------------DOC*/

INT lcr_cfpl(INT las, INT mode, INT lg_mess, STRING buffer, INT position, T_usr_ztf * pt_mess, INT flg_fin, INT * bloc)
{
	INT i, j, k, l, m, n; /* variable de boucle lecture du buffer                  */
	UINT car_min;
	UINT car_max;
	INT config = FALSE;

	/* on se place apres le nom de la commande */
	i = position + 4;
	car_min = 0;
	car_max = MAX_CAR_POLICE;

	x01_cptr.erreur = CPTRD_OK;

	/* si on est pas en fin de message, on continue l'analyse */
	if (i < lg_mess)
	{
		/* on traite la commande */
		config = lcr_cmd_cfpl(&buffer[i], lg_mess - i, las, &car_min, &car_max);
	}

	if (car_max < car_min)
	{
		x01_cptr.erreur = CPTRD_SYNTAXE;
	} else if (x01_cptr.erreur == CPTRD_OK)
	{
		/* on traite l'etat des panneaux */
		/* on initialise le numero de bloc de reponse */
		j = dv1_scpy(buffer, "CFPL\n\r", 0);
		for (k = car_min; k < car_max; k += 8)
		{
			/* une ligne pour donner le code ascii */
			for (l = 0; l < 8; l++)
			{
				j += sprintf(&buffer[j], "  %03d    ", (k + l));
			}

			j += dv1_scpy(&buffer[j], "\n\r", 0);

			for (m = 0; m < 7; m++)
			{
				for (l = 0; l < 8; l++)
				{
					Caractere *car_pt = caractereTableRechercher(k + l);
					if(NULL!=car_pt)
					{
					for (n = 0; n < 5; n++)
					{
						int bit = (6-m) * 5 + n;
						if ((car_pt->datas[bit / 8]) & (0x1 << (7-(bit % 8))))
							buffer[j++] = '#';
						else
							buffer[j++] = ' ';
					}
					}
					else
					{
					for (n = 0; n < 5; n++)
					{
							buffer[j++] = ' ';
					}
					}
					j += dv1_scpy(&buffer[j], " || ", 0);

					if (j >= 160)
					{
						tedi_send_bloc(las, mode, buffer, j, *bloc, FALSE, pt_mess);

						if (++*bloc >= 10)
							*bloc = 0;
						j = 0;
					}
				}
				j += dv1_scpy(&buffer[j], "\n\r", 0);
			}
		}
		if (TRUE == flg_fin)
		{
			buffer[j++] = '\n';
			buffer[j++] = '\r';
		}
		tedi_send_bloc(las, mode, buffer, j, *bloc, (TRUE == flg_fin ? FALSE : TRUE), pt_mess);
	}
	/* sinon, on signale l'erreur */
	else
	{
		tedi_erreur(las, mode);
	}
	return config;
}
