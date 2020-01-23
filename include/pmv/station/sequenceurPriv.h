#ifndef SEQUENCEUR_PRIV_H_
#define SEQUENCEUR_PRIV_H_

struct _Sequenceur
{
	/* Evenement utilise pour la synchronisation et le traitement */
	Kevent *event_pt;
	/* Mutex utilise pour la protection des acces concurents a la liste */
	Kmutex *mutex_pt;
	/* La tache de traitement. */
//	Kthread *thread_pt;
//	/* Liste des evenements a traiter. */
	Klist *listeEvts_pt;
	/* Message en cours de traitement */
	Ktimestamp ts_t;
	/* Les numeros de message */
	int32 numMessCour_dw[NB_CAISSON];
	int32 numMessNext_dw[NB_CAISSON];
	/* Indicateur de message en cours de programmation */
	bool enCours_b[NB_CAISSON];
	int32 etatComm_dw;
	int32 etatThl_dw[AFFICHEUR_NB_MAX];
	int32 etatAff_dw[AFFICHEUR_NB_MAX];
	bool testAnim_b;
	bool reinit_b;
	bool tempoTestActivation_b;
	/* Pour le transfert des messages luminance. */
	ListeEcouteurs *ecouteursLum_pt;

} ;

#endif /*SEQUENCEUR_PRIV_H_*/
