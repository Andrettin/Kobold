#pragma once

#include "country/country.h"
#include "country/country_game_data.h"
#include "economy/commodity.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

template <typename scope_type>
class commodity_throughput_modifier_effect final : public modifier_effect<scope_type>
{
public:
	explicit commodity_throughput_modifier_effect(const kobold::commodity *commodity, const std::string &value)
		: modifier_effect<scope_type>(value), commodity(commodity)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "commodity_throughput_modifier";
		return identifier;
	}

	virtual void apply(const scope_type *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_commodity_throughput_modifier(this->commodity, (this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		return std::format("{} Throughput", this->commodity->get_name());
	}

	virtual bool is_percent() const override
	{
		return true;
	}

private:
	const kobold::commodity *commodity = nullptr;
};

}
