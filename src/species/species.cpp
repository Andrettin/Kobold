#include "kobold.h"

#include "species/species.h"

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

void species::check() const
{
	if (this->get_supertaxon() == nullptr) {
		//throw std::runtime_error("Species \"" + this->get_identifier() + "\" has no supertaxon.");
	}
}

taxonomic_rank species::get_rank() const
{
	return taxonomic_rank::species;
}

}
