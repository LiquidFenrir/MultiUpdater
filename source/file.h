#pragma once

#include "basic.h"

#define EXTRACTION_ERROR_CONFIG -1
#define EXTRACTION_ERROR_WRITEFILE -2
#define EXTRACTION_ERROR_ARCHIVE_OPEN -3
#define EXTRACTION_ERROR_FIND -4
#define EXTRACTION_ERROR_INFO -5
#define EXTRACTION_ERROR_ALLOC -6
#define EXTRACTION_ERROR_OPEN_IN_ARCHIVE -7
#define EXTRACTION_ERROR_READ_IN_ARCHIVE -8

Result copyFile(const char * srcpath, const char * destpath);
Result extractFileFromArchive(const char * archive_path, const char * filename, const char * filepath);
