#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat_type.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/country_feat_type.h"
#include "script/effect/effect.h"
#include "util/string_util.h"

namespace kobold {

template <typename scope_type>
class gain_feat_of_type_effect final : public effect<scope_type>
{
public:
	using feat_type_type = std::conditional_t<std::is_same_v<scope_type, const country>, country_feat_type, feat_type>;

	explicit gain_feat_of_type_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<scope_type>(effect_operator)
	{
		this->type = feat_type_type::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "gain_feat_of_type";
		return class_identifier;
	}

	virtual void do_assignment_effect(scope_type *scope) const override
	{
		scope->get_game_data()->choose_feat(this->type);
	}

	virtual std::string get_assignment_string() const override
	{
		return std::format("Gain {} {} feat", string::get_indefinite_article(this->type->get_name()), string::highlight(this->type->get_name()));
	}

private:
	const feat_type_type *type = nullptr;
};

}
