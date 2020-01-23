/***********************************************************************/
/*                                                                     */
/*                 B A S I C   P A R A M E T E R S                     */
/*                                                                     */
/***********************************************************************/

#ifndef __user_opt_h
#define __user_opt_h

#define YES	1
#define NO	0
#define infini	0
#define _5_millisecondes 5000
#define _40_millisecondes 40000

/***********************************************************************/
/*                                                                     */
/*      S O F T W A R E     C O N F I G U R A T I O N                  */
/*                                                                     */
/***********************************************************************/

#define  FDM_WITH_APER		YES

#if ( FDM_WITH_APER	==	YES )
#define  FDM_WITH_SM_MPS	YES
#endif

#define  FDM_WITH_MESSAGING	YES

#define  FDM_WITH_BA			NO
#define  FDM_WITH_OPTIMIZED_BA	NO

#define  FDM_WITH_DIAG		YES

#if 						(  FDM_WITH_BA == YES )
/******   SUSPEND_DELAI
	_5_millisecondes 	>>>> standard
	infini  		>>>> debug
********/
#define SUSPEND_DELAI		_40_millisecondes
#define	FDM_WITH_FIPIO		NO
#endif

#define FDM_WITH_CONTROLS  	YES

#define FDM_WITH_GT 		  NO
#define FDM_WITH_GT_ONLY_PRODUCED NO
#define FDM_WITH_GT_ONLY_CONSUMED NO


/***********************************************************************/
/*                                                                     */
/*      H A R D W A R E     C O N F I G U R A T I O N                  */
/*                                                                     */
/***********************************************************************/
#define  FDM_WITH_FIPCODE_31_25		YES
#define  FDM_WITH_FIPCODE_1000		NO
#define  FDM_WITH_FIPCODE_5000		NO

#define  FDM_WITH_BI_MEDIUM     	NO
#define  FDM_WITH_REDONDANCY_MGNT	NO
#define FDM_WITH_FREE_ACCESS		NO
#define FDM_WITH_CHAMP_IO			NO
#define FDM_WITH_DTACK   			YES
#define FDM_WITH_FIPCODE_LINKED  	YES
#define FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT	 NO

/***********************************************************************/
/*                                                                     */
/*      P R O C E S S O R    D E F I N I T I O N                       */
/*                                                                     */
/***********************************************************************/
#define FDM_WITH_I960			NO
#define FDM_WITH_68XXX			YES
#define FDM_WITH_DSP			NO
#define FDM_WITH_X86			NO


/***********************************************************************/
/*                                                                     */
/*      O P E R A T I N G    S Y S T E M       D E F I N I T I O N     */
/*                                                                     */
/***********************************************************************/

#define  FDM_WITH_PSOS			NO
#define  FDM_WITH_NT			NO
#define  FDM_WITH_VXWORKS		NO
#define  FDM_WITH_NDIS			NO

#if 						(  FDM_WITH_PSOS == YES )
#define  FDM_WITH_DELETE_RN_OVERRIDE		YES
#if 						(  FDM_WITH_I960  == YES )
#define  SPLX_VALUE_FOR_MASK_ALL	31
#endif
#if 						( FDM_WITH_68XXX   == YES )
#define  SPLX_VALUE_FOR_MASK_ALL	7
#endif
#endif



#define 	FDM_WITH_SEMAPHORE YES	
/* if FDM_WITH_SEMAPHORE == YES then
	if ( !PSOS && !FDM_WITH_VXWORKS && !FDM_WITH_NDIS ) then
		User must write :
			FDM_ressources_create
			FDM_ressources_delete
			OS_semaphore_take
			OS_semaphore_release
		in fdm_os.c
	endif
   endif
*/


/***********************************************************************/
/*                                                                     */
/*      C O M P I L A T E U R     D E F I N I T I O N                  */
/*                                                                     */
/***********************************************************************/
#define FDM_WITH_LITTLE_ENDIAN		NO


#define FDM_WITH_WIN32			NO
#define FDM_WITH_SCOUNIX		YES
#define FDM_WITH_MICROSOFT		NO
#define FDM_WITH_BORLAND		NO
#define FDM_WITH_METAWARE		NO
#define FDM_WITH_MICROTEC		NO
#define FDM_WITH_GNU			YES


/***********************************************************************/
/* AJOUT DE SIAT													   */
/***********************************************************************/

#define FDM_WITH_CAD_UL 		NO
#define FDM_WITH_MSDOS			NO
#define FDM_WITH_RTX			NO

/***********************************************************************/
/*                                                                     */
/*      R E S E R V E D    C E G E L E C                               */
/*                                                                     */
/***********************************************************************/

#define FDM_WITH_SPECIAL_USAGE		NO

#endif
