#ifndef __FTP_LOG_H__
#define __FTP_LOG_H__

#include "FTP_Structures.h"


int get_offset_from_log(const char *filename);
void update_log(const char *filename, int offset);
#endif /* __FTP_LOG_H__ */
