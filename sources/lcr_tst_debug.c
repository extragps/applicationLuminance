/*
 * lcr_tst_debug
 * ===================
 * Objet	: Traitement de la commande TST DEBUG qui permet en theorie
 * 			d'activer ou de desactiver des traces sur un module.
 *
 * --------------------------------------------------------
 *  Created on: 2 juil. 2008
 *      Author: $Author: xg $
 * --------------------------------------------------------
 * $Log: lcr_tst_debug.c,v $
 * Revision 1.3  2018/06/06 08:13:58  xg
 * Le fichier pour changer d'ip est eth.cfg et pas ethRad.cfg
 *
 * Revision 1.2  2018/06/04 08:38:36  xg
 * Passage en version V122
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

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include <kcommon.h>
#include <tokenizerDirif.h>
#include "portage.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "ted_prot.h"
#include "lcr_tst.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

#define TST_DEBUG_CMD			"TST DEBUG"
#define TST_DEBUG_ON			"ON"
#define TST_DEBUG_OFF			"OFF"
#define TST_DEBUG_ACQ			"ACQ="
#define TST_DEBUG_INT			"INT="
#define TST_DEBUG_EQU			"EQU="
#define TST_DEBUG_DET_DIAL		"DET_DIAL="
#define TST_DEBUG_DET_PROT		"DET_PROT="
#define TST_DEBUG_CAC			"CAC="
#define TST_DEBUG_PERF_LCR		"PERF_LCR="
#define TST_DEBUG_PERF_FIP		"PERF_FIP="
#define TST_DEBUG_PERF_SUP		"PERF_SUP="

/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

typedef struct T_tst_debug
{
	char *param;
	int val;
} T_tst_debug;

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static T_tst_debug lDeb[] =
{
{ TST_DEBUG_DET_DIAL, MON_DEBUG_DET_DIAL },
{ TST_DEBUG_DET_PROT, MON_DEBUG_DET_PROT },
{ TST_DEBUG_ACQ, MON_DEBUG_ACK },
{ TST_DEBUG_INT, MON_DEBUG_INT },
{ TST_DEBUG_EQU, MON_DEBUG_EQU },
{ TST_DEBUG_PERF_FIP, MON_DEBUG_PERF_FIP },
{ TST_DEBUG_PERF_LCR, MON_DEBUG_PERF_LCR },
{ TST_DEBUG_PERF_SUP, MON_DEBUG_PERF_SUP },
{ NULL, -1 } };

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static bool _ethConfig_b = false;
static uint32 _ethAddr_udw = 0;
static uint32 _ethMask_udw = 0;
static uint32 _ethPass_udw = 0;

/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

static char *lcr_debug_tst_set(int module, char *ptCour, int lgCour, int *reste, int *flag_err)
{
	if (strncmp(ptCour, TST_DEBUG_ON, strlen(TST_DEBUG_ON)) == 0)
	{
		ptCour += strlen(TST_DEBUG_ON);
		lgCour -= strlen(TST_DEBUG_ON);
		monDebugSet(module, TRUE);
	} else if (strncmp(ptCour, TST_DEBUG_OFF, strlen(TST_DEBUG_OFF)) == 0)
	{
		ptCour += strlen(TST_DEBUG_OFF);
		lgCour -= strlen(TST_DEBUG_OFF);
		monDebugSet(module, FALSE);
	} else
	{
		*flag_err = 3;
	}
	*reste = lgCour;
	return ptCour;
}

/* --------------------------------
 * lcr_tst_debug
 * =============
 * Analyse de la commande "TST DEBUG".
 * Entree :
 * - numero de la liaison sur lequel
 *   se fait le transfert.
 * - mode : mode de transmission,
 * - nombre de caracteres dans le
 *   buffer,
 * - position dans le buffer,
 * - pointeur vers un message de
 *   transmission
 * --------------------------------	*/

int lcr_tst_debug(INT las, INT mode, INT lg_mess, STRING buffer, INT position, struct usr_ztf *pt_mess)
{
	char *ptCour = (char *) &buffer[position];
	int lgCour = lg_mess - position;
	int nbCar = 0;
	int bloc = 0;
	int flag_err = 0;

	ptCour += strlen(TST_DEBUG_CMD);
	lgCour -= strlen(TST_DEBUG_CMD);
	ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	while ((0 < lgCour) && (0 == flag_err))
	{
		int indice;
		indice = 0;
		while (NULL != lDeb[indice].param)
		{
			char *param = lDeb[indice].param;
			if (strncmp(ptCour, param, strlen(param)) == 0)
			{
				int val = lDeb[indice].val;
				ptCour += strlen(param);
				lgCour -= strlen(param);
				ptCour = lcr_debug_tst_set(val, ptCour, lgCour, &lgCour, &flag_err);
				break;
			} else
			{
				indice++;
			}
		}
		if (NULL == lDeb[indice].param)
		{
			flag_err = 2;
		}
		ptCour = tst_passe_blanc(ptCour, lgCour, &lgCour);
	} /* endwhile((0<lgCour)&&(0==flag_err))  */

	if (0 == flag_err)
	{
		int indice = 0;
		tst_send_bloc(las, mode, buffer, &nbCar, &bloc, FALSE, pt_mess, "TST DEBUG ");
		while (NULL != lDeb[indice].param)
		{
			tst_send_bloc(las, mode, buffer, &nbCar, &bloc, FALSE, pt_mess, " %s%s", lDeb[indice].param, ((monDebugGet(
					lDeb[indice].val) == TRUE) ? "ON" : "OFF"));
			indice++;
		}
		tst_send_bloc(las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess, "");
	} else
	{
		tedi_ctrl_ext(las, mode, flag_err);
	} /* endif(0==flag_err)                                   */
	/* ------------------------------------
	 * FIN DE tst_simu_dial
	 * ------------------------------------ */
	return 0;
}

/**--------------------------------------------------------------------------------------------------------------------
 *
 */
static bool _parseAdresse( uint32 *addr_pudw, const char * straddr_pc )
{
	bool success_b = false;
	TokenizerDirif tkz_t;
	char tok1_ac[4];
	char tok2_ac[4];
	char tok3_ac[4];
	char tok4_ac[4];
	int32 byte1_dw;
	int32 byte2_dw;
	int32 byte3_dw;
	int32 byte4_dw;

	tokenizeDirif( &tkz_t, straddr_pc, '.' );
	if( tkzDirifGetSize( &tkz_t ) == 4 && tkzDirifGetFirst( &tkz_t, tok1_ac, 4 ) && tkzDirifGetNext( &tkz_t, tok2_ac, 4 )
	    && tkzDirifGetNext( &tkz_t, tok3_ac, 4 ) && tkzDirifGetNext( &tkz_t, tok4_ac, 4 ) )
	{
		if( str2intBorned( &byte1_dw, tok1_ac, 0, 255 ) && str2intBorned( &byte2_dw, tok2_ac, 0, 255 )
		    && str2intBorned( &byte3_dw, tok3_ac, 0, 255 ) && str2intBorned( &byte4_dw, tok4_ac, 0, 255 ) )
		{
			*addr_pudw = ((uint32) byte1_dw << 24) | ((uint32) byte2_dw << 16) | ((uint32) byte3_dw << 8)
			    | ((uint32) byte4_dw);

			success_b = true;
		}
	}

	return success_b;
}


/**--------------------------------------------------------------------------------------------------------------------
 *
 */
static bool _getCurrentEthInfo( uint32 * curip_pudw, uint32 * curma_pudw, uint32 * curpa_pudw )
{
	bool success_b = false;
	KnetworkConfig ifcfg_t;

	if( ethInfo( &ifcfg_t, "eth0" ) ) {
		if( _parseAdresse( curip_pudw, ifcfg_t.addr_ac  ) && _parseAdresse( curma_pudw, ifcfg_t.mask_ac  )
		    && _parseAdresse( curpa_pudw, ifcfg_t.gatw_ac ) )
		{
			success_b = true;
		}
	}

	if( !success_b ) {
		*curip_pudw = 0;
		*curma_pudw = 0;
		*curpa_pudw = 0;
	}

	return success_b;
}
static bool _writeFile( uint32 ip_udw, uint32 mask_udw, uint32 gw_udw )
{
	bool success_b = false;
	FILE * f;

#if (defined(CPU432)||defined(RAD_IP))
	f = fopen( "/var/flash/eth.cfg", "w" );
#else
	f = fopen( "./eth.cfg", "w" );
#endif
	if( null != f ) {
		fprintf( f, "#!/bin/sh\n" );
		fprintf( f, "ifconfig eth0 %u.%u.%u.%u netmask %u.%u.%u.%u up\n", ((ip_udw >> 24) & 0xFF),
		    ((ip_udw >> 16) & 0xFF), ((ip_udw >> 8) & 0xFF), ((ip_udw) & 0xFF), ((mask_udw >> 24) & 0xFF),
		    ((mask_udw >> 16) & 0xFF), ((mask_udw >> 8) & 0xFF), ((mask_udw) & 0xFF) );
		fprintf( f, "route add -net 0.0.0.0 netmask 0.0.0.0 gw %u.%u.%u.%u\n", ((gw_udw >> 24) & 0xFF),
		    ((gw_udw >> 16) & 0xFF), ((gw_udw >> 8) & 0xFF), ((gw_udw) & 0xFF) );
		fclose( f );
#if (defined(CPU432)||defined(RAD_IP))
		system( "chmod 777 /var/flash/eth.cfg" );
#else
		system( "chmod 777 ./eth.cfg" );
#endif
		success_b = true;
	}

	return success_b;
}

static bool _ethConfigIsValid( uint32 addr_udw, uint32 mask_udw, uint32 gtw_udw )
{
	bool valid_b = false;
	uint32 nwaddr_udw = addr_udw & mask_udw;
	uint32 bcaddr_udw = addr_udw | (~mask_udw);

	if( (mask_udw != 0 && mask_udw != 0xFFFFFFFF) ) {
		if( addr_udw != nwaddr_udw && addr_udw != bcaddr_udw ) {
			if( gtw_udw == 0 ) {
				valid_b = true;
			} else if( gtw_udw != nwaddr_udw && gtw_udw != bcaddr_udw ) {
				if( (gtw_udw & mask_udw) == nwaddr_udw ) {
					valid_b = true;
				} else {
					printDebug( "TST ETH: adresse et passerelle incompatibles\n" );
				}
			} else {
				printDebug( "TST ETH: passerelle invalide\n" );
			}
		} else {
			printDebug( "TST ETH: adresse IP invalide\n" );
		}
	} else {
		printDebug( "TST ETH: masque invalide\n" );
	}

	return  valid_b;
}

int lcr_tst_eth(INT las,INT mode,INT lg_mess,STRING buffer,INT position, struct usr_ztf *pt_mess)
{
	int flag_err=0;
	bool error_b = false;
	char token_ac[32];
	bool ip_b = false;
	bool ma_b = false;
	bool pa_b = false;
	TokenizerDirif tkz2_t;
	char label_ac[4];
	char value_ac[16];
	uint32 curip_udw = 0;
	uint32 curma_udw = 0;
	uint32 curpa_udw = 0;
	uint32 newip_udw = 0;
	uint32 newma_udw = 0;
	uint32 newpa_udw = 0;
	bool ext_b = false;
	TokenizerDirif tkz_t;
	TokenizerDirif *tkz_pt=&tkz_t;
	int nbCar=0;
	int bloc=0;
	char *ptCour=(char *)&buffer[position];
		/* Suprression du dernier caractere.. */
	buffer[lg_mess]=0;
		/* Suppression de l'entete */

	tokenizeDirif( &tkz_t,ptCour,' ');
	tkzDirifGetNext( tkz_pt, token_ac, sizeof(token_ac));
	tkzDirifGetNext( tkz_pt, token_ac, sizeof(token_ac) );

	// *** Recuperation des valeurs courantes
	if( !_getCurrentEthInfo( &curip_udw, &curma_udw, &curpa_udw ) ) {
		printDebug("TST ETH: Erreur de recuperation des parametres IP\n" );
	}

	// *** Recuperation des valeurs de configuration precedentes
	if( _ethConfig_b ) {
		newip_udw = _ethAddr_udw;
		newma_udw = _ethMask_udw;
		newpa_udw = _ethPass_udw;
	} else {
		newip_udw = curip_udw;
		newma_udw = curma_udw;
		newpa_udw = curpa_udw;
	}

	// *** Lecture commande
		while( !error_b && tkzDirifHasNext( tkz_pt ) ) {
			if( tkzDirifGetNext( tkz_pt, token_ac, sizeof(token_ac) ) ) {
				if( !strcmp( token_ac, "EXT" ) && !ext_b ) {
					ext_b = true;
				} else {
					tokenizeDirif( &tkz2_t, token_ac, '=' );
					if( tkzDirifGetSize( &tkz2_t ) == 2 && tkzDirifGetFirst( &tkz2_t, label_ac, sizeof(label_ac) )
					    && tkzDirifGetNext( &tkz2_t, value_ac, sizeof(value_ac) ) )
					{
						if( !strcmp( label_ac, "IP" ) && _parseAdresse( &newip_udw, value_ac ) ) {
							ip_b = true;
						} else if( !strcmp( label_ac, "MA" ) && _parseAdresse( &newma_udw, value_ac ) ) {
							ma_b = true;
						} else if( !strcmp( label_ac, "PA" ) && _parseAdresse( &newpa_udw, value_ac ) ) {
							pa_b = true;
						} else {
							error_b = true;
							flag_err=2;
						}
					} else {
						error_b = true;
						flag_err=2;
					}
				}
			} else {
				error_b = true;
				flag_err=2;
			}
		}

	// *** Traitement commande si ecriture
	if( !error_b && (ip_b || pa_b || ma_b) ) {
		if( _ethConfigIsValid( newip_udw, newma_udw, newpa_udw ) ) {
			_ethAddr_udw = newip_udw;
			_ethMask_udw = newma_udw;
			_ethPass_udw = newpa_udw;
			_ethConfig_b = true;
			error_b = !_writeFile( newip_udw, newma_udw, newpa_udw );
		} else {
			flag_err=2;
		}
	}

// *** Creation reponse
	if( !error_b ) {
		uint32 addr_udw;
		addr_udw=curip_udw;
		tst_send_bloc(las,mode,buffer,&nbCar,&bloc,FALSE,pt_mess, "TST ETH IP=%u.%u.%u.%u",
				((addr_udw >> 24) & 0xFF), ((addr_udw >> 16) & 0xFF),((addr_udw >> 8) & 0xFF), ((addr_udw >> 0) & 0xFF) );
		if( _ethConfig_b ) {
			addr_udw=newip_udw;
			tst_send_bloc(las,mode,buffer,&nbCar,&bloc,FALSE,pt_mess, "(%u.%u.%u.%u)",
				((addr_udw >> 24) & 0xFF), ((addr_udw >> 16) & 0xFF),((addr_udw >> 8) & 0xFF), ((addr_udw >> 0) & 0xFF) );
		}

		addr_udw=curma_udw;
		tst_send_bloc(las,mode,buffer,&nbCar,&bloc,FALSE,pt_mess, " MA=%u.%u.%u.%u",
				((addr_udw >> 24) & 0xFF), ((addr_udw >> 16) & 0xFF),((addr_udw >> 8) & 0xFF), ((addr_udw >> 0) & 0xFF) );

		if( _ethConfig_b ) {
			addr_udw=newma_udw;
			tst_send_bloc(las,mode,buffer,&nbCar,&bloc,FALSE,pt_mess, "(%u.%u.%u.%u)",
				((addr_udw >> 24) & 0xFF), ((addr_udw >> 16) & 0xFF),((addr_udw >> 8) & 0xFF), ((addr_udw >> 0) & 0xFF) );
		}

		addr_udw=curpa_udw;
		tst_send_bloc(las,mode,buffer,&nbCar,&bloc,(_ethConfig_b?FALSE:TRUE),pt_mess, " MA=%u.%u.%u.%u",
				((addr_udw >> 24) & 0xFF), ((addr_udw >> 16) & 0xFF),((addr_udw >> 8) & 0xFF), ((addr_udw >> 0) & 0xFF) );

		if( _ethConfig_b ) {
			addr_udw=newpa_udw;
			tst_send_bloc(las,mode,buffer,&nbCar,&bloc,TRUE,pt_mess, "(%u.%u.%u.%u)",
				((addr_udw >> 24) & 0xFF), ((addr_udw >> 16) & 0xFF),((addr_udw >> 8) & 0xFF), ((addr_udw >> 0) & 0xFF) );
		}

	} else {
		tedi_ctrl_ext(las,mode,flag_err);
	}

	/* Pas de modification de configuration pour TST ETH */
	return 0;
}
