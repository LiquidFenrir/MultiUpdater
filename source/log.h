#pragma once

#include "basic.h"

#define LOGFILE_NAME "output_log.txt"

extern char * log_file_path;
void printf_log(const char *format, ...);
