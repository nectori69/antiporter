#include "motion_spring.h"
#include "mathlib.h"

void UpdateSpring(Spring& spring, const Vector& target, float stiffness, float damping, float deltaTime)
{
	if (deltaTime <= 0.0f)
	{
		return;
	}

	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	const Vector displacement =
		spring.position - target;

	const Vector acceleration =
		displacement * -stiffness -
		spring.velocity * damping;

	spring.velocity =
		spring.velocity +
		acceleration * deltaTime;

	spring.position =
		spring.position +
		spring.velocity * deltaTime;
}

void AddSpringImpulse(Spring& spring, const Vector& impulse)
{
	spring.velocity = spring.velocity + impulse;
}

void ResetSpring(Spring& spring)
{
	spring.position = Vector(0.0f, 0.0f, 0.0f);
	spring.velocity = Vector(0.0f, 0.0f, 0.0f);
}