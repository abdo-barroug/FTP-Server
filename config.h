#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "csapp.h"

#define NB_PROC          4           /* Nombre de processus dans le pool */
#define PORT             2121        /* Port d'écoute du serveur FTP */
#define MAX_FILENAME_LEN 256         /* Taille max du nom de fichier */
#define MAX_NAME_LEN 265
#define MAXBUF 8192                  /* Taille du buffer pour le transfert */

/* Codes de retour */
#define SUCCESS             0
#define ERR_INVALID_REQUEST 1
#define ERR_FILE_NOT_FOUND  2

/* Définition de l'énumération pour les types de requête */
typedef enum {
    GET = 1, /* 1 pour GET */
    PUT = 2, /* 2 pour PUT */
    LS  = 3  /* 3 pour LS */
} typereq_t;

/* Structure de la requête client */
typedef struct {
    typereq_t type;
    char filename[MAX_FILENAME_LEN];
} request_t;

/* Structure de la réponse du serveur */
typedef struct {
    int code;        /* 0 = succès, autre valeur = code d'erreur */
    int filesize;    /* Taille du fichier en octets (valide si code == 0) */
} response_t;


#endif /* __CONFIG_H__ */
