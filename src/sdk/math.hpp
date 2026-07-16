#pragma once
#include "sdk.hpp"

inline QAngle originalAngle;
inline float originalForwardMove, originalSideMove;

inline void startMovementFix(CUserCmd* cmd) {
    originalAngle = cmd->viewangles;
    originalForwardMove = cmd->forwardmove;
    originalSideMove = cmd->sidemove;
}

inline void endMovementFix(CUserCmd* cmd) {
    // Correct absolute yaw rotation correction
    float deltaYaw = cmd->viewangles.y - originalAngle.y;
    float deltaYawRad = DEG2RAD(deltaYaw);
    
    float originalForward = originalForwardMove;
    float originalSide = originalSideMove;
    
    // Standard 2D vector rotation projection formula
    cmd->forwardmove = cos(deltaYawRad) * originalForward - sin(deltaYawRad) * originalSide;
    cmd->sidemove = sin(deltaYawRad) * originalForward + cos(deltaYawRad) * originalSide;
}

inline void normalizeAngles(QAngle& angle) {
	while (angle.x > 89.0f)
		angle.x -= 180.f;

	while (angle.x < -89.0f)
		angle.x += 180.f;

	while (angle.y > 180.f)
		angle.y -= 360.f;

	while (angle.y < -180.f)
		angle.y += 360.f;
}

inline QAngle calcAngle(const Vector& src, const Vector& dst) {
	QAngle vAngle;
	Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
	double hyp = sqrt(delta.x*delta.x + delta.y*delta.y);

	vAngle.x = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
	vAngle.y = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
	vAngle.z = 0.0f;

	if (delta.x >= 0.0)
		vAngle.y += 180.0f;

	return vAngle;
}

inline void angleVectors(const QAngle &angles, Vector& forward) {
	forward.x = cos(DEG2RAD(angles.x)) * cos(DEG2RAD(angles.y));
	forward.y = cos(DEG2RAD(angles.x)) * sin(DEG2RAD(angles.y));
	forward.z = -sin(DEG2RAD(angles.x));
}

inline float getDistance(Vector pos1, Vector pos2) {
    // Do 3d pythag
    float a = abs(pos1.x-pos2.x);
    float b = abs(pos1.y-pos2.y);
    float c = abs(pos1.z-pos2.z);
    return sqrt(pow(a, 2.f) + pow(b, 2.f) + pow(c, 2.f));
}

inline float getDistanceNoSqrt(Vector pos1, Vector pos2) {
    // When you dont need an exact distance and just want to see if 
	// something is x further than something else for example theres no need to sqrt it
    float a = abs(pos1.x-pos2.x);
    float b = abs(pos1.y-pos2.y);
    float c = abs(pos1.z-pos2.z);
    return pow(a, 2.f) + pow(b, 2.f) + pow(c, 2.f);
}

bool worldToScreen(const Vector& origin, Vector& screen);