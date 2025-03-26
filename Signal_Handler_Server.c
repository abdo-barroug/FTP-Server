#include "Signal_Handler_Server.h"
#include "FTP_Structures.h"


void sigint_handler(int sig) {
    printf("\nSIGINT reçu. Arrêt du serveur et terminaison des processus fils...\n");
    
    for (int i = 0; i < NB_PROC; i++) {
        if (children[i] > 0) {
            Kill(children[i], SIGINT);
        }
    }
    
    while (wait(NULL) > 0);
    exit(0);
}