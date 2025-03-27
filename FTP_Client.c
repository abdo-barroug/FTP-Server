#include "Signal_Handler_Client.h"
#include "FTP_Request.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char command_line[256];
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