#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "util/dice.h"

Q_MOC_INCLUDE("character/level_bonus_table.h")

namespace kobold {

class level_bonus_table;
class saving_throw_type;

class character_class final : public named_data_entry, public data_type<character_class>
{
	Q_OBJECT

	Q_PROPERTY(archimedes::dice hit_dice MEMBER hit_dice READ get_hit_dice NOTIFY changed)
	Q_PROPERTY(const kobold::level_bonus_table* base_attack_bonus_table MEMBER base_attack_bonus_table READ get_base_attack_bonus_table NOTIFY changed)
	Q_PROPERTY(int base_skill_points_per_level MEMBER base_skill_points_per_level READ get_base_skill_points_per_level NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "character_class";
	static constexpr const char property_class_identifier[] = "kobold::character_class*";
	static constexpr const char database_folder[] = "character_classes";

	explicit character_class(const std::string &identifier);
	~character_class();

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

	const data_entry_map<saving_throw_type, const level_bonus_table *> &get_saving_throw_tables() const
	{
		return this->saving_throw_tables;
	}

	int get_base_skill_points_per_level() const
	{
		return this->base_skill_points_per_level;
	}

signals:
	void changed();

private:
	dice hit_dice;
	const level_bonus_table *base_attack_bonus_table = nullptr;
	data_entry_map<saving_throw_type, const level_bonus_table *> saving_throw_tables;
	int base_skill_points_per_level = 0;
};

}
