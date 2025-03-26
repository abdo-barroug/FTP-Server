#include "FTP_Structures.h"

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

    while (1) {
        printf("ftp> ");
        if (Fgets(command_line, sizeof(command_line), stdin) == NULL) {
            fprintf(stderr, "Erreur lecture de la commande.\n");
            break;
        }

        /* Extraction de la commande et du nom de fichier (si applicable) */
        char cmd[10], fname[MAX_NAME_LEN];
        int nb_args = sscanf(command_line, "%s %s", cmd, fname);
        if (nb_args < 1) {
            fprintf(stderr, "Commande invalide\n");
            continue;
        }

        /* Traitement de la commande bye */
        if (strcasecmp(cmd, "bye") == 0) {
            req.type = BYE;
            Rio_writen(clientfd, &req.type, sizeof(int));
            printf("Déconnexion.\n");
            break;
        }

        /* Pour l'instant, seule la commande GET est supportée */
        if (strcasecmp(cmd, "get") != 0 || nb_args != 2) {
            fprintf(stderr, "Commande non supportée. Usage: get <filename> ou bye\n");
            continue;
        }
        req.type = GET;
        strncpy(req.filename, fname, MAX_NAME_LEN);
        req.filename[MAX_NAME_LEN - 1] = '\0';

        /* Envoi de la requête au serveur */
        int type = req.type;
        int filename_size = strlen(req.filename);
        Rio_writen(clientfd, &type, sizeof(int));
        Rio_writen(clientfd, &filename_size, sizeof(int));
        Rio_writen(clientfd, req.filename, filename_size);

        /* Chronométrage et réception du fichier */
        gettimeofday(&start, NULL);
        if (Rio_readn(clientfd, &file_size, sizeof(int)) != sizeof(int)) {
            fprintf(stderr, "Erreur lecture de la taille du fichier.\n");
            continue;
        }
        if (file_size < 0) {
            fprintf(stderr, "Erreur serveur : fichier non trouvé ou autre erreur.\n");
            continue;
        }
        file_buf = Malloc(file_size);
        if (Rio_readn(clientfd, file_buf, file_size) != file_size) {
            fprintf(stderr, "Erreur réception du fichier.\n");
            Free(file_buf);
            continue;
        }
        gettimeofday(&end, NULL);
        elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);

        /* Sauvegarde du fichier dans le répertoire ./client */
        {
            char client_file_path[MAX_NAME_LEN + 10];
            snprintf(client_file_path, sizeof(client_file_path), "./client/%s", req.filename);
            int fd = Open(client_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            Rio_writen(fd, file_buf, file_size);
            Close(fd);
        }
        Free(file_buf);
        printf("Transfer successfully complete.\n");
        printf("%d bytes received in %.2f seconds (%.2f Kbytes/s).\n",
               file_size, elapsed_time, (file_size / 1024.0) / elapsed_time);
    }
    Close(clientfd);
    return 0;
}
