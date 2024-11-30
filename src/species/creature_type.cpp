#include "kobold.h"

#include "species/creature_type.h"

#include "character/level_bonus_table.h"
#include "character/saving_throw_type.h"
#include "character/starting_age_category.h"
#include "item/item_slot.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"

namespace kobold {

creature_type::creature_type(const std::string &identifier)
	: species_base(identifier)
{
}

creature_type::~creature_type()
{
}

void creature_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "saving_throw_bonus_tables") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->saving_throw_bonus_tables[saving_throw_type::get(key)] = level_bonus_table::get(value);
		});
	} else if (tag == "starting_age_modifiers") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->starting_age_modifiers[magic_enum::enum_cast<starting_age_category>(key).value()] = dice(value);
		});
	} else if (tag == "item_slots") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->item_slot_counts[item_slot::get(key)] = std::stoi(value);
		});
	} else if (tag == "modifier") {
		auto modifier = std::make_unique<kobold::modifier<const character>>();
		database::process_gsml_data(modifier, scope);
		this->modifier = std::move(modifier);
	} else if (tag == "effects") {
		auto effect_list = std::make_unique<kobold::effect_list<const character>>();
		database::process_gsml_data(effect_list, scope);
		this->effects = std::move(effect_list);
	} else {
		named_data_entry::process_gsml_scope(scope);
	}
}

void creature_type::check() const
{
	if (this->get_hit_dice().is_null()) {
		throw std::runtime_error(std::format("Creature type \"{}\" has no hit dice.", this->get_identifier()));
	} else if (this->get_hit_dice().get_count() != 1) {
		throw std::runtime_error(std::format("Creature type \"{}\" has hit dice with a count different than 1.", this->get_identifier()));
	}

	if (this->get_base_attack_bonus_table() == nullptr) {
		throw std::runtime_error(std::format("Creature type \"{}\" has no base attack bonus table.", this->get_identifier()));
	}

	if (this->get_saving_throw_bonus_tables().empty()) {
		throw std::runtime_error(std::format("Creature type \"{}\" has no saving throw bonus tables.", this->get_identifier()));
	}
}

const dice &creature_type::get_starting_age_modifier(const starting_age_category category) const
{
	const auto find_iterator = this->starting_age_modifiers.find(category);

	if (find_iterator != this->starting_age_modifiers.end()) {
		return find_iterator->second;
	}

	static constexpr dice dice;
	return dice;
}

}
