/************************************************
*
* ZONE DE DEFINITION DES EQUIVALENCES GENERALES
*
***********************************************
*/
/************************************************
*
* ZONE DE DEFINITION DES EQUIVALENCES GENERALES
*
************************************************/
#ifndef LINUX
#include "vxWorks.h"
#endif
#include "standard.h"                  /* redefinition des types                         */


#define STATION   1

#define CPU132 0                       /* 1 on est en CPU 132 */
                                                /* 0 on est en CPU 332 */
#define CPU332 1                       /* 1 on est en CPU 332 */
                                                /* 0 on est en CPU 132 */

#define PROG_BOOT  0                   /* 0 on n'est pas en programme de boot */
                                                         /* 1 on est en programme de boot */

/* adresse haute memoire RAM */

#define AD_MAX_RAM  0x0080000

#define AD_MAX_CPU  0x400000


/* offset pour bcc version (256 K)*/
#define OFFSET_BCC                0x3FFF8
/* offset pour identification de la version */
#define OFFSET_VERSION            0x3FF00

/* adresse de stockage temporaire de la config en RAM */
extern STRING ad_config_ram;
extern STRING ad_config_flash;

#define AD_CONFIG_RAM             ad_config_ram


#define SZ_CONFIG                  0x10000
#define AD_CONFIG_FLASH1          ad_config_flash1
#define AD_FIN_CONFIG_FLASH1       AD_CONFIG_FLASH1+SZ_CONFIG

                                                                                /* La taille maximale de la confi-
                                                                                 * guration est de 64 Ko pour l'
                                                                                 * instant. Modif de XAG le 23/10/00
                                                                                 */

#define TOP_MEM                    0x0
#define ISR1         0x38000A          /* add du interrupt status register 1  */
#define ISR2         0x38002A          /* add du interrupt status register 2  */
#define IPCR1        0x380008          /* add du input port change register 1 */
#define IPCR2        0x380028          /* add du input port change register 2 */
#define QUARTA       0x380000          /* add de base de la voie A du QUART   */
#define QUARTB       0x380010          /* add de base de la voie B du QUART   */
#define QUARTC       0x380020          /* add de base de la voie C du QUART   */
#define QUARTD       0x380030          /* add de base de la voie D du QUART   */
#define SOPBC1			0x38001C

STRING ad_zpower;

#define AD_ZPOWER    ad_zpower


#define ISR1COM1 		0xE1800A        /* add du interrupt status register 1 */
#define ISR2COM1     0xE1802A          /* add du interrupt status register 2 */
#define IPCR1COM1    0xE18008          /* add du input port change register 1 */
#define IPCR2COM1    0xE18028          /* add du input port change register 2 */

#define QUARTCOM1A   0xE18000          /* add de base de la voie A COM104 1   */
#define QUARTCOM1B   0xE18010          /* add de base de la voie B COM104 1   */
#define QUARTCOM1C   0xE18020          /* add de base de la voie C COM104 1   */
#define QUARTCOM1D   0xE18030          /* add de base de la voie D COM104 1   */

#define SPCR0        0xFFFC18          /* add du control register 0           */
#define SPCR1        0xFFFC1A          /* add du control register 1           */
#define SPCR2        0xFFFC1C          /* add du control register 2           */
#define SPCR3        0xFFFC1E          /* add du control register 3           */
#define SPREC        0xFFFD00          /* add de base de la RCV DATA RAM      */
#define SPTRA        0xFFFD20          /* add de base de TR DATA RAM          */
#define SPCDE        0xFFFD40          /* add de base de la CDE DATA RAM      */

#define TPU_R        0xFFFE00          /* add de base des registres du TPU    */
#define CH_CONT      0xFFFF00          /* add de base de la zone controle des canaux */
#define PIN_LEVEL    0xFFFF02          /* add de base des parametres : niveau des TP */
