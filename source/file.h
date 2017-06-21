#pragma once

#include "basic.h"

#define EXTRACTION_ERROR_CONFIG -1
#define EXTRACTION_ERROR_WRITEFILE -2
#define EXTRACTION_ERROR_ZIP_OPEN -3
#define EXTRACTION_ERROR_FIND -4
#define EXTRACTION_ERROR_INFO -5
#define EXTRACTION_ERROR_ALLOC -6
#define EXTRACTION_ERROR_OPEN_INZIP -7
#define EXTRACTION_ERROR_READ_INZIP -8

Result copyFile(const char * srcpath, const char * destpath);
Result extractFileFromZip(const char * in_zip, const char * filename, const char * filepath);
