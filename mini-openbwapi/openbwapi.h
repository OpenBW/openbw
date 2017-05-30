#ifndef MINI_OPENBWAPI_OPENBWAPI_H
#define MINI_OPENBWAPI_OPENBWAPI_H

#define MINI_OPENBWAPI
#define OPENBW_BWAPI

//#define OPENBW_ENABLE_UI

#include <utility>
#include <cstdarg>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <string>
#include <functional>
#include <stdexcept>
#include <array>
#include <tuple>

namespace bwgame {
	struct weapon_type_t;
	struct unit_type_t;
	struct unit_t;
	struct bullet_t;

	struct state_functions;
}

namespace OpenBWAPI {

class PlayerInterface;
using Player = PlayerInterface*;

class UnitInterface;
using Unit = UnitInterface*;

struct openbwapi_functions;

namespace UnitTypes {
	namespace Enum {
		enum {
			Terran_Marine,
			Terran_Ghost,
			Terran_Vulture,
			Terran_Goliath,
			Terran_Goliath_Turret,
			Terran_Siege_Tank_Tank_Mode,
			Terran_Siege_Tank_Tank_Mode_Turret,
			Terran_SCV,
			Terran_Wraith,
			Terran_Science_Vessel,
			Hero_Gui_Montag,
			Terran_Dropship,
			Terran_Battlecruiser,
			Terran_Vulture_Spider_Mine,
			Terran_Nuclear_Missile,
			Terran_Civilian,
			Hero_Sarah_Kerrigan,
			Hero_Alan_Schezar,
			Hero_Alan_Schezar_Turret,
			Hero_Jim_Raynor_Vulture,
			Hero_Jim_Raynor_Marine,
			Hero_Tom_Kazansky,
			Hero_Magellan,
			Hero_Edmund_Duke_Tank_Mode,
			Hero_Edmund_Duke_Tank_Mode_Turret,
			Hero_Edmund_Duke_Siege_Mode,
			Hero_Edmund_Duke_Siege_Mode_Turret,
			Hero_Arcturus_Mengsk,
			Hero_Hyperion,
			Hero_Norad_II,
			Terran_Siege_Tank_Siege_Mode,
			Terran_Siege_Tank_Siege_Mode_Turret,
			Terran_Firebat,
			Spell_Scanner_Sweep,
			Terran_Medic,
			Zerg_Larva,
			Zerg_Egg,
			Zerg_Zergling,
			Zerg_Hydralisk,
			Zerg_Ultralisk,
			Zerg_Broodling,
			Zerg_Drone,
			Zerg_Overlord,
			Zerg_Mutalisk,
			Zerg_Guardian,
			Zerg_Queen,
			Zerg_Defiler,
			Zerg_Scourge,
			Hero_Torrasque,
			Hero_Matriarch,
			Zerg_Infested_Terran,
			Hero_Infested_Kerrigan,
			Hero_Unclean_One,
			Hero_Hunter_Killer,
			Hero_Devouring_One,
			Hero_Kukulza_Mutalisk,
			Hero_Kukulza_Guardian,
			Hero_Yggdrasill,
			Terran_Valkyrie,
			Zerg_Cocoon,
			Protoss_Corsair,
			Protoss_Dark_Templar,
			Zerg_Devourer,
			Protoss_Dark_Archon,
			Protoss_Probe,
			Protoss_Zealot,
			Protoss_Dragoon,
			Protoss_High_Templar,
			Protoss_Archon,
			Protoss_Shuttle,
			Protoss_Scout,
			Protoss_Arbiter,
			Protoss_Carrier,
			Protoss_Interceptor,
			Hero_Dark_Templar,
			Hero_Zeratul,
			Hero_Tassadar_Zeratul_Archon,
			Hero_Fenix_Zealot,
			Hero_Fenix_Dragoon,
			Hero_Tassadar,
			Hero_Mojo,
			Hero_Warbringer,
			Hero_Gantrithor,
			Protoss_Reaver,
			Protoss_Observer,
			Protoss_Scarab,
			Hero_Danimoth,
			Hero_Aldaris,
			Hero_Artanis,
			Critter_Rhynadon,
			Critter_Bengalaas,
			Special_Cargo_Ship,
			Special_Mercenary_Gunship,
			Critter_Scantid,
			Critter_Kakaru,
			Critter_Ragnasaur,
			Critter_Ursadon,
			Zerg_Lurker_Egg,
			Hero_Raszagal,
			Hero_Samir_Duran,
			Hero_Alexei_Stukov,
			Special_Map_Revealer,
			Hero_Gerard_DuGalle,
			Zerg_Lurker,
			Hero_Infested_Duran,
			Spell_Disruption_Web,
			Terran_Command_Center,
			Terran_Comsat_Station,
			Terran_Nuclear_Silo,
			Terran_Supply_Depot,
			Terran_Refinery,
			Terran_Barracks,
			Terran_Academy,
			Terran_Factory,
			Terran_Starport,
			Terran_Control_Tower,
			Terran_Science_Facility,
			Terran_Covert_Ops,
			Terran_Physics_Lab,
			Unused_Terran1,
			Terran_Machine_Shop,
			Unused_Terran2,
			Terran_Engineering_Bay,
			Terran_Armory,
			Terran_Missile_Turret,
			Terran_Bunker,
			Special_Crashed_Norad_II,
			Special_Ion_Cannon,
			Powerup_Uraj_Crystal,
			Powerup_Khalis_Crystal,
			Zerg_Infested_Command_Center,
			Zerg_Hatchery,
			Zerg_Lair,
			Zerg_Hive,
			Zerg_Nydus_Canal,
			Zerg_Hydralisk_Den,
			Zerg_Defiler_Mound,
			Zerg_Greater_Spire,
			Zerg_Queens_Nest,
			Zerg_Evolution_Chamber,
			Zerg_Ultralisk_Cavern,
			Zerg_Spire,
			Zerg_Spawning_Pool,
			Zerg_Creep_Colony,
			Zerg_Spore_Colony,
			Unused_Zerg1,
			Zerg_Sunken_Colony,
			Special_Overmind_With_Shell,
			Special_Overmind,
			Zerg_Extractor,
			Special_Mature_Chrysalis,
			Special_Cerebrate,
			Special_Cerebrate_Daggoth,
			Unused_Zerg2,
			Protoss_Nexus,
			Protoss_Robotics_Facility,
			Protoss_Pylon,
			Protoss_Assimilator,
			Unused_Protoss1,
			Protoss_Observatory,
			Protoss_Gateway,
			Unused_Protoss2,
			Protoss_Photon_Cannon,
			Protoss_Citadel_of_Adun,
			Protoss_Cybernetics_Core,
			Protoss_Templar_Archives,
			Protoss_Forge,
			Protoss_Stargate,
			Special_Stasis_Cell_Prison,
			Protoss_Fleet_Beacon,
			Protoss_Arbiter_Tribunal,
			Protoss_Robotics_Support_Bay,
			Protoss_Shield_Battery,
			Special_Khaydarin_Crystal_Form,
			Special_Protoss_Temple,
			Special_XelNaga_Temple,
			Resource_Mineral_Field,
			Resource_Mineral_Field_Type_2,
			Resource_Mineral_Field_Type_3,
			Unused_Cave,
			Unused_Cave_In,
			Unused_Cantina,
			Unused_Mining_Platform,
			Unused_Independant_Command_Center,
			Special_Independant_Starport,
			Unused_Independant_Jump_Gate,
			Unused_Ruins,
			Unused_Khaydarin_Crystal_Formation,
			Resource_Vespene_Geyser,
			Special_Warp_Gate,
			Special_Psi_Disrupter,
			Unused_Zerg_Marker,
			Unused_Terran_Marker,
			Unused_Protoss_Marker,
			Special_Zerg_Beacon,
			Special_Terran_Beacon,
			Special_Protoss_Beacon,
			Special_Zerg_Flag_Beacon,
			Special_Terran_Flag_Beacon,
			Special_Protoss_Flag_Beacon,
			Special_Power_Generator,
			Special_Overmind_Cocoon,
			Spell_Dark_Swarm,
			Special_Floor_Missile_Trap,
			Special_Floor_Hatch,
			Special_Upper_Level_Door,
			Special_Right_Upper_Level_Door,
			Special_Pit_Door,
			Special_Right_Pit_Door,
			Special_Floor_Gun_Trap,
			Special_Wall_Missile_Trap,
			Special_Wall_Flame_Trap,
			Special_Right_Wall_Missile_Trap,
			Special_Right_Wall_Flame_Trap,
			Special_Start_Location,
			Powerup_Flag,
			Powerup_Young_Chrysalis,
			Powerup_Psi_Emitter,
			Powerup_Data_Disk,
			Powerup_Khaydarin_Crystal,
			Powerup_Mineral_Cluster_Type_1,
			Powerup_Mineral_Cluster_Type_2,
			Powerup_Protoss_Gas_Orb_Type_1,
			Powerup_Protoss_Gas_Orb_Type_2,
			Powerup_Zerg_Gas_Sac_Type_1,
			Powerup_Zerg_Gas_Sac_Type_2,
			Powerup_Terran_Gas_Tank_Type_1,
			Powerup_Terran_Gas_Tank_Type_2,
		
			None,
		};
	}

	namespace {
	      int Resource_Vespene_Geyser = Enum::Resource_Vespene_Geyser;
	}
}


enum struct UpgradeTypes : int {
	Terran_Infantry_Armor,
	Terran_Vehicle_Plating,
	Terran_Ship_Plating,
	Zerg_Carapace,
	Zerg_Flyer_Carapace,
	Protoss_Ground_Armor,
	Protoss_Air_Armor,
	Terran_Infantry_Weapons,
	Terran_Vehicle_Weapons,
	Terran_Ship_Weapons,
	Zerg_Melee_Attacks,
	Zerg_Missile_Attacks,
	Zerg_Flyer_Attacks,
	Protoss_Ground_Weapons,
	Protoss_Air_Weapons,
	Protoss_Plasma_Shields,
	U_238_Shells,
	Ion_Thrusters,
	unk_18,
	Titan_Reactor,
	Ocular_Implants,
	Moebius_Reactor,
	Apollo_Reactor,
	Colossus_Reactor,
	Ventral_Sacs,
	Antennae,
	Pneumatized_Carapace,
	Metabolic_Boost,
	Adrenal_Glands,
	Muscular_Augments,
	Grooved_Spines,
	Gamete_Meiosis,
	Metasynaptic_Node,
	Singularity_Charge,
	Leg_Enhancements,
	Scarab_Damage,
	Reaver_Capacity,
	Gravitic_Drive,
	Sensor_Array,
	Gravitic_Boosters,
	Khaydarin_Amulet,
	Apial_Sensors,
	Gravitic_Thrusters,
	Carrier_Capacity,
	Khaydarin_Core,
	unk_45,
	unk_46,
	Argus_Jewel,
	unk_48,
	Argus_Talisman,
	unk_50,
	Caduceus_Reactor,
	Chitinous_Plating,
	Anabolic_Synthesis,
	Charon_Boosters,

	None = 61
};

namespace UnitCommandTypes {
	enum {
		Attack_Move = 0,
		Attack_Unit,
		Build,
		Build_Addon,
		Train,
		Morph,
		Research,
		Upgrade,
		Set_Rally_Position,
		Set_Rally_Unit,
		Move,
		Patrol,
		Hold_Position,
		Stop,
		Follow,
		Gather,
		Return_Cargo,
		Repair,
		Burrow,
		Unburrow,
		Cloak,
		Decloak,
		Siege,
		Unsiege,
		Lift,
		Land,
		Load,
		Unload,
		Unload_All,
		Unload_All_Position,
		Right_Click_Position,
		Right_Click_Unit,
		Halt_Construction,
		Cancel_Construction,
		Cancel_Addon,
		Cancel_Train,
		Cancel_Train_Slot,
		Cancel_Morph,
		Cancel_Research,
		Cancel_Upgrade,
		Use_Tech,
		Use_Tech_Position,
		Use_Tech_Unit,
		Place_COP,
		None,
		Unknown,
		MAX
	};
}

class UnitCommandType {
public:
	int id = UnitCommandTypes::None;
	UnitCommandType() = default;
	UnitCommandType(int id) : id(id) {}
	int getID() const {
		return id;
	}
};

class Position {
public:
	int x;
	int y;
	Position() : x(-1), y(-1) {}
	Position(int x, int y) : x(x), y(y) {}
	bool isValid() {
		return x != -1 || y != -1;
	}
};

namespace Positions {
	static Position Invalid;
}

class TilePosition {
public:
	int x;
	int y;
	TilePosition() : x(-1), y(-1) {}
	TilePosition(int x, int y) : x(x), y(y) {}
	bool isValid() {
		return x != -1 || y != -1;
	}
};

namespace TilePositions {
	static TilePosition Invalid;
}

class WalkPosition {
public:
	int x;
	int y;
	WalkPosition() : x(-1), y(-1) {}
	WalkPosition(int x, int y) : x(x), y(y) {}
	WalkPosition(TilePosition pos) : x(pos.x * 4), y(pos.y * 4) {}
	bool isValid() {
		return x != -1 || y != -1;
	}
};

class UnitCommand {
public:
	Unit unit = nullptr;
	UnitCommandType type = UnitCommandTypes::None;
	Unit target = nullptr;
	int x = 0;
	int y = 0;
	int extra = 0;
	UnitCommand() = default;
	UnitCommand(Unit unit, UnitCommandType type, Unit target, int x, int y, int extra) : unit(unit), type(type), target(target), x(x), y(y), extra(extra) {}

	Position getTargetPosition() const;
};

static const std::array<int, 228> groundWeaponHits = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 3, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 
  1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 2, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 2, 1, 1, 1, 0, 0, 0, 0, 1, 
  1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const std::array<int, 228> airWeaponHits = {
  1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
  1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 4, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 
  1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

class DamageType {
public:
	int id;
	int getID() const {
		return id;
	}
};

class WeaponType {
public:
	const bwgame::weapon_type_t* w = nullptr;
	const bwgame::state_functions* funcs;
	WeaponType(const bwgame::weapon_type_t* w, const bwgame::state_functions* funcs) : w(w), funcs(funcs) {}
	WeaponType(std::nullptr_t) {}
	explicit operator bool() {
		return w != nullptr;
	}
	bool operator==(const WeaponType n) const {
		return w == n.w;
	}
	bool operator!=(const WeaponType n) const {
		return w == n.w;
	}
	WeaponType* operator->() {
		return this;
	}
	
	int damageAmount();
	int damageBonus();
	UpgradeTypes upgradeType();
	int damageFactor();
	DamageType damageType();
};

class UnitSizeType {
public:
	int id;
	int getID() const {
		return id;
	}
};

class UnitType {
public:
	const bwgame::unit_type_t* ut = nullptr;
	const bwgame::state_functions* funcs = nullptr;
	UnitType(const bwgame::unit_type_t* ut, const bwgame::state_functions* funcs) : ut(ut), funcs(funcs) {}
	UnitType(std::nullptr_t) {}
	UnitType() = default;
	explicit operator bool() {
		return ut != nullptr;
	}
	bool operator==(const UnitType n) const {
		return ut == n.ut;
	}
	bool operator!=(const UnitType n) const {
		return ut != n.ut;
	}
	bool operator==(int n) const {
		return getID() == n;
	}
	bool operator!=(int n) const {
		return getID() != n;
	}
	UnitType* operator->() {
		return this;
	}
	
	int getID() const;
	bool isMineralField() const;
	int width() const;
	int height() const;
	WeaponType groundWeapon() const;
	WeaponType airWeapon() const;
	int maxGroundHits() const;
	int maxAirHits() const;
	int maxHitPoints() const;
	int maxShields() const;
	UnitSizeType size() const;
	bool isWorker() const;
	bool isRefinery() const;
	bool isResourceDepot() const;
};

enum class RaceEnum {
	Zerg,
	Terran,
	Protoss,
};

class Race {
public:
	RaceEnum race;
	const char* c_str() {
		if (race == RaceEnum::Zerg) return "Zerg";
		if (race == RaceEnum::Terran) return "Terran";
		if (race == RaceEnum::Protoss) return "Protoss";
		return "Unknown";
	}
};

using frame_id = std::tuple<int, int>; 

class PlayerInterface {
	size_t index = -1;
	openbwapi_functions* funcs = nullptr;
	std::vector<Unit> units;
	frame_id last_units_update;
public:
	PlayerInterface(size_t index, openbwapi_functions* funcs) : index(index), funcs(funcs) {}
	PlayerInterface() = default;
	
	const std::vector<Unit>& getUnits();
	bool isNeutral() const {
		return index == 11;
	}
	int getID() const {
		return (int)index;
	}
	int minerals() const;
	int gas() const;
	int supplyUsed() const;
	int supplyTotal() const;
	
	int getUpgradeLevel(UpgradeTypes upgrade_type) const;
	
	int weaponDamageCooldown(UnitType unit) const;
	int armor(UnitType unit) const;
	int weaponMaxRange(WeaponType weapon) const;
	bool isObserver() const {
		return false;
	}
	std::string getName() const;
	Race getRace() const;
};

using Player = PlayerInterface*;

class UnitCommand;

enum struct Orders : int {
	Die,
	Stop,
	Guard,
	PlayerGuard,
	TurretGuard,
	BunkerGuard,
	Move,
	ReaverStop,
	AttackDefault,
	MoveToAttack,
	AttackUnit,
	AttackFixedRange,
	AttackTile,
	Hover,
	AttackMove,
	InfestedCommandCenter,
	UnusedNothing,
	UnusedPowerup,
	TowerGuard,
	TowerAttack,
	SpiderMine,
	StayInRange,
	TurretAttack,
	Nothing,
	NothingWait,
	DroneStartBuild,
	DroneBuild,
	CastInfestation,
	MoveToInfest,
	InfestingCommandCenter,
	PlaceBuilding,
	PlaceProtossBuilding,
	CreateProtossBuilding,
	ConstructingBuilding,
	Repair,
	MoveToRepair,
	PlaceAddon,
	BuildAddon,
	Train,
	RallyPointUnit,
	RallyPointTile,
	ZergBirth,
	ZergUnitMorph,
	ZergBuildingMorph,
	IncompleteBuilding,
	IncompleteMorphing,
	BuildNydusExit,
	EnterNydusCanal,
	IncompleteWarping,
	Follow,
	Carrier,
	ReaverCarrierMove,
	CarrierStop,
	CarrierAttack,
	CarrierMoveToAttack,
	CarrierIgnore2,
	CarrierFight,
	CarrierHoldPosition,
	Reaver,
	ReaverAttack,
	ReaverMoveToAttack,
	ReaverFight,
	ReaverHoldPosition,
	TrainFighter,
	InterceptorAttack,
	ScarabAttack,
	RechargeShieldsUnit,
	RechargeShieldsBattery,
	ShieldBattery,
	InterceptorReturn,
	DroneLand,
	BuildingLand,
	BuildingLiftoff,
	DroneLiftOff,
	LiftingOff,
	ResearchTech,
	Upgrade,
	Larva,
	SpawningLarva,
	Harvest1,
	Harvest2,
	MoveToGas,
	WaitForGas,
	HarvestGas,
	ReturnGas,
	MoveToMinerals,
	WaitForMinerals,
	MiningMinerals,
	GatheringInterrupted,
	GatherWaitInterrupted,
	ReturnMinerals,
	RechargeShieldsUnitRemoveOverlay,
	EnterTransport,
	PickupIdle,
	PickupTransport,
	PickupBunker,
	Pickup4,
	PowerupIdle,
	Sieging,
	Unsieging,
	WatchTarget,
	InitCreepGrowth,
	SpreadCreep,
	StoppingCreepGrowth,
	GuardianAspect,
	ArchonWarp,
	CompletingArchonSummon,
	HoldPosition,
	QueenHoldPosition,
	Cloak,
	Decloak,
	Unload,
	MoveUnload,
	FireYamatoGun,
	MoveToFireYamatoGun,
	CastLockdown,
	Burrowing,
	Burrowed,
	Unburrowing,
	CastDarkSwarm,
	CastParasite,
	CastSpawnBroodlings,
	CastEMPShockwave,
	NukeWait,
	NukeTrain,
	NukeLaunch,
	NukePaint,
	NukeUnit,
	CastNuclearStrike,
	NukeTrack,
	InitializeArbiter,
	CloakNearbyUnits,
	PlaceMine,
	RightClickAction,
	SuicideUnit,
	SuicideLocation,
	SuicideHoldPosition,
	CastRecall,
	Teleport,
	CastScannerSweep,
	Scanner,
	CastDefensiveMatrix,
	CastPsionicStorm,
	CastIrradiate,
	CastPlague,
	CastConsume,
	CastEnsnare,
	CastStasisField,
	CastHallucination,
	Hallucination2,
	ResetCollision,
	ResetHarvestCollision,
	Patrol,
	CTFCOPInit,
	CTFCOPStarted,
	CTFCOP2,
	ComputerAI,
	AtkMoveEP,
	HarassMove,
	AIPatrol,
	GuardPost,
	RescuePassive,
	Neutral,
	ComputerReturn,
	InitializePsiProvider,
	SelfDestructing,
	Critter,
	HiddenGun,
	OpenDoor,
	CloseDoor,
	HideTrap,
	RevealTrap,
	EnableDoodad,
	DisableDoodad,
	WarpIn,
	MedicIdle,
	MedicHeal,
	HealMove,
	MedicHoldPosition,
	MedicHealToIdle,
	CastRestoration,
	CastDisruptionWeb,
	CastMindControl,
	DarkArchonMeld,
	CastFeedback,
	CastOpticalFlare,
	CastMaelstrom,
	JunkYardDog,
	Fatal,
	None
};

class Order {
public:
	Orders id;
	int getID() const {
		return (int)id;
	}
	bool operator==(Orders n) const {
		return id == n;
	}
	bool operator!=(Orders n) const {
		return id != n;
	}
};

class UnitInterface {
public:
	int id = -1;
	bwgame::unit_t* u = nullptr;
	openbwapi_functions* funcs = nullptr;
	int last_command_frame = 0;
	UnitCommand last_command;
	
	UnitInterface(int id, bwgame::unit_t* u, openbwapi_functions* funcs) : id(id), u(u), funcs(funcs) {}
	
	bool issueCommand(UnitCommand command);
	Unit getTarget() const;
	Unit getOrderTarget() const;
	bool isAttackFrame() const;
	Order getOrder() const;
	Order getSecondaryOrder() const;
	int getLastCommandFrame() const;
	UnitCommand getLastCommand() const;
	UnitType getType() const;
	bool exists() const {
		return u != nullptr;
	}
	bool isLockedDown() const;
	bool isMaelstrommed() const;
	bool isStasised() const;
	bool isLoaded() const;
	bool isPowered() const;
	bool isStuck() const;
	bool isCompleted() const;
	bool isMorphing() const;

	bool isConstructing() const;
	int getHitPoints() const;
	Position getPosition() const;
	Player getPlayer() const;
	bool isVisible(Player player) const;
	int getID() const;
	int getShields() const;
	int getEnergy() const;
	int getGroundWeaponCooldown() const;
	int getAirWeaponCooldown() const;
	bool isIdle() const;
	bool isDetected() const;
	bool isLifted() const;
	double getVelocityX() const;
	double getVelocityY() const;
	int getResources() const;
	Position getTargetPosition() const;
	bool isAccelerating() const;
	bool isAttacking() const;
	bool isBeingConstructed() const;
	bool isBeingGathered() const;
	bool isBeingHealed() const;
	bool isBlind() const;
	bool isBraking() const;
	bool isBurrowed() const;
	bool isCarryingGas() const;
	bool isCarryingMinerals() const;
	bool isCloaked() const;
	bool isDefenseMatrixed() const;
	bool isEnsnared() const;
	bool isFlying() const;
	bool isFollowing() const;
	bool isGatheringGas() const;
	bool isGatheringMinerals() const;
	bool isHallucination() const;
	bool isHoldingPosition() const;
	bool isInterruptible() const;
	bool isInvincible() const;
	bool isIrradiated() const;
	bool isMoving() const;
	bool isParasited() const;
	bool isPatrolling() const;
	bool isPlagued() const;
	bool isRepairing() const;
	bool isResearching() const;
	bool isSelected() const;
	bool isSieged() const;
	bool isStartingAttack() const;
	bool isStimmed() const;
	bool isTargetable() const;
	bool isTraining() const;
	bool isUnderAttack() const;
	bool isUnderDarkSwarm() const;
	bool isUnderDisruptionWeb() const;
	bool isUnderStorm() const;
	bool isUpgrading() const;
	bool canRightClickPosition(bool = true);
	void rightClick(Position) {}
};

class Bullet {
public:
	bwgame::bullet_t* b = nullptr;
	Bullet(bwgame::bullet_t* b) : b(b) {}
	Bullet(std::nullptr_t) {}
	Bullet() {}
	explicit operator bool() {
		return b != nullptr;
	}
	bool operator==(const Bullet n) const {
		return b == n.b;
	}
	bool operator!=(const Bullet n) const {
		return b != n.b;
	}
	const Bullet* operator->() const {
		return this;
	}
	
	Position getPosition() const;
	int getType() const;
};

enum class EventType {
  MatchStart,
  MatchEnd,
  MatchFrame,
  MenuFrame,
  SendText,
  ReceiveText,
  PlayerLeft,
  NukeDetect,
  UnitDiscover,
  UnitEvade,
  UnitShow,
  UnitHide,
  UnitCreate,
  UnitDestroy,
  UnitMorph,
  UnitRenegade,
  SaveGame,
  UnitComplete,
  None
};

class Event {
	EventType id = EventType::None;
	Unit unit = nullptr;
	bool is_winner = false;
public:
	Event() = default;
	Event(EventType id) : id(id) {}
	Event(EventType id, Unit unit) : id(id), unit(unit) {}
	Event(EventType id, bool is_winner) : id(id), is_winner(is_winner) {}
	const EventType getType() const {
		return id;
	}
	const Unit getUnit() const {
		return unit;
	}
	bool isWinner() const {
		return is_winner;
	}
};

enum class Flag {
	CompleteMapInformation,
	UserInput
};


class Error {
public:
	const char* c_str() {
		return "No error";
	}
	int getID() {
		return 0;
	}
};

class AIModule {
public:
	virtual ~AIModule() {}
	virtual void onStart() {}
	virtual void onEnd(bool isWinner) {}
	virtual void onFrame() {}
	virtual void onSendText(std::string text) {}
	virtual void onReceiveText(Player player, std::string text) {}
	virtual void onPlayerLeft(Player player) {}
	virtual void onNukeDetect(Position target) {}
	virtual void onUnitDiscover(Unit unit) {}
	virtual void onUnitEvade(Unit unit) {}
	virtual void onUnitShow(Unit unit) {}
	virtual void onUnitHide(Unit unit) {}
	virtual void onUnitCreate(Unit unit) {}
	virtual void onUnitDestroy(Unit unit) {}
	virtual void onUnitMorph(Unit unit) {}
	virtual void onUnitRenegade(Unit unit) {}
	virtual void onSaveGame(std::string gameName) {}
	virtual void onUnitComplete(Unit unit) {}
};

struct Game_impl;

class Game {
	std::unique_ptr<Game_impl> impl;
	
	void start();
public:
	Game();
	bool isInGame() const;
	void leaveGame();
	void restartGame();
	void enableFlag(Flag flag);
	void setLocalSpeed(int speed);
	void setGUI(bool enable);
	void setFrameSkip(int frameskip);
	void setCommandOptimizationLevel(int level);
	bool setMap(std::string filename);

	std::string mapFileName();
	std::vector<TilePosition> getStartLocations();
	int mapWidth();
	int mapHeight();
	Unit getUnit(int id);
	int getFrameCount();
	int getLatencyFrames() {
		return 0;
	}
	bool isPaused() {
		return false;
	}
	bool isReplay();
	Player self();
	Player enemy();
	Player neutral();
	Player getPlayer(int n) const;
	const std::vector<Player>& getPlayers();
	Position getScreenPosition() {
		return {};
	}
	bool isExplored(int x, int y);
	bool isVisible(int x, int y);
	const std::vector<Bullet>& getBullets();
	const std::vector<Unit>& getNeutralUnits();
	
	const std::vector<Event>& getEvents();
	bool isWalkable(int x, int y);
	bool isBuildable(int x, int y);
	int getGroundHeight(int x, int y);
	Error getLastError() {
		return {};
	}
	void drawLineMap(int x1, int y1, int x2, int y2, int color) {}
	void drawLineMap(Position from, Position to, int color) {}
	void drawCircleMap(int x, int y, int radius, int color, bool solid = false) {}
	void drawCircleMap(Position pos, int radius, int color, bool solid = false) {}
	void drawTextMap(int x, int y, const char* fmt, ...) {}
	void drawTextMap(Position pos, const char* fmt, ...) {}
	void drawTextScreen(int x, int y, const char* fmt, ...) {}
	void drawTextScreen(Position pos, const char* fmt, ...) {}
	int getFPS() {
		return 1;
	}
	double getAverageFPS() {
		return 1.0;
	}
	void setScreenPosition(Position) {}
	void vPrintf(const char* fmt, va_list args);
	void update();
	
	void saveSnapshot(std::string id);
	void loadSnapshot(const std::string& id);
	void deleteSnapshot(const std::string& id);
	std::vector<std::string> listSnapshots();
	
	void saveGlobalState(const std::string& filename);
	void loadGlobalState(const std::string& filename);
	void saveGameState(const std::string& filename);
	void loadGameState(const std::string& filename);
	void saveState(const std::string& filename);
	void loadState(const std::string& filename);
	
	Unit createUnit(Player player, int type, Position pos);
	void killUnit(Unit u);
	void removeunit(Unit u);
	
	void setRandomSeed(uint32_t value);
	void randomizeRandomSeed();
	
	void disableTriggers();
};

using Playerset = std::vector<Player>;

class Client {
	bool is_connected = false;
	Game& g;
public:
	Client(Game& g) : g(g) {}
	bool isConnected() const {
		return is_connected;
	}
	bool connect() {
		is_connected = true;
		return true;
	}
	void disconnect() {
		is_connected = false;
	}
	void update() {
		if (!is_connected) throw std::runtime_error("Client::update: not connected");
		g.update();
	}
};
extern Client BWAPIClient;

extern Game* Broodwar;
extern Game* BroodwarPtr;

}

#endif
