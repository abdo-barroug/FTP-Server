#ifndef __FTP_SERVICE_H__
#define __FTP_SERVICE_H__

#include "Transfert_Fichier.h"

void ftp_service(int connfd, struct sockaddr_in *clientaddr);

#endif /* __FTP_SERVICE_H__ */
