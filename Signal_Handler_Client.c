#include "Signal_Handler_Client.h"

void sigpipe_handler(int sig) {
    fprintf(stderr, "\nSIGPIPE reçu : connexion interrompue.\n");
    exit(1);
}