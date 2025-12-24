#include "configmgr.h"

void deleteDirectoryContents(const std::filesystem::path& dir)
{
	for (const auto& entry : std::filesystem::directory_iterator(dir))
		std::filesystem::remove_all(entry.path());
}

void ConfigManager::loadConfig() {
	auto path = std::filesystem::path(Globals::appdata) / "Sinewave" / "Settings" / "settings.json";
	if (std::filesystem::exists(path)) {
		std::ifstream ifs(path);

		json j;
		ifs >> j;

		for (auto& [key, val] : Config::values) {
			val = j.value(key, val);
		}
	}
	else {
		std::cout << "[SINEWAVE]: loadConfig() failed because the settings.json file doesn't exist!\n";
	}
}

void ConfigManager::saveConfig() {
	auto path = std::filesystem::path(Globals::appdata) / "Sinewave" / "Settings" / "settings.json";
	if (std::filesystem::exists(path)) {
		json j;

		for (auto& [setting, value] : Config::values) {
			j[setting] = value;
		}

		std::ofstream ofs(path);
		ofs << j.dump(4);
	}
	else {
		std::cout << "[SINEWAVE]: saveConfig() failed because the settings.json file doesn't exist!\n";
	}
}
