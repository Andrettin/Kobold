#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "species/taxon_base.h"
#include "util/dice.h"

namespace kobold {

class character_class;
class feat;
class taxon;
enum class taxonomic_rank;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class species final : public taxon_base, public data_type<species>
{
	Q_OBJECT

	Q_PROPERTY(bool sapient MEMBER sapient READ is_sapient NOTIFY changed)
	Q_PROPERTY(archimedes::dice hit_dice MEMBER hit_dice READ get_hit_dice NOTIFY changed)
	Q_PROPERTY(int level_adjustment MEMBER level_adjustment READ get_level_adjustment NOTIFY changed)
	Q_PROPERTY(int adulthood_age MEMBER adulthood_age READ get_adulthood_age NOTIFY changed)
	Q_PROPERTY(int middle_age MEMBER middle_age READ get_middle_age NOTIFY changed)
	Q_PROPERTY(int old_age MEMBER old_age READ get_old_age NOTIFY changed)
	Q_PROPERTY(int venerable_age MEMBER venerable_age READ get_venerable_age NOTIFY changed)
	Q_PROPERTY(archimedes::dice maximum_age_modifier MEMBER maximum_age_modifier READ get_maximum_age_modifier NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "species";
	static constexpr const char property_class_identifier[] = "kobold::species*";
	static constexpr const char database_folder[] = "species";

	explicit species(const std::string &identifier);
	~species();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	virtual taxonomic_rank get_rank() const override;

	virtual const std::string &get_common_name() const override
	{
		return this->get_name();
	}

	bool is_sapient() const
	{
		return this->sapient;
	}

	const dice &get_hit_dice() const
	{
		return this->hit_dice;
	}

	int get_level_adjustment() const
	{
		return this->level_adjustment;
	}

	const data_entry_set<skill> &get_class_skills() const
	{
		return this->class_skills;
	}

	bool has_class_skill(const skill *skill) const
	{
		return this->class_skills.contains(skill);
	}

	int get_adulthood_age() const
	{
		return this->adulthood_age;
	}

	int get_middle_age() const
	{
		return this->middle_age;
	}

	int get_old_age() const
	{
		return this->old_age;
	}

	int get_venerable_age() const
	{
		return this->venerable_age;
	}

	const dice &get_maximum_age_modifier() const
	{
		return this->maximum_age_modifier;
	}

	const dice &get_starting_age_modifier(const character_class *character_class) const;

	int get_min_attribute_value(const character_attribute *attribute) const
	{
		const auto find_iterator = this->min_attribute_values.find(attribute);
		if (find_iterator != this->min_attribute_values.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	const kobold::modifier<const character> *get_modifier() const
	{
		return this->modifier.get();
	}

	const effect_list<const character> *get_effects() const
	{
		return this->effects.get();
	}

	const data_entry_map<character_class, int> &get_character_class_weights() const
	{
		return this->character_class_weights;
	}

	int get_character_class_weight(const character_class *character_class) const
	{
		const auto find_iterator = this->character_class_weights.find(character_class);

		if (find_iterator != this->character_class_weights.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void add_character_class_weight(const character_class *character_class, const int weight)
	{
		this->character_class_weights[character_class] += weight;
	}

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

private:
	bool sapient = false;
	dice hit_dice;
	int level_adjustment = 0;
	data_entry_set<skill> class_skills;
	int adulthood_age = 0;
	int middle_age = 0;
	int old_age = 0;
	int venerable_age = 0;
	dice maximum_age_modifier;
	data_entry_map<character_class, dice> starting_age_modifiers;
	data_entry_map<character_attribute, int> min_attribute_values;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
	std::unique_ptr<const effect_list<const character>> effects;
	data_entry_map<character_class, int> character_class_weights;
	data_entry_map<feat, int> feat_weights;
};

}
