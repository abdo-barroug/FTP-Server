#ifndef __FTPSERVER_H__
#define __FTPSERVER_H__

#include "csapp.h"
#include <fcntl.h>

#define NB_PROC          4           /* Nombre de processus dans le pool */
#define PORT             2121        /* Port d'écoute du serveur FTP */
#define MAX_FILENAME_LEN 256         /* Taille max du nom de fichier */
pid_t children[NB_PROC];


/* Codes de retour */
#define SUCCESS             0
#define ERR_INVALID_REQUEST 1
#define ERR_FILE_NOT_FOUND  2

/* Structure de la réponse du serveur */
typedef struct {
    int code;        /* 0 = succès, autre valeur = code d'erreur */
    int filesize;    /* Taille du fichier en octets (valide si code == 0) */
} response_t;

/* Prototypes */
void send_file(int connfd, char *filename);

#endif // __FTPSERVER_H__
