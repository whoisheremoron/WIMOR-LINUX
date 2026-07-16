#include "../../includes.hpp"
#include "features.hpp"
#include "autowall.hpp"
#include <cstring>
#include <cmath>
#include <algorithm>

// Surface penetration modifiers based on material type
// These are simplified from CSGO's FireBullet implementation
float Features::AutoWall::GetSurfacePenetrationModifier(unsigned short surfaceFlags, const char* surfaceName) {
    if (!surfaceName)
        return 1.0f;

    // Wood
    if (strstr(surfaceName, "wood") || strstr(surfaceName, "plywood") || strstr(surfaceName, "door"))
        return 1.0f;
    // Concrete / stone
    if (strstr(surfaceName, "concrete") || strstr(surfaceName, "brick") || strstr(surfaceName, "stone") || strstr(surfaceName, "rock"))
        return 0.5f;
    // Metal
    if (strstr(surfaceName, "metal") || strstr(surfaceName, "iron") || strstr(surfaceName, "steel") || strstr(surfaceName, "vent"))
        return 0.4f;
    // Glass
    if (strstr(surfaceName, "glass") || strstr(surfaceName, "window"))
        return 1.5f;
    // Tile
    if (strstr(surfaceName, "tile"))
        return 0.65f;
    // Plastic / rubber
    if (strstr(surfaceName, "plastic") || strstr(surfaceName, "rubber") || strstr(surfaceName, "cardboard"))
        return 1.2f;
    // Flesh
    if (strstr(surfaceName, "flesh") || strstr(surfaceName, "player"))
        return 1.0f;
    // Dirt / sand
    if (strstr(surfaceName, "dirt") || strstr(surfaceName, "sand") || strstr(surfaceName, "mud"))
        return 0.7f;
    // Plaster / drywall
    if (strstr(surfaceName, "plaster") || strstr(surfaceName, "drywall") || strstr(surfaceName, "ceiling"))
        return 1.0f;

    return 1.0f; // default
}

float Features::AutoWall::GetDamageMultiplier(unsigned short surfaceFlags, const char* surfaceName) {
    if (!surfaceName)
        return 0.5f;

    if (strstr(surfaceName, "glass") || strstr(surfaceName, "window"))
        return 0.9f;
    if (strstr(surfaceName, "wood") || strstr(surfaceName, "plywood"))
        return 0.75f;
    if (strstr(surfaceName, "plastic") || strstr(surfaceName, "cardboard"))
        return 0.8f;
    if (strstr(surfaceName, "plaster") || strstr(surfaceName, "drywall"))
        return 0.8f;
    if (strstr(surfaceName, "metal") || strstr(surfaceName, "iron") || strstr(surfaceName, "steel"))
        return 0.3f;
    if (strstr(surfaceName, "concrete") || strstr(surfaceName, "brick") || strstr(surfaceName, "stone"))
        return 0.4f;

    return 0.5f;
}

float Features::AutoWall::ScaleDamage(Player* target, float damage, float armorRatio, HitGroups hitgroup) {
    float modifier = 1.0f;

    switch (hitgroup) {
        case HITGROUP_HEAD:
            modifier = 4.0f; // headshot multiplier
            break;
        case HITGROUP_CHEST:
        case HITGROUP_LEFTARM:
        case HITGROUP_RIGHTARM:
            modifier = 1.0f;
            break;
        case HITGROUP_STOMACH:
            modifier = 1.25f;
            break;
        case HITGROUP_LEFTLEG:
        case HITGROUP_RIGHTLEG:
            modifier = 0.75f;
            break;
        default:
            break;
    }

    damage *= modifier;

    // Apply armor
    int armor = target->armor();
    if (armor > 0) {
        bool hasHelmet = target->helmet();
        bool isHeadshot = (hitgroup == HITGROUP_HEAD);

        if (isHeadshot && !hasHelmet)
            return damage; // No helmet = full headshot damage

        // Armor absorbs (1 - armorRatio) * damage, but limited by armor * 2
        float armorDamage = damage * (1.0f - armorRatio);
        // Each point of armor absorbs 2 damage
        if (armorDamage > (float)armor * 2.0f)
            armorDamage = (float)armor * 2.0f;

        damage -= armorDamage;
    }

    return damage;
}

bool Features::AutoWall::HandleBulletPenetration(FireBulletData& data, float penetrationPower, float penetrationMod) {
    Trace exitTrace;
    
    // Step through the wall to find the exit
    constexpr float MAX_WALL_THICKNESS = 64.0f;
    constexpr float STEP_SIZE = 4.0f;
    bool foundExit = false;
    
    for (float dist = 0.0f; dist < MAX_WALL_THICKNESS; dist += STEP_SIZE) {
        Vector testPoint = data.enterTrace.endpos + data.direction * (dist + STEP_SIZE);
        
        // Trace backwards from the test point to find exit surface
        Ray exitRay;
        exitRay.Init(testPoint, data.enterTrace.endpos + data.direction * dist);
        
        Trace reverseTrace;
        // Use MASK_SHOT for the trace with correct filter
        Interfaces::trace->TraceRay(exitRay, 0x4600400B, &data.filter, &reverseTrace);
        
        if (reverseTrace.startsolid && reverseTrace.surface.flags & 0x8000 /* SURF_HITBOX */) {
            // We hit a player hitbox inside the wall, trace through it
            TraceFilter playerFilter;
            playerFilter.pSkip = reverseTrace.m_pEntityHit;
            
            Ray throughRay;
            throughRay.Init(testPoint, data.enterTrace.endpos + data.direction * dist);
            Interfaces::trace->TraceRay(throughRay, 0x4600400b, &playerFilter, &reverseTrace);
        }
        
        if (!reverseTrace.startsolid && reverseTrace.fraction < 1.0f) {
            exitTrace = reverseTrace;
            exitTrace.endpos = testPoint;
            foundExit = true;
            break;
        }
    }
    
    if (!foundExit)
        return false;
    
    // Calculate wall thickness
    float wallThickness = (exitTrace.endpos - data.enterTrace.endpos).Length();
    
    // Get surface penetration modifier
    float enterPenMod = GetSurfacePenetrationModifier(data.enterTrace.surface.flags, data.enterTrace.surface.name);
    float exitPenMod = GetSurfacePenetrationModifier(exitTrace.surface.flags, exitTrace.surface.name);
    float combinedPenMod = (enterPenMod + exitPenMod) * 0.5f;
    
    // Calculate damage reduction from penetration
    // CSGO formula: damage is reduced based on wall thickness, penetration power, and material
    float modifier = std::max(0.0f, 1.0f / combinedPenMod);
    float lostDamage = (modifier * wallThickness / penetrationPower)
                     + (data.currentDamage * (1.0f - GetDamageMultiplier(data.enterTrace.surface.flags, data.enterTrace.surface.name))) 
                     + 1.0f; // minimum 1 damage lost
    
    if (lostDamage > data.currentDamage)
        return false;
    
    // Bullet still has enough energy
    data.currentDamage -= lostDamage;
    
    if (data.currentDamage < 1.0f)
        return false;
    
    // Move bullet to exit point
    data.src = exitTrace.endpos + data.direction * 1.0f; // small offset past the wall
    data.penetrateCount--;
    
    return true;
}

float Features::AutoWall::GetDamage(Vector src, Vector dst, Player* target, float weaponDamage, float weaponPenetration, float weaponRange, float weaponRangeModifier, float armorRatio) {
    FireBulletData data;
    data.src = src;
    data.filter.pSkip = Globals::localPlayer;
    data.penetrateCount = MAX_PENETRATIONS;
    data.currentDamage = weaponDamage;
    
    // Calculate direction
    Vector delta = dst - src;
    float distance = delta.Length();
    data.direction = delta / distance;
    
    // Range check
    if (distance > weaponRange)
        return 0.0f;
    
    // Apply range modifier (damage falloff over distance)
    float rangeModifier = pow(weaponRangeModifier, distance / 500.0f);
    data.currentDamage *= rangeModifier;
    
    // Iteratively trace through walls
    while (data.penetrateCount > 0 && data.currentDamage > 0.0f) {
        Ray ray;
        ray.Init(data.src, dst);
        
        Interfaces::trace->TraceRay(ray, 0x4600400B, &data.filter, &data.enterTrace);
        
        // Direct hit on target
        if (data.enterTrace.m_pEntityHit == (Player*)target) {
            return ScaleDamage(target, data.currentDamage, armorRatio, data.enterTrace.hitgroup);
        }
        
        // Reached destination without hitting anything solid enough to stop
        if (data.enterTrace.fraction >= 1.0f) {
            return ScaleDamage(target, data.currentDamage, armorRatio, HITGROUP_CHEST); // default hitgroup
        }
        
        // Hit something — try to penetrate
        if (!HandleBulletPenetration(data, weaponPenetration, 1.0f)) {
            return 0.0f; // Can't penetrate
        }
    }
    
    return 0.0f; // Ran out of penetrations
}

// Weapon stats lookup table (simplified)
struct WeaponStats {
    float damage;
    float penetration;
    float range;
    float rangeModifier;
    float armorRatio;
};

static WeaponStats getWeaponStats(ItemIndex idx) {
    // Default rifle stats
    WeaponStats stats = { 36.0f, 2.0f, 8192.0f, 0.98f, 0.475f };
    
    switch (idx) {
        // Pistols
        case WEAPON_GLOCK:      stats = { 28.0f, 1.0f, 8192.0f, 0.75f, 0.47f }; break;
        case WEAPON_HKP2000:    stats = { 35.0f, 1.0f, 8192.0f, 0.79f, 0.505f }; break;
        case WEAPON_USP_SILENCER: stats = { 35.0f, 1.0f, 8192.0f, 0.79f, 0.505f }; break;
        case WEAPON_P250:       stats = { 38.0f, 1.0f, 8192.0f, 0.80f, 0.475f }; break;
        case WEAPON_FIVESEVEN:  stats = { 32.0f, 1.0f, 8192.0f, 0.885f, 0.57f }; break;
        case WEAPON_TEC9:       stats = { 33.0f, 1.0f, 8192.0f, 0.81f, 0.52f }; break;
        case WEAPON_CZ75A:      stats = { 33.0f, 1.0f, 8192.0f, 0.81f, 0.475f }; break;
        case WEAPON_ELITE:      stats = { 36.0f, 1.0f, 8192.0f, 0.75f, 0.505f }; break;
        case WEAPON_DEAGLE:     stats = { 63.0f, 2.0f, 8192.0f, 0.81f, 0.505f }; break;
        case WEAPON_REVOLVER:   stats = { 86.0f, 2.0f, 8192.0f, 0.79f, 0.505f }; break;
        
        // SMGs
        case WEAPON_MAC10:      stats = { 29.0f, 1.0f, 8192.0f, 0.82f, 0.575f }; break;
        case WEAPON_MP9:        stats = { 26.0f, 1.0f, 8192.0f, 0.84f, 0.525f }; break;
        case WEAPON_MP7:        stats = { 29.0f, 1.0f, 8192.0f, 0.84f, 0.525f }; break;
        case WEAPON_MP5:        stats = { 27.0f, 1.0f, 8192.0f, 0.84f, 0.525f }; break;
        case WEAPON_UMP45:      stats = { 35.0f, 1.0f, 8192.0f, 0.75f, 0.65f }; break;
        case WEAPON_P90:        stats = { 26.0f, 1.0f, 8192.0f, 0.84f, 0.51f }; break;
        case WEAPON_BIZON:      stats = { 27.0f, 1.0f, 8192.0f, 0.80f, 0.575f }; break;
        
        // Rifles
        case WEAPON_AK47:       stats = { 36.0f, 2.0f, 8192.0f, 0.98f, 0.475f }; break;
        case WEAPON_M4A1:       stats = { 33.0f, 2.0f, 8192.0f, 0.97f, 0.70f }; break;
        case WEAPON_M4A1_SILENCER: stats = { 33.0f, 2.0f, 8192.0f, 0.97f, 0.70f }; break;
        case WEAPON_FAMAS:      stats = { 30.0f, 2.0f, 8192.0f, 0.96f, 0.70f }; break;
        case WEAPON_GALILAR:    stats = { 30.0f, 2.0f, 8192.0f, 0.98f, 0.475f }; break;
        case WEAPON_AUG:        stats = { 28.0f, 2.0f, 8192.0f, 0.96f, 0.90f }; break;
        case WEAPON_SG556:      stats = { 30.0f, 2.0f, 8192.0f, 0.955f, 0.65f }; break;
        
        // Snipers
        case WEAPON_SSG08:      stats = { 88.0f, 2.5f, 8192.0f, 0.98f, 0.85f }; break;
        case WEAPON_AWP:        stats = { 115.0f, 2.5f, 8192.0f, 0.99f, 0.975f }; break;
        case WEAPON_SCAR20:     stats = { 80.0f, 2.5f, 8192.0f, 0.98f, 0.83f }; break;
        case WEAPON_G3SG1:      stats = { 80.0f, 2.5f, 8192.0f, 0.98f, 0.83f }; break;
        
        // Heavies
        case WEAPON_M249:       stats = { 32.0f, 2.0f, 8192.0f, 0.97f, 0.80f }; break;
        case WEAPON_NEGEV:      stats = { 35.0f, 2.0f, 8192.0f, 0.97f, 0.65f }; break;
        case WEAPON_NOVA:       stats = { 26.0f, 1.0f, 3000.0f, 0.70f, 0.50f }; break;
        case WEAPON_XM1014:     stats = { 20.0f, 1.0f, 3000.0f, 0.70f, 0.80f }; break;
        case WEAPON_MAG7:       stats = { 30.0f, 1.0f, 3000.0f, 0.70f, 0.85f }; break;
        case WEAPON_SAWEDOFF:   stats = { 32.0f, 1.0f, 3000.0f, 0.70f, 0.475f }; break;
        
        default: break;
    }
    return stats;
}

float Features::AutoWall::GetDamageAuto(Vector src, Vector dst, Player* target) {
    if (!Globals::localPlayer)
        return 0.0f;
    
    Weapon* weapon = (Weapon*)Interfaces::entityList->GetClientEntity((uintptr_t)Globals::localPlayer->activeWeapon() & 0xFFF);
    if (!weapon)
        return 0.0f;
    
    ItemIndex idx = weapon->itemIndex();
    WeaponStats stats = getWeaponStats(idx);
    
    return GetDamage(src, dst, target, stats.damage, stats.penetration, stats.range, stats.rangeModifier, stats.armorRatio);
}

bool Features::AutoWall::CanHit(Vector src, Vector dst, Player* target, float minDamage) {
    return GetDamageAuto(src, dst, target) >= minDamage;
}
