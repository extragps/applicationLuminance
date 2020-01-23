#ifndef TRAITEUR_H_
#define TRAITEUR_H_

typedef struct _Traiteur
{
	/* Liste des utilisateurs des messages luminances */
	Sequenceur *sequenceur_pt;
	/* Liste des utilisateurs des reponses bsc. */
	ListeEcouteurs *ecouteursBsc_pt;
	/* Liste de travail... */
	Klist *listeCmd_pt;
	/* Evenement pour signaler le thread de traitement */
	Kevent *event_pt;
	/* Un mutex pour l'acces concurrent a la liste des commandes */
	Kmutex *mutex_pt;
	/* Identifiant du thread utilisé pour le traitement */
	Kthread *thread_pt;
	bool fin_b;
}	Traiteur;

Traiteur *traiteurNew();
void traiteurDelete(Traiteur *traiteur_pt);
void traiteurAjouterEcouteurBsc(Traiteur *traiteur_pt,Ecouteur *ecouteur_pt);
void traiteurSupprimerEcouteurBsc(Traiteur *traiteur_pt,Ecouteur *ecouteur_pt);
void traiteurEcouterMessageBsc(void *,void *);
void traiteurAjouterEcouteurLum(Traiteur *traiteur_pt,Ecouteur *ecouteur_pt);
void traiteurSupprimerEcouteurLum(Traiteur *traiteur_pt,Ecouteur *ecouteur_pt);
void traiteurStart(Traiteur *traiteur_pt);
void traiteurStop(Traiteur *traiteur_pt);
bool traiteurGetFin(Traiteur *traiteur_pt);
void traiteurSetFin(Traiteur *traiteur_pt,bool val_b);
void traiteurSetSequenceur(Traiteur *,Sequenceur *);


BscCmd *traiterAfficher(Traiteur *traiteur_pt,BscCmd *bscCmd_pt);
BscCmd *traiterLireHeure(Traiteur *traiteur_pt,BscCmd *bscCmd_pt);
BscCmd *traiterEcrireHeure(Traiteur *traiteur_pt,BscCmd *bscCmd_pt);
void traiterExtinction(Traiteur *traiteur_pt);
void traiterNeutre(Traiteur *traiteur_pt);
BscCmd *traiterAffichageNeutre(void);
BscCmd * traiterActiver(Traiteur *,BscCmd *);
BscCmd *traiterReponseOk(Traiteur *traiteur_pt,BscCmd *cmd_pt);
BscCmd *traiterAfficherGlobal(Sequenceur *seq_pt,BscCmd *bscCmd_pt,bool enregistrer_b);
void traiterExtinctionGlobal(Sequenceur *seq_pt);

#endif /*TRAITEUR_H_*/
