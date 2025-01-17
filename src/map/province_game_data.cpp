#include "kobold.h"

#include "map/province_game_data.h"

#include "country/country.h"
#include "country/country_game_data.h"
#include "country/country_turn_data.h"
#include "database/defines.h"
#include "database/preferences.h"
#include "economy/commodity.h"
#include "economy/commodity_container.h"
#include "economy/production_type.h"
#include "economy/resource.h"
#include "game/event_trigger.h"
#include "game/game.h"
#include "game/province_event.h"
#include "infrastructure/building_type.h"
#include "infrastructure/improvement.h"
#include "infrastructure/settlement_building_slot.h"
#include "infrastructure/settlement_type.h"
#include "map/diplomatic_map_mode.h"
#include "map/map.h"
#include "map/province.h"
#include "map/province_map_data.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "map/site_type.h"
#include "map/tile.h"
#include "religion/religion.h"
#include "script/context.h"
#include "script/modifier.h"
#include "script/scripted_province_modifier.h"
#include "ui/icon.h"
#include "ui/icon_container.h"
#include "unit/army.h"
#include "unit/military_unit.h"
#include "unit/military_unit_category.h"
#include "unit/military_unit_type.h"
#include "util/assert_util.h"
#include "util/container_util.h"
#include "util/image_util.h"
#include "util/map_util.h"
#include "util/point_util.h"

#include "xbrz.h"

namespace kobold {

province_game_data::province_game_data(const kobold::province *province)
	: province(province)
{
}

province_game_data::~province_game_data()
{
}

void province_game_data::do_turn()
{
	for (const site *site : this->get_sites()) {
		site->get_game_data()->do_turn();
	}

	this->decrement_scripted_modifiers();
}

void province_game_data::do_events()
{
	const bool is_last_turn_of_year = game::get()->get_year() != game::get()->get_next_date().year();

	if (is_last_turn_of_year) {
		province_event::check_events_for_scope(this->province, event_trigger::yearly_pulse);
	}

	province_event::check_events_for_scope(this->province, event_trigger::quarterly_pulse);
	province_event::check_events_for_scope(this->province, event_trigger::per_turn_pulse);
}

void province_game_data::do_ai_turn()
{
}

bool province_game_data::is_on_map() const
{
	return this->province->get_map_data()->is_on_map();
}

void province_game_data::set_owner(const country *country)
{
	if (country == this->get_owner()) {
		return;
	}

	const kobold::country *old_owner = this->owner;

	this->owner = country;

	for (const site *site : this->get_sites()) {
		if (site->get_game_data()->get_owner() == old_owner) {
			site->get_game_data()->set_owner(country);
		}
	}

	if (old_owner != nullptr) {
		old_owner->get_game_data()->remove_province(this->province);
	}

	if (this->get_owner() != nullptr) {
		this->get_owner()->get_game_data()->add_province(this->province);
	}

	if (game::get()->is_running()) {
		for (const QPoint &tile_pos : this->get_border_tiles()) {
			map::get()->calculate_tile_country_border_directions(tile_pos);
		}

		map::get()->update_minimap_rect(this->get_territory_rect());

		emit owner_changed();
	}
}

bool province_game_data::is_capital() const
{
	if (this->get_owner() == nullptr) {
		return false;
	}

	return this->get_owner()->get_game_data()->get_capital_province() == this->province;
}

void province_game_data::set_culture(const kobold::culture *culture)
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

	for (const site *site : this->get_sites()) {
		if (!site->is_settlement()) {
			site->get_game_data()->set_culture(culture);
		}
	}

	for (const site *settlement : this->get_settlement_sites()) {
		settlement->get_game_data()->set_culture(this->get_culture());
	}
}

void province_game_data::set_religion(const kobold::religion *religion)
{
	if (religion == this->get_religion()) {
		return;
	}

	this->religion = religion;

	for (const site *settlement : this->get_settlement_sites()) {
		settlement->get_game_data()->set_religion(this->get_religion());
	}

	if (game::get()->is_running()) {
		if (this->get_owner() != nullptr) {
			this->get_owner()->get_turn_data()->set_diplomatic_map_mode_dirty(diplomatic_map_mode::religious);
		}
	}

	emit religion_changed();
}

const std::string &province_game_data::get_current_cultural_name() const
{
	return this->province->get_cultural_name(this->get_culture());
}

bool province_game_data::is_coastal() const
{
	return this->province->get_map_data()->is_coastal();
}

bool province_game_data::is_near_water() const
{
	return this->province->get_map_data()->is_near_water();
}

const QRect &province_game_data::get_territory_rect() const
{
	return this->province->get_map_data()->get_territory_rect();
}

const QPoint &province_game_data::get_territory_rect_center() const
{
	return this->province->get_map_data()->get_territory_rect_center();
}

const std::vector<const kobold::province *> &province_game_data::get_neighbor_provinces() const
{
	return this->province->get_map_data()->get_neighbor_provinces();
}

bool province_game_data::is_country_border_province() const
{
	for (const kobold::province *neighbor_province : this->get_neighbor_provinces()) {
		const province_game_data *neighbor_province_game_data = neighbor_province->get_game_data();
		if (neighbor_province_game_data->get_owner() != this->get_owner()) {
			return true;
		}
	}

	return false;
}

const QPoint &province_game_data::get_center_tile_pos() const
{
	return this->province->get_map_data()->get_center_tile_pos();
}

const std::vector<QPoint> &province_game_data::get_border_tiles() const
{
	return this->province->get_map_data()->get_border_tiles();
}

const std::vector<QPoint> &province_game_data::get_resource_tiles() const
{
	return this->province->get_map_data()->get_resource_tiles();
}

const std::vector<const site *> &province_game_data::get_sites() const
{
	return this->province->get_map_data()->get_sites();
}

const std::vector<const site *> &province_game_data::get_settlement_sites() const
{
	return this->province->get_map_data()->get_settlement_sites();
}

bool province_game_data::produces_commodity(const commodity *commodity) const
{
	for (const QPoint &tile_pos : this->get_resource_tiles()) {
		const tile *tile = map::get()->get_tile(tile_pos);

		if (tile->produces_commodity(commodity)) {
			return true;
		}
	}

	return false;
}

const resource_map<int> &province_game_data::get_resource_counts() const
{
	return this->province->get_map_data()->get_resource_counts();
}

const terrain_type_map<int> &province_game_data::get_tile_terrain_counts() const
{
	return this->province->get_map_data()->get_tile_terrain_counts();
}

QVariantList province_game_data::get_scripted_modifiers_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_scripted_modifiers());
}

bool province_game_data::has_scripted_modifier(const scripted_province_modifier *modifier) const
{
	return this->get_scripted_modifiers().contains(modifier);
}

void province_game_data::add_scripted_modifier(const scripted_province_modifier *modifier, const int duration)
{
	const read_only_context ctx(this->province);

	this->scripted_modifiers[modifier] = std::max(this->scripted_modifiers[modifier], duration);

	if (modifier->get_modifier() != nullptr) {
		this->apply_modifier(modifier->get_modifier());
	}

	if (game::get()->is_running()) {
		emit scripted_modifiers_changed();
	}
}

void province_game_data::remove_scripted_modifier(const scripted_province_modifier *modifier)
{
	this->scripted_modifiers.erase(modifier);

	if (modifier->get_modifier() != nullptr) {
		this->remove_modifier(modifier->get_modifier());
	}

	if (game::get()->is_running()) {
		emit scripted_modifiers_changed();
	}
}

void province_game_data::decrement_scripted_modifiers()
{
	std::vector<const scripted_province_modifier *> modifiers_to_remove;
	for (auto &[modifier, duration] : this->scripted_modifiers) {
		--duration;

		if (duration == 0) {
			modifiers_to_remove.push_back(modifier);
		}
	}

	for (const scripted_province_modifier *modifier : modifiers_to_remove) {
		this->remove_scripted_modifier(modifier);
	}
}

void province_game_data::apply_modifier(const modifier<const kobold::province> *modifier, const int multiplier)
{
	assert_throw(modifier != nullptr);

	modifier->apply(this->province, multiplier);
}

QVariantList province_game_data::get_military_units_qvariant_list() const
{
	return container::to_qvariant_list(this->get_military_units());
}

void province_game_data::add_military_unit(military_unit *military_unit)
{
	this->military_units.push_back(military_unit);

	if (!military_unit->is_moving()) {
		this->change_military_unit_category_count(military_unit->get_category(), 1);
	}

	if (game::get()->is_running()) {
		emit military_units_changed();
	}
}

void province_game_data::remove_military_unit(military_unit *military_unit)
{
	std::erase(this->military_units, military_unit);

	if (!military_unit->is_moving()) {
		this->change_military_unit_category_count(military_unit->get_category(), -1);
	}

	if (game::get()->is_running()) {
		emit military_units_changed();
	}
}

void province_game_data::clear_military_units()
{
	this->military_units.clear();
	this->military_unit_category_counts.clear();
}

QVariantList province_game_data::get_military_unit_category_counts_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->military_unit_category_counts);
}

void province_game_data::change_military_unit_category_count(const military_unit_category category, const int change)
{
	if (change == 0) {
		return;
	}

	const int count = (this->military_unit_category_counts[category] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->military_unit_category_counts.erase(category);
	}

	if (game::get()->is_running()) {
		emit military_unit_category_counts_changed();
	}
}

bool province_game_data::has_country_military_unit(const country *country) const
{
	for (const military_unit *military_unit : this->get_military_units()) {
		if (military_unit->get_country() == country) {
			return true;
		}
	}

	return false;
}

QVariantList province_game_data::get_country_military_unit_category_counts(kobold::country *country) const
{
	std::map<military_unit_category, int> counts;

	for (const military_unit *military_unit : this->get_military_units()) {
		if (military_unit->get_country() == country && !military_unit->is_moving()) {
			++counts[military_unit->get_category()];
		}
	}

	return archimedes::map::to_qvariant_list(counts);
}

int province_game_data::get_country_military_unit_category_count(const kobold::military_unit_category category, kobold::country *country) const
{
	int count = 0;

	for (const military_unit *military_unit : this->get_military_units()) {
		if (military_unit->get_category() == category && military_unit->get_country() == country && !military_unit->is_moving()) {
			++count;
		}
	}

	return count;
}

const icon *province_game_data::get_military_unit_icon() const
{
	icon_map<int> icon_counts;

	for (const military_unit *military_unit : this->get_military_units()) {
		if (!military_unit->is_moving()) {
			++icon_counts[military_unit->get_icon()];
		}
	}

	const icon *best_icon = nullptr;
	int best_icon_count = 0;
	for (const auto &[icon, count] : icon_counts) {
		if (count > best_icon_count) {
			best_icon = icon;
			best_icon_count = count;
		}
	}

	assert_throw(best_icon != nullptr);

	return best_icon;
}

const icon *province_game_data::get_military_unit_category_icon(const military_unit_category category) const
{
	icon_map<int> icon_counts;

	for (const military_unit *military_unit : this->military_units) {
		if (military_unit->get_category() != category) {
			continue;
		}

		++icon_counts[military_unit->get_icon()];
	}

	const icon *best_icon = nullptr;
	int best_icon_count = 0;
	for (const auto &[icon, count] : icon_counts) {
		if (count > best_icon_count) {
			best_icon = icon;
			best_icon_count = count;
		}
	}

	assert_throw(best_icon != nullptr);

	return best_icon;
}

QString province_game_data::get_military_unit_category_name(const military_unit_category category) const
{
	std::map<QString, int> name_counts;

	for (const military_unit *military_unit : this->military_units) {
		if (military_unit->get_category() != category) {
			continue;
		}

		++name_counts[military_unit->get_type()->get_name_qstring()];
	}

	QString best_name;
	int best_name_count = 0;
	for (const auto &[name, count] : name_counts) {
		if (count > best_name_count) {
			best_name = name;
			best_name_count = count;
		}
	}

	assert_throw(!best_name.isEmpty());

	return best_name;
}

const icon *province_game_data::get_country_military_unit_icon(kobold::country *country) const
{
	icon_map<int> icon_counts;

	for (const military_unit *military_unit : this->get_military_units()) {
		if (military_unit->get_country() == country && !military_unit->is_moving()) {
			++icon_counts[military_unit->get_icon()];
		}
	}

	const icon *best_icon = nullptr;
	int best_icon_count = 0;
	for (const auto &[icon, count] : icon_counts) {
		if (count > best_icon_count) {
			best_icon = icon;
			best_icon_count = count;
		}
	}

	assert_throw(best_icon != nullptr);

	return best_icon;
}

QVariantList province_game_data::get_entering_armies_qvariant_list() const
{
	return container::to_qvariant_list(this->get_entering_armies());
}

void province_game_data::calculate_site_commodity_outputs()
{
	for (const site *site : this->get_sites()) {
		if (!site->is_settlement() && site->get_type() != site_type::resource && site->get_type() != site_type::celestial_body) {
			continue;
		}

		site->get_game_data()->calculate_commodity_outputs();
	}
}

void province_game_data::calculate_site_commodity_output(const commodity *commodity)
{
	for (const site *site : this->get_sites()) {
		if (!site->is_settlement() && site->get_type() != site_type::resource && site->get_type() != site_type::celestial_body) {
			continue;
		}

		if (!site->get_game_data()->produces_commodity(commodity) && !site->get_game_data()->get_base_commodity_outputs().contains(commodity)) {
			continue;
		}

		site->get_game_data()->calculate_commodity_outputs();
	}
}

void province_game_data::change_local_commodity_output(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int &output = (this->local_commodity_outputs[commodity] += change);

	if (output == 0) {
		this->local_commodity_outputs.erase(commodity);
	}
}

void province_game_data::change_improved_resource_commodity_bonus(const resource *resource, const commodity *commodity, const int change)
{
	if (change == 0) {
		return;
	}

	const int count = (this->improved_resource_commodity_bonuses[resource][commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->improved_resource_commodity_bonuses[resource].erase(commodity);

		if (this->improved_resource_commodity_bonuses[resource].empty()) {
			this->improved_resource_commodity_bonuses.erase(resource);
		}
	}

	for (const QPoint &tile_pos : this->get_resource_tiles()) {
		tile *tile = map::get()->get_tile(tile_pos);
		if (tile->get_resource() != resource) {
			continue;
		}

		assert_throw(tile->get_site() != nullptr);

		if (tile->get_site()->get_game_data()->get_resource_improvement() == nullptr) {
			continue;
		}

		tile->get_site()->get_game_data()->change_base_commodity_output(commodity, centesimal_int(change));
	}
}

void province_game_data::set_commodity_bonus_for_tile_threshold(const commodity *commodity, const int threshold, const int value)
{
	const int old_value = this->get_commodity_bonus_for_tile_threshold(commodity, threshold);

	if (value == old_value) {
		return;
	}

	if (value == 0) {
		this->commodity_bonuses_for_tile_thresholds[commodity].erase(threshold);

		if (this->commodity_bonuses_for_tile_thresholds[commodity].empty()) {
			this->commodity_bonuses_for_tile_thresholds.erase(commodity);
		}
	} else {
		this->commodity_bonuses_for_tile_thresholds[commodity][threshold] = value;
	}

	for (const QPoint &tile_pos : this->get_resource_tiles()) {
		tile *tile = map::get()->get_tile(tile_pos);
		if (!tile->produces_commodity(commodity)) {
			continue;
		}

		tile->calculate_commodity_outputs();
	}
}

}
