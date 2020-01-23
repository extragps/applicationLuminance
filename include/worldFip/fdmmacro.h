/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmmacro.h
  Description     : Registres et Macros d'acces au composant



  Date de creation: Fri Mar 15 15:11:39 MET 1996

Version 4.6.0
	a/- Modif Acces Libres

***********************************************************/

#ifndef		__fdmmacro
#define 	__fdmmacro

extern void SWAB_SHORT (char *src,char *dest , int n);

#if 						( FDM_WITH_SCOUNIX == YES )
/* #include "fdm_kd.h" */
#endif	/* SCO-UNIX */

#if 						( FDM_WITH_GNU == YES )
#include "stdio.h"
#endif

#if						( FDM_WITH_VXWORKS == YES )
#include "sysLib.h"
#include "Tache.h"
#endif



#if 						( FDM_WITH_CHAMP_IO == YES )

#if ( FDM_WITH_MICROSOFT == YES ) | ( FDM_WITH_BORLAND == YES ) | ( FDM_WITH_METAWARE == YES )
#include <conio.h>
#endif

#endif




#define delai                   2000 /* attente sur SV et Busy*/

/************************** Private Definition *************************/
/* ustate Value */
#define Busy 0x80 /* transaction en cours               */
#define EOC  0x40 /* fin de cycle                       */
#define IRQ  0x20 /* demande d'interruption             */
#define SV   0x10 /* status valid                       */
#define FR   0x08 /* fifo prete                         */
#define AE   0x04 /* erreur d'acces                     */
#define FE   0x02 /* erreur de format                   */
#define IC   0x01 /*commande illegale                   */


/*definition des registers etr des modes d'acces */
#if 						( FDM_WITH_CHAMP_IO == YES )

/*lecture*/
#define UState     Ref->H.LOC_FIP[0]
#define UFlags     Ref->H.LOC_FIP[1]
#define Var_State  Ref->H.LOC_FIP[2]
#define Var_Size   Ref->H.LOC_FIP[3]
#define FILE_FIP   Ref->H.LOC_FIP[7]
#define FIFO       Ref->H.LOC_FIP[7]


#define UNKNOWN       Ref->H.LOC_FIP[5]


/*ecriture*/
#define UCom       Ref->H.LOC_FIP[0]
#define USwitch    Ref->H.LOC_FIP[1]
#define key_h      Ref->H.LOC_FIP[2]
#define key_l      Ref->H.LOC_FIP[3]
#define time_h     Ref->H.LOC_FIP[4]
#define time_l     Ref->H.LOC_FIP[5]


#define FIFO_Reg register __Port_Type__ FIFO_port

#else

/*lecture*/
#define UState    * Ref->H.LOC_FIP[0]
#define UFlags    * Ref->H.LOC_FIP[1]
#define Var_State * Ref->H.LOC_FIP[2]
#define Var_Size  * Ref->H.LOC_FIP[3]
#define FILE_FIP  * Ref->H.LOC_FIP[7]
#define FIFO        Ref->H.LOC_FIP[7]

#define UNKNOWN   * Ref->H.LOC_FIP[5]


/*ecriture*/
#define UCom      * Ref->H.LOC_FIP[0]
#define USwitch   * Ref->H.LOC_FIP[1]
#define key_h     * Ref->H.LOC_FIP[2]
#define key_l     * Ref->H.LOC_FIP[3]
#define time_h    * Ref->H.LOC_FIP[4]
#define time_l    * Ref->H.LOC_FIP[5]

#define FIFO_Reg   register volatile  unsigned char *FIFO_port

#endif


/** definiton de port E/S simples */
#if 						( FDM_WITH_CHAMP_IO == YES )

extern void blockoutbyte( __Port_Type__ port,unsigned char *src,        int taille);
extern void blockinbyte ( __Port_Type__ port,unsigned char *destination,int taille);

extern void blockoutword( __Port_Type__ port,unsigned short *src, int taille);
extern void blockinword ( __Port_Type__ port,unsigned short *destination,int taille);

#endif

#if 						( FDM_WITH_CHAMP_IO  == YES )
	#if ( FDM_WITH_MICROSOFT == YES )

		#if ( FDM_WITH_NDIS == YES )
			extern char FipndisReadPort (unsigned short port);
			#define LIRE(registre) 		FipndisReadPort(registre)
			extern char FipndisWritePort (unsigned short port, unsigned char value);
			#define ECRITE(registre,valeur) FipndisWritePort (registre,valeur)
			#define ECRIRE_UCOM(valeur) 	FipndisWritePort (UCom,valeur)
		#endif


		#if ( FDM_WITH_NDIS == NO  )
			#define LIRE(registre) 		     _inp (registre)
			#define ECRITE(registre,valeur) _outp(registre,valeur)
			#define ECRIRE_UCOM(valeur) _outp(UCom,valeur)
		#endif	/* autres*/

	#endif		/* FDM_WITH_MICROSOFT */


#if ( FDM_WITH_BORLAND == YES ) | (  FDM_WITH_MICROTEC == YES ) | ( FDM_WITH_METAWARE == YES )

#define LIRE(registre) 		    inp (registre)
#define ECRITE(registre,valeur) outp(registre,valeur)
#define ECRIRE_UCOM(valeur)     outp(UCom,valeur)

#endif /*  Borland .....*/

#if 						( FDM_WITH_CAD_UL == YES )

#define LIRE(registre)			inb (registre)
#define ECRITE(registre,valeur) outb(registre, valeur)
#define ECRIRE_UCOM(valeur)		outb(UCom,     valeur)

#endif	/*  CAD_UL */

#if 						( FDM_WITH_RTX == YES )
#define LIRE(registre)			RtReadPortUchar((PUCHAR) (registre))
#define ECRITE(registre,valeur) RtWritePortUchar((PUCHAR) (registre) , (UCHAR) (valeur) )
#define ECRIRE_UCOM(valeur)		RtWritePortUchar((PUCHAR) (UCom) , (UCHAR) (valeur) )
#endif	/*  RTX */

#if 						( FDM_WITH_SCOUNIX == YES )

#define LIRE(registre)		_inp_kd (registre)
#define ECRITE(registre,valeur) _outp_kd(registre, valeur)
#define ECRIRE_UCOM(valeur)	_outp_kd(UCom,     valeur)

#endif	/* SCO-UNIX */

#if 						( FDM_WITH_GNU == YES )

#define LIRE(registre)		sysInByte (registre)
#define ECRITE(registre,valeur) sysOutByte(registre, valeur)
#define ECRIRE_UCOM(valeur)	sysOutByte(UCom,     valeur)

#endif	/* SCO-UNIX */


#if 						( FDM_WITH_SOLARIS == YES )
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include <sys/types.h>

#include "drv.h"

extern ddi_acc_handle_t get_handle (caddr_t port);


#ifdef   SOLARIS_SPARC_5_5_1
	typedef int		f_int;
	#define fddi_getb(handle,dev_addr)	ddi_getb(handle,(uchar_t*)dev_addr)
	#define fddi_getw(handle,dev_addr)	ddi_getw(handle,(ushort_t*)dev_addr)
	#define fddi_getl(handle,dev_addr)	ddi_getl(handle,(ulong_t*)dev_addr)
	#define fddi_putb(handle,dev_addr,value)	ddi_putb(handle,(uchar_t*)dev_addr,(uchar_t)value)
	#define fddi_putw(handle,dev_addr,value)	ddi_putw(handle,(ushort_t*)dev_addr,(ushort_t)value)
	#define fddi_putl(handle,dev_addr,value)	ddi_putl(handle,(ulong_t*)dev_addr,(ulong_t)value)
	#define fddi_rep_putb(handle,host_addr,dev_addr,taille)		\
			ddi_rep_putb(handle,(uchar_t*)host_addr,(uchar_t*)dev_addr,(size_t) taille,DDI_DEV_NO_AUTOINCR)
	#define fddi_rep_getb(handle,host_addr,dev_addr,taille)		\
			ddi_rep_getb(handle,(uchar_t*)host_addr,(uchar_t*)dev_addr,(size_t) taille,DDI_DEV_NO_AUTOINCR)
	#define fddi_rep_putw(handle,host_addr,dev_addr,taille)		\
			ddi_rep_putw(handle,(ushort_t*)host_addr,(ushort_t*)dev_addr,(size_t) taille,DDI_DEV_NO_AUTOINCR)
	#define fddi_rep_getw(handle,host_addr,dev_addr,taille)		\
			ddi_rep_getw(handle,(ushort_t*)host_addr,(ushort_t*)dev_addr,(size_t) taille,DDI_DEV_NO_AUTOINCR)

#else

	typedef intptr_t	f_int;
	#define fddi_getb(handle,dev_addr)		ddi_get8(handle,(uint8_t*)dev_addr)
	#define fddi_getw(handle,dev_addr)		ddi_get16(handle,(uint16_t*)dev_addr)
	#define fddi_getl(handle,dev_addr)		ddi_get32(handle,(uint32_t*)dev_addr)
	#define fddi_putb(handle,dev_addr,value)	ddi_put8(handle,(uint8_t*)dev_addr,(uint8_t)value)
	#define fddi_putw(handle,dev_addr,value)	ddi_put16(handle,(uint16_t*)dev_addr,(uint16_t)value)
	#define fddi_putl(handle,dev_addr,value)	ddi_put32(handle,(uint32_t*)dev_addr,(uint32_t)value)
	#define fddi_rep_putb(handle,host_addr,dev_addr,taille)		\
			ddi_rep_put8(handle,(uint8_t*)host_addr,(uint8_t*)dev_addr,(size_t) taille,DDI_DEV_NO_AUTOINCR)
	#define fddi_rep_getb(handle,host_addr,dev_addr,taille)		\
			ddi_rep_get8(handle,(uint8_t*)host_addr,(uint8_t*)dev_addr,(size_t) taille,DDI_DEV_NO_AUTOINCR)
	#define fddi_rep_putw(handle,host_addr,dev_addr,taille)		\
			ddi_rep_put16(handle,(uint16_t*)host_addr,(uint16_t*)dev_addr,(size_t) taille,DDI_DEV_NO_AUTOINCR)
	#define fddi_rep_getw(handle,host_addr,dev_addr,taille)		\
			ddi_rep_get16(handle,(uint16_t*)host_addr,(uint16_t*)dev_addr,(size_t) taille,DDI_DEV_NO_AUTOINCR)

#endif

#define LIRE(registre)           fddi_getb(get_handle(registre),registre)
#define ECRITE(registre,valeur)  fddi_putb(get_handle(registre),registre, valeur)
#define ECRIRE_UCOM(valeur)      fddi_putb(get_handle(UCom),UCom,valeur)

#endif /* solaris = yes */
#endif /* Champio = yes */


#if 						( FDM_WITH_CHAMP_IO == NO  )

#define LIRE(registre)	 	registre
#define ECRITE(registre,valeur) registre = valeur
#define ECRIRE_UCOM(valeur)     UCom = valeur;

#endif




#if 						(FDM_WITH_DTACK == NO)
#define WAIT_FR  while (( (LIRE(UState) & FR ) == 0 )) {}
#else
#define WAIT_FR
#endif


/*** macro procedure INLINE ***/
/******************** Compte rendu ************************************/
#define Debut_procedure \
	CR.Fdm_Default = _NO_ERROR;					\
	CR.Information.Additional_Report = 0;				\
 	OS_fdm_sm_p;							\


#define CR_Var_State(Valeur) 	\
	CR.Information.Fipcode_Report._Var_State     = Valeur ;		\

#define CR_UState 		\
	CR.Information.Fipcode_Report._Ustate = LIRE(UState) ;		\

#define Fin_procedure 		\
	OS_fdm_sm_v; 							\
	if ( ( CR.Information.Fipcode_Report._Ustate != 0 ) || 		\
	   (( CR.Information.Fipcode_Report._Var_State & 0x80 ) != 0 ) )\
	  { 								\
	     CR.Fdm_Default = 	_FIPCODE_ERROR	;			\
	     Ref->H.User_Signal_Warning( Ref , CR );			\
  	     return ( 0x8000 ) ; 					\
  	   }								\
	return ( 0 ) ;							\

#define Fin_procedure_no_signal_warning \
	CR.Information.Fipcode_Report._Var_State =			\
				(unsigned)(LIRE(Var_State) & ~0x50);	\
	OS_fdm_sm_v; 							\
  	return ( (Ushort)CR.Information.Fipcode_Report._Var_State ) ; 	\


#define CR_time_out \
	ECRITE(UCom,CMDE_CLOSE) ;					\
	CR.Fdm_Default  = _TIME_OUT;					\
	OS_fdm_sm_v; 							\
	Ref->H.User_Signal_Warning( Ref , CR );				\
  	return ( 0x8000 ) ; 						\

#define WAIT_Busy \
	tempo = delai ;							\
	do {								\
		tempo--;						\
		if ( tempo == 0 )  { CR_time_out ; }			\
	} while ((LIRE(UState) & Busy) != 0  ) ;			\


#define WAIT_SV				\
	tempo = delai ;							\
	do {								\
		tempo--;						\
		if ( tempo == 0 )  { CR_time_out ; }			\
	} while ((LIRE(UState) & SV) == 0  ) ;				\


#define WAIT_READY				\
	tempo = delai ;							\
	do {								\
		tempo--;						\
		if ( tempo == 0 )  { CR_time_out ; }			\
	} while (LIRE(Var_Size)  != 0  ) ;				\



#define ECRIRE_Key(Valeur)			\
	ECRITE(key_h ,  (unsigned char)( Valeur>>8 ));			\
	ECRITE(key_l ,  (unsigned char)Valeur );			\

#define ECRIRE_GRANDE_ADR(Valeur)			\
	ECRITE(USwitch ,(unsigned char)( Valeur>>16 ));			\
	ECRITE(key_h ,  (unsigned char)( Valeur>>8 ));			\
	ECRITE(key_l ,  (unsigned char)Valeur );			\

#if						( FDM_WITH_CHAMP_IO == YES )

#define BLOCKOUTBYTE(port,src,taille)	blockoutbyte(port,src,taille)
#define BLOCKINBYTE(port,dest,taille)   blockinbyte(port,dest,taille)

#define BLOCKOUTWORD(port,src,taille)	blockoutword(port,src,taille)
#define BLOCKINWORD(port,dest,taille)   blockinword(port,dest,taille)


#else            /* mapping memoire */

#define BLOCKOUTBYTE(port,src,taille)		\
	if (taille != 0) {						\
	   do {								\
		WAIT_FR ;						\
		*port = *src++;						\
	   } while (--taille != 0 );					\
	}								\

#define BLOCKINBYTE(port,dest,taille)		\
	if (taille != 0) {						\
	   do {								\
		WAIT_FR ;						\
		*dest++ = *port;					\
	   } while (--taille != 0 );					\
	}								\


#define BLOCKOUTWORD(port,src,taille)		\
	if ((taille) != 0) {						\
	   do {								\
		Ushort Tmp1;						\
		Uchar  Tmp2;						\
		Tmp1 = *src++;						\
		Tmp2 = 	(Uchar)(Tmp1>>8);				\
		WAIT_FR ;						\
		*port = Tmp2;						\
		Tmp2 = 	(Uchar)Tmp1;					\
		WAIT_FR ;						\
		*port = Tmp2;						\
	   } while (--(taille) != 0 );					\
	}								\

#define BLOCKINWORD(port,dest,taille)		\
	if ((taille) != 0) {						\
	   do {								\
		Ushort Tmp1,Tmp2;					\
		WAIT_FR ;						\
		Tmp1   = (Ushort ) *port;				\
		Tmp1   = (Tmp1  & 0xFF ) << 8;				\
		WAIT_FR ;						\
		Tmp2   = (Ushort) *port;				\
		Tmp2   = (Ushort)(Tmp2  & 0xFF) ;			\
		*dest++  =(Ushort)( Tmp1 | Tmp2) ;			\
	   } while (--(taille) != 0 );					\
	}								\


#endif

#if		( FDM_WITH_FREE_ACCESS == YES )
typedef struct {
	volatile Ushort Opcode;
	Ushort Parametre;
	} Dial_AE;


#if (FDM_WITH_NT == YES)

#define NbLoop 			100000

#else

#define NbLoop 			20000
#define dix_microsecondes       0

#endif

#define MAC__WAIT_DVAR_LIBRE \
	if ( Desc->Var_Status != 0 ) {			\
		tempo = NbLoop;				\
		do {					\
			_FIP__petite_tempo(Ref);	\
			tempo --;			\
			if ( tempo == 0 ) {		\
			FDM_Signal_Warning ( Ref  , _CIRCUIT_ACCES_FAILED , free_Acc_time_out  ) ;\
				CR = free_Acc_DV_full ;\
				Desc->Var_Status =0;	\
				goto abortd;		\
			}				\
		} while (  Desc->Var_Status != 0 ) ;	\
	}						\


#define MAC__WAIT_PLACE \
	{\
		Ushort volatile *X ; \
		X = Ref->H.FREE_ACCES_ADDRESS;\
		OS_Enter_Region();\
		Tmp = *FifoUser;\
		X += Tmp ;\
		Tmp = (Ushort)((Tmp+2) & 0xef);\
		if ( *X != 0 ){\
			tempo = NbLoop;\
			while ( *X != 0 ){\
				_FIP__petite_tempo(Ref);\
				tempo--;\
				if ( tempo == 0 )\
				{\
					OS_Leave_Region();\
					FDM_Signal_Warning ( Ref  , _CIRCUIT_ACCES_FAILED , free_Acc_fifo_full  ) ;\
					CR = free_Acc_fifo_full;\
					goto abortd;\
				}\
			}\
		}\
	}\

#define MAC__WAIT_LECTURE_PRETE \
	tempo = NbLoop;\
	do {	\
		_FIP__petite_tempo(Ref);\
		tempo --;\
		if ( tempo == 0 ) {\
			FDM_Signal_Warning ( Ref  , _CIRCUIT_ACCES_FAILED , free_Acc_time_out  ) ;\
			CR = free_Acc_time_out ;\
			goto abort;	\
		}\
	} while (  PDial_AE->Opcode  != 0 ) ;\

#define MAC__WAIT_DONNEE_PRETE \
	OS_Leave_Region();\
	tempo = NbLoop;\
	do {	\
		_FIP__petite_tempo(Ref);\
		tempo --;\
		if ( tempo == 0 ) {\
			FDM_Signal_Warning ( Ref  , _CIRCUIT_ACCES_FAILED , free_Acc_time_out  ) ;\
			CR = free_Acc_time_out ;\
			goto abortd;	\
		}\
	} while (   (DescC->Var_Status & 1)  == 0 ) ;\


#define MAC__ECRIRE_DEMANDE(Code,Param) \
	PDial_AE = (Dial_AE *) Ref->H.FREE_ACCES_ADDRESS;\
	PDial_AE +=  ((*FifoUser)>>1);			\
	PDial_AE->Parametre = Param ;			\
	PDial_AE->Opcode    = Code ;			\
	*FifoUser = Tmp  ;    				\


#define MAC__ADR_DONNEES(Val) \
	P_Value_PTR  = (Ulong) (Val & 0x7fff) ;\
	P_Value_PTR *= 0x40 ;				\
	Abs_ADR      = Ref->H.FREE_ACCES_ADDRESS;	\
	Abs_ADR     += P_Value_PTR ;			\

#define MAC__SORTIE(P) \
	N   =  (P)->Cpt_Var_User  ;			\
	N--;						\
	(P)->Cpt_Var_User = N ;				\
	OS_Leave_Region();				\
	if (N != 0) OS_fdm_sm_v_vcom ;			\

#define MAC__ENTREE(P) \
	OS_Enter_Region();				\
	N   =  (P)->Cpt_Var_User  ;			\
	N++;						\
	(P)->Cpt_Var_User = N ;				\
	OS_Leave_Region();				\
	if (N != 1) OS_fdm_sm_p_vcom ;			\


#define MAC__INIT_REG \
	FifoUser =  Ref->FDM_Internal_Use.AL.FifoUser ;\
	FifoFip	 =  Ref->FDM_Internal_Use.AL.FifoFip ;\

#endif	/* acces libre */

#endif

