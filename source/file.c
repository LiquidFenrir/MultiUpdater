#include "file.h"
#include "stringutils.h"

#include "minizip/unzip.h"

#include "7z/7z.h"
#include "7z/7zAlloc.h"
#include "7z/7zCrc.h"
#include "7z/7zMemInStream.h"

void makeDirs(FS_Archive archive, FS_Path filePath)
{
	Result ret = 0;
	
	char * path = calloc(filePath.size +1, sizeof(char));
	strncpy(path, filePath.data, filePath.size);
	
	ret = FSUSER_OpenArchive(&archive, archive, fsMakePath(PATH_EMPTY, ""));
	if (ret != 0) printf("Error in:\nFSUSER_OpenArchive\nError: 0x%08x\n", (unsigned int)ret);
	
	for (char * slashpos = strchr(path+1, '/'); slashpos != NULL; slashpos = strchr(slashpos+1, '/')) {
		char bak = *(slashpos);
		*(slashpos) = '\0';
		FS_Path dirpath = fsMakePath(PATH_ASCII, path);
		Handle dirHandle;
		ret = FSUSER_OpenDirectory(&dirHandle, archive, dirpath);
		if(ret == 0) FSDIR_Close(dirHandle);
		else {
			printf("Creating dir: %s\n", path);
			ret = FSUSER_CreateDirectory(archive, dirpath, FS_ATTRIBUTE_DIRECTORY);
			if (ret != 0) printf("Error in:\nFSUSER_CreateDirectory\nError: 0x%08x\n", (unsigned int)ret);
		}
		*(slashpos) = bak;
	}
	FSUSER_CloseArchive(archive);
	free(path);
	
}

Result openFile(const char * path, Handle * filehandle, bool write)
{
	FS_Archive archive = (FS_Archive){ARCHIVE_SDMC};
	FS_Path emptyPath = fsMakePath(PATH_EMPTY, "");
	u32 flags = (write ? (FS_OPEN_CREATE | FS_OPEN_WRITE) : FS_OPEN_READ);
	FS_Path filePath = {0};
	int prefixlen = 0;
	Result ret = 0;
	
	if (!strncmp(path, "ctrnand:/", 9)) {
		archive = (FS_Archive){ARCHIVE_NAND_CTR_FS};
		prefixlen = 8;
	}
	else if (!strncmp(path, "twlp:/", 6)) {
		archive = (FS_Archive){ARCHIVE_TWL_PHOTO};
		prefixlen = 5;
	}
	else if (!strncmp(path, "twln:/", 6)) {
		archive = (FS_Archive){ARCHIVE_NAND_TWL_FS};
		prefixlen = 5;
	}
	else if (!strncmp(path, "sdmc:/", 6)) {
		prefixlen = 5;
	}
	else if (*path != '/') {
		//if the path is local (doesnt start with a slash), it needs to be appended to the working dir to be valid
		char * actualPath = malloc(strlen(WORKING_DIR) + strlen(path) + 1);
		sprintf(actualPath, "%s%s", WORKING_DIR, path);
		filePath = fsMakePath(PATH_ASCII, actualPath);
		free(actualPath);
	}
	
	//if the filePath wasnt set above, set it
	if (filePath.size == 0) filePath = fsMakePath(PATH_ASCII, path+prefixlen);
	
	makeDirs(archive, filePath);
	ret = FSUSER_OpenFileDirectly(filehandle, archive, emptyPath, filePath, flags, 0);
	if (ret != 0) printf("Error in:\nFSUSER_OpenFileDirectly\nError: 0x%08x\n", (unsigned int)ret);
	if (write) ret = FSFILE_SetSize(*filehandle, 0); //truncate the file to remove previous contents before writing
	if (ret != 0) printf("Error in:\nFSFILE_SetSize\nError: 0x%08x\n", (unsigned int)ret);
	
	return ret;
}

Result copyFile(const char * srcpath, const char * destpath)
{
	if (srcpath == NULL || destpath == NULL) {
		printf("Can't copy, path is empty.\n");
		return -1;
	}
	
	Handle filehandle;
	Result ret = openFile(srcpath, &filehandle, false);
	if (ret != 0) {
		printf("Can't copy, couldn't open source file.\n");
		return -2;
	}
	
	FILE *destptr = fopen(destpath, "wb");
	if (destptr == NULL) {
		printf("Couldnt open destination file to write.\n");
		return -3;
	}
	
	printf("Copying:\n%s\nto:\n%s\n", srcpath, destpath);
	
	u8 * buf = malloc(0x1000);
	u32 bytesRead = 0;
	u64 offset = 0;
	
	do {
		readFile(filehandle, &bytesRead, offset, buf, 0x1000);
		fwrite(buf, 1, bytesRead, destptr);
		offset += bytesRead;
	} while(bytesRead);
	
	printf("Copied %llu bytes.\n", offset);
	
	closeFile(filehandle);
	fclose(destptr);
	free(buf);
	
	return 0;
}

Result extractFileFrom7z(const char * archive_file, const char * filename, const char * filepath)
{	
	if (filename == NULL) {
		printf("Cannot start, the inarchive in config.json is blank.\n");
		return EXTRACTION_ERROR_CONFIG;
	}
	
	if (filepath == NULL) {
		printf("Cannot start, the path in config.json is blank.\n");
		return EXTRACTION_ERROR_CONFIG;
	}
	
	Result ret = 0;
	
	FILE * archive = fopen(archive_file, "rb");
	if (archive == NULL) {
		printf("Error: couldn't open archive to read.\n");
		return EXTRACTION_ERROR_ARCHIVE_OPEN;
	}
	
	fseek(archive, 0, SEEK_END);
	u32 archiveSize = ftell(archive);
	fseek(archive, 0, SEEK_SET);
	
	u8 * archiveData = malloc(archiveSize);
	fread(archiveData, archiveSize, 1, archive);
	
	CMemInStream memStream;
	CSzArEx db;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	
	MemInStream_Init(&memStream, archiveData, archiveSize);
	
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;
	
	CrcGenerateTable();
	SzArEx_Init(&db);
	
	SRes res = SzArEx_Open(&db, &memStream.s, &allocImp, &allocTempImp);
	if (res != SZ_OK) {
		ret = EXTRACTION_ERROR_ARCHIVE_OPEN;
		goto finish;
	}
	
	for (u32 i = 0; i < db.NumFiles; ++i) {
		// Skip directories
		unsigned isDir = SzArEx_IsDir(&db, i);
		if (isDir) continue;
		
		// Get name
		size_t len;
		len = SzArEx_GetFileNameUtf16(&db, i, NULL);
		// Super long filename? Just skip it..
		if (len >= 256) continue;
		
		u16 name[256] = {0};
		SzArEx_GetFileNameUtf16(&db, i, name);
		
		// Convert name to ASCII (just cut the other bytes)
		char name8[256] = {0};
		for (size_t j = 0; j < len; ++j) {
			name8[j] = name[j] % 0xff;
		}
		
		u8 * buf = NULL;
		UInt32 blockIndex = UINT32_MAX;
		size_t fileBufSize = 0;
		size_t offset = 0;
		size_t fileSize = 0;

		if (!matchPattern(filename, name8)) {
			res = SzArEx_Extract(
						&db,
						&memStream.s,
						i,
						&blockIndex,
						&buf,
						&fileBufSize,
						&offset,
						&fileSize,
						&allocImp,
						&allocTempImp
			);
			if (res != SZ_OK) {
				printf("Error: Couldn't extract file data from archive.\n");
				ret = EXTRACTION_ERROR_READ_IN_ARCHIVE;
				goto finish;
			}
			
			Handle filehandle;
			u32 bytesWritten = 0;
			
			ret = openFile(filepath, &filehandle, true);
			if (ret != 0) {
				printf("Error: couldn't open file to write.\n");
				return EXTRACTION_ERROR_WRITEFILE;
			}
			
			ret = writeFile(filehandle, &bytesWritten, 0, buf+offset, (u32)fileSize);
			closeFile(filehandle);
			
			free(buf);
			
			goto finish;
		}
	}
	
	printf("Couldn't find a file with a name matching %s in the archive.\n", filename);
	ret = EXTRACTION_ERROR_FIND;
	
	finish:
	free(archiveData);
	fclose(archive);
	
	return ret;
}

static int unzipMatchPattern(__attribute__ ((unused)) unzFile file, const char *filename1, const char *filename2)
{
	return matchPattern(filename2, filename1);
}

Result extractFileFromZip(const char * archive_file, const char * filename, const char * filepath)
{	
	if (filename == NULL) {
		printf("Cannot start, the inarchive in config.json is blank.\n");
		return EXTRACTION_ERROR_CONFIG;
	}
	
	if (filepath == NULL) {
		printf("Cannot start, the path in config.json is blank.\n");
		return EXTRACTION_ERROR_CONFIG;
	}
	
	Result ret = 0;
	u8 * buf = NULL;
	Handle filehandle;
	u32 bytesWritten = 0;
	u64 offset = 0;
	
	ret = openFile(filepath, &filehandle, true);
	if (ret != 0) {
		printf("Error: couldn't open file to write.\n");
		return EXTRACTION_ERROR_WRITEFILE;
	}
	
	unzFile uf = unzOpen64(archive_file);
	if (uf == NULL) {
		printf("Couldn't open zip file.\n");
		closeFile(filehandle);
		return EXTRACTION_ERROR_ARCHIVE_OPEN;
	}
	
	unz_file_info payloadInfo = {};
	
	ret = unzLocateFile(uf, filename, &unzipMatchPattern);

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
			printf("Couldn't open file in the archive to read\n");
			ret = EXTRACTION_ERROR_OPEN_IN_ARCHIVE;
			goto finish;
		}
		
		u32 size = payloadInfo.uncompressed_size;
		
		do {
			if (size < toRead) toRead = size;
			ret = unzReadCurrentFile(uf, buf, toRead);
			if (ret < 0) {
				printf("Couldn't read data from the file in the archive\n");
				ret = EXTRACTION_ERROR_READ_IN_ARCHIVE;
				goto finish;
			}
			else {
				ret = writeFile(filehandle, &bytesWritten, offset, buf, toRead);
				offset += toRead;
			}
			size -= toRead;
		} while(size);
	}
	
	finish:
	free(buf);
	unzClose(uf);
	
	closeFile(filehandle);
	
	return ret;
}

Result extractFileFromArchive(const char * archive_path, const char * filename, const char * filepath)
{
	Result ret = EXTRACTION_ERROR_ARCHIVE_OPEN;
	
	unzFile uf = unzOpen64(archive_path);
	if (uf == NULL) {
		printf("Opening as a 7z file...\n");
		//failure to open as a zip -> must be 7z
		ret = extractFileFrom7z(archive_path, filename, filepath);
	}
	else {
		printf("Opening as a zip file...\n");
		unzClose(uf);
		ret = extractFileFromZip(archive_path, filename, filepath);
	}
	
	remove(archive_path);
	return ret;
}
