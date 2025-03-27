#include "FTP_Log.h"
#include "Signal_Handler_Client.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char command_line[512];
    request_t req;
    int file_size;
    struct timeval start, end;
    double elapsed_time;
    int new_received = 0;  // Pour le REGET, octets reçus durant la reprise

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_host>\n", argv[0]);
        exit(1);
    }
    host = argv[1];

    /* Installer le gestionnaire SIGPIPE */
    signal(SIGPIPE, sigpipe_handler);

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

        /* Commande de déconnexion */
        if (strcasecmp(cmd, "bye") == 0) {
            req.type = BYE;
            Rio_writen(clientfd, &req.type, sizeof(int));
            printf("Déconnexion.\n");
            break;
        }
        /* Commande GET pour un transfert complet */
        else if (strcasecmp(cmd, "get") == 0 && nb_args == 2) {
            req.type = GET;
        }
        /* Commande REGET pour reprendre un transfert interrompu */
        else if (strcasecmp(cmd, "reget") == 0 && nb_args == 2) {
            req.type = REGET;
        }
        else {
            fprintf(stderr, "Commande non supportée. Usage: get <filename>, reget <filename> ou bye\n");
            continue;
        }
        
        /* Préparation de la requête */
        strncpy(req.filename, fname, MAX_NAME_LEN);
        req.filename[MAX_NAME_LEN - 1] = '\0';

        int type = req.type;
        int filename_size = strlen(req.filename);
        /* Envoi du type de requête, de la taille et du nom du fichier */
        Rio_writen(clientfd, &type, sizeof(int));
        Rio_writen(clientfd, &filename_size, sizeof(int));
        Rio_writen(clientfd, req.filename, filename_size);

        int offset = 0;
        if (req.type == REGET) {
            /* Lecture de l'offset stocké localement */
            offset = get_offset_from_log(req.filename);
            Rio_writen(clientfd, &offset, sizeof(int));
            printf("Reget : reprise du téléchargement à partir de l'offset %d\n", offset);
        }

        /* Réception de la taille envoyée par le serveur
           - Pour GET : taille totale du fichier.
           - Pour REGET : nombre d'octets restants (total - offset) */
        gettimeofday(&start, NULL);
        if (Rio_readn(clientfd, &file_size, sizeof(int)) != sizeof(int)) {
            fprintf(stderr, "Erreur lecture de la taille du fichier.\n");
            continue;
        }
        if (file_size < 0) {
            fprintf(stderr, "Erreur serveur : fichier non trouvé ou autre erreur.\n");
            continue;
        }

        /* Déclaration du buffer pour la réception */
        char buffer[BLOCK_SIZE];

        /* Ouverture du fichier dans le répertoire client */
        char client_file_path[MAX_NAME_LEN + 10];
        snprintf(client_file_path, sizeof(client_file_path), "./client/%s", req.filename);
        int fd;
        if (req.type == GET) {
            /* Nouveau téléchargement : on tronque le fichier local */
            fd = Open(client_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            int total_received = 0;
            while (total_received < file_size) {
                int to_read = (file_size - total_received > BLOCK_SIZE ? BLOCK_SIZE : file_size - total_received);
                int n = Rio_readn(clientfd, buffer, to_read);
                if (n <= 0) {
                    fprintf(stderr, "Erreur ou fin de réception prématurée, n = %d\n", n);
                    break;
                }
                Rio_writen(fd, buffer, n);
                total_received += n;
                update_log(req.filename, total_received);
                //printf("Reçu %d octets, total reçu = %d/%d\n", n, total_received, file_size);
            }
        } else {  /* REGET */
            /* Ouvre le fichier sans le tronquer et se positionne à l'offset */
            fd = Open(client_file_path, O_WRONLY | O_CREAT, 0644);
            lseek(fd, offset, SEEK_SET);
            new_received = 0;
            while (new_received < file_size) {
                int to_read = (file_size - new_received > BLOCK_SIZE ? BLOCK_SIZE : file_size - new_received);
                int n = Rio_readn(clientfd, buffer, to_read);
                if (n <= 0) {
                    fprintf(stderr, "Erreur ou fin de réception prématurée, n = %d\n", n);
                    break;
                }
                Rio_writen(fd, buffer, n);
                new_received += n;
                update_log(req.filename, offset + new_received);
                //printf("Reçu %d octets, ajouté = %d/%d\n", n, new_received, file_size);
            }
            printf("Reget terminé, total téléchargé = %d octets\n", offset + new_received);
        }
        Close(fd);
        gettimeofday(&end, NULL);
        elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);

        /* Pour GET, une fois le transfert complet, effacer le log */
        if (req.type == GET) {
            char log_filename[MAX_NAME_LEN + 10];
            snprintf(log_filename, sizeof(log_filename), "%s.log", req.filename);
            remove(log_filename);
        }
        int total_downloaded = (req.type == GET ? file_size : (offset + new_received));
        printf("%d bytes received in %.2f seconds (%.2f Kbytes/s).\n",
               total_downloaded, elapsed_time, (total_downloaded / 1024.0) / elapsed_time);
    }
    Close(clientfd);
    return 0;
}