
/* 
 * $Log: dialoguePass.c,v $
 * Revision 1.1  2018/06/04 08:38:44  xg
 * Passage en version V122
 *
 * Revision 1.2  2013/10/02 07:39:05  xg
 * Remplacement des fonctions "deprecated"
 *
 * Revision 1.1  2013/07/05 14:53:07  xg
 * Passage dans le nouveau repository
 *
 * Revision 1.6  2013/07/05 10:17:02  xg
 * *** empty log message ***
 *
 */
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pipe.h>

#include "standard.h"
#include "portage.h"
#include "define.h"
#include "mon_inc.h"
#include "xdg_var.h"
#include "x01_str.h"
#include "x01_var.h"
#include "etaSyst.h"
#include "mon_debug.h"

#include "kcommon.h"
#include "klog.h"
#include "lcr_util.h"
#include "anaSys.h"
#include "ypc_las.h"
#include "ficTrace.h"
#include "seqLib.h"
#include "supRun.h"

#define DEBUG 1
#if DEBUG > 0
#define printDebug printf
#else
#define printDebug monPrintDebug
#endif

/* *******************************************
 * DECLARATION DES CONSTANTES
 ******************************************** */
#define MESS_API_REQUETE 8
#define MESS_API_TEST_PRESENCE 4
#define MESS_API_TEST_BOUT_EN_BOUT 1
#define MESS_API_LONGUEUR 0
#define MESS_API_VERSION 2
#define MESS_API_COMMANDE 3
#define MESS_API_CPT_RENDU 4
#define MESS_API_DEST 5
#define MESS_API_SOURCE 9
#define MESS_API_IDENTIFIANT 13
#define MESS_API_DONNEES 17

#define MESS_API_REP 256
#define MESS_API_REP_MAX 65536

#define MESS_API_MAX_WAIT 7000

#define PASS_MAX_CONNEXION 10

/* Le timeout sur la connexion et donc sa fermeture automatique
 * est fixee a 10 minutes.
 */

#define PASS_CONNEXION_TIMEOUT 600000

/* *******************************************
 * DECLARATION DES TYPES
 ******************************************** */

typedef struct Client {
	uint32 currentTick;
	uint32 sendTick;
	int desc;
	int carCour;
	char mess[MESS_API_DONNEES];
	char abtMess[MESS_API_DONNEES];
	int bufCour;
	char bufCmd[500];
	int bufRepMax;
	int bufRep;
	char *bufRep_pt;
	bool abt;
} Client;

/* *******************************************
 * DECLARATION DES VARIABLES LOCALES
 ******************************************** */

static T_lcr_util_buffer repLcr;
static Client *clientCourant_pt = NULL;
static Client *clientAbt_pt = NULL;
static Klist *listeRequetes_pt = NULL;
static Kmutex *clientMutex_pt = NULL;

static void clientSendRep(Client *client_pt);
static void clientSendAbt(Client *client_pt);
static void clientResetRep(Client *client_pt);
static void clientReset(Client *client_pt);
static int32 clientAddRep(Client *client_pt, char *buffer, int nbCar);
static void clientSendError(Client *client_pt);
static void clientEnregistrerReponse(Client *client_pt);
static void clientEnregistrerCommande(Client *client_pt);
static void clientEnregistrerQuestion(Client *client_pt);

/* *******************************************
 * DECLARATION DES FONCTIONS LOCALES
 ******************************************** */

static Client * clientNew(void)
{
	Client *client_pt = kmmAlloc(NULL,sizeof(Client));
	if (NULL != client_pt)
	{
		client_pt->currentTick = ktickGetCurrent();
		client_pt->carCour = 0;
		client_pt->sendTick = 0;
		client_pt->bufRepMax = MESS_API_REP;
		client_pt->bufRep = 0;
		client_pt->bufRep_pt = kmmAlloc(NULL,client_pt->bufRepMax);
		client_pt->abt = false;
		if (NULL == client_pt->bufRep_pt)
		{
			kmmFree(NULL,client_pt);
			client_pt = NULL;
		}

	}
	return client_pt;
}

static void clientDelete(void *element_p)
{
	if (NULL != element_p)
	{
		if (NULL != ((Client *) element_p)->bufRep_pt)
		{
			kmmFree(NULL,((Client *)element_p)->bufRep_pt);
		}
		kmmFree(NULL,element_p);
	}
}

static int32 clientCompare(void *un_p, void *de_p)
{
	return 1;
}
static void clientSendCmd(Client *client_pt)
{
	klogPut(NULL, LOG_INFO, "radTestSocket: Message complet recu");
	switch (client_pt->mess[MESS_API_COMMANDE])
	{
	case MESS_API_REQUETE: {
		char *buffer = client_pt->mess;
		short nbCarReq = ntohs(*(uint16_t *) &buffer[MESS_API_LONGUEUR]);
		int nbCarMess = nbCarReq - MESS_API_DONNEES + 2;
		if (nbCarMess > 0)
		{
			T_lcr_util_buffer lcrAna;
			int reconnue;
			char *buffMess = client_pt->bufCmd;
			buffMess[nbCarMess] = 0x0D;
			buffMess[nbCarMess + 1] = 0x0;
			/* On verifie si il ne s'agit pas d'une requete systeme */
			lcr_util_buffer_init(&lcrAna, LCR_UTIL_MAX_BUFFER);
			reconnue = anaSys(PORT_PASS, buffMess, nbCarMess, &lcrAna);
			if (0 == reconnue)
			{

				if (1 == supRunLireEtat())
				{
					if (null != clientMutex_pt)
					{
						kmutexLock(clientMutex_pt);
						if (NULL == clientCourant_pt)
						{
							char Buffer[MAX_MESSAGE_LCR + 2];
							short nbCarSent=nbCarMess + 1;
							int status;
							/* Ecriture dans le message du
							 * nombre de caracteres et du
							 * message.                                                     */
							memcpy(Buffer, &nbCarSent, 2);
							memcpy(&Buffer[2], buffMess, nbCarSent);
							status = SysPipeEmettre(MqDialLas[PORT_PASS], Buffer, nbCarSent+2);
							if (status == ERROR) {
								printDebug("DialogueLCR : erreur "
									"SysPipeEmettre liaison %d\n", PORT_PASS);
							} else {
								clientEnregistrerQuestion(client_pt);
							} /* endif(Status==ERROR                          */

							client_pt->sendTick = ktickGetCurrent();
							clientCourant_pt = client_pt;
						}
						else
						{
							/* Sinon, on empile dans la liste des requetes en cours.... */
							if (false == klistAdd(listeRequetes_pt, client_pt))
							{
								klogPut(NULL, LOG_ERROR, "clientSendCmd: Impossible d'ajouter une requete");
								clientReset(client_pt);
							}
						}
						kmutexUnlock(clientMutex_pt);
					}
				}
				else
				{
					klogPut(NULL, LOG_INFO, "clientSendCmd: Station endormie");
					clientSendError(client_pt);
					clientReset(client_pt);
				}
			}
			else
			{
				/* Il faut envoyer directement la reponse... */
				clientAddRep(client_pt, lcr_util_buffer_lire_buffer(&lcrAna), lcr_util_buffer_lire_nb_car(&lcrAna));
				clientSendRep(client_pt);
				clientReset(client_pt);
				clientResetRep(client_pt);
			}
		}
	}
		break;
	case MESS_API_TEST_PRESENCE:
		clientSendRep(client_pt);
		clientReset(client_pt);
		clientResetRep(client_pt);
		break;
	case MESS_API_TEST_BOUT_EN_BOUT:
		clientSendRep(client_pt);
		clientReset(client_pt);
		clientResetRep(client_pt);
		break;
	default:
		klogPut(NULL, LOG_ERROR, "clientSendCmd: Type de requete inconnu");
		clientReset(client_pt);
		break;
	}
}

static void clientSendError(Client *client_pt)
{
	char *buffer = client_pt->mess;
	long source = *((long *) &buffer[MESS_API_DEST]);
	long dest = *((long *) &buffer[MESS_API_SOURCE]);
	//	short nbCarReq = ntohs(*(uint16_t *) &buffer[MESS_API_LONGUEUR]);
	//	int nbCarMess = nbCarReq - MESS_API_DONNEES + 2;
	//	int indice;
	*((long *) &buffer[MESS_API_DEST]) = dest;
	*((long *) &buffer[MESS_API_SOURCE]) = source;
	buffer[MESS_API_CPT_RENDU] = 2;
	*((short *) &buffer[MESS_API_LONGUEUR]) = htons(MESS_API_DONNEES - 2 + client_pt->bufRep);
	send(client_pt->desc, buffer, MESS_API_DONNEES, 0);
	//	if (0 != client_pt->bufRep)
	//	{
	//		send(client_pt->desc, client_pt->bufRep_pt, client_pt->bufRep, 0);
	//	}

}

static void clientSendAbt(Client *client_pt)
{
	char *buffer = client_pt->abtMess;
	long source = *((long *) &buffer[MESS_API_DEST]);
	long dest = *((long *) &buffer[MESS_API_SOURCE]);
	//	short nbCarReq = ntohs(*(uint16_t *) &buffer[MESS_API_LONGUEUR]);
	//	int nbCarMess = nbCarReq - MESS_API_DONNEES + 2;
	//	int indice;
	*((long *) &buffer[MESS_API_DEST]) = dest;
	*((long *) &buffer[MESS_API_SOURCE]) = source;
	buffer[MESS_API_CPT_RENDU] = 1;
	*((short *) &buffer[MESS_API_LONGUEUR]) = htons(MESS_API_DONNEES - 2 + client_pt->bufRep);
	send(client_pt->desc, buffer, MESS_API_DONNEES, 0);
	if (0 != client_pt->bufRep)
	{
		send(client_pt->desc, client_pt->bufRep_pt, client_pt->bufRep, 0);
	}
}

static void clientSendRep(Client *client_pt)
{
	char *buffer = client_pt->mess;
	long source = *((long *) &buffer[MESS_API_DEST]);
	long dest = *((long *) &buffer[MESS_API_SOURCE]);
	//	short nbCarReq = ntohs(*(uint16_t *) &buffer[MESS_API_LONGUEUR]);
	//	int nbCarMess = nbCarReq - MESS_API_DONNEES + 2;
	//	int indice;
	*((long *) &buffer[MESS_API_DEST]) = dest;
	*((long *) &buffer[MESS_API_SOURCE]) = source;
	buffer[MESS_API_CPT_RENDU] = 1;
	*((short *) &buffer[MESS_API_LONGUEUR]) = htons(MESS_API_DONNEES - 2 + client_pt->bufRep);
	send(client_pt->desc, buffer, MESS_API_DONNEES, 0);
	if (0 != client_pt->bufRep)
	{
		send(client_pt->desc, client_pt->bufRep_pt, client_pt->bufRep, 0);
	}
}

static void clientResetRep(Client *client_pt)
{
	if (NULL != client_pt)
	{
		client_pt->bufRep = 0;
		client_pt->sendTick = 0;
		if (client_pt->bufRepMax != MESS_API_REP)
		{
			kmmFree(NULL,client_pt->bufRep_pt);
			client_pt->bufRep_pt = kmmAlloc(NULL,MESS_API_REP);
			if (NULL != client_pt->bufRep_pt)
			{
				client_pt->bufRepMax = MESS_API_REP;
			}
			else
			{
				client_pt->bufRepMax = 0;
			}
		}
	}
}

static int32 clientAddRep(Client *client_pt, char *buffer, int nbCar)
{
	int retour = 0;
	int carCour = 0;
	while ((carCour < nbCar) && ((client_pt->bufRep < client_pt->bufRepMax) || (client_pt->bufRepMax < MESS_API_REP_MAX)) && (0 == retour))
	{
		/* On controle qu'il y ait assez de place pour le stockage. */
		if (client_pt->bufRep == client_pt->bufRepMax)
		{
			char *tmp_pt = NULL;
			tmp_pt = kmmAlloc(NULL,2*client_pt->bufRepMax);
			if (NULL != tmp_pt)
			{
				memcpy(tmp_pt, client_pt->bufRep_pt, client_pt->bufRepMax);
				client_pt->bufRepMax *= 2;
				kmmFree(NULL,client_pt->bufRep_pt);
				client_pt->bufRep_pt = tmp_pt;
			}
			else
			{
				retour = -1;
			}
		}
		if (-1 != retour)
		{
			/* Controle d'un eventuel caractere de fin */
			if ('\\' == buffer[carCour])
			{
				retour = 3;
			}
			else if ('!' == buffer[carCour])
			{
				if (0 == client_pt->bufRep)
				{
					client_pt->bufRep_pt[client_pt->bufRep++] = buffer[carCour++];
				}
				retour = 1;
			}
			else
			{
				if ('?' == buffer[carCour])
				{
					retour = 2;
				}
				else if ('%' == buffer[carCour])
				{
					retour = 2;
					/* On essaie d'ecrire le code
					 * d'erreur. */
					if ((carCour + 1 < nbCar) && ((client_pt->bufRep + 1) < client_pt->bufRepMax))
					{
						client_pt->bufRep_pt[client_pt->bufRep++] = buffer[carCour++];
					}
				} /* endif('?							*/
				client_pt->bufRep_pt[client_pt->bufRep++] = buffer[carCour++];
			} /* endif('!'==buffer[carCour])		*/

		}
	}
	return retour;
}
static int32 clientAddCmd(Client *client_pt, char *buffer, int nbCar)
{
	int courant = 0;
	int retour = 0;
	/* Traiter l'abonnement en cours !!! */
	/*   Si il y a un abonnement en cours, on peut arreter cet abonnement.... */

	/* Si le message est en cours d'emission */
	if (0 == client_pt->sendTick)
	{
		/* On efface les caracteres trop anciens */
		if (ktickGetDeltaCurrent(client_pt->currentTick) > 500)
		{
			client_pt->carCour = 0;
		}
		client_pt->currentTick = ktickGetCurrent();
		while ((client_pt->carCour < MESS_API_DONNEES) && (courant < nbCar))
		{
			((char *) &client_pt->mess)[client_pt->carCour] = buffer[courant];
			courant++;
			client_pt->carCour += 1;
			if (client_pt->carCour == MESS_API_DONNEES)
			{
				client_pt->bufCour = 0;
			}
		}
		if (client_pt->carCour == MESS_API_DONNEES)
		{
			int longueurCible = (int) (ntohs(*(uint16_t *) &client_pt->mess[MESS_API_LONGUEUR])) - MESS_API_DONNEES + sizeof(short);
			if (longueurCible <= 500)
			{
				if (0 == longueurCible)
				{
					/* Il s'agit d'une trame de surveillance */
					retour = 1;
				}
				else
				{
					if (true == client_pt->abt)
					{
						/* Suppression de l'abonnement en retirant le message de la liste des abonnements,
						 * uniquement si l'on est en presence d'une trame d'exploitation.  */
						if (null != clientMutex_pt)
						{
							client_pt->abt = false;
							clientAbt_pt = null;
						}
					}
					while ((courant < nbCar) && (client_pt->bufCour < longueurCible))
					{
						/* On rempli ici le buffer message... */
						client_pt->bufCmd[client_pt->bufCour] = buffer[courant];
						client_pt->bufCour += 1;
						courant++;
						if (longueurCible == client_pt->bufCour)
						{
							retour = 1;
						}
					}
				}
			}
			else
			{
				klogPut(NULL, LOG_ERROR, "clientAdd: Taille du message incorrecte %d", (int) (ntohs(*(uint16_t *) &client_pt->mess[MESS_API_LONGUEUR])));
				retour = -1;
			}
		}
	}
	else
	{
		klogPut(NULL, LOG_ERROR, "clientAdd :message en cours d'utilisation depuis %d ms", ktickGetDeltaCurrent(client_pt->sendTick));
		retour = -1;

	}
	return retour;
}

static void clientReset(Client *client_pt)
{
	if (NULL != client_pt)
	{
		client_pt->sendTick = 0;
		client_pt->carCour = 0;
		client_pt->bufCour = 0;
	}
}

static void clientGetAdresse(Client *client_pt, T_ficTraceAdresse *adresse_pt)
{
	long ident;
	unsigned long adresseIp;
	adresseIp=(*((unsigned long *)(&client_pt->mess[MESS_API_SOURCE])));
	ident = (((long) client_pt->mess[MESS_API_IDENTIFIANT + 1]) << 16) + (((long) client_pt->mess[MESS_API_IDENTIFIANT + 2]) << 8) + (((long) client_pt->mess[MESS_API_IDENTIFIANT
			+ 3]));
	ficTraceAdresseSetFip(adresse_pt, adresseIp, ident, client_pt->mess[MESS_API_IDENTIFIANT]);

}
static void clientEnregistrerReponse(Client *client_pt)
{
	T_ficTraceAdresse adresse;
	struct timespec dateRep;
	mnlcalTime(&dateRep);
	clientGetAdresse(client_pt, &adresse);
	ficTraceEnregistrerReponseBis(&adresse, &dateRep, client_pt->bufRep_pt, client_pt->bufRep, client_pt->bufCmd, client_pt->bufCour);
}

static void clientEnregistrerCommande(Client *client_pt)
{
	T_ficTraceAdresse adresse;
	struct timespec dateRep;
	mnlcalTime(&dateRep);
	clientGetAdresse(client_pt, &adresse);
	ficTraceEnregistrerCommande(&adresse, &dateRep, client_pt->bufCmd, client_pt->bufCour);
}
static void clientEnregistrerQuestion(Client *client_pt)
{
	T_ficTraceAdresse adresse;
	struct timespec dateRep;
	mnlcalTime(&dateRep);
	clientGetAdresse(client_pt, &adresse);
	ficTraceEnregistrerQuestion(&adresse, &dateRep, client_pt->bufCmd, client_pt->bufCour);
}

//int main(int argc, char **argv)
FUNCPTR DialoguePass(int numero)
{
	Klist *listeConnexions_pt = NULL;
	Klist *listeErreurs_pt = NULL;
	int sockServeur = -1;
	int portServeur = 5020;
	T_xdg_cf_las *cfgLas_pt = &xdg_cf_las[PORT_PASS];
	portServeur = cfgLas_pt->numPortIp;
	listeConnexions_pt = klistNew(PASS_MAX_CONNEXION, KTSL_KEEP, clientCompare, clientDelete);
	listeErreurs_pt = klistNew(KTSL_DEFAULT, KTSL_DEFAULT, clientCompare, clientDelete);
	listeRequetes_pt = klistNew(KTSL_DEFAULT, KTSL_DEFAULT, clientCompare, NULL);
	clientMutex_pt = kmutexNew();

	clientCourant_pt = NULL;
	/* Il s'agit du buffer qui va permettre de tracer la reponse de la station. */

	lcr_util_buffer_init(&repLcr, 78);

	if (NULL != listeConnexions_pt)
	{
		Client *client_pt;
		bool notFin = false;
		/*Realiser ici la creation du serveur socket */
		sockServeur = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (SOCKET_ERROR != sockServeur)
		{
			struct sockaddr_in echoServAddr; /* Local address */
			int on = 1;
			setsockopt(sockServeur, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
			memset(&echoServAddr, 0, sizeof(echoServAddr));
			echoServAddr.sin_family = AF_INET;
			echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			echoServAddr.sin_port = htons(portServeur);
			if (bind(sockServeur, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != -1)
			{
				if (listen(sockServeur, 1) != -1)
				{
					klogPut(NULL, LOG_INFO, "DialoguePass : listen du serveur");
					fcntl(sockServeur, F_SETFL, O_NDELAY);
					notFin = true;
				}
				else
				{
					klogPut(NULL, LOG_ERROR, "DialoguePass : Erreur sur le listen %d", errno);
				}
			}
			else
			{
				klogPut(NULL, LOG_ERROR, "DialoguePass : Erreur sur le bind %d", errno);
			}
		}
		else
		{
			klogPut(NULL, LOG_ERROR, "DialoguePass : Erreur de creation de la socket %d", errno);
		}

		while (notFin)
		{
			int maxFds = 0;
			struct timeval timeout = { 0, 500000 };
			fd_set listeFd;
			fd_set listeEx;
			timeout.tv_usec = 500000;
			FD_ZERO(&listeFd);
			if (-1 != sockServeur)
			{
				FD_SET(sockServeur,&listeFd);
				FD_SET(sockServeur,&listeEx);
				maxFds = MAX(maxFds,sockServeur+1);
			}
			for (client_pt = klistFirst(listeConnexions_pt); client_pt != NULL; client_pt = klistNext(listeConnexions_pt))
			{
				/* Un descripteur de client ne devrait pas pouvoir etre egal a -1 */
				if (-1 != client_pt->desc)
				{
					FD_SET(client_pt->desc,&listeFd);
					FD_SET(client_pt->desc,&listeEx);
					maxFds = MAX(maxFds,client_pt->desc+1);
				}
				else
				{
					/* TODO : Si le descripteur n'est pas bon, il faut supprimer la connexion!!! */
				}
			}
			/* Se mettre en attente sur une demande de connexion sur la socket server ou sur
			 * un message recu sur une des socket client.
			 */
			switch (select(maxFds, &listeFd, NULL, &listeEx, &timeout))
			{
			case 0:
				/* TODO : Parcourir la liste des client pour voir si l'un d'eux depasse le temps maximal.... */
				for (client_pt = klistFirst(listeConnexions_pt); client_pt != NULL; client_pt = klistNext(listeConnexions_pt))
				{
					/* Si il n'y a pas eu d'echange depuis plus de une minute... */
					if (ktickGetDeltaCurrent(client_pt->currentTick) > PASS_CONNEXION_TIMEOUT)
					{
						/* Cela fait plus de dix minutes qu'il n'y a pas d'activite... */
						klistAdd(listeErreurs_pt,client_pt);
					}

				}
				/* TODO : Si il y a des elements dans la liste des erreurs, il faut proceder aux fermetures. */
				for (client_pt = klistFirst(listeErreurs_pt); client_pt != NULL; client_pt = klistNext(listeErreurs_pt))
				{
					klistRemove(listeConnexions_pt, client_pt);
					if (-1 == close(client_pt->desc))
					{
						klogPut(NULL, LOG_ERROR, "radTestSocket : Close failed %d", errno);
					}
					else
					{
						klogPut(NULL, LOG_INFO, "radTestSocket : Fermeture du client");
					}
				}
				klistDeleteElements(listeErreurs_pt);
				break;
			case -1:
				break;
			default:
				/* Il y a au moins un truc qui a fonctionné... */
				if (FD_ISSET(sockServeur,&listeFd))
				{
					int clientSocket = -1;
					struct sockaddr_in echoClntAddr;
					unsigned int clntLen;
					clntLen = sizeof(echoClntAddr);
					clientSocket = accept(sockServeur, (struct sockaddr *) &echoClntAddr, &clntLen);
					if (-1 != clientSocket)
					{
						Client *nouveau_pt = clientNew();
						nouveau_pt->desc = clientSocket;
						if(false==klistAdd(listeConnexions_pt, nouveau_pt))
						{
							klogPut(NULL, LOG_ERROR, "DialoguePass : Il y a trop de clients %d (nb=%d)", clientSocket, klistGetSize(listeConnexions_pt));
							close(nouveau_pt->desc);
							clientDelete(nouveau_pt);
						}
						else
						{
							klogPut(NULL, LOG_INFO, "DialoguePass : Ajout d'un client %d (nb=%d)", clientSocket, klistGetSize(listeConnexions_pt));
						}
					}
					else
					{
					}
				}
				/* Controle d'un nouveau client eventuel */
				/* Verification sur les autres descripteurs */
				for (client_pt = klistFirst(listeConnexions_pt); client_pt != NULL; client_pt = klistNext(listeConnexions_pt))
				{
					if (FD_ISSET(client_pt->desc,&listeFd))
					{
						char buffer[500];
						int nbCarLus;
						/* Effectuer la lecture..... */
						/* TODO : Il faut mettre a jour le compteur de tick de la connexion pour permettre de clore la
						 * connexion seulement apres dix minutes.
						 */
						nbCarLus = recv(client_pt->desc, buffer, 500, 0);
						switch (nbCarLus)
						{
						case -1:
							klogPut(NULL, LOG_ERROR, "radTestSocket : Recv failed %d", errno);
							/* Il faudrait mettre le client dans la liste des clients a
							 * detruire.
							 */
							klistAdd(listeErreurs_pt, client_pt);

							break;
						case 0:
							/* Arret normal de l'autre cote de la socket... */
							klistAdd(listeErreurs_pt, client_pt);
							klogPut(NULL, LOG_ERROR, "radTestSocket : Bizarre 0 caracteres");
							break;
						default:

							switch (clientAddCmd(client_pt, buffer, nbCarLus))
							{
							case 1:
								clientSendCmd(client_pt);
								break;
							case 0:
								break;
							case -1:
								klogPut(NULL, LOG_ERROR, "radTestSocket: probleme de message");
								break;
							}
							/* Echanger source et destination */
							//							send(client_pt->desc, buffer, nbCarLus, 0);
						}
					}
					if (FD_ISSET(client_pt->desc,&listeEx))
					{
						klistAdd(listeErreurs_pt, client_pt);
						klogPut(NULL, LOG_INFO, "radTestSocket : probleme sur la socket", client_pt->desc);
					}

				}
				/* Parcourir eventuellement la liste des clients a detruire. */
				for (client_pt = klistFirst(listeErreurs_pt); client_pt != NULL; client_pt = klistNext(listeErreurs_pt))
				{
					klistRemove(listeConnexions_pt, client_pt);
					if (-1 == close(client_pt->desc))
					{
						klogPut(NULL, LOG_ERROR, "radTestSocket : Close failed %d", errno);
					}
					else
					{
						klogPut(NULL, LOG_INFO, "radTestSocket : Fermeture du client");
					}
				}
				klistDeleteElements(listeErreurs_pt);
				break;
			} /* endswitch(select(maxFds,&listeFd,NULL,NULL,&timeout)) */
			/* Ici, on controle l'envoi eventuel d'un nouveau message de test... */
			kmutexLock(clientMutex_pt);
			if (null != clientCourant_pt)
			{
				if (ktickGetDeltaCurrent(clientCourant_pt->sendTick) > MESS_API_MAX_WAIT)
				{
					/* On a trop attendu, on doit renvoyer un compte rendu incorrect. */
					clientSendError(clientCourant_pt);
					clientReset(clientCourant_pt);
					clientResetRep(clientCourant_pt);
					clientCourant_pt = null;
				}
			}
			if (null == clientCourant_pt)
			{
				clientCourant_pt = klistOutFifo(listeRequetes_pt);
				if (null != clientCourant_pt)
				{
					char Buffer[MAX_MESSAGE_LCR + 2];
					short nbCarSent=clientCourant_pt->bufCour + 1;
					int status;
					/* Ecriture dans le message du
					 * nombre de caracteres et du
					 * message.                                                     */
					memcpy(Buffer, &nbCarSent, 2);
					memcpy(&Buffer[2], clientCourant_pt->bufCmd, nbCarSent);
					status = SysPipeEmettre(MqDialLas[PORT_PASS], Buffer, nbCarSent+2);
					if (status == ERROR) {
						printDebug("DialogueLCR : erreur "
							"SysPipeEmettre liaison %d\n", PORT_PASS);
					} else {
						clientEnregistrerQuestion(clientCourant_pt);
					} /* endif(Status==ERROR                          */

//					tac_las_send_direct(PORT_PASS, clientCourant_pt->bufCmd, clientCourant_pt->bufCour + 1);
					clientCourant_pt->sendTick = ktickGetCurrent();
//					clientEnregistrerQuestion(clientCourant_pt);
				}

			}
			kmutexUnlock(clientMutex_pt);

			etaSystIncrCompteur(numero);
		} /* endwhile(notFin) */

		/* Fermeture des connexions */
		for (client_pt = klistFirst(listeConnexions_pt); client_pt != NULL; client_pt = klistNext(listeConnexions_pt))
		{
			if (-1 == close(client_pt->desc))
			{
				klogPut(NULL, LOG_ERROR, "radTestSocket : Close failed %d", errno);
			}
		}
		kmutexLock(clientMutex_pt);
		{
			Kmutex *courant_pt = clientMutex_pt;
			clientMutex_pt = NULL;
			kmutexUnlock(courant_pt);
			kmutexDelete(courant_pt);
		}
		kmutexDelete(clientMutex_pt);
		klistDelete(listeRequetes_pt);
		klistDelete(listeConnexions_pt);
		klistDelete(listeErreurs_pt);
		if (-1 != sockServeur)
		{
			if (-1 == close(sockServeur))
			{
				klogPut(NULL, LOG_ERROR, "radTestSocket : Close server failed %d", errno);

			}
		}
	}
	else
	{
		klogPut(NULL, LOG_ERROR, "radTestSocket : impossible d'allouer la liste des connexions");
	}
	kcommonEnd();
	return 0;
}

int dialoguePassRetour(int numPort, char *buffer, int nbCar)
{
	if (NULL != clientMutex_pt)
	{
		kmutexLock(clientMutex_pt);
		if (clientCourant_pt != NULL)
		{
			switch (clientAddRep(clientCourant_pt, buffer, nbCar))
			{
			case 0:
				/* Le traitement n'est pas termine */
				break;
			case 1:
				/* Enregistrer la reponse */
				clientEnregistrerReponse(clientCourant_pt);
				/* Enregistrer la commande */
				clientEnregistrerCommande(clientCourant_pt);

				/* Il faut envoyer la reponse OK */
				clientReset(clientCourant_pt);
				clientSendRep(clientCourant_pt);
				/* Remettre a 0 le buffer de message. */
				clientResetRep(clientCourant_pt);
				clientCourant_pt = NULL;
				break;
			case 2:
				/* Enregistrer la reponse */
				clientEnregistrerReponse(clientCourant_pt);
				/* Il faut envoyer la reponse NAK */
				clientReset(clientCourant_pt);
				clientSendRep(clientCourant_pt);
				clientResetRep(clientCourant_pt);
				clientCourant_pt = NULL;
				break;
			case 3:
				/* Cas sans reponse... */
				clientReset(clientCourant_pt);
				clientResetRep(clientCourant_pt);
				clientCourant_pt = NULL;
				break;
			case -1:
				/* Pas de reponse a enregistrer */
				clientReset(clientCourant_pt);
				clientResetRep(clientCourant_pt);
				clientCourant_pt = NULL;
				break;
			}
		}
		/* Si le clientCourant a ete traite, on passe au suivant si il y en a un */
		if (null == clientCourant_pt)
		{
			clientCourant_pt = klistOutFifo(listeRequetes_pt);
			if (null != clientCourant_pt) {
				char Buffer[MAX_MESSAGE_LCR + 2];
				short nbCarSent=clientCourant_pt->bufCour + 1;
				int status;
				/* Ecriture dans le message du
				 * nombre de caracteres et du
				 * message.                                                     */
				memcpy(Buffer, &nbCarSent, 2);
				memcpy(&Buffer[2], clientCourant_pt->bufCmd, nbCarSent);
				status = SysPipeEmettre(MqDialLas[PORT_PASS], Buffer, nbCarSent+2);
				if (status == ERROR) {
					printDebug("DialogueLCR : erreur "
						"SysPipeEmettre liaison %d\n", PORT_PASS);
				} else {
					clientEnregistrerQuestion(clientCourant_pt);
				} /* endif(Status==ERROR                          */

//				tac_las_send_direct(PORT_PASS, clientCourant_pt->bufCmd, clientCourant_pt->bufCour + 1);
				clientCourant_pt->sendTick = ktickGetCurrent();
//				clientEnregistrerQuestion(clientCourant_pt);
			}

		}
		kmutexUnlock(clientMutex_pt);
	}

	return nbCar;
}
int dialoguePassAbonnement(int numPort, char *buffer, int nbCar)
{
	if (null != clientMutex_pt)
	{
		kmutexLock(clientMutex_pt);
		if (nbCar != -1)
		{
			if (null != clientAbt_pt)
			{
				switch (clientAddRep(clientAbt_pt, buffer, nbCar))
				{
				case 0:
				case 1:
				case 2:
					clientSendAbt(clientAbt_pt);
					clientResetRep(clientAbt_pt);
					break;
				case -1:
					clientReset(clientAbt_pt);
					clientResetRep(clientAbt_pt);
					break;
				}

			}
		} /* endif(nbCar!=-1)					*/
		kmutexUnlock(clientMutex_pt);
	}
	return nbCar;
}
