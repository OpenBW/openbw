#include "openbwapi.h"

#include "../bwgame.h"
#include "../actions.h"
#include "../replay.h"
#ifdef OPENBW_ENABLE_UI
#include "../ui/ui.h"
#endif

#include <mutex>
#include <unordered_map>
#include <cstdio>

using bwgame::error;

#ifdef OPENBW_ENABLE_UI
namespace bwgame {
namespace ui {
void log_str(a_string str) {
	printf("%s", str.c_str());
	fflush(stdout);
}

void fatal_error_str(a_string str){
	bwgame::error("%s", str);
}
}
}
#endif

namespace OpenBWAPI {

#ifdef OPENBW_ENABLE_UI
struct ui_wrapper {
	bwgame::ui_functions ui;
	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point last_update;
	bwgame::game_player get_player(bwgame::state& st) {
		bwgame::game_player player;
		player.set_st(st);
		return player;
	}
	ui_wrapper(bwgame::state& st) : ui(get_player(st)) {
		auto load_data_file = bwgame::data_loading::data_files_directory(".");
		ui.load_data_file = [&](bwgame::a_vector<uint8_t>& data, bwgame::a_string filename) {
			load_data_file(data, std::move(filename));
		};
		ui.init();

		size_t screen_width = 800;
		size_t screen_height = 600;

		ui.resize(screen_width, screen_height);
		ui.screen_pos = {(int)ui.game_st.map_width / 2 - (int)screen_width / 2, (int)ui.game_st.map_height / 2 - (int)screen_height / 2};
	}
	void update() {
		auto now = clock.now();
		if (now - last_update < std::chrono::milliseconds(40)) return;
		last_update = now;
		ui.update();
	}
};
#else
struct ui_wrapper {
  ui_wrapper(bwgame::state& st) {}
  void update() {
  }
};
#endif

struct openbwapi_functions: bwgame::replay_functions {
	std::vector<bwgame::unit_id_32> destroyed_units;
	int local_player_id = -1;
	int enemy_player_id = -1;
	bool is_replay = false;
	std::unique_ptr<ui_wrapper> ui;

	std::unordered_map<int, Unit> units_lookup;
	std::list<UnitInterface> units_container;
	std::array<PlayerInterface, 12> players_container;

	std::vector<Player> players;
	std::vector<Bullet> bullets;
	std::array<std::vector<Unit>, 12> player_units;

	openbwapi_functions(bwgame::state& st, bwgame::action_state& action_st, bwgame::replay_state& replay_st) : bwgame::replay_functions(st, action_st, replay_st) {
		for (size_t i = 0; i != 12; ++i) {
			players_container[i] = {i, this};
		}

		enable_ui();
	}

	virtual void on_unit_destroy(bwgame::unit_t* u) override {
		auto id = get_unit_id_32(u);
		destroyed_units.push_back(id);
		auto i = units_lookup.find((int)id.raw_value);
		if (i != units_lookup.end()) i->second->u = nullptr;
	}

	void enable_ui() {
		ui = std::make_unique<ui_wrapper>(st);
	}
	void disable_ui() {
		ui = nullptr;
	}

	void next_frame() {
		if (is_replay) bwgame::replay_functions::next_frame();
		else bwgame::state_functions::next_frame();

		if (ui) {
			ui->update();
		}
	}

	Unit get_unit(bwgame::unit_t* u) {
		if (!u) return nullptr;
		static_assert(sizeof(int) >= sizeof(get_unit_id_32(u).raw_value), "int must be at least 32 bits");
		return get_unit((int)get_unit_id_32(u).raw_value, u);
	}
	Unit get_unit(int id) {
		return get_unit(id, state_functions::get_unit(bwgame::unit_id_32(id)));
	}
	Unit get_unit(int id, bwgame::unit_t* u) {
		Unit& r = units_lookup.insert({id, nullptr}).first->second;
		if (!r) {
			units_container.emplace_back(id, u, this);
			r = &units_container.back();
		}
		return r;
	}
	void reset_bwapi() {
		units_lookup.clear();
		units_container.clear();

	}
	Player get_player(int id) {
		return &players_container.at((size_t)id);
	}
};



bwgame::global_state g_global_st;
static bool global_inited = false;
static std::mutex global_init_mut;
void g_global_init() {
	if (global_inited) return;
	std::unique_lock<std::mutex> l;
	if (global_inited) return;
	bwgame::global_init(g_global_st, bwgame::data_loading::data_files_directory("."));
	global_inited = true;
}

static Game g;

Game* Broodwar = &g;
Game* BroodwarPtr = Broodwar;
Client BWAPIClient(g);

static std::string read_ini(const std::string& filename, const std::string& section, const std::string& key) {
	FILE* f = fopen(filename.c_str(), "rb");
	if (!f) return{};
	std::vector<char> data;
	fseek(f, 0, SEEK_END);
	long filesize = ftell(f);
	data.resize(filesize);
	fseek(f, 0, SEEK_SET);
	data.resize(filesize * fread(data.data(), filesize, 1, f));
	fclose(f);
	bool correct_section = section.empty();
	const char* c = data.data();
	const char* e = c + data.size();
	auto whitespace = [&]() {
		switch (*c) {
		case ' ': case '\t': case '\v': case '\f': case '\r':
			return true;
		default:
			return false;
		}
	};
	while (c != e) {
		while (c != e && (whitespace() || *c == '\n')) ++c;
		if (c == e) break;
		if (*c == '#' || *c == ';') {
			while (c != e && *c != '\n') ++c;
		} else if (*c == '[') {
			correct_section = false;
			++c;
			const char* n = c;
			while (c != e && *c != ']' && *c != '\n') ++c;
			if (size_t(c - n) == section.size() && !memcmp(n, section.c_str(), section.size())) correct_section = true;
			if (c != e) ++c;
		} else {
			const char* n = c;
			while (c != e && !whitespace() && *c != '=' && *c != '\n') {
				++c;
			}
			if (c != e) {
				if (correct_section && size_t(c - n) == key.size() && !memcmp(n, key.c_str(), key.size())) {
					while (c != e && whitespace()) ++c;
					if (c != e && *c == '=') {
						++c;
						while (c != e && whitespace()) ++c;
						n = c;
						while (c != e && *c != '\r' && *c != '\n') ++c;
						return std::string(n, c - n);
					}
				} else {
					while (c != e && *c != '\n') ++c;
				}
			}
		}
	}
	return{};
}

int WeaponType::damageAmount() {
	if (!w) return 0;
	return w->damage_amount;
}

int WeaponType::damageBonus() {
	if (!w) return 0;
	return w->damage_bonus;
}

UpgradeTypes WeaponType::upgradeType() {
	if (!w) return UpgradeTypes::None;
	return (UpgradeTypes)(int)w->damage_upgrade->id;
}

int WeaponType::damageFactor() {
	if (!w) return 0;
	return w->bullet_count;
}

DamageType WeaponType::damageType() {
	if (!w) return {0};
	return {w->damage_type};
}

int UnitType::getID() const {
	return (int)ut->id;
}

bool UnitType::isMineralField() const {
	return funcs->unit_is_mineral_field(ut);
}

int UnitType::width() const {
	return ut->dimensions.from.x + 1 + ut->dimensions.to.x;
}

int UnitType::height() const {
	return ut->dimensions.from.y + 1 + ut->dimensions.to.y;
}

WeaponType UnitType::groundWeapon() const {
	return {ut->ground_weapon, funcs};
}

WeaponType UnitType::airWeapon() const {
	return {ut->ground_weapon, funcs};
}

int UnitType::maxGroundHits() const {
	return groundWeaponHits.at((size_t)ut->id);
}

int UnitType::maxAirHits() const {
	return airWeaponHits.at((size_t)ut->id);
}

int UnitType::maxHitPoints() const {
	return std::max((int)ut->hitpoints.ceil().integer_part(), 1);
}

int UnitType::maxShields() const {
	return ut->shield_points;
}

UnitSizeType UnitType::size() const {
	return {ut->unit_size};
}

bool UnitType::isWorker() const {
	return funcs->ut_worker(ut);
}

bool UnitType::isRefinery() const {
	return funcs->unit_is_refinery(ut);
}

bool UnitType::isResourceDepot() const {
	return funcs->ut_resource_depot(ut);
}

const std::vector<Unit>& PlayerInterface::getUnits() {
	units.clear();
	for (auto* u : ptr(funcs->st.player_units[index])) {
		if (!funcs->unit_is_map_revealer(u) && !funcs->ut_turret(u)) {
			units.push_back(funcs->get_unit(u));
		}
	}
	return units;
//	auto gcc_bug_workaround_ptr = [&](auto&& r) {
//		return bwgame::make_transform_range(r, [](auto& ref) {
//			return &ref;
//		});
//	};
//	return bwgame::make_transform_range(bwgame::make_filter_range(gcc_bug_workaround_ptr(funcs->st.player_units[index]), [&](bwgame::unit_t* u) {
//	      return !funcs->unit_is_map_revealer(u) && !funcs->ut_turret(u);
//	}), [this, tmp = Unit()](bwgame::unit_t* u) mutable -> Unit& {
//		return tmp = funcs->get_unit(u);
//	});
}

int PlayerInterface::minerals() const {
	return funcs->st.current_minerals.at(index);
}

int PlayerInterface::gas() const {
	return funcs->st.current_minerals.at(index);
}

int PlayerInterface::supplyUsed() const {
	return funcs->st.supply_used.at(index).at((size_t)funcs->st.players.at(index).race).raw_value;
}

int PlayerInterface::supplyTotal() const {
	return std::min((int)funcs->st.supply_available.at(index).at((size_t)funcs->st.players.at(index).race).raw_value, 400);
}

int PlayerInterface::getUpgradeLevel(UpgradeTypes upgrade_type) const {
	if (upgrade_type == UpgradeTypes::None) return 0;
	return funcs->st.upgrade_levels.at(index).at((bwgame::UpgradeTypes)(int)upgrade_type);
}

int PlayerInterface::weaponDamageCooldown(UnitType unit) const {
	if (!unit) return 0;
	auto* ut = unit->ut;
	if (!ut->ground_weapon) return 0;
	return ut->ground_weapon->cooldown;
}

int PlayerInterface::armor(UnitType unit) const {
	if (!unit) return 0;
	return unit->ut->armor;
}

int PlayerInterface::weaponMaxRange(WeaponType weapon) const {
	if (!weapon) return 0;
	return weapon->w->max_range;
}

std::string PlayerInterface::getName() const {
	return bwgame::format("player %d", index);
}

Race PlayerInterface::getRace() const {
	return {(RaceEnum)(int)funcs->st.players.at(index).race};
}

bool UnitInterface::issueCommand(UnitCommand command) {
	if (!exists()) return false;
	last_command_frame = funcs->st.current_frame;
	last_command = command;
	bwgame::unit_t* u = command.unit ? command.unit->u : nullptr;
	bwgame::unit_t* target = command.target ? command.target->u : nullptr;
	if (u != this->u) error("issueCommand: u != this->u");
	if (u != this->u) return false;
	int x = command.x;
	int y = command.y;
	int type = command.type.getID();
	funcs->action_select(u->owner, u);
	if (type == UnitCommandTypes::Attack_Move) {
		if (u && u->unit_type->id == bwgame::UnitTypes::Zerg_Infested_Terran) {
			return funcs->action_order(u->owner, funcs->get_order_type(bwgame::Orders::AttackDefault), {x, y}, target, nullptr, false);
		} else {
			return funcs->action_order(u->owner, funcs->get_order_type(bwgame::Orders::AttackMove), {x, y}, target, nullptr, false);
		}
	} else if (type == UnitCommandTypes::Attack_Unit) {
		return funcs->action_order(u->owner, funcs->get_order_type(bwgame::Orders::AttackUnit), {x, y}, target, nullptr, false);
	} else if (type == UnitCommandTypes::Move) {
		return funcs->action_default_order(u->owner, {x, y}, nullptr, nullptr, false);
	} else if (type == UnitCommandTypes::Build) {
		auto* ut = funcs->get_unit_type((bwgame::UnitTypes)command.extra);
		bwgame::Orders o{};
		if (funcs->unit_is_nydus(u) && funcs->unit_is_nydus(ut)) {
			o = bwgame::Orders::BuildNydusExit;
		} else if (funcs->ut_addon(ut)) {
			o = bwgame::Orders::PlaceAddon;
		} else {
			auto r = funcs->unit_race(ut);
			if (r == bwgame::race_t::zerg) o = bwgame::Orders::DroneStartBuild;
			else if (r == bwgame::race_t::terran) o = bwgame::Orders::PlaceBuilding;
			else if (r == bwgame::race_t::protoss) o = bwgame::Orders::PlaceProtossBuilding;
		}
		return funcs->action_build(u->owner, funcs->get_order_type(o), ut, {(unsigned)x, (unsigned)y});
	} else if (type == UnitCommandTypes::Train) {
		auto* ut = funcs->get_unit_type((bwgame::UnitTypes)command.extra);
		switch (u->unit_type->id) {
		case bwgame::UnitTypes::Zerg_Larva:
		case bwgame::UnitTypes::Zerg_Mutalisk:
		case bwgame::UnitTypes::Zerg_Hydralisk:
			return funcs->action_morph(u->owner, ut);
		case bwgame::UnitTypes::Zerg_Hatchery:
		case bwgame::UnitTypes::Zerg_Lair:
		case bwgame::UnitTypes::Zerg_Spire:
		case bwgame::UnitTypes::Zerg_Creep_Colony:
			return funcs->action_morph_building(u->owner, ut);
		case bwgame::UnitTypes::Protoss_Carrier:
		case bwgame::UnitTypes::Hero_Gantrithor:
		case bwgame::UnitTypes::Protoss_Reaver:
		case bwgame::UnitTypes::Hero_Warbringer:
			return funcs->action_train_fighter(u->owner);
		default:
			return funcs->action_train(u->owner, ut);
		}
	} else if (type == UnitCommandTypes::Right_Click_Unit) {
		return funcs->action_default_order(u->owner, {x, y}, target, nullptr, false);
	}
	error("issueCommand: unknown command type %d\n", (int)type);
	return false;
}

Unit UnitInterface::getTarget() const {
	if (!exists()) return nullptr;
	return funcs->get_unit(u->move_target.unit);
}

Unit UnitInterface::getOrderTarget() const {
	if (!exists()) return nullptr;
	return funcs->get_unit(u->order_target.unit);
}

bool UnitInterface::isAttackFrame() const {
	return false;
}

Order UnitInterface::getOrder() const {
	if (!exists()) return {};
	return {(Orders)u->order_type->id};
}

Order UnitInterface::getSecondaryOrder() const {
	if (!exists()) return {};
	return {(Orders)u->secondary_order_type->id};
}

int UnitInterface::getLastCommandFrame() const {
	return last_command_frame;
}

UnitCommand UnitInterface::getLastCommand() const {
	return last_command;
}

UnitType UnitInterface::getType() const {
	if (!exists()) return {};
	return {u->unit_type, funcs};
}

bool UnitInterface::isLockedDown() const {
	if (!exists()) return {};
	return u->lockdown_timer != 0;
}

bool UnitInterface::isMaelstrommed() const {
	if (!exists()) return {};
	return u->maelstrom_timer != 0;
}

bool UnitInterface::isStasised() const {
	if (!exists()) return {};
	return u->stasis_timer != 0;
}

bool UnitInterface::isLoaded() const {
	if (!exists()) return {};
	return funcs->u_loaded(u);
}

bool UnitInterface::isPowered() const {
	if (!exists()) return {};
	return !(funcs->unit_race(u) == bwgame::race_t::protoss && funcs->u_grounded_building(u) && funcs->u_disabled(u));
}

bool UnitInterface::isStuck() const {
	if (!exists()) return {};
	return u->movement_state == bwgame::movement_states::UM_MoveToLegal;
}

bool UnitInterface::isCompleted() const {
	if (!exists()) return {};
	return funcs->u_completed(u);
}

bool UnitInterface::isMorphing() const {
	if (!exists()) return {};
	auto o = getOrder();
	return o == Orders::ZergBirth ||
	            o == Orders::ZergBuildingMorph ||
	            o == Orders::ZergUnitMorph ||
	            o == Orders::IncompleteMorphing;
}

bool UnitInterface::isConstructing() const {
	if (!exists()) return {};
	if (isMorphing()) return true;
	auto o = getOrder();
	return o == Orders::ConstructingBuilding ||
	            o == Orders::PlaceBuilding ||
	            o == Orders::DroneBuild ||
	            o == Orders::DroneStartBuild ||
	            o == Orders::DroneLand ||
	            o == Orders::PlaceProtossBuilding ||
	            o == Orders::CreateProtossBuilding ||
	            o == Orders::IncompleteBuilding ||
	            o == Orders::IncompleteWarping ||
	            o == Orders::IncompleteMorphing ||
	            o == Orders::BuildNydusExit ||
	            o == Orders::BuildAddon ||
	            u->secondary_order_type->id == bwgame::Orders::BuildAddon ||
	            (!funcs->u_completed(u) && u->current_build_unit != nullptr);
}

int UnitInterface::getHitPoints() const {
	if (!exists()) return {};
	return u->hp.ceil().integer_part();
}

Position UnitInterface::getPosition() const {
	if (!exists()) return {};
	return {u->sprite->position.x, u->sprite->position.y};
}

Player UnitInterface::getPlayer() const {
	if (!exists()) return {};
	return funcs->get_player(u->owner);
}

bool UnitInterface::isVisible(Player player) const {
	if (!exists()) return {};
	return (u->sprite->visibility_flags & (1 << player->getID())) != 0;
}

int UnitInterface::getID() const {
	return id;
}

int UnitInterface::getShields() const {
	if (!exists()) return {};
	return u->shield_points.ceil().integer_part();
}

int UnitInterface::getEnergy() const {
	if (!exists()) return {};
	return u->energy.ceil().integer_part();
}

int UnitInterface::getGroundWeaponCooldown() const {
	if (!exists()) return {};
	return u->ground_weapon_cooldown;
}

int UnitInterface::getAirWeaponCooldown() const {
	if (!exists()) return {};
	return u->air_weapon_cooldown;
}

bool UnitInterface::isIdle() const {
	if (!exists()) return {};
	auto o = getOrder();
	return o == Orders::PlayerGuard ||
	       o == Orders::Guard ||
	       o == Orders::Stop ||
	       o == Orders::PickupIdle ||
	       o == Orders::Nothing ||
	       o == Orders::MedicIdle ||
	       o == Orders::Carrier ||
	       o == Orders::Reaver ||
	       o == Orders::Critter ||
	       o == Orders::Neutral ||
	       o == Orders::TowerGuard ||
	       o == Orders::Burrowed ||
	       o == Orders::NukeTrain ||
	       o == Orders::Larva;
}

bool UnitInterface::isDetected() const {
	if (!exists()) return {};
	return !funcs->unit_is_undetected(u, funcs->local_player_id);
}

bool UnitInterface::isLifted() const {
	if (!exists()) return {};
	return funcs->u_flying(u) && funcs->ut_building(u);
}

double UnitInterface::getVelocityX() const {
	if (!exists()) return {};
	return u->velocity.x.raw_value / 256.0;
}

double UnitInterface::getVelocityY() const {
	if (!exists()) return {};
	return u->velocity.y.raw_value / 256.0;
}

int UnitInterface::getResources() const {
	if (!exists()) return {};
	return funcs->ut_resource(u) ? u->building.resource.resource_count : 0;
}

Position UnitInterface::getTargetPosition() const {
	if (!exists()) return {};
	return {u->move_target.pos.x, u->move_target.pos.y};
}

bool UnitInterface::isAccelerating() const {
	if (!exists()) return {};
	return funcs->u_movement_flag(u, 2);
}

bool UnitInterface::isAttacking() const {
	if (!exists()) return {};
	if (!u->order_target.unit) return false;
	const bwgame::unit_t* attacking_unit = funcs->unit_attacking_unit(u);
	switch (attacking_unit->sprite->main_image->iscript_state.animation) {
	case bwgame::iscript_anims::GndAttkInit:
	case bwgame::iscript_anims::GndAttkRpt:
	case bwgame::iscript_anims::AirAttkInit:
	case bwgame::iscript_anims::AirAttkRpt:
		return true;
	default:
		return false;
	}
}

bool UnitInterface::isBeingConstructed() const {
	if (!exists()) return {};
	if (isMorphing()) return true;
	if (!isCompleted()) {
		if (funcs->unit_race(u) != bwgame::race_t::terran || u->current_build_unit) return true;
	}
	return false;
}

bool UnitInterface::isBeingGathered() const {
	if (!exists()) return {};
	return funcs->ut_resource(u) && (u->building.resource.is_being_gathered || !u->building.resource.gather_queue.empty());
}

bool UnitInterface::isBeingHealed() const {
	if (!exists()) return {};
	return u->is_being_healed;
}

bool UnitInterface::isBlind() const {
	if (!exists()) return {};
	return u->blinded_by != 0;
}

bool UnitInterface::isBraking() const {
	if (!exists()) return {};
	return funcs->u_movement_flag(u, 4);
}

bool UnitInterface::isBurrowed() const {
	if (!exists()) return {};
	return funcs->u_burrowed(u);
}

bool UnitInterface::isCarryingGas() const {
	if (!exists()) return {};
	return (u->carrying_flags & 1) != 0;
}

bool UnitInterface::isCarryingMinerals() const {
	if (!exists()) return {};
	return (u->carrying_flags & 2) != 0;
}

bool UnitInterface::isCloaked() const {
	if (!exists()) return {};
	return funcs->u_cloaked(u) && !funcs->u_burrowed(u);
}

bool UnitInterface::isDefenseMatrixed() const {
	if (!exists()) return {};
	return u->defensive_matrix_timer != 0;
}

bool UnitInterface::isEnsnared() const {
	if (!exists()) return {};
	return u->ensnare_timer != 0;
}

bool UnitInterface::isFlying() const {
	if (!exists()) return {};
	return funcs->u_flying(u);
}

bool UnitInterface::isFollowing() const {
	if (!exists()) return {};
	return u->order_type->id == bwgame::Orders::Follow;
}

bool UnitInterface::isGatheringGas() const {
	if (!exists()) return {};
	bool isGathering = getType().isWorker() && funcs->u_gathering(u);
	if (!isGathering)
		return false;

	auto order = getOrder();
	if (order != Orders::Harvest1   &&
	            order != Orders::Harvest2   &&
	            order != Orders::MoveToGas  &&
	            order != Orders::WaitForGas &&
	            order != Orders::HarvestGas &&
	            order != Orders::ReturnGas  &&
	            order != Orders::ResetCollision)
		return false;

	if (order == Orders::ResetCollision)
		return isCarryingGas();

	//return true if BWOrder is WaitForGas, HarvestGas, or ReturnGas
	if (order == Orders::WaitForGas ||
	            order == Orders::HarvestGas ||
	            order == Orders::ReturnGas)
		return true;

	//if BWOrder is MoveToGas, Harvest1, or Harvest2 we need to do some additional checks to make sure the unit is really gathering
	Unit targ = getTarget();
	if ( targ &&
	            targ->exists() &&
	            targ->isCompleted() &&
	            targ->getPlayer() == getPlayer() &&
	            targ->getType() != UnitTypes::Resource_Vespene_Geyser &&
	            (targ->getType().isRefinery() || targ->getType().isResourceDepot()) )
		return true;
	targ = getOrderTarget();
	if ( targ &&
	            targ->exists() &&
	            targ->isCompleted() &&
	            targ->getPlayer() == getPlayer() &&
	            targ->getType() != UnitTypes::Resource_Vespene_Geyser &&
	            (targ->getType().isRefinery() || targ->getType().isResourceDepot()))
		return true;
	return false;
}

bool UnitInterface::isGatheringMinerals() const {
	if (!exists()) return {};
	bool isGathering = getType().isWorker() && funcs->u_gathering(u);
	if (!isGathering)
		return false;

	auto order = getOrder();
	if (order != Orders::Harvest1        &&
	            order != Orders::Harvest2        &&
	            order != Orders::MoveToMinerals  &&
	            order != Orders::WaitForMinerals &&
	            order != Orders::MiningMinerals  &&
	            order != Orders::ReturnMinerals  &&
	            order != Orders::ResetCollision)
		return false;

	if (order == Orders::ResetCollision)
		return isCarryingMinerals();

	//return true if BWOrder is WaitForMinerals, MiningMinerals, or ReturnMinerals
	if (order == Orders::WaitForMinerals ||
	            order == Orders::MiningMinerals ||
	            order == Orders::ReturnMinerals)
		return true;

	//if BWOrder is MoveToMinerals, Harvest1, or Harvest2 we need to do some additional checks to make sure the unit is really gathering
	if (getTarget() &&
	            getTarget()->exists() &&
	            (getTarget()->getType().isMineralField() ||
	             (getTarget()->isCompleted() &&
	              getTarget()->getPlayer() == getPlayer() &&
	              getTarget()->getType().isResourceDepot())))
		return true;
	if (getOrderTarget() &&
	            getOrderTarget()->exists() &&
	            (getOrderTarget()->getType().isMineralField() ||
	             (getOrderTarget()->isCompleted() &&
	              getOrderTarget()->getPlayer() == getPlayer() &&
	              getOrderTarget()->getType().isResourceDepot())))
		return true;
	return false;
}

bool UnitInterface::isHallucination() const {
	if (!exists()) return {};
	return funcs->u_hallucination(u);
}

bool UnitInterface::isHoldingPosition() const {
	if (!exists()) return {};
	return getOrder() == Orders::HoldPosition;
}

bool UnitInterface::isInterruptible() const {
	if (!exists()) return {};
	return !funcs->u_order_not_interruptible(u);
}

bool UnitInterface::isInvincible() const {
	if (!exists()) return {};
	return funcs->u_invincible(u);
}

bool UnitInterface::isIrradiated() const {
	if (!exists()) return {};
	return u->irradiate_timer != 0;
}

bool UnitInterface::isMoving() const {
	if (!exists()) return {};
	return getOrder() == Orders::Move || funcs->u_movement_flag(u, 0x10) || funcs->u_movement_flag(u, 2);
}

bool UnitInterface::isParasited() const {
	if (!exists()) return {};
	return u->parasite_flags != 0;
}

bool UnitInterface::isPatrolling() const {
	if (!exists()) return {};
	return getOrder() == Orders::Patrol;
}

bool UnitInterface::isPlagued() const {
	if (!exists()) return {};
	return u->plague_timer != 0;
}

bool UnitInterface::isRepairing() const {
	if (!exists()) return {};
	return getOrder() == Orders::Repair;
}

bool UnitInterface::isResearching() const {
	if (!exists()) return {};
	return getOrder() == Orders::ResearchTech;
}

bool UnitInterface::isSelected() const {
	if (!exists()) return {};
	return false;
}

bool UnitInterface::isSieged() const {
	if (!exists()) return {};
	return funcs->unit_is_sieged_tank(u);
}

bool UnitInterface::isStartingAttack() const {
	if (!exists()) return {};
	return false;
}

bool UnitInterface::isStimmed() const {
	if (!exists()) return {};
	return u->stim_timer != 0;
}

bool UnitInterface::isTargetable() const {
	if (!exists()) return {};
	return true;
}

bool UnitInterface::isTraining() const {
	if (!exists()) return {};
	return !u->build_queue.empty();
}

bool UnitInterface::isUnderAttack() const {
	if (!exists()) return {};
	return false;
}

bool UnitInterface::isUnderDarkSwarm() const {
	if (!exists()) return {};
	return false;
}

bool UnitInterface::isUnderDisruptionWeb() const {
	if (!exists()) return {};
	return false;
}

bool UnitInterface::isUnderStorm() const {
	if (!exists()) return {};
	return false;
}

bool UnitInterface::isUpgrading() const {
	if (!exists()) return {};
	return getOrder() == Orders::Upgrade;
}

bool UnitInterface::canRightClickPosition(bool) {
	if (!exists()) return {};
	return false;
}

Position Bullet::getPosition() const {
	return {b->sprite->position.x, b->sprite->position.y};
}

int Bullet::getType() const {
	return (int)b->flingy_type->id;
}


struct full_state {
	bwgame::global_state& global_st = g_global_st;
	bwgame::game_state game_st;
	bwgame::state st;
	bwgame::action_state action_st;
	bwgame::replay_state replay_st;
	full_state() {
		st.global = &global_st;
		st.game = &game_st;
	}
	void global_init() {
		if (&global_st == &g_global_st) {
			g_global_init();
		}
	}
};

struct Game_impl {
	full_state fst;
	bwgame::state& st;
	const bwgame::game_state& game_st;
	openbwapi_functions funcs;
	std::string map_filename;
	std::string map_full_filename;
	std::vector<Event> events;
	bool left_game = false;
	bwgame::state initial_state;
	bool global_inited = false;
	std::unique_ptr<scenario> current_scenario = nullptr;
	bool is_in_game = false;
	bool has_started = false;
	std::string set_map_filename;
	bool game_type_melee = false;
	int local_player_race = 0;
	int enemy_player_race = 0;

	std::default_random_engine rng_engine{[]{
		std::array<unsigned int, 4> arr;
		arr[0] = 42;
		arr[1] = (unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count();
		arr[2] = (unsigned int)std::hash<std::thread::id>()(std::this_thread::get_id());
		arr[3] = 0;
		std::seed_seq seq(arr.begin(), arr.end());
		std::default_random_engine e(seq);
		return e;
	}()};

	template<typename T>
	auto rng(T v) {
		std::uniform_int_distribution<T> dis(0, v - 1);
		return dis(rng_engine);
	}

	Game_impl() : st(fst.st), game_st(fst.game_st), funcs(fst.st, fst.action_st, fst.replay_st) {}

	void load_map() {
		auto& filename = set_map_filename;

		fst.game_st = bwgame::game_state();
		st = bwgame::state();
		st.global = &fst.global_st;
		st.game = &fst.game_st;

		fst.global_init();

		std::string ext;
		size_t dot_pos = filename.rfind('.');
		if (dot_pos != std::string::npos) {
			ext = filename.substr(dot_pos + 1);
			for (auto& v : ext) v |= 0x20;
		}

		if (ext == "rep") {

			funcs.load_replay_file(filename);

			funcs.is_replay = true;
			funcs.local_player_id = -1;
			funcs.enemy_player_id = -1;
			current_scenario = nullptr;

		} else {
			bwgame::game_load_functions load_funcs(st);



			load_funcs.load_map_file(filename, [&]() {
				bwgame::static_vector<size_t, 12> available_slots;
				for (auto& v : st.players) {
					if (v.controller == bwgame::player_t::controller_open || v.controller == bwgame::player_t::controller_computer) {
						size_t index = (size_t)(&v - st.players.data());
						if (index < 8) available_slots.push_back(index);
						v.controller = bwgame::player_t::controller_closed;
					}
				}
				if (available_slots.size() < 2) error("%s: not enough available player slots (need 2)", filename);
				if (game_type_melee) {
					funcs.local_player_id = available_slots.at(rng(available_slots.size()));
					available_slots.erase(available_slots.begin() + funcs.local_player_id);
					funcs.enemy_player_id = available_slots.at(rng(available_slots.size()));
				} else {
					funcs.local_player_id = 0;
					funcs.enemy_player_id = 1;
				}

				auto& local_player = st.players.at(funcs.local_player_id);
				auto& enemy_player = st.players.at(funcs.enemy_player_id);
				local_player.controller = bwgame::player_t::controller_occupied;
				enemy_player.controller = bwgame::player_t::controller_occupied;

				if ((int)local_player.race == 5) local_player.race = (bwgame::race_t)local_player_race;
				if ((int)enemy_player.race == 5) enemy_player.race = (bwgame::race_t)enemy_player_race;
				if (game_type_melee) {
					load_funcs.setup_info.victory_condition = 1;
					load_funcs.setup_info.starting_units = 1;
				}

				for (auto& v : st.players) {
					if (v.controller == bwgame::player_t::controller_occupied) {
						if ((int)v.race > 2) v.race = (bwgame::race_t)rng(3);
					}
				}

				st.lcg_rand_state = rng((decltype(st.lcg_rand_state))0);
			});

			funcs.is_replay = false;

			if (!game_type_melee) {
				//current_scenario = get_new_scenario(funcs);
			}

			if (st.players.at(funcs.local_player_id).controller != bwgame::player_t::controller_occupied) error("%s: slot %d is not occupied (not a 2 player map?)", filename, funcs.local_player_id);
			if (st.players.at(funcs.enemy_player_id).controller != bwgame::player_t::controller_occupied) error("%s: slot %d is not occupied (not a 2 player map?)", filename, funcs.enemy_player_id);
		}

		size_t slash_pos = filename.rfind('/');
		if (slash_pos == std::string::npos) map_filename = filename;
		else map_filename = filename.substr(slash_pos + 1);
		map_full_filename = filename;
	}

	void set_game_type_melee() {
		game_type_melee = true;
	}
	void set_game_type_ums() {
		game_type_melee = false;
	}

	void set_local_player_race(int race) {
		local_player_race = race;
	}
	void set_enemy_player_race(int race) {
		enemy_player_race = race;
	}

	void update() {
		events.clear();
		if (!is_in_game) {
			if (!has_started) {
				has_started = true;
				start();
			}

			load_map();
			is_in_game = true;
			events.push_back({EventType::MatchStart});
			return;
		}

		if (left_game) {
			left_game = false;
			events.push_back({EventType::MatchEnd});

			is_in_game = false;
			return;
		}

		funcs.next_frame();
		if (current_scenario) current_scenario->update();

		for (auto& v : funcs.destroyed_units) {
			auto unit = funcs.get_unit(v.raw_value);
			if (!unit) continue;
			events.push_back({EventType::UnitDestroy, unit});
		}

		funcs.destroyed_units.clear();
	}

	void start() {
		std::string bwapi_ini_fn = "./bwapi-data/bwapi.ini";

		auto map_fn = read_ini(bwapi_ini_fn.c_str(), "auto_menu", "map");
		if (map_fn.empty()) error("no map specified in %s", bwapi_ini_fn);

		auto game_type = read_ini(bwapi_ini_fn.c_str(), "auto_menu", "game_type");
		if (game_type == "MELEE") set_game_type_melee();
		else if (game_type == "USE_MAP_SETTINGS") set_game_type_ums();
		else error("%s: game type '%s' unrecognized. Supported values are MELEE and USE_MAP_SETTINGS", bwapi_ini_fn);

		auto get_race = [&](std::string str) {
			for (auto& v : str) v |= 0x20;
			if (str == "zerg") return 0;
			if (str == "terran") return 1;
			if (str == "protoss") return 2;
			return 5;
		};

		set_local_player_race(get_race(read_ini(bwapi_ini_fn.c_str(), "auto_menu", "race")));
		set_enemy_player_race(get_race(read_ini(bwapi_ini_fn.c_str(), "auto_menu", "enemy_race")));

		set_map_filename = map_fn;
	}
};

void Game::start() {
	impl->start();
}

Game::Game() : impl(std::make_unique<Game_impl>()) {}

bool Game::isInGame() const {
	return impl->is_in_game;
}

void Game::leaveGame() {
	impl->left_game = true;
}

void Game::restartGame() {
	impl->left_game = true;
}

void Game::enableFlag(Flag flag) {
}

void Game::setLocalSpeed(int speed) {

}

void Game::setGUI(bool enable) {

}

void Game::setFrameSkip(int frameskip) {

}

void Game::setCommandOptimizationLevel(int level) {
}

bool Game::setMap(std::string filename) {
	impl->set_map_filename = filename;
	return true;
}

std::string Game::mapFileName() {
	return impl->map_filename;
}

std::vector<TilePosition> Game::getStartLocations() {
	std::vector<TilePosition> r;
	for (auto& v : impl->game_st.start_locations) {
		if (v != bwgame::xy()) r.push_back({v.x / 32, v.y / 32});
	}
	return r;
}

int Game::mapWidth() {
	return (int)impl->game_st.map_tile_width;
}

int Game::mapHeight() {
	return (int)impl->game_st.map_tile_height;
}

Unit Game::getUnit(int id) {
	if (id == -1) return nullptr;
	return impl->funcs.get_unit(id);
}

int Game::getFrameCount() {
	return impl->st.current_frame;
}

bool Game::isReplay() {
	return impl->funcs.is_replay;
}

Player Game::self() {
	if (impl->funcs.local_player_id == -1) return nullptr;
	return impl->funcs.get_player(impl->funcs.local_player_id);
}

Player Game::enemy() {
	if (impl->funcs.enemy_player_id == -1) return nullptr;
	return impl->funcs.get_player(impl->funcs.enemy_player_id);
}

Player Game::neutral() {
	return impl->funcs.get_player(11);
}

const std::vector<Player>& Game::getPlayers() {
	auto& players = impl->funcs.players;
	if (players.empty()) {
		for (auto& v : impl->st.players) {
			if (v.controller == bwgame::player_t::controller_occupied) {
				players.push_back(impl->funcs.get_player((size_t)(&v - impl->st.players.data())));
			}
		}
	}
	return players;
//	return bwgame::make_transform_range(bwgame::make_filter_range(st.players, [&](auto& v) mutable {
//		return v.controller == bwgame::player_t::controller_occupied;
//	}), [&, tmp = Player{}](auto& v) mutable -> Player& {
//		return tmp = funcs.get_player((size_t)(&v - st.players.data()));
//	});
}

bool Game::isExplored(int x, int y) {
	return impl->funcs.player_position_is_explored(impl->funcs.local_player_id, {x, y});
}

bool Game::isVisible(int x, int y) {
	return impl->funcs.player_position_is_visible(impl->funcs.local_player_id, {x, y});
}

const std::vector<Bullet>& Game::getBullets() {
	auto& bullets = impl->funcs.bullets;
	bullets.clear();
	for (auto* b : ptr(impl->st.active_bullets)) {
		bullets.push_back(b);
	}
	return bullets;
//	return bwgame::make_transform_range(ptr(st.active_bullets), [tmp = Bullet{}](bwgame::bullet_t* b) mutable -> Bullet& {
//		return tmp = Bullet{b};
//	});
}

const std::vector<Unit>& Game::getNeutralUnits() {
	return impl->funcs.get_player(11)->getUnits();
}

const std::vector<Event>& Game::getEvents() {
	return impl->events;
}

bool Game::isWalkable(int x, int y) {
	return impl->funcs.is_walkable({int(x * 8), int(y * 8)});
}

bool Game::isBuildable(int x, int y) {
	return (impl->st.tiles[impl->funcs.tile_index({int(x * 32u), int(y * 32u)})].flags & (bwgame::tile_t::flag_unbuildable | bwgame::tile_t::flag_partially_walkable)) == 0;
}

int Game::getGroundHeight(int x, int y) {
	return impl->funcs.get_ground_height_at({int(x * 32u), int(y * 32u)});
}

void Game::vPrintf(const char *fmt, va_list args) {
	vprintf((std::string(fmt) + "\n").c_str(), args);
	fflush(stdout);
}

void Game::update() {
	impl->update();
}

Position UnitCommand::getTargetPosition() const {
	return target ? target->getPosition() : Position();
}

}
