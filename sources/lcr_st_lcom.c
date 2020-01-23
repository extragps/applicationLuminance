
/* ************************************************************************
 * Auteur 	: X.GAILLARD
 * Date 	: 12/12/00
 * Fichier	: lcr_st_lcom.c
 * Objet	: Analyse et traitement de la commande ST,LCOM....
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,30Nov00,xag  creation.
 * ************************************************************************	*/

/* ********************************
 * FICHIERS D'INCLUDE
 * ********************************	*/

#include <string.h>
#include <stdio.h>
#include "define.h"
#include "mon_str.h"
#include "mon_debug.h"
#include "lcr_util.h"
#include "lcr_st_divers.h"

/* ********************************
 * DEFINITION DES CONSTANTES LOCALES
 * ********************************	*/

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif


/* ********************************
 * DEFINITION DES TYPES LOCAUX
 * ********************************	*/

/* --------------------------------
 * T_lcrStLcomTbl
 * ==============
 * Table  des commandes reconnues par
 * la station ainsi que la syntaxe.
 * --------------------------------	*/

typedef struct T_lcrStLcomCmd
{
  char *commande;
  char *syntaxe;
} T_lcrStLcomCmd;

/* ********************************
 * DEFINITION DES VARIABLES LOCALES
 * ********************************	*/

static const T_lcrStLcomCmd lcrStLcomTbl[] = {
#ifdef RAD
  {"A", "AI[,[S=]m][,[T=]pt]"},
  {"B", "Bp[,[Q=]q][,[U=]u][,[S=]m][,[T=]pt]"},
  {"CF*", "CF*[,m]"},
  {"CFA", "CFA{{,u[/u]...=p[/p]...}...|,m}"},
  {"CFAC", "CFAC{{,y/y}...|m}"},
  {"CFAL", "CFAL{{,n[[,l],[c=][val]]...}|m}"},
  {"CFC", "CFC{{,y=u[/u]...}...|,m}"},
  {"CFDD", "CFDD{{,y=d}...|m}"},
  {"CFF", "CFF{{,nf=t}...|,m}"},
  {"CFID", "CFID{,u=idf/pwd...|,m}"},
  {"CFLD", "CFLD{{,y=l}...|,m}"},
  {"CFMF", "CFMF{[,ID=pwd],FM=i[/s][,u[p]=f]...|m}"},
  {"CFPP", "CFPP{{,AM=Z.Z,SPC=p}...|,m}"},
  {"CFPU", "CFPU{{,p=v}...|,m}"},
  {"CFS", "CFS{{,nr=v}...|,m}"},
  {"CFV", "CFV{,m|{,x=y[/y]...]...}"},
  {"DT", "DT[,j,h]"},
  {"DATE", "DATE[,j,h]"},
  {"FIN", "FIN"},
  {"ID", "ID{,idf},pwd"},
  {"INIT", "INIT"},
  {"M", "Mp[,[Q=]q][,[U=]u][,[S[=]]m][,[T=]pt]"},
  {"RD", "RD,y[,vr][,lr]"},
  {"SET", "SET{,par[=e]}"},
  {"SETU", "SETU{,par[u]=v...|,m}"},
  {"ST", "ST{,par}"},
  {"ST AL", "ST AL{,m|{,par=v}...}"},
  {"ST LCOM", "ST LCOM[,cmd]"},
  {"ST V", "ST Vx{,par=v}..."},
  {"TRACE", "TRACE[,[N=]n][,[U=]u][,[T=]pt]"},
  {"TST", "TST"},
  {"VA", "VA[,u]"},
#endif
#ifdef PMV
  {"BK", "BK[,m]"},
  {"CFAL", "CFAL{{,n[[,l],[c=][val]]...}|m}"},
  {"CFES", "CFES{[, tc[,AM=vt][,{ss=ve|Z}]...]|[,m]}"},
  {"CFET", "CFET{[,pa[ vt=vc}...]|[m]}"},
  {"CFF", "CFF{{,nf=t}...|,m}"},
  {"CFID", "CFID{,u=idf/pwd...|,m}"},
  {"CFPP", "CFPP{{,AM=Z.Z,SPC=p}...|,m}"},
  {"DT", "DT[,j,h]"},
  {"DATE", "DATE[,j,h]"},
  {"FIN", "FIN"},
  {"ID", "ID{,idf},pwd"},
  {"INIT", "INIT"},
  {"P", "P"},
  {"P1", "P1{,AM=am[,c=p[/p]...]...}"},
  {"PA", "PA{,AM=am[,c=p[/p]...]...}"},
  {"PE", "PE{,AM=am,{c}}"},
  {"PS", "PS{,AM=am,{c}}"},
  {"SET", "SET{,par[=e]}"},
  {"SETU", "SETU{,par[u]=v...|,m}"},
  {"ST", "ST{,par}"},
  {"ST AL", "ST AL{,m|{,par=v}...}"},
  {"ST LCOM", "ST LCOM[,cmd]"},
  {"ST LCPI", "ST LCPI"},
  {"TRACE", "TRACE[,[N=]n][,[U=]u][,[T=]pt]"},
  {"TST", "TST"},
#endif
#ifdef RAD
  {"BK", "BK[,m]"},
  {"CFAL", "CFAL{{,n[[,l],[c=][val]]...}|m}"},
  {"CFES", "CFES{[, tc[,AM=vt][,{ss=ve|Z}]...]|[,m]}"},
  {"CFET", "CFET{[,pa[ vt=vc}...]|[m]}"},
  {"CFF", "CFF{{,nf=t}...|,m}"},
  {"CFID", "CFID{,u=idf/pwd...|,m}"},
  {"CFPP", "CFPP{{,AM=Z.Z,SPC=p}...|,m}"},
  {"DT", "DT[,j,h]"},
  {"DATE", "DATE[,j,h]"},
  {"FIN", "FIN"},
  {"ID", "ID{,idf},pwd"},
  {"INIT", "INIT"},
  {"P", "P"},
  {"P1", "P1{,AM=am[,c=p[/p]...]...}"},
  {"PA", "PA{,AM=am[,c=p[/p]...]...}"},
  {"PE", "PE{,AM=am,{c}}"},
  {"PS", "PS{,AM=am,{c}}"},
  {"SET", "SET{,par[=e]}"},
  {"SETU", "SETU{,par[u]=v...|,m}"},
  {"ST", "ST{,par}"},
  {"ST AL", "ST AL{,m|{,par=v}...}"},
  {"ST LCOM", "ST LCOM[,cmd]"},
  {"ST LCPI", "ST LCPI"},
  {"TRACE", "TRACE[,[N=]n][,[U=]u][,[T=]pt]"},
  {"TST", "TST"},
#endif
  {NULL, NULL}
};


/* ********************************
 * FONCTIONS DU MODULE
 * ********************************	*/

/* --------------------------------
 * tst_simu_dial
 * =============
 * Analyse de la commande "TST SIMU".
 * Entree :
 * - numero de la liaison sur lequel
 *   se fait le transfert.
 * - mode : mode de transmission,
 * - nombre de caracteres dans le
 *   buffer,
 * - position dans le buffer,
 * - pointeur vers un message de
 *   transmission
 * --------------------------------	*/

void
lcr_st_lcom (INT las, INT mode, INT lg_mess, STRING buffer, INT position,
             struct usr_ztf *pt_mess)
{
  char *ptCour = (char *) &buffer[position];
  int lgCour = lg_mess - position;
  int trouve = 0;
  int indice = 0;
  int nbCar = 0;
  int bloc = 0;

  ptCour += 7;
  lgCour -= 7;
  ptCour = tst_passe_blanc (ptCour, lgCour, &lgCour);

  while ((lcrStLcomTbl[indice].commande != NULL) && (!trouve))
    {
      if (strncmp (lcrStLcomTbl[indice].commande, ptCour,
                   strlen (lcrStLcomTbl[indice].commande)) == 0)
        {
          tst_send_bloc (las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess,
                         "%s", lcrStLcomTbl[indice].syntaxe);
          trouve = 1;
        }
      indice++;
    }
  if (!trouve)
    {
      indice = 0;
      while (lcrStLcomTbl[indice].commande != NULL)
        {
          tst_send_bloc (las, mode, buffer, &nbCar, &bloc, FALSE, pt_mess,
                         "%s\n\r", lcrStLcomTbl[indice].commande);
          indice++;
        }
      tst_send_bloc (las, mode, buffer, &nbCar, &bloc, TRUE, pt_mess, "");
    }
}
