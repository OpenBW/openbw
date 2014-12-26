
template<typename T>
T load_dat(const char*fn) {
	FILE*f = fopen(fn, "rb");
	if (!fn) xcept("failed to open %s for reading", fn);
	T r;
	if (!fread(&r, sizeof(r), 1, f)) {
		fclose(f);
		xcept("failed to read %d bytes from %s", sizeof(r), fn);
	}
	fclose(f);
	return r;
}

struct units_dat {
	static const size_t total_count = 228;
	static const size_t units_count = 106;
	static const size_t buildings_count = 96;
	uint8_t Graphics[total_count];
	uint16_t Subunit1[total_count];
	uint16_t Subunit2[total_count];
	uint16_t Infestation[buildings_count];
	uint32_t ConstructionAnimation[total_count];
	uint8_t UnitDirection[total_count];
	uint8_t ShieldEnable[total_count];
	uint16_t ShieldAmount[total_count];
	int32_t HitPoints[total_count];
	uint8_t ElevationLevel[total_count];
	uint8_t Unknown[total_count];
	uint8_t Sublabel[total_count];
	uint8_t CompAIIdle[total_count];
	uint8_t HumanAIIdle[total_count];
	uint8_t ReturntoIdle[total_count];
	uint8_t AttackUnit[total_count];
	uint8_t AttackMove[total_count];
	uint8_t GroundWeapon[total_count];
	uint8_t MaxGroundHits[total_count];
	uint8_t AirWeapon[total_count];
	uint8_t MaxAirHits[total_count];
	uint8_t AIInternal[total_count];
	uint32_t SpecialAbilityFlags[total_count];
	uint8_t TargetAcquisitionRange[total_count];
	uint8_t SightRange[total_count];
	uint8_t ArmorUpgrade[total_count];
	uint8_t UnitSize[total_count];
	uint8_t Armor[total_count];
	uint8_t RightClickAction[total_count];
	uint16_t ReadySound[units_count];
	uint16_t WhatSoundStart[total_count];
	uint16_t WhatSoundEnd[total_count];
	uint16_t PissSoundStart[units_count];
	uint16_t PissSoundEnd[units_count];
	uint16_t YesSoundStart[units_count];
	uint16_t YesSoundEnd[units_count];
	uint16_t StarEditPlacementBoxWidth[total_count];
	uint16_t StarEditPlacementBoxHeight[total_count];
	uint16_t AddonHorizontal[buildings_count];
	uint16_t AddonVertical[buildings_count];
	uint16_t UnitSizeLeft[total_count];
	uint16_t UnitSizeUp[total_count];
	uint16_t UnitSizeRight[total_count];
	uint16_t UnitSizeDown[total_count];
	uint16_t Portrait[total_count];
	uint16_t MineralCost[total_count];
	uint16_t VespeneCost[total_count];
	uint16_t BuildTime[total_count];
	uint16_t Unknown1[total_count];
	uint8_t StarEditGroupFlags[total_count];
	uint8_t SupplyProvided[total_count];
	uint8_t SupplyRequired[total_count];
	uint8_t SpaceRequired[total_count];
	uint8_t SpaceProvided[total_count];
	uint16_t BuildScore[total_count];
	uint16_t DestroyScore[total_count];
	uint16_t UnitMapString[total_count];
	uint8_t BroodwarUnitFlag[total_count];
	uint16_t StarEditAvailabilityFlags[total_count];
};

static_assert(sizeof(units_dat) == 19876, "units_dat is wrong size");

units_dat load_units_dat(const char*fn) {
	return load_dat<units_dat>(fn);
}


#pragma pack(1)
struct weapons_dat {
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

static_assert(sizeof(weapons_dat) == 5460, "weapons_dat is wrong size");

weapons_dat load_weapons_dat(const char*fn) {
	return load_dat<weapons_dat>(fn);
}
