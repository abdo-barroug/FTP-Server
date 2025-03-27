#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include "FTP_Structures.h"
#include "FTP_Log.h"
#include "Signal_Handler_Client.h"

void send_request(int clientfd, request_t *req, int offset);
int receive_file(int clientfd, request_t *req, int offset);
void process_command(int clientfd);

#endif /* FTP_CLIENT_H */