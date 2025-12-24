#pragma once
#pragma warning(disable: 4996)
#include <iostream>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <windows.h>
#include <chrono>
#include <thread>
#include <tlhelp32.h>
#include <tchar.h>
#include "http.h"
#include "configmgr.h"
#include "util.h"

using json = nlohmann::json;

namespace Globals {
	inline std::string currentVersion;
	inline std::filesystem::path versionFolder;
	inline char* appdata;
	inline char* localAppdata;
	inline bool firstTime;
}

class Bootstrapper
{
public:
	static void initDirectories();
	static void init();
};

