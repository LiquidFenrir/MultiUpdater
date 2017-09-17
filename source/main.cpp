#include "common.hpp"
#include "menu.hpp"
#include "config.hpp"

int main()
{	
	gfxInitDefault();
	romfsInit();
	httpcInit(0);
	amInit();
	
	PrintConsole topScreen, bottomScreen;
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);
	
	Config config;
	
	unsigned int selectedEntry = 0;
	
	while(aptMainLoop())
	{
		consoleSelect(&topScreen);
		drawMenu(config, selectedEntry);
		consoleSelect(&bottomScreen);
		
		hidScanInput();
		
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
		else if (kDown & KEY_DOWN) {
			selectedEntry++;
			if (selectedEntry >= config.entries.size())
				selectedEntry = config.entries.size()-1;
		}
		else if (kDown & KEY_UP) {
			selectedEntry--;
			if (selectedEntry >= config.entries.size())
				selectedEntry = 0;
		}
		else if (kDown & KEY_LEFT) {
			selectedEntry = 0;
		}
		else if (kDown & KEY_RIGHT) {
			selectedEntry = config.entries.size()-1;
		}
		else if (kDown & KEY_A) {
			config.entries[selectedEntry].update();
		}
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}
	
	amExit();
	httpcExit();
	romfsExit();
	gfxExit();
	return 0;
}