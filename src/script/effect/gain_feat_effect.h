#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat.h"
#include "script/effect/effect.h"
#include "util/string_util.h"

namespace kobold {

class flag;

class gain_feat_effect final : public effect<const character>
{
public:
	explicit gain_feat_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<const character>(effect_operator)
	{
		this->feat = feat::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "gain_feat";
		return class_identifier;
	}

	virtual void do_assignment_effect(const character *scope) const override
	{
		scope->get_game_data()->add_feat(this->feat);
	}

	virtual std::string get_assignment_string() const override
	{
		return std::format("Gain the {} feat", string::highlight(this->feat->get_name()));
	}

private:
	const kobold::feat *feat = nullptr;
};

}
