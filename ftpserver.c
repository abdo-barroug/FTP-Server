#include "ftpserver.h"

int listenfd;  
pid_t children[NB_PROC];


/* Gestionnaire de SIGINT pour le processus père */
void sigint_handler(int sig) {
    int i;

    printf("SIGINT reçu. Arrêt du serveur et terminaison des processus fils...\n");

    // Attendre la terminaison de tous les processus enfants
    while (wait(NULL) > 0) {
        // Rien à faire dans cette boucle
    }

    // Libérer les processus enfants
    if (children != NULL) {
        for (i = 0; i < NB_PROC; i++) {
            if (children[i] > 0) {
                Kill(children[i], SIGINT);
            }
        }
    }

    // Fermer les sockets et effectuer le nettoyage des ressources
    Close(listenfd);  // Fermer le socket d'écoute
    for (i = 0; i < NB_PROC; i++) {
        if (children[i] > 0) {
            // Fermer d'autres ressources si nécessaires pour chaque processus
            Close(children[i]);  // Fermer les connexions si ouvertes
        }
    }

    // Libérer la mémoire allouée si nécessaire (si children ou autres structures nécessitent un free)
    if (children != NULL) {
        Free(children);  // Si la mémoire a été allouée dynamiquement
    }

    printf("Nettoyage effectué, serveur arrêté.\n");
    exit(0);
}

/* Service FTP : lecture de la requête et traitement */
void ftp_service(int connfd, struct sockaddr_in *clientaddr) {
    int type, filename_size;
    request_t req;
    ssize_t n;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    
    /* Obtenez l'adresse IP et le nom d'hôte du client */
    Inet_ntop(AF_INET, &clientaddr->sin_addr, client_ip_string, INET_ADDRSTRLEN);
    Getnameinfo((SA *)clientaddr, sizeof(*clientaddr), client_hostname, MAX_NAME_LEN, 0, 0, 0);

    /* Affichage des informations du client */
    printf("Serveur connecté à %s (%s)\n", client_hostname, client_ip_string);

    /* Lire le type de la requête (GET, PUT, LS) */
    n = Rio_readn(connfd, &type, sizeof(int));
    if (n != sizeof(int)) {
        fprintf(stderr, "Erreur lecture du type de requête\n");
        return;
    }

    /* Lire la taille du nom de fichier */
    n = Rio_readn(connfd, &filename_size, sizeof(int));
    if (n != sizeof(int)) {
        fprintf(stderr, "Erreur lecture de la taille du nom du fichier\n");
        return;
    }

    /* Lire le nom du fichier */
    n = Rio_readn(connfd, req.filename, filename_size);
    if (n != filename_size) {
        fprintf(stderr, "Erreur lecture du nom du fichier\n");
        return;
    }
    req.filename[filename_size] = '\0';  // Assurer que le nom de fichier est bien terminé

    /* Traiter la requête en fonction du type */
    if (type == GET) {
        printf("Demande de fichier : %s\n", req.filename);
        send_file(connfd, req.filename);
    } else {
        fprintf(stderr, "Type de requête non supporté : %d\n", type);
        response_t resp;
        resp.code = ERR_INVALID_REQUEST;
        resp.filesize = 0;
        Rio_writen(connfd, &resp, sizeof(response_t));
    }
}

/* Envoi du fichier demandé */
void send_file(int connfd, char *filename) {
    int filefd;
    struct stat statbuf;
    char path[MAX_FILENAME_LEN + 10];  // Pour "./server/" + nom

    /* Construction du chemin complet vers le fichier dans le répertoire ./server */
    snprintf(path, sizeof(path), "./server/%s", filename);

    if ((filefd = open(path, O_RDONLY)) < 0) {
        perror("Erreur ouverture fichier");
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));  // Envoi d'une valeur négative pour signaler une erreur
        return;
    }

    if (fstat(filefd, &statbuf) < 0) {
        perror("Erreur fstat");
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));
        close(filefd);
        return;
    }

    /* Envoi de la taille du fichier au client */
    int file_size = statbuf.st_size;
    Rio_writen(connfd, &file_size, sizeof(int));

    /* Chargement du fichier en mémoire et envoi du contenu */
    char *file_buf = Malloc(file_size);
    if (Rio_readn(filefd, file_buf, file_size) != file_size) {
        perror("Erreur lecture fichier");
        Free(file_buf);
        close(filefd);
        return;
    }
    close(filefd);

    /* Envoi du contenu du fichier en une seule fois */
    Rio_writen(connfd, file_buf, file_size);
    Free(file_buf);
}


int main() {
    int connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    int i;
    
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
