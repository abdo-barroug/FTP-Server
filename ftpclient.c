#include "ftpclient.h"
#include "ftpserver.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char command_line[512];
    request_t req;
    int file_size;
    char *file_buf;
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
        fprintf(stderr, "Erreur lecture de la commande.\n");
        exit(1);
    }

    /* Extraction de la commande et du nom de fichier */
    {
        char cmd[10], fname[MAX_FILENAME_LEN];
        if (sscanf(command_line, "%s %s", cmd, fname) != 2) {
            fprintf(stderr, "Commande invalide. Usage: get <filename>\n");
            exit(1);
        }
        if (strcasecmp(cmd, "get") != 0) {
            fprintf(stderr, "Commande non supportée. Seule la commande 'get' est supportée.\n");
            exit(1);
        }

        /* Préparation de la requête */
        req.type = GET;
        strncpy(req.filename, fname, MAX_FILENAME_LEN);
        req.filename[MAX_FILENAME_LEN - 1] = '\0';
    }

    /* Envoi du type de requête */
    int type = req.type;
    int filename_size = strlen(req.filename);

    /* Envoi du type de requête, taille du nom de fichier et du nom du fichier */
    Rio_writen(clientfd, &type, sizeof(int));
    Rio_writen(clientfd, &filename_size, sizeof(int));
    Rio_writen(clientfd, req.filename, filename_size);

    /* Démarrage du chronomètre */
    gettimeofday(&start, NULL);

    /* Réception de la taille du fichier envoyé par le serveur */
    if (Rio_readn(clientfd, &file_size, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "Erreur lors de la lecture de la taille du fichier.\n");
        exit(1);
    }
    if (file_size < 0) {
        fprintf(stderr, "Erreur serveur : fichier non trouvé ou autre erreur.\n");
        exit(1);
    }

    /* Réception du contenu du fichier en une seule fois */
    file_buf = Malloc(file_size);
    if (Rio_readn(clientfd, file_buf, file_size) != file_size) {
        fprintf(stderr, "Erreur lors de la réception du fichier.\n");
        Free(file_buf);
        exit(1);
    }

    /* Arrêt du chronomètre */
    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);

    /* Sauvegarde du fichier dans le répertoire ./client */
    {
        char client_file_path[MAX_FILENAME_LEN + 10];
        snprintf(client_file_path, sizeof(client_file_path), "./client/%s", req.filename);
        int fd = Open(client_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        Rio_writen(fd, file_buf, file_size);
        Close(fd);
    }
    Free(file_buf);

    /* Affichage des statistiques de transfert */
    printf("Transfer successfully complete.\n");
    printf("%d bytes received in %.2f seconds (%.2f Kbytes/s).\n", file_size, elapsed_time, (file_size / 1024.0) / elapsed_time);

    Close(clientfd);
    return 0;
}
