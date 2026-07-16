#include "features.h"
#include "../../includes.hpp"

void Features::SlowWalk::createMove(CUserCmd *cmd) {
    if (Interfaces::engine->IsInGame() && Globals::localPlayer && Globals::localPlayer->health() > 0 && 
            Globals::localPlayer->flags() & (1 << 0) && cmd->buttons & (1 << 17) && CONFIGBOOL("Rage>AntiAim>Slow Walk")) {
        float maxSpeed = (float)CONFIGINT("Rage>AntiAim>Slow Walk Speed");
        float currentSpeed = Globals::localPlayer->velocity().Length2D();
        if (currentSpeed > maxSpeed) {
            float moveSpeed = sqrt(cmd->forwardmove * cmd->forwardmove + cmd->sidemove * cmd->sidemove);
            if (moveSpeed > 0.0f) {
                cmd->forwardmove = (cmd->forwardmove / moveSpeed) * maxSpeed;
                cmd->sidemove = (cmd->sidemove / moveSpeed) * maxSpeed;
            }
        }
    }
}
