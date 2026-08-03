#pragma once

#include "../dlls/vector.h"

//------------------------------------------------------------------------------
// What kind of particle this is.
//------------------------------------------------------------------------------

enum class ParticleType
{
	MuzzleFlash,
	MuzzleSmokePistol,
	MuzzleSmokeRifle,
	MuzzleSpark,
	BulletDust,
	Blood,
	Custom
};

//------------------------------------------------------------------------------
// Default particle definition.
//
// Every value used by the basic emitters lives here. The extended emitters
// remain available when a call site needs to override these defaults.
//------------------------------------------------------------------------------

struct ParticleDef
{
	const char* sprite;
	const char* classname;

	float size;
	float brightness;
	float lifetime;

	float gravity;

	float scaleSpeed;
	float contractSpeed;
	float fadeSpeed;

	Vector color;

	Vector initialAnglesMin;
	Vector initialAnglesMax;

	Vector angularVelocityMin;
	Vector angularVelocityMax;

	float stretchX;
	float stretchY;

	int renderMode;
	int renderFlags;

	int collisionFlags;
	float bounceFactor;

	bool affectedByForce;

	int framerate;
};

// Returns the definition for a particle preset.
const ParticleDef& GetParticleDef(ParticleType type);
