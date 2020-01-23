/************************************************************
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "
   
  ALSTOM  - 5 ,Av. Newton 92140 Clamart - CCD
       
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fcod_opt.h
  Description     : Definition du type de station de Fipcode V6
                                      
  
  Date de creation: Fri Mar 15 15:11:48 MET 1996
  
***********************************************************/

#ifndef __fipcode_opt
#define __fipcode_opt

#include "user_opt.h"

#define WITH_ST1	YES	
#define WITH_ST2	NO	
#define WITH_ST3	NO	
#define WITH_ST4	NO	


#if ( FDM_WITH_APER == NO )
#if ( FDM_WITH_MESSAGING == NO )
#define WITH_ST1	YES	
#else
#undef WITH_ST1
#undef WITH_ST2
#define WITH_ST1	NO	
#define WITH_ST2	YES
#endif
#else
#if ( FDM_WITH_BA == NO )
#undef WITH_ST1
#undef WITH_ST3
#define WITH_ST1	NO	
#define WITH_ST3	YES
#else
#undef WITH_ST1
#undef WITH_ST4
#define WITH_ST1	NO	
#define WITH_ST4	YES
#endif
#endif

#endif

