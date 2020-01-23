#ifndef STATIONLUMBSC_H_
#define STATIONLUMBSC_H_

typedef enum _Attente {
	ATTENTE,SELECTING,POLLING,MESSAGE,MESSAGE_EM,ACK_ETAT,EOT_ETAT
} Attente;

typedef enum _BscLiaison {
	BSC_LIAISON_UG=1,
	BSC_LIAISON_MAINT=2,
	BSC_LIAISON_IP=3
} BscLiaison ;

typedef struct _bscTraiterBsc {
	int numero;
	Kmutex *mutex_pt;
	Kevent *event_pt;
	Attente phase;
	Ktimestamp tp;
	int timeout;
	uint8 dest[BSC_LG_ADRESSE];
	int fct;
	MessageBsc *messRecu;
	MessageBsc *bscCmd_pt;
	bool selecting;
	bool polling;
	int cpt;
	CommCnx *conn_pt;
	ListeEcouteurs *ecouteursBsc_pt;
} BscTraiteur;

BscTraiteur *bscTraiterNew(int);
void bscTraiterDelete(BscTraiteur *esc);
MessageBsc *bscTraiterTraiterErreur(BscTraiteur *bsc_pt, MessageBsc *mess_pt);
void 		bscTraiterTraiterMessage(BscTraiteur *bsc_pt, MessageBsc *messRecu2);
int 		bscTraiterWrite(CommCnx *conn_pt, char *buffer, int taille); 
int 		bscTraiterWriteMess(BscTraiteur *bsc_pt, MessageBsc *mess_pt);
MessageBsc * bscTraiterTraiterErreur(BscTraiteur *bsc_pt, MessageBsc *mess_pt);
void bscTraiterTraiterMessage(BscTraiteur *bsc_pt, MessageBsc *messRecu2);
void bscTraiterEnvoyerBsc(void *station_p,void *event_p);
bool bscTraiterCaracteresChanged(void *bsc_p, const char *infos, int32 nbCar) ;
void bscTraiterAjouterEcouteurBsc(BscTraiteur *bscTraiter_pt,Ecouteur *traiteurBsc_pt);
void bscTraiterSupprimerEcouteurBsc(BscTraiteur *bscTraiter_pt,Ecouteur *traiteurBsc_pt);
int bscTraiterGetNumero(BscTraiteur *lum_pt);

#endif /*STATIONLUMBSC_H_*/
