#include "kobold.h"

#include "country/culture.h"

#include "country/cultural_group.h"
#include "infrastructure/building_class.h"
#include "script/condition/and_condition.h"
#include "species/species.h"
#include "species/subspecies.h"
#include "util/assert_util.h"
#include "util/log_util.h"
#include "util/random.h"

namespace kobold {

culture::culture(const std::string &identifier) : culture_base(identifier)
{
}

culture::~culture()
{
}

void culture::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "species") {
		for (const std::string &value : values) {
			species_base *species = subspecies::try_get(value);
			if (species == nullptr) {
				species = species::get(value);
			}
			species->add_culture(this);
			this->species.push_back(species);
		}
	} else if (tag == "derived_cultures") {
		for (const std::string &value : values) {
			this->derived_cultures.push_back(culture::get(value));
		}
	} else {
		culture_base::process_gsml_scope(scope);
	}
}

void culture::initialize()
{
	if (!this->color.isValid()) {
		log::log_error("Culture \"" + this->get_identifier() + "\" has no color. A random one will be generated for it.");
		this->color = random::get()->generate_color();
	}

	culture_base::initialize();
}

void culture::check() const
{
	assert_throw(this->get_color().isValid());

	if (this->get_species().empty()) {
		throw std::runtime_error(std::format("Culture \"{}\" has no species set for it.", this->get_identifier()));
	}

	culture_base::check();
}

}
