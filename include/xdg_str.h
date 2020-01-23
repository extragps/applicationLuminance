/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES STRUCTURES COMMUNES                                  */
/**                                                                       **/
/***************************************************************************/
/***************************************************************************/
/*                  table de configuration des liaisons                    */
/***************************************************************************/
#include "standard.h"
#include "mon_def.h"
#include "xdg_def.h"
#include "rdtComm.h"

#ifndef XDG_STR_H
#define XDG_STR_H

typedef enum _typeLas
{
	LIAISON_SERIE,
	LIAISON_IP,
	LIAISON_WFIP,
	LIAISON_PASS

} E_typeLas;


typedef struct xdg_cf_las
{
	E_typeLas type_e;
  UINT8 cfg;
  UINT8 media;                  /* half duplex full duplex */
  UINT8 suffixe;
  UINT8 prefixe;
  LONG vitesse;
  LONG numPortIp;
  UINT8 data;
  UINT8 parite;
  UINT8 stop;
  UINT8 protect;
  UINT8 protocole;
  UINT8 attente_em;
} T_xdg_cf_las;

typedef struct xdg_las
{
  BYTE cfg;
  BYTE protocole;
  UINT8 phase;
  UINT8 libre;
  UINT8 attente_em;
  UINT8 flag_em;
  UINT8 bcc;
//  UINT bccMastSlv;
  UINT8 mode;
  UINT8 acq;
  INT time_out;
  UINT8 flag_time_out;
  UINT8 flag_fin_em;
  INT pt_in_rec;
  INT pt_out_rec;
  char buff_rec[MAX_BUF_EM];
  INT nb_car_em;
  char buff_em[MAX_BUF_EM];
//  UINT8 adr[5];
//  UINT8 nb_car_adr;
  UINT8 protect;
  UINT8 dispo_rec;

  /* On va remplacer le descripteur de liaison serie par
   * un pointeur sur na connexion.
   */
  CommCnx *conn_pt;
  /*  INT fdLiaisonLas */
  /* Lien sur la message queue associe */
  INT liaison;

  struct usr_ztf *pt_mes;
  char *nomDev;
} T_xdg_las;

/************************************************/
/*  pour la visualisation des transmission      */
/************************************************/
typedef struct xdg_vt
{
  INT port;
  INT tempo;
  INT trace;
  UINT8 pt_in;
  UINT8 pt_out;
  UINT8 buff[256];
} T_xdg_vt;

/***************************************************************************
*                  Structure de controle des racks
****************************************************************************/

struct xdg_ctrl_rack
{
  UINT8 phase;
  UINT8 debut;
  UINT8 flag_carte;
//  UINT8 nb_crt_MEO116;
//  UINT8 nb_crt_MSO116;
//  BYTE nb_crt_MSR116;
//  UINT8 nb_crt_MGP102;
//  BYTE nb_crt_LON100;
//  UINT8 crt_MEO116[NB_CRT_MEO116];      /* nombre de carte entree TOR presentes           */
//  BYTE crt_MSR116[NB_CRT_MSR116];       /* nombre de carte entree TOR presentes           */
//  UINT8 crt_MGP102[NB_CRT_MGP102];      /* nombre de carte MGP102                         */
//#ifdef PMV
//  UINT8 crt_MSO116[NB_CRT_MSO116];      /* nombre de carte sortie TOR presentes           */
//  BYTE crt_LON100[NB_CRT_LON100];       /* nombre de carte MGP102                         */
//#endif
//  UINT8 crt_COM104;
//  UINT8 crt_DSR111;
  UINT8 fin;
  BYTE reposOut;
  BYTE initRack;
  BYTE defautRack;

  UINT memNbWdgDistant;
  UINT nbWdgDistant;
  ULONG nbTrame;
  ULONG nbErreurTrame;
  ULONG bcc;
//  UINT16 input[NB_CRT_MGP102];
//  UINT16 output[NB_CRT_MGP102];
//#ifdef PMV
//  UINT16 inputMSO116[NB_CRT_MSO116];
//  UINT16 outputMSO116[NB_CRT_MSO116];
//#endif
//  UINT16 outputMSR116[NB_CRT_MSR116];
//  UINT masque_sab[NB_CRT_MGP102];
//  UINT output_tac;
//  BYTE luminositeTor;           /* FALSE si cellule sur LON,TRUE sinon */
//  BYTE celluleTor;              /* etat des cellule J,N,SURB */
//  BYTE cmdLuminositeTor;        /* commande du relais de luminosite */
//  BYTE etatLuminositeTor;       /* etat du relais jour/nuit */
//  struct
//  {
//    UINT lumin[2];              /* luminosite cellule 1 en LUX */
//    UINT8 temp;                 /* temperature 1/10e           */
//    UINT8 ana1;                 /* reserve                     */
//    UINT8 ana2;                 /* reserve                     */
//  } lt501[NB_LT501];
};

#endif
