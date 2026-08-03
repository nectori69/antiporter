#pragma once

#include "particle_types.h"

// Forward declarations.
class CBaseParticle;

//------------------------------------------------------------------------------
// Particle utility lifecycle
//------------------------------------------------------------------------------

// Loads and caches every sprite referenced by particle_types.cpp.
//
// Call this from HUD_VidInit(), not by including any .cpp file.
void ParticleUtil_Init();

// Clears cached sprite handles and model pointers.
//
// This does not need to destroy active particles; ParticleMan owns those.
void ParticleUtil_Shutdown();

// Returns true once ParticleUtil_Init() has successfully run.
bool ParticleUtil_IsInitialized();

//------------------------------------------------------------------------------
// Basic preset emission
//------------------------------------------------------------------------------

// Spawns particles using the defaults from particle_types.cpp.
//
// Parameters:
//     type      Particle preset.
//     count     Number of particles to emit.
//     origin    World-space emission position.
//     velocity  Base world-space velocity applied to every particle.
//
// Example:
//
//     SpawnParticles(
//         ParticleType::MuzzleFlash,
//         4,
//         muzzleOrigin,
//         forward * 30.0f);
void SpawnParticles(
	ParticleType type,
	int count,
	const Vector& origin,
	const Vector& velocity);

// Spawns particles using the defaults from particle_types.cpp, with random
// per-axis velocity variation.
//
// Each particle receives:
//
//     velocity + random(-velocityRandom, +velocityRandom)
//
// Example:
//
//     SpawnParticles(
//         ParticleType::Spark,
//         12,
//         impactOrigin,
//         impactNormal * 180.0f,
//         Vector(70.0f, 70.0f, 70.0f));
void SpawnParticles(
	ParticleType type,
	int count,
	const Vector& origin,
	const Vector& velocity,
	const Vector& velocityRandom);

//------------------------------------------------------------------------------
// Fully configurable emission
//------------------------------------------------------------------------------

// Spawns particles using the preset's sprite, but all behavioral values are
// supplied explicitly at the call site.
//
// All ranges are randomized separately for each emitted particle.
//
// Parameters:
//
//     type
//         Selects the preset sprite. All other values in this overload are
//         supplied explicitly.
//
//     count
//         Number of particles emitted.
//
//     origin
//         Base world-space position.
//
//     originRandom
//         Random world-space position variation on each axis.
//         Vector(2, 2, 2) means each axis varies from -2 to +2.
//
//     velocity
//         Base world-space velocity.
//
//     velocityRandom
//         Random world-space velocity variation on each axis.
//
//     gravity
//         Multiplier applied to the engine's gravity.
//         0.0 = no gravity.
//         1.0 = normal downward gravity.
//        -1.0 = upward acceleration.
//
//     lifetimeMin / lifetimeMax
//         Particle lifetime range in seconds.
//
//     sizeMin / sizeMax
//         Initial particle-size range.
//
//     brightnessMin / brightnessMax
//         Initial alpha/brightness range, normally 0 through 255.
//
//     scaleSpeedMin / scaleSpeedMax
//         Expansion-speed range.
//         Positive values expand.
//         Zero disables expansion.
//
//     contractSpeedMin / contractSpeedMax
//         Contraction-speed range.
//         Positive values shrink the particle.
//         Zero disables contraction.
//
//     fadeSpeedMin / fadeSpeedMax
//         Fade-speed range.
//         Use 0 for automatic fading across the particle's lifetime.
//         Use -1 to disable fading.
//
//     colorMin / colorMax
//         Random RGB color range.
//         ParticleMan expects components in the 0 through 255 range.
//
//     initialAnglesMin / initialAnglesMax
//         Random initial Euler-angle range. For RENDER_FACEPLAYER_ROTATEZ,
//         the Z component controls the sprite's starting rotation.
//
//     angularVelocityMin / angularVelocityMax
//         Random angular velocity range. For billboard particles, the Z
//         component controls spin over time.
//
//     stretchXMin / stretchXMax
//         Horizontal sprite-stretch range.
//
//     stretchYMin / stretchYMax
//         Vertical sprite-stretch range.
//
//     renderMode
//         GoldSrc render mode, such as:
//             kRenderNormal
//             kRenderTransColor
//             kRenderTransTexture
//             kRenderGlow
//             kRenderTransAlpha
//             kRenderTransAdd
//
//     renderFlags
//         ParticleMan render flags combined with bitwise OR, such as:
//             RENDER_FACEPLAYER
//             RENDER_FACEPLAYER_ROTATEZ
//             LIGHT_NONE
//             CULL_PVS
//             CULL_FRUSTUM_SPHERE
//
//     collisionFlags
//         ParticleMan collision flags combined with bitwise OR, such as:
//             TRI_COLLIDEWORLD
//             TRI_COLLIDEALL
//             TRI_COLLIDEKILL
//             TRI_WATERTRACE
//             TRI_SPIRAL
//             TRI_ANIMATEDIE
//
//     bounceFactor
//         Collision bounce multiplier.
//
//     affectedByForce
//         Whether ParticleMan forces can affect the particle.
//
//     framerate
//         Sprite animation rate. Use zero for no animation.
//
//     classname
//         Debug/effect classification string, limited internally to 31 chars.
void SpawnParticlesEx(
	ParticleType type,
	int count,

	const Vector& origin,
	const Vector& originRandom,

	const Vector& velocity,
	const Vector& velocityRandom,

	float gravity,

	float lifetimeMin,
	float lifetimeMax,

	float sizeMin,
	float sizeMax,

	float brightnessMin,
	float brightnessMax,

	float scaleSpeedMin,
	float scaleSpeedMax,

	float contractSpeedMin,
	float contractSpeedMax,

	float fadeSpeedMin,
	float fadeSpeedMax,

	const Vector& colorMin,
	const Vector& colorMax,

	const Vector& initialAnglesMin,
	const Vector& initialAnglesMax,

	const Vector& angularVelocityMin,
	const Vector& angularVelocityMax,

	float stretchXMin,
	float stretchXMax,

	float stretchYMin,
	float stretchYMax,

	int renderMode,
	int renderFlags,

	int collisionFlags,
	float bounceFactor,

	bool affectedByForce,

	int framerate,

	const char* classname);

//------------------------------------------------------------------------------
// Directional emission
//------------------------------------------------------------------------------

// Emits a cone of particles relative to a direction.
//
// This is useful for muzzle flashes, smoke, blood sprays, sparks, and impacts.
//
// The function constructs forward/right/up vectors internally.
//
// Parameters:
//
//     direction
//         Center direction of the emission cone.
//
//     speedMin / speedMax
//         Forward-speed range.
//
//     rightSpread
//         Random sideways velocity amount.
//
//     upSpread
//         Random vertical velocity amount.
//
// Remaining behavior comes from the selected particle preset.
void SpawnParticleCone(
	ParticleType type,
	int count,

	const Vector& origin,
	const Vector& direction,

	float speedMin,
	float speedMax,

	float rightSpread,
	float upSpread);

// Fully configurable cone emitter.
//
// Velocity is calculated as:
//
//     normalizedDirection * random(speedMin, speedMax)
//     + right * random(-rightSpread, rightSpread)
//     + up    * random(-upSpread, upSpread)
//
// After that, velocityRandom is added independently on each world-space axis.
void SpawnParticleConeEx(
	ParticleType type,
	int count,

	const Vector& origin,
	const Vector& originRandom,

	const Vector& direction,

	float speedMin,
	float speedMax,

	float rightSpread,
	float upSpread,

	const Vector& velocityRandom,

	float gravity,

	float lifetimeMin,
	float lifetimeMax,

	float sizeMin,
	float sizeMax,

	float brightnessMin,
	float brightnessMax,

	float scaleSpeedMin,
	float scaleSpeedMax,

	float contractSpeedMin,
	float contractSpeedMax,

	float fadeSpeedMin,
	float fadeSpeedMax,

	const Vector& colorMin,
	const Vector& colorMax,

	const Vector& initialAnglesMin,
	const Vector& initialAnglesMax,

	const Vector& angularVelocityMin,
	const Vector& angularVelocityMax,

	float stretchXMin,
	float stretchXMax,

	float stretchYMin,
	float stretchYMax,

	int renderMode,
	int renderFlags,

	int collisionFlags,
	float bounceFactor,

	bool affectedByForce,

	int framerate,

	const char* classname);