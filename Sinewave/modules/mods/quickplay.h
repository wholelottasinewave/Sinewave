#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "../util.h"
#include "../http.h"
#include "../bootstrapper.h"
#include <FreeImage.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class QuickPlay
{
public:
	static void addGame(const std::string& name, const std::string& id);
};

