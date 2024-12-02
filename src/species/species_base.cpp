#include "kobold.h"

#include "species/species_base.h"

#include "character/starting_age_category.h"
#include "database/gsml_data.h"
#include "language/fallback_name_generator.h"
#include "language/gendered_name_generator.h"
#include "language/name_generator.h"
#include "util/gender.h"
#include "util/vector_util.h"

#include <magic_enum/magic_enum_utility.hpp>

namespace kobold {

species_base::species_base(const std::string &identifier) : named_data_entry(identifier)
{
}

species_base::~species_base()
{
}

void species_base::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "starting_age_modifiers") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->starting_age_modifiers[magic_enum::enum_cast<starting_age_category>(key).value()] = dice(value);
		});
	} else if (tag == "personal_names") {
		if (this->personal_name_generator == nullptr) {
			this->personal_name_generator = std::make_unique<gendered_name_generator>();
		}

		if (!values.empty()) {
			this->personal_name_generator->add_names(gender::none, values);
		}

		scope.for_each_child([&](const gsml_data &child_scope) {
			const std::string &tag = child_scope.get_tag();

			const gender gender = enum_converter<archimedes::gender>::to_enum(tag);

			this->personal_name_generator->add_names(gender, child_scope.get_values());
		});
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void species_base::initialize()
{
	if (this->get_supertaxon() != nullptr) {
		if (!this->get_supertaxon()->is_initialized()) {
			this->get_supertaxon()->initialize();
		}

		this->get_supertaxon()->add_personal_names_from(this);
	}

	if (this->personal_name_generator != nullptr) {
		fallback_name_generator::get()->add_personal_names(this->personal_name_generator);
		this->personal_name_generator->propagate_ungendered_names();
	}

	named_data_entry::initialize();
}


const dice &species_base::get_starting_age_modifier(const starting_age_category category) const
{
	const auto find_iterator = this->starting_age_modifiers.find(category);

	if (find_iterator != this->starting_age_modifiers.end()) {
		return find_iterator->second;
	}

	if (this->get_supertaxon() != nullptr) {
		return this->get_supertaxon()->get_starting_age_modifier(category);
	}

	static constexpr dice dice;
	return dice;
}

const name_generator *species_base::get_personal_name_generator(const gender gender) const
{
	const name_generator *name_generator = nullptr;

	if (this->personal_name_generator != nullptr) {
		name_generator = this->personal_name_generator->get_name_generator(gender);
	}
	if (name_generator != nullptr && name_generator->get_name_count() >= name_generator::minimum_name_count) {
		return name_generator;
	}

	if (this->get_supertaxon() != nullptr) {
		return this->get_supertaxon()->get_personal_name_generator(gender);
	}

	return fallback_name_generator::get()->get_personal_name_generator(gender);
}

void species_base::add_personal_name(const gender gender, const name_variant &name)
{
	if (this->personal_name_generator == nullptr) {
		this->personal_name_generator = std::make_unique<gendered_name_generator>();
	}

	this->personal_name_generator->add_name(gender, name);

	if (gender == gender::none) {
		this->personal_name_generator->add_name(gender::male, name);
		this->personal_name_generator->add_name(gender::female, name);
	}

	if (this->get_supertaxon() != nullptr) {
		this->get_supertaxon()->add_personal_name(gender, name);
	}
}

void species_base::add_personal_names_from(const species_base *other)
{
	if (other->personal_name_generator != nullptr) {
		if (this->personal_name_generator == nullptr) {
			this->personal_name_generator = std::make_unique<gendered_name_generator>();
		}

		this->personal_name_generator->add_names_from(other->personal_name_generator);
	}

	if (this->get_supertaxon() != nullptr) {
		this->get_supertaxon()->add_personal_names_from(other);
	}
}

}
