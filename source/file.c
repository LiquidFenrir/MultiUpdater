#include "file.h"
#include "minizip/unzip.h"

Result copyFile(const char * srcpath, const char * destpath)
{
	if (srcpath == NULL || destpath == NULL) {
		printf("Can't copy, path is empty.\n");
		return -1;
	}
	
	FILE *srcptr = fopen(srcpath, "rb");
	if (srcptr == NULL) {
		printf("Can't copy, source file doesn't exist.\n");
		return -2;
	}
	
	FILE *destptr = fopen(destpath, "wb");
	if (destptr == NULL) {
		printf("Couldnt open destination file to write.\n");
		return -3;
	}
	
	printf("Copying:\n%s\nto:\n%s\n", srcpath, destpath);
	
	fseek(srcptr, 0, SEEK_END);
	u32 size = ftell(srcptr);
	fseek(srcptr, 0, SEEK_SET);
	
	printf("Copying %lu bytes.\n", size);
	
	u32 toRead = 0x1000;
	u8 * buf = malloc(toRead);
	
	do {
		if (size < toRead) toRead = size;
		fread(buf, 1, toRead, srcptr);
		fwrite(buf, 1, toRead, destptr);
		size -= toRead;
	} while(size);
	
	fclose(srcptr);
	fclose(destptr);
	free(buf);
	
	return 0;
}

int extractFile(unzFile uf, const char * filename, const char * filepath)
{
	FILE * fp = NULL;
	void * buf = NULL;
	unz_file_info payloadInfo = {};
	int ret = 0;

	ret = unzLocateFile(uf, filename, NULL);

	if (ret == UNZ_END_OF_LIST_OF_FILE)
	{
		return 1;
	}
	else if (ret == UNZ_OK)
	{
		ret = unzGetCurrentFileInfo(uf, &payloadInfo, NULL, 0, NULL, 0, NULL, 0);
		if (ret != UNZ_OK)
		{
			return 2;
		}

		buf = (u8 *)malloc(payloadInfo.uncompressed_size);
		if (buf == NULL)
		{
			return 3;
		}
		memset(buf, 0, payloadInfo.uncompressed_size);

		ret = unzOpenCurrentFile(uf);
		if (ret != UNZ_OK) {
			return 4;
		}

		fp = fopen(filepath, "wb");

		if (fp != NULL)
		{
			ret = unzReadCurrentFile(uf, buf, payloadInfo.uncompressed_size);
			if (ret > 0)
			{
				if (fwrite(buf, payloadInfo.uncompressed_size, 1, fp) != 1)
				{
					return 5;
				}
			}
			else
			{
				return 6;
			}

			if (fp)
			{
				fclose(fp);
				return 0;
			}
		}
		return 7;
	}
	else
	{
		return 8;
	}
}

Result extractFileFromZip(const char * zip_path, const char * filename, const char * filepath)
{
	unzFile uf = NULL;

	if (zip_path != NULL)
	{
		uf = unzOpen64(zip_path);
	}

	Result ret = 9;

	if (uf != NULL)
	{
		ret = extractFile(uf, filename, filepath);
		unzClose(uf);
	}
	
	remove(zip_path);
	return ret;
}
