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
	printf("\x1b[40;34mUpdating %s...\x1b[0m\n", name.c_str());
	std::string downloadPath;
	
	//if the file to download isnt an archive, direcly download where wanted
	if (m_inArchive.empty()) {
		downloadPath = m_path;
	}
	//otherwise, download to an archive in the working dir, then extract where wanted
	else {
		std::stringstream downloadPathStream;
		downloadPathStream << WORKING_DIR << name.c_str() << ".archive";
		downloadPath = downloadPathStream.str();
	}
	
	Result ret = 0;
	
	//if the entry doesnt want anything from a release, expect it to be a normal file
	if (m_inRelease.empty())
		ret = downloadToFile(m_url, downloadPath, false);
	else
		ret = downloadFromRelease(m_url, m_inRelease, downloadPath);
	
	if (ret != 0) {
		printf("\x1b[40;31mDownload failed!");
		goto failure;
	}
	else
		printf("\x1b[40;32mDownload successful!");
	
	if (!(m_inArchive.empty())) {
		printf("\n\x1b[40;34mExtracting file from the archive...\x1b[0m\n");
		ret = extractArchive(downloadPath, m_inArchive, m_path);
		if (ret != 0) {
			printf("\x1b[40;31mExtraction failed!");
			goto failure;
		}
		else
			printf("\x1b[40;32mExtraction successful!");
	}
	
	if (matchPattern(".*(\\.cia)$", m_path)) {
		printf("\n\x1b[40;34mInstalling CIA...\x1b[0m\n");
		ret = installCia(m_path.c_str());
		if (R_FAILED(ret)) {
			printf("\x1b[40;31mExtraction failed!");
			goto failure;
		}
		else
			printf("\x1b[40;32mExtraction successful!");
	}
	
	printf("\n\x1b[40;32mUpdate complete!\x1b[0m\n");
	state = STATE_SUCCESS;
	return;
	
	failure:
	printf("\n\x1b[40;31mUpdate failed!\n");
	printf("\x1b[40;33mError code: %.8lx\x1b[0m\n", ret);
	state = STATE_FAILED;
	return;
}

Config::Config()
{
	m_selfUpdater = true;
	m_deleteCIA = false;
	m_deleteArchive = true;

	Handle configHandle;
	char* configString = nullptr;
	Result ret = openFile(&configHandle, CONFIG_FILE_PATH, false);

	if (R_FAILED(ret)) {
		//try to open the config from romfs
		ret = openFile(&configHandle, CONFIG_FILE_ROMFS, false);
	}
	
	if (R_FAILED(ret)) {
		json configDownloadJson;
		configDownloadJson["name"] = "Download the latest example config!";
		configDownloadJson["url"] = CONFIG_FILE_URL;
		configDownloadJson["path"] = CONFIG_FILE_PATH;

		Entry configDownloadEntry(configDownloadJson);
		entries.push_back(configDownloadEntry);
	}
	else {
		u64 configSize = 0;
		FSFILE_GetSize(configHandle, &configSize);
		configString = (char*)calloc(configSize+1, sizeof(char));
		u32 bytesRead = 0;
		FSFILE_Read(configHandle, &bytesRead, 0, configString, (u32)configSize);

		json parsedConfig = json::parse(configString);

		if (parsedConfig["config"].is_object()) {
			if (parsedConfig["config"]["self_updater"].is_boolean())
				m_selfUpdater = parsedConfig["config"]["self_updater"];
			if (parsedConfig["config"]["delete_cias"].is_boolean())
				m_deleteCIA = parsedConfig["config"]["delete_cias"];
			if (parsedConfig["config"]["delete_archives"].is_boolean())
				m_deleteArchive = parsedConfig["config"]["delete_archives"];
		}

		if (m_selfUpdater) {
			json selfUpdaterJson;
			selfUpdaterJson["name"] = "MultiUpdater";
			selfUpdaterJson["url"] = "https://github.com/LiquidFenrir/MultiUpdater";
			selfUpdaterJson["inrelease"] = "MultiUpdater.*\\.zip";
			selfUpdaterJson["inarchive"] = "MultiUpdater.cia";
			selfUpdaterJson["path"] = "/cias/MultiUpdater.cia";

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
