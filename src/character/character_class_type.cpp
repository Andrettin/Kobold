#include "kobold.h"

#include "character/character_class_type.h"

namespace archimedes {

template class enum_converter<kobold::character_class_type>;

template <>
const std::string enum_converter<kobold::character_class_type>::property_class_identifier = "kobold::character_class_type";

template <>
const std::map<std::string, kobold::character_class_type> enum_converter<kobold::character_class_type>::string_to_enum_map = {
	{ "none", kobold::character_class_type::none },
	{ "racial_class", kobold::character_class_type::racial_class },
	{ "base_class", kobold::character_class_type::base_class },
	{ "prestige_class", kobold::character_class_type::prestige_class },
	{ "epic_prestige_class", kobold::character_class_type::epic_prestige_class }
};

template <>
const bool enum_converter<kobold::character_class_type>::initialized = enum_converter::initialize();

}
