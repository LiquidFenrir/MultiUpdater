#pragma once

#include "basic.h"

Result copyFile(const char * srcpath, const char * destpath);
Result extractFileFromZip(const char * in_zip, const char * filename, const char * filepath);
