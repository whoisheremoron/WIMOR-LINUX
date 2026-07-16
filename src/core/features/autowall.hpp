#pragma once
#include "../../sdk/interfaces/ienginetrace.hpp"
#include "../../sdk/classes/entity.hpp"

namespace Features {
    namespace AutoWall {
        struct FireBulletData {
            Vector src;
            Trace enterTrace;
            Vector direction;
            TraceFilter filter;
            float currentDamage;
            int penetrateCount;
        };

        // CSGO penetration constants
        constexpr int MAX_PENETRATIONS = 4;

        // Surface type penetration modifiers (simplified)
        float GetSurfacePenetrationModifier(unsigned short surfaceFlags, const char* surfaceName);
        float GetDamageMultiplier(unsigned short surfaceFlags, const char* surfaceName);

        // Scale damage based on hitgroup, armor, helmet
        float ScaleDamage(Player* target, float damage, float armorRatio, HitGroups hitgroup);

        // Trace through walls and calculate remaining damage
        bool HandleBulletPenetration(FireBulletData& data, float penetrationPower, float penetrationMod);

        // Main function: returns predicted damage from src to dst through walls
        // Returns 0 if bullet can't reach the target
        float GetDamage(Vector src, Vector dst, Player* target, float weaponDamage, float weaponPenetration, float weaponRange, float weaponRangeModifier, float armorRatio);

        // Simplified overload that auto-reads weapon stats
        float GetDamageAuto(Vector src, Vector dst, Player* target);

        // Check if can hit target for at least minDamage
        bool CanHit(Vector src, Vector dst, Player* target, float minDamage);
    }
}
