#include "hud.h"
#include "cl_util.h"
#include "triangleapi.h"
#include "hud_sprite_filter.h"

// Avoid including Windows.h or GL/gl.h here.
// Windows.h defines an unrelated HSPRITE type that conflicts with GoldSrc's
// HSPRITE type.

#if defined(_WIN32)

#define APIENTRY __stdcall
#define WINGDIAPI __declspec(dllimport)

using GLenum = unsigned int;
using GLint = int;

// Values from the OpenGL 1.1 headers.
constexpr GLenum GL_TEXTURE_2D = 0x0DE1;
constexpr GLenum GL_TEXTURE_MAG_FILTER = 0x2800;
constexpr GLenum GL_TEXTURE_MIN_FILTER = 0x2801;
constexpr GLint GL_NEAREST = 0x2600;

extern "C" {
WINGDIAPI void APIENTRY glTexParameteri(
	GLenum target,
	GLenum pname,
	GLint param);
}

#if defined(_MSC_VER)
#pragma comment(lib, "opengl32.lib")
#endif

#endif

namespace
{
bool SetHudSpriteFrameNearest(
	HSPRITE sprite,
	int frame)
{
	if (!sprite)
	{
		return false;
	}

	if (!gEngfuncs.pTriAPI)
	{
		return false;
	}

	const model_s* spriteModel =
		gEngfuncs.GetSpritePointer(sprite);

	if (!spriteModel)
	{
		return false;
	}

	const int bound =
		gEngfuncs.pTriAPI->SpriteTexture(
			const_cast<model_s*>(spriteModel),
			frame);

	if (!bound)
	{
		return false;
	}

#if defined(_WIN32)
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
#endif

	return true;
}
}

void DrawHudSpriteAdditiveNearest(
	HSPRITE sprite,
	int frame,
	int x,
	int y,
	const Rect* rect,
	int red,
	int green,
	int blue)
{
	if (!sprite)
	{
		return;
	}

	SPR_Set(sprite, red, green, blue);
	SetHudSpriteFrameNearest(sprite, frame);
	SPR_DrawAdditive(frame, x, y, rect);
}

void DrawHudSpriteHolesNearest(
	HSPRITE sprite,
	int frame,
	int x,
	int y,
	const Rect* rect,
	int red,
	int green,
	int blue)
{
	if (!sprite)
	{
		return;
	}

	SPR_Set(sprite, red, green, blue);
	SetHudSpriteFrameNearest(sprite, frame);
	SPR_DrawHoles(frame, x, y, rect);
}

void DrawHudSpriteNearest(
	HSPRITE sprite,
	int frame,
	int x,
	int y,
	const Rect* rect,
	int red,
	int green,
	int blue)
{
	if (!sprite)
	{
		return;
	}

	SPR_Set(sprite, red, green, blue);
	SetHudSpriteFrameNearest(sprite, frame);
	SPR_Draw(frame, x, y, rect);
}