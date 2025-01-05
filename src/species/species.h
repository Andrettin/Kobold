#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "species/species_base.h"

Q_MOC_INCLUDE("species/creature_size.h")
Q_MOC_INCLUDE("species/creature_type.h")

namespace kobold {

class character_class;
class creature_size;
class creature_type;
class feat;
class item_slot;
class level_bonus_table;
class saving_throw_type;
class subspecies;

class species final : public species_base, public data_type<species>
{
	Q_OBJECT

	Q_PROPERTY(kobold::creature_type* creature_type MEMBER creature_type NOTIFY changed)
	Q_PROPERTY(const kobold::creature_size* creature_size MEMBER creature_size READ get_creature_size NOTIFY changed)
	Q_PROPERTY(bool sapient MEMBER sapient READ is_sapient NOTIFY changed)
	Q_PROPERTY(const kobold::subspecies* default_subspecies MEMBER default_subspecies READ get_default_subspecies NOTIFY changed)
	Q_PROPERTY(int hit_dice_count MEMBER hit_dice_count READ get_hit_dice_count NOTIFY changed)
	Q_PROPERTY(int level_adjustment MEMBER level_adjustment READ get_level_adjustment NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "species";
	static constexpr const char property_class_identifier[] = "kobold::species*";
	static constexpr const char database_folder[] = "species";

	explicit species(const std::string &identifier);
	~species();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	virtual const species *get_species() const override
	{
		return this;
	}

	const kobold::creature_type *get_creature_type() const
	{
		return this->creature_type;
	}

	const kobold::creature_size *get_creature_size() const
	{
		return this->creature_size;
	}

	virtual bool is_sapient() const override
	{
		return this->sapient;
	}

	virtual taxon_base *get_supertaxon() const override;

	const subspecies *get_default_subspecies() const
	{
		return this->default_subspecies;
	}

	int get_hit_dice_count() const
	{
		return this->hit_dice_count;
	}

	int get_level_adjustment() const
	{
		return this->level_adjustment;
	}

	const std::vector<const phenotype *> &get_phenotypes() const
	{
		return this->phenotypes;
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

	int get_min_attribute_value(const character_attribute *attribute) const
	{
		const auto find_iterator = this->min_attribute_values.find(attribute);
		if (find_iterator != this->min_attribute_values.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	int get_item_slot_count(const item_slot *slot) const;

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
	kobold::creature_type *creature_type = nullptr;
	const kobold::creature_size *creature_size = nullptr;
	bool sapient = false;
	const subspecies *default_subspecies = nullptr;;
	int hit_dice_count = 0;
	int level_adjustment = 0;
	std::vector<const phenotype *> phenotypes;
	data_entry_map<saving_throw_type, const level_bonus_table *> saving_throw_bonus_tables;
	data_entry_set<skill> class_skills;
	data_entry_map<character_attribute, int> min_attribute_values;
	data_entry_map<item_slot, int> item_slot_counts;
	data_entry_map<character_class, int> character_class_weights;
	data_entry_map<feat, int> feat_weights;
};

}
