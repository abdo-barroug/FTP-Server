#ifndef __FTPSERVER_H__
#define __FTPSERVER_H__

#include "config.h"     // Inclusion de la configuration commune
#include "csapp.h"
#include "ftpclient.h"

/* Prototypes */
void send_file(int connfd, char *filename);

#endif /* __FTPSERVER_H__ */
