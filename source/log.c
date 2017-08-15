#include "log.h"
#include <stdarg.h>

char * log_file_path = NULL;
FILE * log_file = NULL;

void open_log_file(void)
{
	log_file = fopen(log_file_path, "a");
}

void close_log_file(void)
{
	fclose(log_file);
}

void printf_log(const char *format, ...)
{
	open_log_file();
	va_list args;
	va_start(args, format);
	vfprintf(log_file, format, args);
	vprintf(format, args);
	va_end(args);
	close_log_file();
}
