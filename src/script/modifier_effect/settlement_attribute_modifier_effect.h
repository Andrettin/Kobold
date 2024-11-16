#pragma once

#include "map/settlement_attribute.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

template <typename scope_type>
class settlement_attribute_modifier_effect final : public modifier_effect<scope_type>
{
public:
	explicit settlement_attribute_modifier_effect(const settlement_attribute *attribute, const std::string &value)
		: modifier_effect<scope_type>(value), attribute(attribute)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "settlement_attribute";
		return identifier;
	}

	virtual void apply(scope_type *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_settlement_attribute_value(this->attribute, (this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		if constexpr (std::is_same_v<scope_type, const site>) {
			return this->attribute->get_name();
		} else {
			return std::format("{} (Settlement)", this->attribute->get_name());
		}
	}

private:
	const settlement_attribute *attribute = nullptr;
};

}
