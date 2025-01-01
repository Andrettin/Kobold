#pragma once

#include "character/character.h"
#include "script/effect/scope_effect.h"
#include "util/assert_util.h"

namespace kobold {

template <typename upper_scope_type>
class character_scope_effect final : public scope_effect<upper_scope_type, const character>
{
public:
	explicit character_scope_effect(const gsml_operator effect_operator)
		: scope_effect<upper_scope_type, const character>(effect_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "character";
		return class_identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		if (property.get_key() == "identifier") {
			this->identifier = property.get_value();
		} else {
			scope_effect<upper_scope_type, const character>::process_gsml_property(property);
		}
	}

	virtual void check() const override
	{
		assert_throw(character::try_get(this->identifier) != nullptr);
	}

	virtual const character *get_scope(const upper_scope_type *upper_scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(upper_scope);
		Q_UNUSED(ctx);

		return character::get(this->identifier);
	}

	virtual const character *get_scope(const upper_scope_type *upper_scope, context &ctx) const override
	{
		Q_UNUSED(upper_scope);
		Q_UNUSED(ctx);

		return character::get(this->identifier);
	}

private:
	std::string identifier;
};

}
