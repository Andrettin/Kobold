#pragma once

#include "character/character.h"
#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/character_game_data.h"
#include "character/character_reference.h"
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
#include "script/target_variant.h"
#include "species/species.h"
#include "ui/portrait.h"
#include "util/number_util.h"
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
				this->enemy_characters.push_back(string_to_target_variant<const character>(value));
			}
		} else if (tag == "on_victory") {
			this->victory_effects = std::make_unique<effect_list<const character>>();
			database::process_gsml_data(this->victory_effects, scope);
		} else if (tag == "on_defeat") {
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
		std::vector<std::shared_ptr<character_reference>> generated_characters;

		for (const auto &[character_template, quantity] : this->enemies) {
			for (int i = 0; i < quantity; ++i) {
				std::shared_ptr<character_reference> enemy_character = character::generate_temporary(character_template, nullptr, nullptr, nullptr);
				enemy_characters.push_back(enemy_character->get_character());
				generated_characters.push_back(enemy_character);
			}
		}

		for (const target_variant<const character> &enemy_character : this->enemy_characters) {
			enemy_characters.push_back(this->get_enemy_character(enemy_character, ctx));
		}

		const game::combat_result result = this->attacker ? game::get()->do_combat(allied_characters, enemy_characters) : game::get()->do_combat(enemy_characters, allied_characters);

		const bool success = this->attacker ? result.attacker_victory : !result.attacker_victory;

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

		for (const target_variant<const character> &enemy_character : this->enemy_characters) {
			const character *character = this->get_enemy_character(enemy_character, ctx);
			std::string character_class_string;
			const character_class *character_class = character->get_game_data()->get_character_class();
			if (character_class != nullptr) {
				character_class_string += " ";
				if (character_class->get_type() != character_class_type::racial_class) {
					character_class_string += character_class->get_name() + " ";
				}
				character_class_string += character->get_game_data()->get_level();
			}
			str += "\n" + std::string(indent + 1, '\t') + std::format("{} ({}{})", character->get_full_name(), character->get_species()->get_name(), character_class_string);
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

	const character *get_enemy_character(const target_variant<const character> &target_variant, const read_only_context &ctx) const
	{
		if (std::holds_alternative<const character *>(target_variant)) {
			return std::get<const character *>(target_variant);
		} else if (std::holds_alternative<std::string>(target_variant)) {
			const std::string name = std::get<std::string>(target_variant);
			const character *character = ctx.get_saved_scope<const kobold::character>(name);
			return character;
		}

		assert_throw(false);
		return nullptr;
	}

private:
	bool attacker = true;
	data_entry_map<character_template, int> enemies;
	std::vector<target_variant<const character>> enemy_characters;
	std::unique_ptr<effect_list<const character>> victory_effects;
	std::unique_ptr<effect_list<const character>> defeat_effects;
};

}
