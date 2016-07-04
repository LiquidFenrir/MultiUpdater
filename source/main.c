#include "config.h"
#include "download.h"
#include "draw.h"
#include "file.h"

int main()
{
	gfxInitDefault();
	httpcInit(0);
	fsInit();
	
	consoleInit(GFX_TOP, NULL);
	
	char filepath[256];
	strcat(filepath, WORKING_DIR);
	strcat(filepath, "config.json");
	
	config parsed_config;
	get_config(filepath, &parsed_config);
	
	printf("\x1b[40;37m\x1b[2;2HMultiUpdater by LiquidFenrir\x1b[0m\x1b[0;0H");
	printf("\x1b[40;37m\x1b[28;28H Press START to quit.\x1b[0m\x1b[0;0H");
	
	u32 selected_entry = 1;
	
	char * names[256];
	for (u16 i = 0; i < parsed_config.entries_number; i++) {
		names[i] = (char *)parsed_config.entries[i].name;
	}
	u8 done[256] = {0};
	u8 errors[256] = {0};
	
	while (aptMainLoop()) {
	
		drawMenu(names, done, errors, selected_entry);
		
		hidScanInput();
		
		if (hidKeysDown() & KEY_START)
		{
			break;
		}
		else if (hidKeysDown() & KEY_DOWN)
		{
			selected_entry++;
			if (selected_entry > parsed_config.entries_number) {
				selected_entry = parsed_config.entries_number;
			}
		}
		else if (hidKeysDown() & KEY_UP)
		{
			selected_entry--;
			if (selected_entry < 1) {
				selected_entry = 1;
			}
		}
		else if (hidKeysDown() & KEY_A)
		{
			if (parsed_config.entries[selected_entry-1].zip_path == NULL) {
				downloadToFile(parsed_config.entries[selected_entry-1].url, parsed_config.entries[selected_entry-1].path);
				done[selected_entry-1] = 1;
			}
			else
			{
				char filepath[256];
				sprintf(filepath, "%s%s.zip", WORKING_DIR, names[selected_entry-1]);
				downloadToFile(parsed_config.entries[selected_entry-1].url, filepath);
				extractFileFromZip(filepath, parsed_config.entries[selected_entry-1].zip_path, parsed_config.entries[selected_entry-1].path);
				done[selected_entry-1] = 1;
			}
		}
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		gspWaitForVBlank();
	}
	
	fsExit();
	httpcExit();
	gfxExit();
	return 0;
}