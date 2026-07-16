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
        if (!handle) {
            handle = dlopen("libgamesneeze.so", RTLD_NOLOAD | RTLD_LAZY);
        }
        if (!handle) return;
        
        dlclose(handle);
        
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
            ImGui::Columns(2, NULL, false);
            ImGui::SetColumnWidth(-1, ImGui::GetWindowContentRegionWidth() * 0.62);
            ImGui::BeginChild("Misc", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.61, ImGui::GetWindowHeight() * 0.64f), true); {
                ImGui::Text("Misc");
                ImGui::Separator();
                ImGui::Checkbox("Developer window", &devWindow);
                ImGui::Checkbox("Disable Watermark", &CONFIGBOOL("Misc>Misc>Misc>Disable Watermark"));
                ImGui::Checkbox("Force square radar", &CONFIGBOOL("Misc>Misc>Misc>Force square radar"));
                ImGui::Checkbox("Rank Revealer", &CONFIGBOOL("Misc>Misc>Misc>Rank Revealer"));

                ImGui::Checkbox("Spectators", &CONFIGBOOL("Misc>Misc>Misc>Spectators"));
                ImGui::Checkbox("Player List", &CONFIGBOOL("Misc>Misc>Misc>Player List"));
                if (CONFIGBOOL("Misc>Misc>Misc>Player List")) {
                    ImGui::SameLine();
                    ImGui::Checkbox("Only When Menu Open", &CONFIGBOOL("Misc>Misc>Misc>Player List Only When Menu Open"));
                }
                ImGui::Checkbox("Flappy Birb", &CONFIGBOOL("Misc>Misc>Misc>Flappy Birb"));
                ImGui::Checkbox("Auto Accept", &CONFIGBOOL("Misc>Misc>Misc>Auto Accept"));
                ImGui::Checkbox("Auto Defuse", &CONFIGBOOL("Misc>Misc>Misc>Auto Defuse"));
                if (CONFIGBOOL("Misc>Misc>Misc>Auto Defuse")) {
                    ImGui::SameLine();
                    ImGui::Checkbox("Latest Defuse", &CONFIGBOOL("Misc>Misc>Misc>Latest Defuse"));
                }
                ImGui::Checkbox("Chat Filter Bypass", &CONFIGBOOL("Misc>Misc>Misc>Chat Filter Bypass"));
                if (CONFIGBOOL("Misc>Misc>Misc>Use Spam")) {
                    static bool toggled = false;
                    Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Misc>Use Spam Key"), &toggled);
                    ImGui::SameLine();
                }
                ImGui::Checkbox("Use Spam", &CONFIGBOOL("Misc>Misc>Misc>Use Spam"));
                ImGui::Checkbox("Disable Setting Cvars", &CONFIGBOOL("Misc>Misc>Misc>Disable Setting Cvars"));
                ImGui::Checkbox("Disable Post Processing", &CONFIGBOOL("Misc>Misc>Misc>Disable Post Processing"));
                ImGui::EndChild();
            }

            ImGui::BeginChild("Hitmarkers", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.61, ImGui::GetWindowHeight() * 0.21f), true); {
                ImGui::Text("Hitmarkers");
                ImGui::Separator();
                ImGui::Checkbox("Hitlogs", &CONFIGBOOL("Misc>Misc>Hitmarkers>Hitlogs"));
                ImGui::Checkbox("Hitmarkers", &CONFIGBOOL("Misc>Misc>Hitmarkers>Hitmarkers"));
                ImGui::Checkbox("Hitsound", &CONFIGBOOL("Misc>Misc>Hitmarkers>Hitsound"));
                ImGui::Checkbox("Damage Markers", &CONFIGBOOL("Misc>Misc>Hitmarkers>Damage Markers"));
                ImGui::EndChild();
            }

            ImGui::NextColumn();

            ImGui::BeginChild("Config", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.38, ImGui::GetWindowHeight() * 0.31f), true); {
                ImGui::Text("Config");
                ImGui::Separator();

                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                ImGui::ListBoxHeader("##configlist-lbox", ImVec2(0, 70));
                for (std::string file : Config::cfgFiles) {
                    if (ImGui::Button(file.c_str())) {
                        strcpy(Config::configFileName, file.c_str());
                    }
                }
                ImGui::ListBoxFooter();

                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                ImGui::InputText("##configfile-tbox", Config::configFileName, IM_ARRAYSIZE(Config::configFileName));
                if (ImGui::Button("Save", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.315, 0))) {
                    Config::save();
                }; ImGui::SameLine();
                if (ImGui::Button("Load", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.315, 0))) {
                    Config::load();
                    FULLUPDATE();
                }; ImGui::SameLine();
                if (ImGui::Button("Remove", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.315, 0))) {
                    Config::remove();
                };

                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.05f, 1.0f));
                if (ImGui::Button("Unload Cheat", ImVec2(ImGui::GetWindowContentRegionWidth(), 30))) {
                    selfUnload();
                }
                ImGui::PopStyleColor(3);

                ImGui::EndChild();
            }

            ImGui::BeginChild("Clantag", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.38, ImGui::GetWindowHeight() * 0.17f), true); {
                ImGui::Text("Clantag");
                ImGui::Separator();
                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                ImGui::InputText("##clantag-tbox", clantag, IM_ARRAYSIZE(clantag));
                ImGui::Checkbox("Clantag", &CONFIGBOOL("Misc>Misc>Clantag>Clantag"));
                ImGui::Checkbox("Marquee", &CONFIGBOOL("Misc>Misc>Clantag>Clantag Marquee"));
                ImGui::Checkbox("Bee Movie Clantag", &CONFIGBOOL("Misc>Misc>Clantag>Bee Movie Clantag"));
                ImGui::EndChild();
            }

            ImGui::BeginChild("Movement", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.38, ImGui::GetWindowHeight() * 0.39f), true); {
                ImGui::Text("Movement");
                ImGui::Separator();
                ImGui::Checkbox("Auto Hop", &CONFIGBOOL("Misc>Misc>Movement>Auto Hop"));
                ImGui::Checkbox("Humanised Bhop", &CONFIGBOOL("Misc>Misc>Movement>Humanised Bhop"));
                if (CONFIGBOOL("Misc>Misc>Movement>Humanised Bhop")) {
                    ImGui::Text("Bhop Hitchance");
                    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                    ImGui::SliderInt("##Bhop Hitchance", &CONFIGINT("Misc>Misc>Movement>Bhop Hitchance"), 0, 100);
                    ImGui::Text("Bhop Max Misses");
                    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                    ImGui::SliderInt("##Bhop Max Misses", &CONFIGINT("Misc>Misc>Movement>Bhop Max Misses"), 0, 16);
                    ImGui::Text("Bhop Max Hops Hit");
                    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                    ImGui::SliderInt("##Bhop Max Hops Hit", &CONFIGINT("Misc>Misc>Movement>Bhop Max Hops Hit"), 0, 16);
                }
                if (CONFIGBOOL("Misc>Misc>Movement>Edge Jump")) {
                    static bool toggled = false;
                    Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Movement>Edge Jump Key"), &toggled);
                    ImGui::SameLine();
                }
                ImGui::Checkbox("Edge Jump", &CONFIGBOOL("Misc>Misc>Movement>Edge Jump"));
                if (CONFIGBOOL("Misc>Misc>Movement>JumpBug")) {
                    static bool toggled = false;
                    Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Movement>JumpBug Key"), &toggled);
                    ImGui::SameLine();
                }
                ImGui::Checkbox("JumpBug", &CONFIGBOOL("Misc>Misc>Movement>JumpBug"));
                if (CONFIGBOOL("Misc>Misc>Movement>EdgeBug")) {
                    static bool toggled = false;
                    Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Movement>EdgeBug Key"), &toggled);
                    ImGui::SameLine();
                }
                ImGui::Checkbox("EdgeBug", &CONFIGBOOL("Misc>Misc>Movement>EdgeBug"));
                if (CONFIGBOOL("Misc>Misc>Movement>EdgeBug")) {
                    ImGui::Checkbox("EdgeBug Highlight", &CONFIGBOOL("Misc>Misc>Movement>EdgeBug Highlight"));
                    if (CONFIGBOOL("Misc>Misc>Movement>EdgeBug Highlight")) {
                        ImGui::SameLine();
                        ImGui::Checkbox("Always Show", &CONFIGBOOL("Misc>Misc>Movement>EdgeBug Highlight Always"));
                    }
                    ImGui::Checkbox("EdgeBug Finder", &CONFIGBOOL("Misc>Misc>Movement>EdgeBug Finder"));
                }
                ImGui::Checkbox("Fast Duck", &CONFIGBOOL("Misc>Misc>Movement>Fast Duck"));
                ImGui::SameLine();
                ImGui::TextDisabled("?");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("May cause untrusted, use at own risk!");
                
                if (CONFIGBOOL("Misc>Misc>Movement>Auto Strafe")) {
                    static bool toggled = false;
                    Menu::CustomWidgets::drawKeyBinder("Key", &CONFIGINT("Misc>Misc>Movement>Auto Strafe Key"), &toggled);
                    ImGui::SameLine();
                }
                ImGui::Checkbox("Auto Strafe", &CONFIGBOOL("Misc>Misc>Movement>Auto Strafe"));

                if (CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Right")) {
                    static bool toggled = false;
                    Menu::CustomWidgets::drawKeyBinder("Right Key", &CONFIGINT("Misc>Misc>Movement>Auto Strafe Right Key"), &toggled);
                    ImGui::SameLine();
                }
                ImGui::Checkbox("Auto Strafe Right (Sideways)", &CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Right"));

                if (CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Left")) {
                    static bool toggled = false;
                    Menu::CustomWidgets::drawKeyBinder("Left Key", &CONFIGINT("Misc>Misc>Movement>Auto Strafe Left Key"), &toggled);
                    ImGui::SameLine();
                }
                ImGui::Checkbox("Auto Strafe Left (Sideways)", &CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Left"));

                ImGui::Checkbox("Auto Counter-Strafe", &CONFIGBOOL("Misc>Misc>Movement>Fast Stop"));
                ImGui::Checkbox("Speed Indicator", &CONFIGBOOL("Misc>Misc>Movement>Speed Indicator"));
                ImGui::EndChild();
            }
            ImGui::Columns(1);
            ImGui::TextDisabled("Credits!");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("sekc (of course)\nAll other contributors on GitHub (xXx-sans-xXx, luk1337, cristeigabriel, crazily, dave (@dweee), keenan, u2ooS, tango1337, nigma1337, vampur, allbombson, jovvik, and any other contributors)\nDonators:\n moke#9091/github.com/mokeWe - 0.19XMR (~$30)\n hx#5185 - 0.2XMR (~$30)\nand ocornut for his great ImGui UI framework");
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
