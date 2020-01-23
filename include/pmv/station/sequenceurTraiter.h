#ifndef SEQUENCEUR_TRAITER_H_
#define SEQUENCEUR_TRAITER_H_

void sequenceurTraiterAffichageCaisson(int numCaisson);
int32 sequenceurTraiterGetValiditeCaisson(int numCaisson);
bool sequenceurTraiterChangementMessage(int numCaisson);
bool sequenceurGetModeLuminosite(int32 numCaisson);
int32 sequenceurGetValeurLuminosite(int32 numCaisson);
bool sequenceurTraiterLestage(Sequenceur *seq_pt, int numCaisson);

#endif /* SEQUENCEUR_TRAITER_H_ */
