#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat_type.h"
#include "script/effect/effect.h"
#include "util/string_util.h"

namespace kobold {

class flag;

class gain_feat_of_type_effect final : public effect<const character>
{
public:
	explicit gain_feat_of_type_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<const character>(effect_operator)
	{
		this->type = feat_type::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "gain_feat_of_type";
		return class_identifier;
	}

	virtual void do_assignment_effect(const character *scope) const override
	{
		scope->get_game_data()->choose_feat(this->type);
	}

	virtual std::string get_assignment_string() const override
	{
		return std::format("Gain {} {} feat", string::get_indefinite_article(this->type->get_name()), string::highlight(this->type->get_name()));
	}

private:
	const feat_type *type = nullptr;
};

}
