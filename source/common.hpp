#pragma once

#include <3ds.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "file.h"

#ifdef __cplusplus
}

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <regex>

#include "stringutils.hpp"
#include "json.hpp"

using json = nlohmann::json;

#endif

#define WORKING_DIR       "/3ds/"  APP_TITLE

#define CONFIG_FILE_NAME  "config.json"
#define CONFIG_FILE_PATH  WORKING_DIR  "/"  CONFIG_FILE_NAME

#define CONFIG_FILE_URL   "https://raw.githubusercontent.com/LiquidFenrir/MultiUpdater/master/config.json"
