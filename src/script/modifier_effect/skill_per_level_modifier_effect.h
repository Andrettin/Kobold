#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/skill.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/country_skill.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

template <typename scope_type>
class skill_per_level_modifier_effect final : public modifier_effect<scope_type>
{
public:
	using skill_type = std::conditional_t<std::is_same_v<scope_type, const country>, country_skill, skill>;

	explicit skill_per_level_modifier_effect(const skill_type *skill, const std::string &value)
		: modifier_effect<scope_type>(value), skill(skill)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "skill_per_level";
		return identifier;
	}

	virtual void apply(scope_type *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_skill_per_level_bonus(this->skill, (this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		return std::format("{} per level", this->skill->get_name());
	}

private:
	const skill_type *skill = nullptr;
};

}
