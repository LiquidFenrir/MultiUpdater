#pragma once

#include "basic.h"

typedef struct {
	const char * name;
	const char * url;
	const char * path;
	const char * zip_path;
} entry;

typedef struct {
	u8 errorState;
	u8 entries_number;
	entry entries[256];
} config;

void get_config(const char * filepath, config * parsed_config);
