#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/skill.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

class skill_modifier_effect final : public modifier_effect<const character>
{
public:
	explicit skill_modifier_effect(const kobold::skill *skill, const std::string &value)
		: modifier_effect<const character>(value), skill(skill)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "skill";
		return identifier;
	}

	virtual void apply(const character *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_skill_bonus(this->skill, (this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		return std::string(this->skill->get_name());
	}

private:
	const kobold::skill *skill = nullptr;
};

}
