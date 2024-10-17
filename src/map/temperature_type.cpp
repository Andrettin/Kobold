#include "kobold.h"

#include "map/temperature_type.h"

namespace archimedes {

template class enum_converter<kobold::temperature_type>;

template <>
const std::string enum_converter<kobold::temperature_type>::property_class_identifier = "kobold::temperature_type";

template <>
const std::map<std::string, kobold::temperature_type> enum_converter<kobold::temperature_type>::string_to_enum_map = {
	{ "none", kobold::temperature_type::none },
	{ "frozen", kobold::temperature_type::frozen },
	{ "cold", kobold::temperature_type::cold },
	{ "temperate", kobold::temperature_type::temperate },
	{ "tropical", kobold::temperature_type::tropical }
};

template <>
const bool enum_converter<kobold::temperature_type>::initialized = enum_converter::initialize();

}
