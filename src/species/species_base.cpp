#include "kobold.h"

#include "species/species_base.h"

#include "character/starting_age_category.h"
#include "database/database.h"
#include "species/species.h"

#include <magic_enum/magic_enum_utility.hpp>

namespace kobold {

species_base::species_base(const std::string &identifier) : taxon_base(identifier)
{
}

species_base::~species_base()
{
}

void species_base::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier") {
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

void species_base::check() const
{
	if (this->is_sapient()) {
		const bool is_subspecies = this->get_subspecies() != nullptr;

		magic_enum::enum_for_each<starting_age_category>([&](const starting_age_category category) {
			if (category == starting_age_category::none) {
				return;
			}

			if (this->get_starting_age_modifier(category).is_null()) {
				throw std::runtime_error(std::format("Sapient {}species \"{}\" has no starting age modifier for starting age category \"{}\".", is_subspecies ? "sub": "", this->get_identifier(), magic_enum::enum_name(category)));
			}
		});

		if (this->get_cultures().empty() && (is_subspecies || this->get_species()->get_default_subspecies() == nullptr)) {
			throw std::runtime_error(std::format("Sapient {}species \"{}\" has no cultures set for it.", is_subspecies ? "sub" : "", this->get_identifier()));
		}
	}

	taxon_base::check();
}

}
