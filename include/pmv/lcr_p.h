#ifndef INCLUDE_LCR_P
#define INCLUDE_LCR_P

/* *******************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

#define PE_AF	0x01
#define PE_AT	0x20
#define PE_CL	0x10
#define PE_DV	0x04
#define PE_EC	0x08
#define PE_MM	0x02
#define PE_TOUS_PARAM 	0x3F
#define PE_TOUS_SAUF_MM 0x3D

#define PE_ENTREE 0
#define PE_SORTIE 1
#define PE_AFF 	  2

/* *******************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

void lcr_p (INT, INT, INT, STRING, INT, struct usr_ztf *);
void lcr_pa (INT, INT, INT, STRING, INT, struct usr_ztf *);
void lcr_pe (INT, INT, INT, STRING, INT, struct usr_ztf *);
int lcr_pe_param(int,int,int,int,STRING,INT,INT);
void cmd_exec_p (T_pip_cmd_p *cmd_p);
void cmd_init_p (T_pip_cmd_p *cmd_p);
int activation_caisson(INT);
void activation_caisson_lire_action(INT indCaisson,T_pip_sv_act_caisson *sv_action);
int pip_car_dec(T_pip_car *,STRING);
int pip_car_is_null(T_pip_car *);
int pipGetEtat(void);
void pipSetEtat(int etat);

#endif
