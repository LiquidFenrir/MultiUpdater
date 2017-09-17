#include "menu.hpp"

static const unsigned int FRAMES_FOR_TEXT_SCROLL = 40;
static const unsigned int MENU_WIDTH = 40;
static const unsigned int MAX_ENTRIES_PER_SCREEN = 20;

static const unsigned int MENU_Y_OFFSET = 5;
static const unsigned int MENU_X_OFFSET = 5;
static const char MENU_DELIMITER_CHAR = '=';

static const unsigned int TITLE_Y_OFFSET = 3;
static const unsigned int TITLE_X_OFFSET = 3;
#define TITLE_STRING   APP_TITLE " " VERSION_STRING " by " APP_AUTHOR

static unsigned int framesCount = 0;
static unsigned int verticalScroll = 0;
static unsigned int horizontalScroll = 0;
static unsigned int previousSelectedEntry = 0;
static int horizontalScrollChange = 1;

void drawMenu(Config config, unsigned int selectedEntry)
{
	unsigned int i = 0;

	// Scroll the entry name if it's larger than the width of the menu
	//----------------------------------------------------------------
	if (previousSelectedEntry != selectedEntry) {
		previousSelectedEntry = selectedEntry;
		framesCount = 0;
		horizontalScroll = 0;
		horizontalScrollChange = 1;
	}

	framesCount = (framesCount+1) % FRAMES_FOR_TEXT_SCROLL;

	if (framesCount == 0 && (config.entries[selectedEntry].name.size() > MENU_WIDTH))
		horizontalScroll += horizontalScrollChange;

	if (horizontalScroll == (config.entries[selectedEntry].name.size() - MENU_WIDTH))
		horizontalScrollChange = -1;

	if (horizontalScroll == 0)
		horizontalScrollChange = 1;
	//----------------------------------------------------------------

	//Scroll the menu up or down if the selected entry is out of its bounds
	//----------------------------------------------------------------
	for (i = 0; i < config.entries.size(); i++) {
		if (config.entries.size() <= MAX_ENTRIES_PER_SCREEN) break;

		if (verticalScroll > selectedEntry)
			verticalScroll--;

		if ((i < selectedEntry) && \
		   ((selectedEntry - verticalScroll) >= MAX_ENTRIES_PER_SCREEN) && \
		   (verticalScroll != (config.entries.size() - MAX_ENTRIES_PER_SCREEN)))
			verticalScroll++;
	}
	//----------------------------------------------------------------

	printf("\x1b[0m\x1b[%u;%uH%s\n", TITLE_Y_OFFSET, TITLE_X_OFFSET, TITLE_STRING);

	for (i = MENU_X_OFFSET; i < MENU_WIDTH+MENU_X_OFFSET; i++) {
		printf("\x1b[%u;%uH%c\n", MENU_Y_OFFSET, i, MENU_DELIMITER_CHAR);
		printf("\x1b[%u;%uH%c\n", MENU_Y_OFFSET+MAX_ENTRIES_PER_SCREEN+1, i, MENU_DELIMITER_CHAR);
	}

	for (i = verticalScroll; i < (MAX_ENTRIES_PER_SCREEN + verticalScroll); i++) {
		if (i >= config.entries.size())
			break;
		
		// Gets the color of the text depending on the state of the entry
		// black for selected, white for normal
		// red for failed, green for success, yellow for marked
		//----------------------------------------------------------------
		unsigned int textColor = (i == selectedEntry) ? 30 : 37;
		unsigned int textColorArray[] = {
			textColor, // STATE_NONE
			31, // STATE_FAILED
			32, // STATE_SUCCESS
			33, // STATE_MARKED
		};

		textColor = textColorArray[__builtin_clz(0)-__builtin_clz(config.entries[i].state)];
		//----------------------------------------------------------------

		printf("\x1b[%u;%uH\x1b[%u;%um%*.*s\x1b[0m\n",
			MENU_Y_OFFSET+i-verticalScroll+1,
			MENU_X_OFFSET,
			(i == selectedEntry) ? 47 : 40, //background color, white for selected, black for normal
			textColor,
			-MENU_WIDTH, //adds padding on the right
			MENU_WIDTH, //maximum length of text (will truncate if over)
			config.entries[i].name.c_str() + ((i == selectedEntry) ? horizontalScroll : 0)
		);
	}
	
	printf("\x1b[0m\x1b[%u;%uH%*s\n", TITLE_Y_OFFSET+MAX_ENTRIES_PER_SCREEN+MENU_Y_OFFSET, TITLE_X_OFFSET, MENU_WIDTH+TITLE_X_OFFSET+1, "Press START to quit.");
}