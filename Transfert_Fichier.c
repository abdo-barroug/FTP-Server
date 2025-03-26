#include "Transfert_Fichier.h"

/* Envoi du fichier demande par blocs */
void send_file(int connfd, char *filename) {
    int filefd;
    struct stat statbuf;
    char path[MAX_FILENAME_LEN + 10];  // Pour "./server/" + nom de fichier

    /* Construction du chemin complet vers le fichier */
    snprintf(path, sizeof(path), "./server/%s", filename);

    if ((filefd = open(path, O_RDONLY)) < 0) {
        perror("Erreur ouverture fichier");
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));  // Indique l'erreur au client
        return;
    }

    if (fstat(filefd, &statbuf) < 0) {
        perror("Erreur fstat");
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));
        close(filefd);
        return;
    }

    int file_size = statbuf.st_size;
    /* Envoi de la taille totale du fichier au client */
    Rio_writen(connfd, &file_size, sizeof(int));

    /* Découpage du transfert en blocs de taille BLOCK_SIZE */
    int nb_full_blocks = file_size / BLOCK_SIZE;       // Nombre de blocs complets
    int remainder = file_size - (BLOCK_SIZE * nb_full_blocks);  // Le reste (inférieur à BLOCK_SIZE)

    char buffer[BLOCK_SIZE];

    /* Envoi des blocs complets */
    for (int i = 0; i < nb_full_blocks; i++) {
        ssize_t bytes_read = read(filefd, buffer, BLOCK_SIZE);
        if (bytes_read != BLOCK_SIZE) {
            perror("Erreur lecture bloc complet");
            close(filefd);
            return;
        }
        Rio_writen(connfd, buffer, BLOCK_SIZE);
        printf("Bloc %d envoyé (%d octets)\n", i + 1, BLOCK_SIZE);
    }

    /* Envoi du bloc final contenant le reste (s'il y en a un) */
    if (remainder > 0) {
        ssize_t bytes_read = read(filefd, buffer, remainder);
        if (bytes_read != remainder) {
            perror("Erreur lecture du reste du fichier");
            close(filefd);
            return;
        }
        Rio_writen(connfd, buffer, remainder);
        printf("Bloc final envoyé (%d octets)\n", remainder);  //debogage
    }

    close(filefd);
}