#include "draw.h"

#define MENU_WIDTH 40
#define MAX_ENTRIES_PER_SCREEN 18
u8 scroll = 0;

void drawMenu(config * parsed_config, u8 * state, u8 selected_entry)
{
	if (selected_entry >= (MAX_ENTRIES_PER_SCREEN + scroll)) {
		scroll++;
	}
	else if ((selected_entry - scroll) < 0) {
		scroll--;
	}
	
	for (u8 i = 0; i <= 40; i++) {
		printf("\x1b[0;40;37m\x1b[5;%uH=", (4+i));
		printf("\x1b[0;40;37m\x1b[24;%uH=", (4+i));
	}
	
	for(u8 i = scroll; i < (MAX_ENTRIES_PER_SCREEN + scroll); i++) {
		
		char * current_name = (char *)parsed_config->entries[i].name;
		
		if (current_name != NULL) {
			char format[64];
			sprintf(format, "\x1b[%u;4H", (i+6-scroll));
			
			if (i == selected_entry ) {
				strcat(format, "\x1b[47;30m"); //selected entry has gray background
			}
			else {
				strcat(format, "\x1b[40"); //otherwise, black background
				if (state[i] & STATE_MARKED) {
					strcat(format, ";33m"); //marked entries have yellow text
				}
				else if (state[i] & UPDATE_DONE) {
					strcat(format, ";32m"); //already completed entries have green/lime text
				}
				else if (state[i] & UPDATE_ERROR) {
					strcat(format, ";31m"); //entries where errors have happened have red text
				}
				else {
					strcat(format, ";37m"); //all the others have white text
				}
			}
			
			strcat(format, "%s");
			for (u8 i = 0; i < (MENU_WIDTH - strlen(current_name) + 1); i++) {
				strcat(format, " ");
			}
			strcat(format, "\x1b[0m"); //remove all color changes
			
			printf(format, current_name);
		}
	}
}