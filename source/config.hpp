#pragma once

#include "common.hpp"

enum EntryState {	
	STATE_NONE    = 0,
	STATE_FAILED  = BIT(0),
	STATE_SUCCESS = BIT(1),
	STATE_MARKED  = BIT(2),
};

enum EntryAction {
	ACTION_UPDATE,
	ACTION_BACKUP,
	ACTION_RESTORE,
};

class Entry
{
	public:
	
	std::string name;
	EntryState state;
	
	Entry(json jsonEntry);
	
	void update();
	void backup();
	void restore();
	
	private:
	
	std::string m_url;
	std::string m_path;
	std::string m_inRelease;
	std::string m_inArchive;
};

class Config
{
	public:
	
	bool m_selfUpdater;
	bool m_deleteCIA;
	bool m_deleteArchive;
	bool m_logOutput;
	
	std::vector<Entry> entries;
	
	Config();
	
};