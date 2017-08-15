#include "download.h"
#include "draw.h"
#include "file.h"
#include "cia.h"
#include "stringutils.h"
#include "params.h"

u8 update(entry_t entry)
{
	Result ret = 0;
	printf_log("\x1b[40;34mBeginning update...\x1b[0m\n");
	
	char dl_path[256] = {0};
	
	//if the file to download isnt an archive, direcly download where wanted
	if (entry.in_archive == NULL)
		strcpy(dl_path, entry.path);
	//otherwise, download to an archive in the working dir, then extract where wanted
	else {
		sprintf(dl_path, "%s%s.archive", WORKING_DIR, entry.name);
		cleanPath(dl_path);
	}
	
	//if the entry doesnt want anything from a release, expect it to be a normal file
	if (entry.in_release == NULL)
		ret = downloadToFile(entry.url, dl_path, false);
	else
		ret = downloadFromRelease(entry.url, entry.in_release, dl_path);
	
	if (ret != 0) {
		printf_log("\x1b[40;31mDownload failed!");
		goto failure;
	}
	else printf_log("\x1b[40;32mDownload successful!");
	
	if (entry.in_archive != NULL) {
		printf_log("\n\x1b[40;34mExtracting file from the archive...\x1b[0m\n");
		ret = extractFileFromArchive(dl_path, entry.in_archive, entry.path);
		if (ret != 0) {
			printf_log("\x1b[40;31mExtraction failed!");
			goto failure;
		}
		else printf_log("\x1b[40;32mExtraction successful!");
	}
	
	//if the extracted/downloaded file ends with ".cia", try to install it
	if (strncmp(entry.path+strlen(entry.path)-4, ".cia", 4) == 0) {
		printf_log("\n\x1b[40;34mInstalling CIA...\x1b[0m\n");
		ret = installCia(entry.path);
		if (ret != 0) {
			printf_log("\x1b[40;31mInstall failed!");
			goto failure;
		}
		else
			printf_log("\x1b[40;32mInstall successful!");
	}
	
	printf_log("\n\x1b[40;32mUpdate complete!");
	return UPDATE_DONE;
	
	failure:
	printf_log("\nError: 0x%08x", (unsigned int)ret);
	return UPDATE_ERROR;
}

int main()
{
	gfxInitDefault();
	httpcInit(0);
	amInit();
	fsInit();
	AM_InitializeExternalTitleDatabase(false);
	
	PrintConsole topScreen, bottomScreen;
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);
	
	asprintf(&log_file_path, "%s%s", WORKING_DIR, LOGFILE_NAME);
	Handle fileHandle;
	openFile(log_file_path, &fileHandle, true);
	closeFile(fileHandle);
	
	consoleSelect(&topScreen);
	printf_log("\x1b[2;2HMultiUpdater %s by LiquidFenrir", VERSION_STRING);
	printf("\x1b[27;2HPress SELECT to show instructions.");
	printf("\x1b[28;2HPress START to exit.\x1b[0;0H");
	
	config_t parsed_config;
	if (handleParams(&parsed_config.entries[0])) {
		consoleSelect(&bottomScreen);
		printf_log("\x1b[40;33mUpdating requested application:\n%s\n\x1b[0m", parsed_config.entries[0].name);
		update(parsed_config.entries[0]);
		printf_log("\n\x1b[40;33mClosing in 10 seconds.\nTake note of error codes if needed...\n\x1b[0m");
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
		
		svcSleepThread(10e9);
		parsed_config.entries_number = 1;
		goto exit;
	}

	char * filepath = NULL;
	asprintf(&filepath, "%sconfig.json", WORKING_DIR);

	get_config(filepath, &parsed_config);

	if (parsed_config.errorState == 0) {
		
		int selected_entry = 0;
		u8 state[256] = {0};
		
		while (aptMainLoop()) {

			consoleSelect(&topScreen);
			drawMenu(&parsed_config, state, selected_entry);
			consoleSelect(&bottomScreen);
			
			hidScanInput();
			
			if (hidKeysDown() & KEY_START) {
				exit:
				//strings copied with strdup need to be freed
				clean_config(&parsed_config);
				break;
			}
			else if (hidKeysDown() & KEY_SELECT) {
				drawInstructions();
			}
			else if (hidKeysDown() & KEY_DOWN) {
				selected_entry++;
				if (selected_entry >= parsed_config.entries_number)
					selected_entry = parsed_config.entries_number-1;
			}
			else if (hidKeysDown() & KEY_UP) {
				selected_entry--;
				if (selected_entry < 0)
					selected_entry = 0;
			}
			else if (hidKeysDown() & KEY_RIGHT) { //go to bottom of the menu
				selected_entry = parsed_config.entries_number-1;
			}
			else if (hidKeysDown() & KEY_LEFT) { //go to top of the menu
				selected_entry = 0;
			}
			else if (hidKeysDown() & KEY_L) { //mark all entries
				for (u8 i = 0; i < parsed_config.entries_number; i++) {
					state[i] |= STATE_MARKED;
				}
			}
			else if (hidKeysDown() & KEY_R) { //unmark all entries
				for (u8 i = 0; i < parsed_config.entries_number; i++) {
					state[i] &= ~STATE_MARKED;
				}
			}
			else if (hidKeysDown() & KEY_Y) { //mark/unmark selected entry
				state[selected_entry] ^= STATE_MARKED;
			}
			else if (hidKeysDown() & KEY_A) { //update all marked entries and currently selected entry (even if it's not marked)
				int selected_entry_bak = selected_entry;
				for (int i = 0; i < parsed_config.entries_number; i++) {
					if (i == selected_entry || state[i] & STATE_MARKED) {
						consoleSelect(&topScreen);
						drawMenu(&parsed_config, state, i);
						consoleSelect(&bottomScreen);
						
						u8 ret = update(parsed_config.entries[i]);
						printf_log("\x1b[0m\n");
						state[i] |= ret;
						state[i] &= ~STATE_MARKED;
						
						gfxFlushBuffers();
						gfxSwapBuffers();
						gspWaitForVBlank();
					}
				}
				selected_entry = selected_entry_bak;
			}
			else if (hidKeysDown() & KEY_B) { //backup all marked entries and currently selected entry (even if it's not marked)
				int selected_entry_bak = selected_entry;
				for (int i = 0; i < parsed_config.entries_number; i++) {
					if (i == selected_entry || state[i] & STATE_MARKED) {
						consoleSelect(&topScreen);
						drawMenu(&parsed_config, state, i);
						consoleSelect(&bottomScreen);
						
						char * backuppath = NULL;
						asprintf(&backuppath, "%s%s.bak", WORKING_DIR, parsed_config.entries[i].name);
						cleanPath(backuppath);
						printf_log("\x1b[40;32mBacking up %s...\x1b[0m\n", parsed_config.entries[i].name);
						
						Result ret = copyFile(parsed_config.entries[i].path, backuppath);
						if (ret != 0) {
							printf_log("\x1b[40;31mBackup failed...");
						}
						else {
							printf_log("\x1b[40;32mBackup complete!");
						}
						printf_log("\x1b[0m\n");
						
						free(backuppath);
						
						gfxFlushBuffers();
						gfxSwapBuffers();
						gspWaitForVBlank();
					}
				}
				selected_entry = selected_entry_bak;
			}
			else if (hidKeysDown() & KEY_X) { //restore the backups of all marked entries and currently selected entry (even if it's not marked)
				int selected_entry_bak = selected_entry;
				for (int i = 0; i < parsed_config.entries_number; i++) {
					if (i == selected_entry || state[i] & STATE_MARKED) {
						consoleSelect(&topScreen);
						drawMenu(&parsed_config, state, i);
						consoleSelect(&bottomScreen);
						
						char * backuppath = NULL;
						asprintf(&backuppath, "%s%s.bak", WORKING_DIR, parsed_config.entries[i].name);
						cleanPath(backuppath);
						printf_log("\x1b[40;Restoring %s...\x1b[0m\n", parsed_config.entries[i].name);
						
						Result ret = copyFile(backuppath, parsed_config.entries[i].path);
						if (ret != 0) {
							printf_log("\x1b[40;31mRestore failed...");
						}
						else {
							printf_log("\x1b[40;32mRestore complete!");
							remove(backuppath);
						}
						printf_log("\x1b[0m\n");
						
						free(backuppath);
						
						gfxFlushBuffers();
						gfxSwapBuffers();
						gspWaitForVBlank();
					}
				}
				selected_entry = selected_entry_bak;
			}
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			gspWaitForVBlank();
		}
	}
	else if (parsed_config.errorState == ERROR_FILE) {
		
		printf_log("\x1b[40;31m\x1b[13;2HError: config file not found.\x1b[0m");
		printf("\x1b[14;2HPress A to download the example config.json");
		consoleSelect(&bottomScreen);
		
		while (aptMainLoop()) {
			
			hidScanInput();
			if (hidKeysDown() & KEY_START) break;
			else if (hidKeysDown() & KEY_A) {
				printf_log("\x1b[40;34mDownloading example config.json...\x1b[0m\n");
				Result ret = downloadToFile("https://raw.githubusercontent.com/LiquidFenrir/MultiUpdater/master/config.json" , filepath, true);
				if (ret != 0) printf_log("\x1b[40;31mDownload failed!\nError: 0x%08x\x1b[0m\n", (unsigned int)ret);
				else printf_log("\x1b[40;32mDownload successful!\x1b[0m\nYou can now restart the application and enjoy the multiple functions.\n");
			}
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			gspWaitForVBlank();
		}
	}
	else {
		printf_log("\x1b[40;31m\x1b[13;2HError: invalid config.json.\x1b[0m");
		while (aptMainLoop()) {
			
			hidScanInput();
			if (hidKeysDown() & KEY_START) break;
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			
			gspWaitForVBlank();
		}
	}
	
	free(log_file_path);
	
	fsExit();
	amExit();
	httpcExit();
	gfxExit();
	return 0;
}
