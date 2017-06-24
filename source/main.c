#include "download.h"
#include "draw.h"
#include "file.h"
#include "cia.h"

u8 update(entry_t entry)
{
	Result ret = 0;
	printf("\x1b[40;34mBeginning update...\x1b[0m\n");
	
	char dl_path[256] = {0};
	
	//if the file to download isnt an archive, direcly download where wanted
	if (entry.in_archive == NULL)
		strcpy(dl_path, entry.path);
	//otherwise, download to an archive in the working dir, then extract where wanted
	else {
		sprintf(dl_path, "%s%s.archive", WORKING_DIR, entry.name);
		for (int i = 0; dl_path[i]; i++) { //replace all spaces and fat32 reserved characters in the path with underscores 
			switch (dl_path[i]) {
				case ' ':
				case '"':
				case '/':
				case '*':
				case ':':
				case '<':
				case '>':
				case '?':
				case '\\':
				case '|':
					dl_path[i] = '_';
				default:
					break;
			}
}
	}
	
	//if the entry doesnt want anything from a release, expect it to be a normal file
	if (entry.in_release == NULL)
		ret = downloadToFile(entry.url, dl_path);
	else
		ret = downloadFromRelease(entry.url, entry.in_release, dl_path);
	
	if (ret != 0) {
		printf("\x1b[40;31mDownload failed!");
		goto failure;
	}
	else printf("\x1b[40;32mDownload successful!");
	
	if (entry.in_archive != NULL) {
		printf("\n\x1b[40;34mExtracting file from the archive...\x1b[0m\n");
		ret = extractFileFromArchive(dl_path, entry.in_archive, entry.path);
		if (ret != 0) {
			printf("\x1b[40;31mExtraction failed!");
			goto failure;
		}
		else printf("\x1b[40;32mExtraction successful!");
	}
	
	//if the extracted/downloaded file ends with ".cia", try to install it
	if (strncmp(entry.path+strlen(entry.path)-4, ".cia", 4) == 0) {
		printf("\n\x1b[40;34mInstalling CIA...\x1b[0m\n");
		ret = installCia(entry.path);
		if (ret != 0) {
			printf("\x1b[40;31mInstall failed!");
			goto failure;
		}
		else
			printf("\x1b[40;32mInstall successful!");
	}
	
	printf("\n\x1b[40;32mUpdate complete!");
	return UPDATE_DONE;
	
	failure:
	printf("\nError: 0x%08x", (unsigned int)ret);
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

	consoleSelect(&topScreen);
	printf("\x1b[2;2HMultiUpdater %s by LiquidFenrir", VERSION_STRING);
	printf("\x1b[27;2HPress SELECT to show instructions.");
	printf("\x1b[28;2HPress START to exit.\x1b[0;0H");

	char filepath[256];
	sprintf(filepath, "%sconfig.json", WORKING_DIR);

	config_t parsed_config;
	get_config(filepath, &parsed_config);

	if (parsed_config.errorState == 0) {
		
		u8 selected_entry = 0;
		u8 state[256] = {0};
		
		while (aptMainLoop()) {

			consoleSelect(&topScreen);
			drawMenu(&parsed_config, state, selected_entry);
			consoleSelect(&bottomScreen);
			
			hidScanInput();
			
			if (hidKeysDown() & KEY_START) {
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
				if (selected_entry >= parsed_config.entries_number)
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
				for (u8 i = 0; i < parsed_config.entries_number; i++) {
					if (i == selected_entry || state[i] & STATE_MARKED) {
						u8 ret = update(parsed_config.entries[i]);
						printf("\x1b[0m\n");
						state[i] |= ret;
						state[i] &= ~STATE_MARKED;
					}
				}
			}
			else if (hidKeysDown() & KEY_B) { //backup all marked entries and currently selected entry (even if it's not marked)
				for (u8 i = 0; i < parsed_config.entries_number; i++) {
					if (i == selected_entry || state[i] & STATE_MARKED) {
						char backuppath[256];
						sprintf(backuppath, "%s%s.bak", WORKING_DIR, parsed_config.entries[i].name);
						for (u8 i = 0; backuppath[i]; i++) { //replace all spaces in the path with underscores 
							if ((u8 )backuppath[i] == 0x20) backuppath[i] = 0x5F;
						}
						printf("\x1b[40;32mBacking up %s...\x1b[0m\n", parsed_config.entries[i].name);
						
						Result ret = copyFile(parsed_config.entries[i].path, backuppath);
						if (ret != 0) {
							printf("\x1b[40;31mBackup failed...");
						}
						else {
							printf("\x1b[40;32mBackup complete!");
						}
						printf("\x1b[0m\n");
					}
				}
			}
			else if (hidKeysDown() & KEY_X) { //restore the backups of all marked entries and currently selected entry (even if it's not marked)
				for (u8 i = 0; i < parsed_config.entries_number; i++) {
					if (i == selected_entry || state[i] & STATE_MARKED) {
						char backuppath[256];
						sprintf(backuppath, "%s%s.bak", WORKING_DIR, parsed_config.entries[i].name);
						for (u8 i = 0; backuppath[i]; i++) { //replace all spaces in the path with underscores 
							if ((u8 )backuppath[i] == 0x20) backuppath[i] = 0x5F;
						}
						printf("\x1b[40;Restoring %s...\x1b[0m\n", parsed_config.entries[i].name);
						
						Result ret = copyFile(backuppath, parsed_config.entries[i].path);
						if (ret != 0) {
							printf("\x1b[40;31mRestore failed...");
						}
						else {
							printf("\x1b[40;32mRestore complete!");
							remove(backuppath);
						}
						printf("\x1b[0m\n");
					}
				}
			}
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			
			gspWaitForVBlank();
		}
	}
	else {
		char * config_errors[] = {
			"There is an error in your config.json.",
			"The config.json could not be found."
		};
		
		printf("\x1b[40;31m\x1b[13;2Herror");
		printf("\x1b[13;2H%s", config_errors[parsed_config.errorState-1]);
		while (aptMainLoop()) {
			
			hidScanInput();
			if (hidKeysDown() & KEY_START) break;
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			
			gspWaitForVBlank();
		}
	}
	
	fsExit();
	amExit();
	httpcExit();
	gfxExit();
	return 0;
}
