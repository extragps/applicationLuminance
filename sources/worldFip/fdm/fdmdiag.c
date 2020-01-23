/************************************************************
" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD


      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmdiag.c
  Description     : Diagnostiques de defaut
                    tests off line et on line


  Date de creation: Fri Mar 15 15:11:26 MET 1996
  Historique des evolutions/modifications
  	Modification : 96.07.26
  	Reference Ra : RA xx  (test ram lors essais acces libres -motorola-)
        But de la modification :
  	correction procedure _FIP_RAM_TEST:
  		sur SWAB_SHORT ajouter option de compilation
  		#if (FDM_WITH_LITTLE_ENDIAN == YES )

Version:4.1.1
	a/- suppression de Warning compilateur GNU

Version:4.2.1
	a/- mauvaise signalisation de _DIAG_FAIL_ON_INTERNAL_TIMERS
	suppression de code inutile
	b/- fdm_online_test renomage conforme aus specifs

Version:4.3.12
	a/- test FIPCODE page 0

Version:4.3.18
	a/- test SV entre CMDE_VALID et CMDE_VALID

***********************************************************/
#include "user_opt.h"

#include "fdm.h"
#include "fdmmacro.h"


#define FIP_TIMER_CNT_TEST   0x40





/*======================================================*/
/* _FIP_READ_PHY_BYTE_TEST - particulier aux tests     */
/*======================================================*/

Ushort _FIP_READ_PHY_BYTE_TEST ( FDM_REF * Ref ,
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

   	OS_fdm_sm_v;

 	return ((Ushort)CR.Information.Fipcode_Report._Ustate);

}



/*=====================================================================*/
/*			procedures its EOC/IRQ test                    */
/*   flags testes dans FIP_DIAG_OFF_LINE 			       */
/*=====================================================================*/
#if 				( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == NO )


void fdm_process_it_eoc_test(FDM_REF * Ref)
{
    if (Ref->S.Type & EOC_PULSE_MODE )
  	  Ref->FDM_Internal_Use.fipdiag.Flag_it_eoc =1;
    else {
       if ( (LIRE(UState) & EOC) == 0)  /* EOC */
  	  Ref->FDM_Internal_Use.fipdiag.Flag_it_eoc =1;
    }
}


void fdm_process_it_irq_test(FDM_REF * Ref)
{
    if ( (LIRE(UState) & IRQ) != 0)
	Ref->FDM_Internal_Use.fipdiag.Flag_it_irq =1;
}

#else

void fdm_process_its_fip_test (  FDM_REF *Ref)
{
       if ( (LIRE(UState) & EOC) == 0)
  	  Ref->FDM_Internal_Use.fipdiag.Flag_it_eoc =1;

      if ( (LIRE(UState) & IRQ) != 0)
	Ref->FDM_Internal_Use.fipdiag.Flag_it_irq =1;
}

#endif


/*=====================================================================*/
/*			TESTS OFF LINE                                 */
/*=====================================================================*/

/* ---------------------------------------------------------*/
/*  test ram privee :	 _FIP_RAM_TEST   		    */
/* ---------------------------------------------------------*/
Ushort _FIP_RAM_TEST (FDM_REF *Ref)
{
	int    i;
	long  adr_ram;
	Ushort cr;
	Ushort buf[64];
	Ushort buf_lu[64];
	Ushort data , nodata ;

Ref->FDM_Internal_Use.fipdiag.type_test   = NO_TEST_RAM;


#if		( FDM_WITH_FREE_ACCESS == YES)
 /* ecriture 1ere page              - acces libre */
 /*-----------------------------------------------*/
  data   = 0 ;
  nodata = 0 ;
  adr_ram = 0 ;
  for (i =0; i < sizeof(buf)/2;i++){
     	if (data==nodata)  data++;
     	buf[i] = data ;
     	if (++data ==0 ) nodata++;
   }
   _FIP_WRITE_PHY_BYTE( Ref, adr_ram,(Uchar *)buf,128);

  /* lecture 1ere  page   - acces classique - test */
 /*-----------------------------------------------------*/

   _FIP_READ_PHY ( Ref, adr_ram,buf_lu,128);
#if  ( FDM_WITH_LITTLE_ENDIAN ==YES)
    SWAB_SHORT ( (char*)buf_lu ,(char *)buf_lu ,128);
#endif
   /* comparaison */
   /*-------------*/
   if ( memcmp ( buf , buf_lu, sizeof(buf)) ){
         FDM_Signal_Error (  Ref , _TEST_RAM_FAIL , adr_ram);
         return ( FDM_NOK ) ;
   }

   /* ecriture derniere page  - acces classique */
 /*--------------------------------------------------*/
  data   = 0 ;
  nodata = 0 ;
  adr_ram = Ref->S.FULLFIP_RAM_Dim -64 ;  /*derniere page */
  for (i =0; i < sizeof(buf)/2 ;i++){
     	if (data==nodata)  data++;
     	buf[i] = data ;
    	if (++data ==0 ) nodata++;
  }
#if  ( FDM_WITH_LITTLE_ENDIAN ==YES)
   SWAB_SHORT ( (char *)buf ,(char *)buf ,128);
#endif
  cr  = _FIP_WRITE_PHY_BYTE ( Ref, adr_ram,(Uchar *)buf,128);
  if (cr!= 0 ) {
         FDM_Signal_Error (  Ref ,  _CIRCUIT_ACCES_FAILED , Add_Repport_Diag );
         return ( FDM_NOK ) ;
  }

  /* lecture  derniere page  - acces libre- test */
 /*-----------------------------------------------------*/

  cr  = _FIP_READ_PHY ( Ref, adr_ram,buf_lu,128);
  if (cr!= 0 ) {
         FDM_Signal_Error (  Ref , _CIRCUIT_ACCES_FAILED , Add_Repport_Diag );
         return ( FDM_NOK ) ;
  }
  /* comparaison */
     /*-------------*/
#if  ( FDM_WITH_LITTLE_ENDIAN ==YES)
  SWAB_SHORT ( (char*)buf_lu ,(char *)buf_lu ,128);
#endif
 if ( memcmp ( buf , buf_lu, sizeof(buf)) ){
         FDM_Signal_Error (  Ref , _TEST_RAM_FAIL , adr_ram);
         return ( FDM_NOK ) ;
  }

#endif


 /* ecriture du volume en mode STANDART -sens croissant */
 /*-----------------------------------------------------*/
 for ( data   = 0 , nodata = 0 , adr_ram = 0 ;
       adr_ram <(long) Ref->S.FULLFIP_RAM_Dim; adr_ram += 64 ){
     for (i =0; i < sizeof(buf)/sizeof(Ushort) ;i++){
     	if (data==nodata)  data++;
     	buf[i] = data ;
     	if (++data ==0 ) nodata++;
     }
     cr  = _FIP_WRITE_PHY( Ref, adr_ram,buf,128);
     if (cr!= 0 ) {
         FDM_Signal_Error (  Ref , _CIRCUIT_ACCES_FAILED , Add_Repport_Diag );
         return ( FDM_NOK ) ;
     }
  }
 /* lecture - test du volume en mode STANDART           */
 /*-----------------------------------------------------*/
  for ( data   = 0 , nodata = 0 , adr_ram = 0 ;
       adr_ram <(long) Ref->S.FULLFIP_RAM_Dim; adr_ram += 64 ){
     for (i =0; i < sizeof(buf)/sizeof(Ushort) ;i++){
     	if (data==nodata)  data++;
     	buf[i] = data ;
     	if (++data ==0 ) nodata++;
     }
     cr  = _FIP_READ_PHY( Ref, adr_ram,buf_lu,128);
     if (cr!= 0 ) {
         FDM_Signal_Error (  Ref , _CIRCUIT_ACCES_FAILED , Add_Repport_Diag );
         return ( FDM_NOK ) ;
     }
     /* comparaison */
     /*-------------*/
     if ( memcmp ( buf , buf_lu, sizeof(buf)) ){
         FDM_Signal_Error (  Ref , _TEST_RAM_FAIL  , adr_ram);
         return ( FDM_NOK ) ;
     }

  }


 /* ecriture du volume en mode STANDART -sens pages decroissantes */
 /*---------------------------------------------------------------*/

 adr_ram = Ref->S.FULLFIP_RAM_Dim - 64;
 for ( data   = 0 , nodata = 0  ;  0 <= adr_ram ; adr_ram -= 64 ){
     for ( i = (sizeof(buf)/sizeof(Ushort))-1; 0 <= i ; --i){
     	if (data==nodata)  data++;
     	buf[i] = data ;
     	if (++data ==0 ) nodata++;
     }

     cr  = _FIP_WRITE_PHY( Ref, adr_ram,buf,128);
     if (cr!= 0 ) {
         FDM_Signal_Error (  Ref , _CIRCUIT_ACCES_FAILED , Add_Repport_Diag );
         return ( FDM_NOK ) ;
     }
  }
 /* lecture - test du volume en mode STANDART-sens pages decroissantes*/
 /*-------------------------------------------------------------------*/
 adr_ram = Ref->S.FULLFIP_RAM_Dim - 64;
 for ( data   = 0 , nodata = 0  ;  0 <= adr_ram ; adr_ram -= 64 ){
     for ( i = (sizeof(buf)/sizeof(Ushort))-1; 0<= i ; --i){
     	if (data==nodata)  data++;
     	buf[i] = data ;
     	if (++data ==0 ) nodata++;
     }
     cr  = _FIP_READ_PHY( Ref, adr_ram,buf_lu,128);
     if (cr!= 0 ) {
         FDM_Signal_Error (  Ref , _CIRCUIT_ACCES_FAILED , Add_Repport_Diag );
         return ( FDM_NOK ) ;
     }
     /* comparaison */
     /*-------------*/
     if ( memcmp ( buf , buf_lu, sizeof(buf)) ){
         FDM_Signal_Error (  Ref , _TEST_RAM_FAIL , adr_ram);
         return ( FDM_NOK ) ;
     }
  }

  return (FDM_OK);
}


/* ---------------------------------------------------------*/
/*  test fipcode en ram :	 _FIP_TEST_PRG		    */
/* ---------------------------------------------------------*/
/* 	Nombre de pages donne dans Fipcode6.h : Nb_Pages
 pour toutes les pages :
	dans FDM_Intenal_Use.tab_BC on a ;
		-base de la page a lire
		-checksum de la page
	->on lit lecture de la page en fonction de la base
	->calcul du checksum
	->controle avec checksum en table tab_BC

 NOTA : dans la base =0 , on ne controle pas les 48 1er octets
*/

Ushort _FIP_TEST_PRG( FDM_REF *Ref )
{

  unsigned int base;
  int i,j,Nb_Page_Fc;
  unsigned char TAMPON[128];
  unsigned short checksum ;
  Ushort Cr;

Ref->FDM_Internal_Use.fipdiag.type_test   = NO_TEST_PRG;
Nb_Page_Fc =  Ref->FDM_Internal_Use.Nb_Page_Vrai;

for (i = 0 ; i < Nb_Page_Fc ; i++ ){
     base = 	Ref->FDM_Internal_Use.tab_BC[i].base;
     Cr = _FIP_READ_PHY(Ref,(long)base,(Ushort*)&TAMPON[0],128);
     if (Cr != 0 ){
	 FDM_Signal_Error (  Ref , _FIPCODE_FAIL  ,(Ulong) i);
         return FDM_NOK;
     }
#if ( FDM_WITH_LITTLE_ENDIAN  == YES)
	 SWAB_SHORT ((char*)TAMPON,(char*)TAMPON,128);
#endif
     if (base == 0)       j=48;
     else		  j=0;
     for (checksum=0 ; j < 128; j++){
     	checksum +=(unsigned short)(TAMPON[j]) ;
     }
     if (checksum !=Ref->FDM_Internal_Use.tab_BC[i].checksum){
         FDM_Signal_Error (  Ref , _FIPCODE_FAIL ,(Ulong) i);
         return FDM_NOK;
     }
  }
 return FDM_OK;
}



/*======================================================*/
/* complement de fdmmacro.h - particulier aux tests     */
/*======================================================*/

#define CR_time_out_TEST\
	ECRITE(UCom,CMDE_CLOSE) ;					\
	CR.Fdm_Default  = _TIME_OUT;					\
 CR.Information.Additional_Report =Ref->FDM_Internal_Use.fipdiag.test_number;\
 	Ref->H.User_Signal_Fatal_Error( Ref , CR );			\
  	return (FDM_NOK) ;


#define WAIT_BUSY_TEST \
	tempo = delai ;							\
	do {								\
		tempo--;						\
		if ( tempo == 0 )  { CR_time_out_TEST ; }		\
	} while ((LIRE(UState) & Busy) != 0  ) ;			\

#define WAIT_SV_TEST \
	tempo = delai ;							\
	do {								\
		tempo--;						\
		if ( tempo == 0 )  { CR_time_out_TEST ; }		\
	} while ((LIRE(UState) & SV) == 0  ) ;				\



#define WAIT_BUSY_SV_TEST \
 tempo=delai;						        \
 do {								\
	tempo--;						\
	if ( tempo == 0 )  { CR_time_out_TEST ; }		\
 } while ((LIRE(UState) & (SV |Busy)) !=  (SV |Busy)) ;		\
 CR.Information.Fipcode_Report._Var_State =LIRE(Var_State);	\
 if((CR.Information.Fipcode_Report._Var_State & 0x80 )!= 0 ) {	\
   ECRITE(UCom, CMDE_CLOSE);					\
   CR.Fdm_Default  = _DIAG_FAIL_ON_SV_PIN ;			\
   CR.Information.Additional_Report =Ref->FDM_Internal_Use.fipdiag.test_number;\
   Ref->H.User_Signal_Fatal_Error( Ref, CR );			\
   return ( FDM_NOK) ; 						\
 }								\


#define WAIT_FR_TEST \
 while ( (LIRE(UState) & FR ) == 0 ) {}   \



#if		( FDM_WITH_DIAG == YES)

static Ushort TEST_REGS(FDM_REF *, Acces_Key );
static Ushort _FIP_CODE_TEST ( FDM_REF *Ref );
static Ushort _FIP_CTRL_RAM (FDM_REF *Ref);
/* procedure de substitution de Ref->H.User_Signal_Warning pour ne pas appeler
   procedure user */

static void pas_warning(struct _FDM_REF *Ref , FDM_ERROR_CODE err) {}


/* ---------------------------------------------------------*/
/*  test composant fullfip :	 _FIP_DIAG_OFF_LINE   	    */
/* ---------------------------------------------------------*/


Ushort _FIP_DIAG_OFF_LINE (FDM_REF *Ref)
{
    unsigned int taille ,count ;
    int tempo ;
    unsigned long Ltempo;
    FDM_ERROR_CODE CR ;
    Ushort cr_proc;
    FIFO_Reg;
    void (*proc_user_Signal_Warning)(struct _FDM_REF *,FDM_ERROR_CODE);
    unsigned char    User_Buffer[129];
    unsigned char   *User_Buffer_R;
    unsigned short   nb_count ;
    Acces_Key  	    signature;


Ref->FDM_Internal_Use.fipdiag.type_test   = NO_TEST_COMPOSANT;
Ref->FDM_Internal_Use.fipdiag.Flag_it_eoc =0;
Ref->FDM_Internal_Use.fipdiag.Flag_it_irq =0;

	/* sauvegarde procedure warning user, modification pour eviter d'appeler
           warning sur time_out dans la procedure FIP_READ_PHY */
proc_user_Signal_Warning = Ref->H.User_Signal_Warning ;
Ref->H.User_Signal_Warning = pas_warning;

 /*on commence par regarder si FULLFIP est pret (gestion time out sur BUSY)*/

 Ref->FDM_Internal_Use.fipdiag.test_number  = NO_TEST_FULLFIP_PRET;
 CR.Fdm_Default = _NO_ERROR;
 CR.Information.Additional_Report = 0;

 WAIT_BUSY_TEST;

    /* fullfip pret , on teste le bit IC  */
   /*-------------------------------------*/
 Ref->FDM_Internal_Use.fipdiag.test_number  = NO_TEST_IC;
 ECRITE(UCom,CMDE_CLOSE);
 CR_UState;

/* la lecture de USTATE efface le bit IC*/
 if ((CR.Information.Fipcode_Report._Ustate & IC )==0){
   	CR.Fdm_Default  = _DIAG_FAIL_ON_IC_PIN;
	goto aborting_test ;
 }

 /*Test du bit FE. */
 /*----------------*/
   /*La fonction FIP_READ_PHY assure la gestion du time out.
     On lit plus de 128 octets. */

 Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_FE;
 cr_proc = _FIP_READ_PHY_BYTE_TEST(Ref, 0, User_Buffer,sizeof( User_Buffer));
  if (cr_proc == 0x8000 ) {
   	CR.Fdm_Default  = _TIME_OUT;
        goto aborting_test;
  }
  if ((cr_proc & (AE | IC) )!= 0 ) {
   	CR.Fdm_Default  = _FIPCODE_RESPONSE_IMPOSSIBLE;
        goto aborting_test;
  }
  if ((cr_proc & FE ) == 0 ) {     /*bit FE non monte */
   	CR.Fdm_Default  = _DIAG_FAIL_ON_FE_PIN;
	goto aborting_test ;
  }

 /* Test du bit AE.*/
{
	volatile int V;
 	CR.Information.Additional_Report = 0;
 	Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_AE;

   	/* On lit un registre non defini en lecture */
 	V = LIRE(UNKNOWN);
 	V = LIRE(UState);
	if (( V & AE ) == 0 ) {
		CR.Fdm_Default  = _DIAG_FAIL_ON_AE_PIN;
    		goto aborting_test ;
 	}
 }

/* Test du bit SV.*/
 /*----------------*/
 /* On commence par tester si FULLFIP est pret (time out sur BUSY)*/

  CR.Information.Additional_Report = 0;
  Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_SV;

  WAIT_BUSY_TEST;
  /* FULLFIP etant pret, on lit la memoire privee du FULLFIP.
     Si la fonction FIP_READ_PHY signale un time out, il est forcement du a SV
     (car BUSY precedemment filtre) */

    cr_proc = _FIP_READ_PHY_BYTE (Ref, 0, User_Buffer,2);
    /* si cr =0x8000 on a time out du a SV
          cr = ustate  */
    switch (cr_proc){
     case 0:	/*OK*/
     break;
     case 0x8000 :
	CR.Fdm_Default  = _DIAG_FAIL_ON_SV_PIN;
        goto aborting_test ;
     default :
	 CR.Fdm_Default  = _FIPCODE_RESPONSE_IMPOSSIBLE;
         goto aborting_test;
    }

     /* repositionne  procedure warning user*/
 Ref->H.User_Signal_Warning =proc_user_Signal_Warning;


/* On appelle maintenant les fonctions du deuxieme groupe, celles
   necessitant l'emploi d'un FIPCODE specifique de test.*/

	/* prepare test fifo.*/
	/*-------------------*/
	/* On commence par tester si FULLFIP est pret (time out sur BUSY).*/

 Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_PREPAR_WRITE_FIFO;
 CR.Information.Additional_Report = 0;
 WAIT_BUSY_TEST ;

	/* Test write fifo */
 	/*----------------*/

 Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_WRITE_FIFO;

 ECRITE(UCom,CMDE_TST_WFIFO);
 WAIT_BUSY_SV_TEST;

 taille = LIRE (Var_Size);
 if (taille !=8 ) {
	CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_FIFO ;
    	goto abort_test_with_close ;
 }
    /* taille ok , attente montee du bit FR*/
  count =0;
  do {
       if (++count >8 ) break;
       ECRITE (FILE_FIP,0);
  }while ((LIRE (UState) & FR) !=0); /*bit FR mis a 0 a la 8ieme ecriture */

  if (count != taille){
	CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_FIFO;
     	goto abort_test_with_close ;
 }


  ECRITE(UCom, CMDE_CLOSE); /* fermeture session fifo en ecriture */

 	/* Test read fifo */
 	/*----------------*/
 Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_PREPAR_READ_FIFO;
 CR.Information.Additional_Report = 0;

 User_Buffer[0] = 0xa5;
 User_Buffer[1] = 0x5a;

 cr_proc= _FIP_WRITE_PHY_BYTE(Ref, FIP_TIMER_CNT_TEST, User_Buffer,2);
 if (cr_proc !=0){
	CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_FIFO;
     	goto aborting_test;
 }
 WAIT_BUSY_TEST ;

 Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_READ_FIFO;
 CR.Information.Additional_Report = 0;

 ECRITE(UCom,CMDE_TST_RFIFO);
 WAIT_BUSY_SV_TEST;
 taille = LIRE (Var_Size);
 if (taille != 2 ) {
	CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_FIFO;
   	goto abort_test_with_close;
  }
 WAIT_FR_TEST;
   /* lecture de la fifo */

 FIFO_port =FIFO ;
 User_Buffer_R = &User_Buffer[0];

 BLOCKINBYTE (FIFO_port,User_Buffer_R,taille);

 CR.Information.Fipcode_Report._Ustate = LIRE(UState);
 if ( ( (CR.Information.Fipcode_Report._Ustate & FR) !=0 ) ||
       ( User_Buffer[0] != 0xa5) || ( User_Buffer[1] != 0x5a) ) {
	CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_FIFO;
      	goto abort_test_with_close;
 }
 ECRITE(UCom, CMDE_CLOSE); /* fermeture session fifo en lecture */


if (Ref->S.Type & IRQ_CONNECTED ){

	/* test IRQ.         */
	/*-------------------*/
    Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_IRQ;
   CR.Information.Additional_Report = 0;
 	/* test du flag_it ---> on ne doit pas avoir eu it */

   if (	Ref->FDM_Internal_Use.fipdiag.Flag_it_irq !=0 ){
	CR.Fdm_Default  = _DIAG_FAIL_ON_IRQ_PIN ;
      	goto aborting_test;
   }
    WAIT_BUSY_TEST ;
   ECRITE(UCom,CMDE_TST_IRQ);
   WAIT_BUSY_SV_TEST;
   /* attente traitement de l'it , Flag_it_irq=1 sur montee de IRQ */
   tempo = delai ;
	do {

	} while ((--tempo != 0) &&
                 (Ref->FDM_Internal_Use.fipdiag.Flag_it_irq == 0)) ;
    ECRITE(UCom, CMDE_CLOSE);

   if (tempo ==0){
	CR.Fdm_Default  = _DIAG_FAIL_ON_IRQ_PIN ;
     	goto abort_test_with_close;
   }
   Ref->FDM_Internal_Use.fipdiag.Flag_it_irq =0;
 	/* attente descente l'it de IRQ */
   tempo = delai ;
	do {

	} while ((--tempo != 0) && ( (LIRE(UState) & IRQ) != 0) );
   if (tempo ==0){
	CR.Fdm_Default  = _DIAG_FAIL_ON_IRQ_PIN ;
      	goto abort_test_with_close;
   }
 }


	/* test des registres */
	/*-------------------*/
 Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_REGS;
 CR.Information.Additional_Report = 0;
 signature =0xa55a;
 if (TEST_REGS(Ref, signature) != 0){
	CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_REGISTERS ;
        goto aborting_test;
 }
 signature =0x5aa5;
 if (TEST_REGS(Ref, signature) != 0) {
	CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_REGISTERS ;
        goto aborting_test;
 }

	/* test It EOC et TIMER */
	/*----------------------*/
 Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_PREPARE_EOC_TIMER;
 CR.Information.Additional_Report = 0;
 User_Buffer[0] = 0;
 User_Buffer[1] = 0;

 cr_proc= _FIP_WRITE_PHY_BYTE(Ref, FIP_TIMER_CNT_TEST, User_Buffer,2);
 if (cr_proc !=0){
	CR.Fdm_Default  = _FIPCODE_RESPONSE_IMPOSSIBLE ;
        goto aborting_test;
 }
 if (Ref->S.Type & EOC_CONNECTED ){
#if 				( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == NO )
 	if ( (Ref->S.Type & EOC_PULSE_MODE ) == 0 ) {
   		/* test du flag_it_eoc ---> on ne doit pas avoir eu  it */
   		if (	Ref->FDM_Internal_Use.fipdiag.Flag_it_eoc !=0 ){
			CR.Fdm_Default  = _DIAG_FAIL_ON_EOC_PIN ;
       		 	goto aborting_test;
   		}
 	}
#else
   	/* test du flag_it_eoc ---> on ne doit pas avoir eu it */
   	if (	Ref->FDM_Internal_Use.fipdiag.Flag_it_eoc != 0 ){
		CR.Fdm_Default  = _DIAG_FAIL_ON_EOC_PIN ;
       			goto aborting_test;
   	}
#endif
  }
 Ref->FDM_Internal_Use.fipdiag.Flag_it_eoc = 0;
 Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_TIMER;
 CR.Information.Additional_Report = 0;

 WAIT_BUSY_TEST ;
 ECRITE(UCom,CMDE_TST_TIME);

 Ltempo = 0xffffffffL;
 do {
 	_FIP__tres_petite_tempo(Ref);
	Ltempo--;
	if ( Ltempo == 0 )  { CR_time_out_TEST ; }
 } while ((LIRE(UState) & (SV |Busy)) !=  (SV |Busy)) ;

 _FIP__tres_petite_tempo(Ref);

 CR.Information.Fipcode_Report._Var_State =LIRE(Var_State);
 if((CR.Information.Fipcode_Report._Var_State & 0x80 )!= 0 ) {
   CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_TIMERS ;
   goto abort_test_with_close;
 }
	/* attente fifo prete */
 WAIT_FR_TEST;
 taille = LIRE (Var_Size);
 if (taille != 2 ) {
	CR.Fdm_Default  = _FIPCODE_RESPONSE_IMPOSSIBLE ;
    	goto abort_test_with_close;
  }
       /* lecture du nombre de boucles dans la fifo
          doit etre egal a 8 TSLOT non ecoules ,soit :
        X=  (8*TSLOT) / (4*duree_1_instruction )
	 avec duree 1 instruction = 5 * (1/quartz) ==>
        X= 1600 quelque soit TSLOT --> test a 1600 +_ 10  */

 FIFO_port =FIFO ;
#if  ( FDM_WITH_LITTLE_ENDIAN ==YES)
   User_Buffer_R = &User_Buffer[0];
   BLOCKINBYTE (FIFO_port,User_Buffer_R,taille);
   SWAB_SHORT((char*)User_Buffer,(char*)&nb_count ,2);
#else
   User_Buffer_R = (unsigned char *)&nb_count;
   BLOCKINBYTE (FIFO_port,User_Buffer_R,taille);
#endif
if (nb_count < 1590 || nb_count > 1610  ){
   CR.Fdm_Default  = _DIAG_FAIL_ON_INTERNAL_TIMERS ;
   goto abort_test_with_close;
 }

 if (Ref->S.Type & EOC_CONNECTED ){

   Ref->FDM_Internal_Use.fipdiag.test_number   = NO_TEST_EOC;
   CR.Information.Additional_Report = 0;

 	 /* test si on a eu l'it EOC, Flag_it_eoc=1 */
    if (Ref->FDM_Internal_Use.fipdiag.Flag_it_eoc != 1 ){
	CR.Fdm_Default  = _DIAG_FAIL_ON_EOC_PIN ;
         goto abort_test_with_close;
    }

   /* attente descente l'it de EOC */
    tempo = delai ;
	do {
#if 				( FDM_WITH_IRQ_EOC_ON_SAME_INTERRUPT == NO )
	    	if ( Ref->S.Type & EOC_PULSE_MODE )
	   		if ( (LIRE(UState) & EOC) != 0)  break;
		else
#endif
	   		if ( (LIRE(UState) & EOC) == 0)  break;
	} while (--tempo != 0);
    if ( tempo  == 0) {
    	CR.Fdm_Default  = _DIAG_FAIL_ON_EOC_PIN ;
        goto abort_test_with_close;
    }
 }
 ECRITE(UCom, CMDE_CLOSE);

	/* fin de test  */
	/*----------------------*/
 CR.Information.Additional_Report = 0;


 return (FDM_OK);

abort_test_with_close:
   ECRITE(UCom, CMDE_CLOSE);

aborting_test:
   CR.Information.Additional_Report =Ref->FDM_Internal_Use.fipdiag.test_number;
   Ref->H.User_Signal_Fatal_Error( Ref , CR );
   return ( FDM_NOK) ;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*	procedure interne de test des registres fullfip                */
/* Fipcode : ce test lit les 2 octets du registre KEY
             les ecrit en ram externe
	      dans la fifo user
	      dans les registres UFLAGS (KEY_L) et VAR_STATE (KEY_H)   */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static Ushort TEST_REGS(FDM_REF *Ref, Acces_Key signature)
{
      FIFO_Reg;
      int 		tempo;
      unsigned int 	taille ;
      unsigned char	 *dest;
      unsigned char	 buffer_r[2];
      unsigned char      h_signature ;
      unsigned char      l_signature ;
      int		 VAR_STATE , U_FLAGS;
    FDM_ERROR_CODE CR ;



   h_signature =(unsigned char) (signature >> 8 );
   l_signature =(unsigned char) signature ;

	/* test si fullfip pret */
   WAIT_BUSY_TEST;
   ECRIRE_Key(signature);		/*ouverture session */
   ECRITE(UCom,CMDE_TST_REG);


   tempo = delai ;
   do {
	tempo--;
	if ( tempo == 0 )  { CR_time_out_TEST ; }
   } while ((LIRE(UState) & (SV |Busy)) !=  (SV |Busy)) ;

	/* verifie var_size */
   taille = LIRE (Var_Size);
   if (taille != 2 )  {
	goto abort_test_registres;
   }
        /* attente fifo prete */
   WAIT_FR_TEST;
   tempo=200;
   do { --tempo ;
      }while (tempo !=0);

	/* lecture fifo */
   dest = &buffer_r[0];
   FIFO_port =FIFO ;
   BLOCKINBYTE (FIFO_port,dest,taille);

   VAR_STATE = LIRE(Var_State);
   U_FLAGS   = LIRE(UFlags);

  if ( ( buffer_r[0] !=h_signature )|| ( buffer_r[1] != l_signature) ) {
       	goto abort_test_registres;
  }
  if (  (VAR_STATE !=h_signature)|| ( U_FLAGS !=  l_signature)) {
        goto abort_test_registres;
  }
  ECRITE(UCom, CMDE_CLOSE);			/*fermeture session */
  return (FDM_OK);


abort_test_registres :
     ECRITE(UCom, CMDE_CLOSE);
     return ( FDM_NOK) ;

}


/*=====================================================================*/
/*	        	INIT TESTS ON LINE                             */
/*=====================================================================*/

static unsigned short const rand_numbers[64]= {
 33676U, 11517U, 20227U, 35031U, 62103U, 11254U, 46021U, 14839U,
 32425U,  8173U,  5498U, 25535U, 18168U, 24121U, 64450U, 35087U,
 50179U, 42367U, 50275U, 51133U, 53932U,  9957U, 40991U, 20623U,
 22734U, 60109U, 34062U, 26290U, 39764U, 51472U, 61048U, 57011U,
 56788U, 44205U, 49702U, 38134U, 25509U, 23306U, 13122U, 54193U,
 27256U, 30377U, 64170U,  8286U, 13935U, 62813U, 48330U, 26807U,
 51125U, 49669U, 62707U,  1841U, 20888U, 49606U, 15924U, 38636U,
  2845U, 62654U, 20914U,  3890U, 28958U, 59966U, 37502U,  7788U
};




/*=====================================================================*/
/*	        	 TESTS ON LINE                             */
/*=====================================================================*/
static Ushort _FIP_CODE_TEST ( FDM_REF *Ref )
{
  unsigned int base;
  int j,N;
  Uchar TAMPON[128];
  unsigned short checksum ;
  Ushort Cr ;
  int page_number;

     page_number =  Ref->FDM_Internal_Use.fipdiag.page_number;

     base = Ref->FDM_Internal_Use.tab_BC[page_number].base ;
     Cr = _FIP_READ_PHY(Ref,(long)base, (Ushort *)&TAMPON[0],128);
     if (Cr != 0 ){
	 FDM_Signal_Error(Ref , _ON_LINE_DIAG_FIPCODE_FAIL ,(Ulong)base );
         return FDM_NOK;
     }
#if			(  FDM_WITH_LITTLE_ENDIAN == YES )
	SWAB_SHORT( (char *)TAMPON , (char *)TAMPON  , 128 );
#endif

     if (base == 0)  {
		j = 48;
		N =126;
	}
     else {
	  	j = 0;
		N =128;
	}
     for (checksum=0 ; j < N ; j++){
     	checksum +=(unsigned short)(TAMPON[j]) ;
     }
     if (checksum !=Ref->FDM_Internal_Use.tab_BC[page_number].checksum){
	 FDM_Signal_Error(Ref ,_ON_LINE_DIAG_FIPCODE_FAIL  ,(Ulong)base );
        return FDM_NOK;
     }

     Ref->FDM_Internal_Use.fipdiag.page_number += 1;
     if ( Ref->FDM_Internal_Use.fipdiag.page_number >=  Ref->FDM_Internal_Use.Nb_Page_Vrai ){
     	Ref->FDM_Internal_Use.fipdiag.page_number = 0;
	Ref->FDM_Internal_Use.fipdiag.TST_ON_LINE =  _FIP_CTRL_RAM ;

	}

     return FDM_OK;
}


static Ushort _FIP_CTRL_RAM (FDM_REF *Ref)
{

  Ushort cr;
  enum CODE_ERROR	  cr_code_error ;

  int i;

  switch ( Ref->FDM_Internal_Use.fipdiag.test_number){
    default:
    case 0:
	Ref->FDM_Internal_Use.fipdiag.test_number = 1;
	/* 1ere passe : mise a jour des donnees de la variable*/
	for ( i= 0; i <64; i++){
		Ref->FDM_Internal_Use.fipdiag.Write_Data[i]++;
        }
    	break;

    case 1:
	Ref->FDM_Internal_Use.fipdiag.test_number = 2;
	Ref->FDM_Internal_Use.fipdiag.TST_ON_LINE = _FIP_CODE_TEST ;
	/* 2ieme passe : ecriture  de la variable*/
	cr = _FIP_WRITE_VAR(Ref, Ref->FDM_Internal_Use.fipdiag.ram_test_key,
		            (void *) Ref->FDM_Internal_Use.fipdiag.Write_Data);
        if (cr!=0){
	   cr_code_error   = _ON_LINE_DIAG_CIRCUIT_ACCES_FAILED ;
	   goto abort;
        }
	break;

    case 2:
	Ref->FDM_Internal_Use.fipdiag.test_number = 3;
	/* 3ieme passe : lecture de la variable - comparaison */
	cr =  _FIP_READ_VAR (
		Ref, Ref->FDM_Internal_Use.fipdiag.ram_test_key,
		(void *)Ref->FDM_Internal_Use.fipdiag.Read_Data);
        if ( ( cr ^ 0x20 )  != 0){
	   cr_code_error   =  _ON_LINE_DIAG_CIRCUIT_ACCES_FAILED ;
	   goto abort;
         }
      	break;

    case 3:
	Ref->FDM_Internal_Use.fipdiag.test_number = 0;
            /* comparaison */
         i= memcmp ((void *)&Ref->FDM_Internal_Use.fipdiag.Write_Data,
		    (void*)&Ref->FDM_Internal_Use.fipdiag.Read_Data,
		     126);
	 if (i!=0){
	   cr_code_error   = _ON_LINE_DIAG_TEST_RAM_FAIL ;
	   goto abort;
         }
    	break;

  }
  return FDM_OK;


  abort :
	{

	FDM_Signal_Error(Ref ,
		cr_code_error ,
		Ref->FDM_Internal_Use.fipdiag.test_number);
        return(FDM_NOK);
        }

  }

static Ushort _FIP_DIAG_ON_LINE (FDM_REF *Ref)
{
return  Ref->FDM_Internal_Use.fipdiag.TST_ON_LINE(Ref);
}

/* ---------------------------------------------------------*/
/*  sequencement tests on_line : _FIP_DIAG_ON_LINE_TICKS    */
/* ---------------------------------------------------------*/
/* declenche par ticks  , Test_Online_Ticks */
Ushort _FIP_DIAG_ON_LINE_TICKS (_FDM_XAE_Tempo_Ref *T)
{
   	Ushort cr;
   	cr =  _FIP_DIAG_ON_LINE (T->Fip);
   	return cr;
}


void _FIP_INIT_DIAG_ON_LINE (FDM_REF *Ref)
{

 	Ref->FDM_Internal_Use.fipdiag.TST_ON_LINE = _FIP_CTRL_RAM ;
	Ref->FDM_Internal_Use.fipdiag.test_number = 0;
    	Ref->FDM_Internal_Use.fipdiag.page_number = 0;
    	memcpy ((void *)Ref->FDM_Internal_Use.fipdiag.Write_Data,
            (void *)rand_numbers, sizeof (rand_numbers));

}

/* ---------------------------------------------------------*/
/*                  fdm_test_on_line                        */
/* ---------------------------------------------------------*/
Ushort fdm_online_test (FDM_REF *Ref)
{
    Ushort cr;

   if (Ref->S.Online_Tests_Ticks != 0){
	  FDM_Signal_Warning ( Ref ,  _TEST_ON_TICKS_ON_LINE , 0) ;
	  return(FDM_NOK);
   } else {
      cr= _FIP_DIAG_ON_LINE(Ref);
      return(cr);
    }
}


#endif


