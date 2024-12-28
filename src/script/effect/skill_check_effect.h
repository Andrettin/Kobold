#pragma once

#include "character/skill.h"
#include "country/country_skill.h"
#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/context.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"

namespace kobold {

template <typename scope_type> 
class skill_check_effect final : public effect<scope_type>
{
public:
	using skill_type = std::conditional_t<std::is_same_v<scope_type, const country>, country_skill, skill>;

	explicit skill_check_effect(const gsml_operator effect_operator) : effect<scope_type>(effect_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string identifier = "skill_check";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		const std::string &key = property.get_key();

		if (key == "skill") {
			this->skill = skill_type::get(property.get_value());
		} else if (key == "difficulty_class") {
			this->difficulty_class = std::stoi(property.get_value());
		} else {
			effect<scope_type>::process_gsml_property(property);
		}
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		const std::string &tag = scope.get_tag();

		if (tag == "success") {
			this->success_effects = std::make_unique<effect_list<scope_type>>();
			database::process_gsml_data(this->success_effects, scope);
		} else if (tag == "failure") {
			this->failure_effects = std::make_unique<effect_list<scope_type>>();
			database::process_gsml_data(this->failure_effects, scope);
		} else {
			effect<scope_type>::process_gsml_scope(scope);
		}
	}

	virtual void do_assignment_effect(scope_type *scope, context &ctx) const override
	{
		const int roll_result = scope->get_game_data()->do_skill_roll(this->skill);
		const bool success = roll_result >= this->difficulty_class;

		if (success) {
			if (this->success_effects != nullptr) {
				this->success_effects->do_effects(scope, ctx);
			}
		} else {
			if (this->failure_effects != nullptr) {
				this->failure_effects->do_effects(scope, ctx);
			}
		}
	}

	virtual std::string get_assignment_string(scope_type *scope, const read_only_context &ctx, const size_t indent, const std::string &prefix) const override
	{
		std::string str = std::format("{} Check\n{}Difficulty Class: {}", this->skill->get_name(), std::string(indent + 1, '\t'), this->difficulty_class);

		if (this->success_effects != nullptr) {
			const std::string effects_string = this->success_effects->get_effects_string(scope, ctx, indent + 1, prefix);
			if (!effects_string.empty()) {
				str += "\n" + std::string(indent, '\t') + "If successful:\n" + effects_string;
			}
		}

		if (this->failure_effects != nullptr) {
			const std::string effects_string = this->failure_effects->get_effects_string(scope, ctx, indent + 1, prefix);
			if (!effects_string.empty()) {
				str += "\n" + std::string(indent, '\t') + "If failed:\n" + effects_string;
			}
		}

		return str;
	}

private:
	const skill_type *skill = nullptr;
	int difficulty_class = 0;
	std::unique_ptr<effect_list<scope_type>> success_effects;
	std::unique_ptr<effect_list<scope_type>> failure_effects;
};

}
