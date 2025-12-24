#include "rpc.h"

void RPCMgr::startActivity(discord::Core* core, const std::string& game, const std::string& image) {
    std::string details = "Playing " + game;
    discord::Activity activity;
    activity.SetType(discord::ActivityType::Playing);
    activity.SetDetails(details.c_str());
    activity.SetState("Gaming");
    activity.GetAssets().SetLargeImage(image.c_str());
    activity.GetTimestamps().SetStart(0);
    activity.GetTimestamps().SetEnd(0);

    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
            << " updating activity!\n";
    });

    while (true) {
        core->RunCallbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void RPCMgr::start(discord::Core* core) {
    std::filesystem::path logFile = Watcher::getLogFile();
    std::cout << "[SINEWAVE]: Found log file!\n";
    std::string place = Watcher::getPlaceId(logFile);
    std::string universe = Utility::getUniverseId(place);

    HttpResponse universeReq = Http::newRequest("https://games.roblox.com/v1/games?universeIds=" + universe, "GET");
    json universeJ = json::parse(universeReq.content);
    std::string gameName = universeJ["data"][0]["name"].get<std::string>();

    std::string imageKey = Utility::getGameIcon(universe);

    std::thread discordThread(startActivity, core, gameName, imageKey);
    discordThread.detach();
}
