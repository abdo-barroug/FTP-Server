#ifndef __FTP_REQUEST_H__
#define __FTP_REQUEST_H__

#include "FTP_Log.h"

/**
 * Analyse une commande utilisateur FTP.
 *
 * Cette fonction analyse la commande saisie par l'utilisateur, la valide
 * et remplit la structure de requête FTP correspondante.
 *
 * parametres:  command_line Ligne de commande entrée par l'utilisateur.
 *              req          Structure à remplir avec le type de requête et le nom du fichier.
 *
 * retour:      0 en cas de succès, -1 si la commande est invalide ou non supportée.
 */
int parse_command(char *command_line, request_t *req);


/**
 * Envoie une requête FTP au serveur connecté.
 *
 * Cette fonction transmet au serveur le type de la requête (GET, REGET, BYE),
 * le nom du fichier associé, et, dans le cas d'un REGET, l'offset lu depuis le log.
 *
 * parametres:  clientfd Descripteur de fichier du socket connecté au serveur.
 *              req      Structure contenant les détails de la requête à envoyer.
 */
void send_request(int clientfd, request_t *req);


/**
 * Réceptionne un fichier suite à une requête GET.
 *
 * Cette fonction reçoit un fichier complet du serveur après une requête GET.
 * Elle écrit les données reçues dans le répertoire client et met à jour régulièrement
 * le log indiquant l'état d'avancement du téléchargement.
 *
 * parametres:  clientfd Descripteur de fichier du socket connecté au serveur.
 *              req      Structure contenant les détails du fichier demandé.
 *
 * retour:      Nombre total d'octets reçus en cas de succès, ou -1 en cas d'erreur.
 */
int receive_file_get(int clientfd, request_t *req);


/**
 * Réceptionne la reprise d'un transfert de fichier (REGET).
 *
 * Cette fonction poursuit la réception d'un fichier partiellement téléchargé,
 * à partir de l'offset enregistré dans le log associé. Les données reçues sont
 * écrites à la suite des données déjà présentes, et le log est mis à jour progressivement.
 *
 * parametres:  clientfd Descripteur de fichier du socket connecté au serveur.
 *              req      Structure contenant les détails du fichier demandé.
 *
 * retour:      Nombre total d'octets reçus après la reprise, ou -1 en cas d'erreur.
 */
int receive_file_reget(int clientfd, request_t *req);


/**
 * Effectue le transfert d'un fichier selon la requête spécifiée.
 *
 * Cette fonction appelle la méthode appropriée (GET ou REGET) pour recevoir
 * un fichier du serveur, selon le type de la requête reçue.
 *
 * parametres:  clientfd Descripteur de fichier du socket connecté au serveur.
 *              req      Structure contenant le type de requête et le nom du fichier concerné.
 *
 * retour:      Nombre total d'octets reçus ou -1 en cas d'erreur.
 */
int process_transfer(int clientfd, request_t *req);


#endif // __FTP_REQUEST_H__
