#include "kobold.h"

#include "species/subspecies.h"

#include "species/species.h"

namespace kobold {

subspecies::subspecies(const std::string &identifier)
	: species_base(identifier)
{
}

subspecies::~subspecies()
{
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

bool subspecies::is_sapient() const
{
	return this->get_species()->is_sapient();
}

const kobold::effect_list<const character> *subspecies::get_effects() const
{
	if (species_base::get_effects() != nullptr) {
		return species_base::get_effects();
	}

	return this->get_species()->get_effects();
}

}
