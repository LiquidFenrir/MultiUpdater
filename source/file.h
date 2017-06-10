#pragma once

#include "basic.h"

Result copyFile(const char * srcpath, const char * destpath);
Result extractFileFromZip(const char * zip_path, const char * filename, const char * filepath);
