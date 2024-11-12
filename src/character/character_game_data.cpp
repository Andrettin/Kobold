#include "kobold.h"

#include "character/character_game_data.h"

#include "character/character.h"
#include "character/character_attribute.h"
#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/character_history.h"
#include "character/feat.h"
#include "character/feat_type.h"
#include "character/level_bonus_table.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/office.h"
#include "database/defines.h"
#include "game/character_event.h"
#include "game/event_trigger.h"
#include "game/game.h"
#include "map/province.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "map/site_map_data.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/factor.h"
#include "script/modifier.h"
#include "script/scripted_character_modifier.h"
#include "species/species.h"
#include "spell/spell.h"
#include "ui/portrait.h"
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

		bool valid_result = false;
		while (!valid_result) {
			const int result = random::get()->roll_dice(dice(3, 6)) + this->get_attribute_value(attribute);

			valid_result = result >= min_result;
			if (valid_result) {
				this->change_attribute_value(attribute, result);
			}
		}
	}

	for (const auto &[type, character_class] : this->character->get_character_classes()) {
		this->set_character_class(type, character_class);
	}

	const dice &species_hit_dice = species->get_hit_dice();
	if (!species_hit_dice.is_null()) {
		for (int i = 0; i < species_hit_dice.get_count(); ++i) {
			this->change_level(1);
			this->apply_hit_dice(dice(1, species_hit_dice.get_sides()));
		}
	}

	if (this->character->get_character_class(character_class_type::base_class) != nullptr) {
		const int remaining_level = std::max(level - this->get_level(), 1);
		this->change_character_class_level(this->character->get_character_class(character_class_type::base_class), remaining_level);

		if (this->character->get_level() == 0) {
			while (!this->target_feats.empty()) {
				this->change_character_class_level(this->character->get_character_class(character_class_type::base_class), 1);
			}
		}
	}
}

void character_game_data::apply_history(const QDate &start_date)
{
	const character_history *character_history = this->character->get_history();

	this->set_target_feats(character_history->get_feats());

	const int level = std::max(character_history->get_level(), 1);
	this->apply_species_and_class(level);

	if (this->character->get_birth_date() <= start_date && this->character->get_death_date() > start_date) {
		const kobold::character *spouse = character_history->get_spouse();
		if (spouse != nullptr && spouse->get_death_date() > start_date) {
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
					} else if (this->get_spouse()->get_home_settlement()->get_map_data()->is_on_map()) {
						country = this->get_spouse()->get_home_settlement()->get_game_data()->get_owner();
					}
				}
			}
		}

		if (country == nullptr) {
			if (this->character->get_home_settlement()->get_map_data()->is_on_map()) {
				country = this->character->get_home_settlement()->get_game_data()->get_owner();
			}
		}

		if (country != nullptr) {
			this->set_country(country);

			if (character_history->get_office() != nullptr && country == character_history->get_country()) {
				assert_throw(this->get_country()->get_game_data()->get_office_holder(character_history->get_office()) == nullptr);

				this->get_country()->get_game_data()->set_office_holder(character_history->get_office(), this->character);
			}
		}
	} else if (character->get_death_date() <= start_date) {
		this->set_dead(true);
	}
}

void character_game_data::on_setup_finished()
{
	this->check_portrait();
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

void character_game_data::change_level(const int change)
{
	if (change == 0) {
		return;
	}

	this->level += change;

	for (const auto &[skill, per_level_bonus] : this->get_skill_per_level_bonuses()) {
		this->change_skill_bonus(skill, change * per_level_bonus);
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

	this->change_base_attack_bonus(character_class->get_base_attack_bonus_table()->get_bonus_per_level(affected_class_level) * multiplier);

	for (const auto &[saving_throw_type, saving_throw_bonus_table] : character_class->get_saving_throw_bonus_tables()) {
		this->change_saving_throw_bonus(saving_throw_type, saving_throw_bonus_table->get_bonus_per_level(affected_class_level) * multiplier);
	}

	const dice &hit_dice = character_class->get_hit_dice();
	this->apply_hit_dice(hit_dice);

	const effect_list<const kobold::character> *effects = character_class->get_level_effects(affected_class_level);
	if (effects != nullptr) {
		context ctx(this->character);
		effects->do_effects(this->character, ctx);
	}
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

void character_game_data::change_experience(const int change)
{
	if (change == 0) {
		return;
	}

	this->experience += change;

	if (game::get()->is_running()) {
		emit experience_changed();
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

	if (game::get()->is_running()) {
		emit base_attack_bonus_changed();
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
			log::log_error(std::format("Tried to add non-unlimited feat \"{}\" to character \"{}\", but they already have the feat.", feat->get_identifier(), this->character->get_identifier()));
			return;
		}

		if (!this->can_have_feat(feat)) {
			log::log_error(std::format("Tried to add feat \"{}\" to character \"{}\", for which the feat's conditions are not fulfilled.", feat->get_identifier(), this->character->get_identifier()));
			return;
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
	std::vector<const feat *> potential_feats = this->get_potential_feats_from_list(vector::intersected(this->target_feats, type->get_feats()), type);

	if (potential_feats.empty()) {
		potential_feats = this->get_potential_feats_from_list(type->get_feats(), type);
	}

	assert_throw(!potential_feats.empty());

	const feat *chosen_feat = vector::get_random(potential_feats);

	if (vector::contains(target_feats, chosen_feat)) {
		vector::remove_one(target_feats, chosen_feat);
	}

	this->change_feat_count(chosen_feat, 1);
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

}
