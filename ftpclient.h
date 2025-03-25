    #ifndef __FTPCLIENT_H__
    #define __FTPCLIENT_H__

    #include "csapp.h"

    #define PORT             2121        /* Port fixe du serveur FTP */
    #define MAXBUF           8192        /* Taille du buffer pour le transfert */
    #define MAX_FILENAME_LEN 256         /* Taille max du nom de fichier */

    /* Définition de l'énumération pour les types de requête */
    typedef enum {
        GET = 1,  /* 1 pour GET */
        PUT = 2,  /* 2 pour PUT */
        LS = 3    /* 3 pour LS */
    } typereq_t;

    /* Structure de la requête client */
    typedef struct {
        typereq_t type;
        char filename[MAX_FILENAME_LEN];
    } request_t;

    #endif /* __FTPCLIENT_H__ */