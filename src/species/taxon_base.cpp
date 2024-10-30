#include "kobold.h"

#include "species/taxon_base.h"

#include "database/gsml_data.h"
#include "language/fallback_name_generator.h"
#include "language/gendered_name_generator.h"
#include "language/name_generator.h"
#include "species/taxon.h"
#include "util/gender.h"
#include "util/vector_util.h"

namespace kobold {

taxon_base::taxon_base(const std::string &identifier) : named_data_entry(identifier)
{
}

taxon_base::~taxon_base()
{
}

void taxon_base::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "specimen_names") {
		if (this->specimen_name_generator == nullptr) {
			this->specimen_name_generator = std::make_unique<gendered_name_generator>();
		}

		if (!values.empty()) {
			this->specimen_name_generator->add_names(gender::none, values);
		}

		scope.for_each_child([&](const gsml_data &child_scope) {
			const std::string &tag = child_scope.get_tag();

			const gender gender = enum_converter<archimedes::gender>::to_enum(tag);

			this->specimen_name_generator->add_names(gender, child_scope.get_values());
		});
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void taxon_base::initialize()
{
	if (this->get_supertaxon() != nullptr) {
		if (!this->get_supertaxon()->is_initialized()) {
			this->get_supertaxon()->initialize();
		}

		this->get_supertaxon()->add_specimen_names_from(this);
	}

	if (this->specimen_name_generator != nullptr) {
		fallback_name_generator::get()->add_specimen_names(this->specimen_name_generator);
		this->specimen_name_generator->propagate_ungendered_names();
	}

	named_data_entry::initialize();
}

const taxon *taxon_base::get_supertaxon_of_rank(const taxonomic_rank rank) const
{
	if (this->get_supertaxon() == nullptr) {
		return nullptr;
	}

	if (this->get_supertaxon()->get_rank() > rank) {
		return nullptr;
	}

	if (this->get_supertaxon()->get_rank() == rank) {
		return this->get_supertaxon();
	}

	return this->get_supertaxon()->get_supertaxon_of_rank(rank);
}

bool taxon_base::is_subtaxon_of(const taxon *other_taxon) const
{
	if (this->get_supertaxon() == nullptr) {
		return false;
	}

	if (other_taxon->get_rank() <= this->get_rank()) {
		return false;
	}

	if (other_taxon == this->get_supertaxon()) {
		return true;
	}

	return this->get_supertaxon()->is_subtaxon_of(other_taxon);
}

const name_generator *taxon_base::get_specimen_name_generator(const gender gender) const
{
	const name_generator *name_generator = nullptr;

	if (this->specimen_name_generator != nullptr) {
		name_generator = this->specimen_name_generator->get_name_generator(gender);
	}

	if (name_generator != nullptr && name_generator->get_name_count() >= name_generator::minimum_name_count) {
		return name_generator;
	}

	if (this->get_supertaxon() != nullptr) {
		return this->get_supertaxon()->get_specimen_name_generator(gender);
	}

	return name_generator;
}

void taxon_base::add_specimen_name(const gender gender, const name_variant &name)
{
	if (this->specimen_name_generator == nullptr) {
		this->specimen_name_generator = std::make_unique<gendered_name_generator>();
	}

	this->specimen_name_generator->add_name(gender, name);

	if (gender == gender::none) {
		this->specimen_name_generator->add_name(gender::male, name);
		this->specimen_name_generator->add_name(gender::female, name);
	}

	if (this->get_supertaxon() != nullptr) {
		this->get_supertaxon()->add_specimen_name(gender, name);
	}
}

void taxon_base::add_specimen_names_from(const taxon_base *other)
{
	if (other->specimen_name_generator != nullptr) {
		if (this->specimen_name_generator == nullptr) {
			this->specimen_name_generator = std::make_unique<gendered_name_generator>();
		}

		this->specimen_name_generator->add_names_from(other->specimen_name_generator);
	}

	if (this->get_supertaxon() != nullptr) {
		this->get_supertaxon()->add_specimen_names_from(other);
	}
}

}
