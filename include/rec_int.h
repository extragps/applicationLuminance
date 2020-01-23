
/* ************************************************************************	
 * Auteur 	: X.GAILLARD
 * Date 	: 04/02/02
 * Fichier	: rec_int.h
 * Objet	: Prototypage des fonctions des modules d'integration des
 * 			detecteurs.
 * ************************************************************************
 * Historique des modifications
 * ----------------------------
 * 01a,04Feb02,xag  creation.
 * ************************************************************************	*/

#ifndef _REC_INT_H
#define _REC_INT_H

/* ********************************	*/
/* DEFINITION LOCALES				*/
/* ********************************	*/

#define STATUS_OK           0
#define SEUIL_VA_MAX        1
#define SEUIL_TP_MIN        2
#define SEUIL_TP_MAX        3
#define CHEVAUCHEMENT1      4
#define CHEVAUCHEMENT2      5
#define CHEVAUCHEMENT3      6
#define DIE_MIN             0
#define SEUIL_IE11          15
#define SEUIL_IE12          35
#define SEUIL_VA1           150
#define SEUIL_IE_AV2        35         /* seuil en dm de longueur de vehicule   */
#define SEUIL_IE_AR2        12         /* seuil inter-essieu avant en decimetre */
#define SEUIL_IE_AV3        35         /* seuil inter-essieu avant en decimetre */
#define SEUIL_IE_AR3        11         /* seuil inter_essieu arriere en dm      */
#define SEUIL_IE_AV4        11         /* seuil inter-essieu avant en decimetre */
#define SEUIL_IE_AR4        11         /* seuil inter_essieu arriere                        */

/* ********************************	*/
/* DEFINITION DES TYPES				*/
/* ********************************	*/

/* ********************************	*/
/* PROTOTYPAGE DES FONCTIONS		*/
/* ********************************	*/
                                                                                /* Fonctions declarees dans rec_int1 */
INT traiter_fin_coincidence (INT);
INT traiter_anti_coincidence (INT, INT);
VOID traiter_rd (VOID);
INT calc_silh_euro (INT, INT);
                                                                                /* Fonctions declarees dans rec_int0 */
VOID calculer_vilt (INT, INT, INT);
VOID rec_int (VOID);

/* ********************************	*/
/* FIN DE REC_INT.H					*/
/* ********************************	*/

#endif
