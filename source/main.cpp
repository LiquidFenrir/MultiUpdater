#include "common.hpp"
#include "menu.hpp"
#include "config.hpp"

int main()
{
	gfxInitDefault();
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
		else if (kDown & KEY_Y) {
			config.entries[selectedEntry].state ^= STATE_MARKED; //toggle selected
		}
		else if (kDown & (KEY_L | KEY_R)) {
			for (unsigned int i = 0; i < config.entries.size(); i++) {
				if (kDown & KEY_L)
					config.entries[i].state |= STATE_MARKED; //mark all
				else
					config.entries[i].state &= ~STATE_MARKED; //unmark all
			}
		}
		else if (kDown & KEY_A) {
			for (unsigned int i = 0; i < config.entries.size(); i++) {
				if ((config.entries[i].state & STATE_MARKED) || (i == selectedEntry)) {
					consoleSelect(&topScreen);
					drawMenu(config, i);
					consoleSelect(&bottomScreen);
					
					config.entries[i].update(config.m_deleteArchive, config.m_deleteCIA);
					
					gfxFlushBuffers();
					gfxSwapBuffers();
					gspWaitForVBlank();
				}
			}
		}
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}
	
	amExit();
	httpcExit();
	gfxExit();
	return 0;
}