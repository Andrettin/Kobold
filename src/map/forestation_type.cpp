#include "kobold.h"

#include "map/forestation_type.h"

namespace archimedes {

template class enum_converter<kobold::forestation_type>;

template <>
const std::string enum_converter<kobold::forestation_type>::property_class_identifier = "kobold::forestation_type";

template <>
const std::map<std::string, kobold::forestation_type> enum_converter<kobold::forestation_type>::string_to_enum_map = {
	{ "none", kobold::forestation_type::none },
	{ "forest", kobold::forestation_type::forest }
};

template <>
const bool enum_converter<kobold::forestation_type>::initialized = enum_converter::initialize();

}
