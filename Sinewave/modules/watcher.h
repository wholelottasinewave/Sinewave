#pragma once
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <regex>
#include "bootstrapper.h"

class Watcher
{
public:
	static std::filesystem::path getLogFile();
	static std::string getPlaceId(const std::filesystem::path& logFile);
};

