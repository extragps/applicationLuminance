#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "kcommon.h"
#include "klog.h"
#include "standard.h"
#include "tac_conf.h"

#ifdef CPU432
#define REP_SOURCE "/home"
#define REP_SOURCE_CFG "/etc"
#define REP_DESTINATION "/var/flash"
#else
#define REP_SOURCE "."
#define REP_SOURCE_CFG "."
#define REP_DESTINATION "."
#endif

/* Ce n'est pas bien, il faudrait inclure un fichier plutot. */

static unsigned long calculer_checksum_fichier(const char *nomFichier)
{
	long bcc = 0;
	FILE *desc_pt = NULL;
	desc_pt = fopen(nomFichier, "r");
	if (NULL!=desc_pt)
	{
		struct stat infos;
		if (-1 != fstat(fileno(desc_pt), &infos))
		{
			long taille = infos.st_size;
			long nbCarTot = 0;
			long nbCarLus = 0;
			if (0 < taille)
			{
				char buffer[1024];
				while ((nbCarTot < taille) && (-1 != nbCarLus))
				{
					nbCarLus = fread(buffer, sizeof(char), MIN(1024, taille
							- nbCarTot), desc_pt);
					if (-1 != nbCarLus)
					{
						int indice = 0;
						for (indice = 0; indice < nbCarLus; indice++)
						{
							bcc += buffer[indice];
						}
						nbCarTot += nbCarLus;
					}
					else
					{
						/* Il y a erreur */
						printf("checksum_fichier: il y a erreur %ld/%ld\n",
								taille, nbCarTot);
						bcc = 0;
					}
				}
			}

		}
		else
		{
			printf("calculer_checksum_fichier: erreur de fstat %d\n", errno);
		}
		fclose(desc_pt);
	}
	else
	{
		printf(
				"calculer_checksum_fichier: erreur d'ouverture du fichier %s (errno: %d)\n",
				nomFichier, errno);
	}
	return bcc;
}
/**--------------------------------------------------------------------------------------------------------------------
 *
 */
static void buildln(const char * nomFichier, const char * nomFichierDefaut)
{
	struct stat infos;
	char nomFichierNew[512];
	char commande[500];
	bool dejaFait_b = false;
	T_tacConfFichiers confFichier;

	/* On revient a la version claquee sur le disque rom */
	if (cpu432IsResetTriggered())
	{
		sprintf(commande, "mv %s %s.old", nomFichier, nomFichier);
		if (-1 == system(commande))
		{
			printf("Reset config : erreur de renommage du fichier\n");
		}
		else
		{
			printf("Suppression version courante\n");
		}
	}
	else
	{
		printf("Pas de reset de fichier\n");
	}

	if (-1 != tac_conf_cfg_lire_fichier(&confFichier))
	{
		if (0 != confFichier.exploit[0])
		{
			snprintf(nomFichierNew, 512, "versions/%s", confFichier.exploit);
			sprintf(commande, "mv %s %s.old", nomFichier, nomFichier);
			if (-1 == system(commande))
			{
				printf("Erreur de renommage du fichier\n");
			}
			else
			{
				sprintf(commande, "cp %s %s", nomFichierNew, nomFichier);
				if (-1 == system(commande))
				{
					printf("Erreur de recopie du nouveau fichier\n");
				}
				else
				{
					sprintf(commande, "chmod 777 %s", nomFichier);
					if (-1 == system(commande))
					{
						printf(
								"Erreur de changement de mode du nouveau fichier\n");
					}
					else
					{
						int indice;
						struct timeval heure;
						struct timezone zone;
						for (indice = 0; indice < TAC_CONF_FICHIER; indice++)
						{
							confFichier.courant[indice]
									= confFichier.exploit[indice];
							confFichier.exploit[indice] = 0;
						}
						gettimeofday(&heure, &zone);
						confFichier.courantDate = heure.tv_sec;
						confFichier.courantBcc = calculer_checksum_fichier(
								nomFichier);
						tac_conf_cfg_ecr_fichier(&confFichier);
						dejaFait_b = true;
					}
				}
			}

		}
		else
		{
			printf("Pas de fichier a mettre en exploitation\n");
			/* Effectuer ici peut être un contrôle du fichier.... */
		}

	}
	else
	{
		/* Le fichier si il n'existait pas a ete automatiquement cree par la
		 * commande de lecture.... la structure confFichier a ete initialisee
		 * */
		printf("Probleme de lecture du fichier de configuration\n");

	}

	snprintf(nomFichierNew, 512, "%s.new", nomFichier);
	if (-1 != stat(nomFichierNew, &infos))
	{
		/* Il y a un nouvelle version disponible.... */
		sprintf(commande, "mv %s %s.old", nomFichier, nomFichier);
		if (-1 == system(commande))
		{
			printf("Erreur de renommage du fichier\n");
		}
		else
		{
			sprintf(commande, "mv %s.new %s", nomFichier, nomFichier);
			if (-1 == system(commande))
			{
				printf("Erreur de renommage du nouveau fichier\n");
			}
			else
			{
				sprintf(commande, "chmod 777 %s", nomFichier);
				if (-1 == system(commande))
				{
					printf("Erreur de changement de mode du nouveau fichier\n");
				}
				else
				{
					struct timeval heure;
					struct timezone zone;
					snprintf(confFichier.courant, TAC_CONF_FICHIER, "%s",
							nomFichier);
					gettimeofday(&heure, &zone);
					confFichier.courantDate = heure.tv_sec;
					confFichier.courantBcc = calculer_checksum_fichier(
							nomFichier);
					tac_conf_cfg_ecr_fichier(&confFichier);
					dejaFait_b = true;
				}
			}
		}
	}

	if (-1 != stat(nomFichier, &infos))
	{
		unsigned long bcc;
		if (!(S_ISREG(infos.st_mode)&&(S_IRWXO&infos.st_mode)))
		{
			printf("Droits incorrect pour l'execution de %s\n", nomFichier);
		}
		else
		{
			printf("Le fichier existe %s\n", nomFichier);
		}
		if (false == dejaFait_b)
		{
			bcc = calculer_checksum_fichier(nomFichier);
			if (bcc != confFichier.courantBcc)
			{
				struct timeval heure;
				struct timezone zone;
				printf("Defaut de checksum du fichier %s\n", nomFichier);
				snprintf(confFichier.courant, TAC_CONF_FICHIER, "%s",
						nomFichier);
				gettimeofday(&heure, &zone);
				confFichier.courantDate = heure.tv_sec;
				confFichier.courantBcc = bcc;
				tac_conf_cfg_ecr_fichier(&confFichier);
			}
		}

	}
	else
	{
		printf("Fichier non trouve %s\n", nomFichier);
		sprintf(commande, "ln -s %s %s", nomFichierDefaut, nomFichier);
		printf("Execution de la commnade : %s\n", commande);
		if (-1 == system(commande))
		{
			printf("Erreur de creation du lien\n");
		}
		else
		{
			struct timeval heure;
			struct timezone zone;
			snprintf(confFichier.courant, TAC_CONF_FICHIER, "%s", nomFichier);
			gettimeofday(&heure, &zone);
			confFichier.courantDate = heure.tv_sec;
			confFichier.courantBcc = calculer_checksum_fichier(nomFichier);
			tac_conf_cfg_ecr_fichier(&confFichier);
		}
	}
}

/**--------------------------------------------------------------------------------------------------------------------
 *
 */
static void launchSh(const char * nomFichier_pc,
		const char * nomFichierDefaut_pc)
{
	struct stat statfile_t;
	char commande_ac[256];

	if (-1 != stat(nomFichier_pc, &statfile_t))
	{
		if (S_ISREG( statfile_t.st_mode ) && (S_IROTH & statfile_t.st_mode))
		{
			sprintf(commande_ac, "sh %s", nomFichier_pc);
			if (-1 == system(commande_ac))
			{
				printf("Erreur d'execution de la commande '%s' \n", commande_ac);
			}
			else
			{
				printf("Commande '%s' executee \n", commande_ac);
			}
		}
		else
		{
			printf("Droits incorrects pour l'execution de '%s' \n",
					nomFichier_pc);
		}
	}
	else if ((null != nomFichierDefaut_pc) && (-1 != stat(nomFichierDefaut_pc,
			&statfile_t)))
	{
		if (S_ISREG( statfile_t.st_mode ) && (S_IROTH & statfile_t.st_mode))
		{
			sprintf(commande_ac, "sh %s", nomFichierDefaut_pc);
			if (-1 == system(commande_ac))
			{
				printf("Erreur d'execution de la commande '%s' \n", commande_ac);
			}
			else
			{
				printf("Commande '%s' executee \n", commande_ac);
			}
		}
		else
		{
			printf("Droits incorrects pour l'execution de '%s' \n",
					nomFichierDefaut_pc);
		}
	}
	else
	{
		if(null!=nomFichierDefaut_pc)
		{
			klogPut( null, LOG_WARNING, "Fichiers inexistants (%s et %s).", nomFichier_pc, nomFichierDefaut_pc );
		}
		else
		{
			klogPut( null, LOG_WARNING, "Fichier inexistant (%s).", nomFichier_pc );
		}
	}
}

/**--------------------------------------------------------------------------------------------------------------------
 *
 */
int main(int argc, char **argv)
{
	int indice = 1;
	char *prog = "pmvMain";
	char destination[256];
	char source[256];
	char startupDest[256];
	char ethDest[256];
	char ethSrc[256];
	while (indice < argc)
	{
		if (strcmp("-prog", argv[indice]) == 0)
		{
			if (++indice < argc)
			{
				prog = argv[indice];
			}

		}
		else if ((strcmp("-help", argv[indice]) == 0) || (strcmp("-help",
				argv[indice]) == 0))
		{
			printf("Syntaxe : cpuRdtLanceur [-prog nomProgramme] [-help] [-?]");
		}
		indice++;
	}

	snprintf(destination, 256, "%s/%s", REP_DESTINATION,prog);
	snprintf(source, 256, "%s/%s", REP_SOURCE,prog);
	snprintf(ethDest, 256, "%s/eth.cfg", REP_DESTINATION);
	snprintf(startupDest, 256, "%s/startup.cfg", REP_DESTINATION);
	snprintf(ethSrc, 256, "%s/eth.cfg", REP_SOURCE_CFG);
#ifdef CPU432
	launchSh( startupDest,null);
	launchSh( ethDest,ethSrc);
#else
	launchSh(ethDest, NULL);
#endif

	buildln(destination, source);
	return 0;
}
