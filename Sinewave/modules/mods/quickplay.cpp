#include "quickplay.h"

void QuickPlay::addGame(const std::string& name, const std::string& id) {
    std::string universe = Utility::getUniverseId(id);
    std::string imageKey = Utility::getGameIcon(universe);

    auto png = std::filesystem::path(Globals::appdata) / "Sinewave" / "Assets" / (name + ".png");
    Http::downloadFile(imageKey, png.string());

    std::filesystem::path ico;

    FIBITMAP* bitmap = FreeImage_Load(FIF_PNG, png.string().c_str(), PNG_DEFAULT);
    if (bitmap) {
        ico = png;
        ico.replace_extension(".ico");
        if (!FreeImage_Save(FIF_ICO, bitmap, ico.string().c_str(), 0)) {
            std::cout << "[SINEWAVE]: Could not save ICO!\n";
        }
        FreeImage_Unload(bitmap);
    }
    else {
        std::cout << "[SINEWAVE]: Could not load PNG for shortcut!\n";
    }

    std::filesystem::path desktop = std::filesystem::path(getenv("USERPROFILE")) / "Desktop";

    std::string scPath = (desktop / (name + ".lnk")).string();
    std::string target = "roblox://placeId=" + id;
    Utility::CreateShortcut(scPath, target, ico.string());
}