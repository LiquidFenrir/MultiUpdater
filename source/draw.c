#include "draw.h"

#define MENU_WIDTH 40
#define MAX_ENTRIES_PER_SCREEN 18
int scroll = 0;

void drawInstructions()
{
	printf("\x1b[40;33m" //color the text to make it more noticeable
	       "Press (A) to update the highlighted\n and marked entries.\n" 
	       "Press (B) to backup the highlighted\n and marked entries.\n" 
	       "Press (X) to restore the backups of the\n highlighted and marked entries\n"
	       "\n"
	       "Press (Y) to mark or unmark the\n highlighted entry.\n" 
	       "Press [L] to mark all entries.\n" 
	       "Press [R] to unmark all entries.\x1b[0m\n");
}

void drawMenu(config_t * parsed_config, u8 * state, int selected_entry)
{
	int i;
	
	for (i = 0; i < parsed_config->entries_number; i++) {
		if (parsed_config->entries_number <= MAX_ENTRIES_PER_SCREEN) break;
		
		if (scroll > selected_entry) {
			scroll--;
		}
		
		if (i < selected_entry && selected_entry-scroll >= MAX_ENTRIES_PER_SCREEN && scroll != parsed_config->entries_number-MAX_ENTRIES_PER_SCREEN) {
			scroll++;
		}
	}
	
	for (i = 0; i <= 40; i++) {
		printf("\x1b[0;40;37m\x1b[%u;%uH=", 5, (4+i));
		printf("\x1b[0;40;37m\x1b[%u;%uH=", 5+MAX_ENTRIES_PER_SCREEN+1, (4+i));
	}
	
	for(i = scroll; i < (MAX_ENTRIES_PER_SCREEN + scroll); i++) {
		
		char * current_name = (char *)parsed_config->entries[i].name;
		
		if (current_name != NULL) {
			char format[64];
			sprintf(format, "\x1b[%u;4H", (i+6-scroll));
			
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
			
			//always print * characters, with padding if needed, and truncate if over
			strcat(format, "m%*.*s");
			printf(format, -(MENU_WIDTH+1), MENU_WIDTH+1, current_name); //the - tells it to pad to the right
		}
	}
}
