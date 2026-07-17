#include "../menu.hpp"
#include "../config.hpp"

void Menu::drawMovementTab() {
    ImGui::Spacing();
    
    ImGui::Columns(2, "movement_columns", false);
    ImGui::SetColumnWidth(0, ImGui::GetWindowContentRegionWidth() * 0.50f);
    ImGui::SetColumnWidth(1, ImGui::GetWindowContentRegionWidth() * 0.50f);

    // Left Column: Core Movement & Bhop
    ImGui::BeginChild("Core Movement", ImVec2(0, 500), true); {
        ImGui::Spacing();
        ImGui::PushFont(fontMedium);
        ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.00f), "Core & Bhop");
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Checkbox("Auto Hop", &CONFIGBOOL("Misc>Misc>Movement>Auto Hop"));
        ImGui::Spacing();
        
        ImGui::Checkbox("Humanised Bhop", &CONFIGBOOL("Misc>Misc>Movement>Humanised Bhop"));
        if (CONFIGBOOL("Misc>Misc>Movement>Humanised Bhop")) {
            ImGui::Indent(15.f);
            ImGui::Text("Bhop Hitchance");
            ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - 15.f);
            ImGui::SliderInt("##Bhop Hitchance", &CONFIGINT("Misc>Misc>Movement>Bhop Hitchance"), 0, 100);
            ImGui::Text("Bhop Max Misses");
            ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - 15.f);
            ImGui::SliderInt("##Bhop Max Misses", &CONFIGINT("Misc>Misc>Movement>Bhop Max Misses"), 0, 16);
            ImGui::Text("Bhop Max Hops Hit");
            ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - 15.f);
            ImGui::SliderInt("##Bhop Max Hops Hit", &CONFIGINT("Misc>Misc>Movement>Bhop Max Hops Hit"), 0, 16);
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }
        
        ImGui::Checkbox("Fast Duck", &CONFIGBOOL("Misc>Misc>Movement>Fast Duck"));
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("May cause untrusted, use at own risk!");
        ImGui::Spacing();

        ImGui::Checkbox("Auto Counter-Strafe", &CONFIGBOOL("Misc>Misc>Movement>Fast Stop"));
        ImGui::Spacing();
        
        ImGui::Checkbox("Speed Indicator", &CONFIGBOOL("Misc>Misc>Movement>Speed Indicator"));
        
        ImGui::EndChild();
    }

    ImGui::NextColumn();

    // Right Column: Advanced tricks (EdgeBug, JumpBug, Autostrafes)
    ImGui::BeginChild("Tricks & Strafing", ImVec2(0, 500), true); {
        ImGui::Spacing();
        ImGui::PushFont(fontMedium);
        ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.00f), "Tricks & Strafing");
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Checkbox("Pixel Surf", &CONFIGBOOL("Misc>Misc>Movement>Pixel Surf"));
        ImGui::Spacing();

        ImGui::Checkbox("EdgeBug", &CONFIGBOOL("Misc>Misc>Movement>EdgeBug"));
        if (CONFIGBOOL("Misc>Misc>Movement>EdgeBug")) {
            ImGui::Indent(15.f);
            static bool toggled = false;
            Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Movement>EdgeBug Key"), &toggled);
            
            ImGui::Checkbox("EdgeBug Highlight", &CONFIGBOOL("Misc>Misc>Movement>EdgeBug Highlight"));
            if (CONFIGBOOL("Misc>Misc>Movement>EdgeBug Highlight")) {
                ImGui::Indent(15.f);
                ImGui::Checkbox("Always Show", &CONFIGBOOL("Misc>Misc>Movement>EdgeBug Highlight Always"));
                ImGui::Unindent(15.f);
            }
            ImGui::Checkbox("EdgeBug Finder", &CONFIGBOOL("Misc>Misc>Movement>EdgeBug Finder"));
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }

        ImGui::Checkbox("JumpBug", &CONFIGBOOL("Misc>Misc>Movement>JumpBug"));
        if (CONFIGBOOL("Misc>Misc>Movement>JumpBug")) {
            ImGui::Indent(15.f);
            static bool toggled = false;
            Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Movement>JumpBug Key"), &toggled);
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }

        ImGui::Checkbox("Edge Jump", &CONFIGBOOL("Misc>Misc>Movement>Edge Jump"));
        if (CONFIGBOOL("Misc>Misc>Movement>Edge Jump")) {
            ImGui::Indent(15.f);
            static bool toggled = false;
            Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Movement>Edge Jump Key"), &toggled);
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }

        ImGui::Checkbox("Auto Strafe", &CONFIGBOOL("Misc>Misc>Movement>Auto Strafe"));
        if (CONFIGBOOL("Misc>Misc>Movement>Auto Strafe")) {
            ImGui::Indent(15.f);
            static bool toggled = false;
            Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Movement>Auto Strafe Key"), &toggled);
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }

        ImGui::Checkbox("Auto Strafe Right (Sideways)", &CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Right"));
        if (CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Right")) {
            ImGui::Indent(15.f);
            static bool toggled = false;
            Menu::CustomWidgets::drawKeyBinder("Right Key", &CONFIGINT("Misc>Misc>Movement>Auto Strafe Right Key"), &toggled);
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }

        ImGui::Checkbox("Auto Strafe Left (Sideways)", &CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Left"));
        if (CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Left")) {
            ImGui::Indent(15.f);
            static bool toggled = false;
            Menu::CustomWidgets::drawKeyBinder("Left Key", &CONFIGINT("Misc>Misc>Movement>Auto Strafe Left Key"), &toggled);
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }

        ImGui::EndChild();
    }

    ImGui::Columns(1);
}
