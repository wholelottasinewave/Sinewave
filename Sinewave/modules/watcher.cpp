#include "watcher.h"

std::filesystem::path Watcher::getLogFile()
{
	auto logdir = std::filesystem::path(Globals::localAppdata) / "Roblox" / "Logs";

	std::unordered_set<std::string> existing;
	for (const auto& entry : std::filesystem::directory_iterator(logdir)) {
		if (entry.is_regular_file() && entry.path().extension() == ".log") {
			existing.insert(entry.path().string());
		}
	}

	while (true) {
		for (const auto& entry : std::filesystem::directory_iterator(logdir)) {
			if (!entry.is_regular_file() || entry.path().extension() != ".log") {
				continue;
			}

			auto p = entry.path().string();
			if (!existing.contains(p)) {
				return entry.path();
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

std::string Watcher::getPlaceId(const std::filesystem::path& logFile) {
    std::regex r(R"(placeid:(\d+))");

    std::ifstream log;
    while (!log.is_open()) /* just in case */
    {
        log.open(logFile, std::ios::in);
        if (!log.is_open())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    log.seekg(0, std::ios::beg);
    std::string line;
    std::smatch match;

    while (true) {
        std::streampos last = log.tellg();
        if (std::getline(log, line)) {
            if (std::regex_search(line, match, r)) {
                return match[1].str();
            }
        }
        else {
            log.clear();
            log.seekg(last);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
}
