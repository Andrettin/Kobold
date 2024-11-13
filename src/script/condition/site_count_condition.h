#pragma once

#include "country/country.h"
#include "country/country_game_data.h"
#include "script/condition/numerical_condition.h"

namespace kobold {

class site_count_condition final : public numerical_condition<country, read_only_context>
{
public:
	explicit site_count_condition(const std::string &value, const gsml_operator condition_operator)
		: numerical_condition<country, read_only_context>(value, condition_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "site_count";
		return class_identifier;
	}

	virtual int get_scope_value(const country *scope) const override
	{
		return scope->get_game_data()->get_site_count();
	}

	virtual std::string get_value_name() const override
	{
		return "Site Count";
	}
};

}
