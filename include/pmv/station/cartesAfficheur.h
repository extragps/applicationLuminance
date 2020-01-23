#ifndef CARTES_PRIVATE_H_
#define CARTES_PRIVATE_H_

#include "infos/lumInfos.h"
#include "affichage/zone.h"
#include "affichage/page.h"
#include "affichage/affichage.h"
#include "affichage/afficheur.h"
#include "es/es.h"
#include "thl/thl.h"

bool afficheursSetAffichage(int,Affichage *);
Afficheur *afficheursChercherParAdresse(int adresse);
Afficheur *afficheursChercherParIndex(int index);
Es *essChercherParAdresse(int adresse);
Es *essChercherParIndex(int index);
Thl *thlsChercherParAdresse(int adresse);
Thl *thlsChercherParIndex(int adresse);

#endif /*CARTES_PRIVATE_H_*/
