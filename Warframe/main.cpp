#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "font.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include "rapidjson/document.h" 
#include <fstream> 
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    //std::ifstream file("../WarframeItems/relics.json");

    //std::string json((std::istreambuf_iterator<char>(file)),
    //    std::istreambuf_iterator<char>());

    //rapidjson::Document doc;

    //doc.Parse(json.c_str());

    //if (doc.HasParseError()) {
    //    std::cerr << "Error parsing JSON: "
    //        << doc.GetParseError() << std::endl;
    //    return 1;
    //}

    //for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
    //    const std::string relicName = it->name.GetString();
    //    const rapidjson::Value& relicData = it->value;

    //    std::cout << "Relic: " << relicName << "\n";

    //    if (relicData.HasMember("count") && relicData["count"].IsInt())
    //        std::cout << "  Count: " << relicData["count"].GetInt() << "\n";

    //    if (relicData.HasMember("vaulted") && relicData["vaulted"].IsBool())
    //        std::cout << "  Vaulted: " << relicData["vaulted"].GetBool() << "\n";

    //    std::cout << "--------------------------\n";
    //}
    
    std::ifstream ifs("../WarframeItems/relics.json");
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    if (!doc.IsObject()) {
        std::cerr << "Expected root JSON to be an object.\n";
        return 1;
    }

    // Create a new document to hold the array
    rapidjson::Document newDoc;
    newDoc.SetArray();

    // Need an allocator to create new values
    rapidjson::Document::AllocatorType& allocator = newDoc.GetAllocator();

    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        rapidjson::Value relic(rapidjson::kObjectType);

        relic.AddMember("name", rapidjson::Value(it->name, allocator), allocator);
        relic.AddMember("data", rapidjson::Value(it->value, allocator), allocator);

        newDoc.PushBack(relic, allocator);
    }
    for (auto& v : newDoc.GetArray()) {
        std::cout << "Relic name: " << v["name"].GetString() << "\n";
    }
    std::cout << "Count: " << doc.MemberCount() << "\n";
    
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"YourWarframeDatabase", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

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

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    unsigned char* rgba_data = stbi_load_from_memory(image_data, sizeof(image_data), &image_width, &image_height, &channels, 4);

    ID3D11ShaderResourceView* myTexture = nullptr;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;


    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = rgba_data;
    subResource.SysMemPitch = image_width * 4;

    ID3D11Texture2D* pTexture = nullptr;
    HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    if (SUCCEEDED(hr))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &myTexture);
        pTexture->Release();
    }

    stbi_image_free(rgba_data);

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

        

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGuiIO io = ImGui::GetIO();

            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);                          // Create a window called "Hello, world!" and append into it.

            ImGui::Image((ImTextureID)myTexture, ImVec2(image_width/8, image_height/8));
            ImGui::SameLine(); ImGui::SetCursorPosY(ImGui::GetCursorPosY()+30); ImGui::Text("Welcome to YOUR Warframe Database!");               // Display some text (you can use a format strings too)
            /*ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 202, 6));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(201, 162, 6));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(245, 211, 73));
            ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0,0,0));
            ImGui::Button("Relic"); ImGui::SameLine(); ImGui::Button("Prime Parts");
            ImGui::PopStyleColor(4);*/
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("diffTabs", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Relics"))
                {
                    //static int item_selected_idx = 0;
                    //static bool item_highlight = false;
                    //int item_highlighted_idx = -1;

                    //if (ImGui::BeginListBox("listbox 1"))
                    //{
                    //    for (int n = 0; n < doc.MemberCount(); n++)
                    //    {
                    //        auto& relic = newDoc[n];
                    //        const bool is_selected = (item_selected_idx == n);
                    //        if (ImGui::Selectable(relic["name"].GetString(), is_selected))
                    //        {
                    //            item_selected_idx = n;
                    //        }
                    //        if (item_highlight && ImGui::IsItemHovered())
                    //            item_highlighted_idx = n;

                    //        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    //        if (is_selected)
                    //            ImGui::SetItemDefaultFocus();
                    //    }
                    //    ImGui::EndListBox();
                    //}

                    ImGuiTextFilter Filter;

                    rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>>* curRelic = NULL;

                    static int item_selected_idx_x = 0;
                    static int item_selected_idx_y = 0;
                    static bool item_highlight = false;
                    int item_highlighted_idx_x = -1;
                    int item_highlighted_idx_y = -1;
                    int n = 0;
                    
                    if (ImGui::BeginChild("##table", ImVec2(925, 0)))
                    {
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
                        ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                        if (ImGui::InputTextWithHint("##Filter", "Search (incl, -ecl)", Filter.InputBuf, IM_ARRAYSIZE(Filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
                            Filter.Build();
                        ImGui::PopItemFlag();

                        for (int y = 0; y < 537; y++)
                        {
                            

                            for (int x = 0; x < 5; x++)
                            {
                                if (x > 0)
                                    ImGui::SameLine();
                                    if (n == doc.MemberCount() - 2)
                                    {
                                        continue;
                                    }

                                
                                const bool is_selected = (item_selected_idx_x == x && item_selected_idx_y == y);
                                auto& relic = newDoc[n];
                                const char* curName = relic["name"].GetString();
                                
                                if (Filter.PassFilter(curName))
                                {

                                
                                    if (ImGui::Selectable(curName, is_selected, 0, ImVec2(175, 50)))
                                    {
                                        item_selected_idx_x = x;
                                        item_selected_idx_y = y;
                                    }
                                    if (item_highlight && ImGui::IsItemHovered())
                                    {
                                        item_highlighted_idx_x = x;
                                        item_highlighted_idx_y = y;

                                    }

                                    //Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_selected)
                                    { 
                                        ImGui::SetItemDefaultFocus();
                                        curRelic = &relic;
                                    }
    
                                }
                                n++;
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    
                    if (curRelic)
                    {
                        int counter = (*curRelic)["data"]["count"].GetInt();
                        ImGui::Text("%s", (*curRelic)["name"].GetString());
                        ImGui::Text("Farmable: %d", (*curRelic)["data"]["vaulted"].GetBool());
                        
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
                        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { counter--; (*curRelic)["data"]["count"] = rapidjson::Value(counter);}
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { counter++; (*curRelic)["data"]["count"] = rapidjson::Value(counter);
                        }
                        ImGui::PopItemFlag();
                        ImGui::SameLine();
                        ImGui::Text("Count: %d", counter);
                        ImGui::SameLine();

                    }
                    
                    ImGui::EndGroup();

                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Prime Parts"))
                {
                    ImGui::Text("bruh");
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }


    //// Step 3: Save back
    //std::ofstream ofs("relics.json");
    //if (!ofs.is_open()) {
    //    std::cerr << "Failed to open file for writing.\n";
    //    return 1;
    //}
    //rapidjson::OStreamWrapper osw(ofs);
    //rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
    //newDoc.Accept(writer);
    //ofs.close();


    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
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