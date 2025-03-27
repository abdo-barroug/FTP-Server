#include "Signal_Handler_Server.h"
#include "FTP_Structures.h"

/* Arrête proprement le serveur */
void sigint_handler(int sig) {
    printf("\nSIGINT reçu. Arrêt du serveur et terminaison des processus fils...\n");
    
    for (int i = 0; i < NB_PROC; i++) {
        if (children[i] > 0) {
            Kill(children[i], SIGINT);
        }
    }

    //Attendre la mort de tous les fils avant la mort du père car il doit rester le dernier
    while (wait(NULL) > 0);
    exit(0);// Quitter proprement le processus père
}