#include "../menu.hpp"
#include <filesystem>
#include <vector>
#include "../config.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <thread>
#include <chrono>
#include "../../hooks/hooks.hpp"
#include "../../../sdk/interfaces/interfaces.hpp"

static void selfUnload() {
    Globals::unloading = true; // Instantly bypass all hooks to prevent execution during unmapping

    std::thread([]() {
        // Wait 300ms for active hook execution calls to return safely
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        Hooks::unload();
        Interfaces::unload();
        
        // Wait another 300ms for safety
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        void* handle = dlopen("libwimor.so", RTLD_NOLOAD | RTLD_LAZY);
        if (!handle) return;
        
        void* dlclose_fn = dlsym(RTLD_DEFAULT, "dlclose");
        void* pthread_exit_fn = dlsym(RTLD_DEFAULT, "pthread_exit");
        
        if (!dlclose_fn || !pthread_exit_fn) return;

        void* mem = mmap(NULL, 128, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mem == MAP_FAILED) return;

        unsigned char stub[] = {
            0x48, 0x89, 0xf8,       // mov %rdi, %rax
            0x48, 0x89, 0xf7,       // mov %rsi, %rdi
            0xff, 0xd0,             // call *%rax
            0x48, 0x89, 0xd0,       // mov %rdx, %rax
            0x48, 0x31, 0xff,       // xor %rdi, %rdi
            0xff, 0xd0              // call *%rax
        };
        
        memcpy(mem, stub, sizeof(stub));
        
        typedef void (*stub_t)(void* dlclose_fn, void* handle, void* pthread_exit_fn);
        stub_t run_stub = (stub_t)mem;
        
        run_stub(dlclose_fn, handle, pthread_exit_fn);
    }).detach();
}

struct PaintKitInfo {
    int id;
    const char* name;
};

static const std::vector<PaintKitInfo> g_skins = {
    {0, "Default"},
    // AK-47
    {302, "Vulcan"},
    {282, "Redline"},
    {44, "Case Hardened"},
    {639, "Asiimov"},
    {600, "Neon Revolution"},
    {490, "Frontside Misty"},
    {675, "Empress"},
    {380, "Wasteland Rebel"},
    {524, "Fuel Injector"},
    {635, "Bloodsport"},
    {180, "Fire Serpent"},
    {724, "Wild Lotus"},
    {1073, "Gold Arabesque"},
    {1143, "Head Shot"},
    
    // M4A4
    {309, "Howl"},
    {255, "Asiimov"},
    {695, "Neo-Noir"},
    {664, "Hellfire"},
    {632, "Buzz Kill"},
    {155, "Bullet Rain"},
    {588, "Desolate Space"},
    {486, "Evil Daimyo"},
    {844, "Emperor"},
    {1141, "Temukau"},
    
    // M4A1-S
    {430, "Hyper Beast"},
    {644, "Decimator"},
    {548, "Chantico's Fire"},
    {497, "Golden Coil"},
    {440, "Icarus Fell"},
    {33, "Hot Rod"},
    {984, "Printstream"},
    {1001, "Welcome to the Jungle"},
    {326, "Knight"},
    {1112, "Emphorosaur-S"},
    
    // AWP
    {344, "Dragon Lore"},
    {446, "Medusa"},
    {756, "Gungnir"},
    {279, "Asiimov"},
    {445, "Hyper Beast"},
    {212, "Graphite"},
    {662, "Oni Taiji"},
    {627, "Fever Dream"},
    {838, "Atheris"},
    {917, "Wildfire"},
    {803, "Neo-Noir"},
    {1026, "Fade"},
    
    // Desert Eagle
    {37, "Blaze"},
    {961, "Printstream"},
    {711, "Code Red"},
    {185, "Golden Koi"},
    {1096, "Fennec Fox"},
    {599, "Mecha Industries"},
    {351, "Conspiracy"},
    
    // USP-S
    {504, "Kill Confirmed"},
    {653, "Neo-Noir"},
    {313, "Orion"},
    {1037, "Printstream"},
    {360, "Cyrex"},
    {705, "Cortex"},
    {1004, "Monster Mashup"},
    
    // Glock-18
    {38, "Fade"},
    {353, "Water Elemental"},
    {957, "Neo-Noir"},
    {918, "Bullet Queen"},
    {586, "Wasteland Rebel"},
    
    // Knives & General Fades/Crimson Webs
    {12, "Crimson Web"},
    {27, "Bone Mask"},
    {38, "Fade"},
    {40, "Night"},
    {42, "Blue Steel"},
    {43, "Stained"},
    {44, "Case Hardened"},
    {59, "Slaughter"},
    {72, "Safari Mesh"},
    {77, "Boreal Forest"},
    {98, "Ultraviolet"},
    {143, "Hunting Blind"},
    {323, "Lapis Gator"},
    {413, "Marble Fade"},
    {414, "Doppler"},
    {415, "Ruby"},
    {416, "Sapphire"},
    {417, "Black Pearl"},
    {568, "Emerald"},
    {569, "Lore"},
    {570, "Black Laminate"},
    {571, "Autotronic"},
    {572, "Bright Water"},
    {983, "Gold"},
};

void Menu::drawMiscTab() {
    if (ImGui::BeginTabBar("##miscTabs")) {
        if (ImGui::BeginTabItem("Misc")) {
            ImGui::Spacing();
            ImGui::Columns(2, NULL, false);
            ImGui::SetColumnWidth(-1, ImGui::GetWindowContentRegionWidth() * 0.58f);

            // Left Column
            ImGui::BeginChild("Misc", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.57f, ImGui::GetWindowHeight() * 0.55f), true); {
                ImGui::Spacing();
                ImGui::PushFont(fontMedium);
                ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.00f), "Miscellaneous Settings");
                ImGui::PopFont();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Checkbox("Developer window", &devWindow);
                ImGui::Checkbox("Disable Watermark", &CONFIGBOOL("Misc>Misc>Misc>Disable Watermark"));
                ImGui::Checkbox("Force square radar", &CONFIGBOOL("Misc>Misc>Misc>Force square radar"));
                ImGui::Checkbox("Rank Revealer", &CONFIGBOOL("Misc>Misc>Misc>Rank Revealer"));
                ImGui::Checkbox("Spectators", &CONFIGBOOL("Misc>Misc>Misc>Spectators"));
                
                ImGui::Checkbox("Player List", &CONFIGBOOL("Misc>Misc>Misc>Player List"));
                if (CONFIGBOOL("Misc>Misc>Misc>Player List")) {
                    ImGui::Indent(15.f);
                    ImGui::Checkbox("Only When Menu Open", &CONFIGBOOL("Misc>Misc>Misc>Player List Only When Menu Open"));
                    ImGui::Unindent(15.f);
                }
                
                ImGui::Checkbox("Flappy Birb", &CONFIGBOOL("Misc>Misc>Misc>Flappy Birb"));
                ImGui::Checkbox("Auto Accept", &CONFIGBOOL("Misc>Misc>Misc>Auto Accept"));
                
                ImGui::Checkbox("Auto Defuse", &CONFIGBOOL("Misc>Misc>Misc>Auto Defuse"));
                if (CONFIGBOOL("Misc>Misc>Misc>Auto Defuse")) {
                    ImGui::Indent(15.f);
                    ImGui::Checkbox("Latest Defuse", &CONFIGBOOL("Misc>Misc>Misc>Latest Defuse"));
                    ImGui::Unindent(15.f);
                }
                
                ImGui::Checkbox("Chat Filter Bypass", &CONFIGBOOL("Misc>Misc>Misc>Chat Filter Bypass"));
                if (CONFIGBOOL("Misc>Misc>Misc>Use Spam")) {
                    ImGui::Indent(15.f);
                    static bool toggled = false;
                    Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Misc>Use Spam Key"), &toggled);
                    ImGui::Unindent(15.f);
                }
                ImGui::Checkbox("Use Spam", &CONFIGBOOL("Misc>Misc>Misc>Use Spam"));
                ImGui::Checkbox("Disable Setting Cvars", &CONFIGBOOL("Misc>Misc>Misc>Disable Setting Cvars"));
                ImGui::Checkbox("Disable Post Processing", &CONFIGBOOL("Misc>Misc>Misc>Disable Post Processing"));
                ImGui::Checkbox("Now Playing", &CONFIGBOOL("Misc>Misc>Misc>Now Playing"));
                ImGui::EndChild();
            }

            ImGui::BeginChild("Hitmarkers", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.57f, ImGui::GetWindowHeight() * 0.28f), true); {
                ImGui::Spacing();
                ImGui::PushFont(fontMedium);
                ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.00f), "Hitmarkers & Logs");
                ImGui::PopFont();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Checkbox("Hitlogs", &CONFIGBOOL("Misc>Misc>Hitmarkers>Hitlogs"));
                ImGui::Checkbox("Hitmarkers", &CONFIGBOOL("Misc>Misc>Hitmarkers>Hitmarkers"));
                ImGui::Checkbox("Hitsound", &CONFIGBOOL("Misc>Misc>Hitmarkers>Hitsound"));
                ImGui::Checkbox("Damage Markers", &CONFIGBOOL("Misc>Misc>Hitmarkers>Damage Markers"));
                ImGui::EndChild();
            }

            ImGui::NextColumn();

            // Right Column
            ImGui::BeginChild("Config", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.41f, ImGui::GetWindowHeight() * 0.50f), true); {
                ImGui::Spacing();
                ImGui::PushFont(fontMedium);
                ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.00f), "Configurations");
                ImGui::PopFont();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                ImGui::ListBoxHeader("##configlist-lbox", ImVec2(0, 130));
                for (std::string file : Config::cfgFiles) {
                    if (ImGui::Button(file.c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), 22))) {
                        strcpy(Config::configFileName, file.c_str());
                    }
                }
                ImGui::ListBoxFooter();
                ImGui::Spacing();

                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                ImGui::InputText("##configfile-tbox", Config::configFileName, IM_ARRAYSIZE(Config::configFileName));
                ImGui::Spacing();

                float btnWidth = (ImGui::GetWindowContentRegionWidth() - 12.f) / 3.f;
                if (ImGui::Button("Save", ImVec2(btnWidth, 26))) {
                    Config::save();
                } ImGui::SameLine(0, 6.f);
                if (ImGui::Button("Load", ImVec2(btnWidth, 26))) {
                    Config::load();
                    FULLUPDATE();
                } ImGui::SameLine(0, 6.f);
                if (ImGui::Button("Remove", ImVec2(btnWidth, 26))) {
                    Config::remove();
                }

                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.15f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.25f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.05f, 0.05f, 1.0f));
                if (ImGui::Button("Unload Cheat", ImVec2(ImGui::GetWindowContentRegionWidth(), 30))) {
                    selfUnload();
                }
                ImGui::PopStyleColor(3);

                ImGui::EndChild();
            }

            ImGui::BeginChild("Clantag", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.41f, ImGui::GetWindowHeight() * 0.33f), true); {
                ImGui::Spacing();
                ImGui::PushFont(fontMedium);
                ImGui::TextColored(ImVec4(0.35f, 0.45f, 0.95f, 1.00f), "Clantag Options");
                ImGui::PopFont();
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                ImGui::InputText("##clantag-tbox", clantag, IM_ARRAYSIZE(clantag));
                ImGui::Spacing();
                
                ImGui::Checkbox("Clantag", &CONFIGBOOL("Misc>Misc>Clantag>Clantag"));
                ImGui::Checkbox("Marquee", &CONFIGBOOL("Misc>Misc>Clantag>Clantag Marquee"));
                ImGui::Checkbox("Bee Movie Clantag", &CONFIGBOOL("Misc>Misc>Clantag>Bee Movie Clantag"));
                ImGui::EndChild();
            }

            ImGui::Columns(1);
            ImGui::Spacing();
            ImGui::TextDisabled("Credits!");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("sekc (of course)\nwhoisheremoron\nAll other contributors on GitHub (xXx-sans-xXx, luk1337, cristeigabriel, crazily, dave (@dweee), keenan, u2ooS, tango1337, nigma1337, vampur, allbombson, jovvik, and any other contributors)");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Skins")) {
            static ItemIndex curWeaponSelected = ItemIndex::WEAPON_AK47;
            
            ImGui::Columns(2, "skinchanger_columns", false);
            ImGui::SetColumnWidth(0, ImGui::GetWindowContentRegionWidth() * 0.35f);
            ImGui::SetColumnWidth(1, ImGui::GetWindowContentRegionWidth() * 0.65f);
            
            // Left column: Weapon List
            ImGui::BeginChild("Weapon Selection", ImVec2(0, 0), true); {
                ImGui::Text("Weapons");
                ImGui::Separator();
                for (auto item : itemIndexMap) {
                    if (item.first != ItemIndex::INVALID) {
                        const bool is_selected = (curWeaponSelected == item.first);
                        if (ImGui::Selectable(item.second, is_selected)) {
                            curWeaponSelected = item.first;
                        }
                    }
                }
                ImGui::EndChild();
            }
            
            ImGui::NextColumn();
            
            // Right column: Settings for selected weapon
            ImGui::BeginChild("Skin Config", ImVec2(0, 0), true); {
                if (curWeaponSelected != ItemIndex::INVALID) {
                    ImGui::Text("Settings for %s", itemIndexMap.at(curWeaponSelected));
                    ImGui::Separator();
                    
                    bool isKnifeSelected = (curWeaponSelected == ItemIndex::WEAPON_KNIFE || 
                                            curWeaponSelected == ItemIndex::WEAPON_KNIFE_T ||
                                            (curWeaponSelected >= ItemIndex::WEAPON_KNIFE_BAYONET && curWeaponSelected <= ItemIndex::WEAPON_KNIFE_GHOST));
                    
                    const char* weaponName = isKnifeSelected ? "Knife" : itemIndexMap.at(curWeaponSelected);
                    
                    char buf[256];
                    snprintf(buf, 256, "Misc>Skins>Skins>%s>PaintKit", weaponName);
                    
                    char buf2[256];
                    snprintf(buf2, 256, "Misc>Skins>Skins>%s>Wear", weaponName);
                    
                    char buf3[256];
                    snprintf(buf3, 256, "Misc>Skins>Skins>%s>StatTrack", weaponName);
                    
                    // Popular skins combo
                    int currentPaintKit = CONFIGINT(buf);
                    const char* previewName = "Custom / Unknown";
                    for (const auto& skin : g_skins) {
                        if (skin.id == currentPaintKit) {
                            previewName = skin.name;
                            break;
                        }
                    }
                    
                    if (ImGui::BeginCombo("Select Skin Preset", previewName)) {
                        for (const auto& skin : g_skins) {
                            const bool is_selected = (currentPaintKit == skin.id);
                            if (ImGui::Selectable(skin.name, is_selected)) {
                                CONFIGINT(buf) = skin.id;
                            }
                        }
                        ImGui::EndCombo();
                    }
                    
                    // Raw Paintkit ID
                    ImGui::InputInt("PaintKit ID", &CONFIGINT(buf));
                    
                    // Wear Slider
                    ImGui::SliderInt("Wear", &CONFIGINT(buf2), 0, 100);
                    
                    // StatTrak
                    ImGui::InputInt("StatTrak", &CONFIGINT(buf3));
                    
                    if (isKnifeSelected) {
                        ImGui::Separator();
                        const char* knifeModelsList[] = {
                            "Default", "Bayonet", "Flip Knife", "Gut Knife", "Karambit",
                            "M9 Bayonet", "Huntsman Knife", "Falchion Knife", "Bowie Knife",
                            "Butterfly Knife", "Shadow Daggers", "Ursus Knife", "Navaja Knife",
                            "Stiletto Knife", "Talon Knife", "Classic Knife"
                        };
                        int currentKnife = CONFIGINT("Misc>Skins>Knife Model");
                        if (ImGui::BeginCombo("Knife Model", knifeModelsList[currentKnife])) {
                            for (int i = 0; i < 16; i++) {
                                const bool is_selected = (currentKnife == i);
                                if (ImGui::Selectable(knifeModelsList[i], is_selected)) {
                                    CONFIGINT("Misc>Skins>Knife Model") = i;
                                }
                            }
                            ImGui::EndCombo();
                        }
                    }
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    if (ImGui::Button("Apply Skins & Knife Model", ImVec2(-1, 30))) {
                        FULLUPDATE();
                    }
                } else {
                    ImGui::Text("Select a weapon to configure");
                }
                ImGui::EndChild();
            }
            ImGui::Columns(1);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
