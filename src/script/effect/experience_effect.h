#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "script/effect/effect.h"

namespace kobold {

template <typename scope_type>
class experience_effect final : public effect<scope_type>
{
public:
	explicit experience_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<scope_type>(effect_operator)
	{
		this->quantity = std::stoll(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string identifier = "experience";
		return identifier;
	}

	virtual void do_assignment_effect(scope_type *scope) const override
	{
		scope->get_game_data()->set_experience(this->quantity);
	}

	virtual void do_addition_effect(scope_type *scope) const override
	{
		scope->get_game_data()->change_experience(this->quantity);
	}

	virtual void do_subtraction_effect(scope_type *scope) const override
	{
		scope->get_game_data()->change_experience(-this->quantity);
	}

	virtual std::string get_assignment_string() const override
	{
		return std::format("Set Experience to {}", number::to_formatted_string(this->quantity));
	}

	virtual std::string get_addition_string() const override
	{
		return std::format("Gain {} Experience", number::to_formatted_string(this->quantity));
	}

	virtual std::string get_subtraction_string() const override
	{
		return std::format("Lose {} Experience", number::to_formatted_string(this->quantity));
	}

private:
	int64_t quantity = 0;
};

}
