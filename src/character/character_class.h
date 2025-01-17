#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "util/dice.h"

Q_MOC_INCLUDE("character/level_bonus_table.h")
Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class icon;
class level_bonus_table;
class saving_throw_type;
enum class character_class_type;
enum class starting_age_category;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class character_class final : public named_data_entry, public data_type<character_class>
{
	Q_OBJECT

	Q_PROPERTY(kobold::character_class_type type MEMBER type READ get_type NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon READ get_icon NOTIFY changed)
	Q_PROPERTY(int max_level MEMBER max_level READ get_max_level NOTIFY changed)
	Q_PROPERTY(archimedes::dice hit_dice MEMBER hit_dice READ get_hit_dice NOTIFY changed)
	Q_PROPERTY(kobold::starting_age_category starting_age_category MEMBER starting_age_category READ get_starting_age_category NOTIFY changed)
	Q_PROPERTY(const kobold::level_bonus_table* base_attack_bonus_table MEMBER base_attack_bonus_table READ get_base_attack_bonus_table NOTIFY changed)
	Q_PROPERTY(int base_skill_points_per_level MEMBER base_skill_points_per_level READ get_base_skill_points_per_level NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "character_class";
	static constexpr const char property_class_identifier[] = "kobold::character_class*";
	static constexpr const char database_folder[] = "character_classes";

	static const std::set<std::string> database_dependencies;

	static const std::vector<const character_class *> &get_all_of_type(const kobold::character_class_type type)
	{
		const auto find_iterator = character_class::character_classes_by_type.find(type);
		if (find_iterator != character_class::character_classes_by_type.end()) {
			return find_iterator->second;
		}

		static const std::vector<const character_class *> empty_vector;
		return empty_vector;
	}

private:
	static inline std::map<character_class_type, std::vector<const character_class *>> character_classes_by_type;

public:
	explicit character_class(const std::string &identifier);
	~character_class();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	character_class_type get_type() const
	{
		return this->type;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	int get_max_level() const
	{
		return this->max_level;
	}

	const dice &get_hit_dice() const
	{
		return this->hit_dice;
	}

	kobold::starting_age_category get_starting_age_category() const
	{
		return this->starting_age_category;
	}

	const level_bonus_table *get_base_attack_bonus_table() const
	{
		return this->base_attack_bonus_table;
	}

	const data_entry_map<saving_throw_type, const level_bonus_table *> &get_saving_throw_bonus_tables() const
	{
		return this->saving_throw_bonus_tables;
	}

	const data_entry_set<skill> &get_class_skills() const
	{
		return this->class_skills;
	}

	bool has_class_skill(const skill *skill) const
	{
		return this->class_skills.contains(skill);
	}

	int get_base_skill_points_per_level() const
	{
		return this->base_skill_points_per_level;
	}

	int get_min_attribute_value(const character_attribute *attribute) const
	{
		const auto find_iterator = this->min_attribute_values.find(attribute);
		if (find_iterator != this->min_attribute_values.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	int get_rank_level(const std::string &rank) const
	{
		const auto find_iterator = this->rank_levels.find(rank);

		if (find_iterator != this->rank_levels.end()) {
			return find_iterator->second;
		}

		throw std::runtime_error(std::format("Invalid rank for class \"{}\": \"{}\".", this->get_identifier(), rank));
	}

	const and_condition<character> *get_conditions() const
	{
		return this->conditions.get();
	}

	const kobold::modifier<const character> *get_modifier() const
	{
		return this->modifier.get();
	}

	const effect_list<const character> *get_level_effects(const int level) const
	{
		const auto find_iterator = this->level_effects.find(level);
		if (find_iterator != this->level_effects.end()) {
			return find_iterator->second.get();
		}

		return nullptr;
	}

	std::string get_level_effects_string(const int level, const kobold::character *character) const;
	Q_INVOKABLE QString get_tooltip(const kobold::character *character) const;

	const data_entry_map<feat, int> &get_feat_weights() const
	{
		return this->feat_weights;
	}

	int get_feat_weight(const feat *feat) const
	{
		const auto find_iterator = this->feat_weights.find(feat);

		if (find_iterator != this->feat_weights.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void add_feat_weight(const feat *feat, const int weight)
	{
		this->feat_weights[feat] += weight;
	}

signals:
	void changed();

private:
	character_class_type type{};
	const kobold::icon *icon = nullptr;
	int max_level = 0;
	dice hit_dice;
	kobold::starting_age_category starting_age_category{};
	const level_bonus_table *base_attack_bonus_table = nullptr;
	data_entry_map<saving_throw_type, const level_bonus_table *> saving_throw_bonus_tables;
	data_entry_set<skill> class_skills;
	int base_skill_points_per_level = 0;
	data_entry_map<character_attribute, int> min_attribute_values;
	std::map<std::string, int> rank_levels; //names for particular levels
	std::unique_ptr<and_condition<character>> conditions;
	std::unique_ptr<kobold::modifier<const character>> modifier;
	std::map<int, std::unique_ptr<const effect_list<const character>>> level_effects;
	data_entry_map<feat, int> feat_weights;
};

}
