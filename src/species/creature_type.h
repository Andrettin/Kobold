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

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class creature_type final : public species_base, public data_type<creature_type>
{
	Q_OBJECT

	Q_PROPERTY(archimedes::dice hit_dice MEMBER hit_dice READ get_hit_dice NOTIFY changed)
	Q_PROPERTY(const kobold::level_bonus_table* base_attack_bonus_table MEMBER base_attack_bonus_table READ get_base_attack_bonus_table NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "creature_type";
	static constexpr const char property_class_identifier[] = "kobold::creature_type*";
	static constexpr const char database_folder[] = "creature_types";

	explicit creature_type(const std::string &identifier);
	~creature_type();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	virtual species_base *get_supertaxon() const override
	{
		return nullptr;
	}

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
	data_entry_map<item_slot, int> item_slot_counts;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
	std::unique_ptr<const effect_list<const character>> effects;
};

}
