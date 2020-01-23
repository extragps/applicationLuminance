/* XG le 27/10/05 : Creation du fichier pour integration des prototypes des
 * fonctions du module tac_ctrl.c
 * */
#ifndef INCLUDE_TAC_CTRL
#define INCLUDE_TAC_CTRL

INT tac_ctrl_emission (STRING, INT);
VOID tac_ctrl_coupure_secteur (VOID);
VOID tac_ctrl_init (VOID);
VOID tac_ctrl_io (VOID);
BYTE tac_ctrlLocalDistant (INT);
VOID tac_ctrl_main (int);
void tac_ctrl_set_force(int val);
int tac_ctrl_get_force(void);


#endif
