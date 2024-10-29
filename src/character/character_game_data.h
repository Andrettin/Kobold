#pragma once

#include "character/character_container.h"
#include "database/data_entry_container.h"
#include "script/scripted_modifier_container.h"
#include "spell/spell_container.h"
#include "unit/military_unit_type_container.h"
#include "util/fractional_int.h"

Q_MOC_INCLUDE("country/country.h")
Q_MOC_INCLUDE("ui/portrait.h")

namespace kobold {

class character;
class character_attribute;
class character_class;
class country;
class feat;
class feat_type;
class military_unit;
class military_unit_type;
class portrait;
class province;
class saving_throw_type;
class scripted_character_modifier;
class skill;
class spell;
enum class military_unit_stat;

template <typename scope_type>
class modifier;

class character_game_data final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(const kobold::portrait* portrait READ get_portrait NOTIFY portrait_changed)
	Q_PROPERTY(const kobold::country* country READ get_country NOTIFY country_changed)
	Q_PROPERTY(int age READ get_age NOTIFY age_changed)
	Q_PROPERTY(bool dead READ is_dead NOTIFY dead_changed)
	Q_PROPERTY(const kobold::character_class* character_class READ get_character_class NOTIFY character_classes_changed)
	Q_PROPERTY(QVariantList feats READ get_feats_qvariant_list NOTIFY feats_changed)
	Q_PROPERTY(QVariantList scripted_modifiers READ get_scripted_modifiers_qvariant_list NOTIFY scripted_modifiers_changed)
	Q_PROPERTY(bool ruler READ is_ruler NOTIFY ruler_changed)
	Q_PROPERTY(QVariantList spells READ get_spells_qvariant_list NOTIFY spells_changed)

public:
	explicit character_game_data(const kobold::character *character);

	void on_setup_finished();

	const kobold::portrait *get_portrait() const
	{
		return this->portrait;
	}

	bool is_current_portrait_valid() const;
	void check_portrait();

	const kobold::country *get_country() const
	{
		return this->country;
	}

	void set_country(const kobold::country *country);

	int get_age() const;

	bool is_dead() const
	{
		return this->dead;
	}

	void set_dead(const bool dead);
	void die();

	const std::map<character_class_type, const character_class *> &get_character_classes() const
	{
		return this->character_classes;
	}

	const character_class *get_character_class(const character_class_type type) const
	{
		const auto find_iterator = this->character_classes.find(type);

		if (find_iterator != this->character_classes.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	const character_class *get_character_class() const
	{
		if (!this->character_classes.empty()) {
			return std::prev(this->character_classes.end())->second;
		}

		return nullptr;
	}

	void set_character_class(const character_class_type type, const character_class *character_class);

	int get_level() const
	{
		return this->level;
	}

	void change_level(const int change)
	{
		this->level += change;
	}

	int get_character_class_level(const character_class *character_class) const
	{
		const auto find_iterator = this->character_class_levels.find(character_class);
		if (find_iterator != this->character_class_levels.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void change_character_class_level(const character_class *character_class, const int change);
	void on_class_level_gained(const character_class *character_class, const int affected_class_level, const int multiplier);

	int get_attribute_value(const character_attribute *attribute) const
	{
		const auto find_iterator = this->attribute_values.find(attribute);
		if (find_iterator != this->attribute_values.end()) {
			return std::max(0, find_iterator->second);
		}

		return 0;
	}

	void change_attribute_value(const character_attribute *attribute, const int change);

	int get_attribute_modifier(const character_attribute *attribute) const
	{
		const int attribute_value = this->get_attribute_value(attribute);
		int modifier = attribute_value;

		if (attribute_value >= 10) {
			modifier -= 10;
		} else {
			modifier -= 11;
		}

		modifier /= 2;
		return modifier;
	}

	int get_hit_points() const
	{
		return this->hit_points;
	}

	void change_hit_points(const int change)
	{
		this->hit_points += change;
	}

	int get_base_attack_bonus() const
	{
		return this->base_attack_bonus;
	}

	void change_base_attack_bonus(const int change)
	{
		this->base_attack_bonus += change;
	}

	int get_saving_throw_bonus(const saving_throw_type *type) const
	{
		const auto find_iterator = this->saving_throw_bonuses.find(type);
		if (find_iterator != this->saving_throw_bonuses.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void change_saving_throw_bonus(const saving_throw_type *type, const int change);

	int get_skill_bonus(const skill *skill) const
	{
		const auto find_iterator = this->skill_bonuses.find(skill);
		if (find_iterator != this->skill_bonuses.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void change_skill_bonus(const skill *skill, const int change);

	const std::vector<const feat *> &get_feats() const
	{
		return this->feats;
	}

	QVariantList get_feats_qvariant_list() const;

	std::vector<const feat *> get_feats_of_type(const feat_type *feat_type) const;
	Q_INVOKABLE QVariantList get_feats_of_type(const QString &feat_type_str) const;

	int get_feat_count_for_type(const feat_type *feat_type) const
	{
		return static_cast<int>(this->get_feats_of_type(feat_type).size());
	}

	bool can_have_feat(const feat *feat) const;
	bool can_gain_feat(const feat *feat) const;
	bool has_feat(const feat *feat) const;
	void add_feat(const feat *feat);
	void remove_feat(const feat *feat);
	void on_feat_gained(const feat *feat, const int multiplier);

	const scripted_character_modifier_map<int> &get_scripted_modifiers() const
	{
		return this->scripted_modifiers;
	}

	QVariantList get_scripted_modifiers_qvariant_list() const;
	bool has_scripted_modifier(const scripted_character_modifier *modifier) const;
	void add_scripted_modifier(const scripted_character_modifier *modifier, const int duration);
	void remove_scripted_modifier(const scripted_character_modifier *modifier);
	void decrement_scripted_modifiers();

	bool is_ruler() const;

	void apply_modifier(const modifier<const kobold::character> *modifier, const int multiplier);

	const spell_set &get_spells() const
	{
		return this->spells;
	}

	QVariantList get_spells_qvariant_list() const;

	bool has_spell(const spell *spell) const
	{
		return this->get_spells().contains(spell);
	}

	void add_spell(const spell *spell)
	{
		this->spells.insert(spell);
		emit spells_changed();
	}

	void remove_spell(const spell *spell)
	{
		this->spells.erase(spell);
		emit spells_changed();
	}

	bool can_learn_spell(const spell *spell) const;

	bool has_learned_spell(const spell *spell) const
	{
		return this->has_spell(spell);
	}

	void learn_spell(const spell *spell);

	const centesimal_int &get_commanded_military_unit_stat_modifier(const military_unit_stat stat) const
	{
		const auto find_iterator = this->commanded_military_unit_stat_modifiers.find(stat);

		if (find_iterator != this->commanded_military_unit_stat_modifiers.end()) {
			return find_iterator->second;
		}

		static constexpr centesimal_int zero;
		return zero;
	}

	void set_commanded_military_unit_stat_modifier(const military_unit_stat stat, const centesimal_int &value);

	void change_commanded_military_unit_stat_modifier(const military_unit_stat stat, const centesimal_int &change)
	{
		this->set_commanded_military_unit_stat_modifier(stat, this->get_commanded_military_unit_stat_modifier(stat) + change);
	}

	const centesimal_int &get_commanded_military_unit_type_stat_modifier(const military_unit_type *type, const military_unit_stat stat) const
	{
		const auto find_iterator = this->commanded_military_unit_type_stat_modifiers.find(type);

		if (find_iterator != this->commanded_military_unit_type_stat_modifiers.end()) {
			const auto sub_find_iterator = find_iterator->second.find(stat);

			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		static constexpr centesimal_int zero;
		return zero;
	}

	void set_commanded_military_unit_type_stat_modifier(const military_unit_type *type, const military_unit_stat stat, const centesimal_int &value);

	void change_commanded_military_unit_type_stat_modifier(const military_unit_type *type, const military_unit_stat stat, const centesimal_int &change)
	{
		this->set_commanded_military_unit_type_stat_modifier(type, stat, this->get_commanded_military_unit_type_stat_modifier(type, stat) + change);
	}

signals:
	void portrait_changed();
	void country_changed();
	void age_changed();
	void dead_changed();
	void character_classes_changed();
	void feats_changed();
	void scripted_modifiers_changed();
	void ruler_changed();
	void spells_changed();

private:
	const kobold::character *character = nullptr;
	const kobold::portrait *portrait = nullptr;
	const kobold::country *country = nullptr;
	bool dead = false;
	std::map<character_class_type, const character_class *> character_classes;
	int level = 0;
	data_entry_map<character_class, int> character_class_levels;
	data_entry_map<character_attribute, int> attribute_values;
	int hit_points = 0;
	int base_attack_bonus = 0;
	data_entry_map<saving_throw_type, int> saving_throw_bonuses;
	data_entry_map<skill, int> skill_bonuses;
	std::vector<const feat *> feats;
	scripted_character_modifier_map<int> scripted_modifiers;
	spell_set spells;
	spell_set item_spells;
	std::map<military_unit_stat, centesimal_int> commanded_military_unit_stat_modifiers;
	military_unit_type_map<std::map<military_unit_stat, centesimal_int>> commanded_military_unit_type_stat_modifiers;
};

}
