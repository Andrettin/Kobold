#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "script/condition/numerical_condition.h"

namespace kobold {

template <typename scope_type>
class level_condition final : public numerical_condition<scope_type, read_only_context>
{
public:
	explicit level_condition(const std::string &value, const gsml_operator condition_operator)
		: numerical_condition<scope_type, read_only_context>(value, condition_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "level";
		return class_identifier;
	}

	virtual int get_scope_value(const scope_type *scope) const override
	{
		return scope->get_game_data()->get_level();
	}

	virtual std::string get_value_name() const override
	{
		return "level";
	}
};

}
