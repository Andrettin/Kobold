#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "script/context.h"
#include "script/effect/effect.h"
#include "spell/spell.h"
#include "unit/army.h"
#include "unit/military_unit.h"
#include "util/string_util.h"

namespace kobold {

class character;

class gain_spell_scroll_effect final : public effect<const country>
{
public:
	explicit gain_spell_scroll_effect(const std::string &value, const gsml_operator effect_operator) : effect<const country>(effect_operator)
	{
		this->spell = spell::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "gain_spell_scroll";
		return class_identifier;
	}

	const character *get_best_character(const country *scope, const read_only_context &ctx) const
	{
		Q_UNUSED(scope);
		Q_UNUSED(ctx);

		//get a suitable character from the ones participating in the adventure (if any)
		//otherwise, try any of the country's characters
		std::vector<const character *> potential_characters;

		if (!potential_characters.empty()) {
			return potential_characters.front();
		}

		if (!potential_characters.empty()) {
			return potential_characters.front();
		}

		return nullptr;
	}

	virtual void do_assignment_effect(const country *scope, context &ctx) const override
	{
		const character *character = this->get_best_character(scope, ctx);

		if (character == nullptr) {
			return;
		}

		character->get_game_data()->learn_spell(this->spell);
	}

	virtual std::string get_assignment_string(const country *scope, const read_only_context &ctx, const size_t indent, const std::string &prefix) const override
	{
		Q_UNUSED(indent);
		Q_UNUSED(prefix);

		const character *character = this->get_best_character(scope, ctx);

		if (character == nullptr) {
			return std::string();
		}

		return "Gain a Scroll of " + string::highlight(this->spell->get_name()) + " (the spell is learned by " + string::highlight(character->get_full_name()) + ")";
	}


private:
	const kobold::spell *spell = nullptr;
};

}
