#pragma once

#include "mathlib.h"

struct Spring
{
	Vector position;
	Vector velocity;

	Spring()
		: position(0.0f, 0.0f, 0.0f),
		  velocity(0.0f, 0.0f, 0.0f)
	{
	}
};

void UpdateSpring(Spring& spring, const Vector& target, float stiff, float damp, float delta);

void AddSpringImpulse(Spring& spring, const Vector& impulse);

void ResetSpring(Spring& spring);

extern Spring spring_viewmodel_position;
extern Spring spring_viewmodel_rotation;
extern Spring spring_viewmodel_recoil_position;
extern Spring spring_viewmodel_recoil_rotation;