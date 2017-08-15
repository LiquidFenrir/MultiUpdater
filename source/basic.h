#pragma once

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <3ds.h>

#define WORKING_DIR "/3ds/MultiUpdater/"

#include "log.h"

#define ERROR_JSON 1
#define ERROR_FILE 2

#define UPDATE_ERROR 1
#define UPDATE_DONE 2
#define STATE_MARKED 4
