/************************************************************
   
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
 
 
      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdm_gdm.h
  Description     : B_A_O
                    Fichier entete pour Gestion de la Memoire
                   
 
  Date de creation: Fri Mar 15 15:11:06 MET 1996
  
***********************************************************/

#ifndef ___gdm
#define ___gdm

#include "user_opt.h"

#include "gdm_os.h"
 


/***************************************************************************/
/*                     PUBLIC CONSTANTS                                    */
/***************************************************************************/

#define size_objet      int
#define nbr_of_objet    int
enum GDM_STATES {
	_GDM_OPENED =1,
	_GDM_CLOSED
};

/***************************************************************************/
/*                          PUBLIC TYPES                                   */
/***************************************************************************/
typedef void * User_GDM;

typedef struct _T_File {

	struct  _T_File *next;
	struct  _T_File *prev;

	MEMORY_RN  *Type;
	enum	    GDM_STATES etat;

	short       nb_min;
	short     U_nbr;           
	void    * tete;
	void    * queue;
	struct _T_File * extend;
	short     E_nbr;
	short     U_nb_tot;
	} T_File;




/***************************************************************************/
/*                          PUBLIC FUNCTIONS                               */
/***************************************************************************/

#if __cplusplus

extern "C" {
#endif

/*Standard memory */
#if							(FDM_WITH_RTX  == YES)
__declspec ( dllexport )
#endif
T_File *  _NEW_USER ( MEMORY_RN *, size_objet , nbr_of_objet );

#if							(FDM_WITH_RTX  == YES)
__declspec ( dllexport )
#endif
int  	   _DELETE_USER ( T_File * ) ;

#if							(FDM_WITH_RTX  == YES)
__declspec ( dllexport )
#endif
User_GDM *  _ALLOCATE_MEMORY( T_File * );

#if							(FDM_WITH_RTX  == YES)
__declspec ( dllexport )
#endif
int         _FREE_MEMORY ( User_GDM * ) ;

/*Extented memory */
T_File *  _NEW_EXTEND ( MEMORY_RN * , size_objet , nbr_of_objet );

int        _DELETE_EXTEND ( T_File * );

int        _CONNECT_EXTEND ( T_File * , T_File *) ;

void      _DISCONNECT_EXTEND ( T_File * ) ;

T_File *  _GDM__SUIS_JE ( User_GDM * ) ;

void      _GDM__SWITCH_POOL ( User_GDM * , User_GDM *  );

enum	GDM_STATES  _GDM__USER_STATUS  ( T_File *  );


void OS_Leave_Region( void ) ;

void OS_Enter_Region( void ) ;


#if __cplusplus
}
#endif


/***************************************************************************/
/*                          PUBLIC MACROS                                  */
/***************************************************************************/
#define _IS_MEMORY_POOL_IN_USE(_Qui)   (_Qui->U_nb_tot != 0 ) ? 1 : 0

#define _MIN_NBR_OF_BUFFER(_Qui)   (_Qui)->nb_min


#endif
/*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*/
