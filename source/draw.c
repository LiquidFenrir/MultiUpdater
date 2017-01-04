#include "draw.h"

void drawMenu(char ** names, u8 * state, u32 selected_entry)
{
	for (u8 i = 0; i <= 40; i++) {
		printf("\x1b[0;40;37m\x1b[5;%uH=", (4+i));
		printf("\x1b[0;40;37m\x1b[24;%uH=", (4+i));
	}
	
	for(u32 i = 0; i < 18; i++) {
		char format[64];
		sprintf(format, "\x1b[%lu;4H", (i+6));
		
		if (i == selected_entry )
		{
			strcat(format, "\x1b[47;30m"); //selected entry has gray background and black text
		}
		else
		{
			strcat(format, "\x1b[40"); //otherwise, black background
			switch (state[i]) {
				case DL_DONE:
					strcat(format, ";32m"); //already completed entries have green/lime text
					break;
				case DL_ERROR:
					strcat(format, ";31m"); //entries where errors have happened have red text
					break;
				default:
					strcat(format, ";37m"); //all the others have white text
					break;
			}
		}
		
		strcat(format, "%s");
		strcat(format, "\x1b[0m"); //remove all color changes
		if (names[i] != NULL)
		{
			printf(format, names[i]);
		}
	}
}