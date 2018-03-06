#pragma once

#include "common.hpp"

enum DownloadError {
	DL_ERROR_NONE = 0,
	DL_ERROR_WRITEFILE,
	DL_ERROR_ALLOC,
	DL_ERROR_STATUSCODE,
	DL_ERROR_GIT,
};

Result downloadToFile(std::string url, std::string path);
Result downloadFromRelease(std::string url, std::string asset, std::string path);
