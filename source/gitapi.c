#include "gitapi.h"

int matchPattern(const char * pattern, const char * str) {
	int p_offset = 0, s_offset = 0;
	char current_p_char = '\0', current_s_char = '\0', next_p_char = '\0';

	while (str[s_offset] != '\0') {
		current_p_char = pattern[p_offset];
		current_s_char = str[s_offset];
		if (current_p_char == '*' && next_p_char == '\0') {
			next_p_char = pattern[p_offset+1];
		}
		if (next_p_char != '\0') {
			if (current_s_char != next_p_char) {
				s_offset++;
				continue;
			}
			else {
				current_p_char = next_p_char;
				p_offset++;
				next_p_char = '\0';
			}
		}
		if (current_p_char != current_s_char) return 1;
		
		s_offset++;
		p_offset++;
	}
	return (next_p_char != '\0');
}

char * findTagValue(const char * apiresponse, const char * tagname) {
	char * endstring = "\"";
	char *tagstart, *tagend, *retstr = NULL;
	
	if ((tagstart = strstr(apiresponse, tagname)) != NULL) {
		if ((tagend = strstr(tagstart+strlen(tagname), endstring)) != NULL) {
			tagstart += strlen(tagname);
			int len = tagend-tagstart;
			char * tempstr = calloc(len+1, sizeof(char));
			strncpy(tempstr, tagstart, len);
			retstr = strdup(tempstr);
			free(tempstr);
		}
	}
	
	return retstr;
}

void getAssetUrl(const char * apiresponse, const char * element, char ** asseturl)
{
	char * assets_tagname = "\"assets\":";
	char * name_tagname = "\"name\":\"";
	char * url_tagname = "\"browser_download_url\":\"";
	int offset = strstr(apiresponse, assets_tagname)-apiresponse;
	
	char * foundpos = NULL;
	while ((foundpos = strstr(apiresponse+offset, name_tagname)) != NULL) {
		offset = (int)(foundpos+strlen(name_tagname)-apiresponse);
		char * name = findTagValue(foundpos, name_tagname);
		if (!matchPattern(element, name)) {
			printf("Found asset with name matching %s\n", element);
			printf("Finding asset url...\n");
			*asseturl = findTagValue(foundpos, url_tagname);
			free(name);
			return;
		}
		free(name);
	}
	
	printf("No asset with name matching %s found.\n", element);
}