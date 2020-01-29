/* 
 * pmvWatchdog
 * ==============
 * L'objet du présent programme est l'entretien du watchdog hardware.
 * Ce programme est a reserver à la cible.
 * Pour information, compte tenu de la frequence de fonctionnement du 
 * CPU de la carte, la periode maximale est de 5,3 secondes.
 * Pour l'instant, j'ai fixe cette periode a 2 secondes.
 * */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h> 
#include <sys/resource.h>
#include <time.h>
#include <signal.h>
#include <linux/watchdog.h>
#include <netinet/in.h>

#include <stdio.h>

#include "kcommon.h"

#define MAX_CAR 20
#define CPU_WD_PORT		8001	/* Numéro de port IP utilise pour la communication entre appli et wd */

#define WD_SEUIL_WARNING 10.0	/* Si le delai depasse 10 seconde, le programme imprime un message. */
#define WD_SEUIL_ALERTE  300.0	/* Si le délai depasse cinq minutes, le watchdog s'arrête. */
#define WD_DEFAULT  60	/* Valeur par defaut de la tempo de watchdog */



/* WdSurv
 * ======
 * Structure d'un abonné au service de watchdog.
 * La structure contient pour l'heure juste l'identifiant
 * de l'abonné et la date de dernier signal de vie.
 * A terme, on pourra prévoir plus finement d'avoir par
 * exemple le temps maximal de rafraichissement....
 * */

typedef struct WdSurv
{
	int32 ident_dw; // Identifiant de surveillande 
	Ktimestamp ts_t; // Date de la dernière mise a jour
} WdSurv;

/* _wdSurfRaf
 * ==========
 * Rafraichissement du watchog. La fonction prend
 * le timestamp en paramètre pour eviter de nombreux
 * appels a ktsGetTime si on a plusieurs applis a 
 * surveiller.
 * */

static void _wdSurvRaf(WdSurv *surv, Ktimestamp *ts)
{
	if (NULL!=surv)
	{
		surv->ts_t=*ts;
	}
}

/* _wdSurvDiff
 * ===========
 * Récupération de la différente en seconde entre la date
 * passée en paramètre et le dernier signal de vie.
 * Pourrait avantageusement être remplace par un booleen avec
 * gestion integree de la temporisation max. */

static float64 _wdSurvDiff(WdSurv *surv, Ktimestamp *ts)
{
	float64 diff_df=0.0;
	if (NULL!=surv)
	{
		diff_df=kGetDeltaTimestamp(&surv->ts_t, ts);
	}
	return diff_df;
}

/* _wdSurvNew
 * ==========
 * Creation d'une struture. Par defaut, positionnement
 * d'une date initiale comme etant la date courante. */

static WdSurv *_wdSurvNew(int32 num)
{
	WdSurv *surv=kmmAlloc(NULL, sizeof(WdSurv));
	if (NULL!=surv)
	{
		surv->ident_dw=num;
		ktsGetTime(&surv->ts_t);
	}
	return surv;
}

/* _wdSurvComp
 * ===========
 * Fonction de comparaison, non utilisee a priori.
 * */

static int32 _wdSurvComp(void *un, void *deux)
{
	WdSurv *sUn_pt=(WdSurv *)un;
	WdSurv *sDe_pt=(WdSurv *)deux;
	return sUn_pt->ident_dw-sDe_pt->ident_dw;
}

/* _wdSurvDelete
 * =============
 * Liberation des ressources allouees
 * */

static void _wdSurvDelete(void *un)
{
	kmmFree(NULL, un);
}

/* _wdSearch
 * =========
 * Pour la recherche dans la liste des applis surveillees. */

static WdSurv *_wdSearch(Klist *listeSurv, int32 ident)
{
	WdSurv *surv_pt=NULL;
	;
	for (surv_pt=klistFirst(listeSurv); NULL!=surv_pt; surv_pt
			=klistNext(listeSurv))
	{
		if (surv_pt->ident_dw==ident)
		{
			break;
		}
	}
	return surv_pt;
}

/* Fonctions de gestion de la liste des abonnes : 
 * - Ajout d'un abonne,
 * - Rafraichissement,
 * - Suppression d'un abonne.
 * */

static void _wdAbonnement(Klist *listeSurv, int32 ident)
{
	WdSurv *surv_pt;
	surv_pt=_wdSearch(listeSurv, ident);
	if (NULL==surv_pt)
	{
		surv_pt=_wdSurvNew(ident);
		klistAdd(listeSurv, surv_pt);
	}
	else
	{
		printf("%ld deja surveille\n", ident);
	}
}

static void _wdDesabonnement(Klist *listeSurv, int32 ident)
{
	WdSurv *surv_pt;
	surv_pt=_wdSearch(listeSurv, ident);
	if (NULL!=surv_pt)
	{
		klistRemove(listeSurv, surv_pt);
	}
	else
	{
		printf("Pas de surveillance pour %ld\n", ident);
	}
}

static void _wdRafraichissement(Klist *listeSurv, int32 ident)
{
	WdSurv *surv_pt;
	surv_pt=_wdSearch(listeSurv, ident);
	if (NULL!=surv_pt)
	{
		ktsGetTime(&surv_pt->ts_t);
	}
	else
	{
		_wdAbonnement(listeSurv, ident);
	}
}

/* _wdOuvrirServeur
 * ================
 * Ouverture de la socket datagram (UDP) sur laquelle sont
 * transmises les signaux de vie.
 * */

static int _wdOuvrirServeur(int port)
{
	int sock_fd;
	/* création d'un socket */
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0)
	{
		printf("Erreur d'ouverture de socket %d\n", errno);
	}
	else
	{
		struct sockaddr_in addrLocale;
		/* mon adresse */
		addrLocale.sin_family = AF_INET;
		addrLocale.sin_addr.s_addr = htonl(INADDR_ANY );
		addrLocale.sin_port = htons(port);

		if (bind(sock_fd, (struct sockaddr *) &addrLocale, sizeof(addrLocale)) < 0 )
		{
			printf("Erreur de binding %d\n",errno);
			close(sock_fd);
			sock_fd=-1;
		}
	}
	return sock_fd;
}

/* main
 * =====
 * Procedure principale.
 * Les etapes sont les suivantes :
 * - Ouverture du device permettant d'entretenir le watchdog. Sur
 *   le pc, il s'agit pour l'heure seulement d'un fichie de trace.
 *   Il faudrait d'ailleurs controle que tout marche correctement.
 * - Ouverture de la socket datagram.
 * - Boucle d'attente des messages avec : 
 * 		+ select avec une duree max (ajout possible d'un parametrage
 *        a ce niveau.
 * 	    + traitement eventuel des messages recu,
 *      + controle des applications surveilles.
 * Un mécanime est prevu pour eviter les bevues en cas de changement
 * d'heure. Si un changement d'heure est detecte, tous les signaux de
 * vie sont forces.
 * Au niveau du controle, deux niveaux sont prevus : 
 * - Un niveau warning, un message est affiche,
 * - Un niveau panique : on sort du programme et a priori le 
 *   watchdog hard n'etant plus rafraichi, la station redemarrre.
 * */

int main(int argc, char **argv)
{
	int indice=1;
	int verbose=0;
	int force=0;
	int stop=0;
	int set=0;
	int valeur=60;
	int urgent=0;
	int port=CPU_WD_PORT;
#ifdef CPU432
	unsigned short tabShort[4];
	char *sortie="/dev/watchdog";
#else
	char *sortie="fichierCoucou";
//	int cpt=0;
#endif
	int32 periode_dw=2000;
	struct timeval last;
	struct timeval attente=
	{ 1, 0 };
	int desc;
	bool notFin=true;
	while (indice<argc)
	{
		if (strcmp("-verbose", argv[indice])==0)
		{
			verbose=1;
		}
		else if (strcmp("-force", argv[indice])==0)
		{
			force=1;
		}
		else if (strcmp("-stop", argv[indice])==0)
		{
			stop=1;
		}
		else if (strcmp("-port", argv[indice])==0)
		{
			if(++indice<argc)
			{
				sscanf(argv[indice],"%d",&port);
				if(port<=0)
				{
					port=CPU_WD_PORT;
				}
			}
		}
		else if (strcmp("-set", argv[indice])==0)
		{
			if(++indice<argc)
			{
				sscanf(argv[indice],"%d",&valeur);
				if(valeur<=0)
				{
					printf("Valeur incorrect %d\n",valeur);
					valeur=WD_DEFAULT;
				}
				set=1;
			}
		}
		else if (strcmp("-urgent", argv[indice])==0)
		{
			urgent=1;
		}
		else if ((strcmp("-help", argv[indice])==0)||(strcmp("-help",
				argv[indice])==0))
		{
			printf("Syntaxe : cpuRdtWatchdog [-verbose] [-force] [-stop] [-urgent] [-help] [-?]");
		}
		indice++;
	}

	if(1==urgent)
	{
		if(-1==setpriority(PRIO_PROCESS,0,-20))
		{
			printf("Erreur en positionnant la priorite %d",errno);
			perror("libelle de l'erreur");
		}
	}
	/* Ouverture du driver pour le watchdog hard.... */
	desc=open(sortie, O_WRONLY|O_CREAT);
	if (-1!=desc)
	{
		if (verbose)
			printf("Ouverture correcte du port %s : %d\n", sortie, desc);
		if(1==set)
	{
#ifdef CPU432
		if (-1==ioctl(desc, WDIOC_SET, &valeur))
		{
			perror("Erreur de positionnement de la tempo du watchdog");
		}
		else
		{
			if (-1==ioctl(desc, WDIOC_GET, &valeur))
			{
			perror("Erreur de lecture de la tempo du watchdog");
			}
			else
			{
				printf("La valeur de la temporisation est %d\n",valeur);
			}
		}
#else
#endif

	}
	else
	if(1==stop)
	{
#ifdef CPU432
		if (-1==ioctl(desc, WDIOC_STOP, &tabShort[0]))
		{
			perror("Probleme d'arret du watchog");
		}
		else
		{
			printf("Arret du traitement du watchdog\n");
		}
#else
#endif
	}
	else if(1==force)
	{
#ifdef CPU432
						/* Remise a jour du watchdog hard */
						if (-1!=ioctl(desc, WDIOC_GETTIMEOUT, &tabShort[0]))
						{
							if (verbose)
								printf(
										"Les valeurs sont %04x %04x %04x %04x\n",
										tabShort[0], tabShort[1], tabShort[2],
										tabShort[3]);
						}
						if (-1!=ioctl(desc, WDIOC_KEEPALIVE, &tabShort[0]))
						{
							if (verbose)
							{
							int prio=getpriority(PRIO_PROCESS,0);
								printf("Maintien en vie ok (prio : %d)\n",prio);
							}
						}

#endif
	}
	else
	{
		int fdSock;
		if (verbose)
		{
			gettimeofday(&last,NULL);
		}
		fdSock=_wdOuvrirServeur(port);
		if (-1!=fdSock)
		{
			Klist *listeSurv=klistNew(0, KTSL_DEFAULT, _wdSurvComp,
					_wdSurvDelete);
			Ktimestamp datePrec_t;
			/* La date precedente est utilisee pour détecter les éventuels changements d'heure..... */
			ktsGetTime(&datePrec_t);

			/* BOUCLE DE TRAITEMENT */
			/* Faire ici une boucle avec selecting..... */
			do
			{
				Ktimestamp dateCour_t;
				float64 diff_df;
				fd_set monFdSet;
				int maxFd=0;
				attente.tv_sec=periode_dw/1000;
				attente.tv_usec=(periode_dw%1000)*1000;
				ktsGetTime(&dateCour_t);
				diff_df=kGetDeltaTimestamp(&datePrec_t, &dateCour_t);
				/* ------------------------------------------ */
				/* TEST DE DETECTION D'UN CHANGEMENT D'HEURE. */
				/* ------------------------------------------ */
				if ((diff_df<0.0)||((diff_df*1000)>2*periode_dw))
				{

					WdSurv *surv_pt;
					for (surv_pt=klistFirst(listeSurv); surv_pt!=NULL; surv_pt
							=klistNext(listeSurv))
					{
						_wdSurvRaf(surv_pt, &dateCour_t);
					}
				}
				else
				/* ------------------------------------------ */
				/* CONTROLE DE WATCHDOG                       */
				/* ------------------------------------------ */
				{
					WdSurv *surv;
					float64 maxDiff_df=0.0;
					for (surv=klistFirst(listeSurv); surv!=NULL; surv
							=klistNext(listeSurv))
					{
						float64 diffCour_df=_wdSurvDiff(surv, &dateCour_t);
						maxDiff_df=MAX(diffCour_df, maxDiff_df);
					}
					if (maxDiff_df<WD_SEUIL_ALERTE)
					{
						/* On rafraichi le watchdog */

						/* Rafraichissement du watchdog */

#ifdef CPU432
						/* Remise a jour du watchdog hard */
						if (-1!=ioctl(desc, WDIOC_GETTIMEOUT, &tabShort[0]))
						{
							if (verbose)
								printf(
										"Les valeurs sont %04x %04x %04x %04x\n",
										tabShort[0], tabShort[1], tabShort[2],
										tabShort[3]);
						}
						if (-1!=ioctl(desc, WDIOC_KEEPALIVE, &tabShort[0]))
						{
						char chaine[200];
							if (verbose)
							{
							int prio=getpriority(PRIO_PROCESS,0);
							struct timeval current;
							long val;
								gettimeofday(&current,NULL);
								val=(current.tv_sec-last.tv_sec)*1000
									+(current.tv_usec-last.tv_usec)/1000;
								printf("Maintien en vie ok (prio : %d)--"
										"duree %ld ms\n",prio,val);
								last=current;
							}
							if (maxDiff_df>=WD_SEUIL_WARNING)
							{
								sprintf(
										chaine,
										"seuil d'alerte atteint %ld(ms) pour %ld\n",
										(int32)(1000.*maxDiff_df),
										(int32)(1000.*WD_SEUIL_WARNING));
								printf(chaine);
							}
						}

#else
						{
							char chaine[200];
							if (maxDiff_df>=WD_SEUIL_WARNING)
							{
								sprintf(chaine,"seuil d'alerte atteint %d(ms) pour %d\n",
										(int32)(1000.*maxDiff_df),(int32)(1000.*WD_SEUIL_WARNING));
								printf("%s",chaine);

							}
							else
							{
								if(verbose)
								sprintf(chaine,"Signal de vie \n");
							}
							write(desc,chaine,strlen(chaine));
						}
#endif
					}
					else
					{
						/* Le seuil d'alerte est depassé on se casse */
						printf("Seuil d'alerte depasse... \n");
						notFin=false;

					}
				}

				/* ------------------------------------------ */
				/* MEMORISATION DATE CONTROLE                 */
				/* ------------------------------------------ */
				datePrec_t=dateCour_t;
				/* Positionnement du fdset */
				/* ------------------------------------------ */
				/* ATTENTE DES MESSAGES                       */
				/* ------------------------------------------ */
				FD_ZERO(&monFdSet);
				maxFd=MAX(fdSock+1, maxFd);
				FD_SET(fdSock, &monFdSet);
				/* Attente des signaux de vie. */
				switch (select(maxFd, &monFdSet, NULL, NULL, &attente))
				{
				case 0:
					break;

				case -1:
					notFin=false;
					break;
				default:
					if (FD_ISSET(fdSock, &monFdSet))
					{
						struct sockaddr_in addrDist;
						socklen_t lgNom=sizeof(addrDist);
						int nbCarLus;
						char buffer[MAX_CAR+1];
						nbCarLus=recvfrom(fdSock,buffer,MAX_CAR,0,(struct sockaddr *)&addrDist,&lgNom);
						if(-1!=nbCarLus)
						{
							char type;
							int32 ident_dw;
							buffer[nbCarLus]=0;
							if(verbose)
							printf("J'ai recu %s \n",buffer);
							sscanf(buffer,"%c%02ld",&type,&ident_dw);
							switch(type)
							{
								case 'a':
								_wdAbonnement(listeSurv,ident_dw);
								break;
								case 'd':
								_wdDesabonnement(listeSurv,ident_dw);
								break;
								case 'v':
								_wdRafraichissement(listeSurv,ident_dw);
								break;
							}
						}
						else
						{
							if(verbose)
							printf("Probleme de reception\n");
						}

					}
					else
					{
						/* Il faut réaliser des lectures sur les descripteurs. */
						printf("Erreur d'aiguillage!!!");
						notFin=false;
					}
					break;
				}
			}while (notFin);
			/* ------------------------------------------ */
			/* ARRET DU PROGRAMME                         */
			/* ------------------------------------------ */
			/* Liberation des ressources allouees. */
			klistDelete(listeSurv);
			close(fdSock);
		}
		else
		{
			printf("Probleme majeure, sortie immediate");
		}
#ifdef CPU432
		ioSetGar();
		ksleep(1000);
		system("reboot");
#endif
	}
		close(desc);
		/* Faire ici un redémarrage dans le cas de la cible */
	}
	else
	{
		printf("Ouverture incorrecte du port %s errno %d\n", sortie, errno);
	}

	return 0;
}

