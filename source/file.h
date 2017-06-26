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

Result openFile(const char * path, Handle * filehandle, bool write);

inline Result readFile(Handle filehandle, u32 * bytesRead, u64 offset, void * buffer, u32 size)
{
	return FSFILE_Read(filehandle, bytesRead, offset, buffer, size);
}
inline Result writeFile(Handle filehandle, u32 * bytesWritten, u64 offset, void * buffer, u32 size)
{
	return FSFILE_Write(filehandle, bytesWritten, offset, buffer, size, 0);
}
inline Result closeFile(Handle filehandle)
{
	return svcCloseHandle(filehandle);
}

Result copyFile(const char * srcpath, const char * destpath);
Result extractFileFromArchive(const char * archive_path, const char * filename, const char * filepath);
