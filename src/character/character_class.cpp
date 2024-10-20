#include "kobold.h"

#include "character/character_class.h"

namespace kobold {

character_class::character_class(const std::string &identifier)
	: named_data_entry(identifier)
{
}

character_class::~character_class()
{
}

void character_class::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "ruler_modifier") {
//		this->ruler_modifier = std::make_unique<modifier<const country>>();
	//	database::process_gsml_data(this->ruler_modifier, scope);
	} else if (tag == "scaled_ruler_modifier") {
		//this->scaled_ruler_modifier = std::make_unique<modifier<const country>>();
		//database::process_gsml_data(this->scaled_ruler_modifier, scope);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void character_class::check() const
{
	if (this->get_base_skill_points_per_level() == 0) {
		throw std::runtime_error(std::format("Character class \"{}\" has zero base skill points per level.", this->get_identifier()));
	}
}

}
