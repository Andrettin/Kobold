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

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	virtual taxon_base *get_supertaxon() const override;

	const kobold::species *get_species() const
	{
		return this->species;
	}

	virtual const kobold::modifier<const character> *get_modifier() const override;

	const kobold::modifier<const character> *get_additional_modifier() const
	{
		return this->additional_modifier.get();
	}

	virtual const effect_list<const character> *get_effects() const override;

private:
	kobold::species *species = nullptr;
	std::unique_ptr<const kobold::modifier<const character>> additional_modifier;
};

}
