/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


  CEGELEC - 5 ,Av. Newton 92140 Clamart - BTP/CCD/DT4

      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmhandl.c
  Description     : Procedures generales d'interface avec FIPCODE_V6


  Date de creation: Fri Mar 15 15:11:37 MET 1996
  Historique des evolutions/modifications
  	Modification : 96.07.26

  	Reference Ra : RA xx
  	But de la modification :
  		modif Buffer--> FBuffer
  		(nom Buffer utilise dans <stdio.h> compilateur METAWARE 16bits)

   	Reference Ra : RA xx  (test ram lors essais acces libres -motorola-)  	  	But de la modification :
  		correction procedure READ_PHY_BYTE :
  			parametre entree nombre en byte:
  			en acces libre le diviser par 2
Version:4.1.2
	a/- pb compil microtec + chaine 186
Version:4.2.1
	a/-recodage acces libres MPS
Version:4.4.3
	a/ const devant extern fipcode_progr...
Version:4.5.0
	a/ modifs pour portage sur cible driver SCO-UNIX ( a completer avec un driver )
	b/ modifs pour portage sur cible DSP texas
	c/ modifs pour portage sur cible x86 sous VxWorks
Version:4.6.0
	a/ recodage suite a DE READ/WRITE vcom d'heure
	b/ recodage pour performance partie acces libres
***********************************************************/

    /******define NO_FREE_ACCES*********/
#include "user_opt.h"

#include "fdm.h"

#include "fdmmacro.h"

#if						(  FDM_WITH_MICROTEC == YES )
#if						( FDM_WITH_CHAMP_IO == YES )
#include <mriext.h>
void blockoutbyte(unsigned port,unsigned char *src, int taille)
{
#if 		( FDM_WITH_WIN32 == YES )
asm(" mov ecx , `taille`");
asm(" jcxz  arretob	");
asm(" mov edx , `port`	");
asm(" push esi		");
asm(" mov esi , `src`	");
asm(" cld		");
asm(" rep outsb 	");
asm(" pop esi		");
asm("arretob: 		");

#else

asm(" mov cx , `taille`	");
asm(" jcxz  arretob	");
asm(" mov dx , `port`	");
asm(" push ds		");
asm(" push si		");
asm(" lds si , `src`	");
asm(" cld		");
asm(" rep outsb 	");
asm(" pop si		");
asm(" pop ds		");
asm("arretob: 		");

#endif
}

void blockoutword(unsigned port,unsigned short *src, int taille)
{
#if 		( FDM_WITH_WIN32 == YES )
asm(" mov ecx , `taille`");
asm(" jcxz  arretow	");
asm(" mov edx , `port`	");
asm(" push esi		");
asm(" mov esi , `src`	");
asm(" cld		");

asm("suitew: 		");
asm(" lodsw		");
asm(" xchg	ah,al		");
asm(" out edx,al	");
asm(" xchg	ah,al		");
asm(" out edx,al	");
asm(" dec ecx		");
asm(" jne suitew		");

asm(" pop esi		");

asm("arretow: 		");

#else

asm(" mov cx , `taille`	");
asm(" jcxz  arretow	");
asm(" mov dx , `port`	");
asm(" push ds		");
asm(" push si		");
asm(" lds si , `src`	");
asm(" cld		");

asm("suitew: 		");
asm(" lodsw		");
asm(" xchg	ah,al		");
asm(" out dx,al 	");
asm(" xchg	ah,al		");
asm(" out dx,al 	");
asm(" dec cx		");
asm(" jne suitew		");

asm(" pop si		");
asm(" pop ds		");
asm("arretow: 		");

#endif
}


void blockinbyte(unsigned port,unsigned char *destination,int taille)
{
#if 		( FDM_WITH_WIN32 == YES )
asm("  mov ecx , `taille`	");
asm("  jcxz  arretib		");
asm("  mov edx , `port`		");
asm("  push edi 			");
asm("  mov edi , `destination`	");
asm("  cld			");
asm("  rep insb 		");
asm("  pop edi			");
asm("arretib: 			");

#else

asm("  mov cx , `taille`	");
asm("  jcxz  arretib		");
asm("  mov dx , `port`		");
asm("  push di 			");
asm("  les di , `destination`	");
asm("  cld			");
asm("  rep insb 		");
asm("  pop di			");
asm("arretib: 			");
#endif
}

void blockinword(unsigned port,unsigned short *destination,int taille)
{
#if 		( FDM_WITH_WIN32 == YES )
asm("  mov ecx , `taille`	");
asm("  jcxz  arretiw		");
asm("  mov edx , `port`		");
asm("  push edi 			");
asm("  mov edi , `destination`	");
asm("  cld			");

asm("suite: 		");
asm(" in	al,edx		");
asm(" xchg	ah,al		");
asm(" in    al,edx 	");
asm(" stosw 	");
asm(" dec ecx		");
asm(" jne suite		");

asm("  pop edi			");
asm("arretiw: 			");

#else

asm("  mov cx , `taille`	");
asm("  jcxz  arretiw		");
asm("  mov dx , `port`		");
asm("  push di 			");
asm("  les di , `destination`	");
asm("  cld			");

asm("suite: 		");
asm(" in al,dx		");
asm(" xchg	ah,al		");
asm(" in al,dx 	");
asm(" stosw 	");
asm(" dec cx		");
asm(" jne suite		");

asm("  pop di			");
asm("arretiw: 			");
#endif
}


#endif
#endif

/** definition des macro BLOCK in out **/

#if					( FDM_WITH_CHAMP_IO == YES )

#if 	(  FDM_WITH_MICROSOFT == YES ) | (  FDM_WITH_BORLAND == YES )
#pragma optimize("", off)

void blockoutbyte(unsigned port,unsigned char *src, int taille)
{
#if 		( FDM_WITH_WIN32 == YES )
__asm mov ecx , taille;
__asm jcxz  arret;
__asm mov edx , port;
__asm push esi;
__asm mov esi , src;
__asm cld;
__asm rep outsb ;
__asm pop esi;

#else

__asm mov cx , taille;
__asm jcxz  arret;
__asm mov dx , port;
__asm push ds;
__asm push si;
__asm lds si , src;
__asm cld;
__asm rep outsb ;
__asm pop si;
__asm pop ds;

#endif

arret: ;
}

void blockoutword(unsigned port,unsigned short *src, int taille)
{
#if 		( FDM_WITH_WIN32 == YES )
__asm mov ecx , taille;
__asm jecxz  arret;
__asm mov edx , port;
__asm push esi;
__asm mov esi , src;
__asm cld;

suite:
__asm  lodsw
__asm  xchg	ah,al
__asm  out dx,al
__asm  xchg	ah,al
__asm  out  dx,al
__asm  dec  ecx
__asm  jne suite

__asm pop esi;

#else

__asm mov cx , taille;
__asm jcxz  arret;
__asm mov dx , port;
__asm push ds;
__asm push si;
__asm lds si , src;
__asm cld;

suite:
__asm  lodsw
__asm  xchg	ah,al
__asm  out dx,al
__asm  xchg	ah,al
__asm  out dx,al
__asm  dec  cx
__asm  jne suite

__asm pop si;
__asm pop ds;

#endif

arret: ;
}

void blockinbyte(unsigned port,unsigned char *destination,int taille)
{
#if 		( FDM_WITH_WIN32 == YES )
__asm mov ecx , taille;
__asm jcxz  arret
__asm mov edx , port;
__asm push edi ;
__asm mov edi , destination;
__asm cld;
__asm rep insb ;
__asm pop edi

#else

__asm mov cx , taille;
__asm jcxz  arret
__asm mov dx , port;
__asm push di ;
__asm les di , destination;
__asm cld;
__asm rep insb ;
__asm pop di

#endif

arret: ;

}

void blockinword (unsigned port,unsigned short *destination,int taille)
{
#if 		( FDM_WITH_WIN32 == YES )
__asm mov ecx , taille;
__asm jcxz  arret
__asm mov edx , port;
__asm push edi ;
__asm mov edi , destination;
__asm cld;

suite:
__asm in	al,dx
__asm xchg	ah,al
__asm in    al,dx
__asm stosw
__asm  dec  ecx
__asm  jne suite

__asm pop edi

#else

__asm mov cx , taille;
__asm jcxz  arret
__asm mov dx , port;
__asm push di ;
__asm les di , destination;
__asm cld;

suite:
__asm in	al,dx
__asm xchg	ah,al
__asm in    al,dx
__asm stosw
__asm  dec  cx
__asm  jne suite

__asm pop di

#endif

arret: ;

}

#pragma optimize("", on)
#endif /* microsoft ou borland */

#if 						( FDM_WITH_SCOUNIX == YES )

void blockoutbyte(unsigned port, unsigned char *src, int taille)
{
  _outp_block_kd(port, (char *) src, taille);
}

void blockinbyte(unsigned port, unsigned char *destination, int taille)
{
  _inp_block_kd(port, (char *) destination, taille);
}

void blockoutword(unsigned port, unsigned short *src, int taille)
{
	if ((taille) != 0) {
	   do {
		Ushort Tmp1;
		Uchar  Tmp2;
		Tmp1 = *src++;
		Tmp2 = 	(Uchar)(Tmp1>>8);
		WAIT_FR ;
                _outp_kd(port, Tmp2);
		Tmp2 = 	(Uchar)Tmp1;
		WAIT_FR ;
                _outp_kd(port, Tmp2);
	   } while (--(taille) != 0 );
	}

}

void blockinword(unsigned port, unsigned short *destination, int taille)
{
	if ((taille) != 0) {
	   do {	
		Ushort Tmp1,Tmp2;
		WAIT_FR ;
		Tmp1= _inp_kd(port);
		Tmp1   = (Tmp1  & 0xFF ) << 8;
		WAIT_FR ;
		Tmp2 = _inp_kd(port);
		Tmp2   = (Ushort)(Tmp2  & 0xFF) ;
		*destination++  =(Ushort)( Tmp1 | Tmp2) ;
	   } while (--(taille) != 0 );
	}
}

#endif	/* SCO-UNIX */


#if 							( FDM_WITH_GNU == YES )

void blockoutbyte(unsigned port, unsigned char *src, int taille)
{

	if (taille != 0) {
	   do {
		WAIT_FR ;
		sysOutByte( port , *src++ );
	   } while (--taille != 0 );
	}
}

void blockinbyte(unsigned port, unsigned char *destination, int taille)
{
	if (taille != 0) {
	   do {
		WAIT_FR ;
		*destination++ = sysInByte(port);
	   } while (--taille != 0 );
	}

}

void blockoutword(unsigned port, unsigned short *src, int taille)
{
	if ((taille) != 0) {
	   do {
		Ushort Tmp1;
		Uchar  Tmp2;
		Tmp1 = *src++;
		Tmp2 = 	(Uchar)(Tmp1>>8);
		WAIT_FR ;
		sysOutByte( port , Tmp2 );
		Tmp2 = 	(Uchar)Tmp1;
		WAIT_FR ;
		sysOutByte( port , Tmp2 );
	   } while (--(taille) != 0 );
	}
}

void blockinword(unsigned port,  unsigned short *destination, int taille)
{
	if ((taille) != 0) {
	   do {
		Ushort Tmp1,Tmp2;
		WAIT_FR ;
		Tmp1= sysInByte(port);
		Tmp1   = (Tmp1  & 0xFF ) << 8;
		WAIT_FR ;
		Tmp2 = sysInByte(port);
		Tmp2   = (Ushort)(Tmp2  & 0xFF) ;
		*destination++  =(Ushort)( Tmp1 | Tmp2) ;
	   } while (--(taille) != 0 );
	}
}


#endif	/* GNU */


#if 				(  FDM_WITH_CAD_UL	==		YES )

void blockoutbyte(unsigned port,unsigned char *src, int taille)
{  
	if (taille != 0) {
	   do {
		WAIT_FR ;
		outb( port , *src++ );
	   } while (--taille != 0 );
	}

}

void blockoutword(unsigned port,unsigned short *src, int taille)
{ 

	if ((taille) != 0) {
	   do {
		Ushort Tmp1;
		Uchar  Tmp2;
		Tmp1 = *src++;
		Tmp2 = 	(Uchar)(Tmp1>>8);
		WAIT_FR ;
		outb( port , Tmp2 );
		Tmp2 = 	(Uchar)Tmp1;
		WAIT_FR ;
		outb( port , Tmp2 );
	   } while (--(taille) != 0 );
	}

}

void blockinbyte(unsigned port,unsigned char *destination,int taille)
{
	if (taille != 0) {
	   do {
		WAIT_FR ;
		*destination++ = inb(port);
	   } while (--taille != 0 );
	}

}


void blockinword(unsigned port,unsigned short *destination,int taille)
{
	if ((taille) != 0) {
	   do {
		Ushort Tmp1,Tmp2;
		WAIT_FR ;
		Tmp1= inb(port);
		Tmp1   = (Tmp1  & 0xFF ) << 8;
		WAIT_FR ;
		Tmp2 = inb(port);
		Tmp2   = (Ushort)(Tmp2  & 0xFF) ;
		*destination++  =(Ushort)( Tmp1 | Tmp2) ;
	   } while (--(taille) != 0 );
	}
}


#endif   /* CAD_UL */
#if 				(  FDM_WITH_RTX	==		YES )

void blockoutbyte(unsigned port,unsigned char *src, int taille)
{  
	RtWritePortBufferUchar((PUCHAR) port , (PUCHAR) src , (ULONG) taille );
}

void blockoutword(unsigned port,unsigned short *src, int taille)
{ 

	if ((taille) != 0) {
	   do {
		Ushort Tmp1;
		Uchar  Tmp2;
		Tmp1 = *src++;
		Tmp2 = 	(Uchar)(Tmp1>>8);
		WAIT_FR ;
		ECRITE( port , Tmp2 );
		Tmp2 = 	(Uchar)Tmp1;
		WAIT_FR ;
		ECRITE( port , Tmp2 );
	   } while (--(taille) != 0 );
	}

}

void blockinbyte(unsigned port,unsigned char *destination,int taille)
{
	RtReadPortBufferUchar((PUCHAR) port , (PUCHAR) destination , (ULONG) taille );
}


void blockinword(unsigned port,unsigned short *destination,int taille)
{
	if ((taille) != 0) {
	   do {
		Ushort Tmp1,Tmp2;
		WAIT_FR ;
		Tmp1= LIRE(port);
		Tmp1   = (Tmp1  & 0xFF ) << 8;
		WAIT_FR ;
		Tmp2 = LIRE(port);
		Tmp2   = (Ushort)(Tmp2  & 0xFF) ;
		*destination++  =(Ushort)( Tmp1 | Tmp2) ;
	   } while (--(taille) != 0 );
	}
}


#endif   /* RTX */


#if                  						( FDM_WITH_SOLARIS == YES )
#include "drv.h"


extern paramPortIo tabPortIo[4];

ddi_acc_handle_t get_handle (caddr_t port)
{
	int i=0;
	
	while (i<4){
		if (tabPortIo[i].portIo != NULL){
	 		if ((port <= (*(tabPortIo[i].portIo) + MAX_SIZE_PORT_IO)) && 
	     		    (port >= *(tabPortIo[i].portIo))) 				
	     		    	return *(tabPortIo[i].portIo_handle);
	    	}
	    	i++;
	}
	return NULL;
}

void blockoutbyte(__Port_Type__ port, unsigned char *src, int taille)
{
	fddi_rep_putb(get_handle(port),(uint8_t*)src,port,taille); 
}

void blockinbyte(__Port_Type__ port, unsigned char *destination, int taille)
{
	fddi_rep_getb(get_handle(port),(uint8_t*)destination,port,taille); 
}

#if                  						( FDM_WITH_LITTLE_ENDIAN == YES )

void blockoutword(__Port_Type__ port, unsigned short *src, int taille)
{
	fddi_rep_putw(get_handle(port),(uint16_t*)src,port,taille); 
 
}

void blockinword(__Port_Type__ port, unsigned short *destination, int taille)
{
	fddi_rep_getw(get_handle(port),(uint16_t*)destination,port,taille); 
}

#else

void blockoutword(__Port_Type__ port, unsigned short *src, int taille)
{
	ddi_acc_handle_t v;
	
	if ((taille) != 0) {							   
	do {		
		Ushort Tmp1;						
		Uchar  Tmp2;
		
		v=get_handle(port);					
		Tmp1 = *src++;						
		Tmp2 = 	(Uchar)(Tmp1>>8);				
		fddi_putb(v,port,Tmp2); 
		Tmp2 = 	(Uchar)Tmp1;					
		fddi_putb(v,port,Tmp2); 
	   } while (--(taille) != 0 );					
	}
}

void blockinword(__Port_Type__ port, unsigned short *destination, int taille)
{
	ddi_acc_handle_t v;
	
	if ((taille) != 0) {
	   do {
		Ushort Tmp1,Tmp2;
		
		v=get_handle(port);
		WAIT_FR ;
		Tmp1= fddi_getb(v,port);
		Tmp1   = (Tmp1  & 0xFF ) << 8;
		WAIT_FR ;
		Tmp2 = fddi_getb(v,port);
		Tmp2   = (Ushort)(Tmp2  & 0xFF) ;
		*destination++  =(Ushort)( Tmp1 | Tmp2) ;
	   } while (--(taille) != 0 );
	}
}
#endif	/* SOLARIS Little */

#endif	/* SOLARIS */

#endif /* Champ IO */

void SWAB_SHORT (char *src,char *dest , int n)
{
	int i;
	Ushort *S;
	Ushort *D;
	unsigned int R;

	S = ( Ushort *)src ;
	D = ( Ushort *)dest ;

	for (i = n/2 ; i != 0 ; i-=1) {
		R = *S++;
		R =  ((R>>8) | (R<< 8));
		*D++ = (Ushort) R ;
	}

}

Ulong SWAB_ONE_LONG ( Ulong mot_long )
{
    register Ulong res;

	res =
		 ( mot_long >> 24 ) |
		(( mot_long >> 8 ) & 0x0000ff00L ) |
		(( mot_long << 8 ) & 0x00ff0000L ) |
		 ( mot_long << 24 ) ;

	return ( res );
}


	/****************************************
	*       ecriture physique               *
	*****************************************
parametres d'entree :
	adresse physique,
	adresse buffer source,taille de l'objet (en octet)
parametres de sortie:
*/
Ushort _FIP_WRITE_PHY ( FDM_REF * Ref ,
	Absolute_ADR Adr  , const Ushort *source , int nombre )

{
#if		( FDM_WITH_FREE_ACCESS == NO )
	int tempo ;
	FDM_ERROR_CODE CR ;

	register unsigned int taille;
   	FIFO_Reg;
	register Ushort *src;
	Acces_Key Key ;


	Key = (Ushort) (Adr / 64);

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );

	ECRITE(UCom , CMDE_W_PHY );

	/* ici pour occuper utilement le micro */
    	FIFO_port = FIFO;
    	taille = nombre/2 ;
	src = (Ushort *)source ;
	/************ attente bit SV */
	WAIT_SV;

	/************ boucle d'ecriture */
	BLOCKOUTWORD(FIFO_port,src,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;

#else
	Ushort volatile * Destination ;
 	int i;

	i = nombre/2;
	Destination = Ref->H.FREE_ACCES_ADDRESS;
	Destination += Adr;
	do {
		 *Destination++ = *source++ ;

	} while ( --i != 0  ) ;

	return 0;
#endif

}

Ushort _FIP_WRITE_PHY_BYTE ( FDM_REF * Ref ,
	Absolute_ADR Adr  , const Uchar * source , int nombre )

{
	int tempo ;
	FDM_ERROR_CODE CR ;

	register unsigned int taille;

    	FIFO_Reg;

	register unsigned char *src;
	Acces_Key Key ;


	Key = (Ushort) (Adr / 64);

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );

	ECRITE(UCom , CMDE_W_PHY );

	/* ici pour occuper utilement le micro */
    FIFO_port = FIFO;
    src = (unsigned char*) source ;
    taille = nombre ;

	/************ attente bit SV */
	WAIT_SV;

	/************ boucle d'ecriture */
	BLOCKOUTBYTE(FIFO_port,src,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;

}


	/****************************************
	*       lecture physique                *
	*****************************************
parametres d'entree :
	adresse physique,
	adresse buffer destination,
	taille de l'objet(max 128 octets)
parametres de sortie:
*/

Ushort _FIP_READ_PHY_BYTE ( FDM_REF * Ref ,
	Absolute_ADR Adr  , Uchar *destination , int nombre )

{
	int tempo ;
	FDM_ERROR_CODE CR ;

	register unsigned int taille;
    	FIFO_Reg;
	register unsigned char *dst;

	Acces_Key Key ;

	Key = (Ushort) (Adr / 64);

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );

	ECRITE(UCom , CMDE_R_PHY );

	/* ici pour occuper utilement le micro */
    FIFO_port = FIFO;
    dst = (unsigned char*) destination ;
    taille = nombre ;

	/************ attente bit SV */
	WAIT_SV;

	/************ boucle de lecture */
	BLOCKINBYTE(FIFO_port,dst,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;

}


Ushort _FIP_READ_PHY ( FDM_REF * Ref ,
	Absolute_ADR Adr  , Ushort *destination , int nombre )

{
#if		( FDM_WITH_FREE_ACCESS == NO )
	int tempo ;
	FDM_ERROR_CODE CR ;


	register unsigned int taille;
   	FIFO_Reg;
	register Ushort *dst;

	Acces_Key Key ;

	Key = (Ushort) (Adr / 64);

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );

	ECRITE(UCom , CMDE_R_PHY );

	/* ici pour occuper utilement le micro */
	   FIFO_port = FIFO;
	   taille = nombre /2;
	   dst =  destination;
	/************ attente bit SV */
	WAIT_SV;

	/************ boucle de lecture */

	BLOCKINWORD(FIFO_port,dst,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;
#else

	Ushort volatile * Sour ;
 	int i;

	i = nombre/2;
	Sour = Ref->H.FREE_ACCES_ADDRESS;
	Sour  += Adr;
	do {
		 *destination++ = *Sour++ ;

	} while ( --i != 0  ) ;

	return 0;

#endif

}

#if			( FDM_WITH_FREE_ACCESS == NO )

/*********** Procedure d'Ecriture de DESCRIPTEUR
FULLFIP is Running  !!!!!!!!!!!!!!!!!!!!!!!!!!!
************/


static Ushort _FIP_WRITE_DESC
	( FDM_REF * Ref ,Absolute_ADR Adr, const Ushort * source )
{
	int tempo ;
	FDM_ERROR_CODE CR ;

	register unsigned int taille;
    FIFO_Reg;
	register Ushort *src;


	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_GRANDE_ADR( Adr );

	ECRITE(UCom , CMDE_W_DESC );

	/* ici pour occuper utilement le micro */
	FIFO_port = FIFO;
	taille = 16 ;
	src =  (Ushort *)source ;

	/************ attente bit SV */
	WAIT_SV;

	/************ boucle d'ecriture */
	BLOCKOUTWORD(FIFO_port,src,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
    	ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;

}
#endif

/***************** Fipload ***********************************/
#include "fipcode6.h"
#if					( FDM_WITH_NT == YES )
#include <ntddk.h>
#endif
extern    const  Fip_code_Programme_Type Fip_code_Programme  ;

#if (FDM_WITH_SOLARIS == YES)
	#include "drv_fdm.h"
#endif

extern const unsigned char fipcod6_1_txt[] ;
extern const unsigned char fipcod6_3_txt[] ;
extern const unsigned char fipcod6_5_txt[] ;

Ushort _FIP_DOWNLOAD_PRG( FDM_REF *Ref )
{
#if (FDM_WITH_FIPCODE_LINKED == NO )


#if (FDM_WITH_SOLARIS == YES)
	return _FIP_DOWNLOAD_SOLARIS ( Ref ) ;
#endif



#if (	( FDM_WITH_NT == YES ) || ( FDM_WITH_DSP == YES ) )
	return _FIP_DOWNLOAD_NT_FILE( Ref ) ;
#endif



#if	( FDM_WITH_MSDOS  == YES ) 

	FILE	 *stream;
	short    CR;
	int 	 i;
	Ulong	 no_err;
	char 	*pathvar ;
	char	 pathbuf[80];
  	Fip_code_Page  T;
	Ushort X;
	Ushort Nb_Page_Fc;

    pathvar = getenv ("FIP" );
    strcpy (pathbuf , pathvar );
    switch ( Ref->S.Mode ){
    	case FIP_31:
    	case WORLD_FIP_31:
	    	strcat ( pathbuf ,"\\fipcod6_3.dat");
		break;
    	case WORLD_FIP_1000:
    	case WORLD_FIP_2500:
    	case WORLD_FIP_1000:
    	case FAST_FIP_1000:
    	case SLOW_FIP_1000:
	    	strcat ( pathbuf ,"\\fipcod6_1.dat");
		break;
    	case WORLD_FIP_5000:
    	case FIP_5000:
	    	strcat ( pathbuf ,"\\fipcod6_5.dat");
		break;
    	default:
		no_err=1 ;
		goto abort1;
	}

    if ((stream = fopen(pathbuf,"rb")) == NULL ) {
		no_err=1 ;
		goto abort1;
    }
    fread ((char*)&Nb_Page_Fc,1,2,stream);  /* nombre de pages */
    if ( Nb_Page_Fc >= Nb_Page_Max_PREVU ) goto abort1;

    Ref->FDM_Internal_Use.Nb_Page_Vrai = Nb_Page_Fc;
    if ( Nb_Page_Fc > Nb_Page_Max_PREVU ) {
	 FDM_Signal_Error (  Ref , _FIPCODE_FAIL  ,(Ulong) 's');
         return FDM_NOK;
    }
    for (i = 0 ; i < (int) Nb_Page_Fc ; i++ ){
		fread ((char*)&X,1,2,stream);
        if ( feof(stream) || ferror(stream) ){
			no_err=2 ;
			goto abort1;
        }
		fread ((char*)&T.code[0],1,128,stream);
        if ( feof(stream) || ferror(stream) ){
			no_err=2 ;
			goto abort1;
        }
		Ref->FDM_Internal_Use.tab_BC[i].base=(int)X;
		CR = _FIP_WRITE_PHY_BYTE(Ref,(Ulong)X,&T.code[0],128);
		if (CR != 0 ){
			no_err=3;
			goto abort1;
        }
      }

     /* lecture table de checksum du fipcode */
     for (i = 0 ; i < (int) Nb_Page_Fc ; i++ ){
	 fread ((char*)&Ref->FDM_Internal_Use.tab_BC[i].checksum,1,2,stream);
         if ( feof(stream) || ferror(stream) ){
		no_err=4;
		goto abort1;
         }
      }

     /* fin chargement */
	fclose(stream);
	return(0);

abort1:
	fclose(stream);
	FDM_Signal_Warning ( Ref ,_ERR_FIP_DOWLOAD_FILE, no_err );
	return(FDM_NOK);


#endif

#else  /* FDM_WITH_FIPCODE_LINKED == YES) */

    unsigned char	 *stream;
    int 		i;
    Ulong	 	no_err;
    Ushort 		X,Nb_Page_Fc;
    short    CR;
    Fip_code_Page  T;
    unsigned short    Tmp[64];

    no_err = 1;

    switch ( Ref->S.Mode ){
#if ( FDM_WITH_FIPCODE_31_25 == YES )
    case FIP_31:
    case WORLD_FIP_31:
	stream = (unsigned char	*)fipcod6_3_txt;
	break;
#endif
#if ( FDM_WITH_FIPCODE_1000 == YES )
    case WORLD_FIP_1000    :
    case FAST_FIP_1000     :
    case SLOW_FIP_1000   :
    case WORLD_FIP_2500   :
    case FIP_2500    :
	stream = (unsigned char	*)fipcod6_1_txt;
	break;
#endif

#if ( FDM_WITH_FIPCODE_5000 == YES )
    case WORLD_FIP_5000   :
    case FIP_5000    :
	stream = (unsigned char	*)fipcod6_5_txt;
	break;
#endif

    default:
	no_err = 8;
	goto abort1;
	}

    memcpy(&Nb_Page_Fc , stream , 2);
    stream += 2;
#if (FDM_WITH_LITTLE_ENDIAN == NO )
 	SWAB_SHORT ( (char*)&Nb_Page_Fc , (char*)&Nb_Page_Fc , 2 ) ;
#endif 

    if ( Nb_Page_Fc >= Nb_Page_Max_PREVU ) {
	no_err = 1;
	goto abort1;
	}
    Ref->FDM_Internal_Use.Nb_Page_Vrai = Nb_Page_Fc;

    for (i = 0 ; i < (int) Nb_Page_Fc ; i++ ){
    	memcpy(&X , stream , 2);
	stream += 2;
#if (FDM_WITH_LITTLE_ENDIAN == NO )
 	SWAB_SHORT ( (char*)&X , (char*)&X, 2 ) ;
#endif
	memcpy(&T.code[0],stream,128);
	stream += 128;
	Ref->FDM_Internal_Use.tab_BC[i].base=(int)X;

#if (FDM_WITH_LITTLE_ENDIAN == YES )
	SWAB_SHORT((char*)&T.code[0],(char*)Tmp,128);
	CR = _FIP_WRITE_PHY(Ref,(Ulong)X,Tmp,128);
#else
	CR = _FIP_WRITE_PHY(Ref,(Ulong)X,(Ushort*)&T.code[0],128);
#endif
	if (CR != 0 ){
		no_err=3;
		goto abort1;
      	 }
      }

     /* lecture table de checksum du fipcode */
     for (i = 0 ; i < (int) Nb_Page_Fc ; i++ ){
	memcpy(&Ref->FDM_Internal_Use.tab_BC[i].checksum,stream,2);
	stream += 2;
#if (FDM_WITH_LITTLE_ENDIAN == NO )
 	SWAB_SHORT ( (char*)&Ref->FDM_Internal_Use.tab_BC[i].checksum , (char*)&Ref->FDM_Internal_Use.tab_BC[i].checksum , 2 ) ;
#endif

      }
	return(FDM_OK);


abort1:
	FDM_Signal_Warning ( Ref ,_ERR_FIP_DOWLOAD_FILE, no_err );
	return(FDM_NOK);


#endif

}




Ushort _FIP_RAZ_RAM ( FDM_REF *Ref )
{

	Ulong i;
	short CR;
	Uchar Buf[128];

	memset ( Buf , 0 , sizeof(Buf));

	for (i = 0 ; i < Ref->S.FULLFIP_RAM_Dim; i += 64 ){
#if (FDM_WITH_FREE_ACCESS == NO)
		CR = _FIP_WRITE_PHY_BYTE( Ref, i , Buf,128);
#else
		CR = _FIP_WRITE_PHY( Ref, i ,(Ushort *) Buf,128);
#endif
		if (CR != 0 ) { return ( CR ) ;
		}
	}
	return ( 0 );
}


Ushort _FIP_WRITE_DESCRIPTEUR  ( FDM_REF * Ref ,Absolute_ADR Adr,
			const Ushort * buffer )
{
#if			( FDM_WITH_FREE_ACCESS == NO )
	Ushort Desc[4][16];
	Ushort Cr ;
	if ( Ref->FDM_Internal_Use.fipgen.Fip_run ) {
		Cr = _FIP_WRITE_DESC (Ref , Adr , buffer );
	} else {
		Cr = _FIP_READ_PHY ( Ref , Adr , (Ushort *)Desc , 128 );
		memcpy( &Desc[(int)( (Adr>>4) & 0x3)][0] , (char*) buffer , sizeof(DESC_VAR_TYPE) ) ;
		Cr |=_FIP_WRITE_PHY ( Ref ,  Adr , (Ushort *)Desc , 128 );
	}
	return Cr ;
#else
#if						( FDM_WITH_PLX9050 == NO )
	Ushort volatile * Destination ;
 	int i;

	i = 13;
	Destination = Ref->H.FREE_ACCES_ADDRESS;
	Destination += Adr;
	do {
		 *Destination++ = *buffer++ ;

	} while ( --i != 0  ) ;

	Destination++ ;
	buffer++ ;

	*Destination++ = *buffer++ ;
 	*Destination++ = *buffer++ ;


	return 0;
#else
	Ulong volatile * LDestination ;
	Ulong volatile * LSource ;
	Ushort volatile * Destination ;
 	int i;

	i = 6;
	Destination = Ref->H.FREE_ACCES_ADDRESS;
	Destination += Adr;
	LDestination = ( Ulong volatile *) Destination ;
	LSource	     = ( Ulong volatile *) buffer;
	do {
	 	*LDestination++ = *LSource++ ;
	} while ( --i != 0  ) ;

	Destination  	= ( Ushort volatile *) 	LDestination;
	buffer		= ( Ushort  *) 		LSource ;

	*Destination++ = *buffer++ ;  /* 12 */

/*	LDestination++ ;*/	      /* 12 et 13 skip*/
/*	LSource++ ;*/

	*++LDestination = *++LSource ;  /* 14 et 15 */ 

	return 0;

#endif
#endif
}


Ushort _FIP_START_CIRCUIT ( FDM_REF * Ref  )
{

	int volatile tempo ;
	FDM_ERROR_CODE CR ;

	Acces_Key Key =  FIP_START_PROG ;

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture adresse de debut */
	ECRIRE_Key( Key );

	ECRITE(UCom , CMDE_START );

	/************ attente bit SV */
	WAIT_SV;

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;

}

Ushort _FIP_END_DIAG ( FDM_REF * Ref  )
{
	Ulong volatile grande_tempo;

	int volatile tempo ;
	FDM_ERROR_CODE CR ;

	Debut_procedure ;

	WAIT_Busy;


	ECRITE(UCom , CMDE_VALID );

	_FIP__petite_tempo(Ref);


#if			( FDM_WITH_FREE_ACCESS == NO ) || (FDM_WITH_DIAG == NO)
	/************ attente bit SV */
	WAIT_SV;
#endif

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	OS_fdm_sm_v;

	grande_tempo = 10000000L;
	while (LIRE(UFlags) != 1) {
		grande_tempo--;
		if ( grande_tempo == 0 ) {
			CR.Fdm_Default  = _TIME_OUT;
			CR.Information.Additional_Report = 1;
	     		Ref->H.User_Signal_Warning(Ref , CR );
  	     		return ( 0x8000 ) ;
 	     	}
	}

	Ref->FDM_Internal_Use.fipgen.Fip_run = 1;

	return 0;
}

Ushort _FIP_READ_WORD ( FDM_REF * Ref ,Absolute_ADR Adr,
			 Ushort * data )
{
#if			( FDM_WITH_FREE_ACCESS == NO )

	int tempo ;
	FDM_ERROR_CODE CR ;
	register int taille;
	register  Ushort *dst;
    	FIFO_Reg;



	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_GRANDE_ADR( Adr );

	ECRITE(UCom , CMDE_R_WORD );

        dst = data;
        taille = 1;
        FIFO_port = FIFO;

	/************ attente bit SV */
	WAIT_SV;

	BLOCKINWORD(FIFO_port,dst,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;
#else
	Ushort volatile  * Source ;

	Source = Ref->H.FREE_ACCES_ADDRESS;
	Source += Adr;
	*data = *Source ;


	return 0;
#endif
}


Ushort _FIP_WRITE_WORD  ( FDM_REF * Ref ,Absolute_ADR Adr , Ushort Val )
{
#if			( FDM_WITH_FREE_ACCESS == NO )
	int tempo ;
	FDM_ERROR_CODE CR ;


	register  int taille;
    	FIFO_Reg;
	register  Ushort *src;



	Debut_procedure ;

	WAIT_Busy;

	ECRIRE_GRANDE_ADR( Adr );

	ECRITE(UCom , CMDE_W_WORD );

	taille =  1 ;
	FIFO_port = FIFO;
        src = &Val ;

	/************ attente bit SV */
	WAIT_SV;

	/************ boucle d'ecriture */
	BLOCKOUTWORD(FIFO_port,src,taille);

	WAIT_READY;

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;
#else
	Ushort volatile * Dest ;

	Dest = Ref->H.FREE_ACCES_ADDRESS;
	Dest += Adr;
	*Dest = Val ;


	return 0;
#endif
}

void _FIP__petite_tempo(FDM_REF *Ref)
{
#undef rien

#if 			(FDM_WITH_NT == YES)
#define rien 
 	KeStallExecutionProcessor( 10L);   /* 10micro secondes */
#endif

#ifndef  rien
int VAR_STATE ;

	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;


	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;
	VAR_STATE = LIRE(UState) ;

#endif
}

void _FIP__tres_petite_tempo(FDM_REF *Ref)
{

#undef rien

#if 			(FDM_WITH_NT == YES)
#define rien 
 	KeStallExecutionProcessor( 10L);   /* 10micro secondes */
#endif

#ifndef  rien

	int VAR_STATE ;

	VAR_STATE = LIRE(UState) ;

	VAR_STATE = LIRE(UState) ;

#endif
}


	/****************************************
	*       ecriture d'une variable         *
	*****************************************
	parametres d'entree :
		- cle d'acces
		- adresse buffer source
	******************************************/
Ushort _FIP_WRITE_VAR( FDM_REF *Ref ,  Acces_Key Key ,
				 const void *source)
{
#if		( FDM_WITH_FREE_ACCESS == NO )
	int VAR_STATE ;
	int  tempo;
	FDM_ERROR_CODE CR ;
	register unsigned int taille;

    	FIFO_Reg;
	register unsigned char *src;

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );

	/************ ecriture de la commande */
	ECRITE(UCom , CMDE_W_VAR );

	/* ici pour occuper utilement le micro */
	FIFO_port = FIFO;
	src = (unsigned char *)source ;

	/************ attente bit SV */
	WAIT_SV;

	_FIP__tres_petite_tempo(Ref);

	/************ test de valididite de la variable */
        /* Var_state= 81h si non_prod;
                       0  si OK */

	VAR_STATE = LIRE(Var_State) ;
	if ((VAR_STATE & 0x80) != 0 ) {
                ECRITE(UCom,CMDE_CLOSE) ;
                CR_Var_State(VAR_STATE);
                Fin_procedure;
	}

	/************ boucle d'ecriture de l'objet */
	taille = LIRE(Var_Size) ;

	BLOCKOUTBYTE(FIFO_port,src,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	WAIT_Busy;

	CR_Var_State( LIRE(Var_State) ) ;
	/* Var_State = 87h si overflow;
		       8Ch si undeflow;
		        0h si OK
	*/

	Fin_procedure;

#else
	/****************************************/
	/*********** Acces libres ***************/
	/*********** FIP_WRITE_VAR **************/
	/****************************************/
	volatile Ushort *FifoUser, *FifoFip  ;
	T_Desc_Ptr *Desc_Ptr ;
	AE_P_DESC_VAR_TYPE * Desc;
	Ulong P_Value_PTR ;
	int Lg_Var;
	Ushort volatile *Abs_ADR;
	int  tempo;
	Ushort CR ,N ;
	Ushort Tmp  ;
 	Dial_AE *PDial_AE;

	Desc_Ptr = ( T_Desc_Ptr *) Ref->FDM_Internal_Use.AL.BASE_1 ;
	Desc     = (AE_P_DESC_VAR_TYPE*) &Desc_Ptr->Desc[Key];

	/* Get type et taille variable */
	Lg_Var   = (int) Desc->P_Type_Descripteur  ;
	if (( Lg_Var & 0x100 ) == 0 ) {
	     	return  0x81 ; /* type error !!! */
	}


#if						( FDM_WITH_PLX9050 == NO )
{
	Ushort  T;
	Ushort *buffer ;

/* acces 16bits*/
	Lg_Var   = (int) Desc->PDU_Type_Length ;
	Lg_Var	&= 0xff ;
	Lg_Var   = ( Lg_Var / 2) + ( Lg_Var & 1);

	MAC__INIT_REG;

	MAC__ENTREE(Desc);

	MAC__WAIT_DVAR_LIBRE ;

	MAC__ADR_DONNEES(Desc->P_Value_User);

	buffer = (Ushort *) source;
	do {
		T  =  *buffer++ ;
#if 					( FDM_WITH_LITTLE_ENDIAN == YES )
		T = ( T >> 8 ) | ( T << 8 ) ;
#endif
		*Abs_ADR++  =  (Ushort) T   ;
	} while ( --Lg_Var != 0  ) ;
}
#else
    {
/* acces 32bits*/
        Ulong 		T , *Dest, * Src ;
        int i;
	i   = (int) Desc->PDU_Type_Length ;
	i	&= 0xff ;
	Lg_Var   = (( i & 3) == 0) ? 0 : 1;
	Lg_Var  += i / 4 ;

	MAC__INIT_REG;

	MAC__ENTREE(Desc);

	MAC__WAIT_DVAR_LIBRE ;

	MAC__ADR_DONNEES(Desc->P_Value_User);

	Src = (Ulong *) source;
	Dest= (Ulong *) Abs_ADR;

	do {
		T  =  *Src++ ;
#if						(FDM_WITH_NT == YES )
__asm  mov eax , T;
__asm  xchg ah,al;
__asm  ror eax,16;
__asm  xchg ah,al;
__asm  ror eax,16;
__asm  mov T,eax ;
#else
	T = (( T << 8 ) & 0xff00ff00) | ((T >> 8) & 0x00ff00ff);
#endif
		*Dest++  =   T   ;
	} while ( --Lg_Var != 0  ) ;
     }
#endif
	MAC__WAIT_PLACE ;

    	/* valider l'ecriture */
    	Desc->Var_Status = 0x100;

	MAC__ECRIRE_DEMANDE(WRITE_VAR_USER , Key) ;

	MAC__SORTIE(Desc);
	return 0 ;

abortd: OS_Enter_Region() ;
	MAC__SORTIE(Desc);
	return CR ;

#endif
}


	/****************************************
	*  ecriture d'une variable d'heure      *
	*****************************************
	parametres d'entree :
		- cle d'acces
		- adresse buffer source
	******************************************/



Ushort _FIP_WRITE_VAR_TIME ( FDM_REF *Ref ,  Acces_Key Key ,
	Uchar* (*User_Get_Value) ( void  ) , const Ulong *Delta )
{


#if  (  FDM_WITH_FREE_ACCESS == NO )
	int VAR_STATE ;
	int  tempo;
	FDM_ERROR_CODE CR ;
	register unsigned int taille;
	Uchar ZERO[5] ;

    	FIFO_Reg;
	Uchar *src;
	Uchar *srcz;

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );

	/************ ecriture de la commande */
	ECRITE(UCom , CMDE_W_VAR );

	/* ici pour occuper utilement le micro */
	FIFO_port = FIFO;


	/************ attente bit SV */
	WAIT_SV;

	_FIP__tres_petite_tempo(Ref);

	/************ test de valididite de la variable */
        /* Var_state= 81h si non_prod;
                       0  si OK */

	VAR_STATE = LIRE(Var_State) ;
	if ((VAR_STATE & 0x80) != 0 ) {
                ECRITE(UCom,CMDE_CLOSE) ;
                CR_Var_State(VAR_STATE);
                Fin_procedure;
	}

	/************ boucle d'ecriture de l'objet */
	taille = LIRE(Var_Size) ;
	taille -= 5;

	OS_Enter_Region();

	src = User_Get_Value () ;
	BLOCKOUTBYTE(FIFO_port,src,taille);

#if			(   FDM_WITH_LITTLE_ENDIAN == YES )
	{
	  Ulong Ulg;
	  Ulg =  SWAB_ONE_LONG ( *Delta  );
          memcpy (  &ZERO[0] , &Ulg , 4 );
	}
#else
        memcpy (  &ZERO[0] ,  Delta , 4 );
#endif

        ZERO[4] = 0;

	srcz = (Uchar*) &ZERO;
	taille = 5;
	BLOCKOUTBYTE(FIFO_port,srcz,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	OS_Leave_Region();

	WAIT_Busy;

	CR_Var_State( LIRE(Var_State) ) ;
	/* Var_State = 87h si overflow;
		       8Ch si undeflow;
		        0h si OK
	*/
	Fin_procedure;
#else
	/****************************************/
	/*********** Acces libres ***************/
	/********** WRITE_VAR_TIME **************/
	/****************************************/
	volatile Ushort   *FifoUser, *FifoFip  ;
	T_Desc_Ptr *Desc_Ptr ;
	AE_P_DESC_VAR_TYPE * Desc;
	Ulong P_Value_PTR ;
	int Lg_Var;
	register Ulong  T;
	Ushort volatile	*Abs_ADR  ;
	Ushort *buffer ;
	int  tempo;
	Ushort CR ,N,Tmp ;
	Dial_AE *PDial_AE ;

	Desc_Ptr = ( T_Desc_Ptr *) Ref->FDM_Internal_Use.AL.BASE_1 ;
	Desc     = (AE_P_DESC_VAR_TYPE*) &Desc_Ptr->Desc[Key];

	/* Get type et taille variable */
	Lg_Var   = (int) Desc->P_Type_Descripteur  ;
	if (( Lg_Var & 0x100 ) == 0 ) {
	     	return  0x81 ; /* type error !!! */
	}
	Lg_Var   = (int) Desc->PDU_Type_Length ;
	Lg_Var	&= 0xff ;
	Lg_Var  -= 4;
	Lg_Var   = ( Lg_Var / 2)  ;

        MAC__INIT_REG;

	MAC__ENTREE(Desc);

	MAC__ADR_DONNEES(Desc->P_Value_User);

/*	MAC__WAIT_FIFO_VIDE ;*/
	{
	Ushort volatile *X ,Y;
	X = Ref->H.FREE_ACCES_ADDRESS;
	OS_Enter_Region();
	Tmp = *FifoUser;
	Y = (Ushort)((Tmp-2) & 0xef) ;
	X += Y ;
	Tmp = (Ushort)((Tmp+2) & 0xef);
	if ( *X != 0 ){
		tempo = NbLoop;
		while ( *X != 0 ){
			_FIP__petite_tempo(Ref);
			tempo--;
			if ( tempo == 0 )
			{
				FDM_Signal_Warning ( Ref  , _CIRCUIT_ACCES_FAILED , free_Acc_fifo_full  ) ;
				CR = free_Acc_fifo_full;
				goto abort;
			}
		}
	}
	}


	buffer = (Ushort *) User_Get_Value () ;

	do {
		T  =  *buffer++ ;
#if 			( FDM_WITH_LITTLE_ENDIAN == YES )
		T  =  ( T >> 8 ) | ( T << 8 )   ;
#endif
		*Abs_ADR++  =  (Ushort) T  ;
	} while ( --Lg_Var != 0  ) ;

#if			(   FDM_WITH_LITTLE_ENDIAN == YES )
	  T =  SWAB_ONE_LONG ( *Delta  );
#else
	  T =   *Delta  ;
#endif

    	*Abs_ADR++ = (Ushort)(T>>16);
    	*Abs_ADR++ = (Ushort)T ;
    	*Abs_ADR   = 0;

   	/* valider l'ecriture */
    	Desc->Var_Status = 0x100;

	MAC__ECRIRE_DEMANDE( WRITE_VAR_USER , Key );

	MAC__SORTIE(Desc);
	return 0 ;


abort:	MAC__SORTIE(Desc);
	return CR ;


#endif
}


	/****************************************
	*       lecture d'une variable          *
	*****************************************
	parametres d'entree :
	parametres de sortie:
		-variable(si ok)
		-status fip
	****************************************/


Ushort _FIP_READ_VAR( FDM_REF *Ref ,  Acces_Key Key  ,void *destination)
{
#if  (  FDM_WITH_FREE_ACCESS == NO )
	int VAR_STATE ;
	FDM_ERROR_CODE CR ;

	int tempo;

	register unsigned int taille;

    	FIFO_Reg;

	unsigned char *dest;

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );

	/************ ecriture de la commande */
	ECRITE(UCom ,  CMDE_R_VAR);


	/* ici pour occupe utilement le micro */
	   FIFO_port = FIFO;
	   dest = (unsigned char *) destination ;

	/************ attente bit SV */
	WAIT_SV;

	_FIP__tres_petite_tempo(Ref);

	/************ test de valididite de la variable */
	VAR_STATE = LIRE(Var_State) ;
	if ((VAR_STATE & 0x80) != 0 ) {
                ECRITE(UCom,CMDE_CLOSE) ;
                Fin_procedure_no_signal_warning;
	}

	/************ boucle de lecture de l'objet */

	taille = LIRE(Var_Size) ;

	BLOCKINBYTE(FIFO_port,dest,taille);

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	WAIT_Busy;

	Fin_procedure_no_signal_warning;


#else
	/***********************************************/
	/**************** Acces libre ******************/
	/**************** FIP_READ_VAR *****************/
	/***********************************************/

	volatile Ushort   *FifoUser, *FifoFip  ;
	T_Desc_Ptr *Desc_Ptr ;
	AE_C_DESC_VAR_TYPE *DescC;
	int Lg_Var;
	register unsigned  T;
	Ushort volatile *Abs_ADR  ;
	int  tempo;
	Ushort CR ,N ;
	Ushort Tmp;
	Ulong P_Value_PTR ;
	Dial_AE *PDial_AE;

	Desc_Ptr = ( T_Desc_Ptr *) Ref->FDM_Internal_Use.AL.BASE_1 ;
	DescC     = (AE_C_DESC_VAR_TYPE*) &Desc_Ptr->Desc[Key];

	/* Get type et taille variable */
	Lg_Var   = (int)  DescC->C_Type_Descripteur ;
	if (( Lg_Var & 0x200 ) == 0 ) {
	     	return  0x82 ; /* type error !!! */
	}

	MAC__ENTREE(DescC);

	MAC__INIT_REG;

	MAC__WAIT_PLACE;

	MAC__ECRIRE_DEMANDE(READ_VAR_USER,Key);

	MAC__WAIT_DONNEE_PRETE;

	if ( DescC->Var_Status  != 7 ) {
		CR = 0x80;
		/* non valide non ne lit pas !! */
		goto abortd;
	}

	CR = 0x20;

	MAC__ADR_DONNEES(DescC->C_Value_User);


	if (( DescC->C_Var_Type & Var_Type_VLG ) == Var_Type_VLG ){
		Lg_Var   = DescC->PDU_Type_Length ;
	} else {
#if 					( FDM_WITH_LITTLE_ENDIAN == YES )
		T = ( unsigned ) *Abs_ADR ;
		Lg_Var   = ( T >> 8 ) | ( T << 8 )  ;
#else
		Lg_Var = *Abs_ADR;
#endif
	}
#if						( FDM_WITH_PLX9050 == NO )
{
	Ushort *buffer ;
/* AL 16bits */
	buffer = (Ushort *) destination;

	Lg_Var	&= 0xff ;
	Lg_Var   = ( Lg_Var / 2) + ( Lg_Var & 1);

	do {
		T = *Abs_ADR++ ;
#if 					( FDM_WITH_LITTLE_ENDIAN == YES )
		T = ( T >> 8 ) | ( T << 8 ) ;
#endif
		*buffer++  =   T   ;
	} while ( --Lg_Var != 0  ) ;
}
#else
/* AL 32 bits */
	{
	Ulong *Src,*Dst,T;

	Lg_Var	&= 0xff ;
	Lg_Var   = ( Lg_Var / 4) + ((( Lg_Var & 3) == 0)?0:1);

        Dst = (Ulong*) destination;
	Src = (Ulong*) Abs_ADR;
	do {
		T = *Src++ ;
#if 						( FDM_WITH_NT == YES )	
__asm  mov eax , T;
__asm  xchg ah,al;
__asm  ror eax,16;
__asm  xchg ah,al;
__asm  ror eax,16;
__asm  mov T,eax ;
#else
	T = (( T << 8 ) & 0xff00ff00) | ((T >> 8) & 0x00ff00ff);
#endif

		*Dst++  =   T   ;
	} while ( --Lg_Var != 0  ) ;
        }
#endif

abortd:
	OS_Enter_Region();
 	DescC->Var_Status = 0;
	MAC__SORTIE(DescC);
	return CR ;

#endif

}

FDM_MPS_READ_STATUS _FIP_READ_VAR_TIME( FDM_REF *Ref ,  
		Acces_Key Key  , 
		void (*User_Store_Value) (FDM_MPS_VAR_TIME_DATA *),
		FDM_MPS_VAR_TIME_DATA *Destination
	)
{

#if  (  FDM_WITH_FREE_ACCESS == NO )
	int VAR_STATE ;
	Ulong ULG ;

	FDM_ERROR_CODE CR ;

	int tempo;
	Ushort Cr_final;

	unsigned int taille   ;

    	FIFO_Reg;

	Uchar *dest;

	FDM_MPS_READ_STATUS User_CR = { _FDM_MPS_STATUS_FAULT  , 1,1,1,1 };

	Debut_procedure ;

	/************ attente retombe Busy */

	tempo = delai ;
	do {
		tempo--;
		if ( tempo == 0 )  
		{ 
abort1:			ECRITE(UCom,CMDE_CLOSE) ;
			CR.Fdm_Default  = _TIME_OUT;
			OS_fdm_sm_v; 
			Ref->H.User_Signal_Warning( Ref , CR );	
  			return ( User_CR ) ; 	
		}
	} while ((LIRE(UState) & Busy) != 0  ) ;


	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );
	
	/************************************************************************/ 
	OS_Enter_Region();
	/************************************************************************/ 

	/************ ecriture de la commande */
	ECRITE(UCom ,  CMDE_R_VAR_TIME);

	/* ici pour occupe utilement le micro */
	   FIFO_port = FIFO;
	   dest = ( Uchar *) Destination ;
	   *dest++ = 0;
	   *dest++ = 0;
	/************ attente bit SV */
	tempo = delai ;
	do {
		tempo--;
		if ( tempo == 0 )  goto abort1; 	
	} while ((LIRE(UState) & SV ) == 0  ) ;

	_FIP__tres_petite_tempo(Ref);

	/************ test de valididite de la variable */
	VAR_STATE = LIRE(Var_State) ;

	if ((VAR_STATE & 0x80) != 0 ) {
		if (( VAR_STATE  & 0xf ) != 0){
			FDM_Signal_Warning ( Ref ,  _FIPCODE_RESPONSE_IMPOSSIBLE , VAR_STATE );
		}	
		goto Fini;
	}

	User_CR.Non_Significant = 0;
	
	if (( VAR_STATE  & 0x20 ) == 0)	 goto Fini;

	User_CR.Promptness_false = 0;

	/************ boucle de lecture de l'objet */

	taille = LIRE(Var_Size) ;

	BLOCKINBYTE(FIFO_port,dest,taille);
	
	ECRITE(UCom,CMDE_CLOSE) ;


#if			(   FDM_WITH_LITTLE_ENDIAN == YES )
	Destination->Network_Delay = SWAB_ONE_LONG ( Destination->Network_Delay );
#endif
	Destination->Network_Delay  = Destination->Network_Delay * Ref->S.TSlot ;
	if ( Ref->S.TSlot == 62 ) {
		Destination->Network_Delay += (Destination->Network_Delay * 5 )/10;
	}

	switch ( Ref->FDM_Internal_Use.Page_2000.Timer_Cnt & 3 ) {
		case 0: /** 0.4 Ys a 2.5Mhz **/
			Destination->Network_Delay += ((((Ulong)taille*8)+49) * 4 ) /10 ;
			break;
		case 1: /** 1 Ys a 1Mhz **/
			Destination->Network_Delay += (((Ulong)taille*8)+49) * 1 ;
			break;
		case 3: /** 32 Ys a 32.25 Khz **/
			Destination->Network_Delay += (((Ulong)taille*8)+49) * 32 ;
			break;
		}

	dest =  &Destination->FBuffer[ Destination->Pdu_Length - 5 ];
	ULG = *(Ulong *)dest;
#if			(   FDM_WITH_LITTLE_ENDIAN == YES )
	ULG  = SWAB_ONE_LONG ( ULG  );
#endif
 
	Destination->Network_Delay += ULG ;

	/************ compte rendu */
	
	Cr_final = (Ushort ) Destination->FBuffer[ Destination->Pdu_Length - 1 ] ;

	if (( Cr_final  & 0x4 ) == 0)	goto Fini;
		
	User_CR.Signifiance_status_false = 0;

	if ( ( Cr_final  & 0x1 ) == 0 )  goto Fini;

		
	User_CR.Refresment_false = 0 ;

	
	(*User_Store_Value)( Destination );

	User_CR.Report =  _FDM_MPS_READ_OK ;


Fini:   ECRITE(UCom,CMDE_CLOSE) ;
        OS_Leave_Region();
        OS_fdm_sm_v;
	return User_CR;
	
	
#else
	/***********************************************/
	/**************** Acces libre ******************/
	/************ FIP_READ_VAR_TIME ****************/

	volatile Ushort  *FifoUser, *FifoFip ;
	T_Desc_Ptr *Desc_Ptr;
	AE_C_DESC_VAR_TYPE *DescC;
	Ulong P_Value_PTR , ULG;
	int Lg_Var;
	register unsigned  T;
	Ushort volatile *Abs_ADR ;
	Ushort *buffer ;
	int  tempo,taille;
	Ushort CR ,N , Tmp , Cr_final;
	Uchar *dest;
	Dial_AE *PDial_AE;
	
	FDM_MPS_READ_STATUS User_CR = { _FDM_MPS_STATUS_FAULT  , 1,1,1,1 };

	Desc_Ptr = ( T_Desc_Ptr *) Ref->FDM_Internal_Use.AL.BASE_1 ;
	DescC     = (AE_C_DESC_VAR_TYPE*) &Desc_Ptr->Desc[Key];

	/* Get type et taille variable */
	Lg_Var   = (int) DescC->C_Type_Descripteur  ;
	if (( Lg_Var & 0x200 ) == 0 ) {
	     	return  User_CR ; /* type error !!! */
	}

	MAC__ENTREE(DescC);

	MAC__INIT_REG;

	MAC__WAIT_PLACE;

	MAC__ECRIRE_DEMANDE(READ_VAR_TIME_USER,Key);

	/*** ~~= MAC__WAIT_DONNEE_PRETE ***/
	tempo = NbLoop;
	do {	
		_FIP__petite_tempo(Ref);
		tempo --;
		if ( tempo == 0 ) {
			OS_Leave_Region();
			FDM_Signal_Warning ( Ref  , _CIRCUIT_ACCES_FAILED , free_Acc_time_out  ) ;
			CR = free_Acc_time_out ;
			goto abortd;	
		}
	} while (   (DescC->Var_Status & 1)  == 0 ) ;


	if ( DescC->Var_Status  != 7 ) {
		goto abort;
	}


	/* lecture */
	MAC__ADR_DONNEES(DescC->C_Value_User);

	buffer = (Ushort *) &Destination->Pdu_Type;;

	Lg_Var   = DescC->PDU_Type_Length ;
	Lg_Var	&= 0xff ;
	taille = Lg_Var ;
	Lg_Var   = ( Lg_Var / 2) + 1; /* lg impair ! */


	do {
		T = *Abs_ADR++ ;
#if 					( FDM_WITH_LITTLE_ENDIAN == YES )
		T = ( T >> 8 ) | ( T << 8 ) ;
#endif
		*buffer++  =   T   ;
	} while ( --Lg_Var != 0  ) ;


	Destination->Network_Delay  = DescC->Reception_Delay * Ref->S.TSlot ;
	if ( Ref->S.TSlot == 62 ) {
		Destination->Network_Delay += (DescC->Reception_Delay * 5 )/10;
	}
	Destination->Network_Delay += DescC->Trans_Delai;
	
	switch ( Ref->FDM_Internal_Use.Page_2000.Timer_Cnt & 3 ) {
		case 0: /** 0.4 Ys a 2.5Mhz **/
			Destination->Network_Delay += ((((Ulong)taille*8)+49) * 4 ) /10 ;
			break;
		case 1: /** 1 Ys a 1Mhz **/
			Destination->Network_Delay += (((Ulong)taille*8)+49) * 1 ;
			break;
		case 3: /** 32 Ys a 32.25 Khz **/
			Destination->Network_Delay += (((Ulong)taille*8)+49) * 32 ;
			break;
		}

	dest =  &Destination->FBuffer[ Destination->Pdu_Length - 5 ];
	ULG = *(Ulong *)dest;

#if			(   FDM_WITH_LITTLE_ENDIAN == YES )
	ULG  = SWAB_ONE_LONG ( ULG  );
#endif

	Destination->Network_Delay += ULG ;

	/************ compte rendu */
	
	Cr_final = (Ushort ) Destination->FBuffer[ Destination->Pdu_Length - 1 ] ;

	if (( Cr_final  & 0x4 ) == 0)	goto abort;
		
	User_CR.Signifiance_status_false = 0;

	if ( ( Cr_final  & 0x1 ) == 0 )  goto abort;
	
	User_CR.Refresment_false = 0 ;
	
	(*User_Store_Value)( Destination );

	User_CR.Report =  _FDM_MPS_READ_OK ;

abort:
 	DescC->Var_Status = 0;
	MAC__SORTIE(DescC);
	return User_CR ;

abortd:
	OS_Enter_Region();
 	DescC->Var_Status = 0;
	MAC__SORTIE(DescC);
	return User_CR ;

#endif

}



#if 				( FDM_WITH_APER == YES )


static Ushort _FIP_APER_FLUSHOUT ( _FDM_XAE_Tempo_Ref *T , Uchar Nature_FILE )
{

	FDM_REF *Ref	;

#if  (  FDM_WITH_FREE_ACCESS == NO )

	int tempo ;
	FDM_ERROR_CODE CR ;

	Ref = T->Fip;
	Debut_procedure ;

	WAIT_Busy;

	ECRITE( USwitch ,  Nature_FILE );

	ECRITE( UCom , CMDE_PURGE_APER );

	/************ attente bit SV */
	WAIT_SV;

	/************ compte rendu */
	CR_UState;

	/************ close */
    ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;
#else
	volatile Ushort  *FifoUser, *FifoFip ;
	int  tempo;
	Ushort Tmp ,CR  ;
	Dial_AE *PDial_AE;

	CR = 0;

	Ref = T->Fip;

	MAC__INIT_REG;

	MAC__WAIT_PLACE;

	MAC__ECRIRE_DEMANDE( PURGE_APER_USER ,Nature_FILE);

	OS_Leave_Region();
abortd:
	return CR;
#endif
}

Ushort _FIP_APER_FLUSHOUT_N ( _FDM_XAE_Tempo_Ref *T)
{
	return ( _FIP_APER_FLUSHOUT( T,0) );
}
Ushort _FIP_APER_FLUSHOUT_U ( _FDM_XAE_Tempo_Ref *T)
{
	return ( _FIP_APER_FLUSHOUT( T,1) );

}


Ushort _FIP_UPDATE          ( FDM_REF *Ref , Acces_Key Key , short Prio )
{
#if  (  FDM_WITH_FREE_ACCESS == NO )
	int tempo ;
	FDM_ERROR_CODE CR ;

	Debut_procedure ;

	WAIT_Busy;

	/************ ecriture de la cle d'acces */
	ECRIRE_Key( Key );
	/************ ecriture de la commande */
	if ( Prio == 0 )
		ECRITE(UCom ,  CMDE_SEND_APN );
	else
		ECRITE(UCom , CMDE_SEND_APU  );

	/************ attente bit SV */
	WAIT_SV;

	/************ compte rendu */
	CR_UState;

	/************ close */
        ECRITE(UCom,CMDE_CLOSE) ;

	Fin_procedure;
#else
	/***********************************************/
	/**************** Acces libre ******************/
	/**************** FIP_UPDATE   *****************/
	/***********************************************/
	volatile Ushort   *FifoUser, *FifoFip ;
	Dial_AE *PDial_AE;
	int  tempo;
	Ushort CR ;
	Ushort Tmp ;


	CR = 0;

	MAC__INIT_REG;

	MAC__WAIT_PLACE;

	if ( Prio == 0 ){
		MAC__ECRIRE_DEMANDE(SEND_APER_N_USER,Key);
	} else {
		MAC__ECRIRE_DEMANDE(SEND_APER_U_USER,Key);
	}


	OS_Leave_Region();
abortd:
	return CR ;

#endif
}

#endif


Ushort _FIP_DEF_EVT  ( FDM_REF *Ref  ,Acces_Key Key, Ushort  Event_Definition )
{

    	Absolute_ADR Adr ;

	Adr = Ref->FDM_Internal_Use.fipgen.BASE_1 +(Key << 4) + 8;
	return ( _FIP_WRITE_WORD  ( Ref   , Adr , Event_Definition   ));

}
