#include "ftpserver.h"
#include "ftpclient.h"

/* Service FTP : lecture de la requête et traitement */
void ftp_service(int connfd) {
    request_t req;
    response_t resp;
    ssize_t n;

    /* Lecture binaire de la requête */
    n = Rio_readn(connfd, &req, sizeof(request_t));
    if (n != sizeof(request_t)) {
        fprintf(stderr, "Erreur lecture requête (attendu %ld octets, recu %ld)\n",
                sizeof(request_t), n);
        return;
    }

    if (req.type == GET) {
        printf("Demande de fichier : %s\n", req.filename);
        send_file(connfd, req.filename);
    }
    else {
        fprintf(stderr, "Type de requête non supporté : %d\n", req.type);
        resp.code = ERR_INVALID_REQUEST;
        resp.filesize = 0;
        Rio_writen(connfd, &resp, sizeof(response_t));
    }
}

void send_file(int connfd, char *filename) {
    int filefd;
    struct stat statbuf;
    response_t resp;
    char *file_buf;
    char path[MAX_FILENAME_LEN + 10];  // Suffisamment grand pour "./server/" + nom

    /* Construction du chemin complet vers le fichier dans le répertoire ./server */
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

    /* Chargement du fichier en mémoire */
    file_buf = Malloc(statbuf.st_size);
    if (Rio_readn(filefd, file_buf, statbuf.st_size) != statbuf.st_size) {
        perror("Erreur lecture fichier");
        resp.code = ERR_FILE_NOT_FOUND;
        resp.filesize = 0;
        Rio_writen(connfd, &resp, sizeof(response_t));
        Free(file_buf);
        close(filefd);
        return;
    }
    close(filefd);

    /* Envoi de la réponse de succès */
    resp.code = SUCCESS;
    resp.filesize = statbuf.st_size;
    Rio_writen(connfd, &resp, sizeof(response_t));

    /* Envoi du contenu du fichier en mode binaire */
    Rio_writen(connfd, file_buf, statbuf.st_size);
    Free(file_buf);
}



/* Gestionnaire de SIGINT pour le processus père */
void sigint_handler(int sig) {
    int i;
    printf("SIGINT reçu. Arrêt du serveur et terminaison des processus fils...\n");
    /* Envoi du signal SIGINT à chaque enfant */
    for (i = 0; i < NB_PROC; i++) {
        if (children[i] > 0) {
            Kill(children[i], SIGINT);
        }
    }
    /* Attente de la terminaison de tous les processus enfants */
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
    
    /* Installation du traitant de SIGINT dans le processus père */
    Signal(SIGINT, sigint_handler);
    
    /* Ouverture du socket d'écoute sur le port 2121 */
    listenfd = Open_listenfd(PORT);
    clientlen = sizeof(clientaddr);
    
    /* Création d'un pool de NB_PROC processus */
    for (i = 0; i < NB_PROC; i++) {
        pid_t pid = Fork();
        if (pid == 0) { /* Processus fils */
            /* Rétablir le comportement par défaut pour SIGINT dans les enfants */
            Signal(SIGINT, SIG_DFL);
            while (1) {
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                ftp_service(connfd);
                Close(connfd);
            }
            exit(0); /* Ce point n'est jamais atteint */
        } else {
            /* Enregistrer l'ID du processus fils dans le tableau global */
            children[i] = pid;
        }
    }
    
    /* Le processus père attend indéfiniment */
    while (1)
        pause();
    
    return 0;
}