#include "kobold.h"

#include "country/country_game_data.h"

#include "character/character.h"
#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/character_game_data.h"
#include "country/consulate.h"
#include "country/country.h"
#include "country/country_attribute.h"
#include "country/country_feat.h"
#include "country/country_feat_type.h"
#include "country/country_milestone.h"
#include "country/country_rank.h"
#include "country/country_skill.h"
#include "country/country_tier.h"
#include "country/country_tier_data.h"
#include "country/country_turn_data.h"
#include "country/country_type.h"
#include "country/culture.h"
#include "country/diplomacy_state.h"
#include "country/government_group.h"
#include "country/government_type.h"
#include "country/journal_entry.h"
#include "country/law.h"
#include "country/law_group.h"
#include "country/office.h"
#include "country/tradition.h"
#include "country/tradition_category.h"
#include "country/tradition_group_container.h"
#include "database/defines.h"
#include "database/preferences.h"
#include "economy/commodity.h"
#include "economy/expense_transaction_type.h"
#include "economy/income_transaction_type.h"
#include "economy/production_type.h"
#include "economy/resource.h"
#include "engine_interface.h"
#include "game/country_event.h"
#include "game/event_trigger.h"
#include "game/game.h"
#include "game/game_rules.h"
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
#include "map/province_map_data.h"
#include "map/region.h"
#include "map/settlement_attribute.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "map/site_map_data.h"
#include "map/site_type.h"
#include "map/terrain_type.h"
#include "map/tile.h"
#include "religion/religion.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/factor.h"
#include "script/modifier.h"
#include "script/opinion_modifier.h"
#include "script/scripted_country_modifier.h"
#include "species/species.h"
#include "ui/icon.h"
#include "ui/icon_container.h"
#include "ui/portrait.h"
#include "unit/army.h"
#include "unit/civilian_unit.h"
#include "unit/civilian_unit_type.h"
#include "unit/military_unit.h"
#include "unit/military_unit_category.h"
#include "unit/military_unit_class.h"
#include "unit/military_unit_type.h"
#include "util/assert_util.h"
#include "util/container_util.h"
#include "util/gender.h"
#include "util/image_util.h"
#include "util/log_util.h"
#include "util/map_util.h"
#include "util/point_util.h"
#include "util/rect_util.h"
#include "util/size_util.h"
#include "util/string_util.h"
#include "util/vector_random_util.h"
#include "util/vector_util.h"

#include "xbrz.h"

namespace kobold {

country_game_data::country_game_data(kobold::country *country)
	: country(country), tier(country_tier::none), religion(country->get_default_religion())
{
	connect(this, &country_game_data::tier_changed, this, &country_game_data::title_name_changed);
	connect(this, &country_game_data::tier_changed, this, &country_game_data::office_title_names_changed);
	connect(this, &country_game_data::government_type_changed, this, &country_game_data::title_name_changed);
	connect(this, &country_game_data::government_type_changed, this, &country_game_data::office_title_names_changed);
	connect(this, &country_game_data::religion_changed, this, &country_game_data::title_name_changed);
	connect(this, &country_game_data::religion_changed, this, &country_game_data::office_title_names_changed);
	connect(this, &country_game_data::office_holders_changed, this, &country_game_data::office_title_names_changed);
	connect(this, &country_game_data::rank_changed, this, &country_game_data::type_name_changed);

	connect(this, &country_game_data::title_name_changed, this, [this]() {
		if (this->get_ruler() != nullptr) {
			emit this->get_ruler()->get_game_data()->titled_name_changed();
		}
	});

	for (const commodity *commodity : commodity::get_all()) {
		this->add_available_commodity(commodity);

		if (commodity->is_tradeable()) {
			this->add_tradeable_commodity(commodity);
		}
	}
}

country_game_data::~country_game_data()
{
}

void country_game_data::on_setup_finished()
{
	if (this->get_level() == 0) {
		this->change_level(1);
	}

	this->check_milestones();
	this->check_tier();
	this->check_government_type();
	this->check_laws();
	this->check_characters();

	for (const QPoint &border_tile_pos : this->get_border_tiles()) {
		map::get()->calculate_tile_country_border_directions(border_tile_pos);
	}

	//build free on start buildings
	for (const province *province : this->get_provinces()) {
		for (const site *settlement : province->get_game_data()->get_settlement_sites()) {
			if (!settlement->get_game_data()->is_built()) {
				continue;
			}

			for (const building_type *building : building_type::get_all()) {
				if (!building->is_free_on_start()) {
					continue;
				}

				settlement->get_game_data()->check_free_building(building);
			}
		}
	}
}

void country_game_data::do_turn()
{
	try {
		for (const province *province : this->get_provinces()) {
			province->get_game_data()->do_turn();
		}

		this->do_income_phase();

		this->do_production();
		this->do_construction();

		for (const qunique_ptr<civilian_unit> &civilian_unit : this->civilian_units) {
			civilian_unit->do_turn();
		}

		for (const qunique_ptr<military_unit> &military_unit : this->military_units) {
			military_unit->do_turn();
		}

		for (const qunique_ptr<army> &army : this->armies) {
			army->do_turn();
		}

		this->armies.clear();

		this->decrement_scripted_modifiers();

		this->check_journal_entries();
		this->check_milestones();

		this->check_tier();
		this->check_traditions();
		this->check_government_type();
		this->check_characters();
	} catch (...) {
		std::throw_with_nested(std::runtime_error(std::format("Failed to process turn for country \"{}\".", this->country->get_identifier())));
	}
}

void country_game_data::do_income_phase()
{
	this->collect_taxes();
}

void country_game_data::collect_taxes()
{
	const int result = this->do_skill_roll(defines::get()->get_country_tax_skill()) / 3;

	const commodity *wealth_commodity = defines::get()->get_wealth_commodity();
	const commodity_unit *wealth_commodity_unit = defines::get()->get_country_income_commodity_unit();
	assert_throw(wealth_commodity->has_unit(wealth_commodity_unit));
	const int unit_value = wealth_commodity->get_unit_value(wealth_commodity_unit);

	this->change_stored_commodity(wealth_commodity, unit_value * result);
}

void country_game_data::do_production()
{
	try {
		for (const auto &[commodity, output] : this->get_commodity_outputs()) {
			if (!commodity->is_storable()) {
				assert_throw(output >= 0);
				continue;
			}

			this->change_stored_commodity(commodity, output.to_int());
		}

		//reduce inputs from the storage for the next turn (for production this turn it had already been subtracted)
		for (const auto &[commodity, input] : this->get_commodity_inputs()) {
			try {
				if (!commodity->is_storable()) {
					const int output = this->get_commodity_output(commodity).to_int();
					if (input > output) {
						throw std::runtime_error(std::format("Input for non-storable commodity \"{}\" ({}) is greater than its output ({}).", commodity->get_identifier(), input, output));
					}
					continue;
				}

				this->change_stored_commodity(commodity, -input);
			} catch (...) {
				std::throw_with_nested(std::runtime_error("Error processing input storage reduction for commodity \"" + commodity->get_identifier()  + "\"."));
			}
		}
	} catch (...) {
		std::throw_with_nested(std::runtime_error("Error doing production for country \"" + this->country->get_identifier() + "\"."));
	}
}

void country_game_data::do_construction()
{
	try {
	} catch (...) {
		std::throw_with_nested(std::runtime_error("Error doing construction for country \"" + this->country->get_identifier() + "\"."));
	}
}

void country_game_data::do_trade(country_map<commodity_map<int>> &country_luxury_demands)
{
	try {
		if (this->is_under_anarchy()) {
			return;
		}

		//get the known countries and sort them by priority
		std::vector<const kobold::country *> countries = container::to_vector(this->get_known_countries());
		std::sort(countries.begin(), countries.end(), [&](const kobold::country *lhs, const kobold::country *rhs) {
			if (this->is_vassal_of(lhs) != this->is_vassal_of(rhs)) {
				return this->is_vassal_of(lhs);
			}

			if (this->is_any_vassal_of(lhs) != this->is_any_vassal_of(rhs)) {
				return this->is_any_vassal_of(lhs);
			}

			//give trade priority by opinion-weighted prestige
			const int lhs_opinion_weighted_prestige = this->get_opinion_weighted_prestige_for(lhs);
			const int rhs_opinion_weighted_prestige = this->get_opinion_weighted_prestige_for(rhs);

			if (lhs_opinion_weighted_prestige != rhs_opinion_weighted_prestige) {
				return lhs_opinion_weighted_prestige > rhs_opinion_weighted_prestige;
			}

			return lhs->get_identifier() < rhs->get_identifier();
		});

		commodity_map<int> offers = this->get_offers();
		for (auto &[commodity, offer] : offers) {
			const int price = game::get()->get_price(commodity);

			for (const kobold::country *other_country : countries) {
				country_game_data *other_country_game_data = other_country->get_game_data();

				const int bid = other_country_game_data->get_bid(commodity);
				if (bid != 0) {
					int sold_quantity = std::min(offer, bid);
					sold_quantity = std::min(sold_quantity, other_country_game_data->get_wealth() / price);

					if (sold_quantity > 0) {
						this->do_sale(other_country, commodity, sold_quantity, true);

						offer -= sold_quantity;

						if (offer == 0) {
							break;
						}
					}
				}

				int &demand = country_luxury_demands[other_country][commodity];
				if (demand > 0) {
					const int sold_quantity = std::min(offer, demand);

					if (sold_quantity > 0) {
						this->do_sale(country, commodity, sold_quantity, false);

						offer -= sold_quantity;
						demand -= sold_quantity;

						if (offer == 0) {
							break;
						}
					}
				}
			}
		}
	} catch (...) {
		std::throw_with_nested(std::runtime_error(std::format("Error doing trade for country \"{}\".", this->country->get_identifier())));
	}
}

void country_game_data::do_events()
{
	for (const province *province : this->get_provinces()) {
		province->get_game_data()->do_events();
	}

	for (const character *character : this->get_characters()) {
		character->get_game_data()->do_events();
	}

	if (this->is_under_anarchy()) {
		return;
	}

	const bool is_last_turn_of_year = game::get()->get_year() != game::get()->get_next_date().year();

	if (is_last_turn_of_year) {
		country_event::check_events_for_scope(this->country, event_trigger::yearly_pulse);
	}

	country_event::check_events_for_scope(this->country, event_trigger::quarterly_pulse);
	country_event::check_events_for_scope(this->country, event_trigger::per_turn_pulse);
}

void country_game_data::do_ai_turn()
{
	//build buildings
	for (const province *province : this->get_provinces()) {
		province->get_game_data()->do_ai_turn();
	}

	for (const qunique_ptr<civilian_unit> &civilian_unit : this->civilian_units) {
		civilian_unit->do_ai_turn();
	}

	for (const qunique_ptr<military_unit> &military_unit : this->military_units) {
		military_unit->do_ai_turn();
	}

	for (size_t i = 0; i < this->civilian_units.size();) {
		civilian_unit *civilian_unit = this->civilian_units.at(i).get();
		if (civilian_unit->is_busy()) {
			++i;
		} else {
			//if the civilian unit is idle, this means that nothing was found for it to do above; in that case, disband it
			civilian_unit->disband();
		}
	}

	this->assign_trade_orders();
}

bool country_game_data::is_ai() const
{
	return this->country != game::get()->get_player_country();
}

void country_game_data::set_tier(const country_tier tier)
{
	if (tier == this->get_tier()) {
		return;
	}

	assert_throw(tier >= this->country->get_min_tier());
	assert_throw(tier <= this->country->get_max_tier());

	if (this->get_tier() != country_tier::none) {
		country_tier_data::get(this->get_tier())->get_modifier()->remove(this->country);
	}

	this->tier = tier;

	if (this->get_tier() != country_tier::none) {
		country_tier_data::get(this->get_tier())->get_modifier()->apply(this->country);
	}

	if (game::get()->is_running()) {
		emit tier_changed();
	}
}

void country_game_data::check_tier()
{
	country_tier best_tier = this->country->get_min_tier();

	for (int i = static_cast<int>(this->country->get_min_tier()); i <= static_cast<int>(this->country->get_max_tier()); ++i) {
		const country_tier tier = static_cast<country_tier>(i);
		const country_tier_data *tier_data = country_tier_data::get(tier);

		if (tier_data->get_conditions() != nullptr && !tier_data->get_conditions()->check(this->country)) {
			continue;
		}

		best_tier = tier;
	}

	this->set_tier(best_tier);
}

const std::string &country_game_data::get_name() const
{
	return this->country->get_name(this->get_government_type(), this->get_tier());
}

std::string country_game_data::get_titled_name() const
{
	return this->country->get_titled_name(this->get_government_type(), this->get_tier(), this->get_religion());
}

const std::string &country_game_data::get_title_name() const
{
	return this->country->get_title_name(this->get_government_type(), this->get_tier(), this->get_religion());
}

const std::string &country_game_data::get_office_title_name(const office *office) const
{
	const character *office_holder = this->get_office_holder(office);
	const gender gender = office_holder != nullptr ? office_holder->get_gender() : gender::male;
	return this->country->get_office_title_name(office, this->get_government_type(), this->get_tier(), gender, this->get_religion());
}

void country_game_data::set_religion(const kobold::religion *religion)
{
	if (religion == this->get_religion()) {
		return;
	}

	this->religion = religion;

	for (const province *province : this->get_provinces()) {
		province->get_game_data()->set_religion(this->get_religion());
	}

	if (game::get()->is_running()) {
		emit religion_changed();
	}
}

void country_game_data::set_overlord(const kobold::country *overlord)
{
	if (overlord == this->get_overlord()) {
		return;
	}

	if (this->get_overlord() != nullptr) {
		for (const auto &[resource, count] : this->get_resource_counts()) {
			this->get_overlord()->get_game_data()->change_vassal_resource_count(resource, -count);
		}
	}

	this->overlord = overlord;

	if (this->get_overlord() != nullptr) {
		for (const auto &[resource, count] : this->get_resource_counts()) {
			this->get_overlord()->get_game_data()->change_vassal_resource_count(resource, count);
		}
	} else {
		this->set_subject_type(nullptr);
	}

	if (game::get()->is_running()) {
		emit overlord_changed();
	}
}

bool country_game_data::is_vassal_of(const kobold::country *country) const
{
	return this->get_overlord() == country;
}

bool country_game_data::is_any_vassal_of(const kobold::country *country) const
{
	if (this->is_vassal_of(country)) {
		return true;
	}

	if (this->get_overlord() != nullptr) {
		return this->get_overlord()->get_game_data()->is_any_vassal_of(country);
	}

	return false;
}

bool country_game_data::is_overlord_of(const kobold::country *country) const
{
	return country->get_game_data()->is_vassal_of(this->country);
}

bool country_game_data::is_any_overlord_of(const kobold::country *country) const
{
	if (this->is_overlord_of(country)) {
		return true;
	}

	const std::vector<const kobold::country *> vassals = this->get_vassals();
	for (const kobold::country *vassal : this->get_vassals()) {
		if (vassal->get_game_data()->is_any_overlord_of(country)) {
			return true;
		}
	}

	return false;
}

std::string country_game_data::get_type_name() const
{
	switch (this->country->get_type()) {
		case country_type::great_power:
			if (this->get_overlord() != nullptr) {
				return "Subject Power";
			}

			if (this->get_rank() != nullptr) {
				return this->get_rank()->get_name();
			}

			return "Great Power";
		case country_type::minor_nation:
		case country_type::tribe:
			return "Minor Nation";
		default:
			assert_throw(false);
	}

	return std::string();
}


void country_game_data::set_subject_type(const kobold::subject_type *subject_type)
{
	if (subject_type == this->get_subject_type()) {
		return;
	}

	this->subject_type = subject_type;

	if (game::get()->is_running()) {
		emit subject_type_changed();
	}

	this->check_government_type();
}

void country_game_data::change_level(const int change)
{
	if (change == 0) {
		return;
	}

	const int old_level = this->get_level();

	this->level += change;

	for (const auto &[skill, per_level_bonus] : this->get_skill_per_level_bonuses()) {
		this->change_skill_bonus(skill, change * per_level_bonus);
	}

	if (change > 0) {
		for (int i = 1; i <= change; ++i) {
			const int new_level = old_level + i;
			const effect_list<const kobold::country> *effects = defines::get()->get_country_level_effects(new_level);
			if (effects != nullptr) {
				context ctx(this->country);
				effects->do_effects(this->country, ctx);
			}
		}

		const int64_t level_experience = defines::get()->get_experience_for_level(this->get_level());
		if (this->get_experience() < level_experience) {
			this->set_experience(level_experience);
		}
	}

	if (game::get()->is_running()) {
		emit level_changed();
	}
}

void country_game_data::change_experience(const int64_t change)
{
	if (change == 0) {
		return;
	}

	this->experience += change;

	if (game::get()->is_running()) {
		emit experience_changed();
	}

	while (this->get_experience() >= defines::get()->get_experience_for_level(this->get_level() + 1)) {
		this->change_level(1);
	}
}

QVariantList country_game_data::get_attribute_values_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_attribute_values());
}

void country_game_data::change_attribute_value(const country_attribute *attribute, const int change)
{
	if (change == 0) {
		return;
	}

	const int new_value = (this->attribute_values[attribute] += change);
	if (new_value == 0) {
		this->attribute_values.erase(attribute);
	}

	this->change_score(change);

	if (game::get()->is_running()) {
		emit attribute_values_changed();
	}
}

int country_game_data::do_attribute_roll(const country_attribute *attribute) const
{
	return random::get()->roll_dice(dice(1, 20)) + this->get_attribute_value(attribute) - std::max(this->get_unrest(), 0);
}

QVariantList country_game_data::get_skill_bonuses_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_skill_bonuses());
}

void country_game_data::change_skill_bonus(const country_skill *skill, const int change)
{
	if (change == 0) {
		return;
	}

	const int new_value = (this->skill_bonuses[skill] += change);
	if (new_value == 0) {
		this->skill_bonuses.erase(skill);
	}

	if (game::get()->is_running()) {
		emit skill_bonuses_changed();
	}
}

void country_game_data::change_skill_per_level_bonus(const country_skill *skill, const int change)
{
	if (change == 0) {
		return;
	}

	const int new_value = (this->skill_per_level_bonuses[skill] += change);
	if (new_value == 0) {
		this->skill_per_level_bonuses.erase(skill);
	}

	this->change_skill_bonus(skill, change * this->get_level());
}

int country_game_data::get_skill_modifier(const country_skill *skill) const
{
	return this->get_skill_bonus(skill) + this->get_attribute_modifier(skill->get_attribute()) - std::max(this->get_unrest(), 0);
}

int country_game_data::do_skill_roll(const country_skill *skill) const
{
	return random::get()->roll_dice(dice(1, 20)) + this->get_skill_modifier(skill);
}

int country_game_data::get_control_difficulty_class() const
{
	return 20 + this->get_control_modifier();
}

int country_game_data::get_control_modifier() const
{
	return this->get_site_count(); //FIXME: larger settlements should contribute more to the control modifier
}

QVariantList country_game_data::get_provinces_qvariant_list() const
{
	return container::to_qvariant_list(this->get_provinces());
}

void country_game_data::add_province(const province *province)
{
	this->explore_province(province);

	this->provinces.push_back(province);

	this->on_province_gained(province, 1);

	map *map = map::get();
	const province_game_data *province_game_data = province->get_game_data();

	if (province_game_data->is_country_border_province()) {
		this->border_provinces.push_back(province);
	}

	for (const kobold::province *neighbor_province : province_game_data->get_neighbor_provinces()) {
		const kobold::province_game_data *neighbor_province_game_data = neighbor_province->get_game_data();
		if (neighbor_province_game_data->get_owner() != this->country) {
			continue;
		}

		//province ceased to be a country border province, remove it from the list
		if (vector::contains(this->get_border_provinces(), neighbor_province) && !neighbor_province_game_data->is_country_border_province()) {
			std::erase(this->border_provinces, neighbor_province);
		}

		for (const QPoint &tile_pos : neighbor_province_game_data->get_border_tiles()) {
			if (!map->is_tile_on_country_border(tile_pos)) {
				std::erase(this->border_tiles, tile_pos);
			}

			if (game::get()->is_running()) {
				map->calculate_tile_country_border_directions(tile_pos);
			}
		}
	}

	for (const QPoint &tile_pos : province_game_data->get_border_tiles()) {
		if (map->is_tile_on_country_border(tile_pos)) {
			this->border_tiles.push_back(tile_pos);
		}
	}

	this->calculate_territory_rect();

	if (this->get_provinces().size() == 1) {
		game::get()->add_country(this->country);
	}

	for (const kobold::country *country : game::get()->get_countries()) {
		if (country == this->country) {
			continue;
		}

		if (!country->get_game_data()->is_country_known(this->country) && country->get_game_data()->is_province_discovered(province)) {
			country->get_game_data()->add_known_country(this->country);
		}
	}

	if (this->get_capital() == nullptr) {
		this->choose_capital();
	}

	if (game::get()->is_running()) {
		emit provinces_changed();
	}
}

void country_game_data::remove_province(const province *province)
{
	std::erase(this->provinces, province);

	if (this->get_capital_province() == province) {
		this->choose_capital();
	}

	this->on_province_gained(province, -1);

	map *map = map::get();
	const province_game_data *province_game_data = province->get_game_data();

	for (const QPoint &tile_pos : province_game_data->get_border_tiles()) {
		std::erase(this->border_tiles, tile_pos);
	}

	std::erase(this->border_provinces, province);

	for (const kobold::province *neighbor_province : province_game_data->get_neighbor_provinces()) {
		const kobold::province_game_data *neighbor_province_game_data = neighbor_province->get_game_data();
		if (neighbor_province_game_data->get_owner() != this->country) {
			continue;
		}

		//province has become a country border province, add it to the list
		if (neighbor_province_game_data->is_country_border_province() && !vector::contains(this->get_border_provinces(), neighbor_province)) {
			this->border_provinces.push_back(neighbor_province);
		}

		for (const QPoint &tile_pos : neighbor_province_game_data->get_border_tiles()) {
			if (map->is_tile_on_country_border(tile_pos) && !vector::contains(this->get_border_tiles(), tile_pos)) {
				this->border_tiles.push_back(tile_pos);
			}

			if (game::get()->is_running()) {
				map->calculate_tile_country_border_directions(tile_pos);
			}
		}
	}

	this->calculate_territory_rect();

	//remove this as a known country for other countries, if they no longer have explored tiles in this country's territory
	for (const kobold::country *country : game::get()->get_countries()) {
		if (country == this->country) {
			continue;
		}

		if (!country->get_game_data()->is_country_known(this->country)) {
			continue;
		}

		if (!country->get_game_data()->is_province_discovered(province)) {
			//the other country didn't have this province discovered, so its removal from this country's territory couldn't have impacted its knowability to them anyway
			continue;
		}

		bool known_province = false;
		for (const kobold::province *loop_province : this->get_provinces()) {
			if (country->get_game_data()->is_province_discovered(loop_province)) {
				known_province = true;
				break;
			}
		}

		if (!known_province) {
			country->get_game_data()->remove_known_country(this->country);
		}
	}

	if (this->get_provinces().empty()) {
		game::get()->remove_country(this->country);
	}

	if (game::get()->is_running()) {
		emit provinces_changed();
	}
}

void country_game_data::on_province_gained(const province *province, const int multiplier)
{
	province_game_data *province_game_data = province->get_game_data();

	if (province_game_data->is_coastal()) {
		this->coastal_province_count += 1 * multiplier;
	}

	for (const auto &[resource, count] : province_game_data->get_resource_counts()) {
		this->change_resource_count(resource, count * multiplier);

		if (this->get_overlord() != nullptr) {
			this->get_overlord()->get_game_data()->change_vassal_resource_count(resource, count * multiplier);
		}
	}

	for (const auto &[terrain, count] : province_game_data->get_tile_terrain_counts()) {
		this->change_tile_terrain_count(terrain, count * multiplier);
	}

	for (const auto &[resource, commodity_map] : this->improved_resource_commodity_bonuses) {
		for (const auto &[commodity, value] : commodity_map) {
			province_game_data->change_improved_resource_commodity_bonus(resource, commodity, value * multiplier);
		}
	}

	for (const auto &[commodity, threshold_map] : this->commodity_bonuses_for_tile_thresholds) {
		for (const auto &[threshold, value] : threshold_map) {
			province_game_data->change_commodity_bonus_for_tile_threshold(commodity, threshold, value * multiplier);
		}
	}
}

void country_game_data::on_site_gained(const site *site, const int multiplier)
{
	if (site->get_type() == site_type::resource || site->get_type() == site_type::celestial_body || site->is_settlement()) {
		this->change_site_count(multiplier);
	}

	const site_game_data *site_game_data = site->get_game_data();
	if (site->is_settlement() && site_game_data->is_built()) {
		this->on_settlement_gained(site, multiplier);
	}
}

void country_game_data::on_settlement_gained(const site *settlement, const int multiplier)
{
	this->change_settlement_count(1 * multiplier);

	site_game_data *settlement_game_data = settlement->get_game_data();

	for (const auto &[attribute, value] : this->get_settlement_attribute_values()) {
		settlement_game_data->change_settlement_attribute_value(attribute, value * multiplier);
	}

	for (const auto &[commodity, bonus] : this->get_settlement_commodity_bonuses()) {
		settlement_game_data->change_base_commodity_output(commodity, bonus * multiplier);
	}

	for (const auto &[building, commodity_bonuses] : this->get_building_commodity_bonuses()) {
		if (!settlement_game_data->has_building(building)) {
			continue;
		}

		for (const auto &[commodity, bonus] : commodity_bonuses) {
			settlement_game_data->change_base_commodity_output(commodity, centesimal_int(bonus) * multiplier);
		}
	}

	for (const qunique_ptr<settlement_building_slot> &building_slot : settlement_game_data->get_building_slots()) {
		const building_type *building = building_slot->get_building();
		if (building != nullptr) {
			assert_throw(building->is_provincial());
			this->change_settlement_building_count(building, 1 * multiplier);
		}

		const wonder *wonder = building_slot->get_wonder();
		if (wonder != nullptr) {
			this->on_wonder_gained(wonder, multiplier);
		}
	}
}

void country_game_data::set_capital(const site *capital)
{
	if (capital == this->get_capital()) {
		return;
	}

	if (capital != nullptr) {
		assert_throw(capital->is_settlement());
		assert_throw(capital->get_game_data()->get_province()->get_game_data()->get_owner() == this->country);
		assert_throw(capital->get_game_data()->is_built());
	}

	const site *old_capital = this->get_capital();

	this->capital = capital;

	if (capital != nullptr) {
		capital->get_game_data()->calculate_commodity_outputs();
		capital->get_game_data()->check_building_conditions();
	}

	if (old_capital != nullptr) {
		old_capital->get_game_data()->calculate_commodity_outputs();
		old_capital->get_game_data()->check_building_conditions();
	}

	emit capital_changed();
}

void country_game_data::choose_capital()
{
	if (this->country->get_default_capital()->get_game_data()->get_owner() == this->country && this->country->get_default_capital()->get_game_data()->can_be_capital()) {
		this->set_capital(this->country->get_default_capital());
		return;
	}

	const site *best_capital = nullptr;

	for (const kobold::province *province : this->get_provinces()) {
		for (const site *settlement : province->get_game_data()->get_settlement_sites()) {
			const site_game_data *settlement_game_data = settlement->get_game_data();

			if (!settlement_game_data->can_be_capital()) {
				continue;
			}

			if (best_capital != nullptr) {
				if (best_capital->get_game_data()->is_provincial_capital() && !settlement_game_data->is_provincial_capital()) {
					continue;
				}

				if (best_capital->get_game_data()->get_settlement_type()->get_level() >= settlement_game_data->get_settlement_type()->get_level()) {
					continue;
				}
			}

			best_capital = settlement;
		}
	}

	this->set_capital(best_capital);
}

const province *country_game_data::get_capital_province() const
{
	if (this->get_capital() != nullptr) {
		return this->get_capital()->get_game_data()->get_province();
	}

	return nullptr;
}

void country_game_data::change_site_count(const int change)
{
	if (change == 0) {
		return;
	}

	this->site_count += change;

	if (game::get()->is_running()) {
		emit site_count_changed();
	}
}

void country_game_data::change_settlement_count(const int change)
{
	if (change == 0) {
		return;
	}

	const int old_settlement_count = this->get_settlement_count();

	this->settlement_count += change;

	for (const auto &[building, count] : this->settlement_building_counts) {
		if (building->get_country_modifier() != nullptr) {
			building->get_country_modifier()->apply(this->country, centesimal_int(-count) / old_settlement_count);
		}
	}

	if (this->get_settlement_count() != 0) {
		for (const auto &[building, count] : this->settlement_building_counts) {
			if (building->get_country_modifier() != nullptr) {
				//reapply the settlement building's country modifier with the updated province count
				building->get_country_modifier()->apply(this->country, centesimal_int(count) / this->get_settlement_count());
			}
		}
	}
}

void country_game_data::calculate_territory_rect()
{
	QRect territory_rect;
	this->contiguous_territory_rects.clear();

	for (const province *province : this->get_provinces()) {
		const province_game_data *province_game_data = province->get_game_data();

		if (territory_rect.isNull()) {
			territory_rect = province_game_data->get_territory_rect();
		} else {
			territory_rect = territory_rect.united(province_game_data->get_territory_rect());
		}

		this->contiguous_territory_rects.push_back(province_game_data->get_territory_rect());
	}

	bool changed = true;
	while (changed) {
		changed = false;

		for (size_t i = 0; i < this->contiguous_territory_rects.size(); ++i) {
			QRect &first_territory_rect = this->contiguous_territory_rects.at(i);

			for (size_t j = i + 1; j < this->contiguous_territory_rects.size();) {
				const QRect &second_territory_rect = this->contiguous_territory_rects.at(j);

				if (first_territory_rect.intersects(second_territory_rect) || rect::is_adjacent_to(first_territory_rect, second_territory_rect)) {
					first_territory_rect = first_territory_rect.united(second_territory_rect);
					this->contiguous_territory_rects.erase(this->contiguous_territory_rects.begin() + j);
					changed = true;
				} else {
					++j;
				}
			}
		}
	}

	this->territory_rect = territory_rect;

	const QPoint &capital_pos = this->get_capital() ? this->get_capital()->get_game_data()->get_tile_pos() : this->country->get_default_capital()->get_game_data()->get_tile_pos();
	int best_distance = std::numeric_limits<int>::max();
	for (const QRect &contiguous_territory_rect : this->get_contiguous_territory_rects()) {
		if (contiguous_territory_rect.contains(capital_pos)) {
			this->main_contiguous_territory_rect = contiguous_territory_rect;
			break;
		}

		int distance = rect::distance_to(contiguous_territory_rect, capital_pos);

		if (distance < best_distance) {
			best_distance = distance;
			this->main_contiguous_territory_rect = contiguous_territory_rect;
		}
	}

	this->calculate_territory_rect_center();
	this->calculate_text_rect();

	if (game::get()->is_running()) {
		this->country->get_turn_data()->set_diplomatic_map_dirty(true);
	}
}

void country_game_data::calculate_territory_rect_center()
{
	if (this->get_provinces().empty()) {
		return;
	}

	QPoint sum(0, 0);
	int tile_count = 0;

	for (const province *province : this->get_provinces()) {
		const province_map_data *province_map_data = province->get_map_data();
		if (!this->get_main_contiguous_territory_rect().contains(province_map_data->get_territory_rect())) {
			continue;
		}

		const int province_tile_count = static_cast<int>(province_map_data->get_tiles().size());
		sum += province_map_data->get_territory_rect_center() * province_tile_count;
		tile_count += province_tile_count;
	}

	this->territory_rect_center = QPoint(sum.x() / tile_count, sum.y() / tile_count);
}

QVariantList country_game_data::get_contiguous_territory_rects_qvariant_list() const
{
	return container::to_qvariant_list(this->get_contiguous_territory_rects());
}

void country_game_data::calculate_text_rect()
{
	this->text_rect = QRect();

	if (!this->is_alive()) {
		return;
	}

	QPoint center_pos = this->get_territory_rect_center();

	const map *map = map::get();

	if (map->get_tile(center_pos)->get_owner() != this->country && this->get_capital() != nullptr) {
		center_pos = this->get_capital()->get_game_data()->get_tile_pos();

		if (map->get_tile(center_pos)->get_owner() != this->country) {
			return;
		}
	}

	this->text_rect = QRect(center_pos, QSize(1, 1));

	bool changed = true;
	while (changed) {
		changed = false;

		bool can_expand_left = true;
		const int left_x = this->text_rect.left() - 1;
		for (int y = this->text_rect.top(); y <= this->text_rect.bottom(); ++y) {
			const QPoint adjacent_pos(left_x, y);

			if (!this->main_contiguous_territory_rect.contains(adjacent_pos)) {
				can_expand_left = false;
				break;
			}

			const kobold::tile *adjacent_tile = map->get_tile(adjacent_pos);

			if (adjacent_tile->get_owner() != this->country) {
				can_expand_left = false;
				break;
			}
		}
		if (can_expand_left) {
			this->text_rect.setLeft(left_x);
			changed = true;
		}

		bool can_expand_right = true;
		const int right_x = this->text_rect.right() + 1;
		for (int y = this->text_rect.top(); y <= this->text_rect.bottom(); ++y) {
			const QPoint adjacent_pos(right_x, y);

			if (!this->main_contiguous_territory_rect.contains(adjacent_pos)) {
				can_expand_right = false;
				break;
			}

			const kobold::tile *adjacent_tile = map->get_tile(adjacent_pos);

			if (adjacent_tile->get_owner() != this->country) {
				can_expand_right = false;
				break;
			}
		}
		if (can_expand_right) {
			this->text_rect.setRight(right_x);
			changed = true;
		}

		bool can_expand_up = true;
		const int up_y = this->text_rect.top() - 1;
		for (int x = this->text_rect.left(); x <= this->text_rect.right(); ++x) {
			const QPoint adjacent_pos(x, up_y);

			if (!this->main_contiguous_territory_rect.contains(adjacent_pos)) {
				can_expand_up = false;
				break;
			}

			const kobold::tile *adjacent_tile = map->get_tile(adjacent_pos);

			if (adjacent_tile->get_owner() != this->country) {
				can_expand_up = false;
				break;
			}
		}
		if (can_expand_up) {
			this->text_rect.setTop(up_y);
			changed = true;
		}

		bool can_expand_down = true;
		const int down_y = this->text_rect.bottom() + 1;
		for (int x = this->text_rect.left(); x <= this->text_rect.right(); ++x) {
			const QPoint adjacent_pos(x, down_y);

			if (!this->main_contiguous_territory_rect.contains(adjacent_pos)) {
				can_expand_down = false;
				break;
			}

			const kobold::tile *adjacent_tile = map->get_tile(adjacent_pos);

			if (adjacent_tile->get_owner() != this->country) {
				can_expand_down = false;
				break;
			}
		}
		if (can_expand_down) {
			this->text_rect.setBottom(down_y);
			changed = true;
		}
	}
}

QVariantList country_game_data::get_resource_counts_qvariant_list() const
{
	return archimedes::map::to_value_sorted_qvariant_list(this->get_resource_counts());
}

QVariantList country_game_data::get_vassal_resource_counts_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_vassal_resource_counts());
}

QVariantList country_game_data::get_tile_terrain_counts_qvariant_list() const
{
	QVariantList counts = archimedes::map::to_qvariant_list(this->get_tile_terrain_counts());
	std::sort(counts.begin(), counts.end(), [](const QVariant &lhs, const QVariant &rhs) {
		return lhs.toMap().value("value").toInt() > rhs.toMap().value("value").toInt();
	});
	return counts;
}

void country_game_data::add_known_country(const kobold::country *other_country)
{
	this->known_countries.insert(other_country);

	const consulate *current_consulate = this->get_consulate(other_country);

	const consulate *best_free_consulate = nullptr;
	for (const auto &[consulate, count] : this->free_consulate_counts) {
		if (best_free_consulate == nullptr || consulate->get_level() > best_free_consulate->get_level()) {
			best_free_consulate = consulate;
		}
	}

	if (best_free_consulate != nullptr && (current_consulate == nullptr || current_consulate->get_level() < best_free_consulate->get_level())) {
		this->set_consulate(other_country, best_free_consulate);
	}
}

diplomacy_state country_game_data::get_diplomacy_state(const kobold::country *other_country) const
{
	const auto find_iterator = this->diplomacy_states.find(other_country);

	if (find_iterator != this->diplomacy_states.end()) {
		return find_iterator->second;
	}

	return diplomacy_state::peace;
}

void country_game_data::set_diplomacy_state(const kobold::country *other_country, const diplomacy_state state)
{
	const diplomacy_state old_state = this->get_diplomacy_state(other_country);

	if (state == old_state) {
		return;
	}

	//minor nations cannot have diplomacy with one another
	assert_throw(this->country->is_great_power() || other_country->is_great_power());

	switch (state) {
		case diplomacy_state::alliance:
			//only great powers can ally between themselves
			assert_throw(this->country->is_great_power() && other_country->is_great_power());
			break;
		case diplomacy_state::non_aggression_pact:
			//non-aggression pacts can only be formed between a great power and a minor nation
			assert_throw(this->country->is_great_power() != other_country->is_great_power());
			break;
		default:
			break;
	}

	if (is_vassalage_diplomacy_state(state)) {
		//only great powers can have vassals
		assert_throw(other_country->is_great_power());

		this->set_overlord(other_country);
	} else {
		if (this->get_overlord() == other_country) {
			this->set_overlord(nullptr);
		}
	}

	if (old_state != diplomacy_state::peace) {
		this->change_diplomacy_state_count(old_state, -1);
	}

	if (state == diplomacy_state::peace) {
		this->diplomacy_states.erase(other_country);
	} else {
		this->diplomacy_states[other_country] = state;
		this->change_diplomacy_state_count(state, 1);
	}

	if (game::get()->is_running()) {
		emit diplomacy_states_changed();

		if (is_vassalage_diplomacy_state(state) || is_vassalage_diplomacy_state(old_state)) {
			emit type_name_changed();
		}
	}
}

void country_game_data::change_diplomacy_state_count(const diplomacy_state state, const int change)
{
	const int final_count = (this->diplomacy_state_counts[state] += change);

	if (final_count == 0) {
		this->diplomacy_state_counts.erase(state);
		this->diplomacy_state_diplomatic_map_images.erase(state);
	}

	//if the change added the diplomacy state to the map, then we need to create the diplomatic map image for it
	if (game::get()->is_running() && final_count == change && !is_vassalage_diplomacy_state(state) && !is_overlordship_diplomacy_state(state)) {
		this->country->get_turn_data()->set_diplomatic_map_diplomacy_state_dirty(state);
	}
}

QString country_game_data::get_diplomacy_state_diplomatic_map_suffix(kobold::country *other_country) const
{
	if (other_country == this->country || this->is_any_overlord_of(other_country) || this->is_any_vassal_of(other_country)) {
		return "empire";
	}

	return QString::fromStdString(enum_converter<diplomacy_state>::to_string(this->get_diplomacy_state(other_country)));
}

bool country_game_data::at_war() const
{
	return this->diplomacy_state_counts.contains(diplomacy_state::war);
}

bool country_game_data::can_attack(const kobold::country *other_country) const
{
	if (other_country == nullptr) {
		return false;
	}

	if (other_country == this->country) {
		return false;
	}

	if (this->is_any_overlord_of(other_country)) {
		return false;
	}

	switch (this->get_diplomacy_state(other_country)) {
		case diplomacy_state::non_aggression_pact:
		case diplomacy_state::alliance:
			return false;
		case diplomacy_state::war:
			return true;
		default:
			break;
	}

	if (other_country->get_game_data()->is_tribal() || this->is_tribal()) {
		return true;
	}

	if (other_country->get_game_data()->is_under_anarchy() || this->is_under_anarchy()) {
		return true;
	}

	return false;
}

std::optional<diplomacy_state> country_game_data::get_offered_diplomacy_state(const kobold::country *other_country) const
{
	const auto find_iterator = this->offered_diplomacy_states.find(other_country);

	if (find_iterator != this->offered_diplomacy_states.end()) {
		return find_iterator->second;
	}

	return std::nullopt;
}

void country_game_data::set_offered_diplomacy_state(const kobold::country *other_country, const std::optional<diplomacy_state> &state)
{
	const diplomacy_state old_state = this->get_diplomacy_state(other_country);

	if (state == old_state) {
		return;
	}

	if (state.has_value()) {
		this->offered_diplomacy_states[other_country] = state.value();
	} else {
		this->offered_diplomacy_states.erase(other_country);
	}

	if (game::get()->is_running()) {
		emit offered_diplomacy_states_changed();
	}
}

QVariantList country_game_data::get_consulates_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->consulates);
}

void country_game_data::set_consulate(const kobold::country *other_country, const consulate *consulate)
{
	if (consulate == nullptr) {
		this->consulates.erase(other_country);
	} else {
		this->consulates[other_country] = consulate;

		if (other_country->get_game_data()->get_consulate(this->country) != consulate) {
			other_country->get_game_data()->set_consulate(this->country, consulate);
		}
	}

	if (game::get()->is_running()) {
		emit consulates_changed();
	}
}

int country_game_data::get_opinion_of(const kobold::country *other) const
{
	int opinion = this->get_base_opinion(other);

	for (const auto &[modifier, duration] : this->get_opinion_modifiers_for(other)) {
		opinion += modifier->get_value();
	}

	opinion = std::clamp(opinion, country::min_opinion, country::max_opinion);

	return opinion;
}

void country_game_data::set_base_opinion(const kobold::country *other, const int opinion)
{
	assert_throw(other != this->country);

	if (opinion == this->get_base_opinion(other)) {
		return;
	}

	if (opinion < country::min_opinion) {
		this->set_base_opinion(other, country::min_opinion);
		return;
	} else if (opinion > country::max_opinion) {
		this->set_base_opinion(other, country::max_opinion);
		return;
	}

	if (opinion == 0) {
		this->base_opinions.erase(other);
	} else {
		this->base_opinions[other] = opinion;
	}
}

void country_game_data::add_opinion_modifier(const kobold::country *other, const opinion_modifier *modifier, const int duration)
{
	this->opinion_modifiers[other][modifier] = std::max(this->opinion_modifiers[other][modifier], duration);
}

void country_game_data::remove_opinion_modifier(const kobold::country *other, const opinion_modifier *modifier)
{
	opinion_modifier_map<int> &opinion_modifiers = this->opinion_modifiers[other];
	opinion_modifiers.erase(modifier);

	if (opinion_modifiers.empty()) {
		this->opinion_modifiers.erase(other);
	}
}

int country_game_data::get_opinion_weighted_prestige_for(const kobold::country *other) const
{
	const int opinion = this->get_opinion_of(other);
	const int prestige = std::max(1, other->get_game_data()->get_stored_commodity(defines::get()->get_prestige_commodity()));

	const int opinion_weighted_prestige = prestige * (opinion - country::min_opinion) / (country::max_opinion - country::min_opinion);
	return opinion_weighted_prestige;
}

std::vector<const kobold::country *> country_game_data::get_vassals() const
{
	std::vector<const kobold::country *> vassals;

	for (const auto &[country, diplomacy_state] : this->diplomacy_states) {
		if (is_overlordship_diplomacy_state(diplomacy_state)) {
			vassals.push_back(country);
		}
	}

	return vassals;
}

QVariantList country_game_data::get_vassals_qvariant_list() const
{
	return container::to_qvariant_list(this->get_vassals());
}

QVariantList country_game_data::get_subject_type_counts_qvariant_list() const
{
	std::map<const kobold::subject_type *, int> subject_type_counts;

	for (const auto &[country, diplomacy_state] : this->diplomacy_states) {
		if (is_overlordship_diplomacy_state(diplomacy_state)) {
			assert_throw(country->get_game_data()->get_subject_type() != nullptr);
			++subject_type_counts[country->get_game_data()->get_subject_type()];
		}
	}

	QVariantList counts = archimedes::map::to_qvariant_list(subject_type_counts);
	std::sort(counts.begin(), counts.end(), [](const QVariant &lhs, const QVariant &rhs) {
		return lhs.toMap().value("value").toInt() > rhs.toMap().value("value").toInt();
	});

	return counts;
}

std::vector<const kobold::country *> country_game_data::get_neighbor_countries() const
{
	std::vector<const kobold::country *> neighbor_countries;

	for (const province *province : this->get_border_provinces()) {
		for (const kobold::province *neighbor_province : province->get_game_data()->get_neighbor_provinces()) {
			const kobold::province_game_data *neighbor_province_game_data = neighbor_province->get_game_data();
			if (neighbor_province_game_data->get_owner() == this->country) {
				continue;
			}

			if (neighbor_province_game_data->get_owner() == nullptr) {
				continue;
			}

			if (vector::contains(neighbor_countries, neighbor_province_game_data->get_owner())) {
				continue;
			}

			neighbor_countries.push_back(neighbor_province_game_data->get_owner());
		}
	}

	return neighbor_countries;
}

const QColor &country_game_data::get_diplomatic_map_color() const
{
	if (this->get_overlord() != nullptr) {
		return this->get_overlord()->get_game_data()->get_diplomatic_map_color();
	}

	return this->country->get_color();
}

QImage country_game_data::prepare_diplomatic_map_image() const
{
	assert_throw(this->territory_rect.width() > 0);
	assert_throw(this->territory_rect.height() > 0);

	QImage image(this->territory_rect.size(), QImage::Format_RGBA8888);
	image.fill(Qt::transparent);

	return image;
}

QCoro::Task<QImage> country_game_data::finalize_diplomatic_map_image(QImage &&image) const
{
	QImage scaled_image;

	const int tile_pixel_size = map::get()->get_diplomatic_map_tile_pixel_size();

	co_await QtConcurrent::run([tile_pixel_size, &image, &scaled_image]() {
		scaled_image = image::scale<QImage::Format_ARGB32>(image, centesimal_int(tile_pixel_size), [](const size_t factor, const uint32_t *src, uint32_t *tgt, const int src_width, const int src_height) {
			xbrz::scale(factor, src, tgt, src_width, src_height, xbrz::ColorFormat::ARGB);
		});
	});

	image = std::move(scaled_image);

	std::vector<QPoint> border_pixels;

	for (int x = 0; x < image.width(); ++x) {
		for (int y = 0; y < image.height(); ++y) {
			const QPoint pixel_pos(x, y);
			const QColor pixel_color = image.pixelColor(pixel_pos);

			if (pixel_color.alpha() == 0) {
				continue;
			}

			if (pixel_pos.x() == 0 || pixel_pos.y() == 0 || pixel_pos.x() == (image.width() - 1) || pixel_pos.y() == (image.height() - 1)) {
				border_pixels.push_back(pixel_pos);
				continue;
			}

			if (pixel_color.alpha() != 255) {
				//blended color
				border_pixels.push_back(pixel_pos);
				continue;
			}

			bool is_border_pixel = false;
			point::for_each_cardinally_adjacent_until(pixel_pos, [&image, &is_border_pixel](const QPoint &adjacent_pos) {
				if (image.pixelColor(adjacent_pos).alpha() != 0) {
					return false;
				}

				is_border_pixel = true;
				return true;
			});

			if (is_border_pixel) {
				border_pixels.push_back(pixel_pos);
			}
		}
	}

	const QColor &border_pixel_color = defines::get()->get_country_border_color();

	for (const QPoint &border_pixel_pos : border_pixels) {
		image.setPixelColor(border_pixel_pos, border_pixel_color);
	}

	const centesimal_int &scale_factor = preferences::get()->get_scale_factor();

	co_await QtConcurrent::run([&scale_factor, &image, &scaled_image]() {
		scaled_image = image::scale<QImage::Format_ARGB32>(image, scale_factor, [](const size_t factor, const uint32_t *src, uint32_t *tgt, const int src_width, const int src_height) {
			xbrz::scale(factor, src, tgt, src_width, src_height, xbrz::ColorFormat::ARGB);
		});
	});

	co_return scaled_image;
}

QCoro::Task<void> country_game_data::create_diplomatic_map_image()
{
	const map *map = map::get();

	QImage diplomatic_map_image = this->prepare_diplomatic_map_image();
	QImage selected_diplomatic_map_image = diplomatic_map_image;

	const QColor &color = this->get_diplomatic_map_color();
	const QColor &selected_color = defines::get()->get_selected_country_color();

	for (int x = 0; x < this->territory_rect.width(); ++x) {
		for (int y = 0; y < this->territory_rect.height(); ++y) {
			const QPoint relative_tile_pos = QPoint(x, y);
			const tile *tile = map->get_tile(this->territory_rect.topLeft() + relative_tile_pos);

			if (tile->get_owner() != this->country) {
				continue;
			}

			diplomatic_map_image.setPixelColor(relative_tile_pos, color);
			selected_diplomatic_map_image.setPixelColor(relative_tile_pos, selected_color);
		}
	}

	this->diplomatic_map_image = co_await this->finalize_diplomatic_map_image(std::move(diplomatic_map_image));
	this->selected_diplomatic_map_image = co_await this->finalize_diplomatic_map_image(std::move(selected_diplomatic_map_image));

	const int tile_pixel_size = map->get_diplomatic_map_tile_pixel_size();
	this->diplomatic_map_image_rect = QRect(this->territory_rect.topLeft() * tile_pixel_size * preferences::get()->get_scale_factor(), this->diplomatic_map_image.size());

	co_await this->create_diplomatic_map_mode_image(diplomatic_map_mode::diplomatic);
	co_await this->create_diplomacy_state_diplomatic_map_image(diplomacy_state::peace);

	for (const auto &[diplomacy_state, count] : this->get_diplomacy_state_counts()) {
		if (!is_vassalage_diplomacy_state(diplomacy_state) && !is_overlordship_diplomacy_state(diplomacy_state)) {
			co_await this->create_diplomacy_state_diplomatic_map_image(diplomacy_state);
		}
	}

	co_await this->create_diplomatic_map_mode_image(diplomatic_map_mode::terrain);
	co_await this->create_diplomatic_map_mode_image(diplomatic_map_mode::cultural);
	co_await this->create_diplomatic_map_mode_image(diplomatic_map_mode::religious);

	emit diplomatic_map_image_changed();
}

QCoro::Task<void> country_game_data::create_diplomatic_map_mode_image(const diplomatic_map_mode mode)
{
	static const QColor empty_color(Qt::black);
	static constexpr QColor diplomatic_self_color(170, 148, 214);

	const map *map = map::get();

	QImage image = this->prepare_diplomatic_map_image();

	for (int x = 0; x < this->territory_rect.width(); ++x) {
		for (int y = 0; y < this->territory_rect.height(); ++y) {
			const QPoint relative_tile_pos = QPoint(x, y);
			const tile *tile = map->get_tile(this->territory_rect.topLeft() + relative_tile_pos);

			if (tile->get_owner() != this->country) {
				continue;
			}

			const QColor *color = nullptr;

			switch (mode) {
				case diplomatic_map_mode::diplomatic:
					color = &diplomatic_self_color;
					break;
				case diplomatic_map_mode::terrain:
					color = &tile->get_terrain()->get_color();
					break;
				case diplomatic_map_mode::cultural: {
					const culture *culture = nullptr;

					if (tile->get_settlement() != nullptr && tile->get_settlement()->get_game_data()->get_culture() != nullptr) {
						culture = tile->get_settlement()->get_game_data()->get_culture();
					} else {
						culture = tile->get_province()->get_game_data()->get_culture();
					}

					if (culture != nullptr) {
						color = &culture->get_color();
					} else {
						color = &empty_color;
					}
					break;
				}
				case diplomatic_map_mode::religious: {
					const kobold::religion *religion = nullptr;

					if (tile->get_settlement() != nullptr && tile->get_settlement()->get_game_data()->get_religion() != nullptr) {
						religion = tile->get_settlement()->get_game_data()->get_religion();
					} else {
						religion = tile->get_province()->get_game_data()->get_religion();
					}

					if (religion != nullptr) {
						color = &religion->get_color();
					} else {
						color = &empty_color;
					}
					break;
				}
			}

			image.setPixelColor(relative_tile_pos, *color);
		}
	}

	this->diplomatic_map_mode_images[mode] = co_await this->finalize_diplomatic_map_image(std::move(image));
}

QCoro::Task<void> country_game_data::create_diplomacy_state_diplomatic_map_image(const diplomacy_state state)
{
	static const QColor empty_color(Qt::black);

	const map *map = map::get();

	QImage image = this->prepare_diplomatic_map_image();

	for (int x = 0; x < this->territory_rect.width(); ++x) {
		for (int y = 0; y < this->territory_rect.height(); ++y) {
			const QPoint relative_tile_pos = QPoint(x, y);
			const tile *tile = map->get_tile(this->territory_rect.topLeft() + relative_tile_pos);

			if (tile->get_owner() != this->country) {
				continue;
			}

			const QColor &color = defines::get()->get_diplomacy_state_color(state);

			image.setPixelColor(relative_tile_pos, color);
		}
	}

	this->diplomacy_state_diplomatic_map_images[state] = co_await this->finalize_diplomatic_map_image(std::move(image));
}

void country_game_data::change_score(const int change)
{
	if (change == 0) {
		return;
	}

	this->score += change;

	emit score_changed();
}

bool country_game_data::has_building(const building_type *building) const
{
	return this->get_settlement_building_count(building) > 0;
}

void country_game_data::change_settlement_building_count(const building_type *building, const int change)
{
	if (change == 0) {
		return;
	}

	const int count = (this->settlement_building_counts[building] += change);

	if (count < 0) {
		throw std::runtime_error(std::format("The settlement building count for country \"{}\" for building \"{}\" is negative ({}).", this->country->get_identifier(), building->get_identifier(), change));
	}

	if (count == 0) {
		this->settlement_building_counts.erase(building);
	}

	if (building->get_country_modifier() != nullptr) {
		building->get_country_modifier()->apply(this->country, change);
	}

	if (game::get()->is_running()) {
		emit settlement_building_counts_changed();
	}
}

void country_game_data::on_wonder_gained(const wonder *wonder, const int multiplier)
{
	if (wonder->get_country_modifier() != nullptr) {
		wonder->get_country_modifier()->apply(this->country, multiplier);
	}

	if (multiplier > 0) {
		game::get()->set_wonder_country(wonder, this->country);
	} else if (multiplier < 0 && game::get()->get_wonder_country(wonder) == this->country) {
		game::get()->set_wonder_country(wonder, nullptr);
	}
}

int country_game_data::get_wealth() const
{
	return this->get_stored_commodity(defines::get()->get_wealth_commodity());
}

void country_game_data::set_wealth(const int wealth)
{
	return this->set_stored_commodity(defines::get()->get_wealth_commodity(), wealth);
}

void country_game_data::add_taxable_wealth(const int taxable_wealth, const income_transaction_type tax_income_type)
{
	assert_throw(taxable_wealth >= 0);
	assert_throw(tax_income_type == income_transaction_type::tariff || tax_income_type == income_transaction_type::treasure_fleet);

	if (taxable_wealth == 0) {
		return;
	}

	if (this->get_overlord() == nullptr) {
		this->change_wealth(taxable_wealth);
		return;
	}

	const int tax = taxable_wealth * country_game_data::vassal_tax_rate / 100;
	const int taxed_wealth = taxable_wealth - tax;

	this->get_overlord()->get_game_data()->add_taxable_wealth(tax, tax_income_type);

	this->change_wealth(taxed_wealth);

	if (tax != 0) {
		this->get_overlord()->get_turn_data()->add_income_transaction(tax_income_type, tax, nullptr, 0, this->country);
		this->country->get_turn_data()->add_expense_transaction(expense_transaction_type::tax, tax, nullptr, 0, this->get_overlord());
	}
}

QVariantList country_game_data::get_available_commodities_qvariant_list() const
{
	return container::to_qvariant_list(this->get_available_commodities());
}

QVariantList country_game_data::get_tradeable_commodities_qvariant_list() const
{
	std::vector<const commodity *> tradeable_commodities = container::to_vector(this->get_tradeable_commodities());

	std::sort(tradeable_commodities.begin(), tradeable_commodities.end(), [](const commodity *lhs, const commodity *rhs) {
		if (lhs->get_base_price() != rhs->get_base_price()) {
			return lhs->get_base_price() > rhs->get_base_price();
		}

		return lhs->get_identifier() < rhs->get_identifier();
	});

	return container::to_qvariant_list(tradeable_commodities);
}

QVariantList country_game_data::get_stored_commodities_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_stored_commodities());
}

void country_game_data::set_stored_commodity(const commodity *commodity, const int value)
{
	if (value == this->get_stored_commodity(commodity)) {
		return;
	}

	if (value < 0 && !commodity->is_negative_allowed()) {
		throw std::runtime_error("Tried to set the storage of commodity \"" + commodity->get_identifier() + "\" for country \"" + this->country->get_identifier() + "\" to a negative number.");
	}

	if (commodity->is_convertible_to_wealth()) {
		assert_throw(value > 0);
		const int wealth_conversion_income = commodity->get_wealth_value() * value;
		this->add_taxable_wealth(wealth_conversion_income, income_transaction_type::treasure_fleet);
		this->country->get_turn_data()->add_income_transaction(income_transaction_type::liquidated_riches, wealth_conversion_income, commodity, value);
		return;
	}

	if (value > this->get_storage_capacity() && !commodity->is_abstract()) {
		this->set_stored_commodity(commodity, this->get_storage_capacity());
		return;
	}

	if (value <= 0) {
		this->stored_commodities.erase(commodity);
	} else {
		this->stored_commodities[commodity] = value;
	}

	if (this->get_offer(commodity) > value) {
		this->set_offer(commodity, value);
	}

	if (game::get()->is_running()) {
		emit stored_commodities_changed();
	}
}

void country_game_data::set_storage_capacity(const int capacity)
{
	if (capacity == this->get_storage_capacity()) {
		return;
	}

	this->storage_capacity = capacity;

	if (game::get()->is_running()) {
		emit storage_capacity_changed();
	}
}

QVariantList country_game_data::get_commodity_inputs_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_commodity_inputs());
}

int country_game_data::get_commodity_input(const QString &commodity_identifier) const
{
	return this->get_commodity_input(commodity::get(commodity_identifier.toStdString()));
}

void country_game_data::change_commodity_input(const commodity *commodity, const int change)
{
	if (change == 0) {
		return;
	}

	const int count = (this->commodity_inputs[commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->commodity_inputs.erase(commodity);
	}

	if (game::get()->is_running()) {
		emit commodity_inputs_changed();
	}
}

QVariantList country_game_data::get_commodity_outputs_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_commodity_outputs());
}

int country_game_data::get_commodity_output(const QString &commodity_identifier) const
{
	return this->get_commodity_output(commodity::get(commodity_identifier.toStdString())).to_int();
}

void country_game_data::change_commodity_output(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int old_output = this->get_commodity_output(commodity);

	const centesimal_int &new_output = (this->commodity_outputs[commodity] += change);

	assert_throw(new_output >= 0);

	if (new_output == 0) {
		this->commodity_outputs.erase(commodity);
	}

	if (game::get()->is_running()) {
		emit commodity_outputs_changed();
	}
}

void country_game_data::calculate_site_commodity_outputs()
{
	for (const province *province : this->get_provinces()) {
		province->get_game_data()->calculate_site_commodity_outputs();
	}
}

void country_game_data::calculate_site_commodity_output(const commodity *commodity)
{
	for (const province *province : this->get_provinces()) {
		province->get_game_data()->calculate_site_commodity_output(commodity);
	}
}

QVariantList country_game_data::get_everyday_consumption_qvariant_list() const
{
	commodity_map<int> int_everyday_consumption;

	for (const auto &[commodity, consumption] : this->get_everyday_consumption()) {
		int_everyday_consumption[commodity] = consumption.to_int();
	}

	return archimedes::map::to_qvariant_list(int_everyday_consumption);
}

int country_game_data::get_everyday_consumption(const QString &commodity_identifier) const
{
	return this->get_everyday_consumption(commodity::get(commodity_identifier.toStdString())).to_int();
}

void country_game_data::change_everyday_consumption(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int count = (this->everyday_consumption[commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->everyday_consumption.erase(commodity);
	}

	if (game::get()->is_running()) {
		emit everyday_consumption_changed();
	}
}

QVariantList country_game_data::get_luxury_consumption_qvariant_list() const
{
	commodity_map<int> int_luxury_consumption;

	for (const auto &[commodity, consumption] : this->get_luxury_consumption()) {
		int_luxury_consumption[commodity] = consumption.to_int();
	}

	return archimedes::map::to_qvariant_list(int_luxury_consumption);
}

int country_game_data::get_luxury_consumption(const QString &commodity_identifier) const
{
	return this->get_luxury_consumption(commodity::get(commodity_identifier.toStdString())).to_int();
}

void country_game_data::change_luxury_consumption(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int count = (this->luxury_consumption[commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->luxury_consumption.erase(commodity);
	}

	if (game::get()->is_running()) {
		emit luxury_consumption_changed();
	}
}

void country_game_data::change_commodity_demand(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int count = (this->commodity_demands[commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->commodity_demands.erase(commodity);
	}
}

bool country_game_data::produces_commodity(const commodity *commodity) const
{
	if (this->get_commodity_output(commodity).to_int() > 0) {
		return true;
	}

	for (const province *province : this->get_provinces()) {
		if (province->get_game_data()->produces_commodity(commodity)) {
			return true;
		}
	}

	return false;
}

bool country_game_data::can_declare_war_on(const kobold::country *other_country) const
{
	if (!this->country->can_declare_war()) {
		return false;
	}

	if (this->get_overlord() != nullptr) {
		return other_country == this->get_overlord();
	}

	return true;
}

void country_game_data::set_government_type(const kobold::government_type *government_type)
{
	if (government_type == this->get_government_type()) {
		return;
	}

	if (government_type != nullptr) {
		if (this->country->is_tribe() && !government_type->get_group()->is_tribal()) {
			throw std::runtime_error(std::format("Tried to set a non-tribal government type (\"{}\") for a tribal country (\"{}\").", government_type->get_identifier(), this->country->get_identifier()));
		}
	}

	const kobold::government_type *old_government_type = this->get_government_type();
	if (old_government_type != nullptr && old_government_type->get_modifier() != nullptr) {
		old_government_type->get_modifier()->apply(this->country, -1);
	}

	this->government_type = government_type;

	if (this->get_government_type() != nullptr) {
		if (this->get_government_type()->get_modifier() != nullptr) {
			this->get_government_type()->get_modifier()->apply(this->country, 1);
		}

		//apply the government type effects for the initial government type for the country
		if (this->get_government_type()->get_effects() != nullptr && old_government_type == nullptr) {
			context ctx(this->country);
			this->get_government_type()->get_effects()->do_effects(this->country, ctx);
		}
	}

	if (game::get()->is_running()) {
		emit government_type_changed();
	}
}

bool country_game_data::can_have_government_type(const kobold::government_type *government_type) const
{
	if (government_type->get_conditions() != nullptr && !government_type->get_conditions()->check(this->country, read_only_context(this->country))) {
		return false;
	}

	return true;
}

void country_game_data::check_government_type()
{
	if (this->get_government_type() != nullptr && this->can_have_government_type(this->get_government_type())) {
		return;
	}

	std::vector<const kobold::government_type *> potential_government_types;

	for (const kobold::government_type *government_type : government_type::get_all()) {
		if (this->can_have_government_type(government_type)) {
			potential_government_types.push_back(government_type);
		}
	}

	assert_throw(!potential_government_types.empty());

	this->set_government_type(vector::get_random(potential_government_types));
}

bool country_game_data::is_tribal() const
{
	return this->get_government_type()->get_group()->is_tribal();
}

QVariantList country_game_data::get_feats_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_feat_counts());
}

data_entry_map<country_feat, int> country_game_data::get_feat_counts_of_type(const country_feat_type *feat_type) const
{
	data_entry_map<country_feat, int> feat_counts;

	for (const auto &[feat, count] : this->get_feat_counts()) {
		if (!vector::contains(feat->get_types(), feat_type)) {
			continue;
		}

		feat_counts[feat] = count;
	}

	return feat_counts;
}

QVariantList country_game_data::get_feats_of_type(const QString &feat_type_str) const
{
	const country_feat_type *type = country_feat_type::get(feat_type_str.toStdString());
	return archimedes::map::to_qvariant_list(this->get_feat_counts_of_type(type));
}

int country_game_data::get_feat_count_for_type(const country_feat_type *feat_type) const
{
	int total_count = 0;

	for (const auto &[feat, count] : this->get_feat_counts_of_type(feat_type)) {
		total_count += count;
	}

	return total_count;
}

bool country_game_data::can_have_feat(const country_feat *feat) const
{
	if (feat->get_conditions() != nullptr && !feat->get_conditions()->check(this->country, read_only_context(this->country))) {
		return false;
	}

	return true;
}

bool country_game_data::can_gain_feat(const country_feat *feat, const country_feat_type *choice_type) const
{
	if (this->has_feat(feat) && !feat->is_unlimited()) {
		return false;
	}

	if (feat->get_upgraded_feat() != nullptr && !this->has_feat(feat->get_upgraded_feat())) {
		return false;
	}

	for (const country_feat_type *type : feat->get_types()) {
		if (choice_type != nullptr && choice_type->ignores_other_type_conditions() && type != choice_type) {
			continue;
		}

		if (type->get_max_feats() > 0 && this->get_feat_count_for_type(type) >= type->get_max_feats()) {
			return false;
		}

		if (type->get_gain_conditions() != nullptr && !type->get_gain_conditions()->check(this->country, read_only_context(this->country))) {
			return false;
		}
	}

	return this->can_have_feat(feat);
}

bool country_game_data::has_feat(const country_feat *feat) const
{
	return this->get_feat_counts().contains(feat);
}

void country_game_data::change_feat_count(const country_feat *feat, const int change)
{
	if (change == 0) {
		return;
	}

	if (change > 0) {
		if (this->has_feat(feat) && !feat->is_unlimited()) {
			throw std::runtime_error(std::format("Tried to add non-unlimited feat \"{}\" to country \"{}\", but it already has the feat.", feat->get_identifier(), this->country->get_identifier()));
		}

		if (!this->can_have_feat(feat)) {
			throw std::runtime_error(std::format("Tried to add feat \"{}\" to country \"{}\", for which the feat's conditions are not fulfilled.", feat->get_identifier(), this->country->get_identifier()));
		}
	}

	const int new_value = (this->feat_counts[feat] += change);
	if (new_value == 0) {
		this->feat_counts.erase(feat);
	}

	assert_throw(std::abs(change) == 1);
	this->on_feat_gained(feat, change);

	if (game::get()->is_running()) {
		emit feats_changed();
	}
}

void country_game_data::on_feat_gained(const country_feat *feat, const int multiplier)
{
	assert_throw(multiplier != 0);

	if (multiplier > 0) {
		if (feat->get_upgraded_feat() != nullptr) {
			this->change_feat_count(feat->get_upgraded_feat(), -1);
		}

		if (feat->get_effects() != nullptr) {
			context ctx(this->country);
			feat->get_effects()->do_effects(this->country, ctx);
		}

		for (const country_feat_type *type : feat->get_types()) {
			if (type->get_effects() != nullptr) {
				context ctx(this->country);
				type->get_effects()->do_effects(this->country, ctx);
			}
		}
	}

	if (feat->get_modifier() != nullptr) {
		feat->get_modifier()->apply(this->country, multiplier);
	}
}

void country_game_data::choose_feat(const country_feat_type *type)
{
	std::vector<const country_feat *> potential_feats = this->get_potential_feats_from_list(vector::intersected(this->target_feats, type->get_feats()), type);

	if (potential_feats.empty()) {
		potential_feats = this->get_potential_feats_from_list(type->get_feats(), type);
	}

	assert_throw(!potential_feats.empty());

	const country_feat *chosen_feat = vector::get_random(potential_feats);

	if (vector::contains(target_feats, chosen_feat)) {
		vector::remove_one(target_feats, chosen_feat);
	}

	this->change_feat_count(chosen_feat, 1);
}

std::vector<const country_feat *> country_game_data::get_potential_feats_from_list(const std::vector<const country_feat *> &feats, const country_feat_type *type) const
{
	std::vector<const country_feat *> potential_feats;
	bool found_unacquired_feat = false;

	for (const country_feat *feat : feats) {
		if (!this->can_gain_feat(feat, type)) {
			continue;
		}

		if (type->prioritizes_unacquired_feats()) {
			if (!found_unacquired_feat && !this->has_feat(feat)) {
				potential_feats.clear();
				found_unacquired_feat = true;
			} else if (found_unacquired_feat && this->has_feat(feat)) {
				continue;
			}
		}

		int weight = feat->get_weight_factor() != nullptr ? feat->get_weight_factor()->calculate(this->country).to_int() : 1;

		for (int i = 0; i < weight; ++i) {
			potential_feats.push_back(feat);
		}
	}

	return potential_feats;
}

QVariantList country_game_data::get_laws_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_laws());
}

void country_game_data::set_law(const law_group *law_group, const law *law)
{
	if (law == this->get_law(law_group)) {
		return;
	}

	const kobold::law *old_law = this->get_law(law_group);
	if (old_law != nullptr) {
		old_law->get_modifier()->remove(this->country);
	}

	this->laws[law_group] = law;

	if (law != nullptr) {
		assert_throw(law->get_group() == law_group);

		law->get_modifier()->apply(this->country);
	}

	if (game::get()->is_running()) {
		emit laws_changed();
	}
}

bool country_game_data::has_law(const law *law) const
{
	return this->get_law(law->get_group()) == law;
}

bool country_game_data::can_have_law(const kobold::law *law) const
{
	if (law->get_conditions() != nullptr && !law->get_conditions()->check(this->country, read_only_context(this->country))) {
		return false;
	}

	return true;
}

bool country_game_data::can_enact_law(const kobold::law *law) const
{
	if (!this->can_have_law(law)) {
		return false;
	}

	for (const auto &[commodity, cost] : law->get_commodity_costs()) {
		if (this->get_stored_commodity(commodity) < (cost * this->get_total_law_cost_modifier() / 100)) {
			return false;
		}
	}

	return true;
}

void country_game_data::enact_law(const law *law)
{
	for (const auto &[commodity, cost] : law->get_commodity_costs()) {
		this->change_stored_commodity(commodity, -cost * this->get_total_law_cost_modifier() / 100);
	}

	this->set_law(law->get_group(), law);
}

void country_game_data::check_laws()
{
	for (const law_group *law_group : law_group::get_all()) {
		const law *law = this->get_law(law_group);
		if (law != nullptr && !this->can_have_law(law)) {
			this->set_law(law_group, nullptr);
			law = nullptr;
		}

		if (law == nullptr) {
			if (this->can_have_law(law_group->get_default_law())) {
				this->set_law(law_group, law_group->get_default_law());
			}
		}
	}
}

std::vector<const tradition *> country_game_data::get_available_traditions() const
{
	std::vector<const tradition *> traditions;

	for (const tradition *tradition : tradition::get_all()) {
		if (!tradition->is_available_for_country(this->country)) {
			continue;
		}

		traditions.push_back(tradition);
	}

	std::sort(traditions.begin(), traditions.end(), tradition_compare());

	return traditions;
}

QVariantList country_game_data::get_available_traditions_qvariant_list() const
{
	return container::to_qvariant_list(this->get_available_traditions());
}

bool country_game_data::can_have_tradition(const tradition *tradition) const
{
	assert_throw(tradition != nullptr);

	if (!tradition->is_available_for_country(this->country)) {
		return false;
	}

	for (const kobold::tradition *prerequisite : tradition->get_prerequisites()) {
		if (!this->has_tradition(prerequisite)) {
			return false;
		}
	}

	if (tradition->get_conditions() != nullptr && !tradition->get_conditions()->check(this->country, read_only_context(this->country))) {
		return false;
	}

	return true;
}

void country_game_data::gain_tradition(const tradition *tradition, const int multiplier)
{
	assert_throw(tradition != nullptr);

	if (this->has_tradition(tradition) == (multiplier > 0)) {
		return;
	}

	if (multiplier > 0) {
		this->traditions.insert(tradition);
	} else {
		this->traditions.erase(tradition);
	}

	if (tradition->get_modifier() != nullptr) {
		tradition->get_modifier()->apply(this->country, multiplier);
	}

	if (tradition->get_effects() != nullptr && multiplier > 0 && game::get()->is_running()) {
		context ctx(this->country);
		tradition->get_effects()->do_effects(this->country, ctx);
	}

	if (multiplier > 0) {
		for (const kobold::tradition *incompatible_tradition : tradition->get_incompatible_traditions()) {
			if (this->has_tradition(incompatible_tradition)) {
				this->gain_tradition(incompatible_tradition, -1);
			}
		}
	} else if (multiplier < 0) {
		for (const kobold::tradition *requiring_tradition : tradition->get_requiring_traditions()) {
			if (this->has_tradition(requiring_tradition)) {
				this->gain_tradition(requiring_tradition, -1);
			}
		}
	}

	if (game::get()->is_running()) {
		emit traditions_changed();
	}

	this->check_traditions();
	this->check_government_type();
}

void country_game_data::gain_tradition_with_prerequisites(const tradition *tradition)
{
	for (const kobold::tradition *prerequisite : tradition->get_prerequisites()) {
		this->gain_tradition_with_prerequisites(prerequisite);
	}

	this->gain_tradition(tradition, 1);
}

void country_game_data::check_traditions()
{
	//remove traditions which can no longer be had
	const tradition_set traditions = this->get_traditions();
	for (const tradition *tradition : traditions) {
		if (!this->can_have_tradition(tradition)) {
			this->gain_tradition(tradition, -1);
		}
	}

	this->check_beliefs();

	if (this->is_under_anarchy()) {
		if (this->get_next_tradition() != nullptr) {
			this->set_next_tradition(nullptr);
		}

		return;
	}

	if (this->get_next_tradition() != nullptr) {
		if (!this->can_have_tradition(this->get_next_tradition())) {
			if (this->country == game::get()->get_player_country()) {
				const portrait *interior_minister_portrait = defines::get()->get_interior_minister_portrait();

				engine_interface::get()->add_notification("Tradition Unavailable", interior_minister_portrait, std::format("Your Excellency, the {} tradition is no longer available for adoption.", this->get_next_tradition()->get_name()));
			}

			this->set_next_tradition(nullptr);
		} else {
			if (this->get_stored_commodity(defines::get()->get_tradition_commodity()) >= this->get_tradition_cost()) {
				this->change_stored_commodity(defines::get()->get_tradition_commodity(), -this->get_tradition_cost());

				this->gain_tradition(this->get_next_tradition(), 1);

				emit tradition_adopted(this->get_next_tradition());

				this->set_next_tradition(nullptr);
			}
		}
	} else {
		if (this->get_commodity_output(defines::get()->get_tradition_commodity()).to_int() > 0 || this->get_stored_commodity(defines::get()->get_tradition_commodity()) > 0) {
			this->choose_next_tradition();
		}
	}
}

void country_game_data::check_beliefs()
{
	if (this->is_under_anarchy()) {
		if (this->get_next_belief() != nullptr) {
			this->set_next_belief(nullptr);
		}

		return;
	}

	if (this->get_next_belief() != nullptr) {
		if (!this->can_have_tradition(this->get_next_belief())) {
			if (this->country == game::get()->get_player_country()) {
				const portrait *interior_minister_portrait = defines::get()->get_interior_minister_portrait();

				engine_interface::get()->add_notification("Belief Unavailable", interior_minister_portrait, std::format("Your Excellency, the {} belief is no longer available for adoption.", this->get_next_belief()->get_name()));
			}

			this->set_next_belief(nullptr);
		} else {
			if (this->get_stored_commodity(defines::get()->get_piety_commodity()) >= this->get_tradition_cost()) {
				this->change_stored_commodity(defines::get()->get_piety_commodity(), -this->get_tradition_cost());

				this->gain_tradition(this->get_next_belief(), 1);

				emit belief_adopted(this->get_next_belief());

				this->set_next_belief(nullptr);
			}
		}
	} else {
		if (this->get_commodity_output(defines::get()->get_piety_commodity()).to_int() > 0 || this->get_stored_commodity(defines::get()->get_piety_commodity()) > 0) {
			this->choose_next_belief();
		}
	}
}

void country_game_data::choose_next_tradition()
{
	tradition_group_map<std::vector<const tradition *>> potential_traditions_per_group;

	for (const tradition *tradition : this->get_available_traditions()) {
		if (tradition->get_category() != tradition_category::tradition) {
			continue;
		}

		if (this->has_tradition(tradition)) {
			continue;
		}

		if (!this->can_have_tradition(tradition)) {
			continue;
		}

		bool has_incompatible_tradition = false;
		for (const kobold::tradition *incompatible_tradition : tradition->get_incompatible_traditions()) {
			if (this->has_tradition(incompatible_tradition)) {
				has_incompatible_tradition = true;
				break;
			}
		}
		if (has_incompatible_tradition) {
			continue;
		}

		std::vector<const kobold::tradition *> &group_traditions = potential_traditions_per_group[tradition->get_group()];

		group_traditions.push_back(tradition);
	}

	if (potential_traditions_per_group.empty()) {
		return;
	}

	tradition_group_map<const tradition *> potential_tradition_map;

	for (const auto &[group, group_traditions] : potential_traditions_per_group) {
		potential_tradition_map[group] = vector::get_random(group_traditions);
	}

	if (this->is_ai()) {
		std::vector<const tradition *> preferred_traditions;

		int best_desire = 0;
		for (const auto &[group, tradition] : potential_tradition_map) {
			int desire = 100;

			for (const journal_entry *journal_entry : this->get_active_journal_entries()) {
				if (vector::contains(journal_entry->get_adopted_traditions(), tradition)) {
					desire += journal_entry::ai_tradition_desire_modifier;
				}
			}

			assert_throw(desire > 0);

			if (desire > best_desire) {
				preferred_traditions.clear();
				best_desire = desire;
			}

			if (desire >= best_desire) {
				preferred_traditions.push_back(tradition);
			}
		}

		assert_throw(!preferred_traditions.empty());

		const tradition *chosen_tradition = vector::get_random(preferred_traditions);
		this->set_next_tradition(chosen_tradition);
	} else {
		const std::vector<const tradition *> potential_traditions = archimedes::map::get_values(potential_tradition_map);
		emit engine_interface::get()->next_tradition_choosable(container::to_qvariant_list(potential_traditions));
	}
}

void country_game_data::choose_next_belief()
{
	tradition_group_map<std::vector<const tradition *>> potential_beliefs_per_group;

	for (const tradition *tradition : this->get_available_traditions()) {
		if (tradition->get_category() != tradition_category::belief) {
			continue;
		}

		if (this->has_tradition(tradition)) {
			continue;
		}

		if (!this->can_have_tradition(tradition)) {
			continue;
		}

		bool has_incompatible_tradition = false;
		for (const kobold::tradition *incompatible_tradition : tradition->get_incompatible_traditions()) {
			if (this->has_tradition(incompatible_tradition)) {
				has_incompatible_tradition = true;
				break;
			}
		}
		if (has_incompatible_tradition) {
			continue;
		}

		std::vector<const kobold::tradition *> &group_beliefs = potential_beliefs_per_group[tradition->get_group()];

		group_beliefs.push_back(tradition);
	}

	if (potential_beliefs_per_group.empty()) {
		return;
	}

	tradition_group_map<const tradition *> potential_belief_map;

	for (const auto &[group, group_beliefs] : potential_beliefs_per_group) {
		potential_belief_map[group] = vector::get_random(group_beliefs);
	}

	if (this->is_ai()) {
		std::vector<const tradition *> preferred_beliefs;

		int best_desire = 0;
		for (const auto &[group, belief] : potential_belief_map) {
			int desire = 100;

			for (const journal_entry *journal_entry : this->get_active_journal_entries()) {
				if (vector::contains(journal_entry->get_adopted_traditions(), belief)) {
					desire += journal_entry::ai_tradition_desire_modifier;
				}
			}

			assert_throw(desire > 0);

			if (desire > best_desire) {
				preferred_beliefs.clear();
				best_desire = desire;
			}

			if (desire >= best_desire) {
				preferred_beliefs.push_back(belief);
			}
		}

		assert_throw(!preferred_beliefs.empty());

		const tradition *chosen_belief = vector::get_random(preferred_beliefs);
		this->set_next_belief(chosen_belief);
	} else {
		const std::vector<const tradition *> potential_beliefs = archimedes::map::get_values(potential_belief_map);
		emit engine_interface::get()->next_belief_choosable(container::to_qvariant_list(potential_beliefs));
	}
}

void country_game_data::remove_character(const character *character)
{
	std::erase(this->characters, character);

	const office *office = character->get_game_data()->get_office();
	if (office != nullptr) {
		this->set_office_holder(office, nullptr);
		this->check_office_holder(office, character);
	}
}

void country_game_data::check_characters()
{
	const std::vector<const character *> characters = this->get_characters();
	const QDate &current_date = game::get()->get_next_date();
	for (const character *character : characters) {
		if (character->is_immortal()) {
			continue;
		}

		assert_throw(character->get_death_date().isValid());

		if (character->get_death_date() > current_date) {
			continue;
		}

		character->get_game_data()->die();
	}

	for (const office *office : office::get_all()) {
		this->check_office_holder(office, nullptr);
	}
}

const character *country_game_data::get_ruler() const
{
	return this->get_office_holder(defines::get()->get_ruler_office());
}

void country_game_data::generate_ruler()
{
	assert_throw(this->get_government_type() != nullptr);

	const species_base *species = vector::get_random(this->country->get_culture()->get_species());

	std::vector<const character_class *> potential_base_classes;
	for (const character_class *character_class : this->get_government_type()->get_ruler_character_classes()) {
		if (character_class->get_type() == character_class_type::base_class) {
			const int weight = std::max(species->get_species()->get_character_class_weight(character_class), 1);
			for (int i = 0; i < weight; ++i) {
				potential_base_classes.push_back(character_class);
			}
		}
	}
	assert_throw(!potential_base_classes.empty());

	const character_class *base_class = vector::get_random(potential_base_classes);

	const character *ruler = character::generate(species->get_species(), species->get_subspecies(), {{base_class->get_type(), const_cast<character_class *>(base_class)}}, 1, this->country->get_culture(), this->get_religion(), this->get_capital(), {});
	this->set_office_holder(defines::get()->get_ruler_office(), ruler);
}

QVariantList country_game_data::get_office_holders_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_office_holders());
}

void country_game_data::set_office_holder(const office *office, const character *character)
{
	const kobold::character *old_office_holder = this->get_office_holder(office);

	if (character == old_office_holder) {
		return;
	}

	if (old_office_holder != nullptr) {
		old_office_holder->get_game_data()->set_office(nullptr);
		this->apply_office_holder(office, old_office_holder, -1);
	}

	const kobold::office *old_office = character ? character->get_game_data()->get_office() : nullptr;
	if (old_office != nullptr) {
		this->set_office_holder(old_office, nullptr);
	}

	if (character != nullptr) {
		this->office_holders[office] = character;
	} else {
		this->office_holders.erase(office);
	}

	if (character != nullptr) {
		if (office == defines::get()->get_ruler_office()) {
			this->get_ruler()->get_game_data()->set_country(this->country);
		}

		character->get_game_data()->set_office(office);
		this->apply_office_holder(office, character, 1);
	}

	if (old_office != nullptr) {
		this->check_office_holder(old_office, character);
	}

	if (game::get()->is_running()) {
		emit office_holders_changed();

		if (office == defines::get()->get_ruler_office()) {
			emit ruler_changed();
		}
	}
}

void country_game_data::apply_office_holder(const office *office, const character *office_holder, const int multiplier)
{
	assert_throw(office != nullptr);
	assert_throw(office_holder != nullptr);

	//apply the highest character attribute modifier to all country attributes listed
	int attribute_modifier = 0;
	for (const character_attribute *attribute : office->get_character_attributes()) {
		attribute_modifier = std::max(office_holder->get_game_data()->get_attribute_modifier(attribute), attribute_modifier);
	}

	if (office->get_attribute_modifier_divisor() != 0) {
		attribute_modifier /= office->get_attribute_modifier_divisor();
	}

	for (const country_attribute *attribute : office->get_country_attributes()) {
		this->change_attribute_value(attribute, attribute_modifier * multiplier);
	}
}

void country_game_data::check_office_holder(const office *office, const character *previous_holder)
{
	if (this->is_under_anarchy()) {
		this->set_office_holder(office, nullptr);
		return;
	}

	//if the country has no holder for the office, see if there is any character who can become the holder
	if (this->get_office_holder(office) == nullptr) {
		this->choose_office_holder(office, previous_holder);
	}
}

void country_game_data::choose_office_holder(const office *office, const character *previous_holder)
{
	assert_throw(this->get_government_type() != nullptr);

	std::vector<const character *> potential_holders;
	int best_attribute_value = 0;
	bool found_same_dynasty = false;

	for (const character *character : this->get_characters()) {
		const character_game_data *character_game_data = character->get_game_data();

		if (!character_game_data->is_adult()) {
			continue;
		}

		assert_throw(character_game_data->get_country() == this->country);

		if (office->get_conditions() != nullptr && !office->get_conditions()->check(character, read_only_context(character))) {
			continue;
		}

		if (office == defines::get()->get_ruler_office()) {
			const character_class *base_class = character->get_game_data()->get_character_class(character_class_type::base_class);
			if (base_class == nullptr || !vector::contains(this->get_government_type()->get_ruler_character_classes(), base_class)) {
				continue;
			}

			if (previous_holder != nullptr && previous_holder->get_dynasty() != nullptr) {
				const bool same_dynasty = character->get_dynasty() == previous_holder->get_dynasty();
				if (same_dynasty && !found_same_dynasty) {
					potential_holders.clear();
					best_attribute_value = 0;
					found_same_dynasty = true;
				} else if (!same_dynasty && found_same_dynasty) {
					continue;
				}
			}
		} else {
			if (character_game_data->get_office() != nullptr) {
				continue;
			}
		}

		int attribute_value = 0;
		for (const character_attribute *attribute : office->get_character_attributes()) {
			attribute_value = std::max(character_game_data->get_attribute_value(attribute), attribute_value);
		}

		if (attribute_value < best_attribute_value) {
			continue;
		}

		if (attribute_value > best_attribute_value) {
			best_attribute_value = attribute_value;
			potential_holders.clear();
		}

		potential_holders.push_back(character);
	}

	if (!potential_holders.empty()) {
		this->set_office_holder(office, vector::get_random(potential_holders));
	} else {
		if (office == defines::get()->get_ruler_office()) {
			this->generate_ruler();
			assert_throw(this->get_ruler() != nullptr);
		}
	}

	const character *office_holder = this->get_office_holder(office);
	if (this->country == game::get()->get_player_country() && game::get()->is_running() && office_holder != nullptr) {
		const portrait *interior_minister_portrait = defines::get()->get_interior_minister_portrait();

		engine_interface::get()->add_notification(std::format("New {}", office->get_name()), interior_minister_portrait, std::format("{} has become our new {}!", office_holder->get_full_name(), string::lowered(office->get_name())));
	}
}

void country_game_data::on_office_holder_died(const office *office, const character *office_holder)
{
	if (game::get()->is_running()) {
		if (this->country == game::get()->get_player_country()) {
			const portrait *interior_minister_portrait = defines::get()->get_interior_minister_portrait();

			engine_interface::get()->add_notification(std::format("{} Died", office->get_name()), interior_minister_portrait, std::format("Our {}, {}, has died!", string::lowered(office->get_name()), office_holder->get_full_name()));
		}

		if (office == defines::get()->get_ruler_office()) {
			context ctx(this->country);
			ctx.source_scope = office_holder;
			country_event::check_events_for_scope(this->country, event_trigger::ruler_death, ctx);
		}
	}
}

QVariantList country_game_data::get_bids_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_bids());
}

void country_game_data::set_bid(const commodity *commodity, const int value)
{
	if (value == this->get_bid(commodity)) {
		return;
	}

	if (value == 0) {
		this->bids.erase(commodity);
	} else {
		this->set_offer(commodity, 0);
		this->bids[commodity] = value;
	}

	if (game::get()->is_running()) {
		emit bids_changed();
	}
}

QVariantList country_game_data::get_offers_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_offers());
}

void country_game_data::set_offer(const commodity *commodity, const int value)
{
	if (value == this->get_offer(commodity)) {
		return;
	}

	if (value > this->get_stored_commodity(commodity)) {
		this->set_offer(commodity, this->get_stored_commodity(commodity));
		return;
	}

	if (value == 0) {
		this->offers.erase(commodity);
	} else {
		this->set_bid(commodity, 0);
		this->offers[commodity] = value;
	}

	if (game::get()->is_running()) {
		emit offers_changed();
	}
}

void country_game_data::do_sale(const kobold::country *other_country, const commodity *commodity, const int sold_quantity, const bool state_purchase)
{
	this->change_stored_commodity(commodity, -sold_quantity);

	const int price = game::get()->get_price(commodity);
	const int sale_income = price * sold_quantity;
	this->add_taxable_wealth(sale_income, income_transaction_type::tariff);
	this->country->get_turn_data()->add_income_transaction(income_transaction_type::sale, sale_income, commodity, sold_quantity, other_country != this->country ? other_country : nullptr);

	this->change_offer(commodity, -sold_quantity);

	country_game_data *other_country_game_data = other_country->get_game_data();

	if (state_purchase) {
		other_country_game_data->change_stored_commodity(commodity, sold_quantity);
		const int purchase_expense = price * sold_quantity;
		other_country_game_data->change_wealth(-purchase_expense);
		other_country->get_turn_data()->add_expense_transaction(expense_transaction_type::purchase, purchase_expense, commodity, sold_quantity, this->country);

		other_country_game_data->change_bid(commodity, -sold_quantity);
	}

	//improve relations between the two countries after they traded (even if it was not a state purchase)
	if (this->country != other_country) {
		this->change_base_opinion(other_country, 1);
		other_country_game_data->change_base_opinion(this->country, 1);
	}
}

void country_game_data::calculate_commodity_needs()
{
	this->commodity_needs.clear();
}

void country_game_data::assign_trade_orders()
{
	assert_throw(this->is_ai());

	this->bids.clear();
	this->offers.clear();

	if (this->is_under_anarchy()) {
		return;
	}

	for (const auto &[commodity, value] : this->get_stored_commodities()) {
		if (!this->can_trade_commodity(commodity)) {
			continue;
		}

		const int need = this->get_commodity_need(commodity);

		if (value > need) {
			this->set_offer(commodity, value);
		}
	}
}

void country_game_data::add_civilian_unit(qunique_ptr<civilian_unit> &&civilian_unit)
{
	this->civilian_units.push_back(std::move(civilian_unit));
}

void country_game_data::remove_civilian_unit(civilian_unit *civilian_unit)
{
	assert_throw(civilian_unit != nullptr);

	for (size_t i = 0; i < this->civilian_units.size(); ++i) {
		if (this->civilian_units[i].get() == civilian_unit) {
			this->civilian_units.erase(this->civilian_units.begin() + i);
			return;
		}
	}
}

void country_game_data::add_military_unit(qunique_ptr<military_unit> &&military_unit)
{
	this->military_unit_names.insert(military_unit->get_name());
	this->military_units.push_back(std::move(military_unit));
}

void country_game_data::remove_military_unit(military_unit *military_unit)
{
	this->military_unit_names.erase(military_unit->get_name());

	for (size_t i = 0; i < this->military_units.size(); ++i) {
		if (this->military_units[i].get() == military_unit) {
			this->military_units.erase(this->military_units.begin() + i);
			return;
		}
	}
}

void country_game_data::add_army(qunique_ptr<army> &&army)
{
	this->armies.push_back(std::move(army));
}

void country_game_data::remove_army(army *army)
{
	for (size_t i = 0; i < this->armies.size(); ++i) {
		if (this->armies[i].get() == army) {
			this->armies.erase(this->armies.begin() + i);
			return;
		}
	}
}

const military_unit_type *country_game_data::get_best_military_unit_category_type(const military_unit_category category, const culture *culture) const
{
	const military_unit_type *best_type = nullptr;
	int best_score = -1;

	for (const military_unit_class *military_unit_class : military_unit_class::get_all()) {
		if (military_unit_class->get_category() != category) {
			continue;
		}

		const military_unit_type *type = culture->get_military_class_unit_type(military_unit_class);

		if (type == nullptr) {
			continue;
		}

		bool upgrade_is_available = false;
		for (const military_unit_type *upgrade : type->get_upgrades()) {
			if (culture->get_military_class_unit_type(upgrade->get_unit_class()) != upgrade) {
				continue;
			}

			upgrade_is_available = true;
			break;
		}

		if (upgrade_is_available) {
			continue;
		}

		const int score = type->get_score();

		if (score > best_score) {
			best_type = type;
		}
	}

	return best_type;
}

const military_unit_type *country_game_data::get_best_military_unit_category_type(const military_unit_category category) const
{
	return this->get_best_military_unit_category_type(category, this->country->get_culture());
}

void country_game_data::change_settlement_attribute_value(const settlement_attribute *attribute, const int change)
{
	if (change == 0) {
		return;
	}

	const int new_value = (this->settlement_attribute_values[attribute] += change);
	if (new_value == 0) {
		this->settlement_attribute_values.erase(attribute);
	}

	for (const province *province : this->get_provinces()) {
		for (const site *settlement : province->get_game_data()->get_settlement_sites()) {
			if (!settlement->get_game_data()->is_built()) {
				continue;
			}

			settlement->get_game_data()->change_settlement_attribute_value(attribute, change);
		}
	}

	if (game::get()->is_running()) {
		emit settlement_attribute_values_changed();
	}
}

void country_game_data::set_military_unit_type_stat_modifier(const military_unit_type *type, const military_unit_stat stat, const centesimal_int &value)
{
	const centesimal_int old_value = this->get_military_unit_type_stat_modifier(type, stat);

	if (value == old_value) {
		return;
	}

	if (value == 0) {
		this->military_unit_type_stat_modifiers[type].erase(stat);

		if (this->military_unit_type_stat_modifiers[type].empty()) {
			this->military_unit_type_stat_modifiers.erase(type);
		}
	} else {
		this->military_unit_type_stat_modifiers[type][stat] = value;
	}

	const centesimal_int difference = value - old_value;
	for (const qunique_ptr<military_unit> &military_unit : this->military_units) {
		if (military_unit->get_type() != type) {
			continue;
		}

		military_unit->change_stat(stat, difference);
	}
}

void country_game_data::set_output_modifier(const centesimal_int &value)
{
	if (value == this->get_output_modifier()) {
		return;
	}

	this->output_modifier = value;

	this->calculate_site_commodity_outputs();

	if (game::get()->is_running()) {
		emit output_modifier_changed();
	}
}

void country_game_data::set_commodity_output_modifier(const commodity *commodity, const centesimal_int &value)
{
	if (value == this->get_commodity_output_modifier(commodity)) {
		return;
	}

	if (value == 0) {
		this->commodity_output_modifiers.erase(commodity);
	} else {
		this->commodity_output_modifiers[commodity] = value;
	}

	this->calculate_site_commodity_output(commodity);
}

void country_game_data::set_capital_commodity_output_modifier(const commodity *commodity, const centesimal_int &value)
{
	if (value == this->get_capital_commodity_output_modifier(commodity)) {
		return;
	}

	if (value == 0) {
		this->capital_commodity_output_modifiers.erase(commodity);
	} else {
		this->capital_commodity_output_modifiers[commodity] = value;
	}

	if (this->get_capital() != nullptr) {
		this->get_capital()->get_game_data()->calculate_commodity_outputs();
	}
}

void country_game_data::set_throughput_modifier(const int value)
{
	if (value == this->get_throughput_modifier()) {
		return;
	}

	this->throughput_modifier = value;

	if (game::get()->is_running()) {
		emit throughput_modifier_changed();
	}
}

void country_game_data::set_commodity_throughput_modifier(const commodity *commodity, const int value)
{
	if (value == this->get_commodity_throughput_modifier(commodity)) {
		return;
	}

	if (value == 0) {
		this->commodity_throughput_modifiers.erase(commodity);
	} else {
		this->commodity_throughput_modifiers[commodity] = value;
	}
}

void country_game_data::change_improved_resource_commodity_bonus(const resource *resource, const commodity *commodity, const int change)
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

	for (const province *province : this->get_provinces()) {
		province->get_game_data()->change_improved_resource_commodity_bonus(resource, commodity, change);
	}
}

void country_game_data::change_improvement_commodity_bonus(const improvement *improvement, const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int &count = (this->improvement_commodity_bonuses[improvement][commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->improvement_commodity_bonuses[improvement].erase(commodity);

		if (this->improvement_commodity_bonuses[improvement].empty()) {
			this->improvement_commodity_bonuses.erase(improvement);
		}
	}

	for (const province *province : this->get_provinces()) {
		for (const QPoint &tile_pos : province->get_game_data()->get_resource_tiles()) {
			const tile *tile = map::get()->get_tile(tile_pos);
			const site *site = tile->get_site();

			if (site->get_game_data()->has_improvement(improvement)) {
				site->get_game_data()->change_base_commodity_output(commodity, change);
			}
		}
	}
}

void country_game_data::change_building_commodity_bonus(const building_type *building, const commodity *commodity, const int change)
{
	if (change == 0) {
		return;
	}

	const int count = (this->building_commodity_bonuses[building][commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->building_commodity_bonuses[building].erase(commodity);

		if (this->building_commodity_bonuses[building].empty()) {
			this->building_commodity_bonuses.erase(building);
		}
	}

	for (const province *province : this->get_provinces()) {
		for (const site *settlement : province->get_game_data()->get_settlement_sites()) {
			if (!settlement->get_game_data()->is_built()) {
				continue;
			}

			if (settlement->get_game_data()->has_building(building)) {
				settlement->get_game_data()->change_base_commodity_output(commodity, centesimal_int(change));
			}
		}
	}
}

void country_game_data::set_commodity_bonus_for_tile_threshold(const commodity *commodity, const int threshold, const int value)
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

	for (const province *province : this->get_provinces()) {
		province->get_game_data()->change_commodity_bonus_for_tile_threshold(commodity, threshold, value - old_value);
	}
}

void country_game_data::change_settlement_commodity_bonus(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int count = (this->settlement_commodity_bonuses[commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->settlement_commodity_bonuses.erase(commodity);
	}

	for (const province *province : this->get_provinces()) {
		for (const site *settlement : province->get_game_data()->get_settlement_sites()) {
			if (!settlement->get_game_data()->is_built()) {
				continue;
			}

			settlement->get_game_data()->change_base_commodity_output(commodity, centesimal_int(change));
		}
	}
}

void country_game_data::change_capital_commodity_bonus(const commodity *commodity, const centesimal_int &change)
{
	if (change == 0) {
		return;
	}

	const centesimal_int count = (this->capital_commodity_bonuses[commodity] += change);

	assert_throw(count >= 0);

	if (count == 0) {
		this->capital_commodity_bonuses.erase(commodity);
	}

	if (this->get_capital() != nullptr) {
		this->get_capital()->get_game_data()->calculate_commodity_outputs();
	}
}

QVariantList country_game_data::get_scripted_modifiers_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_scripted_modifiers());
}

bool country_game_data::has_scripted_modifier(const scripted_country_modifier *modifier) const
{
	return this->get_scripted_modifiers().contains(modifier);
}

void country_game_data::add_scripted_modifier(const scripted_country_modifier *modifier, const int duration)
{
	const read_only_context ctx(this->country);

	this->scripted_modifiers[modifier] = std::max(this->scripted_modifiers[modifier], duration);

	if (modifier->get_modifier() != nullptr) {
		modifier->get_modifier()->apply(this->country);
	}

	if (game::get()->is_running()) {
		emit scripted_modifiers_changed();
	}
}

void country_game_data::remove_scripted_modifier(const scripted_country_modifier *modifier)
{
	this->scripted_modifiers.erase(modifier);

	if (modifier->get_modifier() != nullptr) {
		modifier->get_modifier()->remove(this->country);
	}

	if (game::get()->is_running()) {
		emit scripted_modifiers_changed();
	}
}

void country_game_data::decrement_scripted_modifiers()
{
	std::vector<const scripted_country_modifier *> modifiers_to_remove;
	for (auto &[modifier, duration] : this->scripted_modifiers) {
		--duration;

		if (duration == 0) {
			modifiers_to_remove.push_back(modifier);
		}
	}

	for (const scripted_country_modifier *modifier : modifiers_to_remove) {
		this->remove_scripted_modifier(modifier);
	}

	//decrement opinion modifiers
	country_map<std::vector<const opinion_modifier *>> opinion_modifiers_to_remove;

	for (auto &[country, opinion_modifier_map] : this->opinion_modifiers) {
		for (auto &[modifier, duration] : opinion_modifier_map) {
			if (duration == -1) {
				//eternal
				continue;
			}

			--duration;

			if (duration == 0) {
				opinion_modifiers_to_remove[country].push_back(modifier);
			}
		}
	}

	for (const auto &[country, opinion_modifiers] : opinion_modifiers_to_remove) {
		for (const opinion_modifier *modifier : opinion_modifiers) {
			this->remove_opinion_modifier(country, modifier);
		}
	}
}

bool country_game_data::is_tile_explored(const QPoint &tile_pos) const
{
	const tile *tile = map::get()->get_tile(tile_pos);

	if (tile->get_province() != nullptr && this->explored_provinces.contains(tile->get_province())) {
		return true;
	}

	if (this->explored_tiles.contains(tile_pos)) {
		return true;
	}

	return false;
}

bool country_game_data::is_province_discovered(const province *province) const
{
	//get whether the province has been at least partially explored
	const province_map_data *province_map_data = province->get_map_data();
	if (!province_map_data->is_on_map()) {
		return false;
	}

	if (this->is_province_explored(province)) {
		return true;
	}

	if (!this->explored_tiles.empty()) {
		for (const QPoint &tile_pos : province_map_data->get_tiles()) {
			if (this->explored_tiles.contains(tile_pos)) {
				return true;
			}
		}
	}

	return false;
}

bool country_game_data::is_region_discovered(const region *region) const
{
	for (const province *province : region->get_provinces()) {
		const province_game_data *province_game_data = province->get_game_data();
		if (!province_game_data->is_on_map()) {
			continue;
		}

		if (this->is_province_explored(province)) {
			return true;
		}
	}

	//go through the explored tiles instead of each province's tiles, as this is likely faster
	for (const QPoint &tile_pos : this->explored_tiles) {
		const tile *tile = map::get()->get_tile(tile_pos);
		if (tile->get_province() != nullptr && vector::contains(region->get_provinces(), tile->get_province())) {
			return true;
		}
	}

	return false;
}

void country_game_data::explore_tile(const QPoint &tile_pos)
{
	this->explored_tiles.insert(tile_pos);

	if (this->country == game::get()->get_player_country()) {
		map::get()->update_minimap_rect(QRect(tile_pos, QSize(1, 1)));
		game::get()->set_exploration_changed();
		emit map::get()->tile_exploration_changed(tile_pos);
	}

	const tile *tile = map::get()->get_tile(tile_pos);
	const kobold::country *tile_owner = tile->get_owner();

	if (tile_owner != nullptr && tile_owner != this->country && !this->is_country_known(tile_owner)) {
		this->add_known_country(tile_owner);
	}

	if (tile->get_province() != nullptr) {
		//add the tile's province to the explored provinces if all of its tiles have been explored
		const province_map_data *province_map_data = tile->get_province()->get_map_data();
		if (this->explored_tiles.size() >= province_map_data->get_tiles().size()) {
			for (const QPoint &province_tile_pos : province_map_data->get_tiles()) {
				if (!this->explored_tiles.contains(province_tile_pos)) {
					return;
				}
			}

			//add the province to the explored provinces and remove its tiles from the explored tiles, as they no longer need to be taken into account separately
			this->explore_province(tile->get_province());
		}
	}
}

void country_game_data::explore_province(const province *province)
{
	if (this->explored_provinces.contains(province)) {
		return;
	}

	const province_game_data *province_game_data = province->get_game_data();
	assert_throw(province_game_data->is_on_map());

	const kobold::country *province_owner = province_game_data->get_owner();

	if (province_owner != nullptr && province_owner != this->country && !this->is_country_known(province_owner)) {
		this->add_known_country(province_owner);
	}

	const province_map_data *province_map_data = province->get_map_data();

	if (!this->explored_tiles.empty()) {
		for (const QPoint &tile_pos : province_map_data->get_tiles()) {
			if (this->explored_tiles.contains(tile_pos)) {
				this->explored_tiles.erase(tile_pos);
			}
		}
	}

	this->explored_provinces.insert(province);

	if (this->country == game::get()->get_player_country()) {
		map::get()->update_minimap_rect(province_map_data->get_territory_rect());
		game::get()->set_exploration_changed();

		for (const QPoint &tile_pos : province_map_data->get_tiles()) {
			emit map::get()->tile_exploration_changed(tile_pos);
		}
	}
}

void country_game_data::prospect_tile(const QPoint &tile_pos)
{
	this->prospected_tiles.insert(tile_pos);

	emit prospected_tiles_changed();

	if (this->country == game::get()->get_player_country()) {
		emit map::get()->tile_prospection_changed(tile_pos);
	}
}

void country_game_data::reset_tile_prospection(const QPoint &tile_pos)
{
	this->prospected_tiles.erase(tile_pos);

	emit prospected_tiles_changed();

	if (this->country == game::get()->get_player_country()) {
		emit map::get()->tile_prospection_changed(tile_pos);
	}
}

QVariantList country_game_data::get_active_journal_entries_qvariant_list() const
{
	return container::to_qvariant_list(this->get_active_journal_entries());
}

void country_game_data::add_active_journal_entry(const journal_entry *journal_entry)
{
	this->active_journal_entries.push_back(journal_entry);

	if (journal_entry->get_active_modifier() != nullptr) {
		journal_entry->get_active_modifier()->apply(this->country);
	}

	for (const building_type *building : journal_entry->get_built_buildings_with_requirements()) {
		this->change_ai_building_desire_modifier(building, journal_entry::ai_building_desire_modifier);
	}

	for (const auto &[settlement, buildings] : journal_entry->get_built_settlement_buildings_with_requirements()) {
		for (const building_type *building : buildings) {
			this->change_ai_settlement_building_desire_modifier(settlement, building, journal_entry::ai_building_desire_modifier);
		}
	}
}

void country_game_data::remove_active_journal_entry(const journal_entry *journal_entry)
{
	std::erase(this->active_journal_entries, journal_entry);

	if (journal_entry->get_active_modifier() != nullptr) {
		journal_entry->get_active_modifier()->remove(this->country);
	}

	for (const building_type *building : journal_entry->get_built_buildings_with_requirements()) {
		this->change_ai_building_desire_modifier(building, -journal_entry::ai_building_desire_modifier);
	}

	for (const auto &[settlement, buildings] : journal_entry->get_built_settlement_buildings_with_requirements()) {
		for (const building_type *building : buildings) {
			this->change_ai_settlement_building_desire_modifier(settlement, building, -journal_entry::ai_building_desire_modifier);
		}
	}
}

QVariantList country_game_data::get_inactive_journal_entries_qvariant_list() const
{
	return container::to_qvariant_list(this->get_inactive_journal_entries());
}

QVariantList country_game_data::get_finished_journal_entries_qvariant_list() const
{
	return container::to_qvariant_list(this->get_finished_journal_entries());
}

void country_game_data::check_journal_entries(const bool ignore_effects, const bool ignore_random_chance)
{
	const read_only_context ctx(this->country);

	bool changed = false;

	//check if any journal entry has become potentially available
	if (this->check_potential_journal_entries()) {
		changed = true;
	}

	if (this->check_inactive_journal_entries()) {
		changed = true;
	}

	if (this->check_active_journal_entries(ctx, ignore_effects, ignore_random_chance)) {
		changed = true;
	}

	if (changed) {
		emit journal_entries_changed();
	}
}

bool country_game_data::check_potential_journal_entries()
{
	bool changed = false;

	for (const journal_entry *journal_entry : journal_entry::get_all()) {
		if (vector::contains(this->get_active_journal_entries(), journal_entry)) {
			continue;
		}

		if (vector::contains(this->get_inactive_journal_entries(), journal_entry)) {
			continue;
		}

		if (vector::contains(this->get_finished_journal_entries(), journal_entry)) {
			continue;
		}

		if (!journal_entry->check_preconditions(this->country)) {
			continue;
		}

		this->inactive_journal_entries.push_back(journal_entry);
		changed = true;
	}

	return changed;
}

bool country_game_data::check_inactive_journal_entries()
{
	bool changed = false;

	const std::vector<const journal_entry *> inactive_entries = this->get_inactive_journal_entries();

	for (const journal_entry *journal_entry : inactive_entries) {
		if (!journal_entry->check_preconditions(this->country)) {
			std::erase(this->inactive_journal_entries, journal_entry);
			changed = true;
			continue;
		}

		if (!journal_entry->check_conditions(this->country)) {
			continue;
		}

		std::erase(this->inactive_journal_entries, journal_entry);

		this->add_active_journal_entry(journal_entry);

		changed = true;
	}

	return changed;
}

bool country_game_data::check_active_journal_entries(const read_only_context &ctx, const bool ignore_effects, const bool ignore_random_chance)
{
	bool changed = false;

	const std::vector<const journal_entry *> active_entries = this->get_active_journal_entries();

	for (const journal_entry *journal_entry : active_entries) {
		if (!journal_entry->check_preconditions(this->country)) {
			this->remove_active_journal_entry(journal_entry);
			changed = true;
			continue;
		}

		if (!journal_entry->check_conditions(this->country)) {
			this->remove_active_journal_entry(journal_entry);
			this->inactive_journal_entries.push_back(journal_entry);
			changed = true;
			continue;
		}

		if (journal_entry->check_completion_conditions(this->country, ignore_random_chance)) {
			this->remove_active_journal_entry(journal_entry);
			this->finished_journal_entries.push_back(journal_entry);
			if (!ignore_effects) {
				if (journal_entry->get_completion_effects() != nullptr) {
					context effects_ctx(this->country);
					journal_entry->get_completion_effects()->do_effects(this->country, effects_ctx);
				}

				if (game::get()->is_running()) {
					emit journal_entry_completed(journal_entry);
				}
			}

			if (journal_entry->get_completion_modifier() != nullptr) {
				journal_entry->get_completion_modifier()->apply(this->country);
			}

			changed = true;
		} else if (journal_entry->get_failure_conditions() != nullptr && journal_entry->get_failure_conditions()->check(this->country, ctx)) {
			this->remove_active_journal_entry(journal_entry);
			this->finished_journal_entries.push_back(journal_entry);
			if (journal_entry->get_failure_effects() != nullptr && !ignore_effects) {
				context effects_ctx(this->country);
				journal_entry->get_failure_effects()->do_effects(this->country, effects_ctx);

				if (this->country == game::get()->get_player_country()) {
					engine_interface::get()->add_notification(journal_entry->get_name(), journal_entry->get_portrait(), std::format("{}{}{}", journal_entry->get_description(), (!journal_entry->get_description().empty() ? "\n\n" : ""), journal_entry->get_failure_effects()->get_effects_string(this->country, ctx)));
				}
			}
			changed = true;
		}
	}

	return changed;
}

void country_game_data::set_free_building_class_count(const building_class *building_class, const int value)
{
	const int old_value = this->get_free_building_class_count(building_class);
	if (value == old_value) {
		return;
	}

	assert_throw(value >= 0);

	if (value == 0) {
		this->free_building_class_counts.erase(building_class);
	} else if (old_value == 0) {
		this->free_building_class_counts[building_class] = value;

		for (const province *province : this->get_provinces()) {
			for (const site *settlement : province->get_game_data()->get_settlement_sites()) {
				if (!settlement->get_game_data()->is_built()) {
					continue;
				}

				settlement->get_game_data()->check_free_buildings();
			}
		}
	}
}

void country_game_data::set_free_infantry_promotion_count(const promotion *promotion, const int value)
{
	const int old_value = this->get_free_infantry_promotion_count(promotion);
	if (value == old_value) {
		return;
	}

	assert_throw(value >= 0);

	if (value == 0) {
		this->free_infantry_promotion_counts.erase(promotion);
	} else if (old_value == 0) {
		this->free_infantry_promotion_counts[promotion] = value;

		for (const qunique_ptr<military_unit> &military_unit : this->military_units) {
			if (!military_unit->get_type()->is_infantry()) {
				continue;
			}

			military_unit->check_free_promotions();
		}
	}
}

void country_game_data::set_free_cavalry_promotion_count(const promotion *promotion, const int value)
{
	const int old_value = this->get_free_cavalry_promotion_count(promotion);
	if (value == old_value) {
		return;
	}

	assert_throw(value >= 0);

	if (value == 0) {
		this->free_cavalry_promotion_counts.erase(promotion);
	} else if (old_value == 0) {
		this->free_cavalry_promotion_counts[promotion] = value;

		for (const qunique_ptr<military_unit> &military_unit : this->military_units) {
			if (!military_unit->get_type()->is_cavalry()) {
				continue;
			}

			military_unit->check_free_promotions();
		}
	}
}

void country_game_data::set_free_artillery_promotion_count(const promotion *promotion, const int value)
{
	const int old_value = this->get_free_artillery_promotion_count(promotion);
	if (value == old_value) {
		return;
	}

	assert_throw(value >= 0);

	if (value == 0) {
		this->free_artillery_promotion_counts.erase(promotion);
	} else if (old_value == 0) {
		this->free_artillery_promotion_counts[promotion] = value;

		for (const qunique_ptr<military_unit> &military_unit : this->military_units) {
			if (!military_unit->get_type()->is_artillery()) {
				continue;
			}

			military_unit->check_free_promotions();
		}
	}
}

void country_game_data::set_free_warship_promotion_count(const promotion *promotion, const int value)
{
	const int old_value = this->get_free_warship_promotion_count(promotion);
	if (value == old_value) {
		return;
	}

	assert_throw(value >= 0);

	if (value == 0) {
		this->free_warship_promotion_counts.erase(promotion);
	} else if (old_value == 0) {
		this->free_warship_promotion_counts[promotion] = value;

		for (const qunique_ptr<military_unit> &military_unit : this->military_units) {
			if (!military_unit->get_type()->is_ship()) {
				continue;
			}

			military_unit->check_free_promotions();
		}
	}
}

void country_game_data::set_free_consulate_count(const consulate *consulate, const int value)
{
	const int old_value = this->get_free_consulate_count(consulate);
	if (value == old_value) {
		return;
	}

	assert_throw(value >= 0);

	if (value == 0) {
		this->free_consulate_counts.erase(consulate);
	} else if (old_value == 0) {
		this->free_consulate_counts[consulate] = value;

		for (const kobold::country *known_country : this->get_known_countries()) {
			const kobold::consulate *current_consulate = this->get_consulate(known_country);
			if (current_consulate == nullptr || current_consulate->get_level() < consulate->get_level()) {
				this->set_consulate(known_country, consulate);
			}
		}
	}
}

void country_game_data::check_milestones()
{
	for (const country_milestone *milestone : country_milestone::get_all()) {
		if (this->has_milestone(milestone)) {
			continue;
		}

		if (milestone->get_conditions() != nullptr && !milestone->get_conditions()->check(this->country)) {
			continue;
		}

		this->milestones.insert(milestone);

		if (milestone->get_effects() != nullptr) {
			context ctx(this->country);
			milestone->get_effects()->do_effects(this->country, ctx);
		}

		if (game::get()->is_running()) {
			emit milestone_reached(milestone);
		}
	}
}

}
