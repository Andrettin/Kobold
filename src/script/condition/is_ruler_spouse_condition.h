#pragma once

#include "character/character.h"
#include "script/condition/condition.h"
#include "util/string_conversion_util.h"

namespace kobold {

class is_ruler_spouse_condition final : public condition<character>
{
public:
	explicit is_ruler_spouse_condition(const std::string &value, const gsml_operator condition_operator)
		: condition<character>(condition_operator)
	{
		this->value = string::to_bool(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "is_ruler_spouse";
		return class_identifier;
	}

	virtual bool check_assignment(const character *scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(ctx);

		bool is_ruler_spouse = false;

		const character *spouse = scope->get_game_data()->get_spouse();
		if (spouse != nullptr && spouse->get_game_data()->is_ruler() && spouse->get_game_data()->get_country() == scope->get_game_data()->get_country()) {
			is_ruler_spouse = true;
		}

		return is_ruler_spouse == this->value;
	}

	virtual std::string get_assignment_string(const size_t indent) const override
	{
		Q_UNUSED(indent);

		if (this->value) {
			return "Ruler's spouse";
		} else {
			return "Not the ruler's spouse";
		}
	}

private:
	bool value = false;
};

}
