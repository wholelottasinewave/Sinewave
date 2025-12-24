#pragma once
#include <windows.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "http.h"

using json = nlohmann::json;

class Utility
{
public:
	static bool CreateShortcut(const std::string& shortcutPath, const std::string& target, const std::string& iconPath);
	static std::string getUniverseId(const std::string& placeId);
	static std::string getGameIcon(const std::string& universeId);
	static void openConsole();
};

