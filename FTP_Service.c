#include "FTP_Service.h"

/* Service FTP : lecture de la requête et traitement */
void ftp_service(int connfd, struct sockaddr_in *clientaddr) {
    int type, filename_size;
    request_t req;
    ssize_t n;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    /* Récupération des informations du client */
    Inet_ntop(AF_INET, &clientaddr->sin_addr, client_ip_string, INET_ADDRSTRLEN);
    Getnameinfo((SA *)clientaddr, sizeof(*clientaddr), client_hostname, MAX_NAME_LEN, 0, 0, 0);
    printf("Client connecté : %s (%s)\n", client_hostname, client_ip_string);

    /* Boucle de traitement des requêtes sur la même connexion */
    while (1) {
        /* Lecture du type de requête */
        n = Rio_readn(connfd, &type, sizeof(int));
        if (n != sizeof(int)) {
            fprintf(stderr, "Erreur lecture du type de requête\n");
            break;
        }

        /* Si la commande BYE est reçue, on termine le traitement */
        if (type == BYE) {
            printf("Commande BYE reçue, fermeture de la connexion côté serveur\n");
            break;
        }

        /* Lecture de la taille du nom de fichier */
        n = Rio_readn(connfd, &filename_size, sizeof(int));
        if (n != sizeof(int)) {
            fprintf(stderr, "Erreur lecture de la taille du nom du fichier\n");
            break;
        }

        /* Lecture du nom du fichier */
        n = Rio_readn(connfd, req.filename, filename_size);
        if (n != filename_size) {
            fprintf(stderr, "Erreur lecture du nom du fichier\n");
            break;
        }
        req.filename[filename_size] = '\0';  // Assure la terminaison

        /* Traitement de la requête GET et REGET*/
        if (type == GET) {
            printf("Demande GET pour le fichier : %s\n", req.filename);
            send_file(connfd, req.filename, 0); // transfert complet
        } else if (type == REGET) {
            int offset;
            /* Lecture de l’offset envoyé par le client */
            n = Rio_readn(connfd, &offset, sizeof(int));
            if (n != sizeof(int)) {
                fprintf(stderr, "Erreur lecture de l’offset\n");
                break;
            }
            printf("Demande REGET pour le fichier : %s à partir de l’offset %d\n", req.filename, offset);
            send_file(connfd, req.filename, offset); // reprise du transfert
        } else {
            fprintf(stderr, "Type de requête non supporté : %d\n", type);
            response_t resp;
            resp.code = ERR_INVALID_REQUEST;
            resp.filesize = 0;
            Rio_writen(connfd, &resp, sizeof(response_t));
        }

    }
}