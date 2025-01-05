#include "kobold.h"

#include "species/subspecies.h"

#include "script/modifier.h"
#include "species/species.h"

namespace kobold {

subspecies::subspecies(const std::string &identifier)
	: species_base(identifier)
{
}

subspecies::~subspecies()
{
}

void subspecies::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier" && scope.get_operator() == gsml_operator::addition) {
		auto modifier = std::make_unique<kobold::modifier<const character>>();
		database::process_gsml_data(modifier, scope);
		this->additional_modifier = std::move(modifier);
	} else {
		species_base::process_gsml_scope(scope);
	}
}

void subspecies::check() const
{
	if (this->get_species() == nullptr) {
		throw std::runtime_error(std::format("Subspecies \"{}\" has no species.", this->get_identifier()));
	}

	species_base::check();
}

taxon_base *subspecies::get_supertaxon() const
{
	return this->species;
}

const kobold::modifier<const character> *subspecies::get_modifier() const
{
	if (species_base::get_modifier() != nullptr) {
		return species_base::get_modifier();
	}

	return this->get_species()->get_modifier();
}

const kobold::effect_list<const character> *subspecies::get_effects() const
{
	if (species_base::get_effects() != nullptr) {
		return species_base::get_effects();
	}

	return this->get_species()->get_effects();
}

}
