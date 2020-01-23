#ifndef LUMTRAITER_H_
#define LUMTRAITER_H_

typedef struct _LumTraiteur
{
	/* Evenement utilise pour la synchronisation et le traitement */
	Kevent *event_pt;
	/* Mutex utilise pour la protection des acces concurents a la liste */
	Kmutex *mutex_pt;
	/* La tache de traitement. */
	Kthread *thread_pt;
	/* Liste des reponses luminance a traiter ... */
	Klist *listeReponses_pt;
	/* Liste des messages luminance a traiter ... */
	Klist *listeMessages_pt;
	/* Numero d'affichage courant */
	int32 numAffCour_dw;
	/* Liste des messages d'affichage... */
	Klist *listeAffichages_pt;
	/* Message en cours de traitement */
	LumMsg *messCour_pt;
	/* Chargement du buffer d'affichage. */
	bool messAff_b;
	/* Date d'emission du message */
	uint32 lastTick_dw;
//	Ktimestamp ts_t;
	/* Sequenceur */
	Sequenceur *sequenceur_pt;
	/* Pour le transfert des messages luminance. */
	ListeEcouteurs *ecouteursLum_pt;
	/* Pour le transfert des messages BSC... */
	LumMsg *cour_pt;
	/* Canal de communication */
	CommCnx *conn_pt;

} LumTraiteur;

LumTraiteur *lumTraiteurNew();
void lumTraiteurDelete(void *);
void lumTraiteurSetComm(LumTraiteur *traiteur_pt,CommCnx *conn_pt);
void lumTraiterAjouterLumMessage(void *traiteur_p,void *event_p);
void lumTraiterAjouterLumReponse(void *traiteur_p,void *event_p);
void lumTraiterAjouterLumAffichage(void *traiteur_p,void *event_p);
void lumTraiteurStart(LumTraiteur *traiteur_pt);
void lumTraiteurStop(LumTraiteur *traiteur_pt);
void lumTraiteurSetSequenceur(LumTraiteur *traiteur_pt,Sequenceur *seq_pt);
LumTraiteur *lumTraiteurGet(void);

#endif /*LUMTRAITER_H_*/
