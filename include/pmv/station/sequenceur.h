#ifndef SEQUENCEUR_H_
#define SEQUENCEUR_H_

#define SEQUENCEUR_ETAT_OK	0
#define SEQUENCEUR_DEFAUT	1	
#define SEQUENCEUR_DEFAUT_MINEUR	2
#define SEQUENCEUR_DEFAUT_MAJEUR	3

typedef struct _Sequenceur Sequenceur;

void sequenceurAjouterEcouteur(Sequenceur *seq_pt,Ecouteur *ecouteur_pt);
void sequenceurSupprimerEcouteur(Sequenceur *seq_pt,Ecouteur *ecouteur_pt);
Sequenceur *sequenceurNew();
void sequenceurDelete(Sequenceur *);
void sequenceurStart(Sequenceur *);
void sequenceurStop(Sequenceur *);
void sequenceurReprogrammerEvent(Sequenceur *sequenceur_pt,Event *evt_pt);
void sequenceurAjouterEvent(Sequenceur *seq_pt,Event *evt_pt);
void sequenceurSupprimerEvent(Sequenceur *sequenceur_pt,Event *evt_pt);
void sequenceurSetReinit(bool val);
bool sequenceurIsReinit(void);

#endif /*SEQUENCEUR_H_*/
