#include "bootstrapper.h"

static void setKey(HKEY hKey, const std::string& key, const std::string& valueName, const std::string& valueData) {
	HKEY h = nullptr;

	if (RegOpenKeyExA(hKey, key.c_str(), 0, KEY_SET_VALUE, &h) != ERROR_SUCCESS) {
		std::cout << "[SINEWAVE]: Could not open registry key " << key << "\n";
		return;
	}

	LSTATUS status;
	if (valueName.empty()) {
		status = RegSetValueA(h, NULL, REG_SZ, valueData.c_str(), valueData.size() + 1);
	}
	else {
		status = RegSetValueExA(h, valueName.c_str(), 0, REG_SZ, (const BYTE*)valueData.c_str(), valueData.size() + 1);
	}

	if (status != ERROR_SUCCESS) {
		std::cout << "[SINEWAVE]: Could not set registry value!\n";
		return;
	}
	else {
		std::cout << "[SINEWAVE]: Set registry value!\n";
	}

	RegCloseKey(h);
}

void Bootstrapper::initDirectories() {
	Globals::appdata = std::getenv("APPDATA");
	Globals::localAppdata = std::getenv("LOCALAPPDATA");

	if (Globals::appdata) {
		auto path = std::filesystem::path(Globals::appdata) / "Sinewave";
		if (!std::filesystem::is_directory(path)) {
			Globals::firstTime = true;
			Utility::openConsole();
			std::cout << "[SINEWAVE]: This console is only visible for the first ever download in case of an error.\n[SINEWAVE]: You can enable the console by enabling Debug Mode.\n";
			std::filesystem::create_directory(path);
			std::filesystem::create_directory(path / "Roblox");
			std::filesystem::create_directory(path / "Settings");
			std::filesystem::create_directory(path / "Assets");
			std::filesystem::create_directory(path / "Assets" / "Cursors2014");
			std::filesystem::create_directory(path / "Assets" / "CursorsDefault");
			std::filesystem::create_directory(path / "Assets" / "Cursors2006");
			
			std::ofstream ofs(path / "Settings" / "settings.json");

			json j;
			for (auto& [setting, value] : Config::values) {
				j[setting] = value;
			}

			ofs << j.dump(4);
			ofs.close();

			std::cout << "[SINEWAVE]: Finished writing AppData directories!\n";

			std::vector<std::string> cursors = {"ArrowCursor.png",  "ArrowFarCursor.png", "IBeamCursor.png"};
			std::vector<std::pair<std::string, std::filesystem::path>> urls = {
				{"https://raw.githubusercontent.com/wholelottasinewave/sinewave-assets/main/cursor2014/", path / "Assets" / "Cursors2014"},
				{"https://raw.githubusercontent.com/wholelottasinewave/sinewave-assets/main/cursordefault/", path / "Assets" / "CursorsDefault"},
				{"https://raw.githubusercontent.com/wholelottasinewave/sinewave-assets/main/cursor2006/", path / "Assets" / "Cursors2006"}
			};

			for (const auto& [base, folder] : urls) {
				for (const auto& cursor : cursors) {
					Http::downloadFile(base + cursor, (folder / cursor).string());
				}
			}
			std::cout << "[SINEWAVE]: Finished downloading cursors!\n";
		}
		else {
			Globals::firstTime = false;
		}
	}
}

void Bootstrapper::init() {
	/* get the latest client version */
	HttpResponse version = Http::newRequest("https://clientsettingscdn.roblox.com/v1/client-version/WindowsPlayer", "GET");
	json j = json::parse(version.content);
	
	/* install Roblox */
	auto path = std::filesystem::path(Globals::appdata) / "Sinewave";
	Globals::currentVersion = j["clientVersionUpload"].get<std::string>();
	
	if (!std::filesystem::exists(std::filesystem::path(Globals::localAppdata) / "Roblox" / "Versions" / Globals::currentVersion) || Globals::firstTime) {
		std::cout << "[SINEWAVE]: Fetched latest Roblox version! Installing..\n";
		std::cout << "[SINEWAVE]: Sinewave uses a lazy method to install Roblox.. it's temporary!\n";

		auto filePath = std::filesystem::path(path / "Roblox" / "RobloxPlayerInstaller.exe");
		Http::downloadFile("https://setup.rbxcdn.com/" + Globals::currentVersion + "-RobloxPlayerInstaller.exe", filePath.string());

		/* todo: make a proper bootstrapper like in sinewave v1.0.7 */

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		if (CreateProcess(filePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			WaitForSingleObject(pi.hProcess, INFINITE);

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			system("taskkill /F /IM RobloxPlayerBeta.exe");

			/* set registry values */
			char exePath[MAX_PATH];
			GetModuleFileNameA(NULL, exePath, MAX_PATH);
			std::string pathh = exePath;

			setKey(HKEY_CURRENT_USER, "Software\\ROBLOX Corporation\\Environments\\roblox-player", "clientExe", pathh);
			setKey(HKEY_CURRENT_USER, "Software\\ROBLOX Corporation\\Environments\\roblox-player\\Capabilities", "ApplicationIcon", pathh);
			setKey(HKEY_CLASSES_ROOT, "roblox\\DefaultIcon", "", pathh);
			std::string launcherPath = "\"" + pathh + "\" \"%1\""; 
			setKey(HKEY_CLASSES_ROOT, "roblox-player\\shell\\open\\command", "", launcherPath);
			setKey(HKEY_CLASSES_ROOT, "roblox\\shell\\open\\command", "", launcherPath);
			setKey(HKEY_CLASSES_ROOT, "roblox-player\\DefaultIcon", "", pathh);

			std::cout << "[SINEWAVE]: Finished installing Roblox! Starting..\n";
		}
	}
	else {

	}
}
