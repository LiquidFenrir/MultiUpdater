#include "download.h"
#include "gitapi.h"
#include "file.h"

#include "certs/cybertrust.h"
#include "certs/digicert.h"

Result setupContext(httpcContext * context, const char * url, u32 * size, bool gitapi)
{
	Result ret = 0;
	u32 statuscode = 0;
	
	ret = httpcOpenContext(context, HTTPC_METHOD_GET, url, 1);
	if (ret != 0) {
		printf("Error in:\nhttpcOpenContext\n");
		httpcCloseContext(context);
		return ret;
	}
	
	ret = httpcAddRequestHeaderField(context, "User-Agent", "MultiUpdater");
	if (ret != 0) {
		printf("Error in:\nhttpcAddRequestHeaderField\n");
		httpcCloseContext(context);
		return ret;
	}
	
	if (gitapi) {
		ret = httpcAddTrustedRootCA(context, cybertrust_cer, cybertrust_cer_len);
		if (ret != 0) {
			printf("Error in:\nhttpcAddRequestHeaderField\n");
			httpcCloseContext(context);
			return ret;
		}
		
		ret = httpcAddTrustedRootCA(context, digicert_cer, digicert_cer_len);
		if (ret != 0) {
			printf("Error in:\nhttpcAddRequestHeaderField\n");
			httpcCloseContext(context);
			return ret;
		}
	}
	else {
		ret = httpcSetSSLOpt(context, SSLCOPT_DisableVerify);
		if (ret != 0) {
			printf("Error in:\nhttpcSetSSLOpt\n");
			httpcCloseContext(context);
			return ret;
		}
	}
	
	ret = httpcAddRequestHeaderField(context, "Connection", "Keep-Alive");
	if (ret != 0) {
		printf("Error in:\nhttpcAddRequestHeaderField\n");
		httpcCloseContext(context);
		return ret;
	}
	
	ret = httpcBeginRequest(context);
	if (ret != 0) {
		printf("Error in:\nhttpcBeginRequest\n");
		httpcCloseContext(context);
		return ret;
	}
	
	ret = httpcGetResponseStatusCode(context, &statuscode);
	if (ret != 0) {
		printf("Error in:\nhttpcGetResponseStatusCode\n");
		httpcCloseContext(context);
		return ret;
	}
	
	if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
		char * newurl = malloc(0x1000); // One 4K page for new URL
		if (newurl == NULL) {
			httpcCloseContext(context);
			return DL_ERROR_ALLOC;
		}
		
		ret = httpcGetResponseHeader(context, "Location", newurl, 0x1000);
		if (ret != 0) {
			printf("Error in:\nhttpcGetResponseHeader\n");
			httpcCloseContext(context);
			free(newurl);
			return ret;
		}
		
		httpcCloseContext(context); // Close this context before we try the next
		
		if (newurl[0] == '/') { //if the url starts with a slash, it's local
			int slashpos = 0;
			char * domainname = strdup(url);
			if (strncmp("http", domainname, 4) == 0) slashpos = 8; //if the url in the entry starts with http:// or https:// we need to skip that
			slashpos = strchr(domainname+slashpos, '/')-domainname;
			domainname[slashpos] = '\0'; // replace the slash with a nullbyte to cut the url
			char * copyurl = strdup(newurl);
			sprintf(newurl, "%s%s", domainname, copyurl);
			free(copyurl);
			free(domainname);
		}
		
		if (gitapi) printf("Redirecting...\n");
		else printf("Redirecting to url:\n%s\n", newurl);
		
		ret = setupContext(context, newurl, size, gitapi);
		free(newurl);
		return ret;
	}
	
	if (statuscode != 200) {
		printf("Error: HTTP status code is not 200 OK.\nStatus code: %lu\n", statuscode);
		httpcCloseContext(context);
		return DL_ERROR_STATUSCODE;
	}
	
	ret = httpcGetDownloadSizeState(context, NULL, size);
	if (ret != 0) {
		printf("Error in:\nhttpcGetDownloadSizeState\n");
		httpcCloseContext(context);
		return ret;
	}
	
	return 0;
}

Result downloadToFile(const char * url, const char * filepath, bool gitapi)
{
	if (url == NULL) {
		printf("Download cannot start, the URL in config.json is blank.\n");
		return DL_ERROR_CONFIG;
	}
	
	if (filepath == NULL) {
		printf("Download cannot start, file path in config.json is blank.\n");
		return DL_ERROR_CONFIG;
	}
	
	printf("Downloading file from:\n%s\nto:\n%s\n", url, filepath);
	
	httpcContext context;
	Result ret = 0;
	u32 contentsize = 0, readsize = 0;
	
	ret = setupContext(&context, url, &contentsize, gitapi);
	if (ret != 0) return ret;
	
	printf("Downloading %lu bytes...\n", contentsize);
	
	Handle filehandle;
	u64 offset = 0;
	u32 bytesWritten = 0;
	
	ret = openFile(filepath, &filehandle, true);
	if (ret != 0) {
		printf("Error: couldn't open file to write.\n");
		httpcCloseContext(&context);
		return DL_ERROR_WRITEFILE;
	}
	
	u8 * buf = malloc(0x1000);
	if (buf == NULL) {
		httpcCloseContext(&context);
		return DL_ERROR_ALLOC;
	}
	
	u64 startTime = osGetTime();
	do {
		ret = httpcDownloadData(&context, buf, 0x1000, &readsize);
		writeFile(filehandle, &bytesWritten, offset, buf, readsize);
		offset += readsize;
	} while (ret == (Result)HTTPC_RESULTCODE_DOWNLOADPENDING);
	printf("Download took %llu milliseconds.\n", osGetTime()-startTime);
	
	free(buf);
	closeFile(filehandle);
	httpcCloseContext(&context);
	
	if (ret != 0) {
		printf("Error in:\nhttpcDownloadData\n");
		return ret;
	}
	
	return 0;
}

Result downloadFromRelease(const char * url, const char * element, const char * filepath)
{
	if (url == NULL) {
		printf("Download cannot start, the URL in config.json is blank.\n");
		return DL_ERROR_CONFIG;
	}
	
	if (element == NULL) {
		printf("Download cannot start, inrelease in config.json is blank.\n");
		return DL_ERROR_CONFIG;
	}
	
	if (filepath == NULL) {
		printf("Download cannot start, file path in config.json is blank.\n");
		return DL_ERROR_CONFIG;
	}
	
	char * copyurl = strdup(url);
	char * repo_name = copyurl;
	char * repo_owner = copyurl;
	char apiurl[256] = {0};
	
	unsigned int startpos = 0;
	if (strncmp("http", copyurl, 4) == 0) startpos = 8; //if the url in the entry starts with http:// or https:// we need to skip that
	for (unsigned int i = startpos; copyurl[i]; i++) {
		if (copyurl[i] == '/') {
			copyurl[i] = '\0';
			if (repo_owner == copyurl) repo_owner += i+1;
			else if (repo_name == copyurl) repo_name += i+1;
			else break;
		}
	}
	sprintf(apiurl, "https://api.github.com/repos/%s/%s/releases/latest", repo_owner, repo_name);
	printf("Downloading latest release from repo:\n%s\nby:\n%s\n", repo_name, repo_owner);
	printf("Crafted api url:\n%s\n", apiurl);
	free(copyurl);
	
	httpcContext context;
	Result ret = 0;
	u32 contentsize = 0, readsize = 0;
	
	ret = setupContext(&context, apiurl, &contentsize, true);
	if (ret != 0) return ret;
	
	char * buf = malloc(contentsize+1);
	if (buf == NULL) {
		httpcCloseContext(&context);
		return DL_ERROR_ALLOC;
	}
	buf[contentsize] = 0; //nullbyte to end it as a proper C style string
	
	do {
		ret = httpcDownloadData(&context, (u8 *)buf, contentsize, &readsize);
	} while (ret == (Result)HTTPC_RESULTCODE_DOWNLOADPENDING);
	
	httpcCloseContext(&context);
	if (ret != 0) {
		printf("Error in:\nhttpcDownloadData\n");
		free(buf);
		return ret;
	}
	
	char * asseturl = NULL;
	printf("\x1b[40;34mSearching for asset %s in api response...\x1b[0m\n", element);
	getAssetUrl((const char *)buf, element, &asseturl);
	free(buf);
	
	if (asseturl == NULL)
		ret = DL_ERROR_GIT;
	else {
		ret = downloadToFile(asseturl, filepath, true);
		free(asseturl);
	}
	
	return ret;
}
