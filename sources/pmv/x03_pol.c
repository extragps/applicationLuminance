/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES               **/
/**                                                                       **/
/***************************************************************************/
#include "standard.h"
#include "mon_inc.h"
#include "xdg_def.h"
#include "xdg_str.h"
#include "xdg_var.h"
#include "pip_def.h"
#include "pip_str.h"
#include "pip_var.h"

/* Valeurs par defaut pour les seuils des cellules */

T_pip_seuil_cellule pip_seuil_cellule[NB_SEUIL_CELLULE] =
{
#ifdef CPU432
{ 1, 10, -1 }, /* SEUIL Minimal */
{ 100, 50, 0 }, /*  SEUIL 3 */
{ 500, 100,0 }, /*  SEUIL 5 */
{ 1250, 110, 0 }, /*  SEUIL 7 */
{ 2000, 120, 1 }, /*  SEUIL 9 */
{ 2500, 130, 1 }, /*  SEUIL 1 */
{ 3200, 140, 1 }, /*  SEUIL 3 */
{ 3750, 160, 1 }, /*  SEUIL 5 */
{ 4500, 180, 2 }, /*  SEUIL 7 */
{ 5000, 187, 2 }
#else
{ 1, 100, -1 }, /* SEUIL Minimal */
{ 100, 120, 0 }, /*  SEUIL 3 */
{ 500, 140, 0 }, /*  SEUIL 5 */
{ 1250, 170, 0 }, /*  SEUIL 7 */
{ 2000, 200, 1 }, /*  SEUIL 9 */
{ 2500, 210, 1 }, /*  SEUIL 1 */
{ 3200, 220, 1 }, /*  SEUIL 3 */
{ 3750, 230, 1 }, /*  SEUIL 5 */
{ 4500, 240, 2 }, /*  SEUIL 7 */
{ 5000, 250, 2 }
#endif
 };


