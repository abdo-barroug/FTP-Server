#include "FTP_Log.h"
#include "Signal_Handler_Client.h"


/* Analyse la ligne de commande saisie par l'utilisateur.
   Remplit la structure 'req'.
   Retourne 0 en cas de succès, -1 en cas d'erreur. */
int parse_command(char *command_line, request_t *req) {
    char cmd[10], fname[MAX_NAME_LEN];
    int nb_args = sscanf(command_line, "%s %s", cmd, fname);
    if (nb_args < 1) {
        fprintf(stderr, "Commande invalide\n");
        return -1;
    }
    if (strcasecmp(cmd, "bye") == 0) {
        req->type = BYE;
    } else if (strcasecmp(cmd, "get") == 0 && nb_args == 2) {
        req->type = GET;
    } else if (strcasecmp(cmd, "reget") == 0 && nb_args == 2) {
        req->type = REGET;
    } else {
        fprintf(stderr, "Commande non supportée. Usage: get <filename>, reget <filename> ou bye\n");
        return -1;
    }
    if (req->type != BYE) {
        strncpy(req->filename, fname, MAX_NAME_LEN);
        req->filename[MAX_NAME_LEN - 1] = '\0';
    }
    return 0;
}

/* Envoie la requête au serveur.
   Pour REGET, envoie aussi l'offset enregistré dans le log. */
void send_request(int clientfd, request_t *req) {
    int type = req->type;
    int filename_size = strlen(req->filename);
    Rio_writen(clientfd, &type, sizeof(int));
    Rio_writen(clientfd, &filename_size, sizeof(int));
    Rio_writen(clientfd, req->filename, filename_size);
    if (req->type == REGET) {
        int offset = get_offset_from_log(req->filename);
        Rio_writen(clientfd, &offset, sizeof(int));
        printf("Reget : reprise du téléchargement à partir de l'offset %d\n", offset);
    }
}

/* Réception pour un transfert complet (GET).
   Lit la taille totale envoyée par le serveur et télécharge le fichier.
   Retourne le nombre total d'octets téléchargés ou -1 en cas d'erreur. */
int receive_file_get(int clientfd, request_t *req) {
    int file_size;
    if (Rio_readn(clientfd, &file_size, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "Erreur lecture de la taille du fichier.\n");
        return -1;
    }
    if (file_size < 0) {
        fprintf(stderr, "Erreur serveur : fichier non trouvé ou autre erreur.\n");
        return -1;
    }
    char client_file_path[MAX_NAME_LEN + 10];
    snprintf(client_file_path, sizeof(client_file_path), "./client/%s", req->filename);
    int fd = Open(client_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    char buffer[BLOCK_SIZE];
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
        update_log(req->filename, total_received);
        //printf("Reçu %d octets, total reçu = %d/%d\n", n, total_received, file_size);
    }
    Close(fd);
    return total_received;
}

/* Réception pour une reprise (REGET).
   Le serveur envoie la taille restante (total_size - offset).
   Le client lit ces données et les ajoute au fichier existant.
   Retourne le total téléchargé (offset + nouveaux octets reçus) ou -1 en cas d'erreur. */
int receive_file_reget(int clientfd, request_t *req) {
    int file_size;
    if (Rio_readn(clientfd, &file_size, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "Erreur lecture de la taille restante du fichier.\n");
        return -1;
    }
    if (file_size < 0) {
        fprintf(stderr, "Erreur serveur : fichier non trouvé ou autre erreur.\n");
        return -1;
    }
    int offset = get_offset_from_log(req->filename);
    char client_file_path[MAX_NAME_LEN + 10];
    snprintf(client_file_path, sizeof(client_file_path), "./client/%s", req->filename);
    int fd = Open(client_file_path, O_WRONLY | O_CREAT, 0644);
    lseek(fd, offset, SEEK_SET);
    
    char buffer[BLOCK_SIZE];
    int new_received = 0;
    while (new_received < file_size) {
        int to_read = (file_size - new_received > BLOCK_SIZE ? BLOCK_SIZE : file_size - new_received);
        int n = Rio_readn(clientfd, buffer, to_read);
        if (n <= 0) {
            fprintf(stderr, "Erreur ou fin de réception prématurée, n = %d\n", n);
            break;
        }
        Rio_writen(fd, buffer, n);
        new_received += n;
        update_log(req->filename, offset + new_received);
        //printf("Reçu %d octets, ajouté = %d/%d\n", n, new_received, file_size);
    }
    Close(fd);
    printf("Reget terminé, total téléchargé = %d octets\n", offset + new_received);
    return offset + new_received;
}

/* Traite le transfert en appelant la fonction adéquate selon le type de requête.
   Retourne le nombre total d'octets téléchargés ou -1 en cas d'erreur. */
int process_transfer(int clientfd, request_t *req) {
    if (req->type == GET)
        return receive_file_get(clientfd, req);
    else if (req->type == REGET)
        return receive_file_reget(clientfd, req);
    else
        return -1;
}

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char command_line[512];
    request_t req;
    struct timeval start, end;
    double elapsed_time;

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

        if (parse_command(command_line, &req) != 0)
            continue;

        if (req.type == BYE) {
            Rio_writen(clientfd, &req.type, sizeof(int));
            printf("Déconnexion.\n");
            break;
        }

        /* Envoi de la requête */
        send_request(clientfd, &req);

        /* Chronométrage et transfert */
        gettimeofday(&start, NULL);
        int downloaded = process_transfer(clientfd, &req);
        gettimeofday(&end, NULL);
        elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);

        /* Pour GET, une fois le transfert complet, effacer le log */
        if (req.type == GET) {
            char log_filename[MAX_NAME_LEN + 10];
            snprintf(log_filename, sizeof(log_filename), "%s.log", req.filename);
            remove(log_filename);
        }
        printf("%d bytes received in %.2f seconds (%.2f Kbytes/s).\n",
               downloaded, elapsed_time, (downloaded / 1024.0) / elapsed_time);
    }
    Close(clientfd);
    return 0;
}