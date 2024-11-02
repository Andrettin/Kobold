#include "kobold.h"

#include "map/site_game_data.h"

#include "country/country.h"
#include "country/country_game_data.h"
#include "country/country_turn_data.h"
#include "country/culture.h"
#include "database/defines.h"
#include "economy/commodity.h"
#include "economy/resource.h"
#include "game/country_event.h"
#include "game/event_trigger.h"
#include "game/game.h"
#include "infrastructure/building_class.h"
#include "infrastructure/building_slot_type.h"
#include "infrastructure/building_type.h"
#include "infrastructure/improvement.h"
#include "infrastructure/improvement_slot.h"
#include "infrastructure/settlement_building_slot.h"
#include "infrastructure/settlement_type.h"
#include "infrastructure/wonder.h"
#include "map/diplomatic_map_mode.h"
#include "map/map.h"
#include "map/province.h"
#include "map/province_game_data.h"
#include "map/site.h"
#include "map/site_map_data.h"
#include "map/tile.h"
#include "script/condition/and_condition.h"
#include "script/context.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "script/scripted_site_modifier.h"
#include "unit/army.h"
#include "util/assert_util.h"
#include "util/log_util.h"
#include "util/map_util.h"
#include "util/vector_random_util.h"
#include "util/vector_util.h"

namespace kobold {

site_game_data::site_game_data(const kobold::site *site) : site(site)
{
	if (site->is_settlement()) {
		this->initialize_building_slots();
	}

	this->set_resource_discovered(true);
}

void site_game_data::do_turn()
{
	this->decrement_scripted_modifiers();
}

const QPoint &site_game_data::get_tile_pos() const
{
	return this->site->get_map_data()->get_tile_pos();
}

tile *site_game_data::get_tile() const
{
	return this->site->get_map_data()->get_tile();
}

bool site_game_data::is_coastal() const
{
	return this->site->get_map_data()->is_coastal();
}

bool site_game_data::is_near_water() const
{
	return this->site->get_map_data()->is_near_water();
}

bool site_game_data::has_route() const
{
	const tile *tile = this->get_tile();

	if (tile == nullptr) {
		return false;
	}

	return tile->has_route();
}

const province *site_game_data::get_province() const
{
	return this->site->get_map_data()->get_province();
}

bool site_game_data::is_provincial_capital() const
{
	if (this->get_province() == nullptr) {
		return false;
	}

	return this->get_province()->get_provincial_capital() == this->site;
}

bool site_game_data::is_capital() const
{
	if (this->get_owner() == nullptr) {
		return false;
	}

	return this->get_owner()->get_game_data()->get_capital() == this->site;
}

bool site_game_data::can_be_capital() const
{
	assert_throw(this->site->is_settlement());

	if (!this->is_built()) {
		return false;
	}

	return true;
}

void site_game_data::set_owner(const country *owner)
{
	if (owner == this->get_owner()) {
		return;
	}

	const country *old_owner = this->get_owner();

	if (old_owner != nullptr) {
		for (const auto &[improvement, commodity_bonuses] : old_owner->get_game_data()->get_improvement_commodity_bonuses()) {
			if (!this->has_improvement(improvement)) {
				continue;
			}

			for (const auto &[commodity, bonus] : commodity_bonuses) {
				this->change_base_commodity_output(commodity, -bonus);
			}
		}

		if (this->site->is_settlement()) {
			for (const auto &[commodity, bonus] : old_owner->get_game_data()->get_settlement_commodity_bonuses()) {
				this->change_base_commodity_output(commodity, -bonus);
			}

			for (const auto &[building, commodity_bonuses] : old_owner->get_game_data()->get_building_commodity_bonuses()) {
				if (!this->has_building(building)) {
					continue;
				}

				for (const auto &[commodity, bonus] : commodity_bonuses) {
					this->change_base_commodity_output(commodity, -centesimal_int(bonus));
				}
			}
		}

		old_owner->get_game_data()->on_site_gained(this->site, -1);

		for (const auto &[commodity, output] : this->get_commodity_outputs()) {
			if (commodity->is_local()) {
				continue;
			}

			old_owner->get_game_data()->change_commodity_output(commodity, -output);
		}
	}

	this->owner = owner;

	if (this->get_owner() != nullptr) {
		for (const auto &[commodity, output] : this->get_commodity_outputs()) {
			if (commodity->is_local()) {
				continue;
			}

			this->get_owner()->get_game_data()->change_commodity_output(commodity, output);
		}

		for (const auto &[improvement, commodity_bonuses] : this->get_owner()->get_game_data()->get_improvement_commodity_bonuses()) {
			if (!this->has_improvement(improvement)) {
				continue;
			}

			for (const auto &[commodity, bonus] : commodity_bonuses) {
				this->change_base_commodity_output(commodity, bonus);
			}
		}

		if (this->site->is_settlement()) {
			for (const auto &[commodity, bonus] : this->get_owner()->get_game_data()->get_settlement_commodity_bonuses()) {
				this->change_base_commodity_output(commodity, bonus);
			}

			for (const auto &[building, commodity_bonuses] : this->get_owner()->get_game_data()->get_building_commodity_bonuses()) {
				if (!this->has_building(building)) {
					continue;
				}

				for (const auto &[commodity, bonus] : commodity_bonuses) {
					this->change_base_commodity_output(commodity, centesimal_int(bonus));
				}
			}
		}

		this->get_owner()->get_game_data()->on_site_gained(this->site, 1);
	}

	if (this->site->is_settlement() && this->is_built()) {
		this->check_building_conditions();
		this->check_free_buildings();
	}

	if (game::get()->is_running()) {
		emit owner_changed();
	}
}

const std::string &site_game_data::get_current_cultural_name() const
{
	return this->site->get_cultural_name(this->get_culture());
}

void site_game_data::set_culture(const kobold::culture *culture)
{
	if (culture == this->get_culture()) {
		return;
	}

	this->culture = culture;

	if (game::get()->is_running()) {
		if (this->get_owner() != nullptr) {
			this->get_owner()->get_turn_data()->set_diplomatic_map_mode_dirty(diplomatic_map_mode::cultural);
		}
	}

	emit culture_changed();
}

void site_game_data::set_religion(const kobold::religion *religion)
{
	if (religion == this->get_religion()) {
		return;
	}

	this->religion = religion;

	if (game::get()->is_running()) {
		if (this->get_owner() != nullptr) {
			this->get_owner()->get_turn_data()->set_diplomatic_map_mode_dirty(diplomatic_map_mode::religious);
		}
	}

	emit religion_changed();
}

void site_game_data::set_settlement_type(const kobold::settlement_type *settlement_type)
{
	assert_throw(this->site->is_settlement());

	if (settlement_type == this->get_settlement_type()) {
		return;
	}

	const kobold::settlement_type *old_settlement_type = this->get_settlement_type();

	if (old_settlement_type != nullptr) {
		if (old_settlement_type->get_modifier() != nullptr) {
			old_settlement_type->get_modifier()->apply(this->site, -1);
		}
	}

	this->settlement_type = settlement_type;

	if (old_settlement_type == nullptr && this->get_settlement_type() != nullptr) {
		this->on_settlement_built(1);

		if (this->get_owner() != nullptr) {
			if (this->get_owner()->get_game_data()->get_capital() == nullptr && this->can_be_capital()) {
				this->get_owner()->get_game_data()->set_capital(this->site);
			}
		}
	} else if (old_settlement_type != nullptr && this->get_settlement_type() == nullptr) {
		this->on_settlement_built(-1);

		if (this->get_owner() != nullptr) {
			if (this->get_owner()->get_game_data()->get_capital() == this->site) {
				this->get_owner()->get_game_data()->choose_capital();
			}
		}
	}

	if (this->get_settlement_type() != nullptr) {
		if (this->get_settlement_type()->get_modifier() != nullptr) {
			this->get_settlement_type()->get_modifier()->apply(this->site, 1);
		}

		this->check_building_conditions();
		this->check_free_buildings();
	}

	if (game::get()->is_running()) {
		emit settlement_type_changed();
		emit map::get()->tile_settlement_type_changed(this->get_tile_pos());
	}
}

void site_game_data::check_settlement_type()
{
	if (this->get_settlement_type() == nullptr) {
		return;
	}

	if (this->get_settlement_type()->get_conditions() == nullptr || this->get_settlement_type()->get_conditions()->check(this->site, read_only_context(this->site))) {
		return;
	}

	std::vector<const kobold::settlement_type *> potential_settlement_types;

	int best_preserved_building_count = 0;

	for (const kobold::settlement_type *base_settlement_type : this->get_settlement_type()->get_base_settlement_types()) {
		if (base_settlement_type->get_conditions() != nullptr && !base_settlement_type->get_conditions()->check(this->site, read_only_context(this->site))) {
			continue;
		}

		int preserved_building_count = 0;
		for (const qunique_ptr<settlement_building_slot> &building_slot : this->building_slots) {
			if (building_slot->get_building() == nullptr) {
				continue;
			}

			if (vector::contains(building_slot->get_building()->get_settlement_types(), base_settlement_type)) {
				++preserved_building_count;
			}
		}

		if (preserved_building_count < best_preserved_building_count) {
			continue;
		} else if (preserved_building_count > best_preserved_building_count) {
			potential_settlement_types.clear();
			best_preserved_building_count = preserved_building_count;
		}

		potential_settlement_types.push_back(base_settlement_type);
	}

	if (!potential_settlement_types.empty()) {
		this->set_settlement_type(vector::get_random(potential_settlement_types));
	}
}

bool site_game_data::is_built() const
{
	assert_throw(this->site->is_settlement());

	return this->get_settlement_type() != nullptr;
}

const resource *site_game_data::get_resource() const
{
	return this->site->get_map_data()->get_resource();
}

const improvement *site_game_data::get_main_improvement() const
{
	const improvement *main_improvement = this->get_improvement(improvement_slot::main);

	if (main_improvement != nullptr) {
		return main_improvement;
	}

	return this->get_improvement(improvement_slot::resource);
}

const improvement *site_game_data::get_resource_improvement() const
{
	return this->get_improvement(improvement_slot::resource);
}

bool site_game_data::has_improvement(const improvement *improvement) const
{
	assert_throw(improvement != nullptr);

	return this->get_improvement(improvement->get_slot()) == improvement;
}

void site_game_data::set_improvement(const improvement_slot slot, const improvement *improvement)
{
	const kobold::improvement *old_improvement = this->get_improvement(slot);

	if (old_improvement == improvement) {
		return;
	}

	if (old_improvement != nullptr) {
		this->on_improvement_gained(old_improvement, -1);
	}

	this->improvements[slot] = improvement;

	if (improvement != nullptr) {
		this->on_improvement_gained(improvement, 1);
	}

	if (slot == improvement_slot::main || (slot == improvement_slot::resource && this->get_improvement(improvement_slot::main) == nullptr)) {
		this->get_tile()->on_main_improvement_changed();
	}

	emit improvements_changed();
	emit map::get()->tile_improvement_changed(this->get_tile_pos());
}

QVariantList site_game_data::get_building_slots_qvariant_list() const
{
	std::vector<const settlement_building_slot *> available_building_slots;

	for (const qunique_ptr<settlement_building_slot> &building_slot : this->building_slots) {
		if (!building_slot->is_available()) {
			continue;
		}

		available_building_slots.push_back(building_slot.get());
	}

	return container::to_qvariant_list(available_building_slots);
}

void site_game_data::initialize_building_slots()
{
	assert_throw(this->site->is_settlement());

	//initialize building slots, placing them in random order
	std::vector<building_slot_type *> building_slot_types = building_slot_type::get_all();
	vector::shuffle(building_slot_types);

	for (const building_slot_type *building_slot_type : building_slot_types) {
		this->building_slots.push_back(make_qunique<settlement_building_slot>(building_slot_type, this->site));
		this->building_slot_map[building_slot_type] = this->building_slots.back().get();
	}
}

const building_type *site_game_data::get_slot_building(const building_slot_type *slot_type) const
{
	const auto find_iterator = this->building_slot_map.find(slot_type);
	if (find_iterator != this->building_slot_map.end()) {
		return find_iterator->second->get_building();
	}

	assert_throw(false);

	return nullptr;
}

void site_game_data::set_slot_building(const building_slot_type *slot_type, const building_type *building)
{
	if (building != nullptr) {
		assert_throw(building->get_slot_type() == slot_type);
	}

	const auto find_iterator = this->building_slot_map.find(slot_type);
	if (find_iterator != this->building_slot_map.end()) {
		find_iterator->second->set_building(building);
		return;
	}

	assert_throw(false);
}

const building_type *site_game_data::get_building_class_type(const building_class *building_class) const
{
	if (this->get_culture() == nullptr) {
		return nullptr;
	}

	return this->get_culture()->get_building_class_type(building_class);
}

bool site_game_data::has_building(const building_type *building) const
{
	return this->get_slot_building(building->get_slot_type()) == building;
}

bool site_game_data::has_building_or_better(const building_type *building) const
{
	if (this->has_building(building)) {
		return true;
	}

	for (const building_type *requiring_building : building->get_requiring_buildings()) {
		if (this->has_building_or_better(requiring_building)) {
			return true;
		}
	}

	return false;
}

bool site_game_data::has_building_class(const building_class *building_class) const
{
	const building_type *building = this->get_slot_building(building_class->get_slot_type());

	if (building == nullptr) {
		return false;
	}

	return building->get_building_class() == building_class;
}

bool site_game_data::can_gain_building(const building_type *building) const
{
	return this->get_building_slot(building->get_slot_type())->can_gain_building(building);
}

bool site_game_data::can_gain_building_class(const building_class *building_class) const
{
	const building_type *building = this->get_building_class_type(building_class);

	if (building == nullptr) {
		return false;
	}

	return this->can_gain_building(building);
}

void site_game_data::add_building(const building_type *building)
{
	this->get_building_slot(building->get_slot_type())->set_building(building);
}

void site_game_data::clear_buildings()
{
	for (const qunique_ptr<settlement_building_slot> &building_slot : this->building_slots) {
		building_slot->set_wonder(nullptr);
		building_slot->set_building(nullptr);
	}
}

void site_game_data::check_building_conditions()
{
	assert_throw(this->site->is_settlement());

	for (const qunique_ptr<settlement_building_slot> &building_slot : this->building_slots) {
		const building_type *building = building_slot->get_building();

		if (building == nullptr) {
			continue;
		}

		const int building_level = building->get_level();

		const wonder *wonder = building_slot->get_wonder();
		if (wonder != nullptr && !building_slot->can_have_wonder(wonder)) {
			building_slot->set_wonder(nullptr);
		}

		//if the building fails its conditions, try to replace it with one of its required buildings, if valid
		while (building != nullptr) {
			if (building_slot->can_maintain_building(building)) {
				break;
			}

			building = building->get_required_building();
		}

		//try to place a building of equivalent level for the same slot which has its conditions fulfilled
		if (building == nullptr) {
			std::vector<const building_type *> potential_buildings;

			for (const building_type *slot_building : building_slot->get_type()->get_building_types()) {
				if (slot_building->get_required_building() != nullptr) {
					continue;
				}

				if (!building_slot->can_maintain_building(slot_building)) {
					continue;
				}

				potential_buildings.push_back(slot_building);
			}

			if (!potential_buildings.empty()) {
				building = vector::get_random(potential_buildings);
			}
		}

		if (building != nullptr && building->get_level() < building_level) {
			for (int i = building->get_level() + 1; i <= building_level; ++i) {
				std::vector<const building_type *> potential_buildings;

				for (const building_type *requiring_building : building->get_requiring_buildings()) {
					if (!building_slot->can_maintain_building(requiring_building)) {
						continue;
					}

					potential_buildings.push_back(requiring_building);
				}

				if (potential_buildings.empty()) {
					break;
				}

				building = vector::get_random(potential_buildings);
				assert_throw(building->get_level() == i);
			}
		}

		if (building != building_slot->get_building()) {
			building_slot->set_building(building);
		}
	}
}

void site_game_data::check_free_buildings()
{
	assert_throw(this->site->is_settlement());

	const country *owner = this->get_owner();
	if (owner == nullptr) {
		return;
	}

	if (this->get_culture() == nullptr) {
		return;
	}

	const country_game_data *owner_game_data = owner->get_game_data();

	bool changed = false;

	for (const auto &[building_class, count] : owner_game_data->get_free_building_class_counts()) {
		assert_throw(count > 0);

		const building_type *building = this->get_culture()->get_building_class_type(building_class);

		if (building == nullptr) {
			continue;
		}

		if (this->check_free_building(building)) {
			changed = true;
		}
	}

	if (this->is_capital()) {
		for (const building_type *building : building_type::get_all()) {
			if (!building->is_free_in_capital()) {
				continue;
			}

			if (this->check_free_building(building)) {
				changed = true;
			}
		}
	}

	if (changed) {
		//check free buildings again, as the addition of a free building might have caused the requirements of others to be fulfilled
		this->check_free_buildings();
	}
}

bool site_game_data::check_free_building(const building_type *building)
{
	if (building != this->get_culture()->get_building_class_type(building->get_building_class())) {
		return false;
	}

	if (this->has_building_or_better(building)) {
		return false;
	}

	settlement_building_slot *building_slot = this->get_building_slot(building->get_slot_type());

	if (building_slot == nullptr) {
		return false;
	}

	if (!building_slot->can_gain_building(building)) {
		return false;
	}

	if (building->get_required_building() != nullptr && building_slot->get_building() != building->get_required_building()) {
		return false;
	}

	building_slot->set_building(building);
	return true;
}

bool site_game_data::check_free_improvement(const improvement *improvement)
{
	if (!improvement->is_buildable_on_site(this->site)) {
		return false;
	}

	this->set_improvement(improvement->get_slot(), improvement);

	return true;
}

void site_game_data::on_settlement_built(const int multiplier)
{
	this->get_province()->get_game_data()->change_settlement_count(multiplier);

	if (this->get_owner() != nullptr) {
		this->get_owner()->get_game_data()->change_settlement_count(multiplier);
	}

	if (this->get_province() != nullptr) {
		for (const auto &[commodity, bonus] : defines::get()->get_settlement_commodity_bonuses()) {
			this->change_base_commodity_output(commodity, centesimal_int(bonus));
		}

		const tile *tile = this->get_tile();
		if (tile != nullptr && tile->has_river()) {
			for (const auto &[commodity, bonus] : defines::get()->get_river_settlement_commodity_bonuses()) {
				this->change_base_commodity_output(commodity, centesimal_int(bonus));
			}
		}

		if (this->get_owner() != nullptr) {
			for (const auto &[commodity, bonus] : this->get_owner()->get_game_data()->get_settlement_commodity_bonuses()) {
				this->change_base_commodity_output(commodity, bonus);
			}
		}
	}
}

void site_game_data::on_building_gained(const building_type *building, const int multiplier)
{
	assert_throw(building != nullptr);
	assert_throw(multiplier != 0);
	assert_throw(this->get_province() != nullptr);

	if (this->get_owner() != nullptr) {
		country_game_data *country_game_data = this->get_owner()->get_game_data();
		country_game_data->change_settlement_building_count(building, multiplier);

		for (const auto &[commodity, bonus] : country_game_data->get_building_commodity_bonuses(building)) {
			this->change_base_commodity_output(commodity, centesimal_int(bonus) * multiplier);
		}
	}

	if (building->get_province_modifier() != nullptr) {
		building->get_province_modifier()->apply(this->get_province(), multiplier);
	}

	if (building->get_settlement_modifier() != nullptr) {
		building->get_settlement_modifier()->apply(this->site, multiplier);
	}

	if (multiplier > 0 && building->get_effects() != nullptr) {
		context effects_ctx(this->site);
		building->get_effects()->do_effects(this->site, effects_ctx);
	}
}

void site_game_data::on_wonder_gained(const wonder *wonder, const int multiplier)
{
	assert_throw(wonder != nullptr);
	assert_throw(multiplier != 0);

	if (this->get_owner() != nullptr) {
		this->get_owner()->get_game_data()->on_wonder_gained(wonder, multiplier);
	}

	if (wonder->get_province_modifier() != nullptr) {
		wonder->get_province_modifier()->apply(this->get_province(), multiplier);
	}
}

void site_game_data::on_improvement_gained(const improvement *improvement, const int multiplier)
{
	if (this->get_province() != nullptr && this->get_resource() != nullptr && improvement->get_slot() == improvement_slot::resource) {
		for (const auto &[commodity, value] : this->get_province()->get_game_data()->get_improved_resource_commodity_bonuses(this->get_resource())) {
			this->change_base_commodity_output(commodity, centesimal_int(value) * multiplier);
		}
	}

	if (improvement->get_modifier() != nullptr) {
		improvement->get_modifier()->apply(this->site, multiplier);
	}

	if (this->get_owner() != nullptr) {
		const country_game_data *country_game_data = this->get_owner()->get_game_data();

		if (improvement->get_country_modifier() != nullptr) {
			improvement->get_country_modifier()->apply(this->get_owner(), multiplier);
		}

		for (const auto &[commodity, bonus] : country_game_data->get_improvement_commodity_bonuses(improvement)) {
			this->change_base_commodity_output(commodity, bonus * multiplier);
		}
	}
}

QVariantList site_game_data::get_scripted_modifiers_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_scripted_modifiers());
}

bool site_game_data::has_scripted_modifier(const scripted_site_modifier *modifier) const
{
	return this->get_scripted_modifiers().contains(modifier);
}

void site_game_data::add_scripted_modifier(const scripted_site_modifier *modifier, const int duration)
{
	const read_only_context ctx(this->site);

	this->scripted_modifiers[modifier] = std::max(this->scripted_modifiers[modifier], duration);

	if (modifier->get_modifier() != nullptr) {
		modifier->get_modifier()->apply(this->site);
	}

	if (game::get()->is_running()) {
		emit scripted_modifiers_changed();
	}
}

void site_game_data::remove_scripted_modifier(const scripted_site_modifier *modifier)
{
	this->scripted_modifiers.erase(modifier);

	if (modifier->get_modifier() != nullptr) {
		modifier->get_modifier()->remove(this->site);
	}

	if (game::get()->is_running()) {
		emit scripted_modifiers_changed();
	}
}

void site_game_data::decrement_scripted_modifiers()
{
	if (this->scripted_modifiers.empty()) {
		return;
	}

	std::vector<const scripted_site_modifier *> modifiers_to_remove;
	for (auto &[modifier, duration] : this->scripted_modifiers) {
		--duration;

		if (duration == 0) {
			modifiers_to_remove.push_back(modifier);
		}
	}

	for (const scripted_site_modifier *modifier : modifiers_to_remove) {
		this->remove_scripted_modifier(modifier);
	}
}

void site_game_data::change_base_commodity_output(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int &count = (this->base_commodity_outputs[commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->base_commodity_outputs.erase(commodity);
	}

	this->calculate_commodity_outputs();
}

QVariantList site_game_data::get_commodity_outputs_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_commodity_outputs());
}

void site_game_data::set_commodity_output(const commodity *commodity, const centesimal_int &output)
{
	assert_throw(output >= 0);

	const centesimal_int old_output = this->get_commodity_output(commodity);
	if (output == old_output) {
		return;
	}

	if (output == 0) {
		this->commodity_outputs.erase(commodity);
	} else {
		this->commodity_outputs[commodity] = output;
	}

	const centesimal_int output_change = output - old_output;

	if (commodity->is_local()) {
		if (this->get_province() != nullptr) {
			this->get_province()->get_game_data()->change_local_commodity_output(commodity, output_change);
		}
	} else {
		if (this->get_owner() != nullptr) {
			this->get_owner()->get_game_data()->change_commodity_output(commodity, output_change);
		}
	}

	emit commodity_outputs_changed();
}

void site_game_data::calculate_commodity_outputs()
{
	commodity_map<centesimal_int> outputs = this->base_commodity_outputs;

	for (const auto &[commodity, output] : this->get_commodity_outputs()) {
		//ensure the current outputs are in the new map, so that they get removed if no longer present
		outputs[commodity];
	}

	centesimal_int output_modifier = this->get_output_modifier();
	commodity_map<centesimal_int> commodity_output_modifiers = this->get_commodity_output_modifiers();

	if (this->get_owner() != nullptr) {
		if (this->is_capital()) {
			for (const auto &[commodity, value] : this->get_owner()->get_game_data()->get_capital_commodity_bonuses()) {
				outputs[commodity] += value;
			}

			for (const auto &[commodity, modifier] : this->get_owner()->get_game_data()->get_capital_commodity_output_modifiers()) {
				commodity_output_modifiers[commodity] += modifier;
			}
		}
	}

	const province *province = this->get_province();
	if (province != nullptr) {
		for (auto &[commodity, output] : outputs) {
			for (const auto &[threshold, bonus] : province->get_game_data()->get_commodity_bonus_for_tile_threshold_map(commodity)) {
				if (output >= threshold) {
					output += bonus;
				}
			}
		}

		output_modifier += province->get_game_data()->get_output_modifier();

		for (const auto &[commodity, modifier] : province->get_game_data()->get_commodity_output_modifiers()) {
			commodity_output_modifiers[commodity] += modifier;
		}
	}

	if (this->get_owner() != nullptr) {
		output_modifier += this->get_owner()->get_game_data()->get_output_modifier();

		for (const auto &[commodity, modifier] : this->get_owner()->get_game_data()->get_commodity_output_modifiers()) {
			commodity_output_modifiers[commodity] += modifier;
		}
	}

	for (auto &[commodity, output] : outputs) {
		const centesimal_int modifier = output_modifier + commodity_output_modifiers[commodity];

		if (modifier != 0) {
			output *= centesimal_int(100) + modifier;
			output /= 100;
		}

		this->set_commodity_output(commodity, output);
	}
}

void site_game_data::change_local_everyday_consumption(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	log_trace(std::format("Changing local everyday consumption in settlement {} of commodity {} (currently {}) by {}.", this->site->get_identifier(), commodity->get_identifier(), this->get_local_everyday_consumption(commodity).to_string(), change.to_string()));

	const centesimal_int count = (this->local_everyday_consumption[commodity] += change);

	log_trace(std::format("Changed local everyday consumption in settlement {} of commodity {} by {}, making it now {}.", this->site->get_identifier(), commodity->get_identifier(), change.to_string(), this->get_local_everyday_consumption(commodity).to_string()));

	assert_throw(count >= 0);

	if (count == 0) {
		this->local_everyday_consumption.erase(commodity);
	}
}

void site_game_data::change_local_luxury_consumption(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	log_trace(std::format("Changing local luxury consumption in settlement {} of commodity {} (currently {}) by {}.", this->site->get_identifier(), commodity->get_identifier(), this->get_local_luxury_consumption(commodity).to_string(), change.to_string()));

	const centesimal_int count = (this->local_luxury_consumption[commodity] += change);

	log_trace(std::format("Changed local luxury consumption in settlement {} of commodity {} by {}, making it now {}.", this->site->get_identifier(), commodity->get_identifier(), change.to_string(), this->get_local_luxury_consumption(commodity).to_string()));

	assert_throw(count >= 0);

	if (count == 0) {
		this->local_luxury_consumption.erase(commodity);
	}
}

bool site_game_data::can_be_visited() const
{
	const improvement *improvement = this->get_improvement(improvement_slot::main);
	return improvement != nullptr && improvement->is_ruins();
}

QVariantList site_game_data::get_visiting_armies_qvariant_list() const
{
	return container::to_qvariant_list(this->get_visiting_armies());
}

}
