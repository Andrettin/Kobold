#pragma once

#include "country/country.h"
#include "country/country_game_data.h"
#include "script/effect/effect.h"

namespace kobold {

class experience_effect final : public effect<const country>
{
public:
	explicit experience_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<const country>(effect_operator)
	{
		this->quantity = std::stoi(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string identifier = "experience";
		return identifier;
	}

	virtual void do_assignment_effect(const country *scope) const override
	{
		scope->get_game_data()->set_experience(this->quantity);
	}

	virtual void do_addition_effect(const country *scope) const override
	{
		scope->get_game_data()->change_experience(this->quantity);
	}

	virtual void do_subtraction_effect(const country *scope) const override
	{
		scope->get_game_data()->change_experience(-this->quantity);
	}

	virtual std::string get_assignment_string() const override
	{
		return std::format("Set Experience to {}", std::to_string(this->quantity));
	}

	virtual std::string get_addition_string() const override
	{
		return std::format("Gain {} Experience", std::to_string(this->quantity));
	}

	virtual std::string get_subtraction_string() const override
	{
		return std::format("Lose {} Experience", std::to_string(this->quantity));
	}

private:
	int quantity = 0;
};

}
