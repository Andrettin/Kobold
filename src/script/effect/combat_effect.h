#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/character_template.h"
#include "database/data_entry_container.h"
#include "database/database.h"
#include "database/defines.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "engine_interface.h"
#include "game/game.h"
#include "script/context.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"
#include "ui/portrait.h"
#include "util/number_util.h"
#include "util/qunique_ptr.h"
#include "util/string_conversion_util.h"
#include "util/vector_util.h"

namespace kobold {

class combat_effect final : public effect<const character>
{
public:
	explicit combat_effect(const gsml_operator effect_operator) : effect<const character>(effect_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string identifier = "combat";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		const std::string &key = property.get_key();

		if (key == "attacker") {
			this->attacker = string::to_bool(property.get_value());
		} else {
			effect::process_gsml_property(property);
		}
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		const std::string &tag = scope.get_tag();
		const std::vector<std::string> &values = scope.get_values();

		if (tag == "enemies") {
			scope.for_each_property([&](const gsml_property &property) {
				const std::string &key = property.get_key();
				const character_template *character_template = character_template::get(key);

				const std::string &value = property.get_value();
				const int quantity = std::stoi(value);

				this->enemies[character_template] = quantity;
			});
		} else if (tag == "enemy_characters") {
			for (const std::string &value : values) {
				this->enemy_characters.push_back(character::get(value));
			}
		} else if (tag == "victory") {
			this->victory_effects = std::make_unique<effect_list<const character>>();
			database::process_gsml_data(this->victory_effects, scope);
		} else if (tag == "defeat") {
			this->defeat_effects = std::make_unique<effect_list<const character>>();
			database::process_gsml_data(this->defeat_effects, scope);
		} else {
			effect::process_gsml_scope(scope);
		}
	}

	virtual void do_assignment_effect(const character *scope, context &ctx) const override
	{
		std::vector<const character *> allied_characters;
		allied_characters.push_back(scope);

		std::vector<const character *> enemy_characters;
		std::vector<character *> generated_characters;

		for (const auto &[character_template, quantity] : this->enemies) {
			for (int i = 0; i < quantity; ++i) {
				character *enemy_character = character::generate(character_template, nullptr, nullptr, nullptr);
				enemy_characters.push_back(enemy_character);
				generated_characters.push_back(enemy_character);
			}
		}

		vector::merge(enemy_characters, this->enemy_characters);

		const game::combat_result result = this->attacker ? game::get()->do_combat(allied_characters, enemy_characters) : game::get()->do_combat(enemy_characters, allied_characters);

		const bool success = this->attacker ? result.attacker_victory : !result.attacker_victory;

		for (character *character : generated_characters) {
			game::get()->remove_generated_character(character);
		}

		if (scope == game::get()->get_player_character()) {
			const portrait *war_minister_portrait = defines::get()->get_war_minister_portrait();

			if (success) {
				std::string effects_string = std::format("Experience: {}", number::to_signed_string(result.experience_award));
				if (this->victory_effects != nullptr) {
					const std::string victory_effects_string = this->victory_effects->get_effects_string(scope, ctx);
					effects_string += "\n" + victory_effects_string;
				}

				engine_interface::get()->add_notification("Victory!", war_minister_portrait, std::format("You have won a combat!\n\n{}", effects_string));
			} else {
				std::string effects_string;
				if (this->defeat_effects != nullptr) {
					const std::string defeat_effects_string = this->defeat_effects->get_effects_string(scope, ctx);
					effects_string += "\n" + defeat_effects_string;
				}

				engine_interface::get()->add_notification("Defeat!", war_minister_portrait, std::format("You have lost a combat!{}", !effects_string.empty() ? ("\n\n" + effects_string) : ""));
			}
		}

		if (success) {
			if (this->victory_effects != nullptr) {
				this->victory_effects->do_effects(scope, ctx);
			}
		} else {
			if (this->defeat_effects != nullptr) {
				this->defeat_effects->do_effects(scope, ctx);
			}
		}
	}

	virtual std::string get_assignment_string(const character *scope, const read_only_context &ctx, const size_t indent, const std::string &prefix) const override
	{
		std::string str = "Combat against:";

		for (const auto &[character_template, quantity] : this->enemies) {
			str += "\n" + std::string(indent + 1, '\t') + std::to_string(quantity) + "x" + character_template->get_name();
		}

		if (this->victory_effects != nullptr) {
			const std::string effects_string = this->victory_effects->get_effects_string(scope, ctx, indent + 1, prefix);
			if (!effects_string.empty()) {
				str += "\n" + std::string(indent, '\t') + "If victorious:\n" + effects_string;
			}
		}

		if (this->defeat_effects != nullptr) {
			const std::string effects_string = this->defeat_effects->get_effects_string(scope, ctx, indent + 1, prefix);
			if (!effects_string.empty()) {
				str += "\n" + std::string(indent, '\t') + "If defeated:\n" + effects_string;
			}
		}

		return str;
	}

private:
	bool attacker = true;
	data_entry_map<character_template, int> enemies;
	std::vector<const character *> enemy_characters;
	std::unique_ptr<effect_list<const character>> victory_effects;
	std::unique_ptr<effect_list<const character>> defeat_effects;
};

}
