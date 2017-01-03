#include "download.h"

Result downloadToFile(const char * url, const char * filepath)
{

	if ( url == NULL )
	{
		printf("Download cannot start, the URL in config.json is blank.");
		return 1;
	}

	if ( filepath == NULL )
	{
		printf("Download cannot start, file path in config.json is blank.");
		return 1;
	}

	printf("Downloading file from:\n%s\nto:\n%s\n", url, filepath);

	httpcContext context;
	Result ret = 0;
	u32 statuscode = 0;
	u32 contentsize = 0;
	u8 *buf;

	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 0);
	if (ret != 0)
	{
		printf("Error in:\nhttpcOpenContext:\nreturn: %lx\n", ret);
		return ret;
	}

	ret = httpcAddRequestHeaderField(&context, "User-Agent", "MultiUpdater");
	if (ret != 0)
	{
		printf("Error in:\nhttpcAddRequestHeaderField\nreturn: %lx\n", ret);
		return ret;
	}

	ret = httpcSetSSLOpt(&context, 1<<9);
	if (ret != 0)
	{
		printf("Error in:\nhttpcSetSSLOpt\nreturn: %lx\n", ret);
		return ret;
	}

	ret = httpcBeginRequest(&context);
	if(ret != 0)
	{
		printf("Error in:\nhttpcBeginRequest\nreturn: %lx\n", ret);
		return ret;
	}

	ret = httpcGetResponseStatusCode(&context, &statuscode);
	if (ret != 0)
	{
		printf("Error in:\nhttpcGetResponseStatusCode\nreturn: %lx\n", ret);
		httpcCloseContext(&context);
		return ret;
	}

	if (statuscode != 200)
	{
		if (statuscode >= 300 && statuscode < 400) {
			char newUrl[1024];
			ret = httpcGetResponseHeader(&context, (char*)"Location", newUrl, 1024);
			if (ret != 0)
			{
				printf("Could not get relocation header in 3XX HTTP response.");
				return ret;
			}
			httpcCloseContext(&context);
			ret = downloadToFile(newUrl, filepath);
			return ret;
		}
		else
		{
			printf("Error: Status code not 200 or redirection (3XX).\nStatus code: %lu\n", statuscode);
			httpcCloseContext(&context);
			return -1;
		}
	}

	ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if (ret != 0)
	{
		printf("Error in:\nhttpcGetDownloadSizeState\nreturn: %lx\n", ret);
		httpcCloseContext(&context);
		return ret;
	}

	buf = (u8*)malloc(contentsize);
	if (buf == NULL)
	{
		printf("Failed to malloc buffer.");
		return -2;
	}
	memset(buf, 0, contentsize);

	ret = httpcDownloadData(&context, buf, contentsize, NULL);
	if(ret != 0)
	{
		free(buf);
		printf("Error in:\nhttpcDownloadData\nreturn: %lx\n", ret);
		httpcCloseContext(&context);
		return ret;
	}

	remove(filepath);
	FILE *fptr = fopen(filepath, "wb");
	fwrite(buf, 1, contentsize, fptr);
	fclose(fptr);
	free(buf);
	httpcCloseContext(&context);

	return 0;
}
