#ifndef __SIGNAL_HANDLER_Client_H__
#define __SIGNAL_HANDLER_Client_H__

#include "FTP_Structures.h"

/**
 * Gestionnaire du signal SIGPIPE côté client.
 *
 * Cette fonction traite le signal SIGPIPE, qui se produit lorsqu'une écriture
 * est tentée sur un socket dont la connexion a été interrompue par le serveur.
 * À la réception de ce signal, un message d'error est affiché et le programme
 * client se termine proprement.
 *
 */
void sigpipe_handler(int sig);


#endif