#include <jansson.h>
#include "config.h"

#define ENTRIES_STRING "entries"

#define NAME_STRING "name"
#define URL_STRING "url"
#define PATH_STRING "path"
#define IN_ARCHIVE_STRING "inarchive"
#define IN_RELEASE_STRING "inrelease"

void parse_entries(json_t * entries_elem, config_t * todo_config)
{
	if (json_is_array(entries_elem)) {
		
		size_t entries_number = json_array_size(entries_elem);

		const char *key;
		json_t *value;
		todo_config->entries_number = entries_number;
		
		for (u16 i = 0; i < entries_number; i++) {
			if (json_is_object(json_array_get(entries_elem, i)))
			{
				json_object_foreach(json_array_get(entries_elem, i), key, value) {
					if(json_is_string(value)) {
						if(!strcmp(key, NAME_STRING))
						{
							todo_config->entries[i].name = strdup(json_string_value(value));
						}
						else if(!strcmp(key, URL_STRING))
						{
							todo_config->entries[i].url = strdup(json_string_value(value));
						}
						else if(!strcmp(key, PATH_STRING))
						{
							todo_config->entries[i].path = strdup(json_string_value(value));
						}
						else if(!strcmp(key, IN_ARCHIVE_STRING))
						{
							todo_config->entries[i].in_archive = strdup(json_string_value(value));
						}
						else if(!strcmp(key, IN_RELEASE_STRING))
						{
							todo_config->entries[i].in_release = strdup(json_string_value(value));
						}
					}
				}
			}
			else
			{
				todo_config->errorState = ERROR_JSON;
			}
		}
	}
	else
	{
		todo_config->errorState = ERROR_JSON;
	}
}

json_t * load_json(const char * text) {
	json_t *root;
	json_error_t error;
	
	root = json_loads(text, 0, &error);
	
	if (root) {
		return root;
	} else {
		return (json_t *)0;
	}
}

void get_config(const char * filepath, config_t * parsed_config)
{
	
	parsed_config->entries_number = 1;
	parsed_config->errorState = 0;
	
	FILE * fptr = fopen(filepath, "r");
	if (fptr == NULL) {
		parsed_config->errorState = ERROR_FILE;
		return;
	}
	
	fseek(fptr, 0, SEEK_END);
	u32 size = ftell(fptr);
	char * config_contents = malloc(size + 1);
	rewind(fptr);
	fread(config_contents, size, 1, fptr);
	fclose(fptr);
	
	json_t *root = load_json(config_contents);
	if (root == 0) {
		parsed_config->errorState = ERROR_JSON;
	}
	
	const char *key;
	json_t *value;
	
	if (json_is_object(root)) {
		json_object_foreach(root, key, value) {
			if (!strcmp(key, ENTRIES_STRING))
			{
				parse_entries(value, parsed_config);
			}
		}
	}
	else
	{
		parsed_config->errorState = ERROR_JSON;
	}
	
	free(config_contents);
	json_decref(root);
}

void clean_config(config_t * parsed_config)
{
	for (int i = 0; i < parsed_config->entries_number; i++) {
		free((char*)(parsed_config->entries[i].name));
		free((char*)(parsed_config->entries[i].url));
		free((char*)(parsed_config->entries[i].path));
		free((char*)(parsed_config->entries[i].in_archive));
		free((char*)(parsed_config->entries[i].in_release));
	}
}
