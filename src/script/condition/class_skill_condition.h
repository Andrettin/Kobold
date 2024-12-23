#pragma once

#include "character/character.h"
#include "character/character_class.h"
#include "character/character_game_data.h"
#include "character/skill.h"
#include "script/condition/condition.h"
#include "species/species.h"

namespace kobold {

class class_skill_condition final : public condition<character>
{
public:
	explicit class_skill_condition(const std::string &value, const gsml_operator condition_operator)
		: condition<character>(condition_operator)
	{
		this->skill = skill::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "class_skill";
		return class_identifier;
	}

	virtual bool check_assignment(const character *scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(ctx);

		if (scope->get_species()->has_class_skill(this->skill)) {
			return true;
		}

		for (const auto &[character_class_type, character_class] : scope->get_game_data()->get_character_classes()) {
			if (character_class->has_class_skill(this->skill)) {
				return true;
			}
		}

		return false;
	}

	virtual std::string get_assignment_string(const size_t indent) const override
	{
		Q_UNUSED(indent);

		return this->skill->get_name() + " class skill";
	}

private:
	const kobold::skill *skill = nullptr;
};

}
