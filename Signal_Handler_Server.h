#ifndef __SIGNAL_HANDLER_Server_H__
#define __SIGNAL_HANDLER_Server_H__

#include "FTP_Structures.h"


void sigint_handler(int sig);

extern pid_t children[];  // Déclaration externe de children

#endif /* __SIGNAL_HANDLER_Server_H__ */