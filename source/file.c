#include "file.h"
#include "minizip/unzip.h"

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
		if (ret == 0)
		{
			remove(zip_path);
		}
		unzClose(uf);
	}
	
	char * error_reports[] = {
		"Everything went OK",
		"Could not find the file in the zip",
		"Failed to get file info",
		"Failed to malloc the buffer",
		"Failed to open the file in the zip",
		"Failed to write to the SD",
		"Failed to read the file in the zip",
		"Failed to open the file on the SD",
		"Error in unzLocateFile",
		"Failed to open the zip file"
	};
	
	printf("file extraction from zip:\n%s\n", error_reports[ret]);
	
	return ret;
}