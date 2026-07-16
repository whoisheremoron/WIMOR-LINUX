#pragma once
#include "../../includes.hpp"
#include "imgui/imgui.h"
#include <cstdint>
#include <filesystem>
#include <string>

namespace Menu {
    inline bool open = false;
    inline bool targetOpen = false;
    inline float menuAlpha = 0.0f;
    inline bool initialised = false;
    inline int tabSelected = 0; // 0=Rage, 1=Legit, 2=Visuals, 3=Movement, 4=Config/Misc

    inline bool devWindow = false;
    inline bool demoWindow = false;

    // Intro animation state
    enum IntroState { INTRO_WAITING, INTRO_FADE_IN, INTRO_HOLD, INTRO_FADE_OUT, INTRO_DONE };
    inline IntroState introState = INTRO_WAITING;
    inline float introTimer = 0.0f;
    inline float introAlpha = 0.0f;
    inline float introSlide = 0.0f;

    // Fonts
    inline ImFont* fontLarge = nullptr;  // 36px for intro
    inline ImFont* fontMedium = nullptr; // 18px for sidebar labels
    inline ImFont* fontDefault = nullptr; // 14px default

    inline char clantag[128] = "LINUX | WIMOR ";
    void drawMenu();
    void drawLegitTab();
    void drawRageTab();
    void drawVisualsTab();
    void drawMovementTab();
    void drawMiscTab();
    void drawDevWindow();
    void drawOverlay(ImDrawList* drawList);
    void onPollEvent(SDL_Event* event, const int result);
    void onSwapWindow(SDL_Window* window);
    
    namespace CustomWidgets {
        void drawKeyBinder(const char* label, int* key, bool* toggled);
        bool isKeyDown(int key);
    }
}