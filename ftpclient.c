#include "ftpclient.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    request_t req;
    response_t resp;
    char command_line[512];
    char cmd[10], filename[MAX_FILENAME_LEN];
    struct timeval start, end;
    double elapsed_time;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_host>\n", argv[0]);
        exit(1);
    }
    host = argv[1];

    //Connexion au serveur FTP
    clientfd = Open_clientfd(host, PORT);
    printf("Connected to %s.\n", host);
    
    while (1) {
        printf("ftp> ");
        if (Fgets(command_line, sizeof(command_line), stdin) == NULL){
            break;
        }
        
        //Extraction de la commande et du paramètre éventuel
        if (sscanf(command_line, "%s %s", cmd, filename) < 1){
            continue;
        }
        
        //Si l'utilisateur tape "bye", on envoie la commande et on quitte
        if (strcasecmp(cmd, "bye") == 0) {
            req.type = BYE;
            memset(req.filename, 0, MAX_FILENAME_LEN);
            Rio_writen(clientfd, &req, sizeof(req));
            break;
        }

        //Si l'utilisateur tape "get <filename>"
        else if (strcasecmp(cmd, "get") == 0) {
            if (sscanf(command_line, "%s %s", cmd, filename) != 2) {
                fprintf(stderr, "Commande invalide. Usage: get <filename>\n");
                continue;
            }
            req.type = GET;
            strncpy(req.filename, filename, MAX_FILENAME_LEN);
            req.filename[MAX_FILENAME_LEN - 1] = '\0';
            Rio_writen(clientfd, &req, sizeof(req));
            
            /* Demarrage du chronomètre pour mesurer le transfert */
            gettimeofday(&start, NULL);
            
            //Lecture de la réponse du serveur
            if (Rio_readn(clientfd, &resp, sizeof(response_t)) != sizeof(response_t)) {
                fprintf(stderr, "Erreur lors de la lecture de la réponse.\n");
                continue;
            }
            
            if (resp.code != SUCCESS) {
                fprintf(stderr, "Erreur serveur : code %d\n", resp.code);
                continue;
            }
            
            //Construction du chemin pour sauvegarder le fichier dans ./client
            char client_file_path[MAX_FILENAME_LEN + 10];
            snprintf(client_file_path, sizeof(client_file_path), "./client/%s", filename);
            int fd = Open(client_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            
            //Reception du fichier par blocs
            int totalBytes = 0;
            char buffer[BLOCK_SIZE];
            while (totalBytes < resp.filesize) {
                int bytesToRead;
                if (resp.filesize - totalBytes < BLOCK_SIZE)
                    bytesToRead = resp.filesize - totalBytes;
                else
                    bytesToRead = BLOCK_SIZE;
                
                int n = Rio_readn(clientfd, buffer, bytesToRead);
                if (n <= 0)
                    break;
                Rio_writen(fd, buffer, n);
                totalBytes += n;
            }
            Close(fd);
            //Le chronometre vient de chatgpt
            gettimeofday(&end, NULL);
            elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);
            
            printf("Transfer successfully complete.\n");
            printf("%d bytes received in %.2f seconds (%.2f Kbytes/s).\n", totalBytes, elapsed_time, (totalBytes / 1024.0) / elapsed_time);
        
        }else {
            fprintf(stderr, "Commande non trouve\n");
        }
    }
    
    Close(clientfd);
    return 0;
}