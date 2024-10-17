#include "kobold.h"

#include "character/trait_type.h"

namespace archimedes {

template class enum_converter<kobold::trait_type>;

template <>
const std::string enum_converter<kobold::trait_type>::property_class_identifier = "kobold::trait_type";

template <>
const std::map<std::string, kobold::trait_type> enum_converter<kobold::trait_type>::string_to_enum_map = {
	{ "none", kobold::trait_type::none },
	{ "ruler", kobold::trait_type::ruler },
	{ "background", kobold::trait_type::background },
	{ "personality", kobold::trait_type::personality },
	{ "expertise", kobold::trait_type::expertise }
};

template <>
const bool enum_converter<kobold::trait_type>::initialized = enum_converter::initialize();

}
