#pragma once

#include "basic.h"

Result downloadToBuffer(const char * url, u8 ** buf, u32 * bufsize);
Result downloadToFile(const char * url, const char * filepath);
