#include "hud.h"
#include "cl_util.h"

#include "triangleapi.h"
#include "particleman.h"

#include "particle_types.h"

//------------------------------------------------------------------------------
// Default particle definitions.
//
// The sprite field is just the filename. The actual sprite is loaded and
// cached by particle_util.cpp.
//------------------------------------------------------------------------------

namespace
{
const ParticleDef g_ParticleDefs[] =
{
	// ParticleType::MuzzleFlash
	{
		// sprite location
		"sprites/explode1.spr",

		// class name
		"muzzle_flash",

		// size
		8.0f,

		// brightness
		255.0f,

		// lifetime
		0.5f,

		// gravity
		0.0f,

		// scale speed
		8.0f / 30.0f,

		// contract speed
		0.0f / 30.0f,

		// fade speed
		100.0f,

		// colour
		Vector(255.0f, 255.0f, 255.0f),

		// initial rotation, min
		Vector(0.0f, 0.0f, -360.0f),

		// initial rotation, max
		Vector(0.0f, 0.0f, 360.0f),

		// angular velocity, min
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, max
		Vector(0.0f, 0.0f, 0.0f),

		// stretch x
		1.0f,

		// stretch y
		1.0f,

		// render mode
		kRenderTransAdd,

		// render flags
		RENDER_FACEPLAYER_ROTATEZ | LIGHT_NONE,

		// collision flags
		0,

		// bounce factor
		0.0f,

		// affected by force
		false,

		// framerate
		0
	},

	// ParticleType::MuzzleSmokePistol
	{
		// sprite location
		"sprites/ballsmoke.spr",

		// class name
		"muzzle_smoke_pistol",

		// size
		16.0f,

		// brightness
		12.0f,

		// lifetime
		5.0f,

		// gravity
		0.0f,

		// scale speed
		8.0f / 30.0f,

		// contract speed
		0.0f / 30.0f,

		// fade speed
		0.8f,

		// colour
		Vector(255.0f, 255.0f, 255.0f),

		// initial rotation, min
		Vector(0.0f, 0.0f, -360.0f),

		// initial rotation, max
		Vector(0.0f, 0.0f, 360.0f),

		// angular velocity, min
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, max
		Vector(0.0f, 0.0f, 0.0f),

		// stretch x
		1.0f,

		// stretch y
		1.0f,

		// render mode
		kRenderTransAdd,

		// render flags
		RENDER_FACEPLAYER_ROTATEZ | LIGHT_INTENSITY,

		// collision flags
		0,

		// bounce factor
		0.0f,

		// affected by force
		false,

		// framerate
		0
	},
	
	// ParticleType::MuzzleSmokeRifle
	{
		// sprite location
		"sprites/ballsmoke.spr",

		// class name
		"muzzle_smoke_rifle",

		// size
		32.0f,

		// brightness
		10.0f,

		// lifetime
		10.0f,

		// gravity
		0.0f,

		// scale speed
		8.0f / 30.0f,

		// contract speed
		0.0f / 30.0f,

		// fade speed
		0.4f,

		// colour
		Vector(255.0f, 255.0f, 255.0f),

		// initial rotation, min
		Vector(0.0f, 0.0f, -360.0f),

		// initial rotation, max
		Vector(0.0f, 0.0f, 360.0f),

		// angular velocity, min
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, max
		Vector(0.0f, 0.0f, 0.0f),

		// stretch x
		1.0f,

		// stretch y
		1.0f,

		// render mode
		kRenderTransAdd,

		// render flags
		RENDER_FACEPLAYER_ROTATEZ | LIGHT_INTENSITY,

		// collision flags
		0,

		// bounce factor
		0.0f,

		// affected by force
		false,

		// framerate
		0
	},

	// ParticleType::MuzzleSpark
	{
		// sprite location
		"sprites/glow01.spr",

		// class name
		"muzzle_spark",

		// size
		0.25f,

		// brightness
		255.0f,

		// lifetime
		2.0f,

		// gravity
		0.5f,

		// scale speed
		0.0f / 30.0f,

		// contract speed
		0.0f / 30.0f,

		// fade speed
		40.0f,

		// colour
		Vector(255.0f, 200.0f, 120.0f),

		// initial rotation, min
		Vector(0.0f, 0.0f, 0.0f),

		// initial rotation, max
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, min
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, max
		Vector(0.0f, 0.0f, 0.0f),

		// stretch x
		1.0f,

		// stretch y
		1.0f,

		// render mode
		kRenderTransAdd,

		// render flags
		RENDER_FACEPLAYER_ROTATEZ | LIGHT_NONE,

		// collision flags
		0,

		// bounce factor
		0.0f,

		// affected by force
		false,

		// framerate
		0
	},

	// ParticleType::BulletDust
	{
		// sprite location
		"sprites/glow01.spr",

		// class name
		"bullet_dust",

		// size
		8.0f,

		// brightness
		128.0f,

		// lifetime
		5.0f,

		// gravity
		0.0f,

		// scale speed
		0.0f / 30.0f,

		// contract speed
		0.0f / 30.0f,

		// fade speed
		15.0f,

		// colour
		Vector(255.0f, 200.0f, 120.0f),

		// initial rotation, min
		Vector(0.0f, 0.0f, 0.0f),

		// initial rotation, max
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, min
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, max
		Vector(0.0f, 0.0f, 0.0f),

		// stretch x
		1.0f,

		// stretch y
		1.0f,

		// render mode
		kRenderTransAdd,

		// render flags
		RENDER_FACEPLAYER_ROTATEZ | LIGHT_NONE,

		// collision flags
		0,

		// bounce factor
		0.0f,

		// affected by force
		false,

		// framerate
		0
	},

	// ParticleType::Blood
	{
		// sprite location
		"sprites/glow01.spr",

		// class name
		"blood",

		// size
		8.0f,

		// brightness
		128.0f,

		// lifetime
		5.0f,

		// gravity
		0.0f,

		// scale speed
		0.0f / 30.0f,

		// contract speed
		0.0f / 30.0f,

		// fade speed
		15.0f,

		// colour
		Vector(255.0f, 200.0f, 120.0f),

		// initial rotation, min
		Vector(0.0f, 0.0f, 0.0f),

		// initial rotation, max
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, min
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, max
		Vector(0.0f, 0.0f, 0.0f),

		// stretch x
		1.0f,

		// stretch y
		1.0f,

		// render mode
		kRenderTransAdd,

		// render flags
		RENDER_FACEPLAYER_ROTATEZ | LIGHT_NONE,

		// collision flags
		0,

		// bounce factor
		0.0f,

		// affected by force
		false,

		// framerate
		0
	},

	// ParticleType::Custom
	{
		// sprite location
		"sprites/glow01.spr",

		// class name
		"custom_particle",

		// size
		8.0f,

		// brightness
		128.0f,

		// lifetime
		5.0f,

		// gravity
		0.0f,

		// scale speed
		0.0f / 30.0f,

		// contract speed
		0.0f / 30.0f,

		// fade speed
		15.0f,

		// colour
		Vector(255.0f, 200.0f, 120.0f),

		// initial rotation, min
		Vector(0.0f, 0.0f, 0.0f),

		// initial rotation, max
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, min
		Vector(0.0f, 0.0f, 0.0f),

		// angular velocity, max
		Vector(0.0f, 0.0f, 0.0f),

		// stretch x
		1.0f,

		// stretch y
		1.0f,

		// render mode
		kRenderTransAdd,

		// render flags
		RENDER_FACEPLAYER_ROTATEZ | LIGHT_NONE,

		// collision flags
		0,

		// bounce factor
		0.0f,

		// affected by force
		false,

		// framerate
		0
	}
};
}

const ParticleDef& GetParticleDef(ParticleType type)
{
	const int index = static_cast<int>(type);
	const int count =
		static_cast<int>(sizeof(g_ParticleDefs) / sizeof(g_ParticleDefs[0]));

	if (index < 0 || index >= count)
	{
		return g_ParticleDefs[
			static_cast<int>(ParticleType::Custom)];
	}

	return g_ParticleDefs[index];
}
