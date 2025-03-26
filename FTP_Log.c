#include "FTP_Log.h"

/* Fonction utilitaire pour obtenir l'offset enregistré dans un fichier log associé au fichier téléchargé.
   Si le fichier log n'existe pas, on renvoie 0. */
int get_offset_from_log(const char *filename) {
    char log_filename[MAX_NAME_LEN + 10];
    snprintf(log_filename, sizeof(log_filename), "%s.log", filename);
    FILE *f = fopen(log_filename, "r");
    int offset = 0;
    if (f != NULL) {
        fscanf(f, "%d", &offset);
        fclose(f);
    }
    return offset;
}

/* Met à jour le fichier log avec l'offset actuel du transfert */
void update_log(const char *filename, int offset) {
    char log_filename[MAX_NAME_LEN + 10];
    snprintf(log_filename, sizeof(log_filename), "%s.log", filename);
    FILE *f = fopen(log_filename, "w");
    if (f != NULL) {
        fprintf(f, "%d", offset);
        fclose(f);
    }
}