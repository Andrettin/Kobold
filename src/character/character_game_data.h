#pragma once

#include "character/challenge_rating.h"
#include "character/character_container.h"
#include "database/data_entry_container.h"
#include "script/scripted_modifier_container.h"
#include "spell/spell_container.h"
#include "unit/military_unit_type_container.h"
#include "util/fractional_int.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("character/character.h")
Q_MOC_INCLUDE("country/country.h")
Q_MOC_INCLUDE("country/office.h")
Q_MOC_INCLUDE("ui/portrait.h")

namespace archimedes {
	class dice;
}

namespace kobold {

class character;
class character_attribute;
class character_class;
class civilian_unit;
class civilian_unit_type;
class country;
class damage_reduction_type;
class enchantment;
class feat;
class feat_type;
class item;
class item_slot;
class item_type;
class military_unit;
class military_unit_type;
class office;
class portrait;
class province;
class saving_throw_type;
class scripted_character_modifier;
class skill;
class spell;
enum class character_class_type;
enum class military_unit_stat;

template <typename scope_type>
class modifier;

class character_game_data final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString titled_name READ get_titled_name_qstring NOTIFY titled_name_changed)
	Q_PROPERTY(const kobold::portrait* portrait READ get_portrait NOTIFY portrait_changed)
	Q_PROPERTY(const kobold::country* country READ get_country NOTIFY country_changed)
	Q_PROPERTY(int age READ get_age NOTIFY age_changed)
	Q_PROPERTY(bool dead READ is_dead NOTIFY dead_changed)
	Q_PROPERTY(const kobold::character_class* character_class READ get_character_class NOTIFY character_classes_changed)
	Q_PROPERTY(int level READ get_level NOTIFY level_changed)
	Q_PROPERTY(qint64 experience READ get_experience NOTIFY experience_changed)
	Q_PROPERTY(QString challenge_rating READ get_challenge_rating_qstring NOTIFY challenge_rating_changed)
	Q_PROPERTY(QVariantList attribute_values READ get_attribute_values_qvariant_list NOTIFY attribute_values_changed)
	Q_PROPERTY(int hit_points READ get_hit_points NOTIFY hit_points_changed)
	Q_PROPERTY(int base_attack_bonus READ get_base_attack_bonus NOTIFY base_attack_bonus_changed)
	Q_PROPERTY(int attack_bonus READ get_attack_bonus NOTIFY attack_bonus_changed)
	Q_PROPERTY(QVariantList weapon_type_attack_bonuses READ get_weapon_type_attack_bonuses_qvariant_list NOTIFY weapon_type_attack_bonuses_changed)
	Q_PROPERTY(int armor_class READ get_armor_class NOTIFY armor_class_changed)
	Q_PROPERTY(int initiative_bonus READ get_initiative_bonus NOTIFY initiative_bonus_changed)
	Q_PROPERTY(QVariantList saving_throw_bonuses READ get_saving_throw_bonuses_qvariant_list NOTIFY saving_throw_bonuses_changed)
	Q_PROPERTY(QVariantList skill_bonuses READ get_skill_bonuses_qvariant_list NOTIFY skill_bonuses_changed)
	Q_PROPERTY(QVariantList damage_reductions READ get_damage_reductions_qvariant_list NOTIFY damage_reductions_changed)
	Q_PROPERTY(QVariantList feats READ get_feats_qvariant_list NOTIFY feats_changed)
	Q_PROPERTY(QVariantList scripted_modifiers READ get_scripted_modifiers_qvariant_list NOTIFY scripted_modifiers_changed)
	Q_PROPERTY(bool ruler READ is_ruler NOTIFY office_changed)
	Q_PROPERTY(const kobold::office* office READ get_office NOTIFY office_changed)
	Q_PROPERTY(const kobold::character* spouse READ get_spouse NOTIFY spouse_changed)
	Q_PROPERTY(QVariantList spells READ get_spells_qvariant_list NOTIFY spells_changed)
	Q_PROPERTY(QVariantList items READ get_items_qvariant_list NOTIFY items_changed)

public:
	static constexpr int base_armor_class = 10;

	explicit character_game_data(const kobold::character *character);

	void apply_species_and_class(const int level);
	void apply_history(const QDate &start_date);
	void on_setup_finished();

	void do_events();

	std::string get_titled_name() const;

	QString get_titled_name_qstring() const
	{
		return QString::fromStdString(this->get_titled_name());
	}

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
	bool is_adult() const;

	bool is_dead() const
	{
		return this->dead;
	}

	void set_dead(const bool dead);
	void die();

	bool is_alive() const
	{
		return !this->is_dead();
	}

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

	[[nodiscard]]
	bool choose_character_class(const character_class_type type);

	Q_INVOKABLE void on_character_class_chosen(const kobold::character_class *character_class);

	int get_level() const
	{
		return this->level;
	}

	void change_level(const int change);

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
	int get_character_class_level_limit(const character_class *character_class) const;
	bool level_up();
	bool gain_character_class_level();
	void check_level_experience();

	int get_character_class_type_level(const character_class_type character_class_type) const
	{
		const character_class *character_class = this->get_character_class(character_class_type);
		if (character_class == nullptr) {
			return 0;
		}

		return this->get_character_class_level(character_class);
	}

	bool has_level_in_classes() const
	{
		for (const auto &[character_class_type, character_class] : this->character_classes) {
			if (this->get_character_class_level(character_class) == 0) {
				return false;
			}
		}

		return true;
	}

	int64_t get_experience() const
	{
		return this->experience;
	}

	void set_experience(const int64_t experience)
	{
		this->change_experience(experience - this->get_experience());
	}

	void change_experience(const int64_t change);

	int get_hit_dice_count() const
	{
		return this->hit_dice_count;
	}

	void change_hit_dice_count(const int change)
	{
		if (change == 0) {
			return;
		}

		this->hit_dice_count += change;
	}

	void apply_hit_dice(const dice &hit_dice);

	const kobold::challenge_rating &get_challenge_rating() const
	{
		return this->challenge_rating;
	}

	QString get_challenge_rating_qstring() const
	{
		return QString::fromStdString(this->get_challenge_rating().to_string());
	}

	void change_challenge_rating(const int change)
	{
		if (change == 0) {
			return;
		}

		this->challenge_rating.change(change);

		emit challenge_rating_changed();
	}

	const data_entry_map<character_attribute, int> &get_attribute_values() const
	{
		return this->attribute_values;
	}

	QVariantList get_attribute_values_qvariant_list() const;

	int get_attribute_value(const character_attribute *attribute) const
	{
		const auto find_iterator = this->attribute_values.find(attribute);
		if (find_iterator != this->attribute_values.end()) {
			return find_iterator->second;
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

	void change_hit_points(const int change);

	int get_base_attack_bonus() const
	{
		return this->base_attack_bonus;
	}

	void change_base_attack_bonus(const int change);

	int get_attack_bonus() const
	{
		return this->attack_bonus;
	}

	void change_attack_bonus(const int change);

	const data_entry_map<item_type, int> &get_weapon_type_attack_bonuses() const
	{
		return this->weapon_type_attack_bonuses;
	}

	QVariantList get_weapon_type_attack_bonuses_qvariant_list() const;

	int get_weapon_type_attack_bonus(const item_type *type) const
	{
		const auto find_iterator = this->weapon_type_attack_bonuses.find(type);
		if (find_iterator != this->weapon_type_attack_bonuses.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void change_weapon_type_attack_bonus(const item_type *type, const int change);

	int get_armor_class() const
	{
		return this->armor_class;
	}

	void change_armor_class(const int change);

	int get_initiative_bonus() const
	{
		return this->initiative_bonus;
	}

	void change_initiative_bonus(const int change);

	const data_entry_map<saving_throw_type, int> &get_saving_throw_bonuses() const
	{
		return this->saving_throw_bonuses;
	}

	QVariantList get_saving_throw_bonuses_qvariant_list() const;

	int get_saving_throw_bonus(const saving_throw_type *type) const
	{
		const auto find_iterator = this->saving_throw_bonuses.find(type);
		if (find_iterator != this->saving_throw_bonuses.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void change_saving_throw_bonus(const saving_throw_type *type, const int change);

	const data_entry_map<skill, int> &get_skill_bonuses() const
	{
		return this->skill_bonuses;
	}

	QVariantList get_skill_bonuses_qvariant_list() const;

	int get_skill_bonus(const skill *skill) const
	{
		const auto find_iterator = this->skill_bonuses.find(skill);
		if (find_iterator != this->skill_bonuses.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void change_skill_bonus(const skill *skill, const int change);

	const data_entry_map<skill, int> &get_skill_per_level_bonuses() const
	{
		return this->skill_per_level_bonuses;
	}

	void change_skill_per_level_bonus(const skill *skill, const int change);
	int get_skill_modifier(const skill *skill) const;
	int do_skill_roll(const skill *skill) const;

	const data_entry_map<damage_reduction_type, int> &get_damage_reductions() const
	{
		return this->damage_reductions;
	}

	QVariantList get_damage_reductions_qvariant_list() const;

	void change_damage_reduction(const damage_reduction_type *type, const int change);

	const data_entry_map<feat, int> &get_feat_counts() const
	{
		return this->feat_counts;
	}

	QVariantList get_feats_qvariant_list() const;

	data_entry_map<feat, int> get_feat_counts_of_type(const feat_type *feat_type) const;
	Q_INVOKABLE QVariantList get_feats_of_type(const QString &feat_type_str) const;
	int get_feat_count_for_type(const feat_type *feat_type) const;

	int get_feat_count(const feat *feat) const
	{
		const auto find_iterator = this->feat_counts.find(feat);

		if (find_iterator != this->feat_counts.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	bool can_have_feat(const feat *feat) const;
	bool can_gain_feat(const feat *feat, const feat_type *choice_type) const;
	bool has_feat(const feat *feat) const;
	void change_feat_count(const feat *feat, const int change);
	void on_feat_gained(const feat *feat, const int multiplier);
	void choose_feat(const feat_type *type);
	Q_INVOKABLE void on_feat_chosen(const kobold::feat *feat);

	std::vector<const feat *> get_potential_feats_from_list(const std::vector<const feat *> &feats, const feat_type *type) const;

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

	const kobold::office *get_office() const
	{
		return this->office;
	}

	void set_office(const kobold::office *office);

	const kobold::character *get_spouse() const
	{
		return this->spouse;
	}

	void set_spouse(const kobold::character *spouse);

	const std::vector<const character *> &get_companions() const
	{
		return this->companions;
	}

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

	const std::vector<qunique_ptr<item>> &get_items() const
	{
		return this->items;
	}

	QVariantList get_items_qvariant_list() const;
	void add_item(qunique_ptr<item> &&item);
	void remove_item(item *item);

	const std::vector<item *> &get_equipped_items(const item_slot *slot) const
	{
		const auto find_iterator = this->equipped_items.find(slot);

		if (find_iterator != this->equipped_items.end()) {
			return find_iterator->second;
		}

		static const std::vector<item *> empty_vector;
		return empty_vector;
	}

	int get_equipped_item_count(const item_slot *slot) const
	{
		return static_cast<int>(this->get_equipped_items(slot).size());
	}

	bool can_equip_item(const item *item, const bool ignore_already_equipped) const;
	void equip_item(item *item);
	void deequip_item(item *item);
	void on_item_equipped(const item *item, const int multiplier);
	void on_item_equipped_with_enchantment(const enchantment *enchantment, const int multiplier);

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

	void set_target_feats(const std::vector<const feat *> &feats)
	{
		this->target_feats = feats;
	}

	const kobold::civilian_unit *get_civilian_unit() const
	{
		return this->civilian_unit;
	}

	void set_civilian_unit(kobold::civilian_unit *civilian_unit);

	bool is_deployed() const
	{
		return this->get_civilian_unit() != nullptr;
	}

	void deploy(const civilian_unit_type *civilian_unit_type, const site *deployment_site);

signals:
	void titled_name_changed();
	void portrait_changed();
	void country_changed();
	void age_changed();
	void dead_changed();
	void character_classes_changed();
	void level_changed();
	void experience_changed();
	void challenge_rating_changed();
	void attribute_values_changed();
	void hit_points_changed();
	void base_attack_bonus_changed();
	void attack_bonus_changed();
	void weapon_type_attack_bonuses_changed();
	void armor_class_changed();
	void initiative_bonus_changed();
	void saving_throw_bonuses_changed();
	void skill_bonuses_changed();
	void damage_reductions_changed();
	void feats_changed();
	void scripted_modifiers_changed();
	void office_changed();
	void spouse_changed();
	void spells_changed();
	void items_changed();
	void equipped_items_changed();

private:
	const kobold::character *character = nullptr;
	const kobold::portrait *portrait = nullptr;
	const kobold::country *country = nullptr;
	bool dead = false;
	std::map<character_class_type, const character_class *> character_classes;
	int level = 0;
	int64_t experience = 0;
	data_entry_map<character_class, int> character_class_levels;
	int hit_dice_count = 0;
	kobold::challenge_rating challenge_rating;
	data_entry_map<character_attribute, int> attribute_values;
	int hit_points = 0;
	int base_attack_bonus = 0;
	int attack_bonus = 0;
	data_entry_map<item_type, int> weapon_type_attack_bonuses;
	int armor_class = character_game_data::base_armor_class;
	int initiative_bonus = 0;
	data_entry_map<saving_throw_type, int> saving_throw_bonuses;
	data_entry_map<skill, int> skill_bonuses;
	data_entry_map<skill, int> skill_per_level_bonuses;
	data_entry_map<damage_reduction_type, int> damage_reductions;
	data_entry_map<feat, int> feat_counts;
	scripted_character_modifier_map<int> scripted_modifiers;
	const kobold::office *office = nullptr;
	const kobold::character *spouse = nullptr;
	std::vector<const kobold::character *> companions; //characters who go adventuring together with this one
	spell_set spells;
	spell_set item_spells;
	std::vector<qunique_ptr<item>> items;
	data_entry_map<item_slot, std::vector<item *>> equipped_items;
	std::map<military_unit_stat, centesimal_int> commanded_military_unit_stat_modifiers;
	military_unit_type_map<std::map<military_unit_stat, centesimal_int>> commanded_military_unit_type_stat_modifiers;
	std::vector<const feat *> target_feats;
	kobold::civilian_unit *civilian_unit = nullptr;
};

}
