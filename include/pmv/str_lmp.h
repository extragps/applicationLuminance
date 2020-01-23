#define NB_LMP_LON	(NB_CRT_MRH132*8)

struct bloc_lon
{
  BYTE numBloc;                 /* numero de bloc */
  BYTE adressage;               /* GROUPE,SUBNET/NODE */
  BYTE groupe;                  /* numero de groupe */
  BYTE subnet;                  /* numero de subnet */
  BYTE node;                    /* numero de node */
  BYTE service;                 /* type de service */
  BYTE variable;
  BYTE txt[10];
};
#define NB_PAGE 			 4
#define TEMPO_CONTROLE   10            /* delai entre l'activation et le debut du controle */

struct ctrlLampe
{
  BYTE cfg;                     /* si la lampe existe */
  BYTE var_init;                /* la variable reseau recu    */
  BYTE var_tdp;                 /* la variable reseau recu    */
  BYTE var_cdp;                 /* la variable reseau recu    */
  BYTE var_lum;                 /* la variable reseau recu    */
  BYTE var_surv;                /* la variable reseau recu    */
  BYTE var_groupe;              /* la variable reseau recu    */
  BYTE msqLampe;                /* 1 pour allumer la lampe normale,2 pour allumer la lampe secours */
  BYTE etat;                    /* pour l'etat du couple de lampe 0 OK,1,2,3 HS ET 4,5,6 HS ALLUM */
  BYTE etatTest;                /* pour l'etat de la lampe suite au test */
  BYTE tempoControle;
  BYTE cligOn;
  BYTE cligOff;
  BYTE clignotement;
  BYTE groupe;                  /* numero de groupe auquel appartient la lampe */
  struct
  {
    BYTE flag;                  /* reception de la variable TDP */
    UINT timeOut;
    UINT timeOutCtrl;
  } tdp;
  struct
  {
    BYTE numPage;               /* le numero de page */
    BYTE type;                  /* 0 : eteint, 1 : fixe, 2 : clignotant */
    BYTE tpOn;                  /* en 1/10 sec */
    BYTE tpOff;                 /* e, 1/10 sec */
  } memPage[NB_PAGE];
  BYTE numPageChargee;
  BYTE nbPageChargee;
  UINT checksum;
  BYTE actPage;                 /* numero de page active 0 : extinction */
  struct
  {
    BYTE tension;
    BYTE mode;                  /* 0=normal/secours 1=surbrillance */
  } luminosite;
  struct
  {
    UINT tempoTest;
    BYTE phaseTest;
    BYTE memEtat;
    BYTE etat;
  } test;
};

GLOBALREF struct ctrlLampe ctrlLampe[];


struct lampe
{
  BYTE mode;                    /* 0 : eteint,1 : normale allumee,2 : secours allumee,3 : normale+secours allume */
  BYTE etat;                    /* 0 OK, 1:normale HSETEINT,2:secours HSETEINT,3:normale+secours HSETEINT */
  /* 4:normale HSALLUMEE,5:secours HSALLUMEE,6:NORMALE+SECOURSHSALLUMEE     */
  BYTE tOn;
  BYTE tOff;
  BYTE tpOn;
  BYTE tpOff;
};

GLOBALREF struct lampe lampe[];
