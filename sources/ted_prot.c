/*
 * ted_prot.c
 * ===================
 *
 *  Created on: 23 juin 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: TED_PROT.c,v $
 * Revision 1.2  2018/06/04 08:38:37  xg
 * Passage en version V122
 *
 * Revision 1.4  2008/10/13 08:18:16  xag
 * Archivage apres passage a tours.
 *
 * Revision 1.3  2008/09/22 07:53:42  xag
 * Archivage apres travaux à Tours.
 *
 * Revision 1.2  2008/07/04 17:05:07  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 * Revision 1.1  2008/07/02 15:06:05  xag
 * NEW - bug 214: Suppression des warnings
 * http://10.29.8.8/Bugzilla/show_bug.cgi?id=214
 *
 */

/* *****************************
 * LISTE DES INCLUDES
 * ***************************** */

#include <string.h>
#include "standard.h"
#include "define.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "x01_str.h"
#include "x01_var.h"
#include "vct_str.h"
#include "x01_vcth.h"
#include "sir_dv1h.h"
#include "ypc_las.h"
#include "ted_prot.h"
#include "extLib.h"
#include "mon_debug.h"
#include "stAlLib.h" /* Le fichier contient aussi les prototypes du module trt alt */
#include "etaSyst.h"
#include "io/iolib.h"

/* *****************************
 * PROTOTYPES DES FONCTIONS LOCALES
 * ***************************** */

static VOID _tedi_fin_reception(INT, INT);
static VOID _tedi_repeat(INT);
static VOID _tedi_abort(INT);

#define DEBUG 1

#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi_repeat                                             /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 17/12/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_esc.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : tedi_repeat                                         /
 /-------------------------------------------------------------------------DOC*/

static VOID _tedi_repeat(INT las)
{
	struct xdg_las *xdg = &xdg_las[las];
	struct xdg_cf_las *xdg_cf = &xdg_cf_las[las];
	/* on emet le message */
	if (xdg_cf->attente_em)
	{
		/* il faut peut etre attendre un peut */
		xdg->attente_em = xdg_cf->attente_em;
		/* on arme la tempo */
		xdg->time_out = TEMPO_MESS + xdg_cf->attente_em;
	} else
	{
		/* on arme la tempo */
		xdg->time_out = TEMPO_MESS;
		switch (mnemuart(las))
		{
		case 1:
			tedi_fin_emission(las, CR_OK);
			break;
		case -1:
			tedi_fin_emission(las, CR_NOK);
			break;
		default:
			break;
		} /* endswitch(mnemuart                           */
	} /* endif (xdg_cf->attente_em)           */
	/* --------------------------------
	 * FIN DE tedi_repeat
	 * --------------------------------     */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi_fin_emission                                      /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 27/05/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_esc.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement de la fin de l'emission                     /
 /-------------------------------------------------------------------------DOC*/
VOID tedi_fin_emission(INT las, INT compte_rendu)
{
	struct usr_ztf *pt_mes = xdg_las[las].pt_mes;
	/* si il y a un message... */
	if (pt_mes != NULL)
	{
		pt_mes->mode = xdg_las[las].mode;
		pt_mes->compte_rendu = compte_rendu;
		/* on retourne le message a son
		 * demandeur */
		monMessSend(pt_mes->entete.MqId, &pt_mes, sizeof(pt_mes));
	}
	/* on regarde si il y a un message
	 * en attente d'emission */
	if (compte_rendu == CR_NOK)
	{
		char Buffer[MSG_SIZE];
		monMessRec(x01_ancrage_TEDI_esclave_em[las], FALSE, (char *) Buffer);
		memcpy(&pt_mes, Buffer, sizeof(struct usr_ztf *));
		if (pt_mes != NULL)
		{
			pt_mes->compte_rendu = compte_rendu;
			/* on retourne le message a son
			 * demandeur */
			monMessSend(pt_mes->entete.MqId, &pt_mes, sizeof(pt_mes));
		}
	}
	/* on positionne la structure de
	 * controle */
	xdg_las[las].phase = 0;
	xdg_las[las].pt_mes = NULL;
	xdg_las[las].time_out = 0;
	/* ----------------------------------
	 * FIN DE tedi_fin_emission
	 * --------------------------------     */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : fin                                                 /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 27/05/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_esc.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement de la fin d'une reception                   /
 /-------------------------------------------------------------------------DOC*/

static VOID _tedi_fin_reception(INT las, INT compte_rendu)
{
	UINT8 i;
	/* c'est une interruption */
	vct_IT[las] = TRUE;
	/* on termine la commande VT */
	for (i = 0; i < NBPORT; i++)
	{
		if (xdg_vt[i].port == las)
		{
			xdg_vt[i].port = N_AFF;
			xdg_vt[i].tempo = 0;
		}
	}
	xdg_vt[las].pt_in = 0;
	xdg_vt[las].pt_out = 0;
	/* on retourne le buffer */
	tedi_fin_emission(las, compte_rendu);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi_abort                                                 /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 27/05/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_esc.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement de la fin d'une reception                   /
 /-------------------------------------------------------------------------DOC*/

static VOID _tedi_abort(int las)
{
	/* c'est une interruption */
	vct_IT[las] = TRUE;
	/* on retourne le buffer */
	tedi_fin_emission(las, CR_NOK);
	/* on libere la liaison en emission */
	xdg_las[las].flag_fin_em = 2;
	/* on signale le defaut */
	if (las == 0)
	{
		x01_status3.er1 = 20 + x01_status3.er1 % 10;
		trt_alt_tester_alerte();
	} else if (las == 1)
	{
		x01_status3.er2 = 20 + x01_status3.er2 % 10;
		trt_alt_tester_alerte();
	}

}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi_em                                                /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 17/12/1996                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_esc.c                                           /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : emission tedi                                          /
 /-------------------------------------------------------------------------DOC*/
VOID tedi_em(int las, T_usr_ztf *pt_mes, int abt)
{
	STRING d;
	INT i;
	UINT8 bcc;
	INT nbCar = 0;
	int itmp;
	T_xdg_las *las_pt = &xdg_las[las];
	T_xdg_cf_las *cfg_pt = &xdg_cf_las[las];

	d = las_pt->buff_em;
	if (pt_mes->mode != MODE_TERMINAL)
	{
		/* si il y a des prefixes */
		for (i = 0; i < cfg_pt->prefixe; i++)
		{
			*d++ = DEL;
			nbCar++;
		}
	}

	if ((pt_mes->nbcar_max < 5) || (pt_mes->mode == MODE_TERMINAL))
	{
		memcpy(d, pt_mes->pt_buff, pt_mes->nbcar_max);
		nbCar += pt_mes->nbcar_max;
		d += nbCar;
	} else
	{
		/* on copie l'adresse du message */
		itmp = dv1_scpy(d, pt_mes->pt_buff, 5);
		d += itmp;
		nbCar += itmp;

//		/* si il y a un bloc apres le numero de bloc */
//		if (vct_hmvl.blanc)
//		{
//			*d++ = ' ';
//			nbCar++;
//		}
//
		/* on copie le reste du buffer */
		itmp = dv1_scpy(d, &pt_mes->pt_buff[5], pt_mes->nbcar_max - 5);
		d += itmp;
		nbCar += itmp;
	}
	/* on calcul le bcc si on est en mode
	 * protege */
	if ((pt_mes->mode == MODE_PROTEGE) && (pt_mes->nbcar_max > 5))
	{
		bcc = tedi_bcc(&las_pt->buff_em[cfg_pt->prefixe], pt_mes->nbcar_max/* + vct_hmvl.blanc*/);
		*d++ = bcc & 0x7F;
		nbCar++;
	} /* endif ((pt_mes->mode == MODE_        */
	/* si il y a des suffixes */
	if (pt_mes->mode != MODE_TERMINAL)
	{
		for (i = 0; i < cfg_pt->suffixe; i++)
		{
			*d++ = DEL;
			nbCar++;
		}
	}

	las_pt->nb_car_em = nbCar;
	las_pt->phase = 0;
	las_pt->mode = pt_mes->mode;
	las_pt->pt_mes = pt_mes;
	las_pt->flag_fin_em = 0;
	las_pt->liaison = MqLasDial[las];
	/* EMISSION DU MESSAGE                          */
	_tedi_repeat(las);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi                                                   /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 27/05/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_esc.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : traitement du protocole TEDI                           /
 /-------------------------------------------------------------------------DOC*/
VOID tedi_prot_reception(INT las, INT car)
{
	struct usr_ztf *pt_mes = xdg_las[las].pt_mes;
	switch (xdg_las[las].phase)
	{
	case 0: /* DDM */
		switch (car)
		{
		case ENQ:
			if (pt_mes == NULL)
			{
				char Buffer[MSG_SIZE];
				monMessRec(x01_ancrage_TEDI_esclave_rec[las], FALSE, (char *) Buffer);
				memcpy(&pt_mes, Buffer, sizeof(struct usr_ztf *));
				if (pt_mes != NULL)
				{
					xdg_las[las].pt_mes = pt_mes;
				}
			}
			if (pt_mes != NULL)
			{
				xdg_las[las].phase = 1;
				pt_mes->nbcar_rec = 0;
				pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
				xdg_las[las].mode = MODE_PROTEGE;
			} else
			{
				_tedi_abort(las);
			}
			break;
		case '-':
			if ((pt_mes != NULL) && (pt_mes->mode == MODE_TERMINAL))
			{
				/* le nombre de caractere */
				if (pt_mes->nbcar_rec < MAX_BUF_RE)
				{
					/* on stocke le caractere */
					pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
				}
			} else
			{
				if (pt_mes == NULL)
				{
					char Buffer[MSG_SIZE];
					if (monMessRec(x01_ancrage_TEDI_esclave_rec[las], FALSE, (char *) Buffer) == OK)
					{
						memcpy(&pt_mes, Buffer, sizeof(struct usr_ztf *));
					}
				}

				if (pt_mes != NULL)
				{
					xdg_las[las].mode = MODE_TEST;
					xdg_las[las].pt_mes = pt_mes;
					xdg_las[las].phase = 3;
					pt_mes->nbcar_rec = 0;
					pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
				} else
					_tedi_abort(las);
			}
			break;
		case ACK:
		case NACK:
			if ((xdg_las[las].mode == MODE_PROTEGE) && (xdg_las[las].nb_car_em))
			{
				xdg_las[las].phase = 4;
				xdg_las[las].acq = car;
			}
			break;
		case '!':
		case '?':
			if ((xdg_las[las].mode == MODE_TEST) && (xdg_las[las].nb_car_em))
			{
				xdg_las[las].phase = 4;
				xdg_las[las].acq = car;
			} else
			{
				if (pt_mes != NULL)
				{
					if (pt_mes->nbcar_rec < MAX_BUF_RE)
					{
						pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
					}
				}
			}
			break;
		case BS:
			switch (xdg_las[las].mode)
			{
			case MODE_TERMINAL:
				if (pt_mes != NULL)
				{
					xdg_las[las].mode = MODE_TERMINAL;
					if (pt_mes->nbcar_rec)
						pt_mes->nbcar_rec--;
				}
				break;
			}
			break;
		case CR:
			if (pt_mes == NULL)
			{
				char Buffer[MSG_SIZE];
				monMessRec(x01_ancrage_TEDI_esclave_rec[las], FALSE, (char *) Buffer);
				memcpy(&pt_mes, Buffer, sizeof(struct usr_ztf *));
				xdg_las[las].mode = MODE_TERMINAL;
				xdg_las[las].pt_mes = pt_mes;
			}
			if (pt_mes != NULL)
			{
				xdg_las[las].mode = MODE_TERMINAL;
				xdg_las[las].pt_mes = pt_mes;
				if (pt_mes->nbcar_rec < MAX_BUF_RE)
				{
					pt_mes->pt_buff[pt_mes->nbcar_rec] = car;
					pt_mes->nbcar_rec++;
				}
				_tedi_fin_reception(las, CR_OK);
			} else
				_tedi_abort(las);
			break;
		case XON:
			switch (xdg_las[las].mode)
			{
			case MODE_TERMINAL:
			case MODE_AUCUN:
				xdg_las[las].mode = MODE_TERMINAL;
				rqtemp[XF_0 + las] = TP_HS;
				break;
			}
			break;

		case XOF:
			switch (xdg_las[las].mode)
			{
			case MODE_TERMINAL:
			case MODE_AUCUN:
				xdg_las[las].mode = MODE_TERMINAL;
				rqtemp[XF_0 + las] = TEMP_XOFF;
				break;
			}
			break;
		default:
			if (pt_mes == NULL)
			{
				char Buffer[MSG_SIZE];
				monMessRec(x01_ancrage_TEDI_esclave_rec[las], FALSE, (char *) Buffer);
				memcpy(&pt_mes, Buffer, sizeof(struct usr_ztf *));
				xdg_las[las].mode = MODE_TERMINAL;
				xdg_las[las].pt_mes = pt_mes;
			}
			if (pt_mes != NULL)
			{
				if (pt_mes->nbcar_rec < MAX_BUF_RE)
				{
					pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
				}
			}
			break;
		}
		break;
	case 1: /* protege */
		switch (car)
		{
		case ENQ:
			pt_mes->nbcar_rec = 0;
			pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
			xdg_las[las].mode = MODE_PROTEGE;
			xdg_las[las].time_out = TEMPO_MESS;
			break;
		case 0:
			if (pt_mes->nbcar_rec < MAX_BUF_RE)
			{
				pt_mes->pt_buff[pt_mes->nbcar_rec++] = 0x55;
			}
			break;
		case ETX:
			xdg_las[las].phase = 2;
		default:
			if (pt_mes->nbcar_rec < MAX_BUF_RE)
			{
				pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
			}
			xdg_las[las].time_out = TEMPO_MESS;
			break;
		}
		break;
	case 2: /* bcc */
		if (pt_mes->nbcar_rec < MAX_BUF_RE)
		{
			pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
		}
		_tedi_fin_reception(las, CR_OK);
		break;
	case 3: /* test */
		switch (car)
		{
		case ENQ:
			pt_mes->nbcar_rec = 0;
			pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
			xdg_las[las].mode = MODE_PROTEGE;
			break;
		case '-':
			pt_mes->nbcar_rec = 0;
			pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
			xdg_las[las].mode = MODE_TEST;
			break;
		case CR:
			if (pt_mes->nbcar_rec < MAX_BUF_RE)
			{
				pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
			}
			_tedi_fin_reception(las, CR_OK);
			break;
		default:
			if (pt_mes->nbcar_rec < MAX_BUF_RE)
			{
				pt_mes->pt_buff[pt_mes->nbcar_rec++] = car;
			} else
				_tedi_abort(las);
			break;
		}
		break;
	case 4: /* no bloc */
		xdg_las[las].phase = 0;
		switch (xdg_las[las].mode)
		{
		case MODE_PROTEGE:
			if ((xdg_las[las].acq == NACK) && (car == xdg_las[las].buff_em[4]))
			{
				_tedi_repeat(las);
			} else if ((xdg_las[las].acq == ACK) && (car == xdg_las[las].buff_em[4]))
			{
				xdg_las[las].flag_fin_em = 2;
			} else
				_tedi_abort(las);
			break;

		case MODE_TEST:
			if ((xdg_las[las].acq == '?') && (car == xdg_las[las].buff_em[4]))
			{
				_tedi_repeat(las);
			} else if ((xdg_las[las].acq == '!') && (car == xdg_las[las].buff_em[4]))
			{
				xdg_las[las].flag_fin_em = 2;
			} else
				_tedi_abort(las);
			break;
		}
		break;
	}
}

VOID tedi_prot_emission(INT las)
{
	xdg_las[las].time_out = 0;
	if (xdg_las[las].nb_car_em > 0)
	{
		/* suivant le mode .. */
		switch (xdg_las[las].mode)
		{
		case MODE_PROTEGE:
			if ((xdg_las[las].nb_car_em > 2) && (xdg_las[las].buff_em[xdg_las[las].nb_car_em - 2] == ETB))
			{
				/* l'emission n'est pas terminee */
				xdg_las[las].flag_fin_em = 0;
			}
			break;
		case MODE_TEST:
			if ((xdg_las[las].nb_car_em > 2) && (xdg_las[las].buff_em[xdg_las[las].nb_car_em - 1] == '+'))
			{
				xdg_las[las].flag_fin_em = 0;
			}
			break;
		case MODE_TERMINAL:
			break;
		}
	}
}

VOID tedi_prot_time(INT las)
{
	_tedi_abort(las);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi_ctrl                                               /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_ge1.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : permet l'envoi d'un acquitement vers une liaison serie /
 /-------------------------------------------------------------------------DOC*/

VOID tedi_ctrl(INT las, INT mode, INT ctrl)
{
	UINT8 buffer[100]; /* contenu du message d'acquitement     */
	INT lg_mess; /* longueur du message de controle  */
	struct usr_ztf mess, *pt_mess; /* structure des messages de trans-
	 * fert      */
	/* une reponse est a faire que si
	 * l'adresse ne comportait pas de
	 * joker */
	if ((x01_flag_jocker[las] == 0) && (las >= 0))
	{
		buffer[1] = '0';
		switch (mode)
		{
		case MODE_PROTEGE:
			if (ctrl)
				buffer[0] = ACK;
			else
				buffer[0] = NACK;
			lg_mess = 2;
			break;

		case MODE_TEST:
			if (ctrl)
				buffer[0] = '!';
			else
				buffer[0] = '?';
			lg_mess = 1;
			break;

		case MODE_TERMINAL:
		default:
			/* selon le type d'erreur */
			lg_mess = 1;
			if (ctrl)
				buffer[0] = '!';
			else
				buffer[0] = '?';
			break;
		}
		/* on indique le mode */
		mess.mode = mode;
		mess.entete.MqId = ancrageTedi_ctrl;
		mess.nbcar_max = lg_mess;
		mess.pt_buff = (STRING) buffer;
		pt_mess = &mess;
		monMessSend(x01_ancrage_TEDI_esclave_em[las], &pt_mess, sizeof(pt_mess));
		/* on attend la reponse sur l'ancrage */
		if(0!=monMessRec(ancrageTedi_ctrl, 5000, NULL))
		{
			vct_IT[las]=TRUE;
		}
	} /* endif ((x01_flag_jocker[     */
	/* ---------------------------------
	 * FIN DE tedi_ctlr
	 * --------------------------------     */
}

/* --------------------------------
 * tedi_ctrl_ext
 * =============
 * Commande de controle avec extension.
 * Il s'agit ici de gerer l'extension
 * du code d'erreur.
 * Les valeurs de ctrl peuvent etre
 * maintenant multiples :
 * -	0 OK
 * -	1 Commande non reconnue,
 * -	2 Parametre commande inconnu,
 * -	3 Parrametre errone,
 * -	4 Commande correcte qui ne peut
 * 		etre executee,
 * -	5 Mot de passe errone.
 * --------------------------------	*/
VOID tedi_ctrl_ext(INT las, INT mode, INT ctrl)
{
	INT lg_mess = 0; /* longueur du message de controle  */
	struct usr_ztf mess, *pt_mess; /* structure des messages de trans-
	 * fert      */
	/* une reponse est a faire que si
	 * l'adresse ne comportait pas de
	 * joker */
	if ((x01_flag_jocker[las] == 0) && (las >= 0))
	{
		mess.buffer[1] = '0';
		switch (mode)
		{
		case MODE_PROTEGE:
			if (!ctrl)
			{
				mess.buffer[0] = ACK;
				lg_mess = 2;
			} else
			{
				if (extGet(EXT_ECE))
				{
					mess.buffer[0] = STX;
					/* l'adresse */
					dv1_scpy(&mess.buffer[1], x01_status1.adr, 3);
					/* numero du bloc courant */
					mess.buffer[4] = '0';
					sprintf(&mess.buffer[5], "%%%d", ctrl);
					mess.buffer[7] = ETX;
					lg_mess = 8;
				} else
				{
					if (1 != ctrl)
					{
						mess.buffer[0] = NACK;
						lg_mess = 2;
					} else
					{
						lg_mess = 0;
					}
				}
			}
			break;
		case MODE_TEST:
		case MODE_TERMINAL:
		default:
			if (!ctrl)
			{
				mess.buffer[lg_mess++] = '!';
			} else
			{
				if (extGet(EXT_ECE))
				{
					lg_mess = sprintf(mess.buffer, "%%%d", ctrl);
				} else
				{
					if (1 != ctrl)
					{
						mess.buffer[lg_mess++] = '?';
					} else
					{
						/* Ajout par XG pour tester un
						 * autre terminateur */
						mess.buffer[lg_mess++] = '\\';
					}
				}
			}
			break;
		}
		/* on indique le mode */
		if (lg_mess)
		{
			mess.mode = mode;
			mess.entete.MqId = ancrageTedi_ctrl;
			mess.nbcar_max = lg_mess;
			mess.pt_buff = (STRING) mess.buffer;
			pt_mess = &mess;
			monMessSend(x01_ancrage_TEDI_esclave_em[las], &pt_mess, sizeof(pt_mess));
			/* on attend la reponse sur l'ancrage */
			if(0!=monMessRec(ancrageTedi_ctrl, 5000, NULL))
			{
				vct_IT[las]=TRUE;
			}
		}
	} /* endif ((x01_flag_jocker[     */
	/* ---------------------------------
	 * FIN DE tedi_ctlr
	 * --------------------------------     */
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : ge1_send_bloc                                          /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/01/1991                                               /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : lcr_ge1.c                                            /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : permet l'envoi d'un bloc de donnees vers un las        /
 /-------------------------------------------------------------------------DOC*/

/* --------------------------------
 * tedi_send_abt
 * ==============
 * Emission sur la liaison abonnement
 * des informations relatives e un
 * abonnement.
 * --------------------------------	*/

VOID tedi_send_abt(INT las, INT mode, STRING pt_buff, INT nbcar, INT no_bloc, INT der_blk, struct usr_ztf *pt_mess)
{
#ifdef RAD
	tedi_send_ancrage(las, mode, pt_buff, nbcar, no_bloc, der_blk, pt_mess, x01_ancrage_TEDI_abt);
#endif
}

INT tedi_test_bloc(INT las, INT mode, STRING buffer, INT* nbcar, INT *bloc, struct usr_ztf *pt_mess)
{
	INT fini = FALSE;
	if (*nbcar >= MAXLCR)
	{

		tedi_send_bloc(las, mode, buffer, *nbcar, *bloc, FALSE, pt_mess);
		if (++(*bloc) >= 10)
			*bloc = 0;
		*nbcar = 0;

		if (vct_IT[las])
			fini = TRUE;
	}
	return fini;
}
/* --------------------------------
 * tedi_send_bloc
 * ==============
 * Emission sur la liaison normale
 * d'une reponse e une commande
 * --------------------------------	*/

VOID tedi_send_bloc(INT las, INT mode, STRING pt_buff, INT nbcar, INT no_bloc, INT der_blk, struct usr_ztf *pt_mess)
{
	tedi_send_ancrage(las, mode, pt_buff, nbcar, no_bloc, der_blk, pt_mess, x01_ancrage_TEDI_esclave_em);
}

/* --------------------------------
 * tedi_send_ancrage
 * ==============
 * Emission sur la liaison normale
 * sur un ancrage.
 * --------------------------------	*/

VOID tedi_send_ancrage(INT las, INT mode, STRING pt_buff, INT nbcar, INT no_bloc, INT der_blk, struct usr_ztf *pt_mess,
		mqd_t * ancrage)
{
	/* une reponse est a faire que si
	 * l'adresse ne comportait pas de
	 * joker et si on n'est pas en
	 * interne */
	if ((x01_flag_jocker[las] == 0) && (las >= 0))
	{

		/* suivant le mode de transmission
		 * voulu */
		switch (mode)
		{
		case MODE_PROTEGE:
			pt_buff -= 5;
			pt_buff[0] = STX;
			dv1_scpy(&pt_buff[1], x01_status1.adr, 3);
			/* numero du bloc courant */
			pt_buff[4] = no_bloc + '0';
			/* si on est au dernier bloc de la
			 * transmission */
			if (der_blk)
			{
				pt_buff[5 + nbcar] = ETX;
			} else
			{
				pt_buff[5 + nbcar] = ETB;
			}
			/* on indique la longueur du buffer */
			nbcar += 6;
			break;
		case MODE_TEST:
			pt_buff -= 5;
			pt_buff[0] = '-';
			/* l'adresse */
			dv1_scpy(&pt_buff[1], x01_status1.adr, 3);
			/* numero du bloc courant */
			pt_buff[4] = no_bloc + '0';
			/* si on est au dernier bloc de la
			 * transmission */
			if (der_blk)
			{
				pt_buff[5 + nbcar] = '!';
			} else
			{
				pt_buff[5 + nbcar] = '+';
			}
			/* on indique la longueur du buffer */
			nbcar += 6;

			break;
		case MODE_TERMINAL:
			/* si on est au dernier bloc de la
			 * transmission , on ajoute ! */
			if (der_blk)
			{
				pt_buff[nbcar++] = '!';
			}
			break;
		}

		/* on indique la longueur du buffer */
		/* et l'adresse du buffer           */
		pt_mess->nbcar_max = nbcar;
		pt_mess->pt_buff = pt_buff;
		/* on envoie le message vers la
		 * transmission */
		pt_mess->pt_buff[nbcar]=0;
		monMessSend(ancrage[las], &pt_mess, sizeof(pt_mess));
		/* on attend la reponse sur l'ancrage */
		if(0!=monMessRec(pt_mess->entete.MqId,5000, NULL))
		{
			vct_IT[las]=TRUE;
		}
	}
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi_bcc                                                /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/12/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_prot                                             /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : calcul le BCC                                          /
 /-------------------------------------------------------------------------DOC*/

INT tedi_bcc(STRING pt_buff, INT nbcar)
{
	INT i; /* indice de lecture du message                       */
	UINT8 bcc = 0; /* valeur recevant le bcc calcule                     */
	for (i = 0; i < nbcar; i++)
	{
		bcc += pt_buff[i];
	}
	return (bcc & 0x7F);
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi_securite                                                /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/12/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_prot                                             /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : positionnement                                         /
 /-------------------------------------------------------------------------DOC*/

INT tedi_securite(VOID)
{
INT retour=FALSE;

	if(ioIsUnlocked())
	{
		retour=TRUE;
	}
	return retour;
}

/*DOC-------------------------------------------------------------------------/
 / S.I.A.T.    |     FICHE DE FONCTION                                         /
 /-----------------------------------------------------------------------------/
 / NOM DE LA FONCTION : tedi_erreur                                                /
 /-----------------------------------------------------------------------------/
 / DATE DE CREATION : 14/12/96                                                 /
 /-----------------------------------------------------------------------------/
 / UNITE DE COMPILATION : tedi_prot                                             /
 /-----------------------------------------------------------------------------/
 / BUT DE LA FONCTION : positionnement                                         /
 /-------------------------------------------------------------------------DOC*/

VOID tedi_erreur(INT las, INT mode)
{
	/* si on n'est pas en interne */
	if (las >= 0)
	{
		/* suivant la valeur de flag_err */
		switch (x01_cptr.erreur)
		{
		case CPTRD_SYNTAXE: /* erreur de syntaxe */
			/* on signale cette erreur */
			switch (las)
			{
			case 0:
				x01_status3.er1 = E_SYNTAXE - '0';
				trt_alt_tester_alerte();
				break;
			case 1:
				x01_status3.er2 = E_SYNTAXE - '0';
				trt_alt_tester_alerte();
				break;
			case 2:
				x01_status3.er3 = E_SYNTAXE - '0';
				trt_alt_tester_alerte();
				break;
			}
			tedi_ctrl_ext(las, mode, 2);
			break;

		case CPTRD_PARAM: /* commande non recevable */
			tedi_ctrl_ext(las, mode, 2);
			break;
		case CPTRD_PROTOCOLE: /* non autorise en ecriture */
			tedi_ctrl_ext(las, mode, 5);
			break;
		case CPTRD_INTERDIT: /* Commande valide mais non autorisee actuellement */
			tedi_ctrl_ext(las, mode, 4);
			break;
		case CPTRD_VALEUR_PARAM: /* Parametre incorrect */
			tedi_ctrl_ext(las, mode, 3);
			break;
		}
	}
}
