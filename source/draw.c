#include "draw.h"

static u8 scroll = 0;

void drawInstructions()
{
	printf("\x1b[40;33m" //color the text to make it more noticeable
	       "Press (A) to update the highlighted\n and marked entries.\n" 
	       "Press (B) to backup the highlighted\n and marked entries.\n" 
	       "Press (X) to restore the backups of the\n highlighted and marked entries\n"
	       "\n"
	       "Press (Y) to mark or unmark the\n highlighted entry.\n" 
	       "Press [L] to mark all entries.\n" 
	       "Press [R] to unmark all entries.\n"
	       "\x1b[0m"); //clears the text color back to white on black background
}

void drawMenu(config_t * parsed_config, u8 * state, u8 selected_entry)
{
	if (selected_entry == 0) {
		scroll = 0;
	}
	else if (selected_entry == parsed_config->entries_number-1) {
		scroll = parsed_config->entries_number - MAX_ENTRIES_PER_SCREEN;
		if (scroll >= parsed_config->entries_number)
			scroll = 0;
	}
	else if (selected_entry >= (MAX_ENTRIES_PER_SCREEN + scroll)) {
		scroll++;
	}
	else if ((selected_entry - scroll) < 0) {
		scroll--;
	}
	
	int i = 0;
	
	//top delimiter
	for (i = 0; i < MENU_WIDTH; i++)
		printf((scroll == 0) ? "=" : "\x1E"); //up arrow
	
	for(i = scroll; i < (MAX_ENTRIES_PER_SCREEN + scroll); i++) {
		
		char * current_name = (char *)parsed_config->entries[i].name;
		char format[32] = {0};
		
		strcat(format, (i == selected_entry) ? "\x1b[47;" : "\x1b[40;"); //selected entry has gray background, otherwise, black background
		if (state[i] & STATE_MARKED) {
			strcat(format, "33"); //marked entries have yellow text
		}
		else if (state[i] & UPDATE_DONE) {
			strcat(format, "32"); //already completed entries have green/lime text
		}
		else if (state[i] & UPDATE_ERROR) {
			strcat(format, "31"); //entries where errors have happened have red text
		}
		else {
			strcat(format, (i == selected_entry) ? "30" : "37"); //all the others have white text, except the selected which has black
		}
		
		//finishes the text color escape code
		strcat(format, "m");
		//always print * characters, with padding if needed, and truncate if over
		strcat(format, "%*.*s");
		strcat(format, "\x1b[0m");
		printf(format, -(MENU_WIDTH), MENU_WIDTH, (current_name != NULL) ? current_name : ""); //the - tells it to pad to the right
	}
	
	//bottom delimiter
	for (i = 0; i < MENU_WIDTH; i++) {
		int maxnum = parsed_config->entries_number - MAX_ENTRIES_PER_SCREEN;
		printf((maxnum > 0 && scroll != maxnum) ? "\x1F" : "="); //down arrow
	}
}
