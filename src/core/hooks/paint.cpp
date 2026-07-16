#include "../../includes.hpp"
#include "hooks.hpp"


void Hooks::Paint::hook(void* thisptr, PaintMode_t mode) {
    if (Globals::unloading) {
        original(thisptr, mode);
        return;
    }

    bool* scoped = nullptr;
    bool oldScoped = false;

    if (Interfaces::engine->IsInGame() && Globals::localPlayer && Globals::localPlayer->health() > 0) {
        if (CONFIGBOOL("Visuals>World>World>Third Person") && CONFIGBOOL("Visuals>World>World>Third Person While Scoped")) {
            scoped = &Globals::localPlayer->scoped();
            oldScoped = *scoped;
            *scoped = false;
        }
    }

    original(thisptr, mode);

    if (scoped) {
        *scoped = oldScoped;
    }

    if (mode & PAINT_UIPANELS) {
        Globals::worldToScreenMatrix = Interfaces::engine->WorldToScreenMatrix();
    }
}