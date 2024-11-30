#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "species/species_base.h"
#include "util/dice.h"

Q_MOC_INCLUDE("character/level_bonus_table.h")

namespace kobold {

class item_slot;
class level_bonus_table;
class saving_throw_type;
enum class starting_age_category;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class creature_type final : public species_base, public data_type<creature_type>
{
	Q_OBJECT

	Q_PROPERTY(archimedes::dice hit_dice MEMBER hit_dice READ get_hit_dice NOTIFY changed)
	Q_PROPERTY(const kobold::level_bonus_table* base_attack_bonus_table MEMBER base_attack_bonus_table READ get_base_attack_bonus_table NOTIFY changed)
	Q_PROPERTY(int adulthood_age MEMBER adulthood_age READ get_adulthood_age NOTIFY changed)
	Q_PROPERTY(int middle_age MEMBER middle_age READ get_middle_age NOTIFY changed)
	Q_PROPERTY(int old_age MEMBER old_age READ get_old_age NOTIFY changed)
	Q_PROPERTY(int venerable_age MEMBER venerable_age READ get_venerable_age NOTIFY changed)
	Q_PROPERTY(archimedes::dice maximum_age_modifier MEMBER maximum_age_modifier READ get_maximum_age_modifier NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "creature_type";
	static constexpr const char property_class_identifier[] = "kobold::creature_type*";
	static constexpr const char database_folder[] = "creature_types";

	explicit creature_type(const std::string &identifier);
	~creature_type();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const dice &get_hit_dice() const
	{
		return this->hit_dice;
	}

	const level_bonus_table *get_base_attack_bonus_table() const
	{
		return this->base_attack_bonus_table;
	}

	const data_entry_map<saving_throw_type, const level_bonus_table *> &get_saving_throw_bonus_tables() const
	{
		return this->saving_throw_bonus_tables;
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

	const dice &get_starting_age_modifier(const starting_age_category category) const;

	int get_item_slot_count(const item_slot *slot) const
	{
		const auto find_iterator = this->item_slot_counts.find(slot);

		if (find_iterator != this->item_slot_counts.end()) {
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

private:
	dice hit_dice;
	const level_bonus_table *base_attack_bonus_table = nullptr;
	data_entry_map<saving_throw_type, const level_bonus_table *> saving_throw_bonus_tables;
	int adulthood_age = 0;
	int middle_age = 0;
	int old_age = 0;
	int venerable_age = 0;
	dice maximum_age_modifier;
	std::map<starting_age_category, dice> starting_age_modifiers;
	data_entry_map<item_slot, int> item_slot_counts;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
	std::unique_ptr<const effect_list<const character>> effects;
};

}
