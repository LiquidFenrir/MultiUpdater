#include "extract.hpp"
#include "stringutils.hpp"

#include "minizip/unzip.h"

#include "7z/7z.h"
#include "7z/7zAlloc.h"
#include "7z/7zCrc.h"
#include "7z/7zMemInStream.h"

Result extractArchive(std::string archivePath, std::string wantedFile, std::string outputPath)
{
	return -1;
}