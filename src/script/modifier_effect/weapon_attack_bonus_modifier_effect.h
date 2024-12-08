#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "item/item_type.h"
#include "script/modifier_effect/modifier_effect.h"

namespace kobold {

class weapon_attack_bonus_modifier_effect final : public modifier_effect<const character>
{
public:
	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "weapon_attack_bonus";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();

		if (key == "weapon_type") {
			this->weapon_type = item_type::get(value);
		} else if (key == "value") {
			this->value = centesimal_int(value);
		} else {
			modifier_effect::process_gsml_property(property);
		}
	}

	virtual void apply(const character *scope, const centesimal_int &multiplier) const override
	{
		scope->get_game_data()->change_weapon_type_attack_bonus(this->weapon_type, (this->value * multiplier).to_int());
	}

	virtual std::string get_base_string() const override
	{
		return std::format("{} Attack Bonus", this->weapon_type->get_name());
	}

private:
	const item_type *weapon_type = nullptr;
};

}
