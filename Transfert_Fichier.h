#ifndef __TRANSFERT_FICHIER_H__
#define __TRANSFERT_FICHIER_H__

#include "FTP_Structures.h"

void send_file(int connfd, char *filename, int offset);

#endif
