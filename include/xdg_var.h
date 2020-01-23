#include "xdg_str.h"
/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES                */
/**                                                                       **/
/***************************************************************************/
/* table de configuration des LAS  */
GLOBALREF T_xdg_cf_las xdg_cf_las[];
GLOBALREF T_xdg_cf_las xdg_sv_cf_las[];

/* table de gestion des las LAS  */
GLOBALREF T_xdg_las xdg_las[];
GLOBALREF T_xdg_vt xdg_vt[];

GLOBALREF struct xdg_ctrl_rack xdg_ctrl_rack;

GLOBALREF struct xdg_ctrl_pile xdg_ctrl_pile;

GLOBALREF struct xdg_reprise_secteur xdg_reprise_secteur;

GLOBALREF UINT8 xdg_trace[];

GLOBALREF UINT8 xdg_reinit;
