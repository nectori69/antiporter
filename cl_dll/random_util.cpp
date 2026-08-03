#include "hud.h"
#include "cl_util.h"

#include "random_util.h"

float RandomFloat(float minimum, float maximum)
{
	if (minimum > maximum)
	{
		const float temp = minimum;
		minimum = maximum;
		maximum = temp;
	}

	if (minimum == maximum)
		return minimum;

	return gEngfuncs.pfnRandomFloat(minimum, maximum);
}

int RandomInt(int minimum, int maximum)
{
	if (minimum > maximum)
	{
		const int temp = minimum;
		minimum = maximum;
		maximum = temp;
	}

	if (minimum == maximum)
		return minimum;

	return gEngfuncs.pfnRandomLong(minimum, maximum);
}