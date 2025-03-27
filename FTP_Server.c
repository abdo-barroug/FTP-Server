#include "Signal_Handler_Server.h"
#include "FTP_Service.h"
#include "FTP_Structures.h"

pid_t children[NB_PROC]; //tableau des processus fils 


int main() {
    int connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    int i;
    int listenfd;
    
    /* Création du socket d'écoute du serveur */
    listenfd = Open_listenfd(PORT);  // Initialisation de la variable globale listenfd
    clientlen = sizeof(clientaddr);
    pid_t pid;

    Signal(SIGINT, sigint_handler);  // Installation du gestionnaire de signal pour SIGINT

    /* Création d'un pool de NB_PROC processus */
    for (i = 0; i < NB_PROC; i++) {
        if ((pid = Fork()) == 0) {  /* Processus fils : sortir de la boucle de création */
            Signal(SIGINT, SIG_DFL);  // Rétablir le comportement par défaut pour SIGINT
            break;
        } else {       /* Processus père : continuer à forker */
            children[i] = pid;
        }
    }
    
    if (i < NB_PROC) {  /* Code exécuté par un processus fils */
        while (1) {
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            if (connfd != -1) {
                ftp_service(connfd, &clientaddr);  // Passer les informations du client
                Close(connfd);
            }
            // En cas d'erreur sur Accept, on recommence la boucle
        }
    } else {  /* Code exécuté par le processus père */
        pause();
    }
    
    return 0;
}
