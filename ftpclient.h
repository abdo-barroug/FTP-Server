#ifndef __FTPCLIENT_H__
#define __FTPCLIENT_H__

#include "csapp.h"
#include "ftpserver.h"

#define PORT             2121 //Port fixe du serveur FTP
#define MAXBUF           8192 //Taille du buffer pour le transfert
#define MAX_FILENAME_LEN 256  //Taille max du nom de fichier

/* Même définition pour le type de requête */
typedef enum {
    GET,
    PUT,
    LS,
    BYE
} typereq_t;

/* Structure de la requête client */
typedef struct {
    int type;
    char filename[MAX_FILENAME_LEN];
} request_t;

#endif 