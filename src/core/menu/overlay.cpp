#include "../../includes.hpp"
#include <cstring>
#include <unistd.h>
#include <pwd.h>

// p100 flex ur distro
char distro[32];
void getDistro() {
    static bool gotDistro = false;
    if (!gotDistro) {
        std::ifstream osRelease("/etc/os-release");
        if (osRelease.is_open()) {
            std::string line;
            while (getline(osRelease, line)) {
                if (strstr(line.c_str(), "ID=") == line.c_str()) {
                    memcpy(distro, line.substr(3).c_str(), 32);
                }
            }
            osRelease.close();
        }
        gotDistro = true;
    }
}

void Menu::drawOverlay(ImDrawList* drawList) {
    Globals::drawList = drawList;

    if (introState != INTRO_DONE) {
        if (introState != INTRO_WAITING) {
            // Background dim
            drawList->AddRectFilled(ImVec2(0, 0), ImVec2(Globals::screenSizeX, Globals::screenSizeY), ImColor(10, 10, 10, (int)(introAlpha * 220.0f)));
            
            ImVec2 center = ImVec2(Globals::screenSizeX * 0.5f, Globals::screenSizeY * 0.5f);
            ImVec2 textPos = ImVec2(center.x, center.y + introSlide);
            
            ImGui::PushFont(fontLarge);
            std::string text = "WIMOR";
            ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
            
            // Subtly glow / Shadow
            for (int i = 1; i <= 4; i++) {
                drawList->AddText(fontLarge, 72.f, ImVec2(textPos.x - textSize.x * 0.5f - i, textPos.y - textSize.y * 0.5f - i), ImColor(0, 0, 0, (int)(introAlpha * 100.f)), text.c_str());
                drawList->AddText(fontLarge, 72.f, ImVec2(textPos.x - textSize.x * 0.5f + i, textPos.y - textSize.y * 0.5f + i), ImColor(0, 0, 0, (int)(introAlpha * 100.f)), text.c_str());
            }
            
            // Fatality purple/indigo gradient simulation
            ImColor wimorColor = ImColor(25, 1, 145, (int)(introAlpha * 255.f));
            drawList->AddText(fontLarge, 72.f, ImVec2(textPos.x - textSize.x * 0.5f, textPos.y - textSize.y * 0.5f), wimorColor, text.c_str());
            ImGui::PopFont();
            
            // Bar calculation
            float barWidth = 350.0f;
            float barHeight = 4.0f;
            ImVec2 barPos = ImVec2(textPos.x - barWidth * 0.5f, textPos.y + textSize.y * 0.5f + 25.0f);
            
            drawList->AddRectFilled(barPos, ImVec2(barPos.x + barWidth, barPos.y + barHeight), ImColor(30, 30, 30, (int)(introAlpha * 180.f)));
            
            float progress = 0.0f;
            if (introState == INTRO_FADE_IN) {
                progress = (introTimer / 0.8f) * 0.25f;
            } else if (introState == INTRO_HOLD) {
                progress = 0.25f + (introTimer / 1.5f) * 0.50f;
            } else if (introState == INTRO_FADE_OUT) {
                progress = 0.75f + (introTimer / 0.8f) * 0.25f;
            }
            progress = std::clamp(progress, 0.0f, 1.0f);
            
            drawList->AddRectFilled(barPos, ImVec2(barPos.x + barWidth * progress, barPos.y + barHeight), ImColor(25, 1, 145, (int)(introAlpha * 255.f)));
        }
        return; // Block other renders during intro
    }

    getDistro();
    char hostname[64];
    gethostname(hostname, 64);
    if(!CONFIGBOOL("Misc>Misc>Misc>Disable Watermark")) {
        char watermarkText[64];
        sprintf(watermarkText, "wimor (%s - %s@%s) | %.1f FPS | %i ms", distro, getpwuid(getuid())->pw_name, hostname, ImGui::GetIO().Framerate, (Interfaces::engine->IsInGame() && playerResource) ? playerResource->GetPing(Interfaces::engine->GetLocalPlayer()) : 0);
        // Hacky way to do black shadow but it works
        Globals::drawList->AddText(ImVec2(4, 4), ImColor(0, 0, 0, 255), watermarkText);
        Globals::drawList->AddText(ImVec2(3, 3), ImColor(255, 255, 255, 255), watermarkText);
    }

    if (CONFIGBOOL("Rage>Enabled") && CONFIGBOOL("Rage>RageBot>Default>AutoShoot")) {
        char autoShootText[32];
        int mode = CONFIGINT("Rage>RageBot>Default>AutoShootMode"); // 0 = Hold, 1 = Toggle
        sprintf(autoShootText, "AutoShoot: %s", mode == 1 ? (Features::RageBot::autoShootActive ? "TOGGLED" : "OFF") : (Features::RageBot::autoShootActive ? "HOLDING" : "OFF"));
        
        ImColor color = Features::RageBot::autoShootActive ? ImColor(0, 255, 0, 255) : ImColor(255, 0, 0, 255);
        Globals::drawList->AddText(ImVec2(4, 21), ImColor(0, 0, 0, 255), autoShootText);
        Globals::drawList->AddText(ImVec2(3, 20), color, autoShootText);
    }

    Features::ESP::draw();
    Features::RecoilCrosshair::draw();
    Features::Spectators::draw();
    Features::PlayerList::draw();
    Features::FlappyBird::draw();
    Features::Notifications::draw();
    Features::Hitmarkers::draw();

    Features::Movement::draw();
}
