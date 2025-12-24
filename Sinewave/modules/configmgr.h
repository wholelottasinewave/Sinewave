#pragma once
#include <iostream>
#include "bootstrapper.h"

namespace Config {
	inline std::unordered_map<std::string, bool> values = {
		{"multiroblox", false},
		{"discordrpc", false},
		{"cursors2014", false},
		{"cursors2006", false},
		{"serverRegion", false},
		{"debugmode", false}, 
		{"topmost", true}
	};
}

class ConfigManager
{
public:
	static void loadConfig();
	static void saveConfig();
};


void deleteDirectoryContents(const std::filesystem::path& dir);
