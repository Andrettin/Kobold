#include "kobold.h"

#include "character/character_game_data.h"

#include "character/character.h"
#include "character/character_attribute.h"
#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/character_history.h"
#include "character/damage_reduction_type.h"
#include "character/feat.h"
#include "character/feat_type.h"
#include "character/level_bonus_table.h"
#include "character/skill.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/culture.h"
#include "country/office.h"
#include "database/defines.h"
#include "engine_interface.h"
#include "game/character_event.h"
#include "game/event_trigger.h"
#include "game/game.h"
#include "item/enchantment.h"
#include "item/item.h"
#include "item/item_type.h"
#include "map/province.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "map/site_map_data.h"
#include "map/tile.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/factor.h"
#include "script/modifier.h"
#include "script/scripted_character_modifier.h"
#include "species/creature_size.h"
#include "species/creature_type.h"
#include "species/species.h"
#include "spell/spell.h"
#include "ui/portrait.h"
#include "unit/civilian_unit.h"
#include "unit/military_unit.h"
#include "unit/military_unit_category.h"
#include "util/assert_util.h"
#include "util/container_util.h"
#include "util/gender.h"
#include "util/log_util.h"
#include "util/map_util.h"
#include "util/number_util.h"
#include "util/string_util.h"
#include "util/vector_random_util.h"
#include "util/vector_util.h"

namespace kobold {

character_game_data::character_game_data(const kobold::character *character)
	: character(character)
{
	connect(game::get(), &game::turn_changed, this, &character_game_data::age_changed);
	connect(this, &character_game_data::office_changed, this, &character_game_data::titled_name_changed);

	this->portrait = this->character->get_portrait();
}

void character_game_data::apply_species_and_class(const int level)
{
	const species *species = this->character->get_species();

	if (species->get_level_adjustment() != 0) {
		this->change_level(species->get_level_adjustment());
	}

	const creature_type *creature_type = species->get_creature_type();
	if (creature_type->get_modifier() != nullptr) {
		creature_type->get_modifier()->apply(this->character, 1);
	}

	const creature_size *creature_size = species->get_creature_size();
	if (creature_size->get_modifier() != nullptr) {
		creature_size->get_modifier()->apply(this->character, 1);
	}

	if (species->get_modifier() != nullptr) {
		species->get_modifier()->apply(this->character, 1);
	}

	const character_class *base_class = this->character->get_character_class(character_class_type::base_class);

	for (const character_attribute *attribute : character_attribute::get_all()) {
		int min_result = 1;
		min_result = std::max(species->get_min_attribute_value(attribute), min_result);
		if (base_class != nullptr) {
			min_result = std::max(base_class->get_min_attribute_value(attribute), min_result);
		}

		const dice attribute_dice(3, 6);
		const int maximum_result = attribute_dice.get_maximum_result() + this->get_attribute_value(attribute);
		if (maximum_result < min_result) {
			throw std::runtime_error("Character \"{}\" of species \"{}\" cannot be generated with base class \"{}\", since it cannot possibly fulfill the attribute requirements.");
		}

		bool valid_result = false;
		while (!valid_result) {
			const int base_result = random::get()->roll_dice(attribute_dice);
			const int result = base_result + this->get_attribute_value(attribute);

			valid_result = result >= min_result;
			if (valid_result) {
				this->change_attribute_value(attribute, base_result);
			}
		}
	}

	for (const auto &[type, character_class] : this->character->get_character_classes()) {
		this->set_character_class(type, character_class);
	}

	const dice species_hit_dice(species->get_hit_dice_count(), creature_type->get_hit_dice().get_sides());
	if (!species_hit_dice.is_null()) {
		for (int i = 0; i < species_hit_dice.get_count(); ++i) {
			this->change_level(1);

			const int hit_dice_count = i + 1;

			this->change_base_attack_bonus(creature_type->get_base_attack_bonus_table()->get_bonus_per_level(hit_dice_count));

			if (!species->get_saving_throw_bonus_tables().empty()) {
				for (const auto &[saving_throw_type, saving_throw_bonus_table] : species->get_saving_throw_bonus_tables()) {
					this->change_saving_throw_bonus(saving_throw_type, saving_throw_bonus_table->get_bonus_per_level(hit_dice_count));
				}
			} else {
				for (const auto &[saving_throw_type, saving_throw_bonus_table] : creature_type->get_saving_throw_bonus_tables()) {
					this->change_saving_throw_bonus(saving_throw_type, saving_throw_bonus_table->get_bonus_per_level(hit_dice_count));
				}
			}

			this->apply_hit_dice(dice(1, species_hit_dice.get_sides()));
		}
	}

	while (level > this->get_level() || !this->has_level_in_classes() || !this->target_feats.empty()) {
		const bool changed = this->level_up();
		if (!changed) {
			break;
		}
	}
}

void character_game_data::apply_history(const QDate &start_date)
{
	try {
		const character_history *character_history = this->character->get_history();

		this->set_target_feats(character_history->get_feats());

		const int level = std::max(character_history->get_level(), 1);
		this->apply_species_and_class(level);

		if (this->character->is_alive_on_start_date(start_date)) {
			const kobold::character *spouse = character_history->get_spouse();
			if (spouse != nullptr && spouse->is_alive_on_start_date(start_date)) {
				this->set_spouse(spouse);
			}

			const kobold::country *country = character_history->get_country();
			if (country == nullptr) {
				if (this->get_spouse() != nullptr) {
					const kobold::character_history *spouse_history = this->get_spouse()->get_history();

					bool is_spouse_higher_ranking = false;
					if (spouse_history->get_office() != nullptr) {
						if (character_history->get_office() == nullptr) {
							is_spouse_higher_ranking = true;
						} else if (spouse_history->get_office() == defines::get()->get_ruler_office() && character_history->get_office() != defines::get()->get_ruler_office()) {
							is_spouse_higher_ranking = true;
						}
					} else {
						if (character_history->get_office() == nullptr && spouse_history->get_level() > character_history->get_level()) {
							is_spouse_higher_ranking = true;
						}
					}

					if (is_spouse_higher_ranking) {
						if (spouse_history->get_country() != nullptr) {
							country = spouse_history->get_country();
						} else if (this->get_spouse()->get_home_settlement() != nullptr && this->get_spouse()->get_home_settlement()->get_map_data()->is_on_map()) {
							country = this->get_spouse()->get_home_settlement()->get_game_data()->get_owner();
						}
					}
				}
			}

			if (country == nullptr) {
				if (this->character->get_home_settlement() != nullptr && this->character->get_home_settlement()->get_map_data()->is_on_map()) {
					country = this->character->get_home_settlement()->get_game_data()->get_owner();
				}
			}

			if (country != nullptr && country->get_game_data()->is_alive() && !country->get_game_data()->is_under_anarchy()) {
				this->set_country(country);

				const kobold::office *office = character_history->get_office();
				if (office != nullptr && country == character_history->get_country()) {
					assert_throw(this->get_country()->get_game_data()->get_office_holder(office) == nullptr);

					this->get_country()->get_game_data()->set_office_holder(office, this->character);

					const site *deployment_site = character_history->get_deployment_site();
					if (deployment_site != nullptr && deployment_site->get_map_data()->is_on_map()) {
						assert_throw(office->get_civilian_unit_class() != nullptr);

						const civilian_unit_type *civilian_unit_type = this->character->get_culture()->get_civilian_class_unit_type(office->get_civilian_unit_class());
						assert_throw(civilian_unit_type != nullptr);

						this->deploy(civilian_unit_type, deployment_site);
					}
				}
			}
		} else if (this->character->get_death_date().isValid() && this->character->get_death_date() <= start_date) {
			this->set_dead(true);
		}
	} catch (...) {
		std::throw_with_nested(std::runtime_error(std::format("Failed to apply history for character \"{}\".", this->character->get_identifier())));
	}
}

void character_game_data::on_setup_finished()
{
	this->check_portrait();
}

void character_game_data::do_events()
{
	const bool is_last_turn_of_year = game::get()->get_year() != game::get()->get_next_date().year();

	if (is_last_turn_of_year) {
		character_event::check_events_for_scope(this->character, event_trigger::yearly_pulse);
	}

	character_event::check_events_for_scope(this->character, event_trigger::quarterly_pulse);
	character_event::check_events_for_scope(this->character, event_trigger::per_turn_pulse);
}

std::string character_game_data::get_titled_name() const
{
	if (this->get_office() == nullptr) {
		return this->character->get_full_name();
	}

	return this->get_country()->get_game_data()->get_office_title_name(this->get_office()) + " " + this->character->get_full_name();
}

bool character_game_data::is_current_portrait_valid() const
{
	if (this->get_portrait() == nullptr) {
		return false;
	}

	if (this->get_portrait() == this->character->get_portrait()) {
		//this is the character's explicitly-defined portrait
		return true;
	}

	assert_throw(this->get_portrait()->get_character_conditions() != nullptr);

	if (!this->get_portrait()->get_character_conditions()->check(this->character, read_only_context(this->character))) {
		return false;
	}

	return true;
}

void character_game_data::check_portrait()
{
	if (this->is_current_portrait_valid()) {
		return;
	}

	std::vector<const kobold::portrait *> potential_portraits;

	for (const kobold::portrait *portrait : portrait::get_character_portraits()) {
		assert_throw(portrait->is_character_portrait());
		assert_throw(portrait->get_character_conditions() != nullptr);

		if (!portrait->get_character_conditions()->check(this->character, read_only_context(this->character))) {
			continue;
		}

		potential_portraits.push_back(portrait);
	}

	//there must always be an available portrait for characters which need them
	if (potential_portraits.empty()) {
		throw std::runtime_error(std::format("No portrait is suitable for character \"{}\".", this->character->get_identifier()));
	}

	this->portrait = vector::get_random(potential_portraits);
}

void character_game_data::set_country(const kobold::country *country)
{
	if (country == this->get_country()) {
		return;
	}

	if (this->get_civilian_unit() != nullptr) {
		this->civilian_unit->disband();
	}

	if (this->get_country() != nullptr) {
		this->get_country()->get_game_data()->remove_character(this->character);
	}

	this->country = country;

	if (this->get_country() != nullptr) {
		this->get_country()->get_game_data()->add_character(this->character);
	}

	if (game::get()->is_running()) {
		emit country_changed();
	}
}

int character_game_data::get_age() const
{
	const QDate &birth_date = this->character->get_birth_date();
	const QDate &current_date = game::get()->get_date();

	int age = current_date.year() - birth_date.year() - 1;

	const QDate current_birthday(current_date.year(), birth_date.month(), birth_date.day());
	if (current_date >= current_birthday) {
		++age;
	}

	return age;
}

bool character_game_data::is_adult() const
{
	return this->get_age() >= this->character->get_species()->get_adulthood_age();
}

void character_game_data::set_dead(const bool dead)
{
	if (dead == this->is_dead()) {
		return;
	}

	this->dead = dead;

	if (game::get()->is_running()) {
		emit dead_changed();
	}
}

void character_game_data::die()
{
	if (this->get_office() != nullptr) {
		this->get_country()->get_game_data()->on_office_holder_died(this->get_office(), this->character);
	}

	if (this->get_spouse() != nullptr) {
		this->get_spouse()->get_game_data()->set_spouse(nullptr);
	}

	if (this->get_country() != nullptr) {
		this->set_country(nullptr);
	}

	this->set_dead(true);
}

void character_game_data::set_character_class(const character_class_type type, const character_class *character_class)
{
	if (this->get_character_class(type) == character_class) {
		return;
	}

	if (character_class != nullptr) {
		assert_throw(type == character_class->get_type());
	}

	this->character_classes[type] = character_class;

	if (game::get()->is_running()) {
		emit character_classes_changed();
	}
}

bool character_game_data::choose_character_class(const character_class_type type)
{
	try {
		assert_throw(this->get_character_class(type) == nullptr);

		std::vector<const character_class *> potential_classes;

		for (const character_class *character_class : character_class::get_all_of_type(type)) {
			if (character_class->get_conditions() != nullptr && !character_class->get_conditions()->check(this->character, read_only_context(this->character))) {
				continue;
			}

			if (this->character == game::get()->get_player_character()) {
				potential_classes.push_back(character_class);
			} else {
				const int weight = std::max(this->character->get_species()->get_character_class_weight(character_class), 1);
				for (int i = 0; i < weight; ++i) {
					potential_classes.push_back(character_class);
				}
			}
		}

		if (potential_classes.empty()) {
			return true;
		}

		if (this->character == game::get()->get_player_character()) {
			emit engine_interface::get()->character_class_choosable(this->character, container::to_qvariant_list(potential_classes));
			return false;
		} else {
			const character_class *chosen_class = vector::get_random(potential_classes);
			this->set_character_class(type, chosen_class);
			return true;
		}
	} catch (...) {
		std::throw_with_nested(std::runtime_error(std::format("Failed to choose character class of type \"{}\" for character \"{}\".", magic_enum::enum_name(type), this->character->get_identifier())));
	}
}

void character_game_data::on_character_class_chosen(const kobold::character_class *character_class)
{
	const character_class_type type = character_class->get_type();
	assert_throw(this->get_character_class(type) == nullptr);

	this->set_character_class(type, character_class);

	const bool changed = this->gain_character_class_level();
	if (!changed) {
		return;
	}

	this->check_level_experience();
}

void character_game_data::change_level(const int change)
{
	if (change == 0) {
		return;
	}

	this->level += change;

	for (const auto &[skill, per_level_bonus] : this->get_skill_per_level_bonuses()) {
		this->change_skill_bonus(skill, change * per_level_bonus);
	}

	if (change > 0) {
		const int64_t level_experience = defines::get()->get_experience_for_level(this->get_level());
		if (this->get_experience() < level_experience) {
			this->set_experience(level_experience);
		}
	}

	if (game::get()->is_running()) {
		emit level_changed();
	}
}

void character_game_data::change_character_class_level(const character_class *character_class, const int change)
{
	if (change == 0) {
		return;
	}

	const int old_value = this->get_character_class_level(character_class);
	const int new_value = (this->character_class_levels[character_class] += change);
	if (new_value == 0) {
		this->character_class_levels.erase(character_class);
	}

	const int multiplier = number::sign(change);
	int levels_applied = 0;
	while ((old_value + levels_applied) != new_value) {
		const int affected_class_level = old_value + levels_applied + (multiplier > 0 ? multiplier : 0);
		this->on_class_level_gained(character_class, affected_class_level, multiplier);

		levels_applied += multiplier;
	}
}

void character_game_data::on_class_level_gained(const character_class *character_class, const int affected_class_level, const int multiplier)
{
	//only the effects of one level at a time should be applied
	assert_throw(std::abs(multiplier) == 1);

	//only gaining levels is possible at present
	assert_throw(multiplier > 0);

	//should not decrease a character's level below 1
	assert_throw(multiplier > 0 || this->get_level() > 1);

	this->change_level(multiplier);

	this->challenge_rating.change(multiplier);

	this->change_base_attack_bonus(character_class->get_base_attack_bonus_table()->get_bonus_per_level(affected_class_level) * multiplier);

	for (const auto &[saving_throw_type, saving_throw_bonus_table] : character_class->get_saving_throw_bonus_tables()) {
		this->change_saving_throw_bonus(saving_throw_type, saving_throw_bonus_table->get_bonus_per_level(affected_class_level) * multiplier);
	}

	if (affected_class_level == 1) {
		const modifier<const kobold::character> *modifier = character_class->get_modifier();
		if (modifier != nullptr) {
			this->apply_modifier(modifier, multiplier);
		}
	}

	const dice &hit_dice = character_class->get_hit_dice();
	this->apply_hit_dice(hit_dice);

	const effect_list<const kobold::character> *effects = character_class->get_level_effects(affected_class_level);
	if (effects != nullptr) {
		context ctx(this->character);
		effects->do_effects(this->character, ctx);
	}
}

int character_game_data::get_character_class_level_limit(const character_class *character_class) const
{
	int limit = character_class->get_max_level();

	switch (character_class->get_type()) {
		case character_class_type::epic_prestige_class:
			limit = std::min(limit, this->get_character_class_type_level(character_class_type::prestige_class));
			[[fallthrough]];
		case character_class_type::prestige_class:
			limit = std::min(limit, this->get_character_class_type_level(character_class_type::base_class));
			break;
		default:
			break;
	}

	return limit;
}

bool character_game_data::level_up()
{
	if (this->get_level() >= defines::get()->get_min_character_class_type_level(character_class_type::prestige_class) && this->get_character_class(character_class_type::prestige_class) == nullptr) {
		const bool completed = this->choose_character_class(character_class_type::prestige_class);
		if (!completed) {
			return false;
		}
	} else if (this->get_level() >= defines::get()->get_min_character_class_type_level(character_class_type::epic_prestige_class) && this->get_character_class(character_class_type::epic_prestige_class) == nullptr) {
		const bool completed = this->choose_character_class(character_class_type::epic_prestige_class);
		if (!completed) {
			return false;
		}
	}

	return this->gain_character_class_level();
}

bool character_game_data::gain_character_class_level()
{
	//character class types, by order of priority for leveling up
	static constexpr std::array character_class_types {
		character_class_type::racial_class,
		character_class_type::epic_prestige_class,
		character_class_type::prestige_class,
		character_class_type::base_class
	};

	const character_class *chosen_character_class = nullptr;

	for (const character_class_type character_class_type : character_class_types) {
		const character_class *character_class = this->get_character_class(character_class_type);

		if (character_class == nullptr) {
			continue;
		}

		if (this->get_level() < defines::get()->get_min_character_class_type_level(character_class_type)) {
			continue;
		}

		if (this->get_character_class_level_limit(character_class) > this->get_character_class_level(character_class)) {
			chosen_character_class = character_class;
			break;
		}
	}

	if (chosen_character_class != nullptr) {
		if (this->character == game::get()->get_player_character()) {
			const kobold::portrait *interior_minister_portrait = defines::get()->get_interior_minister_portrait();
			std::string level_effects_string = chosen_character_class->get_level_effects_string(this->get_character_class_level(chosen_character_class) + 1, this->character);

			const effect_list<const kobold::character> *hit_dice_count_effects = defines::get()->get_character_hit_dice_count_effects(this->get_hit_dice_count() + 1);
			if (hit_dice_count_effects != nullptr) {
				level_effects_string += "\n" + hit_dice_count_effects->get_effects_string(this->character, read_only_context(this->character));
			}

			engine_interface::get()->add_notification("Level Up", interior_minister_portrait, std::format("You have gained a level!\n\n{}", level_effects_string));
		}

		this->change_character_class_level(chosen_character_class, 1);

		return true;
	} else {

		return false;
	}
}

void character_game_data::check_level_experience()
{
	while (this->get_experience() >= defines::get()->get_experience_for_level(this->get_level() + 1)) {
		const bool changed = this->level_up();
		if (!changed) {
			break;
		}
	}
}

void character_game_data::change_experience(const int64_t change)
{
	if (change == 0) {
		return;
	}

	this->experience += change;

	if (game::get()->is_running()) {
		emit experience_changed();
	}

	this->check_level_experience();
}

void character_game_data::apply_hit_dice(const dice &hit_dice)
{
	assert_throw(hit_dice.get_count() == 1);

	this->change_hit_dice_count(1);

	if (this->get_hit_dice_count() == 1) {
		this->change_hit_points(hit_dice.get_maximum_result());
	} else {
		assert_throw(hit_dice.get_count() > 0);
		this->change_hit_points(random::get()->roll_dice(hit_dice));
	}

	if (this->get_hit_dice_count() == 1) {
		if (this->character->get_species()->get_creature_type()->get_effects() != nullptr) {
			context ctx(this->character);
			this->character->get_species()->get_creature_type()->get_effects()->do_effects(this->character, ctx);
		}

		if (this->character->get_species()->get_effects() != nullptr) {
			context ctx(this->character);
			this->character->get_species()->get_effects()->do_effects(this->character, ctx);
		}
	}

	const effect_list<const kobold::character> *effects = defines::get()->get_character_hit_dice_count_effects(this->get_hit_dice_count());
	if (effects != nullptr) {
		context ctx(this->character);
		effects->do_effects(this->character, ctx);
	}
}

QVariantList character_game_data::get_attribute_values_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_attribute_values());
}

void character_game_data::change_attribute_value(const character_attribute *attribute, const int change)
{
	if (change == 0) {
		return;
	}

	const bool office_attribute_value_changed = this->get_office() != nullptr && vector::contains(this->get_office()->get_character_attributes(), attribute);

	if (office_attribute_value_changed) {
		assert_throw(this->get_country() != nullptr);
		this->get_country()->get_game_data()->apply_office_holder(this->get_office(), this->character, -1);
	}

	const int new_value = (this->attribute_values[attribute] += change);
	if (new_value == 0) {
		this->attribute_values.erase(attribute);
	}

	if (office_attribute_value_changed) {
		assert_throw(this->get_country() != nullptr);
		this->get_country()->get_game_data()->apply_office_holder(this->get_office(), this->character, 1);
	}

	if (game::get()->is_running()) {
		emit attribute_values_changed();
	}
}

void character_game_data::change_hit_points(const int change)
{
	if (change == 0) {
		return;
	}

	this->hit_points += change;

	if (game::get()->is_running()) {
		emit hit_points_changed();
	}
}

void character_game_data::change_base_attack_bonus(const int change)
{
	if (change == 0) {
		return;
	}

	this->base_attack_bonus += change;

	this->change_attack_bonus(change);

	if (game::get()->is_running()) {
		emit base_attack_bonus_changed();
	}
}

void character_game_data::change_attack_bonus(const int change)
{
	if (change == 0) {
		return;
	}

	this->attack_bonus += change;

	if (game::get()->is_running()) {
		emit attack_bonus_changed();
	}
}

QVariantList character_game_data::get_weapon_type_attack_bonuses_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_weapon_type_attack_bonuses());
}

void character_game_data::change_weapon_type_attack_bonus(const item_type *type, const int change)
{
	assert(type != nullptr);
	assert(type->is_weapon());

	if (change == 0) {
		return;
	}

	const int new_value = (this->weapon_type_attack_bonuses[type] += change);
	if (new_value == 0) {
		this->weapon_type_attack_bonuses.erase(type);
	}

	if (game::get()->is_running()) {
		emit weapon_type_attack_bonuses_changed();
	}
}

void character_game_data::change_armor_class(const int change)
{
	if (change == 0) {
		return;
	}

	this->armor_class += change;

	if (game::get()->is_running()) {
		emit armor_class_changed();
	}
}

void character_game_data::change_initiative_bonus(const int change)
{
	if (change == 0) {
		return;
	}

	this->initiative_bonus += change;

	if (game::get()->is_running()) {
		emit initiative_bonus_changed();
	}
}

QVariantList character_game_data::get_saving_throw_bonuses_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_saving_throw_bonuses());
}

void character_game_data::change_saving_throw_bonus(const saving_throw_type *type, const int change)
{
	if (change == 0) {
		return;
	}

	const int new_value = (this->saving_throw_bonuses[type] += change);
	if (new_value == 0) {
		this->saving_throw_bonuses.erase(type);
	}

	if (game::get()->is_running()) {
		emit saving_throw_bonuses_changed();
	}
}

QVariantList character_game_data::get_skill_bonuses_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_skill_bonuses());
}

void character_game_data::change_skill_bonus(const skill *skill, const int change)
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

void character_game_data::change_skill_per_level_bonus(const skill *skill, const int change)
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

int character_game_data::get_skill_modifier(const skill *skill) const
{
	return this->get_skill_bonus(skill) + this->get_attribute_modifier(skill->get_attribute());
}

int character_game_data::do_skill_roll(const skill *skill) const
{
	return random::get()->roll_dice(dice(1, 20)) + this->get_skill_modifier(skill);
}

QVariantList character_game_data::get_damage_reductions_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_damage_reductions());
}

void character_game_data::change_damage_reduction(const damage_reduction_type *type, const int change)
{
	if (change == 0) {
		return;
	}

	const int new_value = (this->damage_reductions[type] += change);
	if (new_value == 0) {
		this->damage_reductions.erase(type);
	}

	if (game::get()->is_running()) {
		emit damage_reductions_changed();
	}
}

QVariantList character_game_data::get_feats_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_feat_counts());
}

data_entry_map<feat, int> character_game_data::get_feat_counts_of_type(const feat_type *feat_type) const
{
	data_entry_map<feat, int> feat_counts;

	for (const auto &[feat, count] : this->get_feat_counts()) {
		if (!vector::contains(feat->get_types(), feat_type)) {
			continue;
		}

		feat_counts[feat] = count;
	}

	return feat_counts;
}

QVariantList character_game_data::get_feats_of_type(const QString &feat_type_str) const
{
	const feat_type *type = feat_type::get(feat_type_str.toStdString());
	return archimedes::map::to_qvariant_list(this->get_feat_counts_of_type(type));
}

int character_game_data::get_feat_count_for_type(const feat_type *feat_type) const
{
	int total_count = 0;

	for (const auto &[feat, count] : this->get_feat_counts_of_type(feat_type)) {
		total_count += count;
	}

	return total_count;
}

bool character_game_data::can_have_feat(const feat *feat) const
{
	if (feat->get_conditions() != nullptr && !feat->get_conditions()->check(this->character, read_only_context(this->character))) {
		return false;
	}

	return true;
}

bool character_game_data::can_gain_feat(const feat *feat, const feat_type *choice_type) const
{
	if (this->has_feat(feat) && !feat->is_unlimited()) {
		return false;
	}

	if (feat->get_upgraded_feat() != nullptr && !this->has_feat(feat->get_upgraded_feat())) {
		return false;
	}

	for (const feat_type *type : feat->get_types()) {
		if (choice_type != nullptr && choice_type->ignores_other_type_conditions() && type != choice_type) {
			continue;
		}

		if (type->get_max_feats() > 0 && this->get_feat_count_for_type(type) >= type->get_max_feats()) {
			return false;
		}

		if (type->get_gain_conditions() != nullptr && !type->get_gain_conditions()->check(this->character, read_only_context(this->character))) {
			return false;
		}
	}

	return this->can_have_feat(feat);
}

bool character_game_data::has_feat(const feat *feat) const
{
	return this->get_feat_counts().contains(feat);
}

void character_game_data::change_feat_count(const feat *feat, const int change)
{
	if (change == 0) {
		return;
	}

	if (change > 0) {
		if (this->has_feat(feat) && !feat->is_unlimited()) {
			throw std::runtime_error(std::format("Tried to add non-unlimited feat \"{}\" to character \"{}\", but they already have the feat.", feat->get_identifier(), this->character->get_identifier()));
		}

		if (!this->can_have_feat(feat)) {
			throw std::runtime_error(std::format("Tried to add feat \"{}\" to character \"{}\", for which the feat's conditions are not fulfilled.", feat->get_identifier(), this->character->get_identifier()));
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

void character_game_data::on_feat_gained(const feat *feat, const int multiplier)
{
	assert_throw(multiplier != 0);

	if (multiplier > 0) {
		if (feat->get_upgraded_feat() != nullptr) {
			this->change_feat_count(feat->get_upgraded_feat(), -1);
		}

		if (feat->get_effects() != nullptr) {
			context ctx(this->character);
			feat->get_effects()->do_effects(this->character, ctx);
		}

		for (const feat_type *type : feat->get_types()) {
			if (type->get_effects() != nullptr) {
				context ctx(this->character);
				type->get_effects()->do_effects(this->character, ctx);
			}
		}
	}

	if (feat->get_modifier() != nullptr) {
		this->apply_modifier(feat->get_modifier(), multiplier);
	}
}

void character_game_data::choose_feat(const feat_type *type)
{
	try {
		std::vector<const feat *> potential_feats = this->get_potential_feats_from_list(vector::intersected(this->target_feats, type->get_feats()), type);

		if (potential_feats.empty()) {
			potential_feats = this->get_potential_feats_from_list(type->get_feats(), type);
		}

		assert_throw(!potential_feats.empty());

		if (this->character == game::get()->get_player_character()) {
			std::sort(potential_feats.begin(), potential_feats.end(), [](const kobold::feat *lhs, const kobold::feat *rhs) {
				return lhs->get_identifier() < rhs->get_identifier();
			});

			emit engine_interface::get()->feat_choosable(this->character, type, container::to_qvariant_list(potential_feats));
		} else {
			const feat *chosen_feat = vector::get_random(potential_feats);
			this->on_feat_chosen(chosen_feat);
		}
	} catch (...) {
		std::throw_with_nested(std::runtime_error(std::format("Failed to choose feat of type \"{}\" for character \"{}\".", type->get_identifier(), this->character->get_identifier())));
	}
}

void character_game_data::on_feat_chosen(const feat *feat)
{
	if (vector::contains(this->target_feats, feat)) {
		vector::remove_one(this->target_feats, feat);
	}

	this->change_feat_count(feat, 1);
}

std::vector<const feat *> character_game_data::get_potential_feats_from_list(const std::vector<const feat *> &feats, const feat_type *type) const
{
	const character_class *character_class = this->get_character_class();

	std::vector<const feat *> potential_feats;
	bool found_unacquired_feat = false;

	for (const feat *feat : feats) {
		if (!this->can_gain_feat(feat, type)) {
			continue;
		}

		if (this->character == game::get()->get_player_character()) {
			potential_feats.push_back(feat);
		} else {
			if (type->prioritizes_unacquired_feats()) {
				if (!found_unacquired_feat && !this->has_feat(feat)) {
					potential_feats.clear();
					found_unacquired_feat = true;
				} else if (found_unacquired_feat && this->has_feat(feat)) {
					continue;
				}
			}

			int weight = feat->get_weight_factor() != nullptr ? feat->get_weight_factor()->calculate(this->character).to_int() : 1;
			if (character_class != nullptr) {
				weight *= std::max(character_class->get_feat_weight(feat), 1);
			} else {
				weight *= std::max(this->character->get_species()->get_feat_weight(feat), 1);
			}

			for (int i = 0; i < weight; ++i) {
				potential_feats.push_back(feat);
			}
		}
	}

	return potential_feats;
}

QVariantList character_game_data::get_scripted_modifiers_qvariant_list() const
{
	return archimedes::map::to_qvariant_list(this->get_scripted_modifiers());
}

bool character_game_data::has_scripted_modifier(const scripted_character_modifier *modifier) const
{
	return this->get_scripted_modifiers().contains(modifier);
}

void character_game_data::add_scripted_modifier(const scripted_character_modifier *modifier, const int duration)
{
	const read_only_context ctx(this->character);

	this->scripted_modifiers[modifier] = std::max(this->scripted_modifiers[modifier], duration);

	if (modifier->get_modifier() != nullptr) {
		this->apply_modifier(modifier->get_modifier(), 1);
	}

	if (game::get()->is_running()) {
		emit scripted_modifiers_changed();
	}
}

void character_game_data::remove_scripted_modifier(const scripted_character_modifier *modifier)
{
	this->scripted_modifiers.erase(modifier);

	if (modifier->get_modifier() != nullptr) {
		this->apply_modifier(modifier->get_modifier(), -1);
	}

	if (game::get()->is_running()) {
		emit scripted_modifiers_changed();
	}
}

void character_game_data::decrement_scripted_modifiers()
{
	std::vector<const scripted_character_modifier *> modifiers_to_remove;
	for (auto &[modifier, duration] : this->scripted_modifiers) {
		--duration;

		if (duration == 0) {
			modifiers_to_remove.push_back(modifier);
		}
	}

	for (const scripted_character_modifier *modifier : modifiers_to_remove) {
		this->remove_scripted_modifier(modifier);
	}
}

bool character_game_data::is_ruler() const
{
	return this->get_country() != nullptr && this->get_country()->get_game_data()->get_ruler() == this->character;
}

void character_game_data::set_office(const kobold::office *office)
{
	if (office == this->get_office()) {
		return;
	}

	this->office = office;

	if (game::get()->is_running()) {
		emit office_changed();
	}
}

void character_game_data::set_spouse(const kobold::character *spouse)
{
	if (spouse == this->get_spouse()) {
		return;
	}

	const kobold::character *old_spouse = this->get_spouse();

	this->spouse = spouse;

	if (old_spouse != nullptr) {
		old_spouse->get_game_data()->set_spouse(nullptr);
	}

	if (spouse != nullptr) {
		spouse->get_game_data()->set_spouse(this->character);
	}

	if (game::get()->is_running()) {
		emit spouse_changed();
	}
}

void character_game_data::apply_modifier(const modifier<const kobold::character> *modifier, const int multiplier)
{
	assert_throw(modifier != nullptr);

	modifier->apply(this->character, multiplier);
}

QVariantList character_game_data::get_spells_qvariant_list() const
{
	return container::to_qvariant_list(this->get_spells());
}

bool character_game_data::can_learn_spell(const spell *spell) const
{
	bool has_suitable_class = false;
	for (const auto &[type, character_class] : this->get_character_classes()) {
		if (spell->is_available_for_character_class(character_class)) {
			has_suitable_class = true;
			break;
		}
	}
	if (!has_suitable_class) {
		return false;
	}

	if (this->has_learned_spell(spell)) {
		return false;
	}

	return true;
}

void character_game_data::learn_spell(const spell *spell)
{
	this->add_spell(spell);
}

QVariantList character_game_data::get_items_qvariant_list() const
{
	return container::to_qvariant_list(this->get_items());
}

void character_game_data::add_item(qunique_ptr<item> &&item)
{
	kobold::item *item_ptr = item.get();
	this->items.push_back(std::move(item));
	emit items_changed();

	if (this->can_equip_item(item_ptr, false)) {
		this->equip_item(item_ptr);
	}
}

void character_game_data::remove_item(item *item)
{
	if (item->is_equipped()) {
		this->deequip_item(item);
	}

	vector::remove(this->items, item);
	emit items_changed();
}

bool character_game_data::can_equip_item(const item *item, const bool ignore_already_equipped) const
{
	const item_slot *slot = item->get_slot();
	if (slot == nullptr) {
		return false;
	}

	const int item_slot_count = this->character->get_species()->get_item_slot_count(slot);
	if (ignore_already_equipped) {
		if (item_slot_count == 0) {
			return false;
		}
	} else {
		if (item_slot_count == this->get_equipped_item_count(slot)) {
			return false;
		}
	}

	return true;
}

void character_game_data::equip_item(item *item)
{
	assert_throw(item->get_slot() != nullptr);

	const int slot_count = this->character->get_species()->get_item_slot_count(item->get_slot());
	assert_throw(slot_count > 0);

	const int used_slots = this->get_equipped_item_count(item->get_slot());
	assert_throw(used_slots <= slot_count);
	if (used_slots == slot_count) {
		this->deequip_item(this->get_equipped_items(item->get_slot()).at(0));
	}

	this->equipped_items[item->get_slot()].push_back(item);

	item->set_equipped(true);

	this->on_item_equipped(item, 1);
}

void character_game_data::deequip_item(item *item)
{
	std::vector<kobold::item *> &equipped_items = this->equipped_items[item->get_slot()];
	std::erase(equipped_items, item);

	if (equipped_items.empty()) {
		this->equipped_items.erase(item->get_slot());
	}

	item->set_equipped(false);

	this->on_item_equipped(item, -1);
}

void character_game_data::on_item_equipped(const item *item, const int multiplier)
{
	const item_type *type = item->get_type();
	if (type->get_modifier() != nullptr) {
		type->get_modifier()->apply(this->character, multiplier);
	}

	if (item->get_enchantment() != nullptr) {
		this->on_item_equipped_with_enchantment(item->get_enchantment(), multiplier);
	}
}

void character_game_data::on_item_equipped_with_enchantment(const enchantment *enchantment, const int multiplier)
{
	if (enchantment->get_modifier() != nullptr) {
		enchantment->get_modifier()->apply(this->character, multiplier);
	}

	for (const kobold::enchantment *subenchantment : enchantment->get_subenchantments()) {
		this->on_item_equipped_with_enchantment(subenchantment, multiplier);
	}
}

void character_game_data::set_commanded_military_unit_stat_modifier(const military_unit_stat stat, const centesimal_int &value)
{
	const centesimal_int old_value = this->get_commanded_military_unit_stat_modifier(stat);

	if (value == old_value) {
		return;
	}

	if (value == 0) {
		this->commanded_military_unit_stat_modifiers.erase(stat);
	} else {
		this->commanded_military_unit_stat_modifiers[stat] = value;
	}
}

void character_game_data::set_commanded_military_unit_type_stat_modifier(const military_unit_type *type, const military_unit_stat stat, const centesimal_int &value)
{
	const centesimal_int old_value = this->get_commanded_military_unit_type_stat_modifier(type, stat);

	if (value == old_value) {
		return;
	}

	if (value == 0) {
		this->commanded_military_unit_type_stat_modifiers[type].erase(stat);

		if (this->commanded_military_unit_type_stat_modifiers[type].empty()) {
			this->commanded_military_unit_type_stat_modifiers.erase(type);
		}
	} else {
		this->commanded_military_unit_type_stat_modifiers[type][stat] = value;
	}
}

void character_game_data::set_civilian_unit(kobold::civilian_unit *civilian_unit)
{
	if (civilian_unit == this->get_civilian_unit()) {
		return;
	}

	if (civilian_unit != nullptr) {
		assert_throw(this->get_civilian_unit() == nullptr);
	} else {
		assert_throw(this->get_civilian_unit() != nullptr);
	}

	this->civilian_unit = civilian_unit;
}

void character_game_data::deploy(const civilian_unit_type *civilian_unit_type, const site *deployment_site)
{
	assert_throw(civilian_unit_type != nullptr);
	assert_throw(deployment_site != nullptr);
	assert_throw(this->get_country() != nullptr);
	assert_throw(this->get_country()->get_game_data()->is_alive());
	assert_throw(!this->get_country()->get_game_data()->is_under_anarchy());
	assert_throw(deployment_site->get_map_data()->get_tile()->get_civilian_unit() == nullptr);

	auto civilian_unit = make_qunique<kobold::civilian_unit>(civilian_unit_type, this->character);
	civilian_unit->set_tile_pos(deployment_site->get_map_data()->get_tile_pos());

	this->get_country()->get_game_data()->add_civilian_unit(std::move(civilian_unit));
}

}
