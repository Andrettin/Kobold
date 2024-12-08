#pragma once

#include "character/character.h"
#include "species/species.h"
#include "script/condition/condition.h"

namespace kobold {

class species_condition final : public condition<character>
{
public:
	explicit species_condition(const std::string &value, const gsml_operator condition_operator)
		: condition<character>(condition_operator)
	{
		this->species = species::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "species";
		return class_identifier;
	}

	virtual bool check_assignment(const character *scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(ctx);

		return scope->get_species() == this->species;
	}

	virtual std::string get_assignment_string(const size_t indent) const override
	{
		Q_UNUSED(indent);

		return std::format("{} species", this->species->get_name());
	}

private:
	const kobold::species *species = nullptr;
};

}
