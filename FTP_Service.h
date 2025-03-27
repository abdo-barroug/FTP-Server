#ifndef __FTP_SERVICE_H__
#define __FTP_SERVICE_H__

#include "Transfert_Fichier.h"

/**
 * Gère une session FTP avec un client connecté.
 *
 * Cette fonction prend en charge une connexion avec un client, traite les requêtes FTP
 * envoyées par ce client (GET, REGET, BYE) et exécute les opérations correspondantes
 * (envoi complet ou reprise de transfert d'un fichier). La connexion reste ouverte
 * jusqu'à la réception d'une requête BYE ou en cas d'erreur de communication.
 *
 * parametres:  connfd Descripteur de fichier de la socket connectée au client.
 *              clientaddr Structure contenant les informations réseau du client connecté.
 */
void ftp_service(int connfd, struct sockaddr_in *clientaddr);


#endif /* __FTP_SERVICE_H__ */