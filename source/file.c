#include "file.h"

Result makeDirs(FS_ArchiveID archiveID, char * path)
{
	Result ret = 0;
	FS_Archive archive;
	
	ret = FSUSER_OpenArchive(&archive, archiveID, fsMakePath(PATH_EMPTY, ""));
	
	for (char * slashpos = strchr(path+1, '/'); slashpos != NULL; slashpos = strchr(slashpos+1, '/')) {
		char bak = *(slashpos);
		*(slashpos) = '\0';
		
		FS_Path dirpath = fsMakePath(PATH_ASCII, path);
		Handle dirHandle;
		
		ret = FSUSER_OpenDirectory(&dirHandle, archive, dirpath);
		if (R_SUCCEEDED(ret))
			FSDIR_Close(dirHandle);
		else
			ret = FSUSER_CreateDirectory(archive, dirpath, FS_ATTRIBUTE_DIRECTORY);

		*(slashpos) = bak;
	}
	
	FSUSER_CloseArchive(archive);
	free(path);
	
	return ret;
}

Result openFile(Handle* fileHandle, const char * path, bool write)
{
	FS_ArchiveID archive = ARCHIVE_SDMC;
	u32 flags = (write ? (FS_OPEN_CREATE | FS_OPEN_WRITE) : FS_OPEN_READ);
	FS_Path filePath = {0};
	unsigned int prefixlen = 0;
	Result ret = 0;
	
	if (!strncmp(path, "ctrnand:/", 9)) {
		archive = ARCHIVE_NAND_CTR_FS;
		prefixlen = 8;
	}
	else if (!strncmp(path, "twlp:/", 6)) {
		archive = ARCHIVE_TWL_PHOTO;
		prefixlen = 5;
	}
	else if (!strncmp(path, "twln:/", 6)) {
		archive = ARCHIVE_NAND_TWL_FS;
		prefixlen = 5;
	}
	else if (!strncmp(path, "sdmc:/", 6)) {
		prefixlen = 5;
	}
	else if (!strncmp(path, "romfs:/", 7)) {
		if (write)
			return -1;
		archive = ARCHIVE_ROMFS;
		prefixlen = 6;
	}
	else if (*path != '/') {
		//if the path is local (doesnt start with a slash), it needs to be appended to the working dir to be valid
		char * actualPath = NULL;
		asprintf(&actualPath, "%s%s", WORKING_DIR, path);
		filePath = fsMakePath(PATH_ASCII, actualPath);
		free(actualPath);
	}
	
	//if the filePath wasnt set above, set it
	if (filePath.size == 0)
		filePath = fsMakePath(PATH_ASCII, path+prefixlen);
	
	ret = makeDirs(archive, strdup(path));
	ret = FSUSER_OpenFileDirectly(fileHandle, archive, fsMakePath(PATH_EMPTY, ""), filePath, flags, 0);
	if (write)
		ret = FSFILE_SetSize(*fileHandle, 0); //truncate the file to remove previous contents before writing
	
	return ret;
}
