#include "kobold.h"

#include "species/species_base.h"

#include "database/database.h"

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

}
