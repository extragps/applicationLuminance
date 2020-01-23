/************************************************************
   
"@(#) All rigts reserved (c) 2000                                   "
"@(#) ALSTOM (Paris, France)                                         "
"@(#) This computer program may not be used, copied, distributed,    "
"@(#) corrected, modified, transmitted or assigned without ALSTOM's  "
"@(#) prior written authorization                     
 
  Projet          : SEGMENT FDM R1
  Nom du fichier  : configuration des options MCS
  Description     : 
                   
 
  Auteur:			ROBIN S   CCD  ALSTOM Clamart

  Date de creation: 	 08.01.2000 
************************************************************************/
/***********************************************************************/
/*                                                                     */
/*      S O F T W A R E  MCS   C O N F I G U R A T I O N                  */
/*                                                                     */
/***********************************************************************/
#ifndef __mcs_opt_h
#define __mcs_opt_h

#define _MCS_OPTION_YES 1
#define _MCS_OPTION_NO  0


/***********************************************************************/

#define  MCS_WITH_CONNECTED_SERVICES   	_MCS_OPTION_YES

#define  MCS_WITH_COUNTERS              _MCS_OPTION_NO

/***********************************************************************/
/* option car dans ce cas Fdm est dans le Kernel */
#define _STACK_WITH_WindowsNT		   _MCS_OPTION_NO

#endif
