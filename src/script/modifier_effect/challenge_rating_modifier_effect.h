#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

class challenge_rating_modifier_effect final : public modifier_effect<const character>
{
public:
	explicit challenge_rating_modifier_effect(const std::string &value)
		: modifier_effect<const character>(value)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "challenge_rating";
		return identifier;
	}

	virtual void apply(const character *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_challenge_rating((this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		return "Challenge Rating";
	}
};

}
