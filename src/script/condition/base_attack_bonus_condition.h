#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "script/condition/numerical_condition.h"

namespace kobold {

class base_attack_bonus_condition final : public numerical_condition<character, read_only_context>
{
public:
	explicit base_attack_bonus_condition(const std::string &value, const gsml_operator condition_operator)
		: numerical_condition<character, read_only_context>(value, condition_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "base_attack_bonus";
		return class_identifier;
	}

	virtual int get_scope_value(const character *scope) const override
	{
		return scope->get_game_data()->get_base_attack_bonus();
	}

	virtual std::string get_value_name() const override
	{
		return "Base Attack Bonus";
	}
};

}