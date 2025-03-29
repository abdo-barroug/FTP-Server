#include "FTP_Log.h"

/* Fonction utilitaire pour obtenir l'offset enregistré dans un fichier log associé au fichier téléchargé.
   Si le fichier log n'existe pas, on renvoie 0. */
int get_offset_from_log(const char *filename) {
    char log_filename[MAX_NAME_LEN + 10];

    /* Construction du nom du fichier log */
    snprintf(log_filename, sizeof(log_filename), "%s.log", filename);

    /* Ouverture du fichier log en lecture */
    FILE *f = fopen(log_filename, "r");
    int offset = 0;

    if (f != NULL) {
        /* Lecture de l'offset enregistré */
        fscanf(f, "%d", &offset);
        fclose(f);
    }
    //printf("DEBUG: get_offset_from_log(%s) = %d\n", filename, offset);
    return offset;
}


/* Met à jour le fichier log avec l'offset actuel du transfert */
void update_log(const char *filename, int offset) {
    char log_filename[MAX_NAME_LEN + 10];

    /* Construction du nom du fichier log */
    snprintf(log_filename, sizeof(log_filename), "%s.log", filename);

    FILE *f = fopen(log_filename, "w");

    if (f != NULL) {/* Écriture de la nouvelle position */
        fprintf(f, "%d", offset);
        fclose(f);
    } else {
        fprintf(stderr, "Erreur d'ouverture de %s en écriture\n", log_filename);
    }
    //printf("DEBUG: update_log(%s) = %d\n", filename, offset);
    
    /* Vérification de l'intégrité des données écrites */
    f = fopen(log_filename, "r");
    if (f != NULL) {
        int temp = 0;
        fscanf(f, "%d", &temp);
        fclose(f);
        //printf("DEBUG: relu dans log = %d\n", temp);
    }
}



