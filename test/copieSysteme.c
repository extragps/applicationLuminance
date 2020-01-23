/**********************************************************************************************************************
 * SIAT - Copyright SIAT
 * --------------------------------------------------------------------------------------------------------------------
 * fichier     : copieSysteme.c
 * description : Le fichier est a l'origine le programme permettant la mise a jour de l'ensemble
 * 		systeme et application a partir du systeme lui meme.
 * 		Dans la version courante, le programme permet en plus de calculer le md5 d'un fichier et de
 * 		lire la version firmware d'une carte COM483.
 * 		A terme, il serait peut être interessant de mettre ce fichier directement dans linux (comme
 * 		le watchdog).
 * prefixe     :
 * --------------------------------------------------------------------------------------------------------------------
 * Auteur      : XGAILLARD
 * Date        : 02/04/2009
 * OS          : uClinux
 * --------------------------------------------------------------------------------------------------------------------
 * $Log: copieSysteme.c,v $
 * Revision 1.2  2018/06/04 08:38:42  xg
 * Passage en version V122
 *
 * Revision 1.4  2009/09/17 08:15:28  xgaillard
 * Mise a jour.
 *
 * Revision 1.9  2009/08/31 13:46:44  xgaillard
 * Ajout d'un indicateur de progression comme pour le eraseall
 *
 * Revision 1.8  2009/08/31 13:38:35  xgaillard
 * Ajout d'un indicateur de progression
 *
 * Revision 1.7  2009/04/17 15:20:55  xag
 * Ajout de l'option -big pour permettre de depasser la limite fixee de 0x400000.
 * Pour info, la limite max est 0x700000. Elle correspond a la taille reservee au systeme dans le file system.
 *
 * Revision 1.6  2009/04/17 15:16:42  xag
 * On limite maintenant la taille maximale du fichier a claquer....
 *
 * Revision 1.5  2009/04/09 17:34:32  sgronchi
 * correction type buffer
 *
 * Revision 1.4  2009/04/09 14:34:04  xag
 * Ajout du traitement MD5
 *
 * Revision 1.3.4.1  2009/04/02 08:02:58  xag
 * Integration de la lecture de la version firmware des COM485 et du calcul MD5
 *
 *********************************************************************************************************************/

/* ********************************
 * INCLUDES DU FICHIER
 * ******************************** */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "kcommon.h"
#include "global.h"
#include "md5.h"

/* ********************************
 * DEFINITIONS DU MODULE
 * ******************************** */

#define MAX_SIZE 0x400000
#define BIG_SIZE 0x700000

#define TAILLE_TAMPON 4096

/* ********************************
 * FONCTIONS DU MODULE
 * ******************************** */

int main(int argc, char **argv) {

	FILE *origine = NULL;
	int destination;
	int indice = 1;
	char *nomFichier = "stationLum";
	char *destDevice = "/dev/mtd1";
	char *device = "/dev/ttyN0"; /* Le device par defaut pour le controle du firmware */
	bool md5_b = false; /* Flag indicateur du test MD5 */
	bool firmware_b = false; /* Flag indicateur de la lecture de la version firmware. */
	bool fichier_b = false;
	unsigned long copieMaxSize = MAX_SIZE;

	/* *************************************************
	 * ANALYSE DES PARAMETRES DE LA LIGNE DE COMMANDE
	 * ************************************************* */

	while (indice < argc) {
		if (0 == strcmp("-fichier", argv[indice])) {
			if (++indice < argc) {
				nomFichier = argv[indice];
				fichier_b = true;
			}
		} else if (0 == strcmp("-dest", argv[indice])) {
			if (++indice < argc) {
				destDevice = argv[indice];
			}
		} else if (0 == strcmp("-device", argv[indice])) {
			if (++indice < argc) {
				device = argv[indice];
			}
		} else if (0 == strcmp("-big", argv[indice])) {
			copieMaxSize = BIG_SIZE;
		} else if (0 == strcmp("-firmware", argv[indice])) {
			firmware_b = true;
		} else if (0 == strcmp("-md5", argv[indice])) {
			md5_b = true;
		}

		else if (0 == strcmp("-help", argv[indice])) {
			printf(
					"Syntaxe : %s -fichier nomFichier -dest deviceMtd | -fichier nomFichier -md5 | -md5 | -big -firmware -device nomDevice\n",
					argv[0]);
			return 0;
		}
		indice++;
	}
	/* ****************************************************
	 * LECTURE DE LA VALEUR DU FIRWARE
	 * **************************************************** */
	if (firmware_b) {
		int desc = open(device, O_RDWR);
		if (-1 == desc) {
			printf("Firmware : Erreur d'ouverture du device %s\n", device);
		} else {
			//		int firmware;
			char version;
			if (-1 == ioctl(desc, 0x400 + 73, &version)) {
				printf("Firmware : Erreur d'ioctl %d du device %s\n", 0x400
						+ 73, device);
				perror("L'erreur");
			} else {
				printf("Firmware : device %s version %#02x\n", device, version);
			}
			close(desc);
		}

	} else if (md5_b) {
		if (fichier_b) {
			unsigned char digestOri[16];
			MD5FileSize(nomFichier, digestOri, copieMaxSize);
			printf("MD (%s)  : ", nomFichier);
			MD5Print(digestOri);
			printf("\n");
		} else {
			destination = open(destDevice, O_RDWR);
			if (-1 == destination) {
				perror(destDevice);

			} else {
				/* Calcul de la somme md5 du fichier systeme courant */
				if (-1 != lseek(destination, 0, SEEK_SET)) {
					long nbCarEcrits = 0;
					bool error_b = false;
					MD5_CTX contexte;
					unsigned char buffer[TAILLE_TAMPON];
					long progression = 0;
					MD5Init(&contexte);
					while ((nbCarEcrits < copieMaxSize) && (false == error_b)) {
						int aLire = MIN(TAILLE_TAMPON,copieMaxSize-nbCarEcrits);
						unsigned int nbCarLus = 0;
						nbCarLus = read(destination, buffer, aLire);
						if (nbCarLus != aLire) {
							printf("Probleme de lecture ");
							error_b = true;
						} else {
							MD5Update(&contexte, buffer, nbCarLus);
							/* Imprimer ici la progression.... */
						}
						nbCarEcrits += nbCarLus;
						{
							long courant = (nbCarEcrits * 100) / copieMaxSize;
							if (courant != progression) {
								progression = courant;
								printf("\rMD5 calcul -- %ld %% en cours",
										progression);
							}
						}
					}

					if (false == error_b) {
						unsigned char digest[16];
						MD5Final(digest, &contexte);
						printf("\nMD Firmware   : ");
						MD5Print(digest);
						printf("\n");
					}
					close(destination);
				}

			}
		}
	} else {
		destination = open(destDevice, O_RDWR);
		if (-1 == destination) {
			perror(destDevice);

		} else {

			char commande[500];
			unsigned char digestOri[16];
			unsigned char digestFin[16];
			close(destination);
			printf("CALCUL MD5 du fichier %s\n", nomFichier);

			if (-1 != MD5FileSize(nomFichier, digestOri, copieMaxSize)) {
				/* Il faut effacer le device destination */
				snprintf(commande, 500, "eraseall %s", destDevice);
				if (-1 == system(commande)) {
					printf("Probleme d'effacement du device %s\n", destDevice);
				} else {

					/* Recopie du fichier */
					destination = open(destDevice, O_RDWR);

					origine = fopen(nomFichier, "r");
					if ((NULL != origine) && (-1 != destination)) {
						int nbCarTot = 0;
						if (-1 != lseek(destination, 0, SEEK_SET)) {
							long progression = 0;
							unsigned char buffer[TAILLE_TAMPON];
							while (!feof(origine)) {
								int nbCarLus = fread(buffer, 1, TAILLE_TAMPON,
										origine);
								write(destination, buffer, nbCarLus);
								nbCarTot += nbCarLus;
								{
									long courant = (nbCarTot * 100)
											/ copieMaxSize;
									if (courant != progression) {
										progression = courant;
										printf(
												"\rMD5 copie -- %ld %% en cours",
												progression);
									}
								}
							}
						} else {
							printf("Probleme de deplacement\n");
						}
						fclose(origine);
						/* Controle du MD5 */
						if (-1 != lseek(destination, 0, SEEK_SET)) {
							long progression = 0;
							int nbCarEcrits = 0;
							bool error_b = false;
							MD5_CTX contexte;
							MD5Init(&contexte);
							while ((nbCarEcrits < copieMaxSize) && (false
									== error_b)) {
								unsigned char buffer[TAILLE_TAMPON];
								int
										aLire =
												MIN(TAILLE_TAMPON,copieMaxSize-nbCarEcrits);
								unsigned int nbCarLus = 0;
								nbCarLus = read(destination, buffer, aLire);
								if (nbCarLus != aLire) {
									printf("Probleme de lecture ");
									error_b = true;
								} else {
									MD5Update(&contexte, buffer, nbCarLus);
								}
								nbCarEcrits += nbCarLus;
								{
									long courant = (nbCarEcrits * 100)
											/ copieMaxSize;
									if (courant != progression) {
										progression = courant;
										printf(
												"\rMD5 controle -- %ld %% en cours",
												progression);
									}
								}
							}

							if (false == error_b) {
								MD5Final(digestFin, &contexte);

								printf("\nMD fichier : ");
								MD5Print(digestOri);
								printf("\n");
								printf("MD copie   : ");
								MD5Print(digestFin);
								printf("\n");
							}

						}

						close(destination);

						printf("Nombre de caracteres ecrits %d\n", nbCarTot);
					} else {
						perror(nomFichier);
						printf("Probleme d'ouverture %x %d \n",
								(unsigned int) origine, destination);
					}
				}

			} else {
				printf("Erreur sur le fichier %s\n", nomFichier);
			}
		}
	}
	return 0;
}
