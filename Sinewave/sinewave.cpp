#include <iostream>
#include <curl/curl.h>
#include <conio.h>
#include "modules/http.h"
#include "modules/bootstrapper.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>
#include <imgui_internal.h>
#include "modules/configmgr.h"
#include <shellapi.h>
#include "modules/watcher.h"
#include <regex>
#include "discord/discord.h"
#include "imgui_stdlib.h"
#include <FreeImage.h>
#include "modules/util.h"
#include "modules/mods/quickplay.h"
#include "modules/mods/rpc.h"

/*

    this main file will be organized soon

*/

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

discord::Core* core = nullptr;

void setCursors(const std::filesystem::path& source) {
    auto dest = Globals::versionFolder / "content" / "textures" / "Cursors" / "KeyboardMouse";
    deleteDirectoryContents(dest);
    std::filesystem::copy(source, dest, std::filesystem::copy_options::recursive);
}

std::filesystem::path getexepath()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    Bootstrapper::initDirectories();

    Bootstrapper::init();

    ConfigManager::loadConfig();
    Globals::versionFolder = std::filesystem::path(Globals::localAppdata) / "Roblox" / "Versions" / Globals::currentVersion;
    discord::Core::Create(1367606413567725619, DiscordCreateFlags_Default, &core);

    if (Config::values["debugmode"]) {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        SetConsoleTitleA("Sinewave Console");
    }

    if (lpCmdLine && lpCmdLine[0] != '\0') {
        std::cout << "[SINEWAVE]: Roblox is up to date! Starting..\n";
        std::cout << "[SINEWAVE]: Thank you for using Sinewave, mods and RPC coming soon!!\n";

        if (Config::values["multiroblox"]) {
            std::cout << "[SINEWAVE]: Multi Roblox is enabled!\n";
            HANDLE mutex = CreateMutexW(0, TRUE, L"ROBLOX_singletonMutex");
            HANDLE sevent = CreateMutexW(0, TRUE, L"ROBLOX_singletonEvent");
        }

        /* start roblox */
        STARTUPINFOA si{ sizeof(si) };
        PROCESS_INFORMATION pi{};
        auto path = std::filesystem::path(std::filesystem::path(Globals::localAppdata) / "Roblox" / "Versions" / Globals::currentVersion.c_str() / "RobloxPlayerBeta.exe").string();
        CreateProcessA(path.c_str(), lpCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

        if (Config::values["discordrpc"]) {
            std::thread rpc(RPCMgr::start, core);
            rpc.detach();
        }

        if (Config::values["multiroblox"] || Config::values["discordrpc"]) {
            WaitForSingleObject(pi.hProcess, INFINITE);
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        FreeImage_Initialise();
        std::string name{};
        std::string id{};
        /* imgui */
      // Make process DPI aware and obtain main monitor scale
        ImGui_ImplWin32_EnableDpiAwareness();
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

        // Create application window
        WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
        ::RegisterClassExW(&wc);
        HWND hwnd;
        if (Config::values["topmost"]) {
            hwnd = ::CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST, _T("ImGui Example"), NULL, WS_POPUP, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
        }
        else {
            hwnd = ::CreateWindowEx(WS_EX_LAYERED, _T("ImGui Example"), NULL, WS_POPUP, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
        }
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);

        // Initialize Direct3D
        if (!CreateDeviceD3D(hwnd))
        {
            CleanupDeviceD3D();
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        // Show the window
        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        //ImGui::StyleColorsLight();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Catppuccin Mocha Palette
        // --------------------------------------------------------
        const ImVec4 base = ImVec4(0.117f, 0.117f, 0.172f, 1.0f); // #1e1e2e
        const ImVec4 mantle = ImVec4(0.109f, 0.109f, 0.156f, 1.0f); // #181825
        const ImVec4 surface0 = ImVec4(0.200f, 0.207f, 0.286f, 1.0f); // #313244
        const ImVec4 surface1 = ImVec4(0.247f, 0.254f, 0.337f, 1.0f); // #3f4056
        const ImVec4 surface2 = ImVec4(0.290f, 0.301f, 0.388f, 1.0f); // #4a4d63
        const ImVec4 overlay0 = ImVec4(0.396f, 0.403f, 0.486f, 1.0f); // #65677c
        const ImVec4 overlay2 = ImVec4(0.576f, 0.584f, 0.654f, 1.0f); // #9399b2
        const ImVec4 text = ImVec4(0.803f, 0.815f, 0.878f, 1.0f); // #cdd6f4
        const ImVec4 subtext0 = ImVec4(0.639f, 0.658f, 0.764f, 1.0f); // #a3a8c3
        const ImVec4 mauve = ImVec4(0.796f, 0.698f, 0.972f, 1.0f); // #cba6f7
        const ImVec4 peach = ImVec4(0.980f, 0.709f, 0.572f, 1.0f); // #fab387
        const ImVec4 yellow = ImVec4(0.980f, 0.913f, 0.596f, 1.0f); // #f9e2af
        const ImVec4 green = ImVec4(0.650f, 0.890f, 0.631f, 1.0f); // #a6e3a1
        const ImVec4 teal = ImVec4(0.580f, 0.886f, 0.819f, 1.0f); // #94e2d5
        const ImVec4 sapphire = ImVec4(0.458f, 0.784f, 0.878f, 1.0f); // #74c7ec
        const ImVec4 blue = ImVec4(0.533f, 0.698f, 0.976f, 1.0f); // #89b4fa
        const ImVec4 lavender = ImVec4(0.709f, 0.764f, 0.980f, 1.0f); // #b4befe

        // Main window and backgrounds
        colors[ImGuiCol_WindowBg] = base;
        colors[ImGuiCol_ChildBg] = base;
        colors[ImGuiCol_PopupBg] = surface0;
        colors[ImGuiCol_Border] = surface1;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_FrameBg] = surface0;
        colors[ImGuiCol_FrameBgHovered] = surface1;
        colors[ImGuiCol_FrameBgActive] = surface2;
        colors[ImGuiCol_TitleBg] = mantle;
        colors[ImGuiCol_TitleBgActive] = surface0;
        colors[ImGuiCol_TitleBgCollapsed] = mantle;
        colors[ImGuiCol_MenuBarBg] = mantle;
        colors[ImGuiCol_ScrollbarBg] = surface0;
        colors[ImGuiCol_ScrollbarGrab] = surface2;
        colors[ImGuiCol_ScrollbarGrabHovered] = overlay0;
        colors[ImGuiCol_ScrollbarGrabActive] = overlay2;
        colors[ImGuiCol_CheckMark] = ImVec4(0.859f, 0.882f, 1.000f, 1.000f);
        colors[ImGuiCol_SliderGrab] = sapphire;
        colors[ImGuiCol_SliderGrabActive] = blue;
        colors[ImGuiCol_Button] = surface0;
        colors[ImGuiCol_ButtonHovered] = surface1;
        colors[ImGuiCol_ButtonActive] = surface2;
        colors[ImGuiCol_Header] = surface0;
        colors[ImGuiCol_HeaderHovered] = surface1;
        colors[ImGuiCol_HeaderActive] = surface2;
        colors[ImGuiCol_Separator] = surface1;
        colors[ImGuiCol_SeparatorHovered] = mauve;
        colors[ImGuiCol_SeparatorActive] = mauve;
        colors[ImGuiCol_ResizeGrip] = surface2;
        colors[ImGuiCol_ResizeGripHovered] = mauve;
        colors[ImGuiCol_ResizeGripActive] = mauve;
        colors[ImGuiCol_Tab] = surface0;
        colors[ImGuiCol_TabHovered] = surface2;
        colors[ImGuiCol_TabActive] = surface1;
        colors[ImGuiCol_TabUnfocused] = surface0;
        colors[ImGuiCol_TabUnfocusedActive] = surface1;

        colors[ImGuiCol_PlotLines] = blue;
        colors[ImGuiCol_PlotLinesHovered] = peach;
        colors[ImGuiCol_PlotHistogram] = teal;
        colors[ImGuiCol_PlotHistogramHovered] = green;
        colors[ImGuiCol_TableHeaderBg] = surface0;
        colors[ImGuiCol_TableBorderStrong] = surface1;
        colors[ImGuiCol_TableBorderLight] = surface0;
        colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = surface2;
        colors[ImGuiCol_DragDropTarget] = yellow;
        colors[ImGuiCol_NavHighlight] = lavender;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);
        colors[ImGuiCol_Text] = text;
        colors[ImGuiCol_TextDisabled] = subtext0;

        // Rounded corners
        style.WindowRounding = 0.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;

        // Padding and spacing
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(5.0f, 3.0f);
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
        style.IndentSpacing = 21.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 10.0f;

        // Borders
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;
        style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        auto fontp = getexepath() / "Roboto-Regular.ttf";
        io.Fonts->AddFontFromFileTTF(fontp.string().c_str(), 16.0f);

        // Our state
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // Main loop
        bool done = false;
        while (!done)
        {
            // Poll and handle messages (inputs, window resize, etc.)
            // See the WndProc() function below for our to dispatch events to the Win32 backend.
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    done = true;
            }
            if (done)
                break;

            // Handle window being minimized or screen locked
            if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
            {
                ::Sleep(10);
                continue;
            }
            g_SwapChainOccluded = false;

            // Handle window resize (we don't resize directly in the WM_SIZE handler)
            if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
            {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
                g_ResizeWidth = g_ResizeHeight = 0;
                CreateRenderTarget();
            }

            // Start the Dear ImGui frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            bool show = true;
            if (show) {
                ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImVec2 center = viewport->GetCenter();

                ImGui::SetNextWindowPos(center, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
                ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_Always);
                ImGui::Begin("Sinewave | Public Beta v0.0.1", &show, ImGuiWindowFlags_NoResize);

                if (ImGui::BeginTabBar("Tabs")) {
                    if (ImGui::BeginTabItem("Home"))
                    {
                        if (ImGui::BeginTable("HomeT", 1, ImGuiTableFlags_SizingStretchSame)) {
                            ImGui::TableNextColumn();
                            ImGui::BeginChild("HomeP", ImVec2(0, 180), true);
                            ImGui::Text("Home");
                            ImGui::Separator();
                            ImGui::Text("Thank you for choosing Sinewave!");
                            ImGui::Text("Please start games from the browser.");
                            ImGui::EndChild();
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Mods"))
                    {
                        if (ImGui::BeginTable("Sections", 2, ImGuiTableFlags_SizingStretchSame)) {
                            /* fun */
                            ImGui::TableNextColumn();
                            ImGui::BeginChild("FunP", ImVec2(0, 180), true);
                            ImGui::Text("Fun");
                            ImGui::Separator();

                            if (ImGui::Checkbox("Multi Roblox", &Config::values["multiroblox"])) {
                                ConfigManager::saveConfig();
                            }

                            if (ImGui::Checkbox("Discord Rich Presence", &Config::values["discordrpc"])) {
                                ConfigManager::saveConfig();
                            }

                            /*

                            if (ImGui::Checkbox("Show Server Region", &Config::values["serverRegion"])) {
                                ConfigManager::saveConfig();
                            }*/

                            ImGui::EndChild();

                            /* customization */
                            ImGui::TableNextColumn();
                            ImGui::BeginChild("CustomizationP", ImVec2(0, 180), true);
                            ImGui::Text("Customization");
                            ImGui::Separator();

                            if (ImGui::Checkbox("2014 Cursors", &Config::values["cursors2014"])) {
                                const auto& source = Config::values["cursors2014"] ? std::filesystem::path(Globals::appdata) / "Sinewave" / "Assets" / "Cursors2014" : std::filesystem::path(Globals::appdata) / "Sinewave" / "Assets" / "CursorsDefault";
                                setCursors(source);
                                ConfigManager::saveConfig();
                            }

                            if (ImGui::Checkbox("2006 Cursors", &Config::values["cursors2006"])) {
                                const auto& source = Config::values["cursors2006"] ? std::filesystem::path(Globals::appdata) / "Sinewave" / "Assets" / "Cursors2006" : std::filesystem::path(Globals::appdata) / "Sinewave" / "Assets" / "CursorsDefault";
                                setCursors(source);
                                ConfigManager::saveConfig();
                            }

                            ImGui::EndChild();
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("FFlags")) {
                        ImGui::Text("Coming soon!!");
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Quick Play")) {
                        if (ImGui::BeginTable("QuickT", 1, ImGuiTableFlags_SizingStretchSame)) {
                            ImGui::TableNextColumn();
                            ImGui::BeginChild("QP", ImVec2(0, 180), true);
                            ImGui::Text("Quick Play");
                            ImGui::Separator();

                            ImGui::InputTextWithHint("##name", "Name", &name);
                            ImGui::InputTextWithHint("##id", "Game ID", &id);
                            ImGui::Spacing();
                            if (ImGui::Button("Create Shortcut")) {
                                QuickPlay::addGame(name, id);
                            }
                            ImGui::EndChild();
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Settings")) {
                        if (ImGui::BeginTable("SettingsT", 1, ImGuiTableFlags_SizingStretchSame)) {
                            ImGui::TableNextColumn();
                            ImGui::BeginChild("SettingsP", ImVec2(0, 180), true);
                            ImGui::Text("Settings");
                            ImGui::Separator();
                            if (ImGui::Checkbox("Debug Mode", &Config::values["debugmode"])) {
                                ConfigManager::saveConfig();
                            }

                            if (ImGui::Checkbox("Keep GUI On Top", &Config::values["topmost"])) {
                                ConfigManager::saveConfig();
                            }

                            ImGui::EndChild();
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }
                ImGui::End();
            }

            if (!show)
            {
                FreeImage_DeInitialise();
                done = true;
                ::PostQuitMessage(0);
            }
            // Rendering
            ImGui::Render();
            const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // Present
            HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
            //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
            g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
        }

        // Cleanup
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    }
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    // This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}