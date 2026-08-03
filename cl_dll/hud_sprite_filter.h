#pragma once

#include "hud.h"

void DrawHudSpriteAdditiveNearest(
	HSPRITE sprite,
	int frame,
	int x,
	int y,
	const Rect* rect,
	int red,
	int green,
	int blue);

void DrawHudSpriteHolesNearest(
	HSPRITE sprite,
	int frame,
	int x,
	int y,
	const Rect* rect,
	int red,
	int green,
	int blue);

void DrawHudSpriteNearest(
	HSPRITE sprite,
	int frame,
	int x,
	int y,
	const Rect* rect,
	int red,
	int green,
	int blue);