#include "kobold.h"

#include "species/species.h"

#include "character/character_attribute.h"
#include "character/character_class.h"
#include "character/character_class_type.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "species/taxon.h"
#include "species/taxonomic_rank.h"

namespace kobold {

species::species(const std::string &identifier)
	: taxon_base(identifier)
{
}

species::~species()
{
}

void species::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "starting_age_modifiers") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->starting_age_modifiers[character_class::get(key)] = dice(value);
		});
	} else if (tag == "min_attribute_values") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->min_attribute_values[character_attribute::get(key)] = std::stoi(value);
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
		taxon_base::process_gsml_scope(scope);
	}
}

void species::check() const
{
	if (this->get_supertaxon() == nullptr) {
		//throw std::runtime_error("Species \"" + this->get_identifier() + "\" has no supertaxon.");
	}

	if (this->is_sapient()) {
		for (const character_class *character_class : character_class::get_all()) {
			if (character_class->get_type() != character_class_type::base_class) {
				continue;
			}

			if (!this->starting_age_modifiers.contains(character_class)) {
				//throw std::runtime_error(std::format("Sapient species \"{}\" has no starting age modifier for base character class \"{}\".", this->get_identifier()));
			}
		}
	}
}

taxonomic_rank species::get_rank() const
{
	return taxonomic_rank::species;
}

const dice &species::get_starting_age_modifier(const character_class *character_class) const
{
	const auto find_iterator = this->starting_age_modifiers.find(character_class);

	if (find_iterator != this->starting_age_modifiers.end()) {
		return find_iterator->second;
	}

	throw std::runtime_error(std::format("Species \"{}\" has no starting age modifier for the character class \"{}\".", this->get_identifier(), character_class->get_identifier()));
}

}
