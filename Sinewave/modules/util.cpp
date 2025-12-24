#include "util.h"


bool Utility::CreateShortcut(const std::string& shortcutPath, const std::string& target, const std::string& iconPath) {
    HRESULT hres;
    bool success = false;
    auto strToWstr = [](const std::string& str) -> std::wstring {
        return std::wstring(str.begin(), str.end());
    };

    std::wstring wShortcut = strToWstr(shortcutPath);
    std::wstring wTarget = strToWstr(target);
    std::wstring wIcon = strToWstr(iconPath);

    hres = CoInitialize(NULL);
    if (FAILED(hres)) {
        return false;
    }

    IShellLink* psl;
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres)) {
        psl->SetPath(wTarget.c_str());
        psl->SetIconLocation(wIcon.c_str(), 0);

        IPersistFile* ppf;
        hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
        if (SUCCEEDED(hres)) {
            hres = ppf->Save(wShortcut.c_str(), TRUE);
            ppf->Release();
            success = SUCCEEDED(hres);
        }
        psl->Release();
    }

    CoUninitialize();
    return success;
}

std::string Utility::getUniverseId(const std::string& placeId) {
    HttpResponse placeReq = Http::newRequest("https://apis.roblox.com/universes/v1/places/" + placeId + "/universe", "GET");
    json placeJ = json::parse(placeReq.content);
    std::string universe = placeJ["universeId"].dump();
    return universe;
}

std::string Utility::getGameIcon(const std::string& universeId) {
    HttpResponse imageReq = Http::newRequest("https://thumbnails.roblox.com/v1/games/icons?universeIds=" + universeId + "&returnPolicy=PlaceHolder&size=256x256&format=Png&isCircular=false", "GET");
    json img = json::parse(imageReq.content);
    std::string imageKey = img["data"][0]["imageUrl"].get<std::string>();
    return imageKey;
}

void Utility::openConsole() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    SetConsoleTitleA("Sinewave Console");
}
