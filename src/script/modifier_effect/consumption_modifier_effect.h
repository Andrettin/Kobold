#pragma once

#include "country/country.h"
#include "country/country_game_data.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

class consumption_modifier_effect final : public modifier_effect<const country>
{
public:
	explicit consumption_modifier_effect(const std::string &value)
		: modifier_effect<const country>(value)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "consumption";
		return identifier;
	}

	virtual void apply(const country *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_consumption((this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		return "Consumption";
	}

	virtual bool is_negative(const centesimal_int &multiplier) const override
	{
		return (this->value * multiplier) > 0;
	}
};

}
