#include "kobold.h"

#include "species/species.h"

#include "character/character_attribute.h"
#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/level_bonus_table.h"
#include "character/saving_throw_type.h"
#include "character/skill.h"
#include "character/skill_category.h"
#include "item/item_slot.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "species/creature_size.h"
#include "species/creature_type.h"
#include "species/phenotype.h"

namespace kobold {

species::species(const std::string &identifier)
	: species_base(identifier)
{
}

species::~species()
{
}

void species::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "phenotypes") {
		for (const std::string &value : values) {
			this->phenotypes.push_back(phenotype::get(value));
		}
	} else if (tag == "saving_throw_bonus_tables") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->saving_throw_bonus_tables[saving_throw_type::get(key)] = level_bonus_table::get(value);
		});
	} else if (tag == "class_skills") {
		for (const std::string &value : values) {
			const skill *skill = skill::try_get(value);
			if (skill != nullptr) {
				this->class_skills.insert(skill);
			} else {
				const skill_category *skill_category = skill_category::get(value);
				for (const kobold::skill *category_skill : skill_category->get_skills()) {
					this->class_skills.insert(category_skill);
				}
			}
		}
	} else if (tag == "min_attribute_values") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->min_attribute_values[character_attribute::get(key)] = std::stoi(value);
		});
	} else if (tag == "item_slots") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->item_slot_counts[item_slot::get(key)] = std::stoi(value);
		});
	} else {
		species_base::process_gsml_scope(scope);
	}
}

void species::check() const
{
	if (this->get_creature_type() == nullptr) {
		throw std::runtime_error(std::format("Species \"{}\" has no creature type.", this->get_identifier()));
	}

	if (this->get_creature_size() == nullptr) {
		throw std::runtime_error(std::format("Species \"{}\" has no creature size.", this->get_identifier()));
	}

	if (this->get_hit_dice_count() == 0 && !this->get_class_skills().empty()) {
		throw std::runtime_error(std::format("Species \"{}\" has no hit dice count, but has class skills.", this->get_identifier()));
	}

	species_base::check();
}

taxon_base *species::get_supertaxon() const
{
	return this->creature_type;
}

int species::get_item_slot_count(const item_slot *slot) const
{
	const auto find_iterator = this->item_slot_counts.find(slot);

	if (find_iterator != this->item_slot_counts.end()) {
		return find_iterator->second;
	}

	return this->get_creature_type()->get_item_slot_count(slot);
}

}
