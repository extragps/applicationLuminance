/************************************************************

" @(#) All rights reserved (c) 1999                                         "
" @(#) ALSTOM (Paris, France)                                               "
" @(#) This computer program may not be used, copied, distributed,          "
" @(#) corrected, modified, transmitted or assigned without ALSTOM's        "
" @(#) prior written authorization.                                         "

  ALSTOM - 5 ,Av. Newton 92140 Clamart - CCD
 


      Projet      : Fip Device Manager Version 4

  Nom du fichier  : fdmcreba.c
  Description     : Procedures de creation d'un BA



  Date de creation: Fri Mar 15 15:11:23 MET 1996

Version 4.1.1
	 calcul taille prog BA fausse -> Send_liste_init
	 rangement BA si @ > ffff  -> fdm_ba_load_macrocycle_fipconfb
Version 4.1.3
	fdm_ba_delete_macrocycle -> delete accepter si BA dans etat IDLE
Version:4.5.0
	a/ modifs pour portage sur cible DSP texas
	b/ allegement code pour NT
Version:4.7.0
	a/ instruction BA : SYN_WAIT_SILENT
***********************************************************/
#include "user_opt.h"

#include "fdm.h"


#define  NO_VALID 0xff


#if		(  FDM_WITH_BA == YES )


/*************** BUS ARBITRATOR user program type definition ***************/
#define LABEL           101
#define END_BA          102



typedef struct {
		Ushort longueur;
		Ushort label_code;
		Ushort nom_label;
		Ushort lg_liste;
		Ushort LISTE[1];
		}FIPCONF;

typedef struct {
	Ushort ID ;
	Ushort Frame_Code ;
	} LIST ;


typedef struct {
	Ushort Op_Code ;
	Ushort Param1 ;
	Ushort Param2 ;
	} BA_PRG ;

typedef struct { BA_PRG INST[1];
               } BA_TAB;


typedef struct { Ushort BA[1] ; } TYPE_BA;

typedef struct{
        	int	 long_BA;
		_TABLE  *tab_AntiDup;
		Ushort  *ptr_id_dat_exist;	/* pointeur dans tab_ba */
		int	 index;

		TYPE_BA *tab_BA;
		Ushort   FFip_AntiDup;
		Ushort   mot_bidon;
	       } PRIVE_BA ;


/* procedures decrites */

FDM_BA_REF * fdm_ba_load_macrocycle_fipconfb ( FDM_REF * , const Ushort * );
Ushort fdm_ba_delete_macrocycle ( FDM_BA_REF * );


static void   lib_mem_BA  ( FDM_REF* , Ushort ,int  );




/****************************************************************/
/*   			 dem_mem_MC  			        */
/* ce SP cherche un emplacement de N places consecutives libres */
/* il retourne l'adresse trouvee libre                          */
/****************************************************************/

Ushort dem_mem_MC( FDM_REF *Fip,int N )
{
 Ushort i,j,k,m;

 m = (Ushort) ( ((int)Fip->S.BA_Dim / 32)/16) ;
 for (i = 0 ; i < m ; i++) {
    if (Fip->FDM_Internal_Use.fipgen.N_PAGE_BA->bit_map[i] != 0xff) {
      for (j = 0 ; j < 8 ; j++) {
	 if ((Fip->FDM_Internal_Use.fipgen.N_PAGE_BA->bit_map[i] & (1<<j)) == 0) {
	    for (k = j ; k < (Ushort)(j+N) ; k++) {
	      if ((Ushort) (i+(k/8))  >= m ) goto l0;
		 if ((Fip->FDM_Internal_Use.fipgen.N_PAGE_BA->bit_map[i+(k/8)] &
		     (1<<k%8)) != 0  )   goto l;
	    }
	    for (k = j ; k < (Ushort)(j+N) ; k++) {
	       Fip->FDM_Internal_Use.fipgen.N_PAGE_BA->bit_map[i+(k/8)] =
	  		(Uchar)(Fip->FDM_Internal_Use.fipgen.N_PAGE_BA->bit_map[i+(k/8)]
	    			  | (1<<k%8) );
            }
	    return((Ushort)(((i*8)+j)*64));
            l:;
          }
        }
     }
  }
 l0:;
 return (0xffff); /* il n'y a plus de place*/

}

/*****************************************************************/
/*      lib_mem_BA :   ce SP rend les places libres pour le BA   */
/*****************************************************************/

static void   lib_mem_BA( FDM_REF *Fip,Ushort adr,int N_pages )
{
   int i,j,k;

 if (N_pages == 0 ) return;

 j = (int)(adr/64) ;
 i = j / 8 ;
 j = j % 8 ;
 for ( k = j; k < N_pages +j; k++) {
     Fip->FDM_Internal_Use.fipgen.N_PAGE_BA->bit_map[i+(k/8)] =
	 (Uchar)( Fip->FDM_Internal_Use.fipgen.N_PAGE_BA->bit_map[i+(k/8)] & ~(1<<k%8) );
  }

}

/***********************************************************************/
/*          procedure interne : Send_liste_init                        */
/***********************************************************************/
/* retourne enum CODE_ERROR   a appeler dans procedure de warning
 si ID_DATE  ---> 3 mots : BA_SEND_ID_DAT , identifieur, Antidup_Elem_Adr
 si ID_MSG   ---> 2 mots : BA_SEND_ID_MSG , identifieur
*/

static enum CODE_ERROR  Send_liste_init( FDM_BA_REF *Ba_Ref ,Ushort nb_id,
					LIST * adr_liste,PRIVE_BA *private_ba)
{
     enum  __dicho_result     cr1;
     Ushort    *ptr_exist ;
     element    elem ;
     int        i;
     int        lg = 0;

  if (nb_id == 0 ){
	return (_GEN_BA_NO_ID_IN_LIST);
  }

  /* calcul de la table du progamme ba */
{
  LIST * liste;

  liste = adr_liste ;
  for (i=0 ;i < (int) nb_id; i++){
    if ((liste->Frame_Code  & 0xf ) == ID_DAT )
	lg +=3;			/* ID_DAT --> 3 mots */
    else
	lg +=2 ;		/* ID_MSG --> 2 mots */
    liste++;
  }
  private_ba->long_BA  += lg;
}

  /* init creation ou modification tab antidup en ram */
  for (i=0 ;i < (int) nb_id; i++){
    if ( ( (adr_liste->Frame_Code  & 0xf ) == ID_DAT ) &&
     	 ( (adr_liste->ID & 0xff00) !=0x1400 )) {

      if (private_ba->tab_AntiDup == NULL) {
           	    /* creation table dicho */
	  cr1= _DICHO_CREATE_VAR ( Ba_Ref->Fip->H.Memory_Management_Ref,
	  		&(private_ba->tab_AntiDup));
	  if (cr1 !=__dicho_result_ok)
	    	return ( _GEN_BA_DEF_DICHO );

	            /* ajout de l ' element en table */
           elem.objet_num          =(long)adr_liste->ID;  /* no id */
	   elem.ptr                = private_ba->ptr_id_dat_exist;
	   cr1 = _DICHO_ADD_OBJECT (&(private_ba->tab_AntiDup),&elem);
	   if (cr1 !=__dicho_result_ok)
	      return ( _GEN_BA_DEF_DICHO );
       }
       else { /* table dicho deja cree */

	    /* verifie si id_dat existe deja */
	    ptr_exist =(Ushort*) _DICHO_SCAN_OBJECT(&(private_ba->tab_AntiDup),
	                                   (long)adr_liste->ID );
	    if (ptr_exist ==NULL){
	            elem.objet_num          =(long)adr_liste->ID;  /* no id */
	            elem.ptr                = private_ba->ptr_id_dat_exist;
	            cr1= _DICHO_ADD_OBJECT (&(private_ba->tab_AntiDup),&elem);
	            if (cr1 !=__dicho_result_ok)
	     		 return ( _GEN_BA_DEF_DICHO );
	     }
	     /* sinon id_dat existe deja */
       }
      }
      adr_liste++;
    }
    return( _NO_ERROR );
}


/***********************************************************************/
/*          procedure interne : Send_liste                             */
/***********************************************************************/
 static void Send_liste(FDM_BA_REF *Ba_Ref, Ushort nb_id,
			LIST * adr_liste, PRIVE_BA *private_ba)
{
	int i,j;
	int rang ;	    /* rang de l'id dans la table antiduplication */
	_TABLE *tab_dich;

  rang = 0;
  for (i=0 ;i < (int) nb_id; i++){
    if ( ((adr_liste->Frame_Code  & 0xf ) == ID_DAT ) &&
     	 ((adr_liste->ID & 0xff00) !=0x1400 )) {

           tab_dich = private_ba->tab_AntiDup;
           for (j=0;j <tab_dich->nombre  ; j++){
                if (tab_dich->k[j].objet_num == (long)adr_liste->ID){
                     rang=j;
                     break;
                 }
           }

    	     		/* code instruction */
     	   private_ba->tab_BA->BA[private_ba->index]= BA_SEND_ID_DAT;
    	   private_ba->index++;
     	                /* id   */
           private_ba->tab_BA->BA[private_ba->index]= adr_liste->ID ;
           private_ba->index++;
                        /* index id dans table antidup */
    	   private_ba->tab_BA->BA[private_ba->index]=
    	     		(Ushort) ((rang * 2 ) + private_ba->FFip_AntiDup);
    	   private_ba->index++;

    }else {
       if ((adr_liste->ID & 0xff00) ==0x1400 ) {
	   private_ba->tab_BA->BA[private_ba->index] = BA_SEND_ID_DAT;
           private_ba->index++;
	   private_ba->tab_BA->BA[private_ba->index] = adr_liste->ID ;
	   private_ba->index++;
		/*id presence ->  table antidup du programme ba_test_p */
	   private_ba->tab_BA->BA[private_ba->index] =
	        (Ushort)(  Ba_Ref->Fip->FDM_Internal_Use.Page_2000.BA_Base_B +
			   0x40+ (adr_liste->ID & 0xff)*2 );
	   private_ba->index++;
	}
        else {
		/* ID_MSG */
	   private_ba->tab_BA->BA[private_ba->index] = BA_SEND_ID_MSG;
	   private_ba->index++;
	   private_ba->tab_BA->BA[private_ba->index] = adr_liste->ID ;
	   private_ba->index++;
	}
     }

    adr_liste++;
  }

}




/***********************************************************************/
/*                                                                     */
/*           chargement du programme Ba fourni au format FIPCONF       */
/*                                                                     */
/***********************************************************************/

FDM_BA_REF * fdm_ba_load_macrocycle_fipconfb ( FDM_REF * Fip ,
				const Ushort * User_prog_ba )
{
	FDM_BA_REF	  *Ba_ref;
	FIPCONF		  *deb_fipconf;
	Ushort            *adr_x;
        Ushort             xxx;
        int	           lg;
        int 		   lg_liste;		/* taille de la liste ,en mots*/
 	int 	 	   i,j;
	BA_TAB            *BA_User;		/* debut programme BA */
	LIST              *adr_liste;
	PRIVE_BA          private_ba;

        Ulong     	  dest ;
        Ushort  	  *src ;


   enum CODE_ERROR	  cr_code_error = 0;
   int			  cr_valid =0;


   	 /* phase de controles     */
 	 /*=======================*/

   	/* controle du checksum */
   deb_fipconf =(FIPCONF *)User_prog_ba;
   adr_x = (Ushort *) deb_fipconf;
   lg =(int) deb_fipconf->longueur;
   xxx = 0;
   for (i=0 ; i < lg ;i++){
   	xxx += *adr_x++;
   }
   if ( xxx != 0){
	cr_code_error  = _GEN_BA_CHEKSUM_ERROR  ;
	goto abort1;
   }

    	/* controle de la liste ; id_dat id_msg */
   lg_liste =(int)deb_fipconf->lg_liste;
   for (i=0 ; i < lg_liste  ;i=i+2){
   	xxx = deb_fipconf->LISTE[i+1] & 0xf;
        if ((xxx != ID_DAT)  && (xxx!= ID_MSG)){
	  cr_code_error  =_GEN_BA_LIST_ERROR  ;
	  goto abort1;
	}
   }

   /* taille du programme BA = taille totale - 5 mots -lg_liste */

   BA_User= (BA_TAB*)&deb_fipconf->LISTE[lg_liste];
   lg = (int)(deb_fipconf->longueur -(lg_liste  +5));
   lg = lg/3;

     	/* controle de l'existance de l'instruction END_BA en derniere
     	   instruction  */
   xxx=BA_User->INST[lg-1].Op_Code ;
   if (xxx != END_BA){
	cr_code_error   =_GEN_BA_END_BA_NOT_FOUND ;
	goto abort1;
    }
        	/* controle de la validite des code operation  */
   for (i=0 ; i < (lg -1) ;i++){
     xxx=BA_User->INST[i].Op_Code ;
     switch (xxx){
     	case NEXT_MACRO:
 	case SEND_MSG :
	case SEND_APER:
	case BA_WAIT:
	case SEND_LIST:
	case TEST_P:
        case SYN_WAIT:
        case SYN_WAIT_SILENT:
        case END_BA:
        	break;
        default:
                cr_valid       = NO_VALID;
		cr_code_error  = _GEN_BA_NOT_EXISTED_INSTRUCTION  ;
		break;
      }
   }
   if (cr_valid !=0 ) goto abort1;


   /* creation du programme ba : init */
   /*=================================*/
{
  private_ba.long_BA         = 0;
  private_ba.index           = 0;
  private_ba.tab_AntiDup     = NULL;
  private_ba.tab_BA          = NULL;
  private_ba.ptr_id_dat_exist= NULL;
  private_ba.FFip_AntiDup    =  0 ;


      /* alloue zone memoire reference  BA  */

  Ba_ref=(FDM_BA_REF *)FDM_Malloc(Fip,sizeof(FDM_BA_REF));
  if(Ba_ref ==NULL){
	cr_code_error   = _ALLOCATE_MEMORY_FAULT ;
	goto abort1;
  }


    Ba_ref->Fip		    = Fip;
    Ba_ref->Macro_Cycle_Adr = 0;
    Ba_ref->Nbre_page	    = 0;
    Ba_ref->Label	    = deb_fipconf->nom_label;

      /* alloue 1 pointeur  pour initialiser  la table antiduplication  */

    private_ba.ptr_id_dat_exist  = &private_ba.mot_bidon;

}

   /* creation du programme ba : Phase 1: preparation */
   /*=================================================*/
   /* - calcul de la taille du ba
      - init table d'anti-dupplication */
 {
  j=0;
  while (BA_User->INST[j].Op_Code !=END_BA){
    switch ( BA_User->INST[j].Op_Code){
    	case NEXT_MACRO:
	     private_ba.long_BA++;
	break;

    	case SEND_MSG :
	case SEND_APER:
	case BA_WAIT:
        case SYN_WAIT:
        case SYN_WAIT_SILENT:
 	     private_ba.long_BA += 2;
	break;

	case TEST_P:
#if	(FDM_WITH_REDONDANCY_MGNT == YES )
 	     private_ba.long_BA += 6;
#else
             private_ba.long_BA += 2;
#endif
	 break;

        case SEND_LIST:
        	adr_liste =(LIST*) &deb_fipconf->LISTE[BA_User->INST[j].Param2];
             	cr_code_error = Send_liste_init ( Ba_ref ,
             		BA_User->INST[j].Param1 ,
             		adr_liste , &private_ba );
		if ( cr_code_error != _NO_ERROR ) goto abort;
        break;

        default:
        break;
     }
     j++;
   }
  }

   /* creation du programme ba : Phase 2  : verification ,
   					    allocation prog_ba  */
   /*===========================================================*/
{
       int	nb_elem_ba ;
       int      lg_anti_dup ;
       LIST     *ptr_liste ;
       LIST     *ptr_liste_x;
       Ushort 	 cpt_id_dat;
       int       N_page;

  private_ba.index   = 0;
  lg = private_ba.long_BA * 2 ;
  if ( (lg % (1<< 7)) !=0 )
    		lg = ((lg >>7 )+1) <<7;
  private_ba.long_BA = lg/2;
  nb_elem_ba =lg /128 ;

  if (private_ba.tab_AntiDup ==NULL){
  	/* pas de table antidup */
      N_page                  = nb_elem_ba ;
      Ba_ref->Macro_Cycle_Adr = dem_mem_MC(Fip,N_page);
      Ba_ref->Nbre_page       = N_page;

      if (Ba_ref->Macro_Cycle_Adr == 0xffff ) {
  		/* probleme allocation pages dans fullfip : pas de place dispo*/
        cr_code_error    = _GEN_BA_DEF_DEM_MEM_MC  ;
        goto abort;
      }
  }else {

        /* taille table antidup avec demande du nombre de pages dans FULLFIP*/
        /*------------------------------------------------------------------*/
   lg =4*  private_ba.tab_AntiDup->nombre ;
   if ( (lg % (1<< 7)) !=0 )
    		lg = ((lg >>7 )+1) <<7;
   lg_anti_dup =lg ;

   N_page          = nb_elem_ba + (lg_anti_dup /128);
   Ba_ref->Macro_Cycle_Adr = dem_mem_MC(Ba_ref->Fip,N_page);
   Ba_ref->Nbre_page       = N_page;

   if (Ba_ref->Macro_Cycle_Adr == 0xffff ) {
  		/* probleme allocation pages dans fullfip : pas de place dispo*/
        cr_code_error    = _GEN_BA_DEF_DEM_MEM_MC  ;
        goto abort;
   }
    private_ba.FFip_AntiDup =Ba_ref->Macro_Cycle_Adr +  private_ba.long_BA ;


          /* alloue table antidup destinee a fullfip */
    ptr_liste =  (LIST*)FDM_Malloc( Fip , lg_anti_dup );
    if (ptr_liste ==NULL){	/*probleme memoire*/
    	cr_code_error   = _ALLOCATE_MEMORY_FAULT ;
        goto abort;
     }

    memset((void*)ptr_liste, 0 ,lg_anti_dup);

    ptr_liste_x =ptr_liste;

    /*remplissage de la table antidup du FULLFIP:
      table constituee de  n elements avec
	1 element = ID
	reserve fipcode = 0      		      */
   /*-------------------------------------------------------------------------*/
    for (i =0 , cpt_id_dat=0 ; i< private_ba.tab_AntiDup->nombre ; i++) {
        if (private_ba.tab_AntiDup->k[i].objet_num !=-1){
   	   ptr_liste_x->ID = (Ushort) (private_ba.tab_AntiDup->k[i].objet_num);
	   cpt_id_dat +=1;

        }else{
	 	ptr_liste_x->ID = 0xffff;
	 }
         ptr_liste_x++;
    }

   /* controle de la taille des files par rapport a celle prevue ,
      FDM_V4 :  en fonction des n id_dat                         */
   /*------------------------------------------------------------*/
    if (cpt_id_dat > Fip->S.NB_OF_DIFFERENT_ID_PROG_BA){
	    cr_code_error  = _GEN_BA_DEF_DIM_FILES ;
	               /* liberation de la table ptr_liste allouee */
                FDM_Free(Fip,(char *)ptr_liste );
                goto abort;
     	}

   /* ecriture de la table antidup dans Fullfip */
   /*  mise a jour de Ba_Ref                    */
   /*-------------------------------------------*/

    dest  = private_ba.FFip_AntiDup;
    src   = (Ushort*)ptr_liste;
    for (i=0 ;i < lg_anti_dup /128 ; i++) {
    		/* ecriture de 128 bytes */
      if (  _FIP_WRITE_PHY ( Fip ,
		FDM_ADR_USER + dest,src,128)  !=0){
   	    cr_code_error    = _CIRCUIT_ACCES_FAILED ;

            /* liberation de la table ptr_liste allouee */
                FDM_Free( Fip , (char *)ptr_liste );
  	    goto abort;
      }
      src  = src  + 64 ;
      dest = dest + 64 ;
    }


   /* liberation de la table ptr_liste allouee  */
   /*-------------------------------------------*/
   FDM_Free(  Fip,(char*)ptr_liste );


 }/*end else : fin du traitement table antidup   */


    /* allocation du programme ba                */
   /*-------------------------------------------*/
   lg = private_ba.long_BA * 2 ;

   private_ba.tab_BA =  (TYPE_BA*)FDM_Malloc(Fip,lg);
   if(private_ba.tab_BA ==NULL){
	    cr_code_error    = _ALLOCATE_MEMORY_FAULT ;
  	    goto abort;
    }
   memset((void*) private_ba.tab_BA, 0 ,lg);


}

   /* creation du programme ba : Phase 3  : remplissage table BA  */
   /*=============================================================*/
 {
    Ushort  Utempo = 0;
    j=0;

  while (BA_User->INST[j].Op_Code !=END_BA){

    switch ( BA_User->INST[j].Op_Code){
	case SEND_MSG  :
	case SEND_APER :
	case BA_WAIT   :
	case SYN_WAIT  :
	case SYN_WAIT_SILENT :
	     {
	         Ulong     Tempo;
	         Ulong     ll;

		/* calcul + verifie time */
	        Tempo =  ((Ulong) BA_User->INST[j].Param1) <<16;
	        Tempo =  Tempo |  BA_User->INST[j].Param2;
	        Tempo = (Tempo *100)  ;

	        ll = Tempo / (Fip->S.TSlot);       /*conversion en temps */
 	        if (Fip->S.TSlot == 62){
 	            	Ulong llc;
		        llc = Tempo/63;
		        ll -=  ( ll - llc)/2;
	         }
	        if (( ll & 0xffff8000L ) != 0 ) {
		       cr_valid        = NO_VALID;
		       cr_code_error   = _GEN_BA_NOT_VALID_TIME  ;
	        }
		else {
	       	  Utempo  =(Ushort) ll;

                }
              }
        break;
        default:
	break;
     }
     if (cr_valid !=0) break;

     switch ( BA_User->INST[j].Op_Code){
    	case NEXT_MACRO:
    	     private_ba.tab_BA->BA[private_ba.index++]= BA_NEXT_MACRO;
    	break;

   	case SEND_MSG :
    	     private_ba.tab_BA->BA[private_ba.index++]= BA_SEND_MSG;
     	     private_ba.tab_BA->BA[private_ba.index++]= Utempo ;
      	break;

	case SEND_APER:
    	     private_ba.tab_BA->BA[private_ba.index++]= BA_SEND_APER;
     	     private_ba.tab_BA->BA[private_ba.index++]= Utempo ;
      	break;

	case BA_WAIT:
    	     private_ba.tab_BA->BA[private_ba.index++]= BA_WAIT_TIME;
     	     private_ba.tab_BA->BA[private_ba.index++]= Utempo ;
      	break;

	case SYN_WAIT:
    	     private_ba.tab_BA->BA[private_ba.index++]= BA_WAIT_SYNC;
     	     private_ba.tab_BA->BA[private_ba.index++]= Utempo ;
      	break;

	case  SYN_WAIT_SILENT:
    	     private_ba.tab_BA->BA[private_ba.index++]= BA_WAIT_SYNC_SILENT;
     	     private_ba.tab_BA->BA[private_ba.index++]= Utempo ;
      	break;

	case TEST_P:
#if	(FDM_WITH_REDONDANCY_MGNT ==YES)
		/* on prend delai max pour execution environ  5 ms ! */
    	     private_ba.tab_BA->BA[private_ba.index++]= BA_SUSPEND;
     	     private_ba.tab_BA->BA[private_ba.index++]= SUSPEND_DELAI /Fip->S.TSlot;
     	     private_ba.tab_BA->BA[private_ba.index++]= EVT_TESTP_DEBUT;

    	     private_ba.tab_BA->BA[private_ba.index++]= BA_SEND_ID_DAT;
    	     private_ba.tab_BA->BA[private_ba.index++]= ID_BOURRAGE;
    	     private_ba.tab_BA->BA[private_ba.index++]= 0x0000;

#else
    	     private_ba.tab_BA->BA[private_ba.index++]= BA_TESTP;
     	     private_ba.tab_BA->BA[private_ba.index++]= BA_TESTP_END;
#endif
         break;

        case SEND_LIST:

             adr_liste =(LIST*)&deb_fipconf->LISTE[BA_User->INST[j].Param2];
             Send_liste(Ba_ref, BA_User->INST[j].Param1,adr_liste,&private_ba);
        break;

        default:
        break;
     }
     if (cr_valid !=0) break;
     j++;
   }
   if(cr_valid!=0) goto abort;
 }



   /* creation du programme ba : Phase 4  : store_BA              */
   /*=============================================================*/
      /* ecriture de la table BA dans Fullfip */
   /*-------------------------------------------*/
    dest  = Ba_ref->Macro_Cycle_Adr;
    src   = (Ushort *)private_ba.tab_BA;
    for (i=0 ;i < private_ba.long_BA /64  ; i++) {

       if ( _FIP_WRITE_PHY(Fip,FDM_ADR_USER + dest,src,128)  !=0){
   	  cr_code_error   = _CIRCUIT_ACCES_FAILED  ;
          goto abort;
       }
       src = src+64;
       dest=dest+64;
     }


    /*  Phase 5  : liberation des zones memoires allouees -fin ok  */
    /*==============================================================*/
     FDM_Free(Fip,(char *) private_ba.tab_BA ) ;
     if (private_ba.tab_AntiDup !=NULL )
 	 _DICHO_REMOVE(& (private_ba.tab_AntiDup));

      return (Ba_ref);



     /*  Phase abort  : liberation des zones memoires allouees     */
    /*============================================================*/
abort:
                 /*  libere table du ba */
       if (private_ba.tab_BA !=NULL )
           FDM_Free(Fip,(char *) private_ba.tab_BA);


            /*  libere table dichotomique antidup*/
       if (private_ba.tab_AntiDup !=NULL )
       	   _DICHO_REMOVE(& (private_ba.tab_AntiDup));


       if (Ba_ref->Macro_Cycle_Adr !=0 ){
        	/* liberation memoire BA dans FFIP */
           lib_mem_BA( Fip,Ba_ref->Macro_Cycle_Adr ,Ba_ref->Nbre_page );
       }

       FDM_Free(Fip, (char *)Ba_ref);

abort1 :
  	FDM_Signal_Warning(Fip,cr_code_error,0) ;
        return (NULL);
}

#if					( FDM_WITH_NT != YES )

/***********************************************************************/
/*                                                                     */
/*     chargement du programme Ba fourni par le USER                   */
/*                                                                     */
/***********************************************************************/
 FDM_BA_REF*  fdm_ba_load_macrocycle_manual ( FDM_REF *Fip, int Nb_of_Liste,
                                     int Nb_of_Instruction , Ushort Label_no,
	                             const PTR_LISTS  *Listes,
	                             const PTR_INSTRUCTIONS *Instructions )

{
  FDM_BA_REF	  *Ba_ref;

   Ushort size_macro_cycle;
   Ushort size_all_listes=0;
   Ushort size_liste=0;
   Ushort offset=0;
   Ulong V,V1;
   int i,j;
   int       no_list;
   Ushort   *ba_fipconf;
   FIPCONF  *deb_fipconf;
   BA_TAB   *ba_inst_fipconf;
   const PTR_LISTS   *ptr_listes_user;
   Ushort   *ptr_list_fipconf;


 /* allocation zone memoire  pour mise en forme ba user en format Fipconf */


 /* calcul de la taille totale des  listes */
 ptr_listes_user= Listes;
 for (i=0 ; i< Nb_of_Liste ; i++,ptr_listes_user++){
 	 size_all_listes =size_all_listes+ptr_listes_user->List_Size;
 }
 size_all_listes =size_all_listes /sizeof(Ushort);	/*taille en mots*/
 size_macro_cycle = 5 +size_all_listes+( (Nb_of_Instruction+1) *3) ;


  ba_fipconf=(Ushort*)FDM_Malloc(Fip,size_macro_cycle*2);
  if(ba_fipconf ==NULL){
   	FDM_Signal_Warning(Fip,_ALLOCATE_MEMORY_FAULT,0) ;
        return (NULL);
  }
  deb_fipconf=  (FIPCONF*)ba_fipconf;
  deb_fipconf->longueur   = size_macro_cycle;
  deb_fipconf->label_code = LABEL;
  deb_fipconf->nom_label  = Label_no;
  deb_fipconf->lg_liste   = size_all_listes;

  /* remplissage des listes */

  ptr_list_fipconf = &deb_fipconf->LISTE[0];
  ba_inst_fipconf  =(BA_TAB*) (ptr_list_fipconf+ size_all_listes) ;
  ptr_listes_user= Listes;

  for (i=0 ; i< Nb_of_Liste ; i++,ptr_listes_user++){
        size_liste= ptr_listes_user->List_Size ;
  	memcpy ( ptr_list_fipconf,ptr_listes_user->Ptr_List_Element,size_liste );
        ptr_list_fipconf=ptr_list_fipconf+ (size_liste /sizeof(Ushort));
  }

   /* remplissage instructions */
    for (i=0 ; i< Nb_of_Instruction ; i++,Instructions++){
    	 ba_inst_fipconf->INST[i].Op_Code = Instructions->Op_Code;
    	switch (Instructions->Op_Code){
    	     case TEST_P:
    	     case NEXT_MACRO:
    		ba_inst_fipconf->INST[i].Param1=0;
    		ba_inst_fipconf->INST[i].Param2=0;
    		break;
    	     case SEND_APER:
    	     case SEND_MSG:
    	     case BA_WAIT:
    	     case SYN_WAIT:
    	     case SYN_WAIT_SILENT:
    	      V=  Instructions->Param1 ;
    	      V= V/100;	/* temps fourni par user en micro secondes */
    	      V1=  V >>16;
    	      ba_inst_fipconf->INST[i].Param1 = (Ushort) V1;
    	      ba_inst_fipconf->INST[i].Param2= (Ushort) V;

    	     break;
    	     case SEND_LIST:
    	          no_list =(int)Instructions->Param1;
    	          ptr_listes_user= Listes;
    	          size_liste =0;
    	          offset=0;
    	          for (j=0;j< no_list;j++){
       			 offset += ptr_listes_user->List_Size ;
    	           	 ptr_listes_user++;
    	          }
    	          ba_inst_fipconf->INST[i].Param1=ptr_listes_user->List_Size/(sizeof(Ushort)*2);
    		  ba_inst_fipconf->INST[i].Param2=offset/sizeof(Ushort);
    	     break;
    	     default:
    	     break;
         }
    }
    ba_inst_fipconf->INST[i].Op_Code = END_BA;
    ba_inst_fipconf->INST[i].Param1=0;
    ba_inst_fipconf->INST[i].Param2=0;

{ /* Calcul_checksum */

	Ushort lg , i , j;
	Ushort *CK ;

 	i = 0 ;
	CK = ( Ushort * ) ba_fipconf ;
	lg = *CK ;
	lg--;
	for (  j = 0 ; j < lg ; j++){
		i+=*CK++;
	}

	*CK =~i +1;

}

  Ba_ref= fdm_ba_load_macrocycle_fipconfb ( Fip,ba_fipconf);

 /* liberation table ba_fipconf  */
 FDM_Free(Fip, (char*)ba_fipconf );
 return(Ba_ref);

}

#endif

/***********************************************************************/
/*                     destruction du programme Ba                     */
/***********************************************************************/
Ushort  fdm_ba_delete_macrocycle ( FDM_BA_REF * BA)
{


  FDM_REF        *Ref;

  Ref= BA->Fip;
  if ( Ref->FDM_Internal_Use.Ba.BA_in_use == BA ){
       FDM_Signal_Warning(Ref , _DELETE_MC_IN_USE,0) ;
       return(FDM_NOK);
  }
  if ( ( Ref->FDM_Internal_Use.Ba.BA_running == 0 ) ||
       ( Ref->FDM_Internal_Use.Ba.Current_Macrocycle != BA->Macro_Cycle_Adr) ){
      lib_mem_BA(Ref , BA->Macro_Cycle_Adr,BA->Nbre_page);
      FDM_Free(Ref , (char*)BA );
      return (FDM_OK);
   }else {
       FDM_Signal_Warning(Ref , _DELETE_MC_IN_USE,0) ;
       return(FDM_NOK);
   }

}


#endif
