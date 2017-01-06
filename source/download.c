#include "download.h"

Result downloadToFile(const char * url, const char * filepath)
{

	if ( url == NULL )
	{
		printf("Download cannot start, the URL in config.json is blank.\n");
		return 1;
	}

	if ( filepath == NULL )
	{
		printf("Download cannot start, file path in config.json is blank.\n");
		return 1;
	}

	printf("Downloading file from:\n%s\nto:\n%s\n", url, filepath);

	httpcContext context;
	Result ret = 0;
	char *newurl=NULL;
	u32 statuscode=0;
	u32 contentsize=0, readsize=0, size=0;
	u8 *buf, *lastbuf;

	do {
	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
	ret = httpcAddRequestHeaderField(&context, "User-Agent", "MultiUpdater");
	if (ret != 0) {
		printf("Error in:\nhttpcAddRequestHeaderField\nreturn: %lx\n", ret);
		return ret;
		}

	ret = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
	ret = httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");

	ret = httpcBeginRequest(&context);
	if(ret!=0) {
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		printf("Error in:\nhttpcBeginRequest\nreturn: %lx\n", ret);
		return ret;
	}

	ret = httpcGetResponseStatusCode(&context, &statuscode);
	if(ret!=0) {
		printf("Error in:\nhttpcGetResponseStatusCode\nreturn: %lx\n", ret);
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return ret;
		}

	if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
		if(newurl==NULL) newurl = malloc(0x1000); // One 4K page for new URL
			if (newurl==NULL) {
				httpcCloseContext(&context);
				return -1;
				}
		ret = httpcGetResponseHeader(&context, "Location", newurl, 0x1000);
		url = newurl; // Change pointer to the url that we just learned
		printf("redirecting to url: %s\n",url);
		httpcCloseContext(&context); // Close this context before we try the next
			if (newurl[0] != '/') {
				ret = downloadToFile(newurl, filepath);
				return ret;
				}
			else if (statuscode == CITRA_STATUSCODE) {
				printf("Error: Running in Citra, changing state\n");
				httpcCloseContext(&context);
				return 6;
				}
			}
	} while ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308));

	if(statuscode!=200) {
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return -2;
	}

	ret=httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if(ret!=0) {
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return ret;
		}

	// Start with a single page buffer
	buf = (u8*)malloc(0x1000);
	if(buf==NULL) {
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return -1;
		}

	do {
	// This download loop resizes the buffer as data is read.
	ret = httpcDownloadData(&context, buf+size, 0x1000, &readsize);
	size += readsize;
	if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING) {
			lastbuf = buf; // Save the old pointer, in case realloc() fails.
			buf = realloc(buf, size + 0x1000);
			if(buf==NULL) {
				httpcCloseContext(&context);
				free(lastbuf);
				if(newurl!=NULL) free(newurl);
				return -1;
				}
			}
		} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);

		if(ret!=0) {
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			free(buf);
			return -1;
		}

		// Resize the buffer back down to our actual final size
		lastbuf = buf;
		buf = realloc(buf, size);
		if(buf==NULL) { // realloc() failed.
			httpcCloseContext(&context);
			free(lastbuf);
			if(newurl!=NULL) free(newurl);
			return -1;
		}

		remove(filepath);
		FILE *fptr = fopen(filepath, "wb");
		fwrite(buf, 1, contentsize, fptr);
		fclose(fptr);
		free(buf);
		httpcCloseContext(&context);

		return 0;
}
