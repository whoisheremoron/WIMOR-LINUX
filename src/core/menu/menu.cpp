#include "menu.hpp"
#include "imgui/imgui.h"
#include "roboto.hpp"

void style() {
    ImVec4* colors = ImGui::GetStyle().Colors;
    
    // Core Backgrounds
    colors[ImGuiCol_WindowBg]               = ImVec4(0.045f, 0.045f, 0.065f, 1.00f); // Deep dark blue-grey
    colors[ImGuiCol_ChildBg]                = ImVec4(0.075f, 0.075f, 0.105f, 1.00f); // Slightly lighter for card styling
    colors[ImGuiCol_PopupBg]                = ImVec4(0.075f, 0.075f, 0.105f, 0.96f);
    
    // Borders & Shadows
    colors[ImGuiCol_Border]                 = ImVec4(0.140f, 0.140f, 0.190f, 0.65f); // Soft blue-grey border
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);
    
    // Text
    colors[ImGuiCol_Text]                   = ImVec4(0.950f, 0.950f, 0.980f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.500f, 0.500f, 0.600f, 1.00f);
    
    // Headers & Navigation
    colors[ImGuiCol_Header]                 = ImVec4(0.120f, 0.120f, 0.220f, 0.50f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.160f, 0.160f, 0.320f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.200f, 0.200f, 0.450f, 1.00f);

    // Frame Elements (Checkboxes, Inputs, Sliders)
    colors[ImGuiCol_FrameBg]                = ImVec4(0.110f, 0.110f, 0.160f, 1.00f); // Clean input field bg
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.150f, 0.150f, 0.240f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.180f, 0.180f, 0.300f, 1.00f);
    
    // Buttons
    colors[ImGuiCol_Button]                 = ImVec4(0.120f, 0.120f, 0.200f, 0.80f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.180f, 0.180f, 0.320f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.250f, 0.250f, 0.450f, 1.00f);
    
    // Active Highlights (Checkmarks, Sliders, Tabs)
    colors[ImGuiCol_CheckMark]              = ImVec4(0.350f, 0.450f, 0.950f, 1.00f); // Soft bright blue
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.350f, 0.450f, 0.950f, 0.90f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.450f, 0.550f, 1.00f, 1.00f);
    
    colors[ImGuiCol_Tab]                    = ImVec4(0.110f, 0.110f, 0.200f, 0.60f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.180f, 0.180f, 0.320f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.250f, 0.250f, 0.450f, 1.00f);
    
    colors[ImGuiCol_Separator]              = ImVec4(0.140f, 0.140f, 0.190f, 0.65f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.350f, 0.450f, 0.950f, 0.80f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.350f, 0.450f, 0.950f, 1.00f);

    // Style Properties
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.f;
    style.ChildRounding = 8.f;
    style.FrameRounding = 6.f;
    style.PopupRounding = 8.f;
    style.ScrollbarRounding = 12.f;
    style.GrabRounding = 6.f;
    style.TabRounding = 6.f;
    
    style.WindowPadding = ImVec2(12.f, 12.f);
    style.FramePadding = ImVec2(10.f, 6.f);
    style.ItemSpacing = ImVec2(8.f, 12.f);
    style.ItemInnerSpacing = ImVec2(8.f, 8.f);
    style.ScrollbarSize = 6.f;
}

void Menu::onPollEvent(SDL_Event* event, const int result) {
    if (result && ImGui_ImplSDL2_ProcessEvent(event) && Menu::open && Menu::introState == Menu::INTRO_DONE) {
        event->type = 0;
    }
}

void Menu::onSwapWindow(SDL_Window* window) {
    if (!initialised) {
        gl3wInit();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 100");
        ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
        style();
        
        fontDefault = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Roboto_compressed_data, Roboto_compressed_size, 14.f);
        fontMedium = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Roboto_compressed_data, Roboto_compressed_size, 18.f);
        fontLarge = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Roboto_compressed_data, Roboto_compressed_size, 72.f);
        
        initialised = true;
        
        // Start the intro animation
        introState = INTRO_WAITING;
        introTimer = 0.0f;
        introAlpha = 0.0f;
        introSlide = -40.0f;
        
        open = false;
        targetOpen = false;
        menuAlpha = 0.0f;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);

    ImGuiIO& io = ImGui::GetIO();
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    Globals::screenSizeX = w;
    Globals::screenSizeY = h;
    io.DisplaySize = ImVec2((float)w, (float)h);

    ImGui::NewFrame();

    // Handle Intro state machine updates
    float dt = io.DeltaTime;
    if (introState != INTRO_DONE) {
        introTimer += dt;
        if (introState == INTRO_WAITING) {
            if (introTimer >= 2.0f) {
                introState = INTRO_FADE_IN;
                introTimer = 0.0f;
            }
        } else if (introState == INTRO_FADE_IN) {
            float progress = std::clamp(introTimer / 0.8f, 0.0f, 1.0f);
            introAlpha = progress;
            introSlide = -40.0f * (1.0f - progress); // slide up to 0.0f
            if (introTimer >= 0.8f) {
                introState = INTRO_HOLD;
                introTimer = 0.0f;
            }
        } else if (introState == INTRO_HOLD) {
            introAlpha = 1.0f;
            introSlide = 0.0f;
            if (introTimer >= 1.5f) {
                introState = INTRO_FADE_OUT;
                introTimer = 0.0f;
            }
        } else if (introState == INTRO_FADE_OUT) {
            float progress = std::clamp(introTimer / 0.8f, 0.0f, 1.0f);
            introAlpha = 1.0f - progress;
            introSlide = 40.0f * progress; // slide down/away
            if (introTimer >= 0.8f) {
                introState = INTRO_DONE;
                targetOpen = true; // Auto open menu after intro
            }
        }
    }

    // Handle smooth menu open/close fade
    if (introState == INTRO_DONE) {
        if (targetOpen) {
            menuAlpha = std::clamp(menuAlpha + dt * 6.0f, 0.0f, 1.0f);
        } else {
            menuAlpha = std::clamp(menuAlpha - dt * 6.0f, 0.0f, 1.0f);
        }
        open = (menuAlpha > 0.0f);
    }

    Menu::drawOverlay(ImGui::GetBackgroundDrawList());
    
    if (Menu::open) {
        io.MouseDrawCursor = (menuAlpha > 0.1f); // Only show cursor when menu is mostly visible
        Menu::drawMenu();
        if (devWindow) {
            drawDevWindow();
        }
        if (demoWindow) {
            ImGui::ShowDemoWindow();
        }
    }
    else {
        io.MouseDrawCursor = false;
    }

    if (ImGui::IsKeyPressed(SDL_SCANCODE_INSERT, false) && Menu::introState == Menu::INTRO_DONE) {
        Config::reloadCfgList();
        Menu::targetOpen = !Menu::targetOpen;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Menu::drawMenu() {
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menuAlpha);
    ImGui::SetNextWindowSize(ImVec2{900, 580});
    ImGui::Begin("wimor-linux", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // Sidebar structure: Width 95px, floating card layout
    ImGui::BeginChild("##Sidebar", ImVec2(95, 556), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    
    // Draw top branding (Centered properly)
    ImGui::Spacing();
    ImGui::PushFont(fontMedium);
    float textWidth = ImGui::CalcTextSize("WIMOR").x;
    ImGui::SetCursorPosX((95.f - textWidth) * 0.5f);
    ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.0f), "W"); ImGui::SameLine(0, 0);
    ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.98f, 1.0f), "IMOR");
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    struct SidebarItem {
        const char* label;
        const char* subLabel;
        ImColor color;
    } items[] = {
        { "R", "Rage", ImColor(255, 60, 60) },
        { "L", "Legit", ImColor(255, 215, 0) },
        { "VIS", "Visuals", ImColor(51, 153, 255) },
        { "MV", "Motion", ImColor(51, 255, 102) },
        { "CFG", "Misc", ImColor(200, 200, 200) }
    };

    for (int i = 0; i < 5; i++) {
        ImGui::PushID(i);
        
        bool isSelected = (tabSelected == i);
        
        // Highlight background for selected or hovered item
        ImGui::PushStyleColor(ImGuiCol_Button, isSelected ? ImVec4(0.35f, 0.45f, 0.95f, 0.18f) : ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.45f, 0.95f, 0.08f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.45f, 0.95f, 0.25f));

        // Floating rounded button inside sidebar (giving it 6px margin left and right)
        ImGui::SetCursorPosX(6.0f);
        if (ImGui::Button("##SidebarButton", ImVec2(83, 62))) {
            tabSelected = i;
        }

        // Draw custom colored label centered inside the button area
        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectMax = ImGui::GetItemRectMax();
        ImVec2 center = ImVec2((rectMin.x + rectMax.x) * 0.5f, (rectMin.y + rectMax.y) * 0.5f);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Selected indicator bar on left (curved neatly inside the floating button margins)
        if (isSelected) {
            drawList->AddRectFilled(ImVec2(rectMin.x + 3.0f, rectMin.y + 8), ImVec2(rectMin.x + 6.0f, rectMax.y - 8), items[i].color, 3.f);
        }

        // Colored icon text
        ImGui::PushFont(fontMedium);
        ImVec2 textSize = ImGui::CalcTextSize(items[i].label);
        drawList->AddText(ImVec2(center.x - textSize.x * 0.5f, center.y - 12.f), items[i].color, items[i].label);
        ImGui::PopFont();

        // Sublabel text
        ImVec2 subSize = ImGui::CalcTextSize(items[i].subLabel);
        drawList->AddText(ImVec2(center.x - subSize.x * 0.5f, center.y + 10.f), isSelected ? ImColor(240, 240, 250, 255) : ImColor(130, 130, 150, 255), items[i].subLabel);

        ImGui::PopStyleColor(3);
        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::EndChild();

    ImGui::SameLine(0, 12.f); // Mellow padding between sidebar and content

    // Content area structure: Width 781px, Height 100% of window
    ImGui::BeginChild("##ContentArea", ImVec2(781, 556), false);
    ImGui::SetCursorPosY(4.f);

    switch(tabSelected) {
        case 0: {
            Menu::drawRageTab(); break;
        }
        case 1: {
            Menu::drawLegitTab(); break;
        }
        case 2: {
            Menu::drawVisualsTab(); break;
        }
        case 3: {
            Menu::drawMovementTab(); break;
        }
        case 4: {
            Menu::drawMiscTab(); break;
        }
    }

    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar();
}