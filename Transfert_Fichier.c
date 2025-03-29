#include "Transfert_Fichier.h"

/* Envoi du fichier demande par blocs */
void send_file(int connfd, char *filename, int offset) {
    int filefd;
    struct stat statbuf;
    char path[MAX_NAME_LEN + 10];  // Pour "./server/" + nom de fichier

    /* Construction du chemin complet vers le fichier */
    snprintf(path, sizeof(path), "./server/%s", filename);


    /* Ouverture du fichier en mode lecture seule */
    if ((filefd = open(path, O_RDONLY)) < 0) {
        fprintf(stderr,"Erreur dans l'ouverture fichier");
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));  // Indique l'erreur au client
        return;
    }

    /* Récupération des infos sur le fichier */
    if (fstat(filefd, &statbuf) < 0) { 
        fprintf(stderr,"Erreur dans fstat");
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));
        close(filefd);
        return;
    }

    int total_size = statbuf.st_size; // Taille totale du fichier en octets
    
    /* Vérification que l'offset demandé est valide */
    if (offset > total_size) {
        fprintf(stderr, "Offset supérieur à la taille du fichier\n");
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));
        close(filefd);
        return;
    }

    /* Se positionner à l'offset demandé */
    if (lseek(filefd, offset, SEEK_SET) < 0) {
        fprintf(stderr, "Erreur lors de l'appel à lseek");
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));
        close(filefd);
        return;
    }

    // Nombre d'octets restants à envoyer
    int remaining = total_size - offset;

    /* Envoi de la taille restante du fichier au client */
    Rio_writen(connfd, &remaining, sizeof(int));

    /* Découpage du transfert en blocs de taille BLOCK_SIZE */
    int nb_full_blocks = remaining / BLOCK_SIZE;
    int remainder = remaining - (BLOCK_SIZE * nb_full_blocks);
    char buffer[BLOCK_SIZE]; // Buffer de lecture/écriture

    /* Envoi des blocs complets */
    for (int i = 0; i < nb_full_blocks; i++) {
        ssize_t bytes_read = read(filefd, buffer, BLOCK_SIZE);
        if (bytes_read != BLOCK_SIZE) {
            fprintf(stderr,"Erreur On n'a pas lu un bloc complet");
            close(filefd);
            return;
        }
        Rio_writen(connfd, buffer, BLOCK_SIZE);
        printf("Bloc %d envoyé (%d octets)\n", i + 1, BLOCK_SIZE);
    }
    
    /* Envoi du bloc partiel restant s'il existe*/ 
    if (remainder > 0) {
        ssize_t bytes_read = read(filefd, buffer, remainder);
        if (bytes_read != remainder) {
            fprintf(stderr,"Erreur On n'a pas pu faire la lecture du reste du fichier");
            close(filefd);
            return;
        }
        Rio_writen(connfd, buffer, remainder);
        printf("Bloc final envoyé (%d octets)\n", remainder);
    }

    close(filefd);
}
