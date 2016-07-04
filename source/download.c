#include "download.h"
#include <inttypes.h>

Result downloadToFile(const char *url, const char *filepath)
{
	
	printf("downloading file from:\n%s\nto:\n%s\n", url, filepath);
	
	httpcContext context;
	Result ret = 0;
	u32 statuscode = 0;
	u32 contentsize = 0;
	u8 *buf;
	
	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 0);
	if (ret != 0)
	{
		printf("error in:\nhttpcOpenContext:\nreturn: %"PRId32"\n", ret);
		return ret;
	}
	
	ret = httpcBeginRequest(&context);
	if(ret != 0)
	{
		printf("error in:\nhttpcBeginRequest\nreturn: %"PRId32"\n", ret);
		return ret;
	}
	
	ret = httpcGetResponseStatusCode(&context, &statuscode, 0);
	if (ret != 0)
	{
		printf("error in:\nhttpcGetResponseStatusCode\nreturn: %"PRId32"\n", ret);
		httpcCloseContext(&context);
		return ret;
	}
	
	if (statuscode != 200)
	{
		printf("error: status code not 200.\nStatus code: %lu\n", statuscode);
		httpcCloseContext(&context);
		return -2;
	}
	
	ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if (ret != 0)
	{
		printf("error in:\nhttpcGetDownloadSizeState\nreturn: %"PRId32"\n", ret);
		httpcCloseContext(&context);
		return ret;
	}
	
	buf = (u8*)malloc(contentsize);
	if (buf == NULL)
	{
		printf("failure to malloc buffer");
		return -2;
	}
	memset(buf, 0, contentsize);
	
	ret = httpcDownloadData(&context, buf, contentsize, NULL);
	if(ret != 0)
	{
		free(buf);
		printf("error in:\nhttpcDownloadData\nreturn: %"PRId32"\n", ret);
		httpcCloseContext(&context);
		return ret;
	}
	
	remove(filepath);
	FILE *fptr = fopen(filepath, "wb");
	fwrite(buf, 1, contentsize, fptr);
	fclose(fptr);
	free(buf);
	httpcCloseContext(&context);
	
	printf("download went 10/10 captain!\n");
	
	return 0;
}