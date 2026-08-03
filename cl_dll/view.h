//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

void V_StartPitchDrift();
void V_StopPitchDrift();
void V_CSStyleRecoil(float pitch, float yaw);

extern Vector recoilAngle;