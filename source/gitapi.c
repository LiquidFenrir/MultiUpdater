#include "gitapi.h"
#include <jansson.h>

void getAssetUrl(const char * text, const char * element, char ** asseturl)
{
	json_t *root;
	json_error_t error;
	const char *root_obj_key;
	json_t *root_obj_value;
	
	root = json_loads(text, 0, &error);
	
	json_object_foreach(root, root_obj_key, root_obj_value) {
		if (json_is_array(root_obj_value)) { //first array is always the assets one
			
			const char *assets_obj_key;
			json_t *assets_obj_value;
			json_t *assets_arr_value;
			size_t index;
			
			json_array_foreach(root_obj_value, index, assets_arr_value) {
				json_object_foreach(assets_arr_value, assets_obj_key, assets_obj_value) {
					if (json_is_string(assets_obj_value)) {
						if (!strncmp(assets_obj_key, "name", 4)) {
							if (!strcmp(json_string_value(assets_obj_value), element))
								continue;
							else
								break;
						}
						else if (!strncmp(assets_obj_key, "browser_download_url", 20)) {
							*asseturl = strdup(json_string_value(assets_obj_value));
							json_decref(root);
							return;
						}
					}
				}
			}
			
			break;
		}
	}
	
	json_decref(root);
}
