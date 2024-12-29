#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/character_template.h"
#include "character/skill.h"
#include "country/country_skill.h"
#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "game/game.h"
#include "script/context.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"
#include "util/assert_util.h"
#include "util/random.h"
#include "util/string_conversion_util.h"

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
		const std::string &value = property.get_value();

		if (key == "skill") {
			this->skill = skill_type::get(value);
		} else if (key == "opposed_skill") {
			this->opposed_skill = skill_type::get(value);
		} else if (key == "difficulty_class") {
			this->difficulty_class = std::stoi(value);
		} else if (key == "opposed_roll_modifier") {
			this->opposed_roll_modifier = std::stoi(value);
		} else {
			if constexpr (std::is_same_v<scope_type, const character>) {
				if (key == "opposed_character") {
					this->opposed_roller = character::get(value);
					return;
				} else if (key == "opposed_character_template") {
					this->opposed_character_template = character_template::get(value);
					return;
				}
			} else if constexpr (std::is_same_v<scope_type, const country>) {
				if (key == "opposed_country") {
					this->opposed_roller = country::get(value);
					return;
				}
			}

			effect<scope_type>::process_gsml_property(property);
		}
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		const std::string &tag = scope.get_tag();

		if (tag == "on_success") {
			this->success_effects = std::make_unique<effect_list<scope_type>>();
			database::process_gsml_data(this->success_effects, scope);
		} else if (tag == "on_failure") {
			this->failure_effects = std::make_unique<effect_list<scope_type>>();
			database::process_gsml_data(this->failure_effects, scope);
		} else {
			effect<scope_type>::process_gsml_scope(scope);
		}
	}

	virtual void check() const override
	{
		if (this->opposed_roller != nullptr) {
			assert_throw(this->opposed_character_template == nullptr);
			assert_throw(this->difficulty_class == 0);
		} else if (this->opposed_character_template != nullptr) {
			assert_throw(this->difficulty_class == 0);
		}
	}

	virtual void do_assignment_effect(scope_type *scope, context &ctx) const override
	{
		const int roll_result = scope->get_game_data()->do_skill_roll(this->skill);

		int difficulty_class = this->difficulty_class;

		std::remove_const_t<scope_type> *generated_roller = nullptr;
		scope_type *opposed_roller = this->opposed_roller;
		if constexpr (std::is_same_v<scope_type, const character>) {
			if (this->opposed_character_template != nullptr) {
				generated_roller = character::generate(this->opposed_character_template, nullptr, nullptr, nullptr);
				opposed_roller = generated_roller;
			}
		}

		if (opposed_roller != nullptr) {
			assert_throw(difficulty_class == 0);
			const int opposed_roll_result = opposed_roller->get_game_data()->do_skill_roll(this->opposed_skill) + this->opposed_roll_modifier;
			difficulty_class = opposed_roll_result + 1;
		}

		if constexpr (std::is_same_v<scope_type, const character>) {
			if (this->opposed_character_template != nullptr) {
				assert_throw(generated_roller != nullptr);
				game::get()->remove_generated_character(generated_roller);
			}
		}

		const bool success = roll_result >= difficulty_class;

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
		std::string str = std::format("{} ({}) Check", this->skill->get_name(), number::to_signed_string(scope->get_game_data()->get_skill_modifier(this->skill)));
		str += "\n" + std::string(indent + 1, '\t');

		if (this->opposed_roller != nullptr || this->opposed_character_template != nullptr) {
			std::remove_const_t<scope_type> *generated_roller = nullptr;
			scope_type *opposed_roller = this->opposed_roller;
			if constexpr (std::is_same_v<scope_type, const character>) {
				if (this->opposed_character_template != nullptr) {
					generated_roller = character::generate(this->opposed_character_template, nullptr, nullptr, nullptr);
					opposed_roller = generated_roller;
				}
			}

			if (opposed_roller != nullptr) {
				if constexpr (std::is_same_v<scope_type, const character>) {
					str += std::format("Opposed Character: {}", this->opposed_character_template ? this->opposed_character_template->get_name() : opposed_roller->get_full_name());
				} else if constexpr (std::is_same_v<scope_type, const country>) {
					str += std::format("Opposed Country: {}", opposed_roller->get_game_data()->get_name());
				} else {
					static_assert(false);
				}

				str += "\n" + std::string(indent + 1, '\t') + std::format("Opposed Skill: {} ({})", this->opposed_skill->get_name(), number::to_signed_string(opposed_roller->get_game_data()->get_skill_modifier(this->opposed_skill) + this->opposed_roll_modifier));
			}

			if constexpr (std::is_same_v<scope_type, const character>) {
				if (this->opposed_character_template != nullptr) {
					assert_throw(generated_roller != nullptr);
					game::get()->remove_generated_character(generated_roller);
				}
			}
		} else {
			str += std::format("Difficulty Class: {}", this->difficulty_class);
		}

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
	const skill_type *opposed_skill = nullptr;
	int difficulty_class = 0;
	scope_type *opposed_roller = nullptr;
	const character_template *opposed_character_template = nullptr;
	int opposed_roll_modifier = 0;
	std::unique_ptr<effect_list<scope_type>> success_effects;
	std::unique_ptr<effect_list<scope_type>> failure_effects;
};

}
