#pragma once

#include "database/data_type.h"
#include "species/species_base.h"

Q_MOC_INCLUDE("species/species.h")

namespace kobold {

class species;

class subspecies final : public species_base, public data_type<subspecies>
{
	Q_OBJECT

	Q_PROPERTY(kobold::species* species MEMBER species NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "subspecies";
	static constexpr const char property_class_identifier[] = "kobold::subspecies*";
	static constexpr const char database_folder[] = "subspecies";

	explicit subspecies(const std::string &identifier);
	~subspecies();

	virtual void check() const override;

	virtual taxon_base *get_supertaxon() const override;

	virtual const kobold::species *get_species() const override
	{
		return this->species;
	}

	virtual const subspecies *get_subspecies() const override
	{
		return this;
	}

	virtual bool is_sapient() const override;

	virtual const effect_list<const character> *get_effects() const override;

private:
	kobold::species *species = nullptr;
};

}
