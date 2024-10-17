#include "kobold.h"

#include "map/elevation_type.h"

namespace archimedes {

template class enum_converter<kobold::elevation_type>;

template <>
const std::string enum_converter<kobold::elevation_type>::property_class_identifier = "kobold::elevation_type";

template <>
const std::map<std::string, kobold::elevation_type> enum_converter<kobold::elevation_type>::string_to_enum_map = {
	{ "none", kobold::elevation_type::none },
	{ "water", kobold::elevation_type::water },
	{ "flatlands", kobold::elevation_type::flatlands },
	{ "hills", kobold::elevation_type::hills },
	{ "mountains", kobold::elevation_type::mountains }
};

template <>
const bool enum_converter<kobold::elevation_type>::initialized = enum_converter::initialize();

}
