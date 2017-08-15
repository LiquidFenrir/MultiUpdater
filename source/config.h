#pragma once

#include "basic.h"

typedef struct {
	const char * name;
	const char * url;
	const char * path;
	const char * in_archive;
	const char * in_release;
} entry_t;

typedef struct {
	u8 errorState;
	int entries_number;
	entry_t entries[256];
} config_t;

void get_config(const char * filepath, config_t * parsed_config);
void clean_config(config_t * parsed_config);
