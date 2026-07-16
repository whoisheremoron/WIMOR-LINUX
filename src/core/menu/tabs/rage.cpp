#include "../menu.hpp"

const char *antiAimTypes[] = {"None",        "Static",      "Jitter",
                              "Fake Jitter", "Real Jitter", "SpingBot (p100)"};

void Menu::drawRageTab() {
  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.45f, 0.95f, 1.00f));
  ImGui::Checkbox("Master Switch", &CONFIGBOOL("Rage>Enabled"));
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Ragebot aims and shoots at enemies automatically using advanced prediction.");
  
  ImGui::Spacing();

  float halfWidth = (ImGui::GetWindowContentRegionWidth() / 2.0f) - 6.0f;
  
  ImGui::BeginChild("Rage", ImVec2(halfWidth, 500), true);
  {
    ImGui::Spacing();
    ImGui::PushFont(fontMedium);
    ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.00f), "RageBot configuration");
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::BeginTabBar("Weapons Tabbar")) {
      if (ImGui::BeginTabItem("Default")) {
        ImGui::Spacing();
        ImGui::Checkbox("Resolver", &CONFIGBOOL("Rage>RageBot>Default>Resolver"));
        ImGui::Spacing();
        
        ImGui::Checkbox("Auto Shoot", &CONFIGBOOL("Rage>RageBot>Default>AutoShoot"));
        if (CONFIGBOOL("Rage>RageBot>Default>AutoShoot")) {
            ImGui::Indent(15.f);
            const char* autoShootModes[] = { "Hold", "Toggle" };
            ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - 15.f);
            ImGui::Combo("Auto Shoot Mode", &CONFIGINT("Rage>RageBot>Default>AutoShootMode"), autoShootModes, IM_ARRAYSIZE(autoShootModes));
            
            static bool toggled = false;
            Menu::CustomWidgets::drawKeyBinder("Auto Shoot Key", &CONFIGINT("Rage>RageBot>Default>AutoShootKey"), &toggled);
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }
        
        ImGui::Checkbox("Auto Peek", &CONFIGBOOL("Rage>RageBot>Default>AutoPeek"));
        if (CONFIGBOOL("Rage>RageBot>Default>AutoPeek")) {
            ImGui::Indent(15.f);
            static bool toggled = false;
            Menu::CustomWidgets::drawKeyBinder("Auto Peek Key", &CONFIGINT("Rage>RageBot>Default>AutoPeekKey"), &toggled);
            ImGui::SameLine();
            ImGui::ColorEdit4("##PeekColor", (float*)&CONFIGCOL("Rage>RageBot>Default>AutoPeekColor"), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_PickerHueWheel);
            ImGui::Unindent(15.f);
            ImGui::Spacing();
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "Target Hitboxes");
        ImGui::Separator();
        ImGui::Spacing();
        
        // Hitbox checkboxes using bitmask
        {
            int& hitboxes = CONFIGINT("Rage>RageBot>Default>Hitboxes");
            bool head = hitboxes & (int)HitBoxes::HEAD;
            bool neck = hitboxes & (int)HitBoxes::NECK;
            bool chest = hitboxes & (int)HitBoxes::CHEST;
            bool stomach = hitboxes & (int)HitBoxes::STOMACH;
            bool pelvis = hitboxes & (int)HitBoxes::PELVIS;
            
            if (ImGui::Checkbox("Head", &head)) { hitboxes ^= (int)HitBoxes::HEAD; } ImGui::SameLine(100.f);
            if (ImGui::Checkbox("Neck", &neck)) { hitboxes ^= (int)HitBoxes::NECK; } ImGui::SameLine(200.f);
            if (ImGui::Checkbox("Chest", &chest)) { hitboxes ^= (int)HitBoxes::CHEST; }
            
            if (ImGui::Checkbox("Stomach", &stomach)) { hitboxes ^= (int)HitBoxes::STOMACH; } ImGui::SameLine(100.f);
            if (ImGui::Checkbox("Pelvis", &pelvis)) { hitboxes ^= (int)HitBoxes::PELVIS; }
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "Accuracy & Targeting");
        ImGui::Separator();
        ImGui::Spacing();
        
        const char* targetModes[] = { "Crosshair", "Distance", "Highest Damage" };
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::Combo("Target Selection", &CONFIGINT("Rage>RageBot>Default>TargetSelection"), targetModes, IM_ARRAYSIZE(targetModes));
        ImGui::Spacing();
        
        ImGui::Checkbox("DM Mode", &CONFIGBOOL("Rage>RageBot>Default>DMMode"));
        ImGui::Checkbox("Force Baim if health < X", &CONFIGBOOL("Rage>RageBot>Default>ForceBaim"));
        if (CONFIGBOOL("Rage>RageBot>Default>ForceBaim")) {
          ImGui::Indent(15.f);
          ImGui::Text("Health threshold");
          ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - 15.f);
          ImGui::SliderInt("##HEALTH", &CONFIGINT("Rage>RageBot>Default>ForceBaimValue"), 1, 100);
          ImGui::Unindent(15.f);
          ImGui::Spacing();
        }
        
        ImGui::Spacing();
        ImGui::Text("Min Damage");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::SliderInt("##MINDMG", &CONFIGINT("Rage>RageBot>Default>MinDamage"), 1, 120);
        
        ImGui::Spacing();
        ImGui::Text("FOV (x10)");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::SliderInt("##FOV", &CONFIGINT("Rage>RageBot>Default>FOV"), 0, 1800);
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
    ImGui::EndChild();
  }
  
  ImGui::SameLine();
  
  ImGui::BeginChild("Anti-Aim", ImVec2(halfWidth, 500), true);
  {
    ImGui::Spacing();
    ImGui::PushFont(fontMedium);
    ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.00f), "Anti-Aim setup");
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Angles Type");
    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
    ImGui::Combo("##Type", &CONFIGINT("Rage>AntiAim>Type"), antiAimTypes, IM_ARRAYSIZE(antiAimTypes));
    ImGui::Spacing();

    if (CONFIGINT("Rage>AntiAim>Type")) {
      ImGui::Text("Pitch");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Pitch", &CONFIGINT("Rage>AntiAim>Pitch"), -89, 89);
      ImGui::Spacing();

      ImGui::Text("Yaw Offset");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Offset", &CONFIGINT("Rage>AntiAim>Offset"), 0, 360);
      ImGui::Spacing();

      ImGui::Text("FakeLag");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##FakeLag", &CONFIGINT("Rage>AntiAim>FakeLag"), 0, 16);
      ImGui::Spacing();
    }

    if (CONFIGINT("Rage>AntiAim>Type") == 1) { // Static
      ImGui::Text("Desync");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Desync", &CONFIGINT("Rage>AntiAim>Static>Desync"),
                       -60, 60);
    }

    if (CONFIGINT("Rage>AntiAim>Type") == 2) { // Jitter
      ImGui::Text("Desync");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Desync", &CONFIGINT("Rage>AntiAim>Jitter>Desync"),
                       -60, 60);

      ImGui::Text("Jitter Amount");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Jitter Amount",
                       &CONFIGINT("Rage>AntiAim>Jitter>Jitter Amount"), 0, 180);

      ImGui::Text("Jitter Delay");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Jitter Delay",
                       &CONFIGINT("Rage>AntiAim>Jitter>Jitter Delay"), 1, 256);

      ImGui::Checkbox("Random", &CONFIGBOOL("Rage>AntiAim>Jitter>Random"));
      if (CONFIGBOOL("Rage>AntiAim>Jitter>Random")) {
        ImGui::Text("Random Min");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::SliderInt("##Random Min",
                         &CONFIGINT("Rage>AntiAim>Jitter>Random Min"), 0, 180);

        ImGui::Text("Random Max");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::SliderInt("##Random Max",
                         &CONFIGINT("Rage>AntiAim>Jitter>Random Max"), 0, 180);
      }
    }

    if (CONFIGINT("Rage>AntiAim>Type") == 3) { // Fake Jitter
      ImGui::Text("Jitter Amount");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Jitter Amount",
                       &CONFIGINT("Rage>AntiAim>Fake Jitter>Jitter Amount"), 0,
                       60);

      ImGui::Text("Jitter Delay");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Jitter Delay",
                       &CONFIGINT("Rage>AntiAim>Fake Jitter>Jitter Delay"), 1,
                       256);

      ImGui::Checkbox("Random", &CONFIGBOOL("Rage>AntiAim>Fake Jitter>Random"));
      if (CONFIGBOOL("Rage>AntiAim>Fake Jitter>Random")) {
        ImGui::Text("Random Min");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::SliderInt("##Random Min",
                         &CONFIGINT("Rage>AntiAim>Fake Jitter>Random Min"), 0,
                         60);

        ImGui::Text("Random Max");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::SliderInt("##Random Max",
                         &CONFIGINT("Rage>AntiAim>Fake Jitter>Random Max"), 0,
                         60);
      }
    }

    if (CONFIGINT("Rage>AntiAim>Type") == 4) { // Real Jitter
      ImGui::Text("Jitter Amount");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Jitter Amount",
                       &CONFIGINT("Rage>AntiAim>Real Jitter>Jitter Amount"), 0,
                       60);

      ImGui::Text("Jitter Delay");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Jitter Delay",
                       &CONFIGINT("Rage>AntiAim>Real Jitter>Jitter Delay"), 1,
                       256);

      ImGui::Checkbox("Random", &CONFIGBOOL("Rage>AntiAim>Fake Jitter>Random"));
      if (CONFIGBOOL("Rage>AntiAim>Real Jitter>Random")) {
        ImGui::Text("Random Min");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::SliderInt("##Random Min",
                         &CONFIGINT("Rage>AntiAim>Real Jitter>Random Min"), 0,
                         60);

        ImGui::Text("Random Max");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::SliderInt("##Random Max",
                         &CONFIGINT("Rage>AntiAim>Real Jitter>Random Max"), 0,
                         60);
      }
    }

    if (CONFIGINT("Rage>AntiAim>Type") == 5) { // Sping
      ImGui::Text("Desync");
      ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
      ImGui::SliderInt("##Desync", &CONFIGINT("Rage>AntiAim>Spin>Desync"), -60,
                       60);
    }
    ImGui::Checkbox("Slow Walk", &CONFIGBOOL("Rage>AntiAim>Slow Walk"));
    ImGui::SliderInt("Slow Walk Speed (%)",
                     &CONFIGINT("Rage>AntiAim>Slow Walk Speed"), 0, 255);
    ImGui::EndChild();
  }
}