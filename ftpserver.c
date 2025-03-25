#include "ftpserver.h"
 
/*Envoi du fichier par bloc*/
void send_file(int connfd, char *filename) {
    int filefd;
    struct stat statbuf;
    response_t resp;
    char path[MAX_FILENAME_LEN + 10];  //Pour "./server/" + nom

    //Construction du chemin complet vers le fichier dans le répertoire ./server
    snprintf(path, sizeof(path), "./server/%s", filename);

    if ((filefd = open(path, O_RDONLY)) < 0) {
        perror("Erreur ouverture fichier");
        resp.code = ERR_FILE_NOT_FOUND;
        resp.filesize = 0;
        Rio_writen(connfd, &resp, sizeof(response_t));
        return;
    }

    if (fstat(filefd, &statbuf) < 0) {
        perror("Erreur fstat");
        resp.code = ERR_FILE_NOT_FOUND;
        resp.filesize = 0;
        Rio_writen(connfd, &resp, sizeof(response_t));
        close(filefd);
        return;
    }

    //Envoi de la réponse de succès avec la taille totale du fichier
    resp.code = SUCCESS;
    resp.filesize = statbuf.st_size;
    Rio_writen(connfd, &resp, sizeof(response_t));

    //Transfert du fichier par blocs
    char buffer[BLOCK_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(filefd, buffer, BLOCK_SIZE)) > 0) {
        Rio_writen(connfd, buffer, bytesRead);
    }
    close(filefd);
}

/* Traitement de la connexion */
void ftp_service(int connfd) {
    request_t req;
    ssize_t n;

    while (1) {
        n = Rio_readn(connfd, &req, sizeof(request_t));
        if (n == 0) {  
            /* Le client a fermé la connexion normalement */
            break;
        }
        if (n != sizeof(request_t)) {
            fprintf(stderr, "Erreur lecture requête (attendu %ld octets, recu %ld)\n",
                    sizeof(request_t), n);
            break;
        }

        if (req.type == GET) {
            printf("Demande de fichier : %s\n", req.filename);
            send_file(connfd, req.filename);
        }
        else if (req.type == BYE) {
            printf("Commande BYE reçue, fermeture de la connexion.\n");
            break;
        }
        else {
            fprintf(stderr, "Type de requête non supporté : %d\n", req.type);
            response_t resp;
            resp.code = ERR_INVALID_REQUEST;
            resp.filesize = 0;
            Rio_writen(connfd, &resp, sizeof(response_t));
        }
    }
}


/* Gestionnaire de SIGINT pour le processus père */
void sigint_handler(int sig) {
    int i;
    printf("SIGINT reçu. Arrêt du serveur et terminaison des processus fils...\n");
    //Envoi du signal SIGINT à chaque enfant
    for (i = 0; i < NB_PROC; i++) {
        if (children[i] > 0) {
            Kill(children[i], SIGINT);
        }
    }
    //Attente de la terminaison de tous les processus enfants
    for (i = 0; i < NB_PROC; i++) {
        waitpid(children[i], NULL, 0);
    }
    exit(0);
}

int main(void) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    int i;
    
    //Ajout du handler de sigint 
    Signal(SIGINT, sigint_handler);
    
    //Ouverture du socket d'écoute sur le port 2121
    listenfd = Open_listenfd(PORT);
    clientlen = sizeof(clientaddr);
    
    //Création d'un pool de NB_PROC processus
    for (i = 0; i < NB_PROC; i++) {
        pid_t pid = Fork();
        if (pid == 0) { //Processus fils
            //Rétablir le comportement par defaut pour SIGINT dans les enfants
            Signal(SIGINT, SIG_DFL);
            while (1) {
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                ftp_service(connfd);
                Close(connfd);
            }
        } else {
            //Enregistrer l'ID du processus fils dans le tableau global
            children[i] = pid;
        }
    }
    
    /* Le processus père attend indéfiniment */
    while (1){
        pause();
    }
    
    return 0;
}