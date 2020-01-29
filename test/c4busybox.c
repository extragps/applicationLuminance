/**********************************************************************************************************************
 * SIAT - Copyright SIAT
 * --------------------------------------------------------------------------------------------------------------------
 * fichier     : c4busybox.c
 * description :
 * prefixe     :
 * --------------------------------------------------------------------------------------------------------------------
 * Auteur      : sylvestre
 * Date        : 3 févr. 2017
 *********************************************************************************************************************/
#include <stdio.h>
#include <string.h>

#include <kcommon.h>
#include <klog.h>

#include <su3/su3_busybox.h>
#include "sysupdater/c4syscopy.h"
#include "startup/c4startup.h"
#include "watchdog/c4watchdog.h"
#include <swt.h>

/**--------------------------------------------------------------------------------------------------------------------
 *
 */
static const char * _getCommand( const char * arg0_pc )
{
	const char * cmd_pc;

	cmd_pc = strrchr( arg0_pc, '\\' );
	if( cmd_pc == null ) {
		cmd_pc = strrchr( arg0_pc, '/' );
		if( cmd_pc == null ) {
			cmd_pc = arg0_pc;
		} else {
			cmd_pc = cmd_pc + 1;
		}
	} else {
		cmd_pc = cmd_pc + 1;
	}

	return cmd_pc;
}

/**--------------------------------------------------------------------------------------------------------------------
 *
 */
static struct
{
	const char * cmd_pc;
	int (*app_pf)( int argc, char ** argv );
} _busyboxApps_at[] = {
//
   { "c4su3", su3_main }, //
   { "c4syscopy", c4syscopy }, //
   { "c4startup", c4startup }, //
   { "c4cmd", c4cmd }, //
   { "c4watchdog", c4watchdog }, //
   { "c4shttpd", httpd_busybox }, //
   { "copieSysteme", c4syscopy }, //
   { "cpu432Launcher", c4startup }, //
   { "cpu432Watchdog", c4watchdog }, //
   { null, null } //
};

/**--------------------------------------------------------------------------------------------------------------------
 *
 */
int main( int argc, char ** argv )
{
	int32 result_dw;
	uint32 k_udw;
	const char * cmd_pc = _getCommand( argv[0] );

	for( k_udw = 0; _busyboxApps_at[k_udw].cmd_pc != null; k_udw++ ) {
		if( kzbufEquals( cmd_pc, _busyboxApps_at[k_udw].cmd_pc ) ) {
			break;
		}
	}

	if( _busyboxApps_at[k_udw].app_pf != null ) {
		result_dw = _busyboxApps_at[k_udw].app_pf( argc, argv );
	} else {
		fprintf( stderr, "%s: invalid command\n", argv[0] );
		result_dw = -1;
	}

	return result_dw;
}
