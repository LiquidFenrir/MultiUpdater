#include "config.h"
#include "download.h"
#include "draw.h"
#include "file.h"

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
	printf("\x1b[40;37m\x1b[28;28H Press START to quit.\x1b[0m\x1b[0;0H");

	char filepath[256];
	strcat(filepath, WORKING_DIR);
	strcat(filepath, "config.json");

	config parsed_config;
	get_config(filepath, &parsed_config);

	char * config_errors[] = {
		"There is an error in your config.json.",
		"The config.json could not be found."
	};

	if (parsed_config.errorState == 0)
	{
		u32 selected_entry = 0;

		char * names[256];
		for (u16 i = 0; i < parsed_config.entries_number; i++) {
			names[i] = (char *)parsed_config.entries[i].name;
		}
		u8 state[256] = {0};

		while (aptMainLoop()) {

			consoleSelect(&topScreen);
			drawMenu(names, state, selected_entry);
			consoleSelect(&bottomScreen);

			hidScanInput();

			if (hidKeysDown() & KEY_START)
			{
				break;
			}
			else if (hidKeysDown() & KEY_DOWN)
			{
				selected_entry++;
				if (selected_entry >= parsed_config.entries_number) {
					selected_entry = parsed_config.entries_number-1;
				}
			}
			else if (hidKeysDown() & KEY_UP)
			{
				selected_entry--;
				if (selected_entry >= parsed_config.entries_number) {
					selected_entry = 0;
				}
			}
			else if (hidKeysDown() & KEY_A)
			{
				Result ret = 1;
				printf("\x1b[40;32mStarting download... ");
				if (parsed_config.entries[selected_entry].zip_path == NULL) {
					printf("Direct file\x1b[40;37m\n");
					ret = downloadToFile(parsed_config.entries[selected_entry].url, parsed_config.entries[selected_entry].path);
				}
				else
				{
					printf("ZIP archive\x1b[40;37m\n");
					char filepath[256];
					sprintf(filepath, "%s%s.zip", WORKING_DIR, names[selected_entry]);
					ret = downloadToFile(parsed_config.entries[selected_entry].url, filepath);
					if (ret == 0)
					{
						ret = 7;
					}
				}
				if (ret == 0)
				{
					printf("\x1b[40;32mDownload complete!\x1b[40;37m\n");
					printf("\x1b[40;32mUpdate complete!\x1b[40;37m\n");
					state[selected_entry] = DL_DONE;
				}
				else if (ret == 7)
				{
					printf("\x1b[40;32mDownload complete!\x1b[40;37m\n");
					printf("\x1b[40;32mExtracting files from archive...\x1b[40;37m\n");
					char filepath[256];
					sprintf(filepath, "%s%s.zip", WORKING_DIR, names[selected_entry]);
					ret = extractFileFromZip(filepath, parsed_config.entries[selected_entry].zip_path, parsed_config.entries[selected_entry].path);
					if (ret == 0)
					{
						printf("\x1b[40;32mExtraction complete!\x1b[40;37m\n");
						printf("\x1b[40;32mUpdate complete!\x1b[40;37m\n");
						state[selected_entry] = DL_DONE;
					}
					else
					{
						printf("\x1b[40;31mExtraction failed. Retry or check your config.json.\x1b[40;37m\n");
						state[selected_entry] = DL_ERROR;
					}
				}
				else if (ret == 6)
				{
					state[selected_entry]++;
					if (state[selected_entry] > DL_ERROR)
					{
						state[selected_entry] = 0;
					}
				}
				else
				{
					printf("\x1b[40;31mDownload failed. Retry or check your config.json.\x1b[40;37m\n");
					state[selected_entry] = DL_ERROR;
				}
			}

			gfxFlushBuffers();
			gfxSwapBuffers();

			gspWaitForVBlank();
		}
	}
	else
	{
		printf("\x1b[40;31m\x1b[13;2Herror\x1b[0m");
		printf("\x1b[40;31m\x1b[13;2H%s\x1b[0m", config_errors[parsed_config.errorState-1]);
		while (aptMainLoop()) {

			hidScanInput();

			if (hidKeysDown() & KEY_START)
			{
				break;
			}

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
