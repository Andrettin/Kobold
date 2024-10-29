#include "kobold.h"

#include "character/character_game_data.h"

#include "character/character.h"
#include "character/character_attribute.h"
#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/character_role.h"
#include "character/feat.h"
#include "character/feat_type.h"
#include "character/level_bonus_table.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "database/defines.h"
#include "game/character_event.h"
#include "game/event_trigger.h"
#include "game/game.h"
#include "map/province.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "script/scripted_character_modifier.h"
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

	this->portrait = this->character->get_portrait();
}

void character_game_data::on_setup_finished()
{
	this->check_portrait();
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
	if (this->character->get_role() != character_role::ruler) {
		//only rulers need portraits
		return;
	}

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

	this->country = country;

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
	this->set_country(nullptr);
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

	const dice &hit_dice = character_class->get_hit_dice();
	if (this->get_level() == 1 && multiplier > 0) {
		this->change_hit_points(hit_dice.get_maximum_result() * multiplier);
	} else {
		assert_throw(hit_dice.get_count() > 0);
		const int base_value = hit_dice.get_maximum_result() + hit_dice.get_count();
		int hit_points = base_value / 2;
		if (base_value % 2 == 1 && affected_class_level % 2 == 0) {
			hit_points += 1;
		}
		this->change_hit_points(hit_points * multiplier);
	}

	this->change_base_attack_bonus(character_class->get_base_attack_bonus_table()->get_bonus_per_level(affected_class_level) * multiplier);

	for (const auto &[saving_throw_type, saving_throw_bonus_table] : character_class->get_saving_throw_bonus_tables()) {
		this->change_saving_throw_bonus(saving_throw_type, saving_throw_bonus_table->get_bonus_per_level(affected_class_level) * multiplier);
	}

	if (multiplier > 0) {
		const effect_list<const kobold::character> *effects = defines::get()->get_character_level_effects(this->get_level());
		if (effects != nullptr) {
			context ctx(this->character);
			effects->do_effects(this->character, ctx);
		}
	}
}

void character_game_data::change_attribute_value(const character_attribute *attribute, const int change)
{
	if (change == 0) {
		return;
	}

	const int new_value = (this->attribute_values[attribute] += change);
	if (new_value == 0) {
		this->attribute_values.erase(attribute);
	}
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
}

QVariantList character_game_data::get_feats_qvariant_list() const
{
	return container::to_qvariant_list(this->get_feats());
}

std::vector<const feat *> character_game_data::get_feats_of_type(const feat_type *feat_type) const
{
	std::vector<const feat *> feats;

	for (const feat *feat : this->get_feats()) {
		if (!vector::contains(feat->get_types(), feat_type)) {
			continue;
		}

		feats.push_back(feat);
	}

	return feats;
}

QVariantList character_game_data::get_feats_of_type(const QString &feat_type_str) const
{
	const feat_type *type = feat_type::get(feat_type_str.toStdString());
	return container::to_qvariant_list(this->get_feats_of_type(type));
}

bool character_game_data::can_have_feat(const feat *feat) const
{
	if (feat->get_conditions() != nullptr && !feat->get_conditions()->check(this->character, read_only_context(this->character))) {
		return false;
	}

	return true;
}

bool character_game_data::can_gain_feat(const feat *feat) const
{
	if (this->has_feat(feat)) {
		return false;
	}

	for (const feat_type *type : feat->get_types()) {
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
	return vector::contains(this->get_feats(), feat);
}

void character_game_data::add_feat(const feat *feat)
{
	if (this->has_feat(feat)) {
		log::log_error("Tried to add feat \"" + feat->get_identifier() + "\" to character \"" + this->character->get_identifier() + "\", but they already have the feat.");
		return;
	}

	if (!this->can_have_feat(feat)) {
		log::log_error("Tried to add feat \"" + feat->get_identifier() + "\" to character \"" + this->character->get_identifier() + "\", for which the feat's conditions are not fulfilled.");
		return;
	}

	this->feats.push_back(feat);

	this->on_feat_gained(feat, 1);

	if (game::get()->is_running()) {
		emit feats_changed();
	}
}

void character_game_data::remove_feat(const feat *feat)
{
	std::erase(this->feats, feat);

	this->on_feat_gained(feat, -1);

	if (game::get()->is_running()) {
		emit feats_changed();
	}
}

void character_game_data::on_feat_gained(const feat *feat, const int multiplier)
{
	if (feat->get_effects() != nullptr && multiplier > 0) {
		context ctx(this->character);
		feat->get_effects()->do_effects(this->character, ctx);
	}

	if (feat->get_modifier() != nullptr) {
		this->apply_modifier(feat->get_modifier(), multiplier);
	}
}

void character_game_data::choose_feat(const feat_type *type)
{
	std::vector<const feat *> potential_feats;

	for (const feat *feat : type->get_feats()) {
		if (this->can_gain_feat(feat)) {
			potential_feats.push_back(feat);
		}
	}

	assert_throw(!potential_feats.empty());

	this->add_feat(vector::get_random(potential_feats));
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
