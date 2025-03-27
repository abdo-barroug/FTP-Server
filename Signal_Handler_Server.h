#ifndef __SIGNAL_HANDLER_Server_H__
#define __SIGNAL_HANDLER_Server_H__

#include "FTP_Structures.h"

/**
 * Gestionnaire du signal SIGINT côté serveur.
 *
 * Cette fonction traite le signal SIGINT (Ctrl+C), permettant au serveur
 * de s’arrêter proprement en terminant tous les processus fils actifs. 
 * À la réception du signal, la fonction affiche un message, envoie un SIGINT
 * à tous les processus fils, attend leur terminaison, puis termine le processus principal.
 *
 */
void sigint_handler(int sig);


extern pid_t children[];  // Déclaration externe de children

#endif /* __SIGNAL_HANDLER_Server_H__ */