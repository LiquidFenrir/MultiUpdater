#include "download.h"
#include "draw.h"
#include "file.h"

u8 update(config parsed_config, u8 selected_entry)
{
	Result ret = 0;
	printf("\x1b[40;32mStarting download... ");

	if (parsed_config.entries[selected_entry].zip_path == NULL) {
		printf("Direct file\x1b[40;37m\n");
		
		ret = downloadToFile(parsed_config.entries[selected_entry].url, parsed_config.entries[selected_entry].path);
		if (ret == 0) {
			printf("\x1b[40;32mDownload complete!\x1b[40;37m\n");
			goto update;
		}
	}
	else {
		printf("ZIP archive\x1b[40;37m\n");

		char filepath[256];
		sprintf(filepath, "%s%s.zip", WORKING_DIR, parsed_config.entries[selected_entry].name);
		for (u8 i = 0; filepath[i]; i++) { //replace all spaces in the path with underscores 
			if ((u8 )filepath[i] == 0x20) filepath[i] = 0x5F;
		}

		ret = downloadToFile(parsed_config.entries[selected_entry].url, filepath);
		if (ret == 0) {
			printf("\x1b[40;32mDownload complete!\x1b[40;37m\n");
			printf("\x1b[40;32mExtracting files from archive...\x1b[40;37m\n");
			ret = extractFileFromZip(filepath, parsed_config.entries[selected_entry].zip_path, parsed_config.entries[selected_entry].path);
			if (ret == 0) {
				printf("\x1b[40;32mExtraction complete!\x1b[40;37m\n");
				goto update;
			}
			else {
				printf("\x1b[40;31mExtraction failed. Retry or check your config.json.\x1b[40;37m\n");
				return UPDATE_ERROR;
			}
		}
	}
	
	if (ret == 7) {
		printf("Update cancelled.\n");
		return 0; //just remove the marking
	}
	
	printf("\x1b[40;31mDownload failed. Retry or check your config.json.\x1b[40;37m\n");
	return UPDATE_ERROR;
	
	update:
		printf("\x1b[40;32mUpdate complete!\x1b[40;37m\n");
		return UPDATE_DONE;
	
}

int main()
{
	gfxInitDefault();
	httpcInit(0);
	fsInit();

	PrintConsole topScreen, bottomScreen;
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);

	consoleSelect(&topScreen);
	printf("\x1b[40;37m\x1b[2;2HMultiUpdater by LiquidFenrir\x1b[0m\x1b[0;0H");
	printf("\x1b[40;37m\x1b[27;2HPress SELECT to show instructions.\x1b[0m\x1b[0;0H");
	printf("\x1b[40;37m\x1b[28;2HPress START to exit.\x1b[0m\x1b[0;0H");

	char filepath[256];
	strcat(filepath, WORKING_DIR);
	strcat(filepath, "config.json");

	config parsed_config;
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
			else if (hidKeysDown() & KEY_A) //update all marked entries and currently selected entry (even if it's not marked)
			{
				for (u8 i = 0; i < parsed_config.entries_number; i++) {
					if (i == selected_entry || state[i] & STATE_MARKED) {
						u8 ret = update(parsed_config, i);
						state[i] |= ret;
						state[i] &= ~STATE_MARKED;
					}
				}
			}
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			
			gspWaitForVBlank();
		}
	}
	else
	{
		char * config_errors[] = {
			"There is an error in your config.json.",
			"The config.json could not be found."
		};
		
		printf("\x1b[40;31m\x1b[13;2Herror\x1b[0m");
		printf("\x1b[40;31m\x1b[13;2H%s\x1b[0m", config_errors[parsed_config.errorState-1]);
		while (aptMainLoop()) {
			
			hidScanInput();
			if (hidKeysDown() & KEY_START) break;
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			
			gspWaitForVBlank();
		}
	}

	fsExit();
	httpcExit();
	gfxExit();
	return 0;
}
