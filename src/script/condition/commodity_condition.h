#pragma once

#include "country/country.h"
#include "country/country_game_data.h"
#include "economy/commodity.h"
#include "script/condition/numerical_condition.h"

namespace kobold {

template <typename scope_type>
class commodity_condition final : public numerical_condition<scope_type, read_only_context>
{
public:
	explicit commodity_condition(const kobold::commodity *commodity, const std::string &value, const gsml_operator condition_operator)
		: numerical_condition<scope_type, read_only_context>(commodity->string_to_value(value), condition_operator), commodity(commodity)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "commodity";
		return class_identifier;
	}

	virtual int get_scope_value(const scope_type *scope) const override
	{
		if constexpr (std::is_same_v<scope_type, character>) {
			if (!scope->get_game_data()->is_ruler()) {
				return 0;
			}
		}

		const country *country = condition<scope_type>::get_scope_country(scope);

		if (country == nullptr) {
			return 0;
		}

		return country->get_game_data()->get_stored_commodity(this->commodity);
	}

	virtual std::string get_value_name() const override
	{
		return this->commodity->get_name();
	}

private:
	const kobold::commodity *commodity = nullptr;
};

}
