#include "draw.h"

void drawMenu(char ** names, u8 * done, u8 * errors, u32 selected_entry)
{
	for (u8 i = 0; i <= 40; i++) {
		printf("\x1b[0;40;37m\x1b[5;%uH=", (4+i));
		printf("\x1b[0;40;37m\x1b[24;%uH=", (4+i));
	}
	
	for(u32 i = 0; i < 18; i++) {
		char format[64];
		sprintf(format, "\x1b[%lu;4H", (i+6));
		
		if ((i+1) == selected_entry )
		{
			strcat(format, "\x1b[2;47"); //selected entry has gray background
		}
		else
		{
			strcat(format, "\x1b[0;40"); //otherwise, black background
		}
		
		if (done[i] == 1)
		{
			strcat(format, ";32m"); //already downloaded entries have green/lime text
		}
		else if (errors[i] == 1)
		{
			strcat(format, ";31m"); //entries where retrieval of name has failed have red text
		}
		else if ((i+1) == selected_entry)
		{
			strcat(format, ";30m"); //selected entry has black text
		}
		else
		{
			strcat(format, ";37m"); //all the others have white text
		}
		
		strcat(format, "%s");
		strcat(format, "\x1b[0m"); //remove all color changes
		if (names[i] != NULL)
		{
			printf(format, names[i]);
		}
	}
}