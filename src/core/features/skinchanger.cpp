#include "features.hpp"
#include "../../includes.hpp"

struct KnifeModelInfo {
    const char* modelPath;
    const char* worldModelPath;
    ItemIndex itemIndex;
};

static const KnifeModelInfo knifeModels[] = {
    {"", "", ItemIndex::INVALID},
    {"models/weapons/v_knife_bayonet.mdl", "models/weapons/w_knife_bayonet.mdl", ItemIndex::WEAPON_KNIFE_BAYONET},
    {"models/weapons/v_knife_flip.mdl", "models/weapons/w_knife_flip.mdl", ItemIndex::WEAPON_KNIFE_FLIP},
    {"models/weapons/v_knife_gut.mdl", "models/weapons/w_knife_gut.mdl", ItemIndex::WEAPON_KNIFE_GUT},
    {"models/weapons/v_knife_karam.mdl", "models/weapons/w_knife_karam.mdl", ItemIndex::WEAPON_KNIFE_KARAMBIT},
    {"models/weapons/v_knife_m9_bay.mdl", "models/weapons/w_knife_m9_bay.mdl", ItemIndex::WEAPON_KNIFE_M9_BAYONET},
    {"models/weapons/v_knife_tactical.mdl", "models/weapons/w_knife_tactical.mdl", ItemIndex::WEAPON_KNIFE_TACTICAL},
    {"models/weapons/v_knife_falchion_advanced.mdl", "models/weapons/w_knife_falchion_advanced.mdl", ItemIndex::WEAPON_KNIFE_FALCHION},
    {"models/weapons/v_knife_survival_bowie.mdl", "models/weapons/w_knife_survival_bowie.mdl", ItemIndex::WEAPON_KNIFE_SURVIVAL_BOWIE},
    {"models/weapons/v_knife_butterfly.mdl", "models/weapons/w_knife_butterfly.mdl", ItemIndex::WEAPON_KNIFE_BUTTERFLY},
    {"models/weapons/v_knife_push.mdl", "models/weapons/w_knife_push.mdl", ItemIndex::WEAPON_KNIFE_PUSH},
    {"models/weapons/v_knife_ursus.mdl", "models/weapons/w_knife_ursus.mdl", ItemIndex::WEAPON_KNIFE_URSUS},
    {"models/weapons/v_knife_gypsy_jackknife.mdl", "models/weapons/w_knife_gypsy_jackknife.mdl", ItemIndex::WEAPON_KNIFE_GYPSY_JACKKNIFE},
    {"models/weapons/v_knife_stiletto.mdl", "models/weapons/w_knife_stiletto.mdl", ItemIndex::WEAPON_KNIFE_STILETTO},
    {"models/weapons/v_knife_widowmaker.mdl", "models/weapons/w_knife_widowmaker.mdl", ItemIndex::WEAPON_KNIFE_WIDOWMAKER},
    {"models/weapons/v_knife_css.mdl", "models/weapons/w_knife_css.mdl", ItemIndex::WEAPON_KNIFE_CSS}
};

void Features::SkinChanger::frameStageNotify(FrameStage frame) {
    if (frame == FRAME_NET_UPDATE_POSTDATAUPDATE_START && Globals::localPlayer && Interfaces::engine->IsInGame() && Globals::localPlayer->health() > 0) {
        Weapon* weapon = (Weapon*) Interfaces::entityList->GetClientEntity((uintptr_t)Globals::localPlayer->activeWeapon() & 0xFFF);
        try {
            if (weapon && weapon->itemIndex() != ItemIndex::INVALID) {
                ItemIndex currentIdx = weapon->itemIndex();
                bool isKnife = (currentIdx == ItemIndex::WEAPON_KNIFE || 
                                currentIdx == ItemIndex::WEAPON_KNIFE_T ||
                                (currentIdx >= ItemIndex::WEAPON_KNIFE_BAYONET && currentIdx <= ItemIndex::WEAPON_KNIFE_GHOST));
                
                const char* weaponName = "Knife";
                if (isKnife) {
                    int knifeIdx = CONFIGINT("Misc>Skins>Knife Model");
                    if (knifeIdx > 0 && knifeIdx < 16) {
                        const auto& knifeInfo = knifeModels[knifeIdx];
                        *weapon->itemIndex_ptr() = knifeInfo.itemIndex;
                        
                        int vModelIndex = Interfaces::modelInfo->GetModelIndex(knifeInfo.modelPath);
                        *weapon->modelIndex_ptr() = vModelIndex;
                        
                        Entity* viewmodel = (Entity*)Interfaces::entityList->GetClientEntity(Globals::localPlayer->viewModel() & 0xFFF);
                        if (viewmodel) {
                            *viewmodel->modelIndex_ptr() = vModelIndex;
                        }
                    }
                } else {
                    weaponName = itemIndexMap.at(currentIdx);
                }
                
                char buf[256];
                snprintf(buf, 256, "Misc>Skins>Skins>%s>PaintKit", weaponName);
 
                char buf2[256];
                snprintf(buf2, 256, "Misc>Skins>Skins>%s>Wear", weaponName);
 
                char buf3[256];
                snprintf(buf3, 256, "Misc>Skins>Skins>%s>StatTrack", weaponName);
 
                int paintkit = CONFIGINT(buf);
                float wear = (float)CONFIGINT(buf2)/100.f;
                int statTrack = CONFIGINT(buf3);
 
                player_info_t info;
                Interfaces::engine->GetPlayerInfo(Interfaces::engine->GetLocalPlayer(), &info);
                *weapon->accountID_ptr() = info.xuid;
                *weapon->itemIDHigh_ptr() = -1;
                *weapon->paintKit_ptr() = paintkit;
                *weapon->wear_ptr() = wear;
                *weapon->statTrack_ptr() = statTrack;
            }
        } catch (std::out_of_range& e) {
            //Log::log(WARN, "itemDefinitionIndex %d not found!", ((Weapon*)ent)->itemIndex());
        }
    }
}