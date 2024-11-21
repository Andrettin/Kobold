#include "kobold.h"

#include "species/creature_size.h"

#include "script/modifier.h"

namespace kobold {

creature_size::creature_size(const std::string &identifier)
	: named_data_entry(identifier)
{
}

creature_size::~creature_size()
{
}

void creature_size::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier") {
		auto modifier = std::make_unique<kobold::modifier<const character>>();
		database::process_gsml_data(modifier, scope);
		this->modifier = std::move(modifier);
	} else {
		named_data_entry::process_gsml_scope(scope);
	}
}

}
