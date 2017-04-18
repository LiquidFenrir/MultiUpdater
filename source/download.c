#include "download.h"

Result downloadToBuffer(const char * url, u8 ** buf, u32 * bufsize)
{
	
	httpcContext context;
	Result ret = 0;
	char * newurl = NULL;
	u32 statuscode = 0;
	u32 contentsize = 0, readsize = 0, size = 0;
	u8 * lastbuf = NULL;
	
	do {
		ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
		ret = httpcAddRequestHeaderField(&context, "User-Agent", "MultiUpdater");
		if (ret != 0) {
			httpcCloseContext(&context);
			if (newurl != NULL) free(newurl);
			printf("Error in:\nhttpcAddRequestHeaderField\nreturn: %lx\n", ret);
			return ret;
		}
		
		ret = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
		ret = httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");
		
		ret = httpcBeginRequest(&context);
		if (ret != 0) {
			httpcCloseContext(&context);
			if (newurl != NULL) free(newurl);
			printf("Error in:\nhttpcBeginRequest\nreturn: %lx\n", ret);
			return ret;
		}
		
		ret = httpcGetResponseStatusCode(&context, &statuscode);
		if (ret != 0) {
			printf("Error in:\nhttpcGetResponseStatusCode\nreturn: %lx\n", ret);
			httpcCloseContext(&context);
			if (newurl != NULL) free(newurl);
			return ret;
		}
		
		if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
			if (newurl == NULL) newurl = malloc(0x1000); // One 4K page for new URL
				if (newurl == NULL) {
					httpcCloseContext(&context);
					return -1;
				}
			
			ret = httpcGetResponseHeader(&context, "Location", newurl, 0x1000);
			httpcCloseContext(&context); // Close this context before we try the next
			
			if (newurl[0] == '/') { //if the url starts with a slash, it's local
				int slashpos = 0;
				char * domainname = strdup(url);
				if (strncmp("http", domainname, 4) == 0) slashpos = 7; //if the url in the entry starts with http:// or https:// we need to skip that
				slashpos += (int )strchr(domainname+slashpos, '/');
				domainname[slashpos] = '\0'; // replace the slash with a nullbyte to cut the url
				char * copyurl = strdup(newurl);
				sprintf(newurl, "%s%s", domainname, copyurl);
				free(copyurl);
				free(domainname);
			}
			url = newurl; // Change pointer to the url that we just learned
			printf("Redirecting to url: %s\n", url);
			ret = downloadToBuffer(newurl, buf, bufsize);
			return ret;
		}
	} while ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308));
	
	if (statuscode != 200) {
		printf("Error: HTTP status code is not 200 OK.\nStatus code: %lu\n", statuscode);
		httpcCloseContext(&context);
		if (newurl != NULL) free(newurl);
		return -2;
	}
	
	ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if (ret != 0) {
		httpcCloseContext(&context);
		if (newurl != NULL) free(newurl);
		return ret;
	}
	
	// Start with a single page buffer
	*buf = (u8 *)malloc(0x1000);
	if (buf == NULL) {
		httpcCloseContext(&context);
		if (newurl != NULL) free(newurl);
		return -1;
	}
	
	do {
		// This download loop resizes the buffer as data is read.
		ret = httpcDownloadData(&context, *buf+size, 0x1000, &readsize);
		size += readsize;
		if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING) {
			
			hidScanInput();

			if (hidKeysDown() & KEY_B) {
				if (newurl != NULL) free(newurl);
				if (lastbuf != NULL) free(lastbuf);
				if (buf != NULL) free(buf);
				httpcCloseContext(&context);
				return 7;
			}
			
			lastbuf = *buf; // Save the old pointer, in case realloc() fails.
			*buf = realloc(*buf, size + 0x1000);
			if (buf == NULL) {
				httpcCloseContext(&context);
				free(lastbuf);
				if (newurl != NULL) free(newurl);
				return -1;
			}
		}
	} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);
	
	if(ret != 0) {
		httpcCloseContext(&context);
		if (newurl != NULL) free(newurl);
		free(buf);
		return -1;
	}
	
	// Resize the buffer back down to our actual final size
	lastbuf = *buf;
	*buf = realloc(*buf, size);
	if(buf == NULL) { // realloc() failed.
		httpcCloseContext(&context);
		free(lastbuf);
		if(newurl != NULL) free(newurl);
		return -1;
	}
	
	*bufsize = contentsize;
	httpcCloseContext(&context);
	return 0;
}

Result downloadToFile(const char * url, const char * filepath)
{
	
	if (url == NULL) {
		printf("Download cannot start, the URL in config.json is blank.\n");
		return -1;
	}
	
	if (filepath == NULL) {
		printf("Download cannot start, file path in config.json is blank.\n");
		return -1;
	}
	
	printf("Downloading file from:\n%s\nto:\n%s\n", url, filepath);
	
	Result ret = 0;
	u8 * buf = NULL;
	u32 size = 0;
	
	ret = downloadToBuffer(url, &buf, &size);
	if (ret != 0) return ret;
	
	FILE *fptr = fopen(filepath, "wb");
	if (fptr == NULL) {
		printf("Couldnt open file to write.\n");
		return -1;
	}
	fwrite(buf, 1, size, fptr);
	fclose(fptr);
	free(buf);
	return 0;
}
