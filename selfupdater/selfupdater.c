/*
Those 2 files (selfupdater.c and selfupdater.h) are in the public domain.
You can use them in whatever your project is, without caring about the license
*/

#include <3ds.h>
#include <string.h>

#define MULTIUPDATER_TITLEID 0x000400000D5C4900

typedef struct {
	char name[256];
	char url[256];
	char path[256];
	char in_archive[256];
	char in_release[256];
	char magic[5];
} temp_entry_t;

void update(const char * name, const char * url, const char * path, const char * in_archive, const char * in_release)
{
	temp_entry_t sent_entry;
	
	strncpy(sent_entry.name, name, 256);
	strncpy(sent_entry.url, url, 256);
	strncpy(sent_entry.path, path, 256);
	strncpy(sent_entry.in_archive, in_archive, 256);
	strncpy(sent_entry.in_release, in_release, 256);
	strncpy(sent_entry.magic, "MULTI", 5);
	
	APT_PrepareToDoApplicationJump(0, MULTIUPDATER_TITLEID, MEDIATYPE_SD);
	
	u8 hmac[0x20] = {0};
	APT_DoApplicationJump(&sent_entry, sizeof(sent_entry), hmac);
}