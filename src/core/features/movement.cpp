#include "../../includes.hpp"
#include "features.hpp"
#include <vector>

struct EdgeInfo {
    Vector pos;
};
static std::vector<EdgeInfo> scannedEdges;

struct TrailPoint {
    Vector pos;
    float time;
};
static std::vector<TrailPoint> movementTrail;

struct LightningStrike {
    Vector origin;
    float startTime;
    std::vector<std::vector<Vector>> branches;
};
static std::vector<LightningStrike> activeStrikes;


enum StrafeMode {
    STRAFE_NORMAL = 0,
    STRAFE_RIGHT,
    STRAFE_LEFT
};
inline StrafeMode currentStrafeMode = STRAFE_NORMAL;

void bhop(CUserCmd *cmd) {
    if (CONFIGBOOL("Misc>Misc>Movement>JumpBug") &&
        Menu::CustomWidgets::isKeyDown(CONFIGINT("Misc>Misc>Movement>JumpBug Key")))
        return;
    if (CONFIGBOOL("Misc>Misc>Movement>Auto Hop")) {
        if (Globals::localPlayer->moveType() == 9)
            return;
        if (CONFIGBOOL("Misc>Misc>Movement>Humanised Bhop")) {
            // https://www.unknowncheats.me/forum/counterstrike-global-offensive/333797-humanised-bhop.html
            static int hopsRestricted = 0;
            static int hopsHit = 0;
            if (!(Globals::localPlayer->flags() & FL_ONGROUND)) {
                cmd->buttons &= ~IN_JUMP;
                hopsRestricted = 0;
            } else if ((rand() % 100 > CONFIGINT("Misc>Misc>Movement>Bhop Hitchance") &&
                        hopsRestricted <
                           CONFIGINT("Misc>Misc>Movement>Bhop Max Misses")) ||
                       (CONFIGINT("Misc>Misc>Movement>Bhop Max Hops Hit") > 0 &&
                        hopsHit > CONFIGINT("Misc>Misc>Movement>Bhop Max Hops Hit"))) {
                cmd->buttons &= ~IN_JUMP;
                hopsRestricted++;
                hopsHit = 0;
            } else {
                hopsHit++;
            }
        } else {
            if (!(Globals::localPlayer->flags() & FL_ONGROUND)) {
                cmd->buttons &= ~IN_JUMP;
            }
        }
    }
}

void edgeJump(CUserCmd *cmd) {
    if (CONFIGBOOL("Misc>Misc>Movement>Edge Jump") &&
        Menu::CustomWidgets::isKeyDown(CONFIGINT("Misc>Misc>Movement>Edge Jump Key")) &&
        Features::Movement::flagsBackup & FL_ONGROUND &&
        !(Globals::localPlayer->flags() & FL_ONGROUND))
        cmd->buttons |= IN_JUMP;
}

void jumpBug(CUserCmd *cmd) {
    static bool shouldSkip = false;
    if (shouldSkip) {
        shouldSkip = false;
        return;
    }
    if (CONFIGBOOL("Misc>Misc>Movement>JumpBug") &&
        Menu::CustomWidgets::isKeyDown(CONFIGINT("Misc>Misc>Movement>JumpBug Key")) &&
        !(Features::Movement::flagsBackup & FL_ONGROUND ||
          Features::Movement::flagsBackup & FL_PARTIALGROUND) &&
        (Globals::localPlayer->flags() & FL_ONGROUND ||
         Globals::localPlayer->flags() & FL_PARTIALGROUND)) {
        cmd->buttons |= IN_DUCK;
        cmd->buttons &= ~IN_JUMP;
        shouldSkip = true;
    }
}

bool checkEdgebug() {
    static ConVar *sv_gravity = Interfaces::convar->FindVar("sv_gravity");
    float edgebugZVel =
       (sv_gravity->GetFloat() * 0.5f * Interfaces::globals->interval_per_tick);

    return Features::Movement::velBackup.z < -edgebugZVel &&
           round(Globals::localPlayer->velocity().z) == -round(edgebugZVel) &&
           Globals::localPlayer->moveType() != MOVETYPE_LADDER;
}

void draw3DBox(const Vector& origin, float size, ImColor color) {
    Vector points[8];
    points[0] = origin + Vector(-size, -size, -size);
    points[1] = origin + Vector(size, -size, -size);
    points[2] = origin + Vector(size, size, -size);
    points[3] = origin + Vector(-size, size, -size);
    points[4] = origin + Vector(-size, -size, size);
    points[5] = origin + Vector(size, -size, size);
    points[6] = origin + Vector(size, size, size);
    points[7] = origin + Vector(-size, size, size);

    Vector screenPoints[8];
    for (int i = 0; i < 8; i++) {
        if (!worldToScreen(points[i], screenPoints[i]))
            return;
    }

    // Connect top and bottom faces
    for (int i = 0; i < 4; i++) {
        Globals::drawList->AddLine(ImVec2(screenPoints[i].x, screenPoints[i].y), ImVec2(screenPoints[(i + 1) % 4].x, screenPoints[(i + 1) % 4].y), color, 1.5f);
        Globals::drawList->AddLine(ImVec2(screenPoints[i + 4].x, screenPoints[i + 4].y), ImVec2(screenPoints[((i + 1) % 4) + 4].x, screenPoints[((i + 1) % 4) + 4].y), color, 1.5f);
        Globals::drawList->AddLine(ImVec2(screenPoints[i].x, screenPoints[i].y), ImVec2(screenPoints[i + 4].x, screenPoints[i + 4].y), color, 1.5f);
    }
}

void autoStrafe(CUserCmd *cmd) {
    if (!Globals::localPlayer || Globals::localPlayer->health() <= 0)
        return;

    // Handle keypress toggle detection (runs on ground too!)
    static bool rightWasDown = false;
    static bool leftWasDown = false;

    bool rightIsDown = CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Right") &&
                       Menu::CustomWidgets::isKeyDown(CONFIGINT("Misc>Misc>Movement>Auto Strafe Right Key"));

    bool leftIsDown = CONFIGBOOL("Misc>Misc>Movement>Auto Strafe Left") &&
                      Menu::CustomWidgets::isKeyDown(CONFIGINT("Misc>Misc>Movement>Auto Strafe Left Key"));

    if (rightIsDown && !rightWasDown) {
        if (currentStrafeMode == STRAFE_RIGHT) {
            currentStrafeMode = STRAFE_NORMAL;
        } else {
            currentStrafeMode = STRAFE_RIGHT;
        }
    }
    rightWasDown = rightIsDown;

    if (leftIsDown && !leftWasDown) {
        if (currentStrafeMode == STRAFE_LEFT) {
            currentStrafeMode = STRAFE_NORMAL;
        } else {
            currentStrafeMode = STRAFE_LEFT;
        }
    }
    leftWasDown = leftIsDown;

    // Standard air checks
    if (Globals::localPlayer->moveType() == MOVETYPE_LADDER || Globals::localPlayer->moveType() == MOVETYPE_NOCLIP)
        return;

    if (Globals::localPlayer->flags() & FL_ONGROUND)
        return;

    bool mainStrafeKeyHeld = CONFIGBOOL("Misc>Misc>Movement>Auto Strafe") &&
                             Menu::CustomWidgets::isKeyDown(CONFIGINT("Misc>Misc>Movement>Auto Strafe Key"));

    if (!mainStrafeKeyHeld)
        return;

    if (currentStrafeMode == STRAFE_NORMAL) {
        cmd->forwardmove = 0.0f;
        originalForwardMove = 0.0f;

        if (cmd->mousedx < 0) {
            cmd->sidemove = -450.0f;
            originalSideMove = -450.0f;
        } else if (cmd->mousedx > 0) {
            cmd->sidemove = 450.0f;
            originalSideMove = 450.0f;
        } else {
            float speed2d = Globals::localPlayer->velocity().Length2D();
            if (speed2d > 1.0f) {
                Vector velocity = Globals::localPlayer->velocity();
                float velocity_yaw = RAD2DEG(atan2(velocity.y, velocity.x));
                float yaw_delta = cmd->viewangles.y - velocity_yaw;
                while (yaw_delta > 180.f) yaw_delta -= 360.f;
                while (yaw_delta < -180.f) yaw_delta += 360.f;

                if (yaw_delta < 0.0f) {
                    cmd->sidemove = 450.0f;
                    originalSideMove = 450.0f;
                } else {
                    cmd->sidemove = -450.0f;
                    originalSideMove = -450.0f;
                }
            }
        }
    }
    else if (currentStrafeMode == STRAFE_RIGHT) {
        cmd->sidemove = 0.0f;
        originalSideMove = 0.0f;

        if (cmd->mousedx < 0) {
            cmd->forwardmove = -450.0f; // S
            originalForwardMove = -450.0f;
        } else if (cmd->mousedx > 0) {
            cmd->forwardmove = 450.0f;  // W
            originalForwardMove = 450.0f;
        } else {
            float speed2d = Globals::localPlayer->velocity().Length2D();
            if (speed2d > 1.0f) {
                Vector velocity = Globals::localPlayer->velocity();
                float velocity_yaw = RAD2DEG(atan2(velocity.y, velocity.x));
                float yaw_delta = cmd->viewangles.y - (velocity_yaw - 90.0f);
                while (yaw_delta > 180.f) yaw_delta -= 360.f;
                while (yaw_delta < -180.f) yaw_delta += 360.f;

                if (yaw_delta < 0.0f) {
                    cmd->forwardmove = 450.0f; // W
                    originalForwardMove = 450.0f;
                } else {
                    cmd->forwardmove = -450.0f; // S
                    originalForwardMove = -450.0f;
                }
            }
        }
    }
    else if (currentStrafeMode == STRAFE_LEFT) {
        cmd->sidemove = 0.0f;
        originalSideMove = 0.0f;

        if (cmd->mousedx < 0) {
            cmd->forwardmove = 450.0f;  // W
            originalForwardMove = 450.0f;
        } else if (cmd->mousedx > 0) {
            cmd->forwardmove = -450.0f; // S
            originalForwardMove = -450.0f;
        } else {
            float speed2d = Globals::localPlayer->velocity().Length2D();
            if (speed2d > 1.0f) {
                Vector velocity = Globals::localPlayer->velocity();
                float velocity_yaw = RAD2DEG(atan2(velocity.y, velocity.x));
                float yaw_delta = cmd->viewangles.y - (velocity_yaw + 90.0f);
                while (yaw_delta > 180.f) yaw_delta -= 360.f;
                while (yaw_delta < -180.f) yaw_delta += 360.f;

                if (yaw_delta < 0.0f) {
                    cmd->forwardmove = -450.0f; // S
                    originalForwardMove = -450.0f;
                } else {
                    cmd->forwardmove = 450.0f;  // W
                    originalForwardMove = 450.0f;
                }
            }
        }
    }
}

void fastStop(CUserCmd *cmd) {
    if (!CONFIGBOOL("Misc>Misc>Movement>Fast Stop"))
        return;

    if (!Globals::localPlayer || Globals::localPlayer->moveType() == MOVETYPE_LADDER || Globals::localPlayer->moveType() == MOVETYPE_NOCLIP)
        return;

    if (!(Globals::localPlayer->flags() & FL_ONGROUND))
        return;

    // Only skip counter-strafe if the player is actively pressing movement keys
    // AND the movement values reflect genuine player input (not ragebot/autopeek override)
    if ((cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)) &&
        (cmd->forwardmove != 0.0f || cmd->sidemove != 0.0f))
        return;

    Vector velocity = Globals::localPlayer->velocity();
    float speed = velocity.Length2D();
    
    // Stop applying inputs when speed is below 40.0 u/s to prevent walking in the opposite direction
    if (speed < 40.0f) {
        cmd->forwardmove = 0.0f;
        cmd->sidemove = 0.0f;
        originalForwardMove = 0.0f;
        originalSideMove = 0.0f;
        return;
    }

    QAngle velocity_angle = calcAngle(Vector(0,0,0), velocity);
    float yaw_delta = cmd->viewangles.y - velocity_angle.y;
    float rad = DEG2RAD(yaw_delta);
    
    // Scale counter-strafe input to prevent overshooting
    float stop_speed = std::clamp(speed * 2.0f, 100.0f, 450.0f);
    
    cmd->forwardmove = -cos(rad) * stop_speed;
    cmd->sidemove = -sin(rad) * stop_speed;
    originalForwardMove = cmd->forwardmove;
    originalSideMove = cmd->sidemove;
}

void scanForEdges() {
    scannedEdges.clear();

    if (!CONFIGBOOL("Misc>Misc>Movement>EdgeBug Finder"))
        return;

    if (!Globals::localPlayer || Globals::localPlayer->health() <= 0)
        return;

    Vector origin = Globals::localPlayer->origin();
    
    const int directions = 48;
    const int steps = 16;
    const float max_radius = 1200.0f;
    
    for (int d = 0; d < directions; d++) {
        float angle = (float)d * (360.0f / (float)directions);
        float rad = DEG2RAD(angle);
        Vector dir(cos(rad), sin(rad), 0.0f);
        
        for (int s = 1; s <= steps; s++) {
            float dist = ((float)s / (float)steps) * max_radius;
            Vector scanPoint = origin + dir * dist;
            
            Ray ray1;
            ray1.Init(scanPoint + Vector(0.0f, 0.0f, 64.0f), scanPoint - Vector(0.0f, 0.0f, 256.0f));
            
            TraceFilter filter;
            filter.pSkip = Globals::localPlayer;
            
            Trace trace1;
            Interfaces::trace->TraceRay(ray1, 0x1, &filter, &trace1);
            
            if (trace1.fraction < 1.0f && !trace1.allsolid) {
                Vector groundPos = trace1.endpos;
                
                // Line-of-sight check
                Ray visibilityRay;
                visibilityRay.Init(origin + Vector(0.0f, 0.0f, 64.0f), groundPos + Vector(0.0f, 0.0f, 32.0f));
                Trace visibilityTrace;
                Interfaces::trace->TraceRay(visibilityRay, 0x1, &filter, &visibilityTrace);
                if (visibilityTrace.fraction < 0.98f)
                    continue;
                
                Vector testOut = groundPos + dir * 4.0f;
                
                Ray ray2;
                ray2.Init(testOut + Vector(0.0f, 0.0f, 4.0f), testOut - Vector(0.0f, 0.0f, 64.0f));
                
                Trace trace2;
                Interfaces::trace->TraceRay(ray2, 0x1, &filter, &trace2);
                
                if (trace2.fraction == 1.0f || (groundPos.z - trace2.endpos.z) > 2.0f) {
                    bool duplicate = false;
                    for (const auto& edge : scannedEdges) {
                        if (getDistance(edge.pos, groundPos) < 5.0f) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate) {
                        scannedEdges.push_back({ groundPos });
                    }
                }
            }
        }
    }
}

void Features::Movement::pixelSurf(CUserCmd* cmd) {
    if (!CONFIGBOOL("Misc>Misc>Movement>Pixel Surf")) {
        shouldPixelSurf = false;
        return;
    }

    static int ticks = 0;
    if (!Globals::localPlayer || Globals::localPlayer->health() <= 0) {
        ticks = 0;
        shouldPixelSurf = false;
        return;
    }

    if (Globals::localPlayer->moveType() == MOVETYPE_LADDER || Globals::localPlayer->moveType() == MOVETYPE_NOCLIP) {
        shouldPixelSurf = false;
        return;
    }

    if (Globals::localPlayer->flags() & FL_ONGROUND) {
        shouldPixelSurf = false;
        return;
    }

    // Don't run pixelsurf if edgebug is active/running
    if (CONFIGBOOL("Misc>Misc>Movement>EdgeBug") && Menu::CustomWidgets::isKeyDown(CONFIGINT("Misc>Misc>Movement>EdgeBug Key"))) {
        shouldPixelSurf = false;
        return;
    }

    static ConVar *sv_gravity = Interfaces::convar->FindVar("sv_gravity");
    float targetVelo = (sv_gravity->GetFloat() * 0.5f * Interfaces::globals->interval_per_tick);

    bool locked = shouldPixelSurf && cmd->tick_count >= ticks;

    // Only steer if we are not locked on the ledge yet
    if (!locked) {
        // --- AUTO PIXELSURF WALL DETECTION & STEERING ---
        Vector origin = Globals::localPlayer->origin();
        TraceFilter filter;
        filter.pSkip = Globals::localPlayer;
        
        Vector bestNormal(0.0f, 0.0f, 0.0f);
        float closestWall = 999.0f;
        bool wallFound = false;
        
        for (int i = 0; i < 8; i++) {
            float angle = (float)i * (360.0f / 8.0f);
            float rad = DEG2RAD(angle);
            Vector dir(cos(rad), sin(rad), 0.0f);
            
            Ray ray;
            ray.Init(origin + Vector(0.0f, 0.0f, 32.0f), origin + Vector(0.0f, 0.0f, 32.0f) + dir * 60.0f);
            Trace trace;
            Interfaces::trace->TraceRay(ray, 0x1, &filter, &trace);
            
            if (trace.fraction < 1.0f && !trace.allsolid) {
                // Check if the wall is vertical (pixelsurf ledges are on vertical walls/clips)
                if (std::abs(trace.plane.normal.z) < 0.15f) {
                    float dist = trace.fraction * 60.0f;
                    if (dist < closestWall) {
                        closestWall = dist;
                        bestNormal = trace.plane.normal;
                        wallFound = true;
                    }
                }
            }
        }

        if (wallFound && !(Globals::localPlayer->flags() & FL_ONGROUND)) {
            // Calculate tangent direction parallel to the wall based on view direction
            Vector viewForward(cos(DEG2RAD(cmd->viewangles.y)), sin(DEG2RAD(cmd->viewangles.y)), 0.0f);
            Vector tangent(bestNormal.y, -bestNormal.x, 0.0f);
            if (viewForward.x * tangent.x + viewForward.y * tangent.y < 0.0f) {
                tangent = Vector(-tangent.x, -tangent.y, 0.0f);
            }
            
            // Hug the wall by steering slightly into it (25%) while mostly moving parallel to it (85%)
            Vector steerVec = tangent * 0.85f - bestNormal * 0.25f;
            steerVec.Normalize();
            
            float steerYaw = RAD2DEG(atan2(steerVec.y, steerVec.x));
            float yaw_delta = cmd->viewangles.y - steerYaw;
            float rad = DEG2RAD(yaw_delta);
            
            cmd->forwardmove = cos(rad) * 450.f;
            cmd->sidemove = -sin(rad) * 450.f;
        }
    }

    if (!shouldPixelSurf) {
        int nCmdsPred = Interfaces::prediction->Split->nCommandsPredicted;
        int BackupButtons = cmd->buttons;

        // Restore prediction state to the last predicted frame
        Features::Prediction::restoreEntityToPredictedFrame(nCmdsPred - 1);

        cmd->buttons |= IN_DUCK;

        float lastPredictedVeloZ = velBackup.z;

        for (int z = 0; z < 8; z++) {
            Features::Prediction::start(cmd);
            Features::Prediction::end();

            if (Globals::localPlayer->flags() & FL_ONGROUND) {
                break;
            }

            float zVelo = Globals::localPlayer->velocity().z;
            float precedingVeloZ = lastPredictedVeloZ;
            lastPredictedVeloZ = zVelo;

            // Detect exact vertical velocity signature of a pixelsurf dynamically
            if (precedingVeloZ < 10.0f && std::abs(zVelo + targetVelo) < 1.0f) {
                shouldPixelSurf = true;
                ticks = cmd->tick_count + z; // Predict target tick precisely without delay offset
                BackupButtons = cmd->buttons;
                Features::Notifications::addNotification(ImColor(0, 255, 255), "PixelSurf predicted at tick offset: %d", z);
                break;
            }
        }

        cmd->buttons = BackupButtons;
        Features::Prediction::restoreEntityToPredictedFrame(nCmdsPred - 1);
    } else {
        // We predicted a pixelsurf, so we must crouch immediately to match the predicted trajectory
        cmd->buttons |= IN_DUCK;

        // Once we reach the target predicted tick, lock movement inputs to stick to the ledge
        if (cmd->tick_count >= ticks) {
            cmd->forwardmove = 0.0f;
            cmd->sidemove = 0.0f;
        }
        
        if (cmd->tick_count > ticks) {
            if (std::abs(velBackup.z + targetVelo) > 1.0f) {
                shouldPixelSurf = false;
            }
        }
    }
}

void Features::Movement::prePredCreateMove(CUserCmd *cmd) {
    if (!Globals::localPlayer)
        return;

    if (Interfaces::globals->tickcount % 8 == 0) {
        scanForEdges();
    }

    flagsBackup = Globals::localPlayer->flags();
    velBackup = Globals::localPlayer->velocity();

    bhop(cmd);
    autoStrafe(cmd);
    fastStop(cmd);
    pixelSurf(cmd);

    if (shouldEdgebug && shouldDuckNext)
        cmd->buttons |= IN_DUCK;

    // Movement Trail recording
    if (Globals::localPlayer->health() > 0) {
        if (CONFIGBOOL("Visuals>World>World>Movement Trail")) {
            if (!(Globals::localPlayer->flags() & FL_ONGROUND)) {
                movementTrail.push_back({ Globals::localPlayer->origin(), static_cast<float>(ImGui::GetTime()) });
            }
        }
    } else {
        movementTrail.clear();
    }

    float curTime = ImGui::GetTime();
    movementTrail.erase(std::remove_if(movementTrail.begin(), movementTrail.end(), 
        [curTime](const TrailPoint& p) { return curTime - p.time > 1.0f; }), movementTrail.end());

    // PixelSurf Lightning Trigger (rising edge detection)
    static bool wasPixelSurfing = false;
    if (shouldPixelSurf && !wasPixelSurfing) {
        if (CONFIGBOOL("Visuals>World>World>PixelSurf Effects")) {
            LightningStrike strike;
            strike.origin = Globals::localPlayer->origin();
            strike.startTime = curTime;
            
            int numBranches = 4 + (rand() % 3); // 4 to 6 branches
            for (int b = 0; b < numBranches; b++) {
                std::vector<Vector> branch;
                Vector current = strike.origin;
                branch.push_back(current);
                
                int segments = 4 + (rand() % 3); // 4 to 6 segments per branch
                for (int s = 0; s < segments; s++) {
                    float offsetX = ((rand() % 200) - 100) * 0.4f; // -40 to 40
                    float offsetY = ((rand() % 200) - 100) * 0.4f; // -40 to 40
                    float offsetZ = 20.0f + (rand() % 40); // 20 to 60
                    
                    current = current + Vector(offsetX, offsetY, offsetZ);
                    branch.push_back(current);
                }
                strike.branches.push_back(branch);
            }
            activeStrikes.push_back(strike);
        }
    }
    wasPixelSurfing = shouldPixelSurf;
}

void Features::Movement::postPredCreateMove(CUserCmd *cmd) {
    if (!Globals::localPlayer || Globals::localPlayer->moveType() == MOVETYPE_LADDER ||
        Globals::localPlayer->moveType() == MOVETYPE_NOCLIP)
        return;

    edgeJump(cmd);
    jumpBug(cmd);
    autoPeek(cmd);
}

void Features::Movement::edgeBugPredictor(CUserCmd *cmd) {
    if (!CONFIGBOOL("Misc>Misc>Movement>EdgeBug") ||
        !Menu::CustomWidgets::isKeyDown(CONFIGINT("Misc>Misc>Movement>EdgeBug Key")) ||
        !Globals::localPlayer->health())
        return;

    if (Globals::localPlayer->flags() & FL_ONGROUND)
        return;

    if (Globals::localPlayer->velocity().z >= 0.0f)
        return;

    // --- AUTO EDGEBUG / EDGEBUG HELPER STEERING (IMPROVED) ---
    Vector playerOrigin = Globals::localPlayer->origin();
    Vector playerVel = Globals::localPlayer->velocity();
    
    // Use velocity direction for edge selection when moving, view direction as fallback
    Vector moveDir;
    float speed2D = playerVel.Length2D();
    if (speed2D > 10.0f) {
        moveDir = Vector(playerVel.x, playerVel.y, 0.0f);
        moveDir.Normalize();
    } else {
        angleVectors(cmd->viewangles, moveDir);
        moveDir.z = 0.0f;
        moveDir.Normalize();
    }

    Vector targetLedge(0.0f, 0.0f, 0.0f);
    float bestEdgeScore = -999999.0f;
    bool foundEdge = false;

    for (const auto& edge : scannedEdges) {
        Vector vecToEdge = edge.pos - playerOrigin;
        float dist = vecToEdge.Length2D();
        
        if (dist > 300.0f)
            continue;
            
        if (edge.pos.z >= playerOrigin.z + 20.0f || edge.pos.z < playerOrigin.z - 350.0f)
            continue;
            
        Vector vecToEdge2D = vecToEdge;
        vecToEdge2D.z = 0.0f;
        vecToEdge2D.Normalize();
        
        float dot = vecToEdge2D.Dot(moveDir);
        if (dot > 0.5f) { // ±60 degree cone
            // Score: prefer edges that are closer AND more aligned with movement
            float score = dot * 100.0f - dist;
            if (score > bestEdgeScore) {
                bestEdgeScore = score;
                targetLedge = edge.pos;
                foundEdge = true;
            }
        }
    }

    if (foundEdge && playerVel.z < -30.0f) {
        Vector edgeToPlayer = playerOrigin - targetLedge;
        edgeToPlayer.z = 0.0f;
        float distToEdge = edgeToPlayer.Length();
        
        if (distToEdge > 0.05f) {
            Vector dir = edgeToPlayer / distToEdge;
            // Position 1 unit from edge — tight enough for consistent edgebugs
            Vector idealP = targetLedge + dir * 1.0f;
            Vector steerVec = idealP - playerOrigin;
            steerVec.z = 0.0f;
            float steerDist = steerVec.Length();
            
            if (steerDist > 0.1f) {
                steerVec.Normalize();
                float steerYaw = RAD2DEG(atan2(steerVec.y, steerVec.x));
                float yaw_delta = cmd->viewangles.y - steerYaw;
                float rad = DEG2RAD(yaw_delta);
                
                // Adaptive steering: strong when far, gentle when close
                float steerForce = std::clamp(steerDist * 15.0f, 30.0f, 450.0f);
                
                cmd->forwardmove = cos(rad) * steerForce;
                cmd->sidemove = -sin(rad) * steerForce;
            }
        }
    }
    // --------------------------------------------------------

    struct MovementVars {
        QAngle viewangles;
        QAngle view_delta;
        float forwardmove;
        float sidemove;
        int buttons;
    };
    static MovementVars backup_move;
    MovementVars original_move;
    original_move.viewangles = cmd->viewangles;
    original_move.view_delta = (cmd->viewangles - Globals::oldViewangles);
    original_move.forwardmove = cmd->forwardmove;
    original_move.sidemove = cmd->sidemove;
    original_move.buttons = cmd->buttons;
    if (!shouldEdgebug)
        backup_move = original_move;

    int nCmdsPred = Interfaces::prediction->Split->nCommandsPredicted;

    int predictAmount = 64; // TODO: make amount configurable
    for (int t = 0; t < 4; t++) {
        Features::Prediction::restoreEntityToPredictedFrame(nCmdsPred - 1);

        bool doStrafe = (t % 2 == 0);
        bool doDuck = t > 1;

        cmd->viewangles = backup_move.viewangles;

        for (int i = 0; i < predictAmount; i++) {
            if (doStrafe) {
                cmd->viewangles += backup_move.view_delta;
                cmd->forwardmove = backup_move.forwardmove;
                cmd->sidemove = backup_move.sidemove;
            } else {
                cmd->forwardmove = 0.f;
                cmd->sidemove = 0.f;
            }
            if (doDuck)
                cmd->buttons |= IN_DUCK;
            else
                cmd->buttons &= ~IN_DUCK;

            Features::Prediction::start(cmd);
            shouldEdgebug = checkEdgebug();
            velBackup = Globals::localPlayer->velocity();
            edgebugPos = Globals::localPlayer->origin();
            Features::Prediction::end();
            if (Globals::localPlayer->flags() & FL_ONGROUND || Globals::localPlayer->moveType() == MOVETYPE_LADDER) {
                break;
            }
            if (shouldEdgebug) {
                shouldDuckNext = doDuck;
                if (doStrafe) {
                    cmd->viewangles = backup_move.viewangles + backup_move.view_delta;
                    backup_move.viewangles = cmd->viewangles;
                }
                if (i == 1)
                    Interfaces::engine->ExecuteClientCmd(
                       "play buttons/blip1.wav"); // TODO: play sound via a better method
                return;
            }
        }
    }

    cmd->viewangles = original_move.viewangles;
    cmd->forwardmove = original_move.forwardmove;
    cmd->sidemove = original_move.sidemove;
    cmd->buttons = original_move.buttons;
}

void Features::Movement::draw() {
    if (Features::Movement::shouldEdgebug) {
        Globals::drawList->AddText(
           ImVec2((Globals::screenSizeX / 2) - (ImGui::CalcTextSize("EdgeBug").x / 2) + 1,
                  (Globals::screenSizeY / 2) + 31),
           ImColor(0, 0, 0, 255), "EdgeBug");
        Globals::drawList->AddText(
           ImVec2((Globals::screenSizeX / 2) - (ImGui::CalcTextSize("EdgeBug").x / 2),
                  (Globals::screenSizeY / 2) + 30),
           ImColor(255, 255, 255, 255), "EdgeBug");

        Vector edgebugPos2D;
        if (worldToScreen(edgebugPos, edgebugPos2D)) {
            Globals::drawList->AddText(
               ImVec2(edgebugPos2D.x - (ImGui::CalcTextSize("gaming").x / 2) + 1,
                      edgebugPos2D.y + 1),
               ImColor(0, 0, 0, 255), "gaming");
            Globals::drawList->AddText(
               ImVec2(edgebugPos2D.x - (ImGui::CalcTextSize("gaming").x / 2),
                      edgebugPos2D.y),
               ImColor(255, 255, 255, 255), "gaming");
        }
    }

    if (Features::Movement::shouldPixelSurf) {
        Globals::drawList->AddText(
           ImVec2((Globals::screenSizeX / 2) - (ImGui::CalcTextSize("PixelSurf").x / 2) + 1,
                  (Globals::screenSizeY / 2) + 51),
           ImColor(0, 0, 0, 255), "PixelSurf");
        Globals::drawList->AddText(
           ImVec2((Globals::screenSizeX / 2) - (ImGui::CalcTextSize("PixelSurf").x / 2),
                  (Globals::screenSizeY / 2) + 50),
            ImColor(0, 255, 255, 255), "PixelSurf");
    }

    // Movement Trail rendering
    if (CONFIGBOOL("Visuals>World>World>Movement Trail") && Globals::localPlayer) {
        float curTime = ImGui::GetTime();
        ImColor baseColor = CONFIGCOL("Visuals>World>World>Movement Trail Color");
        
        for (size_t i = 0; i + 1 < movementTrail.size(); i++) {
            Vector p1, p2;
            if (worldToScreen(movementTrail[i].pos, p1) && worldToScreen(movementTrail[i+1].pos, p2)) {
                float age = curTime - movementTrail[i].time;
                float alpha = std::clamp(1.0f - age / 1.0f, 0.0f, 1.0f);
                if (alpha <= 0.0f) continue;
                
                // Draw a beautiful glowing trail
                ImColor glowColor = ImColor(baseColor.Value.x, baseColor.Value.y, baseColor.Value.z, baseColor.Value.w * alpha * 0.35f);
                ImColor coreColor = ImColor(255, 255, 255, (int)(alpha * 220.f));
                
                Globals::drawList->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), glowColor, 4.0f);
                Globals::drawList->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), coreColor, 1.2f);
            }
        }
    }

    // PixelSurf Lightning rendering
    if (CONFIGBOOL("Visuals>World>World>PixelSurf Effects")) {
        float curTime = ImGui::GetTime();
        ImColor baseColor = CONFIGCOL("Visuals>World>World>PixelSurf Effects Color");
        
        // Clean up expired strikes
        activeStrikes.erase(std::remove_if(activeStrikes.begin(), activeStrikes.end(), 
            [curTime](const LightningStrike& s) { return curTime - s.startTime > 0.40f; }), activeStrikes.end());
            
        for (const auto& strike : activeStrikes) {
            float age = curTime - strike.startTime;
            float alpha = std::clamp(1.0f - age / 0.40f, 0.0f, 1.0f);
            if (alpha <= 0.0f) continue;
            
            for (const auto& branch : strike.branches) {
                for (size_t i = 0; i + 1 < branch.size(); i++) {
                    Vector p1, p2;
                    if (worldToScreen(branch[i], p1) && worldToScreen(branch[i+1], p2)) {
                        // Cyan/blue lightning glow with white hot core
                        ImColor glowColor = ImColor(baseColor.Value.x, baseColor.Value.y, baseColor.Value.z, baseColor.Value.w * alpha * 0.7f);
                        ImColor coreColor = ImColor(255, 255, 255, (int)(alpha * 255.f));
                        
                        Globals::drawList->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), glowColor, 5.0f);
                        Globals::drawList->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), coreColor, 1.5f);
                    }
                }
            }
        }
    }

    if (CONFIGBOOL("Misc>Misc>Movement>EdgeBug Finder") && Globals::localPlayer && Globals::localPlayer->health() > 0) {
        Vector playerOrigin = Globals::localPlayer->origin();
        
        Vector forward;
        QAngle viewAngles;
        Interfaces::engine->GetViewAngles(viewAngles);
        angleVectors(viewAngles, forward);
        forward.z = 0.0f;
        forward.Normalize();
        
        for (const auto& edge : scannedEdges) {
            Vector vecToEdge = edge.pos - playerOrigin;
            float dist = vecToEdge.Length2D();
            
            if (dist > 450.0f)
                continue;
                
            if (edge.pos.z >= playerOrigin.z + 16.0f || edge.pos.z < playerOrigin.z - 400.0f)
                continue;
                
            Vector vecToEdge2D = vecToEdge;
            vecToEdge2D.z = 0.0f;
            vecToEdge2D.Normalize();
            
            float dot = vecToEdge2D.Dot(forward);
            if (dot > 0.5f) {
                draw3DBox(edge.pos, 4.0f, ImColor(255, 0, 255, 255));
            }
        }
    }



    if (CONFIGBOOL("Misc>Misc>Movement>Speed Indicator") && Globals::localPlayer && Globals::localPlayer->health() > 0) {
        int speed = round(Globals::localPlayer->velocity().Length2D());
        std::string speedStr = std::to_string(speed) + " u/s";
        
        ImVec2 textSize = ImGui::CalcTextSize(speedStr.c_str());
        ImVec2 pos((Globals::screenSizeX / 2.0f) - (textSize.x / 2.0f), Globals::screenSizeY - 150.0f);
        
        Globals::drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), ImColor(0, 0, 0, 255), speedStr.c_str());
        Globals::drawList->AddText(pos, ImColor(255, 255, 255, 255), speedStr.c_str());
    }

    if (CONFIGBOOL("Misc>Misc>Movement>Auto Strafe") && Globals::localPlayer && Globals::localPlayer->health() > 0) {
        std::string modeStr = "";
        if (currentStrafeMode == STRAFE_RIGHT) {
            modeStr = "Right Sideways";
        } else if (currentStrafeMode == STRAFE_LEFT) {
            modeStr = "Left Sideways";
        }
        
        if (!modeStr.empty()) {
            std::string fullStr = "Strafe Mode: " + modeStr;
            ImVec2 textSize = ImGui::CalcTextSize(fullStr.c_str());
            ImVec2 pos((Globals::screenSizeX / 2.0f) - (textSize.x / 2.0f), Globals::screenSizeY - 180.0f);
            Globals::drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), ImColor(0, 0, 0, 255), fullStr.c_str());
            Globals::drawList->AddText(pos, ImColor(0, 255, 255, 255), fullStr.c_str());
        }
    }

    if (CONFIGBOOL("Rage>Enabled") && CONFIGBOOL("Rage>RageBot>Default>AutoPeek") && hasStartPos && Globals::localPlayer && Globals::localPlayer->health() > 0) {
        ImColor circleColor = hasShot ? ImColor(0, 255, 0, 150) : CONFIGCOL("Rage>RageBot>Default>AutoPeekColor");
        
        Vector prevScreenPos;
        bool hasPrev = false;
        for (int i = 0; i <= 32; i++) {
            float angle = (i / 32.f) * (3.14159265f * 2.f);
            Vector point = peekStartPos + Vector(cos(angle) * 20.f, sin(angle) * 20.f, 0.f);
            Vector screenPos;
            if (worldToScreen(point, screenPos)) {
                if (hasPrev) {
                    Globals::drawList->AddLine(ImVec2(prevScreenPos.x, prevScreenPos.y), ImVec2(screenPos.x, screenPos.y), circleColor, 2.0f);
                }
                prevScreenPos = screenPos;
                hasPrev = true;
            } else {
                hasPrev = false;
            }
        }
    }
}

void Features::Movement::autoPeek(CUserCmd *cmd) {
    if (!CONFIGBOOL("Rage>Enabled") || !CONFIGBOOL("Rage>RageBot>Default>AutoPeek")) {
        hasStartPos = false;
        hasShot = false;
        return;
    }

    if (!Globals::localPlayer || Globals::localPlayer->health() <= 0) {
        hasStartPos = false;
        hasShot = false;
        return;
    }

    bool keyHeld = Menu::CustomWidgets::isKeyDown(CONFIGINT("Rage>RageBot>Default>AutoPeekKey"));
    if (keyHeld) {
        if (!hasStartPos) {
            peekStartPos = Globals::localPlayer->origin();
            hasStartPos = true;
        }

        if (cmd->buttons & IN_ATTACK) {
            hasShot = true;
        }

        if (hasShot) {
            Vector delta = peekStartPos - Globals::localPlayer->origin();
            if (delta.Length2D() > 5.0f) {
                // Calculate absolute movement vector to go from origin to peekStartPos
                // targetYaw is the absolute direction angle in degrees
                float targetYaw = RAD2DEG(atan2(delta.y, delta.x));
                
                // We want to walk towards targetYaw.
                // Normally, cmd->forwardmove moves along cmd->viewangles.y, and cmd->sidemove moves along cmd->viewangles.y + 90.
                // To do this independently of current viewangles, we calculate movement relative to cmd->viewangles.y.
                float deltaYaw = targetYaw - cmd->viewangles.y;
                
                // Align deltaYaw to range [-180, 180]
                while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
                while (deltaYaw < -180.0f) deltaYaw += 360.0f;
                
                // Convert angle direction to forwardmove and sidemove values
                // Negate the sine calculation because in Source Engine, positive sidemove goes RIGHT
                // but a positive trigonometric angle rotation goes LEFT.
                cmd->forwardmove = cos(DEG2RAD(deltaYaw)) * 450.0f;
                cmd->sidemove = -sin(DEG2RAD(deltaYaw)) * 450.0f;

                // Sync with original angles and moves so endMovementFix works correctly
                originalForwardMove = cmd->forwardmove;
                originalSideMove = cmd->sidemove;
                originalAngle = cmd->viewangles;
            } else {
                hasShot = false;
            }
        }
    } else {
        hasStartPos = false;
        hasShot = false;
    }
}