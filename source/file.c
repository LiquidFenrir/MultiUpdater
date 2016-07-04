#include "file.h"
#include "unzip.h"

#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#define WRITEBUFFERSIZE (8192)

int do_extract_currentfile(unzFile uf, const char * filepath)
{
	unz_file_info64 file_info = {0};
	FILE* fout = NULL;
	void* buf = NULL;
	uInt size_buf = WRITEBUFFERSIZE;
	int err = UNZ_OK;
	int errclose = UNZ_OK;
	char filename_inzip[256] = {0};

	err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
	if (err != UNZ_OK)
	{
		// printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
		return err;
	}

	buf = (void*)malloc(size_buf);
	if (buf == NULL)
	{
		// printf("Error allocating memory\n");
		return UNZ_INTERNALERROR;
	}
	
	unzOpenCurrentFile(uf);
	fout = fopen(filepath, "wb");

	/* Read from the zip, unzip to buffer, and write to disk */
	if (fout != NULL)
	{
		// printf(" extracting: %s\n", inzip);

		do
		{
			err = unzReadCurrentFile(uf, buf, size_buf);
			if (err < 0)
			{
				// printf("error %d with zipfile in unzReadCurrentFile\n", err);
				break;
			}
			if (err == 0)
			{
				break;
			}
			
			if (fwrite(buf, err, 1, fout) != 1)
			{
				// printf("error %d in writing extracted file\n", errno);
				err = UNZ_ERRNO;
				break;
			}
		}
		while (err > 0);

		if (fout)
		{
			fclose(fout);
		}
	}

	errclose = unzCloseCurrentFile(uf);
	if (errclose != UNZ_OK)
	{
		// printf("error %d with zipfile in unzCloseCurrentFile\n", errclose);
	}

	free(buf);
	return err;
}

int do_extract_onefile(unzFile uf, const char * filename, const char * filepath)
{
	if (unzLocateFile(uf, filename, NULL) != UNZ_OK)
	{
		return 2;
	}
	if (do_extract_currentfile(uf, filepath) == UNZ_OK)
	{
		return 0;
	}
	return 1;
}

void extractFileFromZip(const char * zip_path, const char * filename, const char * filepath)
{
	unzFile uf = NULL;
	if (zip_path != NULL)
	{
		uf = unzOpen64(zip_path);
	}
	
	do_extract_onefile(uf, filename, filepath);
		
	unzClose(uf);
}