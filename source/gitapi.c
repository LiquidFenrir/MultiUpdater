#include "gitapi.h"
#include "stringutils.h"

char * findTagValue(const char * apiresponse, const char * tagname)
{
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
			printf_log("Found asset with name matching %s\n", element);
			printf_log("Finding asset url...\n");
			*asseturl = findTagValue(foundpos, url_tagname);
			free(name);
			return;
		}
		free(name);
	}
	
	printf_log("No asset with name matching %s found.\n", element);
}