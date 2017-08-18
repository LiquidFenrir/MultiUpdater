#include "download.hpp"

#include "certs/digicert.h"
#include "certs/cybertrust.h"

static Result setupContext(httpcContext * context, const char * url, u32 * size, bool gitapi)
{
	Result ret = 0;
	u32 statuscode = 0;
	
	ret = httpcOpenContext(context, HTTPC_METHOD_GET, url, 1);
	if (R_FAILED(ret)) {
		printf("Error in:\nhttpcOpenContext\n");
		httpcCloseContext(context);
		return ret;
	}
	
	ret = httpcAddRequestHeaderField(context, "User-Agent", "MultiUpdater");
	if (R_FAILED(ret)) {
		printf("Error in:\nhttpcAddRequestHeaderField\n");
		httpcCloseContext(context);
		return ret;
	}
	
	if (gitapi) {
		ret = httpcAddTrustedRootCA(context, cybertrust_cer, cybertrust_cer_len);
		if (R_FAILED(ret)) {
			printf("Error in:\nhttpcAddRequestHeaderField\n");
			httpcCloseContext(context);
			return ret;
		}
		
		ret = httpcAddTrustedRootCA(context, digicert_cer, digicert_cer_len);
		if (R_FAILED(ret)) {
			printf("Error in:\nhttpcAddRequestHeaderField\n");
			httpcCloseContext(context);
			return ret;
		}
	}
	else {
		ret = httpcSetSSLOpt(context, SSLCOPT_DisableVerify);
		if (R_FAILED(ret)) {
			printf("Error in:\nhttpcSetSSLOpt\n");
			httpcCloseContext(context);
			return ret;
		}
	}
	
	ret = httpcAddRequestHeaderField(context, "Connection", "Keep-Alive");
	if (R_FAILED(ret)) {
		printf("Error in:\nhttpcAddRequestHeaderField\n");
		httpcCloseContext(context);
		return ret;
	}
	
	ret = httpcBeginRequest(context);
	if (R_FAILED(ret)) {
		printf("Error in:\nhttpcBeginRequest\n");
		httpcCloseContext(context);
		return ret;
	}
	
	ret = httpcGetResponseStatusCode(context, &statuscode);
	if (R_FAILED(ret)) {
		printf("Error in:\nhttpcGetResponseStatusCode\n");
		httpcCloseContext(context);
		return ret;
	}
	
	if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
		char* newurl = (char*)malloc(0x1000); // One 4K page for new URL
		if (newurl == NULL) {
			httpcCloseContext(context);
			return DL_ERROR_ALLOC;
		}
		
		ret = httpcGetResponseHeader(context, "Location", newurl, 0x1000);
		if (R_FAILED(ret)) {
			printf("Error in:\nhttpcGetResponseHeader\n");
			httpcCloseContext(context);
			free(newurl);
			return ret;
		}
		
		httpcCloseContext(context); // Close this context before we try the next
		
		if (gitapi)
			printf("Redirecting...\n");
		else
			printf("Redirecting to url:\n%s\n", newurl);
		
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
	if (R_FAILED(ret)) {
		printf("Error in:\nhttpcGetDownloadSizeState\n");
		httpcCloseContext(context);
		return ret;
	}
	
	return 0;
}

Result downloadToFile(std::string url, std::string path, bool gitapi)
{
	printf("Downloading from:\n%s\nto:\n%s\n", url.c_str(), path.c_str());
	
	httpcContext context;
	Result ret = 0;
	u32 contentsize = 0, readsize = 0;
	
	ret = setupContext(&context, url.c_str(), &contentsize, gitapi);
	if (ret != 0)
		return ret;
	
	printf("Downloading %lu bytes...\n", contentsize);
	
	Handle fileHandle;
	u64 offset = 0;
	u32 bytesWritten = 0;
	
	ret = openFile(&fileHandle, path.c_str(), true);
	if (R_FAILED(ret)) {
		printf("Error: couldn't open file to write.\n");
		httpcCloseContext(&context);
		return DL_ERROR_WRITEFILE;
	}
	
	u8* buf = (u8*)malloc(0x1000);
	if (buf == NULL) {
		httpcCloseContext(&context);
		return DL_ERROR_ALLOC;
	}
	
	u64 startTime = osGetTime();
	do {
		ret = httpcDownloadData(&context, buf, 0x1000, &readsize);
		FSFILE_Write(fileHandle, &bytesWritten, offset, buf, readsize, 0);
		offset += readsize;
	} while (ret == (Result)HTTPC_RESULTCODE_DOWNLOADPENDING);
	u64 endTime = osGetTime();
	u64 totalTime = endTime - startTime;
	printf("Download took %llu milliseconds.\n", totalTime);
	
	free(buf);
	FSFILE_Close(fileHandle);
	httpcCloseContext(&context);
	
	if (R_FAILED(ret)) {
		printf("Error in:\nhttpcDownloadData\n");
		return ret;
	}
	
	return 0;
}

Result downloadFromRelease(std::string url, std::string asset, std::string path)
{	
	std::regex parseUrl("github\\.com\\/(.+)\\/(.+)");
	std::smatch result;
	regex_search(url, result, parseUrl);
	
	std::string repoOwner = result[1].str(), repoName = result[2].str();
	
	std::stringstream apiurlStream;
	apiurlStream << "https://api.github.com/repos/" << repoOwner << "/" << repoName << "/releases/latest";
	std::string apiurl = apiurlStream.str();
	
	printf("Downloading latest release from repo:\n%s\nby:\n%s\n", repoName.c_str(), repoOwner.c_str());
	printf("Crafted API url:\n%s\n", apiurl.c_str());
	
	httpcContext context;
	Result ret = 0;
	u32 contentsize = 0, readsize = 0;
	
	ret = setupContext(&context, apiurl.c_str(), &contentsize, true);
	if (ret != 0)
		return ret;
	
	char * buf = (char*)malloc(contentsize+1);
	if (buf == NULL) {
		httpcCloseContext(&context);
		return DL_ERROR_ALLOC;
	}
	buf[contentsize] = 0; //nullbyte to end it as a proper C style string
	
	do {
		ret = httpcDownloadData(&context, (u8 *)buf, contentsize, &readsize);
	} while (ret == (Result)HTTPC_RESULTCODE_DOWNLOADPENDING);
	
	httpcCloseContext(&context);
	if (R_FAILED(ret)) {
		printf("Error in:\nhttpcDownloadData\n");
		free(buf);
		return ret;
	}
	
	// if (asseturl == NULL)
		// ret = DL_ERROR_GIT;
	// else {
		// ret = downloadToFile(asseturl, filepath, true);
		// free(asseturl);
	// }
	
	return ret;
}
