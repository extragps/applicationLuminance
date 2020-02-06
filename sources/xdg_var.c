#ifdef VXWORKS
#include "vxworks.h"
#endif
#include "standard.h"
#include "mon_def.h"
#include "mon_str.h"
#include "xdg_def.h"
#include "xdg_str.h"


/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES                */
/**                                                                       **/
/***************************************************************************/
#if STATION
/*#option sep_on segment xdg_var xdg_var class ram_xdg ram_xdg*/
#endif

/* table de configuration des LAS  */
struct xdg_cf_las xdg_cf_las[16];
struct xdg_cf_las xdg_sv_cf_las[16];

/* table de gestion des las LAS  */
struct xdg_las xdg_las[16];
struct xdg_ctrl_rack xdg_ctrl_rack;

struct xdg_vt xdg_vt[NBPORT];

#if STATION
/*#option sep_off*/
#endif
