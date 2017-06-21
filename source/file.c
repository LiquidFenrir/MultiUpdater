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

Result extractFileFromZip(const char * in_zip, const char * filename, const char * filepath)
{	
	if (filename == NULL) {
		printf("Cannot start, the inzip in config.json is blank.\n");
		remove(in_zip);
		return EXTRACTION_ERROR_CONFIG;
	}
	
	if (filepath == NULL) {
		printf("Cannot start, the path in config.json is blank.\n");
		remove(in_zip);
		return EXTRACTION_ERROR_CONFIG;
	}
	
	Result ret = 0;
	u8 * buf = NULL;
	FILE * fh = NULL;
	u64 offset = 0;
	
	fh =  fopen(filepath, "wb");
	if (fh == NULL) {
		remove(in_zip);
		printf("Error: couldn't open file to write.\n");
		return EXTRACTION_ERROR_WRITEFILE;
	}
	
	unzFile uf = unzOpen64(in_zip);
	if (uf == NULL) {
		printf("Couldn't open zip file.\n");
		remove(in_zip);
		return EXTRACTION_ERROR_ZIP_OPEN;
	}
	
	unz_file_info payloadInfo = {};
	
	ret = unzLocateFile(uf, filename, NULL);

	if (ret == UNZ_END_OF_LIST_OF_FILE) {
		printf("Couldn't find the wanted file in the zip.\n");
		ret = EXTRACTION_ERROR_FIND;
	}
	else if (ret == UNZ_OK) {
		ret = unzGetCurrentFileInfo(uf, &payloadInfo, NULL, 0, NULL, 0, NULL, 0);
		if (ret != UNZ_OK) {
			printf("Couldn't get the file information.\n");
			ret = EXTRACTION_ERROR_INFO;
			goto finish;
		}
		
		u32 toRead = 0x1000;
		buf = malloc(toRead);
		if (buf == NULL) {
			ret = EXTRACTION_ERROR_ALLOC;
			goto finish;
		}
		
		ret = unzOpenCurrentFile(uf);
		if (ret != UNZ_OK) {
			ret = EXTRACTION_ERROR_OPEN_INZIP;
			goto finish;
		}
		
		u32 size = payloadInfo.uncompressed_size;
		
		do {
			if (size < toRead) toRead = size;
			ret = unzReadCurrentFile(uf, buf, toRead);
			if (ret > 0) {
				fwrite(buf, 1, toRead, fh);
				ret = 0;
			}
			else {
				ret = EXTRACTION_ERROR_READ_INZIP;
				goto finish;
			}
			size -= toRead;
		} while(size);
	}
	
	finish:
	free(buf);
	unzClose(uf);
	remove(in_zip);
	
	elsefclose(fh);
	
	return ret;
}
