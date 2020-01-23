/* **************************************************************************
 * Module : pip_util.h
 * Createur :  X.Gaillard
 * Date : Le 22 Novembre 2005
 * Objet : Prototype des fonctions du module pip_util.c
 * ************************************************************************	*/

#ifndef INCLUDE_PIP_UTIL
#define INCLUDE_PIP_UTIL
void pip_init(bool);
int pip_util_ind_caisson (int ind);
int pip_util_ind_deb_topo (int mod, int cais);
int pip_util_ind_fin_topo (int mod, int cais);
int pip_util_ind_ctrl_topo (int mod, int cai);
int pip_util_ind_deb_const (int mod, int cais);
int pip_util_ind_fin_const (int mod, int cais);
int pip_util_ind_deb (int mod, int cais);
int pip_util_ind_fin (int mod, int cais);
int pip_util_ind_ctrl (int mod, int cais);
int pip_util_j3(char);
int pip_util_j6(char);
int pip_util_g2(char);
int pip_util_g2_accent(char);
int pip_util_test_nom_symbole(char *);
int pip_util_test_symbole(int,int,char *);
int pip_util_test_symbole_ec(int,int,char *);
int pip_util_test_litteral(int,int,char *);
int pip_util_test_numerique(int,int,char *);
void pip_clig_set(T_pip_clig *,UINT8,int ,int );
void pip_alt_set(T_pip_alt *,UINT8 ,int ,int ,int ,int );
int pip_alt_imprimer(STRING buffer,T_pip_alt *alt);
int pip_dv_imprimer(STRING buffer,long val,int fmtLong);
int pip_clig_imprimer(STRING buffer,T_pip_clig *clig);

#endif
