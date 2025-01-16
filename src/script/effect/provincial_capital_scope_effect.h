#pragma once

#include "script/effect/scope_effect.h"
#include "map/province.h"

namespace kobold {

template <typename upper_scope_type>
class provincial_capital_scope_effect final : public scope_effect<upper_scope_type, const site>
{
public:
	explicit provincial_capital_scope_effect(const gsml_operator effect_operator)
		: scope_effect<upper_scope_type, const site>(effect_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "provincial_capital";
		return class_identifier;
	}

	virtual const site *get_scope(const upper_scope_type *upper_scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(ctx);

		const province *province = effect<upper_scope_type>::get_scope_province(upper_scope);
		if (province == nullptr) {
			return nullptr;
		}

		return province->get_provincial_capital();
	}

	virtual const site *get_scope(const upper_scope_type *upper_scope, context &ctx) const override
	{
		Q_UNUSED(ctx);

		const province *province = effect<upper_scope_type>::get_scope_province(upper_scope);
		if (province == nullptr) {
			return nullptr;
		}

		return province->get_provincial_capital();
	}
};

}
