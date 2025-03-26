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

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_host>\n", argv[0]);
        exit(1);
    }
    host = argv[1];

    /* Installer le gestionnaire SIGPIPE */
    Signal(SIGPIPE, sigpipe_handler);

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
        /* Envoi du type de requête et du nom du fichier */
        Rio_writen(clientfd, &type, sizeof(int));
        Rio_writen(clientfd, &filename_size, sizeof(int));
        Rio_writen(clientfd, req.filename, filename_size);

        if (req.type == REGET) {
            /* Lecture de l'offset stocké localement pour ce fichier */
            int offset = get_offset_from_log(req.filename);
            Rio_writen(clientfd, &offset, sizeof(int));
            printf("Reget : reprise du téléchargement à partir de l'offset %d\n", offset);
        }

        /* Chronométrage et réception de la taille (restante) du fichier */
        gettimeofday(&start, NULL);
        if (Rio_readn(clientfd, &file_size, sizeof(int)) != sizeof(int)) {
            fprintf(stderr, "Erreur lecture de la taille du fichier.\n");
            continue;
        }
        if (file_size < 0) {
            fprintf(stderr, "Erreur serveur : fichier non trouvé ou autre erreur.\n");
            continue;
        }

        /* Pour la réception, on va lire par blocs et mettre à jour le log */
        char buffer[MAXBUF];
        int received = 0;
        int fd;
        char client_file_path[MAX_NAME_LEN + 10];
        snprintf(client_file_path, sizeof(client_file_path), "./client/%s", req.filename);
        if (req.type == GET) {
            /* Nouveau téléchargement : création/tronquage du fichier */
            fd = Open(client_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else {
            /* Reprise de téléchargement : ouverture en mode écriture et positionnement à la fin */
            fd = Open(client_file_path, O_WRONLY | O_CREAT, 0644);
            received = get_offset_from_log(req.filename);
            lseek(fd, received, SEEK_SET);
        }

        while (received < file_size) {
            int to_read = (file_size - received > MAXBUF ? MAXBUF : file_size - received);
            int n = Rio_readn(clientfd, buffer, to_read);
            if (n <= 0) {
                fprintf(stderr, "Erreur ou fin de réception prématurée\n");
                break;
            }
            Rio_writen(fd, buffer, n);
            received += n;
            update_log(req.filename, received);
        }
        Close(fd);
        gettimeofday(&end, NULL);
        elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);

        if (received == file_size) {
            printf("Transfert terminé avec succès.\n");
            /* Optionnel : effacer le fichier log si le transfert est complet */
            char log_filename[MAX_NAME_LEN + 10];
            snprintf(log_filename, sizeof(log_filename), "%s.log", req.filename);
            remove(log_filename);
        } else {
            printf("Transfert interrompu. %d octets reçus sur %d.\n", received, file_size);
        }
        printf("%d bytes received in %.2f seconds (%.2f Kbytes/s).\n",
               received, elapsed_time, (received / 1024.0) / elapsed_time);
    }
    Close(clientfd);
    return 0;
}