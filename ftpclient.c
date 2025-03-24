#include "ftpclient.h"
#include "ftpserver.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    request_t req;
    response_t resp;
    char *file_buf;
    char command_line[512];
    char cmd[10], filename[MAX_FILENAME_LEN];
    struct timeval start, end;
    double elapsed_time;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_host>\n", argv[0]);
        exit(1);
    }
    host = argv[1];

    /* Connexion au serveur FTP */
    clientfd = Open_clientfd(host, PORT);
    printf("Connected to %s.\n", host);
    
    /* Affichage de l'invite ftp> et lecture de la commande */
    printf("ftp> ");
    if (Fgets(command_line, sizeof(command_line), stdin) == NULL) {
        fprintf(stderr, "Erreur de lecture de la commande.\n");
        exit(1);
    }
    /* Analyse de la commande attendue : "get <nom_du_fichier>" */
    if (sscanf(command_line, "%s %s", cmd, filename) != 2) {
        fprintf(stderr, "Commande invalide.\n");
        exit(1);
    }
    if (strcasecmp(cmd, "get") != 0) {
        fprintf(stderr, "Commande non supportée. Seule la commande 'get' est supportée.\n");
        exit(1);
    }
    /* Préparation de la requête */
    req.type = GET;
    strncpy(req.filename, filename, MAX_FILENAME_LEN);
    req.filename[MAX_FILENAME_LEN - 1] = '\0';
    
    /* Envoi de la requête au serveur */
    Rio_writen(clientfd, &req, sizeof(req));
    
    /* Démarrage du chronomètre avant de recevoir la réponse */
    gettimeofday(&start, NULL);

    /* Lecture de la réponse du serveur */
    if (Rio_readn(clientfd, &resp, sizeof(response_t)) != sizeof(response_t)) {
        fprintf(stderr, "Erreur lors de la lecture de la réponse.\n");
        exit(1);
    }
    if (resp.code != SUCCESS) {
        fprintf(stderr, "Erreur serveur : code %d\n", resp.code);
        exit(1);
    }
    
    /* Réception du fichier en une seule fois, selon la taille annoncée par le serveur */
    file_buf = Malloc(resp.filesize);
    if (Rio_readn(clientfd, file_buf, resp.filesize) != resp.filesize) {
        fprintf(stderr, "Erreur lors de la réception du fichier.\n");
        Free(file_buf);
        exit(1);
    }
    
    /* Arrêt du chronomètre après la réception */
    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);
    
    /* Construction du chemin complet pour sauvegarder le fichier dans ./client */
    char client_file_path[MAX_FILENAME_LEN + 10];
    snprintf(client_file_path, sizeof(client_file_path), "./client/%s", filename);
    
    int fd = Open(client_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    Rio_writen(fd, file_buf, resp.filesize);
    Close(fd);
    Free(file_buf);
    
    /* Affichage du message de transfert réussi et des statistiques */
    printf("Transfer successfully complete.\n");
    printf("%d bytes received in %.2f seconds (%.2f Kbytes/s).\n", 
           resp.filesize, elapsed_time, (resp.filesize / 1024.0) / elapsed_time);
    
    Close(clientfd);
    return 0;
}