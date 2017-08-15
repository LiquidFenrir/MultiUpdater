#include "cia.h"

Result deletePrevious(u64 titleid, FS_MediaType media)
{
	Result ret = 0;
	
	u32 titles_amount = 0;
	ret = AM_GetTitleCount(media, &titles_amount);
	if (ret) {
		printf_log("Error in:\nAM_GetTitleCount\n");
		return ret;
	}
	
	u32 read_titles = 0;
	u64 * titleIDs = malloc(titles_amount * sizeof(u64));
	ret = AM_GetTitleList(&read_titles, media, titles_amount, titleIDs);
	if (ret) {
		printf_log("Error in:\nAM_GetTitleList\n");
		return ret;
	}
	
	for (unsigned int i = 0; i < read_titles; i++) {
		if (titleIDs[i] == titleid) {
			ret = AM_DeleteAppTitle(media, titleid);
			break;
		}
	}
	
	free(titleIDs);
	if (ret) {
		printf_log("Error in:\nAM_DeleteAppTitle\n");
		return ret;
	}
	
	return 0;
}

Result installCia(const char * ciaPath)
{
	u64 size = 0;
	u32 bytes;
	Handle ciaHandle;
	Handle fileHandle;
	AM_TitleEntry info;
	Result ret = 0;
	
	FS_Archive sdmcArchive = (FS_Archive){ARCHIVE_SDMC};
	FS_Path emptyPath = (FS_Path){PATH_EMPTY, 1, (u8*)""};
	FS_Path filePath = fsMakePath(PATH_ASCII, ciaPath);
	FS_MediaType media = MEDIATYPE_SD;
	
	ret = FSUSER_OpenFileDirectly(&fileHandle, sdmcArchive, emptyPath, filePath, FS_OPEN_READ, 0x00000000);
	if (ret) {
		printf_log("Error in:\nFSUSER_OpenFileDirectly\n");
		return ret;
	}
	
	ret = AM_GetCiaFileInfo(media, &info, fileHandle);
	if (ret) {
		printf_log("Error in:\nAM_GetCiaFileInfo\n");
		return ret;
	}
	
	ret = deletePrevious(info.titleID, media);
	if (ret) return ret;
	
	ret = FSFILE_GetSize(fileHandle, &size);
	if (ret) {
		printf_log("Error in:\nFSFILE_GetSize\n");
		return ret;
	}
	ret = AM_StartCiaInstall(media, &ciaHandle);
	if (ret) {
		printf_log("Error in:\nAM_StartCiaInstall\n");
		return ret;
	}
	
	u32 toRead = 0x1000;
	u8 * cia_buffer = malloc(toRead);
	for (u64 startSize = size; size != 0; size -= toRead) {
		if (size < toRead) toRead = size;
		FSFILE_Read(fileHandle, &bytes, startSize-size, cia_buffer, toRead);
		FSFILE_Write(ciaHandle, &bytes, startSize-size, cia_buffer, toRead, 0);
	}
	free(cia_buffer);
	
	ret = AM_FinishCiaInstall(ciaHandle);
	if (ret) {
		printf_log("Error in:\nAM_FinishCiaInstall\n");
		return ret;
	}
	ret = svcCloseHandle(fileHandle);
	if (ret) {
		printf_log("Error in:\nsvcCloseHandle\n");
		return ret;
	}
	
	return 0;
}
