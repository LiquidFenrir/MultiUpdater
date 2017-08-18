#include "config.hpp"

#include "extract.hpp"
#include "download.hpp"

extern "C" {
	#include "cia.h"
}

Entry::Entry(json jsonEntry)
{
	state = STATE_NONE;
	if (jsonEntry["inarchive"].is_string())
		m_inArchive = jsonEntry["inarchive"];
	if (jsonEntry["inrelease"].is_string())
		m_inRelease = jsonEntry["inrelease"];
	if (jsonEntry["path"].is_string())
		m_path = jsonEntry["path"];
	if (jsonEntry["url"].is_string())
		m_url = jsonEntry["url"];
	if (jsonEntry["name"].is_string())
		name = jsonEntry["name"];
}

void Entry::update()
{
	printf("\x1b[40;34;1mUpdating %s...\x1b[0m\n", name.c_str());
	std::string downloadPath;
	
	//if the file to download isnt an archive, direcly download where wanted
	if (m_inArchive.empty()) {
		downloadPath = m_path;
	}
	//otherwise, download to an archive in the working dir, then extract where wanted
	else {
		auto pathLength = std::snprintf(nullptr, 0, "%s%s.archive", WORKING_DIR, name.c_str());
		std::string tempString(pathLength+1, '\0');
		std::sprintf(&tempString[0], "%s%s.archive", WORKING_DIR, name.c_str());
		downloadPath = tempString;
	}
	
	Result ret = 0;
	
	//if the entry doesnt want anything from a release, expect it to be a normal file
	if (m_inRelease.empty())
		ret = downloadToFile(m_url, downloadPath, false);
	else
		ret = downloadFromRelease(m_url, m_inRelease, downloadPath);
	
	if (ret != 0) {
		printf("\x1b[40;31;1mDownload failed!");
		goto failure;
	}
	else
		printf("\x1b[40;32;1mDownload successful!");
	
	if (!(m_inArchive.empty())) {
		printf("\n\x1b[40;34;1mExtracting file from the archive...\x1b[0m\n");
		ret = extractArchive(downloadPath, m_inArchive, m_path);
		if (ret != 0) {
			printf("\x1b[40;31;1mExtraction failed!");
			goto failure;
		}
		else
			printf("\x1b[40;32;1mExtraction successful!");
	}
	
	if (matchPattern(".*(\\.cia)$", m_path)) {
		printf("\n\x1b[40;34;1mInstalling CIA...\x1b[0m\n");
		ret = installCia(m_path.c_str());
		if (R_FAILED(ret)) {
			printf("\x1b[40;31;1mExtraction failed!");
			goto failure;
		}
		else
			printf("\x1b[40;32;1mExtraction successful!");
	}
	
	printf("\n\x1b[40;32;1mUpdate complete!\x1b[0m\n");
	state = STATE_SUCCESS;
	return;
	
	failure:
	printf("\n\x1b[40;31;1mUpdate failed!\n");
	printf("\x1b[40;33;1mError code: %.8lx\x1b[0m\n", ret);
	state = STATE_FAILED;
	return;
}

void Entry::backup()
{

}

void Entry::restore()
{

}

Config::Config()
{	
	m_selfUpdater = true;
	m_deleteCIA = false;
	m_deleteArchive = true;
	m_logOutput = true;

	Handle configHandle;
	char* configString = nullptr;
	Result ret = openFile(&configHandle, CONFIG_FILE_PATH, false);

	if (R_FAILED(ret)) {
		auto configDownloadJson = R"(
			{
				"name": "Download the latest example config!",
				"url": CONFIG_FILE_URL,
				"path": CONFIG_FILE_PATH
			}
		)"_json;
		Entry configDownloadEntry(configDownloadJson);
		entries.push_back(configDownloadEntry);
	}
	else {
		u64 configSize = 0;
		FSFILE_GetSize(configHandle, &configSize);
		configString = (char*)calloc(configSize+1, sizeof(char));
		u32 bytesRead = 0;
		FSFILE_Read(configHandle, &bytesRead, 0, configString, (u32)configSize);

		auto parsedConfig = json::parse(configString);

		if (parsedConfig["config"].is_object()) {
			m_selfUpdater = parsedConfig["config"]["self_updater"];
			m_deleteCIA = parsedConfig["config"]["delete_cias"];
			m_deleteArchive = parsedConfig["config"]["delete_archives"];
			m_logOutput = parsedConfig["config"]["log_output"];
		}

		if (m_selfUpdater) {
			auto selfUpdaterJson = R"(
				{
					"name": "MultiUpdater",
					"url": "https://github.com/LiquidFenrir/MultiUpdater",
					"inrelease": "MultiUpdater-v*zip",
					"inarchive": "MultiUpdater.cia",
					"path": "/cias/MultiUpdater.cia"
				}
			)"_json;
			
			Entry selfUpdateEntry(selfUpdaterJson);
			entries.push_back(selfUpdateEntry);
		}

		if (parsedConfig["entries"].is_array()) {
			for (auto jsonEntry : parsedConfig["entries"]) {
				if (jsonEntry.is_object()) {
					Entry configEntry(jsonEntry);
					entries.push_back(configEntry);
				}
			}
		}
	}

	free(configString);
	FSFILE_Close(configHandle);
}
