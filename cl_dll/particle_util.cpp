#include "hud.h"
#include "cl_util.h"

#include <cmath>

#include "triangleapi.h"
#include "particleman.h"

#include "particle_types.h"
#include "particle_util.h"

//------------------------------------------------------------------------------
// Internal sprite cache
//------------------------------------------------------------------------------

namespace
{
const int PARTICLE_TYPE_COUNT =
	static_cast<int>(ParticleType::Custom) + 1;

struct CachedParticleSprite
{
	HSPRITE handle;
	model_s* model;
	int frameCount;
	bool attemptedLoad;
};

CachedParticleSprite g_ParticleSprites[PARTICLE_TYPE_COUNT];
bool g_ParticleUtilInitialized = false;

//--------------------------------------------------------------------------
// Random helpers
//--------------------------------------------------------------------------

float RandomFloat(float minimum, float maximum)
{
	if (maximum < minimum)
	{
		const float temporary = minimum;
		minimum = maximum;
		maximum = temporary;
	}

	if (minimum == maximum)
	{
		return minimum;
	}

	return gEngfuncs.pfnRandomFloat(minimum, maximum);
}

float RandomSymmetric(float amount)
{
	const float absoluteAmount =
		amount < 0.0f ? -amount : amount;

	return RandomFloat(-absoluteAmount, absoluteAmount);
}

Vector RandomVector(const Vector& minimum, const Vector& maximum)
{
	return Vector(
		RandomFloat(minimum.x, maximum.x),
		RandomFloat(minimum.y, maximum.y),
		RandomFloat(minimum.z, maximum.z));
}

Vector RandomSymmetricVector(const Vector& amount)
{
	return Vector(
		RandomSymmetric(amount.x),
		RandomSymmetric(amount.y),
		RandomSymmetric(amount.z));
}

float ClampMinimum(float value, float minimum)
{
	return value < minimum ? minimum : value;
}

float ClampRange(float value, float minimum, float maximum)
{
	if (value < minimum)
	{
		return minimum;
	}

	if (value > maximum)
	{
		return maximum;
	}

	return value;
}

//--------------------------------------------------------------------------
// Vector helpers
//--------------------------------------------------------------------------

float VectorLengthSquared(const Vector& value)
{
	return value.x * value.x +
		   value.y * value.y +
		   value.z * value.z;
}

Vector NormalizeDirection(const Vector& direction)
{
	const float lengthSquared = VectorLengthSquared(direction);

	if (lengthSquared <= 0.000001f)
	{
		return Vector(1.0f, 0.0f, 0.0f);
	}

	const float inverseLength =
		1.0f / static_cast<float>(std::sqrt(lengthSquared));

	return direction * inverseLength;
}

Vector CrossProductLocal(const Vector& first, const Vector& second)
{
	return Vector(
		first.y * second.z - first.z * second.y,
		first.z * second.x - first.x * second.z,
		first.x * second.y - first.y * second.x);
}

void BuildDirectionBasis(
	const Vector& direction,
	Vector& forward,
	Vector& right,
	Vector& up)
{
	forward = NormalizeDirection(direction);

	Vector referenceUp(0.0f, 0.0f, 1.0f);

	// Forward is nearly vertical, so use another reference axis.
	if (forward.z > 0.999f || forward.z < -0.999f)
	{
		referenceUp = Vector(0.0f, 1.0f, 0.0f);
	}

	right = NormalizeDirection(
		CrossProductLocal(forward, referenceUp));

	up = NormalizeDirection(
		CrossProductLocal(right, forward));
}

//--------------------------------------------------------------------------
// Type validation
//--------------------------------------------------------------------------

bool IsValidParticleType(ParticleType type)
{
	const int index = static_cast<int>(type);

	return index >= 0 && index < PARTICLE_TYPE_COUNT;
}

int ParticleTypeToIndex(ParticleType type)
{
	if (!IsValidParticleType(type))
	{
		return static_cast<int>(ParticleType::MuzzleFlash);
	}

	return static_cast<int>(type);
}

//--------------------------------------------------------------------------
// Sprite loading
//--------------------------------------------------------------------------

model_s* LoadParticleSprite(ParticleType type, int* frameCount)
{
	const int index = ParticleTypeToIndex(type);
	CachedParticleSprite& cached = g_ParticleSprites[index];

	if (cached.attemptedLoad)
	{
		if (frameCount != nullptr)
		{
			*frameCount = cached.frameCount;
		}

		return cached.model;
	}

	cached.attemptedLoad = true;

	const ParticleDef& definition = GetParticleDef(type);

	if (definition.sprite == nullptr || definition.sprite[0] == '\0')
	{
		gEngfuncs.Con_DPrintf(
			"ParticleUtil: Particle type %d has no sprite path\n",
			index);

		return nullptr;
	}

	cached.handle = gEngfuncs.pfnSPR_Load(definition.sprite);

	if (!cached.handle)
	{
		gEngfuncs.Con_Printf(
			"ParticleUtil: Failed to load sprite \"%s\"\n",
			definition.sprite);

		return nullptr;
	}

	// GetSpritePointer returns const model_s*, while ParticleMan's older
	// CreateParticle interface accepts model_s*. ParticleMan does not
	// modify the model, so removing constness here is safe.
	const model_s* spriteModel =
		gEngfuncs.GetSpritePointer(cached.handle);

	cached.model = const_cast<model_s*>(spriteModel);

	if (cached.model == nullptr)
	{
		gEngfuncs.Con_Printf(
			"ParticleUtil: GetSpritePointer failed for \"%s\"\n",
			definition.sprite);

		cached.handle = 0;
		return nullptr;
	}

	cached.frameCount =
		gEngfuncs.pfnSPR_Frames(cached.handle);

	if (cached.frameCount < 1)
	{
		cached.frameCount = 1;
	}

	if (frameCount != nullptr)
	{
		*frameCount = cached.frameCount;
	}

	return cached.model;
}

//--------------------------------------------------------------------------
// Shared particle creation
//--------------------------------------------------------------------------

CBaseParticle* CreateConfiguredParticle(
	ParticleType type,
	const Vector& origin,
	const Vector& velocity,
	float gravity,
	float lifetime,
	float size,
	float brightness,
	float scaleSpeed,
	float contractSpeed,
	float fadeSpeed,
	const Vector& color,
	const Vector& initialAngles,
	const Vector& angularVelocity,
	float stretchX,
	float stretchY,
	int renderMode,
	int renderFlags,
	int collisionFlags,
	float bounceFactor,
	bool affectedByForce,
	int framerate,
	const char* classname)
{
	if (g_pParticleMan == nullptr)
	{
		gEngfuncs.Con_DPrintf(
			"ParticleUtil: g_pParticleMan is null\n");

		return nullptr;
	}

	int frameCount = 0;
	model_s* sprite =
		LoadParticleSprite(type, &frameCount);

	if (sprite == nullptr)
	{
		return nullptr;
	}

	const float safeSize = ClampMinimum(size, 0.0001f);
	const float safeBrightness =
		ClampRange(brightness, 0.0f, 255.0f);

	// The fourth CreateParticle argument is named "normal" in ParticleMan,
	// but CBaseParticle stores it as the particle's initial Euler angles.

	const ParticleDef& definition = GetParticleDef(type);

	const char* safeClassname =
		classname != nullptr && classname[0] != '\0'
			? classname
			: definition.classname;

	CBaseParticle* particle =
		g_pParticleMan->CreateParticle(
			origin,
			initialAngles,
			sprite,
			safeSize,
			safeBrightness,
			safeClassname);

	if (particle == nullptr)
	{
		return nullptr;
	}

	const float currentTime =
		gEngfuncs.GetClientTime();

	particle->m_vVelocity = velocity;
	particle->m_flGravity = gravity;

	particle->m_flDieTime =
		currentTime + ClampMinimum(lifetime, 0.001f);

	particle->m_flScaleSpeed = scaleSpeed;
	particle->m_flContractSpeed = contractSpeed;
	particle->m_flFadeSpeed = fadeSpeed;

	particle->m_vColor = Vector(
		ClampRange(color.x, 0.0f, 255.0f),
		ClampRange(color.y, 0.0f, 255.0f),
		ClampRange(color.z, 0.0f, 255.0f));

	particle->m_vAVelocity = angularVelocity;

	particle->m_flStretchX =
		ClampMinimum(stretchX, 0.0001f);

	particle->m_flStretchY =
		ClampMinimum(stretchY, 0.0001f);

	particle->m_iRendermode = renderMode;
	particle->SetRenderFlags(renderFlags);

	particle->SetCollisionFlags(collisionFlags);
	particle->m_flBounceFactor = bounceFactor;

	particle->m_bAffectedByForce = affectedByForce;

	particle->m_iFramerate =
		framerate < 0 ? 0 : framerate;

	particle->m_iNumFrames = frameCount;
	particle->m_iFrame = 0;

	return particle;
}
}

//------------------------------------------------------------------------------
// Public lifecycle
//------------------------------------------------------------------------------

void ParticleUtil_Init()
{
	ParticleUtil_Shutdown();

	g_ParticleUtilInitialized = true;

	for (int index = 0; index < PARTICLE_TYPE_COUNT; ++index)
	{
		const ParticleType type =
			static_cast<ParticleType>(index);

		const ParticleDef& definition =
			GetParticleDef(type);

		// Custom deliberately has no predefined sprite.
		if (definition.sprite == nullptr ||
			definition.sprite[0] == '\0')
		{
			continue;
		}

		LoadParticleSprite(type, nullptr);
	}
}

void ParticleUtil_Shutdown()
{
	for (int index = 0; index < PARTICLE_TYPE_COUNT; ++index)
	{
		g_ParticleSprites[index].handle = 0;
		g_ParticleSprites[index].model = nullptr;
		g_ParticleSprites[index].frameCount = 0;
		g_ParticleSprites[index].attemptedLoad = false;
	}

	g_ParticleUtilInitialized = false;
}

bool ParticleUtil_IsInitialized()
{
	return g_ParticleUtilInitialized;
}

//------------------------------------------------------------------------------
// Basic preset emission
//------------------------------------------------------------------------------

void SpawnParticles(
	ParticleType type,
	int count,
	const Vector& origin,
	const Vector& velocity)
{
	SpawnParticles(
		type,
		count,
		origin,
		velocity,
		Vector(0.0f, 0.0f, 0.0f));
}

void SpawnParticles(
	ParticleType type,
	int count,
	const Vector& origin,
	const Vector& velocity,
	const Vector& velocityRandom)
{
	if (count <= 0 || !IsValidParticleType(type))
	{
		return;
	}

	const ParticleDef& definition =
		GetParticleDef(type);

	for (int index = 0; index < count; ++index)
	{
		const Vector particleVelocity =
			velocity +
			RandomSymmetricVector(velocityRandom);

		CreateConfiguredParticle(
			type,
			origin,
			particleVelocity,
			definition.gravity,
			definition.lifetime,
			definition.size,
			definition.brightness,
			definition.scaleSpeed,
			definition.contractSpeed,
			definition.fadeSpeed,
			definition.color,
			RandomVector(
				definition.initialAnglesMin,
				definition.initialAnglesMax),
			RandomVector(
				definition.angularVelocityMin,
				definition.angularVelocityMax),
			definition.stretchX,
			definition.stretchY,
			definition.renderMode,
			definition.renderFlags,
			definition.collisionFlags,
			definition.bounceFactor,
			definition.affectedByForce,
			definition.framerate,
			definition.classname);
	}
}

//------------------------------------------------------------------------------
// Fully configurable emission
//------------------------------------------------------------------------------

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

	const char* classname)
{
	if (count <= 0 || !IsValidParticleType(type))
	{
		return;
	}

	for (int index = 0; index < count; ++index)
	{
		const Vector particleOrigin =
			origin +
			RandomSymmetricVector(originRandom);

		const Vector particleVelocity =
			velocity +
			RandomSymmetricVector(velocityRandom);

		CreateConfiguredParticle(
			type,
			particleOrigin,
			particleVelocity,
			gravity,
			RandomFloat(lifetimeMin, lifetimeMax),
			RandomFloat(sizeMin, sizeMax),
			RandomFloat(brightnessMin, brightnessMax),
			RandomFloat(scaleSpeedMin, scaleSpeedMax),
			RandomFloat(contractSpeedMin, contractSpeedMax),
			RandomFloat(fadeSpeedMin, fadeSpeedMax),
			RandomVector(colorMin, colorMax),
			RandomVector(
				initialAnglesMin,
				initialAnglesMax),
			RandomVector(
				angularVelocityMin,
				angularVelocityMax),
			RandomFloat(stretchXMin, stretchXMax),
			RandomFloat(stretchYMin, stretchYMax),
			renderMode,
			renderFlags,
			collisionFlags,
			bounceFactor,
			affectedByForce,
			framerate,
			classname);
	}
}

//------------------------------------------------------------------------------
// Directional emission
//------------------------------------------------------------------------------

void SpawnParticleCone(
	ParticleType type,
	int count,
	const Vector& origin,
	const Vector& direction,
	float speedMin,
	float speedMax,
	float rightSpread,
	float upSpread)
{
	if (count <= 0 || !IsValidParticleType(type))
	{
		return;
	}

	const ParticleDef& definition =
		GetParticleDef(type);

	Vector forward;
	Vector right;
	Vector up;

	BuildDirectionBasis(
		direction,
		forward,
		right,
		up);

	for (int index = 0; index < count; ++index)
	{
		const Vector velocity =
			forward * RandomFloat(speedMin, speedMax) +
			right * RandomSymmetric(rightSpread) +
			up * RandomSymmetric(upSpread);

		CreateConfiguredParticle(
			type,
			origin,
			velocity,
			definition.gravity,
			definition.lifetime,
			definition.size,
			definition.brightness,
			definition.scaleSpeed,
			definition.contractSpeed,
			definition.fadeSpeed,
			definition.color,
			RandomVector(
				definition.initialAnglesMin,
				definition.initialAnglesMax),
			RandomVector(
				definition.angularVelocityMin,
				definition.angularVelocityMax),
			definition.stretchX,
			definition.stretchY,
			definition.renderMode,
			definition.renderFlags,
			definition.collisionFlags,
			definition.bounceFactor,
			definition.affectedByForce,
			definition.framerate,
			definition.classname);
	}
}

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

	const char* classname)
{
	if (count <= 0 || !IsValidParticleType(type))
	{
		return;
	}

	Vector forward;
	Vector right;
	Vector up;

	BuildDirectionBasis(
		direction,
		forward,
		right,
		up);

	for (int index = 0; index < count; ++index)
	{
		const Vector particleOrigin =
			origin +
			RandomSymmetricVector(originRandom);

		const Vector particleVelocity =
			forward * RandomFloat(speedMin, speedMax) +
			right * RandomSymmetric(rightSpread) +
			up * RandomSymmetric(upSpread) +
			RandomSymmetricVector(velocityRandom);

		CreateConfiguredParticle(
			type,
			particleOrigin,
			particleVelocity,
			gravity,
			RandomFloat(lifetimeMin, lifetimeMax),
			RandomFloat(sizeMin, sizeMax),
			RandomFloat(brightnessMin, brightnessMax),
			RandomFloat(scaleSpeedMin, scaleSpeedMax),
			RandomFloat(contractSpeedMin, contractSpeedMax),
			RandomFloat(fadeSpeedMin, fadeSpeedMax),
			RandomVector(colorMin, colorMax),
			RandomVector(
				initialAnglesMin,
				initialAnglesMax),
			RandomVector(
				angularVelocityMin,
				angularVelocityMax),
			RandomFloat(stretchXMin, stretchXMax),
			RandomFloat(stretchYMin, stretchYMax),
			renderMode,
			renderFlags,
			collisionFlags,
			bounceFactor,
			affectedByForce,
			framerate,
			classname);
	}
}