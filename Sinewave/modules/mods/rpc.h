#pragma once
#include <iostream>
#include <thread>
#include "discord/discord.h"
#include "../watcher.h"
#include "../util.h"

class RPCMgr
{
public:
	static void startActivity(discord::Core* core, const std::string& game, const std::string& image);
	static void start(discord::Core* core);
};

