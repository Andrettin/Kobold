#pragma once

#include "country/country.h"
#include "country/country_attribute.h"
#include "country/country_game_data.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

class country_attribute_modifier_effect final : public modifier_effect<const country>
{
public:
	explicit country_attribute_modifier_effect(const country_attribute *attribute, const std::string &value)
		: modifier_effect<const country>(value), attribute(attribute)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "country_attribute";
		return identifier;
	}

	virtual void apply(const country *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_attribute_value(this->attribute, (this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		return std::string(this->attribute->get_name());
	}

private:
	const country_attribute *attribute = nullptr;
};

}
