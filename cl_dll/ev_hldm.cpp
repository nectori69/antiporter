/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "hud.h"
#include "cl_util.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "com_weapons.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <string.h>

#include "r_studioint.h"
#include "com_model.h"

extern engine_studio_api_t IEngineStudio;

static int tracerCount[MAX_PLAYERS];

#include "pm_shared.h"

void V_Recoil(float recoil);

void V_PunchAxis(int axis, float punch);
void VectorAngles(const float* forward, float* angles);

extern cvar_t* cl_lw;

// play a strike sound based on the texture that was hit by the attack traceline.  VecSrc/VecEnd are the
// original traceline endpoints used by the attacker, iBulletType is the type of bullet that hit the texture.
// returns volume of strike instrument (crowbar) to play
float EV_HLDM_PlayTextureSound(int idx, pmtrace_t* ptr, float* vecSrc, float* vecEnd, int iBulletType)
{
	// hit the world, try to play sound based on texture material type
	char chTextureType = CHAR_TEX_CONCRETE;
	float fvol;
	float fvolbar;
	const char* rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	char* pTextureName;
	char texname[64];
	char szbuffer[64];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace(ptr);

	// FIXME check if playtexture sounds movevar is set
	//

	chTextureType = 0;

	// Player
	if (entity >= 1 && entity <= gEngfuncs.GetMaxClients())
	{
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	}
	else if (entity == 0)
	{
		// get texture from entity or world (world is ent(0))
		pTextureName = (char*)gEngfuncs.pEventAPI->EV_TraceTexture(ptr->ent, vecSrc, vecEnd);

		if (pTextureName)
		{
			strcpy(texname, pTextureName);
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+')
			{
				pTextureName += 2;
			}

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
			{
				pTextureName++;
			}

			// '}}'
			strcpy(szbuffer, pTextureName);
			szbuffer[CBTEXTURENAMEMAX - 1] = 0;

			// get texture type
			chTextureType = PM_FindTextureType(szbuffer);
		}
	}

	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE:
		fvol = 1.0;
		fvolbar = 0.5;
		rgsz[0] = "debris/hits/hit_conc1.wav";
		rgsz[1] = "debris/hits/hit_conc2.wav";
		rgsz[2] = "debris/hits/hit_conc3.wav";
		rgsz[3] = "debris/hits/hit_conc4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_METAL:
	case CHAR_TEX_GRATE:
	case CHAR_TEX_VENT:
	case CHAR_TEX_COMPUTER:
		fvol = 1.0;
		fvolbar = 0.5;
		rgsz[0] = "debris/hits/hit_metal1.wav";
		rgsz[1] = "debris/hits/hit_metal2.wav";
		rgsz[2] = "debris/hits/hit_metal3.wav";
		rgsz[3] = "debris/hits/hit_metal4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_GRASS:
	case CHAR_TEX_DIRT:
		fvol = 1.0;
		fvolbar = 0.5;
		rgsz[0] = "debris/hits/hit_dirt1.wav";
		rgsz[1] = "debris/hits/hit_dirt2.wav";
		rgsz[2] = "debris/hits/hit_dirt3.wav";
		rgsz[3] = "debris/hits/hit_dirt4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_TILE:
		fvol = 1.0;
		fvolbar = 0.5;
		rgsz[0] = "debris/hits/hit_tile1.wav";
		rgsz[1] = "debris/hits/hit_tile2.wav";
		rgsz[2] = "debris/hits/hit_tile3.wav";
		rgsz[3] = "debris/hits/hit_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH:
		fvol = 1.0;
		fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD:
		fvol = 1.0;
		fvolbar = 0.5;
		rgsz[0] = "debris/hits/hit_wood1.wav";
		rgsz[1] = "debris/hits/hit_wood2.wav";
		rgsz[2] = "debris/hits/hit_wood3.wav";
		rgsz[3] = "debris/hits/hit_wood4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_CARPET:
		fvol = 1.0;
		fvolbar = 0.1;
		rgsz[0] = "debris/hits/hit_carpet1.wav";
		rgsz[1] = "debris/hits/hit_carpet2.wav";
		rgsz[2] = "debris/hits/hit_carpet3.wav";
		rgsz[3] = "debris/hits/hit_carpet4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_GLASS:
		fvol = 1.0;
		fvolbar = 0.5;
		rgsz[0] = "debris/hits/hit_glass1.wav";
		rgsz[1] = "debris/hits/hit_glass2.wav";
		rgsz[2] = "debris/hits/hit_glass3.wav";
		rgsz[3] = "debris/hits/hit_glass4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_FLESH:
		if (iBulletType == BULLET_PLAYER_CROWBAR)
			return 0.0; // crowbar already makes this sound
		fvol = 1.0;
		fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		rgsz[2] = "weapons/bullet_hit3.wav";
		rgsz[3] = "weapons/bullet_hit4.wav";
		fattn = 1.0;
		cnt = 4;
		break;
	}

	// play material hit sound
	gEngfuncs.pEventAPI->EV_PlaySound(0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong(0, cnt - 1)], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong(0, 0xf));
	return fvolbar;
}

void EV_HLDM_MuzzleFlash(float* pos, float amount)
{
	// make a dlight first
	dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);

	// Original color values
	int originalR = 231;
	int originalG = 219;
	int originalB = 14;

	// Randomize color components within the range of +/- 20
	dl->color.r = originalR + gEngfuncs.pfnRandomLong(-20, 20);
	dl->color.g = originalG + gEngfuncs.pfnRandomLong(-20, 20);
	dl->color.b = originalB + gEngfuncs.pfnRandomLong(0, 0);

	// Randomize the die value by +/- 0.01
	dl->die = gEngfuncs.GetClientTime() + 0.05 + gEngfuncs.pfnRandomFloat(-0.01, 0.01);

	// Randomize the radius based on amount
	dl->radius = gEngfuncs.pfnRandomFloat(245.0f, 256.0f);

	// Randomize the decay value
	dl->decay = gEngfuncs.pfnRandomFloat(400.0f, 600.0f);
}

char* EV_HLDM_DamageDecal(physent_t* pe)
{
	static char decalname[32];
	int idx;

	if (pe->classnumber == 1)
	{
		idx = gEngfuncs.pfnRandomLong(0, 2);
		sprintf(decalname, "{break%i", idx + 1);
	}
	else if (pe->rendermode != kRenderNormal)
	{
		sprintf(decalname, "{bproof1");
	}
	else
	{
		idx = gEngfuncs.pfnRandomLong(0, 4);
		sprintf(decalname, "{shot%i", idx + 1);
	}
	return decalname;
}

void EV_HLDM_GunshotDecalTrace(pmtrace_t* pTrace, char* decalName)
{
	int iRand;
	physent_t* pe;

	gEngfuncs.pEfxAPI->R_BulletImpactParticles(pTrace->endpos);

	iRand = gEngfuncs.pfnRandomLong(0, 0x7FFF);
	if (iRand < (0x7fff / 2)) // not every bullet makes a sound.
	{
		switch (iRand % 5)
		{
		case 0:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 2:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 3:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 4:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent(pTrace->ent);

	// Only decal brush models such as the world etc.
	if (decalName && '\0' != decalName[0] && pe && (pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP))
	{
		if (CVAR_GET_FLOAT("r_decals"))
		{
			gEngfuncs.pEfxAPI->R_DecalShoot(
				gEngfuncs.pEfxAPI->Draw_DecalIndex(gEngfuncs.pEfxAPI->Draw_DecalIndexFromName(decalName)),
				gEngfuncs.pEventAPI->EV_IndexFromTrace(pTrace), 0, pTrace->endpos, 0);
		}
	}
}

void EV_HLDM_DecalGunshot(pmtrace_t* pTrace, int iBulletType)
{
	physent_t* pe;

	pe = gEngfuncs.pEventAPI->EV_GetPhysent(pTrace->ent);

	if (pe && pe->solid == SOLID_BSP)
	{
		switch (iBulletType)
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		case BULLET_PLAYER_M4:
		case BULLET_PLAYER_M92:
		default:
			// smoke and decal
			EV_HLDM_GunshotDecalTrace(pTrace, EV_HLDM_DamageDecal(pe));
			break;
		}
	}
}

void EV_HLDM_CheckTracer(int idx, float* vecSrc, float* end, float* forward, float* right, int iBulletType, int iTracerFreq, int* tracerCount)
{
	int i;
	bool player = idx >= 1 && idx <= gEngfuncs.GetMaxClients();

	if (iTracerFreq != 0 && ((*tracerCount)++ % iTracerFreq) == 0)
	{
		Vector vecTracerSrc;

		if (player)
		{
			Vector offset(0, 0, -4);

			// adjust tracer position for player
			for (i = 0; i < 3; i++)
			{
				vecTracerSrc[i] = vecSrc[i] + offset[i] + right[i] * 2 + forward[i] * 16;
			}
		}
		else
		{
			VectorCopy(vecSrc, vecTracerSrc);
		}

		switch (iBulletType)
		{
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_MONSTER_9MM:
		case BULLET_MONSTER_12MM:
		default:
			EV_CreateTracer(vecTracerSrc, end);
			break;
		}
	}
}


/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void EV_HLDM_FireBullets(int idx, float* forward, float* right, float* up, int cShots, float* vecSrc, float* vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int* tracerCount, float flSpreadX, float flSpreadY)
{
	int i;
	pmtrace_t tr;
	int iShot;

	for (iShot = 1; iShot <= cShots; iShot++)
	{
		Vector vecDir, vecEnd;

		float x, y, z;
		//We randomize for the Shotgun.
		if (iBulletType == BULLET_PLAYER_BUCKSHOT)
		{
			do
			{
				x = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
				y = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = vecDirShooting[i] + x * flSpreadX * right[i] + y * flSpreadY * up[i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		} //But other guns already have their spread randomized in the synched spread.
		else
		{

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = vecDirShooting[i] + flSpreadX * right[i] + flSpreadY * up[i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		}

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(0, 1);

		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();

		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);

		EV_HLDM_CheckTracer(idx, vecSrc, tr.endpos, forward, right, iBulletType, iTracerFreq, tracerCount);

		// do damage, paint decals
		if (tr.fraction != 1.0)
		{
			switch (iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:

				EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			case BULLET_PLAYER_MP5:

				EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
				EV_HLDM_DecalGunshot(&tr, iBulletType);
				break;
			case BULLET_PLAYER_BUCKSHOT:

				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			case BULLET_PLAYER_357:

				EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			}
		}

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}

//======================
//	    GLOCK START
//======================
void EV_FireGlock1(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/9x19mm.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? GLOCK_SHOOT_EMPTY : GLOCK_SHOOT, 0);

		V_PunchAxis(0, -2.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/g17/g17_fire.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	EV_GetGunPosition(args, vecSrc, origin);
	EV_HLDM_MuzzleFlash(vecSrc, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}

void EV_FireGlock2(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/9x19mm.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? GLOCK_SHOOT_EMPTY : GLOCK_SHOOT, 0);

		V_PunchAxis(0, -2.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/g17/g17_fire.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	EV_GetGunPosition(args, vecSrc, origin);
	EV_HLDM_MuzzleFlash(vecSrc, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}
//======================
//	   GLOCK END
//======================

//======================
//	  SHOTGUN START
//======================
void EV_FireShotGunDouble(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	int j;
	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/12ga.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE2, 0);
		V_PunchAxis(0, -10.0);
	}

	for (j = 0; j < 2; j++)
	{
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/spas12/spas_firedouble.wav", gEngfuncs.pfnRandomFloat(0.98, 1.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong(0, 0x1f));

	EV_GetGunPosition(args, vecSrc, origin);
	EV_HLDM_MuzzleFlash(vecSrc, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));
	VectorCopy(forward, vecAiming);

	if (gEngfuncs.GetMaxClients() > 1)
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 8, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.17365, 0.04362);
	}
	else
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 12, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.08716);
	}
}

void EV_FireShotGunSingle(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/12ga.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE, 0);

		V_PunchAxis(0, -5.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/spas12/spas_firesingle.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0x1f));

	EV_GetGunPosition(args, vecSrc, origin);
	EV_HLDM_MuzzleFlash(vecSrc, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));
	VectorCopy(forward, vecAiming);

	if (gEngfuncs.GetMaxClients() > 1)
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 4, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.04362);
	}
	else
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 6, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.08716);
	}
}
//======================
//	   SHOTGUN END
//======================

//======================
//	    MP5 START
//======================
void EV_FireMP5(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/9x19mm.mdl"); // brass shell
	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_FIRE1 + gEngfuncs.pfnRandomLong(0, 2), 0);

		V_PunchAxis(0, -1.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	switch (gEngfuncs.pfnRandomLong(0, 2))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/mp5/mp5_fire1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/mp5/mp5_fire2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 2:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/mp5/mp5_fire3.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	}

	EV_GetGunPosition(args, vecSrc, origin);
	EV_HLDM_MuzzleFlash(vecSrc, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));
	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP5, 2, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}

// We only predict the animation and sound
// The grenade is still launched from the server.
void EV_FireMP52(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_LAUNCH, 0);
		V_PunchAxis(0, -10);
	}

	switch (gEngfuncs.pfnRandomLong(0, 1))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m203/m203_fire1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m203/m203_fire2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	}
}
//======================
//		 MP5 END
//======================

//======================
//	   PHYTON START
//	     ( .357 )
//======================
void EV_FirePython(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		// Python uses different body in multiplayer versus single player
		bool multiplayer = gEngfuncs.GetMaxClients() != 1;

		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(PYTHON_FIRE1, multiplayer ? 1 : 0);

		V_PunchAxis(0, -10.0);
	}

	switch (gEngfuncs.pfnRandomLong(0, 1))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/python/357_fire1.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM);
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/python/357_fire2.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM);
		break;
	}

	EV_GetGunPosition(args, vecSrc, origin);
	EV_HLDM_MuzzleFlash(vecSrc, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_357, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}
//======================
//	    PHYTON END
//	     ( .357 )
//======================

//======================
//	   GAUSS START
//======================
void EV_SpinGauss(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	int iSoundState = 0;

	int pitch;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	pitch = args->iparam1;

	iSoundState = 0 != args->bparam1 ? SND_CHANGE_PITCH : 0;

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, iSoundState, pitch);
}

/*
==============================
EV_StopPreviousGauss

==============================
*/
void EV_StopPreviousGauss(int idx)
{
	// Make sure we don't have a gauss spin event in the queue for this guy
	gEngfuncs.pEventAPI->EV_KillEvents(idx, "events/gaussspin.sc");
	gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_WEAPON, "ambience/pulsemachine.wav");
}

extern float g_flApplyVel;

void EV_FireGauss(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	float flDamage = args->fparam1;

	bool m_fPrimaryFire = 0 != args->bparam1;
	Vector vecSrc;
	Vector vecDest;
	edict_t* pentIgnore;
	pmtrace_t tr, beam_tr;
	float flMaxFrac = 1.0;
	bool fHasPunched = false;
	bool fFirstBeam = true;
	int nMaxHits = 10;
	physent_t* pEntity;
	int m_iBeam, m_iGlow, m_iBalls;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	if (0 != args->bparam2)
	{
		EV_StopPreviousGauss(idx);
		return;
	}

	//	Con_Printf( "Firing gauss with %f\n", flDamage );
	EV_GetGunPosition(args, vecSrc, origin);
	EV_HLDM_MuzzleFlash(vecSrc, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));

	m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");
	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");

	AngleVectors(angles, forward, right, up);

	VectorMA(vecSrc, 8192, forward, vecDest);

	if (EV_IsLocal(idx))
	{
		V_PunchAxis(0, -2.0);
		gEngfuncs.pEventAPI->EV_WeaponAnimation(GAUSS_FIRE2, 0);

		if (!m_fPrimaryFire)
			g_flApplyVel = flDamage;
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/gauss2.wav", 0.5 + flDamage * (1.0 / 400.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong(0, 0x1f));

	while (flDamage > 10 && nMaxHits > 0)
	{
		nMaxHits--;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(0, 1);

		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();

		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecDest, PM_STUDIO_BOX, -1, &tr);

		gEngfuncs.pEventAPI->EV_PopPMStates();

		if (0 != tr.allsolid)
			break;

		if (fFirstBeam)
		{
			if (EV_IsLocal(idx))
			{
				// Add muzzle flash to current weapon model
				EV_MuzzleFlash();
			}
			fFirstBeam = false;

			gEngfuncs.pEfxAPI->R_BeamEntPoint(
				idx | 0x1000,
				tr.endpos,
				m_iBeam,
				0.1,
				m_fPrimaryFire ? 1.0 : 2.5,
				0.0,
				(m_fPrimaryFire ? 128.0 : flDamage) / 255.0,
				0,
				0,
				0,
				(m_fPrimaryFire ? 255 : 255) / 255.0,
				(m_fPrimaryFire ? 128 : 255) / 255.0,
				(m_fPrimaryFire ? 0 : 255) / 255.0);
		}
		else
		{
			gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc,
				tr.endpos,
				m_iBeam,
				0.1,
				m_fPrimaryFire ? 1.0 : 2.5,
				0.0,
				(m_fPrimaryFire ? 128.0 : flDamage) / 255.0,
				0,
				0,
				0,
				(m_fPrimaryFire ? 255 : 255) / 255.0,
				(m_fPrimaryFire ? 128 : 255) / 255.0,
				(m_fPrimaryFire ? 0 : 255) / 255.0);
		}

		pEntity = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);
		if (pEntity == NULL)
			break;

		if (pEntity->solid == SOLID_BSP)
		{
			float n;

			pentIgnore = NULL;

			n = -DotProduct(tr.plane.normal, forward);

			if (n < 0.5) // 60 degrees
			{
				// ALERT( at_console, "reflect %f\n", n );
				// reflect
				Vector r;

				VectorMA(forward, 2.0 * n, tr.plane.normal, r);

				flMaxFrac = flMaxFrac - tr.fraction;

				VectorCopy(r, forward);

				VectorMA(tr.endpos, 8.0, forward, vecSrc);
				VectorMA(vecSrc, 8192.0, forward, vecDest);

				gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT);

				Vector fwd;
				VectorAdd(tr.endpos, tr.plane.normal, fwd);

				gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100,
					255, 100);

				// lose energy
				if (n == 0)
				{
					n = 0.1;
				}

				flDamage = flDamage * (1 - n);
			}
			else
			{
				// tunnel
				EV_HLDM_DecalGunshot(&tr, BULLET_MONSTER_12MM);

				gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 1.0, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT);

				// limit it to one hole punch
				if (fHasPunched)
				{
					break;
				}
				fHasPunched = true;

				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if (!m_fPrimaryFire)
				{
					Vector start;

					VectorMA(tr.endpos, 8.0, forward, start);

					// Store off the old count
					gEngfuncs.pEventAPI->EV_PushPMStates();

					// Now add in all of the players.
					gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

					gEngfuncs.pEventAPI->EV_SetTraceHull(2);
					gEngfuncs.pEventAPI->EV_PlayerTrace(start, vecDest, PM_STUDIO_BOX, -1, &beam_tr);

					if (0 == beam_tr.allsolid)
					{
						Vector delta;
						float n;

						// trace backwards to find exit point

						gEngfuncs.pEventAPI->EV_PlayerTrace(beam_tr.endpos, tr.endpos, PM_STUDIO_BOX, -1, &beam_tr);

						VectorSubtract(beam_tr.endpos, tr.endpos, delta);

						n = Length(delta);

						if (n < flDamage)
						{
							if (n == 0)
								n = 1;
							flDamage -= n;

							// absorption balls
							{
								Vector fwd;
								VectorSubtract(tr.endpos, forward, fwd);
								gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100,
									255, 100);
							}

							//////////////////////////////////// WHAT TO DO HERE
							// CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

							EV_HLDM_DecalGunshot(&beam_tr, BULLET_MONSTER_12MM);

							gEngfuncs.pEfxAPI->R_TempSprite(beam_tr.endpos, vec3_origin, 0.1, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT);

							// balls
							{
								Vector fwd;
								VectorSubtract(beam_tr.endpos, forward, fwd);
								gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, beam_tr.endpos, fwd, m_iBalls, (int)(flDamage * 0.3), 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 200,
									255, 40);
							}

							VectorAdd(beam_tr.endpos, forward, vecSrc);
						}
					}
					else
					{
						flDamage = 0;
					}

					gEngfuncs.pEventAPI->EV_PopPMStates();
				}
				else
				{
					if (m_fPrimaryFire)
					{
						// slug doesn't punch through ever with primary
						// fire, so leave a little glowy bit and make some balls
						gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT);

						{
							Vector fwd;
							VectorAdd(tr.endpos, tr.plane.normal, fwd);
							gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100,
								255, 200);
						}
					}

					flDamage = 0;
				}
			}
		}
		else
		{
			VectorAdd(tr.endpos, forward, vecSrc);
		}
	}
}
//======================
//	   GAUSS END
//======================

//======================
//	   CROWBAR START
//======================
int g_iSwing;

//Only predict the miss sounds, hit sounds are still played
//server side, so players don't get the wrong idea.
void EV_Crowbar(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	//Play Swing sound
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM);

	if (EV_IsLocal(idx))
	{
		switch ((g_iSwing++) % 3)
		{
		case 0:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK1MISS, 0);
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK2MISS, 0);
			break;
		case 2:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK3MISS, 0);
			break;
		}
	}
}
//======================
//	   CROWBAR END
//======================

//======================
//	  CROSSBOW START
//======================
//=====================
// EV_BoltCallback
// This function is used to correct the origin and angles
// of the bolt, so it looks like it's stuck on the wall.
//=====================
void EV_BoltCallback(struct tempent_s* ent, float frametime, float currenttime)
{
	ent->entity.origin = ent->entity.baseline.vuser1;
	ent->entity.angles = ent->entity.baseline.vuser2;
}

void EV_FireCrossbow2(event_args_t* args)
{
	Vector vecSrc, vecEnd;
	Vector up, right, forward;
	pmtrace_t tr;

	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);

	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	EV_GetGunPosition(args, vecSrc, origin);

	VectorMA(vecSrc, 8192, forward, vecEnd);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));

	if (EV_IsLocal(idx))
	{
		if (0 != args->iparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE1, 0);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE3, 0);
	}

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);

	//We hit something
	if (tr.fraction < 1.0)
	{
		physent_t* pe = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);

		//Not the world, let's assume we hit something organic ( dog, cat, uncle joe, etc ).
		if (pe->solid != SOLID_BSP)
		{
			switch (gEngfuncs.pfnRandomLong(0, 1))
			{
			case 0:
				gEngfuncs.pEventAPI->EV_PlaySound(idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				break;
			case 1:
				gEngfuncs.pEventAPI->EV_PlaySound(idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				break;
			}
		}
		//Stick to world but don't stick to glass, it might break and leave the bolt floating. It can still stick to other non-transparent breakables though.
		else if (pe->rendermode == kRenderNormal)
		{
			gEngfuncs.pEventAPI->EV_PlaySound(0, tr.endpos, CHAN_BODY, "weapons/xbow_hit1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, PITCH_NORM);

			//Not underwater, do some sparks...
			if (gEngfuncs.PM_PointContents(tr.endpos, NULL) != CONTENTS_WATER)
				gEngfuncs.pEfxAPI->R_SparkShower(tr.endpos);

			Vector vBoltAngles;
			int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("models/crossbow_bolt.mdl");

			VectorAngles(forward, vBoltAngles);

			TEMPENTITY* bolt = gEngfuncs.pEfxAPI->R_TempModel(tr.endpos - forward * 10, Vector(0, 0, 0), vBoltAngles, 5, iModelIndex, TE_BOUNCE_NULL);

			if (bolt)
			{
				bolt->flags |= (FTENT_CLIENTCUSTOM);					 //So it calls the callback function.
				bolt->entity.baseline.vuser1 = tr.endpos - forward * 10; // Pull out a little bit
				bolt->entity.baseline.vuser2 = vBoltAngles;				 //Look forward!
				bolt->callback = EV_BoltCallback;						 //So we can set the angles and origin back. (Stick the bolt to the wall)
			}
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

//TODO: Fully predict the fliying bolt.
void EV_FireCrossbow(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
	{
		if (0 != args->iparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE1, 0);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE3, 0);

		V_PunchAxis(0, -2.0);
	}
}
//======================
//	   CROSSBOW END
//======================

//======================
//	    RPG START
//======================
void EV_FireRpg(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/rocketfire1.wav", 0.9, ATTN_NORM, 0, PITCH_NORM);
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/glauncher.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(RPG_FIRE2, 0);

		V_PunchAxis(0, -5.0);
	}
}
//======================
//	     RPG END
//======================

//======================
//	    EGON END
//======================
int g_fireAnims1[] = {EGON_FIRE1, EGON_FIRE2, EGON_FIRE3, EGON_FIRE4};
int g_fireAnims2[] = {EGON_ALTFIRECYCLE};

BEAM* pBeam;
BEAM* pBeam2;
TEMPENTITY* pFlare; // Vit_amiN: egon's beam flare

void EV_EgonFlareCallback(struct tempent_s* ent, float frametime, float currenttime)
{
	float delta = currenttime - ent->tentOffset.z; // time past since the last scale
	if (delta >= ent->tentOffset.y)
	{
		ent->entity.curstate.scale += ent->tentOffset.x * delta;
		ent->tentOffset.z = currenttime;
	}
}

void EV_EgonFire(event_args_t* args)
{
	int idx, iFireMode;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	iFireMode = args->iparam2;
	bool iStartup = 0 != args->bparam1;


	if (iStartup)
	{
		if (iFireMode == FIRE_WIDE)
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.98, ATTN_NORM, 0, 125);
		else
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.9, ATTN_NORM, 0, 100);
	}
	else
	{
		//If there is any sound playing already, kill it.
		//This is necessary because multiple sounds can play on the same channel at the same time.
		//In some cases, more than 1 run sound plays when the egon stops firing, in which case only the earliest entry in the list is stopped.
		//This ensures no more than 1 of those is ever active at the same time.
		gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, EGON_SOUND_RUN);

		if (iFireMode == FIRE_WIDE)
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.98, ATTN_NORM, 0, 125);
		else
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.9, ATTN_NORM, 0, 100);
	}

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
		gEngfuncs.pEventAPI->EV_WeaponAnimation(g_fireAnims1[gEngfuncs.pfnRandomLong(0, 3)], 0);

	if (iStartup && EV_IsLocal(idx) && !pBeam && !pBeam2 && !pFlare && 0 != cl_lw->value) //Adrian: Added the cl_lw check for those lital people that hate weapon prediction.
	{
		Vector vecSrc, vecEnd, angles, forward, right, up;
		pmtrace_t tr;

		cl_entity_t* pl = gEngfuncs.GetEntityByIndex(idx);

		if (pl)
		{
			VectorCopy(gHUD.m_vecAngles, angles);

			AngleVectors(angles, forward, right, up);

			EV_GetGunPosition(args, vecSrc, pl->origin);

			VectorMA(vecSrc, 2048, forward, vecEnd);

			gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(0, 1);

			// Store off the old count
			gEngfuncs.pEventAPI->EV_PushPMStates();

			// Now add in all of the players.
			gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

			gEngfuncs.pEventAPI->EV_SetTraceHull(2);
			gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);

			gEngfuncs.pEventAPI->EV_PopPMStates();

			int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex(EGON_BEAM_SPRITE);

			float r = 50.0f;
			float g = 50.0f;
			float b = 125.0f;

			//if ( IEngineStudio.IsHardware() )
			{
				r /= 255.0f;
				g /= 255.0f;
				b /= 255.0f;
			}


			pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 3.5, 0.2, 0.7, 55, 0, 0, r, g, b);

			if (pBeam)
				pBeam->flags |= (FBEAM_SINENOISE);

			pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 5.0, 0.08, 0.7, 25, 0, 0, r, g, b);

			// Vit_amiN: egon beam flare
			pFlare = gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 1.0,
				gEngfuncs.pEventAPI->EV_FindModelIndex(EGON_FLARE_SPRITE),
				kRenderGlow, kRenderFxNoDissipation, 1.0, 99999, FTENT_SPRCYCLE | FTENT_PERSIST);
		}
	}

	if (pFlare) // Vit_amiN: store the last mode for EV_EgonStop()
	{
		pFlare->tentOffset.x = (iFireMode == FIRE_WIDE) ? 1.0f : 0.0f;
	}
}

void EV_EgonStop(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, EGON_SOUND_RUN);

	//Only stop the sound if the event was sent by the same source as the owner of the egon.
	//If the local player owns the egon then only the local event should play this sound.
	//If another player owns it, only the server event should play it.
	if (0 != args->iparam1)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_OFF, 0.98, ATTN_NORM, 0, 100);

	if (EV_IsLocal(idx))
	{
		if (pBeam)
		{
			pBeam->die = 0.0;
			pBeam = NULL;
		}


		if (pBeam2)
		{
			pBeam2->die = 0.0;
			pBeam2 = NULL;
		}

		if (pFlare) // Vit_amiN: egon beam flare
		{
			pFlare->die = gEngfuncs.GetClientTime();

			if (gEngfuncs.GetMaxClients() == 1 || (pFlare->flags & FTENT_NOMODEL) == 0)
			{
				if (pFlare->tentOffset.x != 0.0f) // true for iFireMode == FIRE_WIDE
				{
					pFlare->callback = &EV_EgonFlareCallback;
					pFlare->fadeSpeed = 2.0;			// fade out will take 0.5 sec
					pFlare->tentOffset.x = 10.0;		// scaling speed per second
					pFlare->tentOffset.y = 0.1;			// min time between two scales
					pFlare->tentOffset.z = pFlare->die; // the last callback run time
					pFlare->flags = FTENT_FADEOUT | FTENT_CLIENTCUSTOM;
				}
			}

			pFlare = NULL;
		}

		// HACK: only reset animation if the Egon is still equipped.
		if (g_CurrentWeaponId == WEAPON_EGON)
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(EGON_IDLE1, 0);
		}
	}
}
//======================
//	    EGON END
//======================

//======================
//	   HORNET START
//======================
void EV_HornetGunFire(event_args_t* args)
{
	int idx;
	Vector origin, angles;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
	{
		V_PunchAxis(0, gEngfuncs.pfnRandomLong(0, 2));
		gEngfuncs.pEventAPI->EV_WeaponAnimation(HGUN_SHOOT, 0);
	}

	switch (gEngfuncs.pfnRandomLong(0, 2))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire1.wav", 1, ATTN_NORM, 0, 100);
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire2.wav", 1, ATTN_NORM, 0, 100);
		break;
	case 2:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire3.wav", 1, ATTN_NORM, 0, 100);
		break;
	}
}
//======================
//	   HORNET END
//======================

//======================
//	   TRIPMINE START
//======================
//We only check if it's possible to put a trip mine
//and if it is, then we play the animation. Server still places it.
void EV_TripmineFire(event_args_t* args)
{
	int idx;
	Vector vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy(args->origin, vecSrc);
	VectorCopy(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	// Grab predicted result for local player
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);

	vecSrc = vecSrc + view_ofs;

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + forward * 128, PM_NORMAL, -1, &tr);

	//Hit something solid
	if (tr.fraction < 1.0)
		gEngfuncs.pEventAPI->EV_WeaponAnimation(TRIPMINE_DRAW, 0);

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   TRIPMINE END
//======================

//======================
//	   SQUEAK START
//======================
void EV_SnarkFire(event_args_t* args)
{
	int idx;
	Vector vecSrc, angles, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy(args->origin, vecSrc);
	VectorCopy(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	if (0 != args->ducking)
		vecSrc = vecSrc - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc + forward * 20, vecSrc + forward * 64, PM_NORMAL, -1, &tr);

	//Find space to drop the thing.
	if (tr.allsolid == 0 && tr.startsolid == 0 && tr.fraction > 0.25)
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SQUEAK_THROW, 0);

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   SQUEAK END
//======================

//======================
//	DESERT EAGLE START
//======================

//exact same as the values from deserteagle.cpp, these are the animation names
enum desert_eagle_e
{
	DESERT_EAGLE_IDLE1 = 0,
	DESERT_EAGLE_IDLE2,
	DESERT_EAGLE_IDLE3,
	DESERT_EAGLE_IDLE4,
	DESERT_EAGLE_IDLE5,
	DESERT_EAGLE_SHOOT,
	DESERT_EAGLE_SHOOT_EMPTY,
	DESERT_EAGLE_RELOAD,
	DESERT_EAGLE_RELOAD_NOT_EMPTY,
	DESERT_EAGLE_DRAW,
	DESERT_EAGLE_HOLSTER,
};

void EV_FireDesertEagle(event_args_t* args)
{
	//variables, copy/paste code, etc...
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSRC, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/50ae.mdl"); //casing model

	//if the entity firing this event is the player...
	if (EV_IsLocal(idx))
	{
		//...render muzzleflash...
		EV_MuzzleFlash();

		//...show the gun firing, or firing and locking to the rear if it's the last round in the magazine...
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? DESERT_EAGLE_SHOOT_EMPTY : DESERT_EAGLE_SHOOT, 0);

		//...and recoil the camera
		V_PunchAxis(0, -4.5);
	}

	//eject the casing!!!
	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	//make a very loud bang

	//make a very loud bang
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/deagle/deagle_fire.wav", gEngfuncs.pfnRandomFloat(0.92, 1), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	//throw some metal down-range
	EV_GetGunPosition(args, vecSRC, origin);
	EV_HLDM_MuzzleFlash(vecSRC, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));
	VectorCopy(forward, vecAiming);
	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSRC, vecAiming, 8192, BULLET_PLAYER_357, 0, 0, args->fparam1, args->fparam2);
}

//======================
//	 DESERT EAGLE END
//======================

//======================
//		M92 START
//======================

//exact same as the values from m92.cpp, these are the animation names
enum m92_e
{
	M92_IDLE1 = 0,
	M92_IDLE2,
	M92_IDLE3,
	M92_FIDGET,
	M92_SHOOT,
	M92_SHOOT_EMPTY,
	M92_RELOAD,
	M92_RELOAD_NOT_EMPTY,
	M92_DRAW,
	M92_HOLSTER,
};

void EV_FireM92(event_args_t* args)
{
	//variables, copy/paste code, etc...
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSRC, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/9x19mm.mdl"); //casing model

	//if the entity firing this event is the player...
	if (EV_IsLocal(idx))
	{
		//...render muzzleflash...
		EV_MuzzleFlash();

		//...show the gun firing, or firing and locking to the rear if it's the last round in the magazine...
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? M92_SHOOT_EMPTY : M92_SHOOT, 0);

		//...and recoil the camera
		V_PunchAxis(0, -2.5);
	}

	//eject the casing!!!
	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	//make a very loud bang
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m92/m92_fire.wav", gEngfuncs.pfnRandomFloat(0.92, 1), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	//throw some metal down-range
	EV_GetGunPosition(args, vecSRC, origin);
	EV_HLDM_MuzzleFlash(vecSRC, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));
	VectorCopy(forward, vecAiming);
	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSRC, vecAiming, 8192, BULLET_PLAYER_M92, 0, 0, args->fparam1, args->fparam2);
}

//======================
//		M92 END
//======================

//======================
//		M4 START
//======================

//exact same as the values from m4.cpp, these are the animation names
enum m4_e
{
	M4_LONGIDLE = 0,
	M4_IDLE,
	M4_RELOAD_EMPTY,
	M4_RELOAD,
	M4_DEPLOY,
	M4_FIRE1,
	M4_FIRE2,
	M4_FIRE3,
};

void EV_FireM4(event_args_t* args)
{
	//variables, copy/paste code, etc...
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSRC, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/556x45.mdl"); //casing model

	//if the entity firing this event is the player...
	if (EV_IsLocal(idx))
	{
		//...render muzzleflash...
		EV_MuzzleFlash();

		//...show the gun firing!!
		switch (gEngfuncs.pfnRandomLong(0, 2))
		{
		case 0:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(M4_FIRE1, 0);
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(M4_FIRE2, 0);
			break;
		case 2:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(M4_FIRE3, 0);
			break;
		}

		//...and recoil the camera
		V_Recoil(0.5);
		V_PunchAxis(0, -1.5);
	}

	//eject the casing!!!
	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	switch (gEngfuncs.pfnRandomLong(0, 3))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m4/m4_fire1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m4/m4_fire2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 2:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m4/m4_fire3.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 3:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m4/m4_fire4.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	}

	//throw some metal down-range
	EV_GetGunPosition(args, vecSRC, origin);
	EV_HLDM_MuzzleFlash(vecSRC, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));
	VectorCopy(forward, vecAiming);
	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSRC, vecAiming, 8192, BULLET_PLAYER_M4, 0, 0, args->fparam1, args->fparam2);
}

//======================
//		M4 END
//======================

//======================
//		BDRifle START
//======================

//exact same as the values from bd_rifle.cpp, these are the animation names
enum bdrifle_e
{
	BDRIFLE_LONGIDLE = 0,
	BDRIFLE_IDLE,
	BDRIFLE_RELOAD_EMPTY,
	BDRIFLE_RELOAD,
	BDRIFLE_DEPLOY,
	BDRIFLE_FIRE1,
	BDRIFLE_FIRE2,
	BDRIFLE_FIRE3,
};

void EV_FireBDRifle(event_args_t* args)
{
	//variables, copy/paste code, etc...
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSRC, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/556x45.mdl"); //casing model

	//if the entity firing this event is the player...
	if (EV_IsLocal(idx))
	{
		//...render muzzleflash...
		EV_MuzzleFlash();

		//...show the gun firing!!
		switch (gEngfuncs.pfnRandomLong(0, 2))
		{
		case 0:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BDRIFLE_FIRE1, 0);
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BDRIFLE_FIRE2, 0);
			break;
		case 2:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BDRIFLE_FIRE3, 0);
			break;
		}

		//...and recoil the camera
		V_Recoil(0.5);
		V_PunchAxis(0, -1.5);
	}

	//eject the casing!!!
	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	switch (gEngfuncs.pfnRandomLong(0, 3))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/brutaldoom/bdrifle_fire1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/brutaldoom/bdrifle_fire2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 2:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/brutaldoom/bdrifle_fire3.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 3:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/brutaldoom/bdrifle_fire4.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	}

	//throw some metal down-range
	EV_GetGunPosition(args, vecSRC, origin);
	EV_HLDM_MuzzleFlash(vecSRC, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));
	VectorCopy(forward, vecAiming);
	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSRC, vecAiming, 8192, BULLET_PLAYER_M4, 0, 0, args->fparam1, args->fparam2);
}

//======================
//		BDRifle END
//======================

//======================
//		DUKE4 PISTOL START
//======================

//exact same as the values from duke4_pistol.cpp, these are the animation names
enum duke4pistol_e
{
	DUKE4_PISTOL_IDLE1 = 0,
	DUKE4_PISTOL_IDLE2,
	DUKE4_PISTOL_IDLE3,
	DUKE4_PISTOL_FIDGET,
	DUKE4_PISTOL_SHOOT,
	DUKE4_PISTOL_SHOOT_EMPTY,
	DUKE4_PISTOL_RELOAD,
	DUKE4_PISTOL_RELOAD_NOT_EMPTY,
	DUKE4_PISTOL_DRAW,
	DUKE4_PISTOL_HOLSTER,
};

void EV_FireDuke4Pistol(event_args_t* args)
{
	//variables, copy/paste code, etc...
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSRC, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shells/50ae.mdl"); //casing model

	//if the entity firing this event is the player...
	if (EV_IsLocal(idx))
	{
		//...render muzzleflash...
		EV_MuzzleFlash();

		//...show the gun firing, or firing and locking to the rear if it's the last round in the magazine...
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? DUKE4_PISTOL_SHOOT_EMPTY : DUKE4_PISTOL_SHOOT, 0);

		//...and recoil the camera
		V_PunchAxis(0, -4);
	}

	//eject the casing!!!
	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	//make a very loud bang
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/duke4/duke4_pistol_fire.wav", gEngfuncs.pfnRandomFloat(0.92, 1), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	//throw some metal down-range
	EV_GetGunPosition(args, vecSRC, origin);
	EV_HLDM_MuzzleFlash(vecSRC, 1.0 + gEngfuncs.pfnRandomFloat(-0.2, 0.2));
	VectorCopy(forward, vecAiming);
	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSRC, vecAiming, 8192, BULLET_PLAYER_357, 0, 0, args->fparam1, args->fparam2);
}

//======================
//		DUKE4 PISTOL END
//======================


void EV_TrainPitchAdjust(event_args_t* args)
{
	int idx;
	Vector origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	bool stop;

	char sz[256];

	idx = args->entindex;

	VectorCopy(args->origin, origin);

	us_params = (unsigned short)args->iparam1;
	stop = 0 != args->bparam1;

	m_flVolume = (float)(us_params & 0x003f) / 40.0;
	noise = (int)(((us_params) >> 12) & 0x0007);
	pitch = (int)(10.0 * (float)((us_params >> 6) & 0x003f));

	switch (noise)
	{
	case 1:
		strcpy(sz, "plats/ttrain1.wav");
		break;
	case 2:
		strcpy(sz, "plats/ttrain2.wav");
		break;
	case 3:
		strcpy(sz, "plats/ttrain3.wav");
		break;
	case 4:
		strcpy(sz, "plats/ttrain4.wav");
		break;
	case 5:
		strcpy(sz, "plats/ttrain6.wav");
		break;
	case 6:
		strcpy(sz, "plats/ttrain7.wav");
		break;
	default:
		// no sound
		strcpy(sz, "");
		return;
	}

	if (stop)
	{
		gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, sz);
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch);
	}
}

bool EV_TFC_IsAllyTeam(int iTeam1, int iTeam2)
{
	return false;
}
