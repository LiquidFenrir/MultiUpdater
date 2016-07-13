#include <jansson.h>
#include <malloc.h>
#include "config.h"

#define ENTRIES_STRING "entries"

#define NAME_STRING "name"
#define URL_STRING "url"
#define PATH_STRING "path"
#define ZIP_PATH_STRING "inzip"

void parse_entries(json_t * entries_elem, config * todo_config)
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
						else if(!strcmp(key, ZIP_PATH_STRING))
						{
							todo_config->entries[i].zip_path = strdup(json_string_value(value));
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

void get_config(const char * filepath, config * parsed_config)
{
	
	parsed_config->entries_number = 1;
	parsed_config->errorState = 0;
	
	FILE * fptr = fopen(filepath, "rt");
	if (fptr == NULL) {
		parsed_config->errorState = ERROR_FILE;
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