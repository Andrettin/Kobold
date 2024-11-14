#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "util/dice.h"

Q_MOC_INCLUDE("character/level_bonus_table.h")

namespace kobold {

class level_bonus_table;
class saving_throw_type;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class creature_type final : public named_data_entry, public data_type<creature_type>
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
	std::unique_ptr<const kobold::modifier<const character>> modifier;
	std::unique_ptr<const effect_list<const character>> effects;
};

}