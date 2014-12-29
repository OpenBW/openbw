
template<typename T>
T load_dat(const char*fn) {
	SFile f(fn);
	T r;
	f.read(&r, sizeof(r));
	return r;
// 	FILE*f = fopen(fn, "rb");
// 	if (!fn) xcept("failed to open %s for reading", fn);
// 	T r;
// 	if (!fread(&r, sizeof(r), 1, f)) {
// 		fclose(f);
// 		xcept("failed to read %d bytes from %s", sizeof(r), fn);
// 	}
// 	fclose(f);
// 	return r;
}

struct units_dat_t {
	static const size_t total_count = 228;
	static const size_t units_count = 106;
	static const size_t buildings_count = 96;

	std::array<uint8_t, total_count> Graphics;
	std::array<uint16_t, total_count> Subunit1;
	std::array<uint16_t, total_count> Subunit2;
	std::array<uint16_t, buildings_count> Infestation;
	std::array<uint32_t, total_count> ConstructionAnimation;
	std::array<uint8_t, total_count> UnitDirection;
	std::array<uint8_t, total_count> ShieldEnable;
	std::array<uint16_t, total_count> ShieldAmount;
	std::array<int32_t, total_count> HitPoints;
	std::array<uint8_t, total_count> ElevationLevel;
	std::array<uint8_t, total_count> Unknown;
	std::array<uint8_t, total_count> Sublabel;
	std::array<uint8_t, total_count> CompAIIdle;
	std::array<uint8_t, total_count> HumanAIIdle;
	std::array<uint8_t, total_count> ReturntoIdle;
	std::array<uint8_t, total_count> AttackUnit;
	std::array<uint8_t, total_count> AttackMove;
	std::array<uint8_t, total_count> GroundWeapon;
	std::array<uint8_t, total_count> MaxGroundHits;
	std::array<uint8_t, total_count> AirWeapon;
	std::array<uint8_t, total_count> MaxAirHits;
	std::array<uint8_t, total_count> AIInternal;
	std::array<uint32_t, total_count> SpecialAbilityFlags;
	std::array<uint8_t, total_count> TargetAcquisitionRange;
	std::array<uint8_t, total_count> SightRange;
	std::array<uint8_t, total_count> ArmorUpgrade;
	std::array<uint8_t, total_count> UnitSize;
	std::array<uint8_t, total_count> Armor;
	std::array<uint8_t, total_count> RightClickAction;
	std::array<uint16_t, units_count> ReadySound;
	std::array<uint16_t, total_count> WhatSoundStart;
	std::array<uint16_t, total_count> WhatSoundEnd;
	std::array<uint16_t, units_count> PissSoundStart;
	std::array<uint16_t, units_count> PissSoundEnd;
	std::array<uint16_t, units_count> YesSoundStart;
	std::array<uint16_t, units_count> YesSoundEnd;
	std::array<uint16_t, total_count> StarEditPlacementBoxWidth;
	std::array<uint16_t, total_count> StarEditPlacementBoxHeight;
	std::array<uint16_t, buildings_count> AddonHorizontal;
	std::array<uint16_t, buildings_count> AddonVertical;
	std::array<uint16_t, total_count> UnitSizeLeft;
	std::array<uint16_t, total_count> UnitSizeUp;
	std::array<uint16_t, total_count> UnitSizeRight;
	std::array<uint16_t, total_count> UnitSizeDown;
	std::array<uint16_t, total_count> Portrait;
	std::array<uint16_t, total_count> MineralCost;
	std::array<uint16_t, total_count> VespeneCost;
	std::array<uint16_t, total_count> BuildTime;
	std::array<uint16_t, total_count> Unknown1;
	std::array<uint8_t, total_count> StarEditGroupFlags;
	std::array<uint8_t, total_count> SupplyProvided;
	std::array<uint8_t, total_count> SupplyRequired;
	std::array<uint8_t, total_count> SpaceRequired;
	std::array<uint8_t, total_count> SpaceProvided;
	std::array<uint16_t, total_count> BuildScore;
	std::array<uint16_t, total_count> DestroyScore;
	std::array<uint16_t, total_count> UnitMapString;
	std::array<uint8_t, total_count> BroodwarUnitFlag;
	std::array<uint16_t, total_count> StarEditAvailabilityFlags;
};

static_assert(sizeof(units_dat_t) == 19876, "units_dat is wrong size");

units_dat_t load_units_dat(const char*fn) {
	return load_dat<units_dat_t>(fn);
}


#pragma pack(1)
struct weapons_dat_t {
	static const size_t count = 130;
	uint16_t Label[count];
	uint32_t Graphics[count];
	uint8_t Unused[count];
	uint16_t TargetFlags[count];
	uint32_t MinimumRange[count];
	uint32_t MaximumRange[count];
	uint8_t DamageUpgrade[count];
	uint8_t WeaponType[count];
	uint8_t WeaponBehavior[count];
	uint8_t RemoveAfter[count];
	uint8_t ExplosionType[count];
	uint16_t InnerSplashRange[count];
	uint16_t MediumSplashRange[count];
	uint16_t OuterSplashRange[count];
	uint16_t DamageAmount[count];
	uint16_t DamageBonus[count];
	uint8_t WeaponCooldown[count];
	uint8_t DamageFactor[count];
	uint8_t AttackAngle[count];
	uint8_t LaunchSpin[count];
	int8_t ForwardOffset[count];
	int8_t UpwardOffset[count];
	uint16_t TargetErrorMessage[count];
	uint16_t Icon[count];
};
#pragma pack()

static_assert(sizeof(weapons_dat_t) == 5460, "weapons_dat is wrong size");

weapons_dat_t load_weapons_dat(const char*fn) {
	return load_dat<weapons_dat_t>(fn);
}

#pragma pack(1)
struct upgrades_dat_t {
	static const size_t count = 61;
	std::array<uint16_t,count> MineralCostBase;
	std::array<uint16_t, count> MineralCostFactor;
	std::array<uint16_t, count> VespeneCostBase;
	std::array<uint16_t, count> BespeneCostFactor;
	std::array<uint16_t, count> ResearchTimeBase;
	std::array<uint16_t, count> ResearchTimeFactor;
	std::array<uint16_t, count> Unknown;
	std::array<uint16_t, count> Icon;
	std::array<uint16_t, count> Label;
	std::array<uint8_t, count> Race;
	std::array<uint8_t, count> MaxRepeats;
	std::array<uint8_t, count> BroodwarOnly;
};
#pragma pack()

static_assert(sizeof(upgrades_dat_t) == 1281, "upgrades_dat_t is wrong size");

upgrades_dat_t load_upgrades_dat(const char*fn) {
	return load_dat<upgrades_dat_t>(fn);
}

#pragma pack(1)
struct techdata_dat_t {
	static const size_t count = 44;
	std::array<uint16_t, count> mineralCost;
	std::array<uint16_t, count> gasCost;
	std::array<uint16_t, count> researchTime;
	std::array<uint16_t, count> energyCost;
	std::array<uint32_t, count> unknown;
	std::array<uint16_t, count> icon;
	std::array<uint16_t, count> label;
	std::array<uint8_t, count> race;
	std::array<uint8_t, count> unused;
	std::array<uint8_t, count> isBroodWarOnly;
};
#pragma pack()

static_assert(sizeof(techdata_dat_t) == 836, "techdata_dat_t is wrong size");

techdata_dat_t load_techdata_dat(const char*fn) {
	return load_dat<techdata_dat_t>(fn);
}

#pragma pack(1)
struct flingy_dat_t {
	static const size_t count = 209;
	std::array<uint16_t, count> sprite;
	std::array<uint32_t, count> topSpeed;
	std::array<uint16_t, count> acceleration;
	std::array<uint32_t, count> haltDistance;
	std::array<uint8_t, count> turnSpeed;
	std::array<uint8_t, count> unused;
	std::array<uint8_t, count> moveControl;
};
#pragma pack()

static_assert(sizeof(flingy_dat_t) == 3135, "flingy_dat_t is wrong size");

flingy_dat_t load_flingy_dat(const char*fn) {
	return load_dat<flingy_dat_t>(fn);
}

#pragma pack(1)
struct sprites_dat_t {
	static const size_t count = 517;
	static const size_t selectable_count = 387;
	std::array<uint16_t, count> image;
	std::array<uint8_t, selectable_count> healthBarSize;
	std::array<uint8_t, count> unknown;
	std::array<uint8_t, count> isVisible;
	std::array<uint8_t, selectable_count> selectionCircle;
	std::array<uint8_t, selectable_count> selectionCircleVPos;
};
#pragma pack()

static_assert(sizeof(sprites_dat_t) == 3229, "sprites_dat_t is wrong size");

sprites_dat_t load_sprites_dat(const char*fn) {
	return load_dat<sprites_dat_t>(fn);
}

#pragma pack(1)
struct images_dat_t {
	static const size_t count = 999;
	std::array<uint32_t, count> grpFile;
	std::array<uint8_t, count> isTurnable;
	std::array<uint8_t, count> isClickable;
	std::array<uint8_t, count> useFullIscript;
	std::array<uint8_t, count> drawIfCloaked;
	std::array<uint8_t, count> drawFunction;
	std::array<uint8_t, count> remapping;
	std::array<uint32_t, count> iscriptEntry;
	std::array<uint32_t, count> shieldOverlayLO;
	std::array<uint32_t, count> attackOverlayLO;
	std::array<uint32_t, count> injuryOverlayLO;
	std::array<uint32_t, count> specialOverlayLO;
	std::array<uint32_t, count> landingDustLO;
	std::array<uint32_t, count> liftOffDustLO;
};
#pragma pack()

static_assert(sizeof(images_dat_t) == 37962, "images_dat is wrong size");

images_dat_t load_images_dat(const char*fn) {
	return load_dat<images_dat_t>(fn);
}




