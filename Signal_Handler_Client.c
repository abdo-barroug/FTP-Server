#include "Signal_Handler_Client.h"

// Gestionnaire du signal SIGPIPE.
void sigpipe_handler(int sig) {
    fprintf(stderr, "\nSIGPIPE reçu : connexion interrompue.\n");
    exit(1);
}