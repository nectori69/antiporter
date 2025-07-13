#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"

// These correspond directly to the sequences in the weapon's view model
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

LINK_ENTITY_TO_CLASS(weapon_m4, CM4)

void CM4::Spawn()
{
	// Define the classname of the entity
	// This is the name you should use to reference this entity name in your code base.
	pev->classname = MAKE_STRING("weapon_m4");

	// Precache the weapon models and sounds
	// This might be called by the engine separately, but it's best to call it here as well just in case.
	Precache();

	// Set the weapon ID
	m_iId = WEAPON_M4;

	// Tell the engine about the weapon's world model
	SET_MODEL(ENT(pev), "models/w_m4.mdl");

	// Set the default ammo value for the weapon
	m_iDefaultAmmo = M4_DEFAULT_GIVE;

	// Set up some default behaviour for the weapon
	// This will tell the engine that the weapon should "fall" to the ground when it spawns.
	// It also sets up the behaviour so that the weapon is equipped when the player touches it.
	FallInit();
}

void CM4::Precache()
{
	// Precache models
	PRECACHE_MODEL("models/v_m4.mdl");
	PRECACHE_MODEL("models/w_m4.mdl");
	PRECACHE_MODEL("models/p_m4.mdl");
	PRECACHE_MODEL("models/shells/556x45.mdl");

	// Precache sounds
	PRECACHE_SOUND("weapons/m4/m4_fire1.wav"); // H to the K
	PRECACHE_SOUND("weapons/m4/m4_fire2.wav"); // H to the K
	PRECACHE_SOUND("weapons/m4/m4_fire3.wav"); // H to the K
	PRECACHE_SOUND("weapons/m4/m4_fire4.wav"); // H to the K

	// Precache fire event
	m_usFireM4 = PRECACHE_EVENT(1, "events/m4.sc");
}

bool CM4::GetItemInfo(ItemInfo* p)
{
	// This should match the classname - the HUD uses it to find the matching .txt file in the sprites/ folder
	p->pszName = STRING(pev->classname);

	// The "primary" ammo type for this weapon and the maximum ammo of that type that the player can hold
	p->pszAmmo1 = "9MM";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;

	// Same as above, but for "secondary" ammo. This should be NULL and -1 for weapons with no secondary
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;

	// The size of a full clip
	p->iMaxClip = M4_MAX_CLIP;

	// Special weapon flags - leave this as 0 for now, this is covered in a different article
	p->iFlags = 0;

	// The "slot" in the HUD that the weapon appears in. This is a pistol, so it goes into slot 1 with the others
	p->iSlot = 2;

	// The "position" in the HUD that the weapon is added to. We'll put this after the deagle (which is in slot 2)
	p->iPosition = 3;

	// Set the ID and auto-switching weights of the weapon
	p->iId = m_iId = WEAPON_M4;
	p->iWeight = M4_WEIGHT;

	return true;
}

void CM4::SecondaryAttack()
{
}

void CM4::PrimaryAttack()
{
	// Don't fire underwater - waterlevel 3 indicates that the player's head is underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		// Play a "click" and don't allow another primary attack for a short time
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	// Check if the clip is empty
	if (m_iClip <= 0)
	{
		if (!m_fInReload && m_fFireOnEmpty)
		{
			// If the player has fired previously, but is still holding the attack button down,
			// just play the empty "click" sound until the player releases the button.
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	// If we get to this point - we're shooting!

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	// Decrease the number of bullets in the clip
	m_iClip--;

	// Add a muzzleflash to the player effects
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	// Player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Set global vectors in the engine (don't ask)
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	// kick the camera up a bit
	Recoil(15.0);

	// Shoot bullets!
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
	Vector vecDir = m_pPlayer->FireBulletsPlayer(
		1,					   // Number of bullets to shoot
		vecSrc,				   // The source of the bullets (i.e. the gun)
		vecAiming,			   // The direction to fire in (i.e. where the player is pointing)
		VECTOR_CONE_1DEGREES,  // The accuracy spread of the weapon
		8192,				   // The distance the bullet can go (8192 is the limit for the engine)
		BULLET_PLAYER_M4,	   // The type of bullet being fired
		0,					   // Number of tracer bullets to fire (none in this case)
		0,					   // Set to non-zero to override the amount of damage (usually, leave this as 0)
		m_pPlayer->pev,		   // Attacker entity
		m_pPlayer->random_seed // The random seed
	);

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM4, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, 0, 0, (m_iClip == 0) ? 1 : 0, 0);

	// If the clip is now empty and there's no more ammo available, update the HEV
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.075;

	// Set the time until the weapon should start idling again
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

void CM4::Recoil(float flRecoil)
{
	m_pPlayer->pev->angles[0] += flRecoil;
}

bool CM4::Deploy()
{
	return DefaultDeploy(
		"models/v_m4.mdl", // Weapon view model
		"models/p_m4.mdl", // Weapon player model
		M4_DEPLOY,			// "Draw" animation index for the view model
		"onehanded",		// Third person animation set for the weapon. We'll use the generic "onehanded" animation set
		pev->body			// The weapon model's "body" pointer
	);
}

void CM4::Holster()
{
	// Cancel any reload in progress
	m_fInReload = false;

	// Delay the next player's attack for about the same time as the holster animation takes
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	// Play the "holster" animation
}

void CM4::Reload()
{
	// Don't reload if the player doesn't have any ammo
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	int iResult;

	// The view model has two different animations depending on if there are any bullets in the clip
	if (m_iClip == 0)
		iResult = DefaultReload(M4_MAX_CLIP, M4_RELOAD_EMPTY, (100 / 35.0));
	else
		iResult = DefaultReload(M4_MAX_CLIP, M4_RELOAD, (90 / 35.0));

	if (iResult)
	{
		// If the reload happened, then reset the weapon's idle time
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	}
}

void CM4::WeaponIdle()
{
	// This is used in conjunction with the PlayEmptySound function.
	// This resets a flag so the "click" for an empty weapon can be replayed after a short delay
	ResetEmptySound();

	// Despite the name, this will SET the autoaim vector.
	// 10 degrees is what the magnum uses, so we'll use the same.
	m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);

	// Exit out of the method if the weapon time hasn't passed yet or if the clip is empty
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() || m_iClip <= 0)
		return;

	// Weapon idle is only called after the weapon hasn't been used (fired or reloaded)
	// for a while. In this case we want to play one of the idle animations for the weapon.
	// The desert eagle view model has 5 different idle animations, and we'll give each one
	// a 20% chance of playing, using the random number util function.
	int iAnim;
	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);

	if (flRand <= 0.7)
	{
		// The numbers here (110.0 / 20.0) are a way to represent the time taken by the
		// animation, so the next idle animation isn't played before the current one has
		// been completed. This animation is 110 frames long, and runs at 20 frames per second.
		iAnim = M4_IDLE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (110 / 20.0);
	}
	else
	{
		iAnim = M4_LONGIDLE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (130 / 30.0);
	}

	// Play the idle animation
	SendWeaponAnim(iAnim, pev->body);
}