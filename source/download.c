#include "download.h"

Result downloadToFile(const char *url, const char *filepath)
{
	
	remove(filepath);
	
	httpcContext context;
	Result ret = 0;
	u32 statuscode = 0;
	u32 contentsize = 0;
	u8 *buf;
	
	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
	if (ret != 0) return ret;
	
	ret = httpcGetResponseStatusCode(&context, &statuscode, 0);
	if (ret != 0) return ret;
	
	if (statuscode != 200) return -2;
	
	ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if (ret != 0) return ret;
	
	buf = (u8*)malloc(contentsize);
	
	ret = httpcDownloadData(&context, buf, contentsize, NULL);
	if(ret!=0)
	{
		free(buf);
		return ret;
	}
	
	FILE *fptr = fopen(filepath, "wb");
	fwrite(buf, 1, contentsize, fptr);
	fclose(fptr);
	
	free(buf);
	
	return 0;
}