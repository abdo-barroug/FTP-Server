#ifndef __FTP_LOG_H__
#define __FTP_LOG_H__

#include "FTP_Structures.h"

/**
 * Renvoie l'offset enregistré dans le fichier log associé au fichier spécifié.
 *
 * Cette fonction récupère la dernière position de transfert (offset) enregistrée
 * dans un fichier log portant le nom "nomfichier.log". Si le fichier log n'existe pas,
 * la fonction renvoie un offset par défaut égal à 0.
 *
 * parametres:  filename Nom du fichier concerné (sans l'extension .log).
 *
 * retour:      Offset récupéré dans le fichier log ou 0 si le log est inexistant.
 */
int get_offset_from_log(const char *filename);


/**
 * Met à jour le fichier log associé au fichier spécifié avec l'offset indiqué.
 *
 * Cette fonction enregistre l'offset actuel dans un fichier log nommé "nomfichier.log".
 * Elle crée ou écrase le fichier log existant. En cas d'erreur d'ouverture ou d'écriture,
 * un message d'erreur est affiché sur la sortie d'erreur standard.
 *
 * parametres:  filename Nom du fichier concerné (sans l'extension .log).
 *              offset   Offset à enregistrer dans le fichier log.
 */
void update_log(const char *filename, int offset);


#endif /* __FTP_LOG_H__ */