#include "../../includes.hpp"
#include "features.hpp"
#include "autowall.hpp"
#include <algorithm>

// Toggle state for autoshoot (persists across frames)
static bool autoShootToggled = false;
static bool autoShootKeyWasDown = false;

// Hitbox bone indices for CSGO
static constexpr int HITBOX_HEAD = 8;
static constexpr int HITBOX_NECK = 7;
static constexpr int HITBOX_CHEST = 6;
static constexpr int HITBOX_STOMACH = 5;
static constexpr int HITBOX_PELVIS = 3;

struct HitboxScanResult {
    Vector position;
    float damage;
    int hitbox;
    bool valid;
};

void Features::RageBot::createMove(CUserCmd* cmd) {
    bool autoShoot = CONFIGBOOL("Rage>RageBot>Default>AutoShoot");
    autoShootActive = false;

    if (autoShoot) {
        int key = CONFIGINT("Rage>RageBot>Default>AutoShootKey");
        int mode = CONFIGINT("Rage>RageBot>Default>AutoShootMode"); // 0 = Hold, 1 = Toggle

        if (key > 0) {
            bool keyDown = Menu::CustomWidgets::isKeyDown(key);

            if (mode == 1) { // Toggle
                // Detect rising edge (key just pressed)
                if (keyDown && !autoShootKeyWasDown) {
                    autoShootToggled = !autoShootToggled;
                }
                autoShootKeyWasDown = keyDown;
                autoShootActive = autoShootToggled;
            } else { // Hold (original behavior)
                autoShootActive = keyDown;
            }
        } else {
            // No key bound — always active when autoshoot enabled
            autoShootActive = true;
        }
    }

    if (Interfaces::engine->IsInGame() && Globals::localPlayer && Globals::localPlayer->health() > 0 && ((autoShoot && autoShootActive) || (cmd->buttons & IN_ATTACK))) {
        Weapon *weapon = (Weapon *) Interfaces::entityList->GetClientEntity((uintptr_t)Globals::localPlayer->activeWeapon() & 0xFFF);
        if (weapon) {
            ItemIndex currentIdx = weapon->itemIndex();
            
            // Check if it's a gun (prevent ragebot aiming/firing with utilities/melee)
            bool isGun = false;
            for (auto idx : pistols) { if ((currentIdx & 0xFFF) == (idx & 0xFFF)) isGun = true; }
            for (auto idx : heavyPistols) { if ((currentIdx & 0xFFF) == (idx & 0xFFF)) isGun = true; }
            for (auto idx : rifles) { if ((currentIdx & 0xFFF) == (idx & 0xFFF)) isGun = true; }
            for (auto idx : smgs) { if ((currentIdx & 0xFFF) == (idx & 0xFFF)) isGun = true; }
            for (auto idx : heavyWeapons) { if ((currentIdx & 0xFFF) == (idx & 0xFFF)) isGun = true; }
            if (currentIdx == WEAPON_AWP || currentIdx == WEAPON_SSG08 || currentIdx == WEAPON_SCAR20 || currentIdx == WEAPON_G3SG1 || currentIdx == WEAPON_TASER) {
                isGun = true;
            }
            
            if (!isGun || weapon->clip1() <= 0) {
                return;
            }

            float FOV = CONFIGINT("Rage>RageBot>Default>FOV") / 10.f;
            float minDamage = std::max(1.0f, (float)CONFIGINT("Rage>RageBot>Default>MinDamage"));
            int hitboxSelection = CONFIGINT("Rage>RageBot>Default>Hitboxes"); // bitmask of HitBoxes enum

            float closestCrosshair = FLT_MAX;
            float closestDistance = FLT_MAX;
            int targetMode = CONFIGINT("Rage>RageBot>Default>TargetSelection"); // 0 = Crosshair, 1 = Distance, 2 = Highest Damage
            bool dmMode = CONFIGBOOL("Rage>RageBot>Default>DMMode");

            QAngle angleToClosestPlayer = {0, 0, 0};
            bool hasTarget = false;
            float highestDamage = 0.0f;

            Vector localPlayerEyePos = Globals::localPlayer->eyePos();

            // Enumerate over players and get angle to target
            for (int i = 1; i < Interfaces::globals->maxClients; i++) {
                Player* p = (Player*)Interfaces::entityList->GetClientEntity(i);
                if (p && p != Globals::localPlayer) {
                    bool isTarget = false;
                    if (p->health() > 0 && !p->dormant()) {
                        if (dmMode) {
                            isTarget = true;
                        } else {
                            isTarget = p->isEnemy();
                        }
                    }
                    if (isTarget) {
                        matrix3x4_t boneMatrix[128];
                        if (p->getAnythingBones(boneMatrix)) {
                            // Force baim check
                            bool forceBaim = CONFIGBOOL("Rage>RageBot>Default>ForceBaim") && 
                                             p->health() <= CONFIGINT("Rage>RageBot>Default>ForceBaimValue");

                            // Scan multiple hitboxes
                            struct { int bone; int hitboxFlag; } hitboxes[] = {
                                { HITBOX_HEAD,    (int)HitBoxes::HEAD },
                                { HITBOX_NECK,    (int)HitBoxes::NECK },
                                { HITBOX_CHEST,   (int)HitBoxes::CHEST },
                                { HITBOX_STOMACH, (int)HitBoxes::STOMACH },
                                { HITBOX_PELVIS,  (int)HitBoxes::PELVIS },
                            };

                            HitboxScanResult bestHitbox = { Vector(0,0,0), 0.0f, -1, false };

                            for (auto& hb : hitboxes) {
                                // If force baim, skip head and neck
                                if (forceBaim && (hb.hitboxFlag == (int)HitBoxes::HEAD || hb.hitboxFlag == (int)HitBoxes::NECK))
                                    continue;

                                // Check if this hitbox is selected (or use all if none selected)
                                if (hitboxSelection > 0 && !(hitboxSelection & hb.hitboxFlag))
                                    continue;

                                Vector targetBonePos = p->getBonePos(hb.bone);
                                if (targetBonePos.x == 0.0f && targetBonePos.y == 0.0f && targetBonePos.z == 0.0f)
                                    continue;
                                
                                // AutoWall damage check — scan through ALL walls
                                float damage = Features::AutoWall::GetDamageAuto(localPlayerEyePos, targetBonePos, p);

                                if (damage >= minDamage && damage > bestHitbox.damage) {
                                    bestHitbox.position = targetBonePos;
                                    bestHitbox.damage = damage;
                                    bestHitbox.hitbox = hb.bone;
                                    bestHitbox.valid = true;
                                }
                            }

                            if (bestHitbox.valid) {
                                QAngle angleToCurrentPlayer = calcAngle(localPlayerEyePos, bestHitbox.position) - cmd->viewangles - Globals::localPlayer->aimPunch()*2;
                                normalizeAngles(angleToCurrentPlayer);

                                float fovDelta = angleToCurrentPlayer.Length();
                                float distDelta = (Globals::localPlayer->origin() - p->origin()).Length();

                                bool selectThisTarget = false;

                                if (targetMode == 0) { // Closest to Crosshair
                                    if (fovDelta < closestCrosshair && fovDelta < FOV) {
                                        closestCrosshair = fovDelta;
                                        selectThisTarget = true;
                                    }
                                } else if (targetMode == 1) { // Closest physically
                                    if (distDelta < closestDistance && fovDelta < FOV) {
                                        closestDistance = distDelta;
                                        selectThisTarget = true;
                                    }
                                } else { // Highest Damage
                                    if (bestHitbox.damage > highestDamage && fovDelta < FOV) {
                                        highestDamage = bestHitbox.damage;
                                        selectThisTarget = true;
                                    }
                                }

                                if (selectThisTarget) {
                                    angleToClosestPlayer = angleToCurrentPlayer;
                                    hasTarget = true;
                                }
                            }
                        }
                    }
                }
            }
            if (hasTarget) {
                cmd->viewangles += angleToClosestPlayer;
                if (autoShoot && autoShootActive) {
                    float serverTime = Globals::localPlayer->tickbase() * Interfaces::globals->interval_per_tick;
                    
                    if (weapon->nextPrimaryAttack() <= serverTime) {
                        cmd->buttons |= IN_ATTACK;
                    } else {
                        cmd->buttons &= ~IN_ATTACK;
                    }
                }
            } else if (autoShoot && autoShootActive) {
                cmd->buttons &= ~IN_ATTACK; // Clear IN_ATTACK if no target
            }
        }
    }
}
