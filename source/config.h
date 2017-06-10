#pragma once

#include "basic.h"

typedef struct {
	const char * name;
	const char * url;
	const char * path;
	const char * in_zip;
} entry_t;

typedef struct {
	u8 errorState;
	u8 entries_number;
	entry_t entries[256];
} config_t;

void get_config(const char * filepath, config_t * parsed_config);
