#include "params.h"

typedef struct {
	char name[256];
	char url[256];
	char path[256];
	char in_archive[256];
	char in_release[256];
	char magic[5];
} temp_entry_t;

bool handleParams(entry_t * received_entry)
{
	u8 hmac[0x20];
	u64 sender = 0;
	bool received = false;
	temp_entry_t temp_entry;
	
	APT_ReceiveDeliverArg(&temp_entry, sizeof(temp_entry), hmac, &sender, &received);
	//received is useless, it will get set to 1 (true) everytime
	if (strncmp(temp_entry.magic, "MULTI", 5))
		return false;
	
	if (strlen(temp_entry.name) != 0)
		received_entry->name = strdup(temp_entry.name);
	if (strlen(temp_entry.url) != 0)
		received_entry->url = strdup(temp_entry.url);
	if (strlen(temp_entry.path) != 0)
		received_entry->path = strdup(temp_entry.path);
	if (strlen(temp_entry.in_archive) != 0)
		received_entry->in_archive = strdup(temp_entry.in_archive);
	if (strlen(temp_entry.in_release) != 0)
		received_entry->in_release = strdup(temp_entry.in_release);
	
	return true;
}
