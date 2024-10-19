#include "kobold.h"

#include "character/character_type.h"

#include "character/character_attribute.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "ui/portrait.h"
#include "unit/military_unit_category.h"
#include "util/assert_util.h"

namespace kobold {

character_type::character_type(const std::string &identifier)
	: named_data_entry(identifier), attribute(character_attribute::none), military_unit_category(military_unit_category::none)
{
}

character_type::~character_type()
{
}

void character_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "ruler_modifier") {
		this->ruler_modifier = std::make_unique<modifier<const country>>();
		database::process_gsml_data(this->ruler_modifier, scope);
	} else if (tag == "scaled_ruler_modifier") {
		this->scaled_ruler_modifier = std::make_unique<modifier<const country>>();
		database::process_gsml_data(this->scaled_ruler_modifier, scope);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void character_type::check() const
{
	if (this->get_attribute() == character_attribute::none) {
		throw std::runtime_error(std::format("Character type \"{}\" has no attribute.", this->get_identifier()));
	}
}

}
