#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

class armor_class_modifier_effect final : public modifier_effect<const character>
{
public:
	explicit armor_class_modifier_effect(const std::string &value)
		: modifier_effect<const character>(value)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "armor_class";
		return identifier;
	}

	virtual void apply(const character *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_armor_class((this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		return "Armor Class";
	}
};

}
