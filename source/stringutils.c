#include "stringutils.h"

int matchPattern(const char * pattern, const char * str)
{
	int p_offset = 0, s_offset = 0;
	char current_p_char = '\0', current_s_char = '\0', next_p_char = '\0';

	while (str[s_offset] != '\0') {
		current_p_char = pattern[p_offset];
		current_s_char = str[s_offset];
		if (current_p_char == '*' && next_p_char == '\0') {
			next_p_char = pattern[p_offset+1];
		}
		if (next_p_char != '\0') {
			if (current_s_char != next_p_char) {
				s_offset++;
				continue;
			}
			else {
				current_p_char = next_p_char;
				p_offset++;
				next_p_char = '\0';
			}
		}
		if (current_p_char != current_s_char) return 1;
		
		s_offset++;
		p_offset++;
	}
	return (next_p_char != '\0');
}

void cleanPath(char * path)
{
	for (int i = 0; path[i]; i++) { //replace all spaces and fat32 reserved characters in the path with underscores 
		switch (path[i]) {
			case ' ':
			case '"':
			case '*':
			case ':':
			case '<':
			case '>':
			case '?':
			case '\\':
			case '|':
				path[i] = '_';
			default:
				break;
		}
	}
}
