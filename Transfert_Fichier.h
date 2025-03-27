#ifndef __TRANSFERT_FICHIER_H__
#define __TRANSFERT_FICHIER_H__

#include "FTP_Structures.h"

/**
 * Envoie au client le contenu d'un fichier en blocs de données.
 *
 * Cette fonction prend en charge l'envoi complet ou partiel d'un fichier demandé par le client.
 * Le transfert s'effectue par blocs de taille fixe (BLOCK_SIZE). Si l'offset spécifié est différent
 * de zéro, l'envoi commencera à partir de cet offset, permettant ainsi une reprise du transfert.
 * En cas d'erreur (fichier inexistant, problème de lecture ou d'accès), la fonction signale 
 * explicitement l'erreur au client.
 *
 * parametres:  connfd   Descripteur de fichier de la socket connectée au client.
 *              filename Nom du fichier à envoyer.
 *              offset   Position dans le fichier à partir de laquelle l'envoi doit débuter.
 */
void send_file(int connfd, char *filename, int offset);

#endif