#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/country_feat.h"
#include "script/effect/effect.h"
#include "util/string_util.h"

namespace kobold {

template <typename scope_type>
class gain_feat_effect final : public effect<scope_type>
{
public:
	using feat_type = std::conditional_t<std::is_same_v<scope_type, const country>, country_feat, feat>;

	explicit gain_feat_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<scope_type>(effect_operator)
	{
		this->feat = feat_type::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "gain_feat";
		return class_identifier;
	}

	virtual void do_assignment_effect(scope_type *scope) const override
	{
		if (!scope->get_game_data()->can_gain_feat(this->feat, nullptr)) {
			return;
		}

		scope->get_game_data()->change_feat_count(this->feat, 1);
	}

	virtual std::string get_assignment_string() const override
	{
		return std::format("Gain the {} feat", string::highlight(this->feat->get_name()));
	}

private:
	const feat_type *feat = nullptr;
};

}
