#include "kobold.h"

#include "map/moisture_type.h"

namespace archimedes {

template class enum_converter<kobold::moisture_type>;

template <>
const std::string enum_converter<kobold::moisture_type>::property_class_identifier = "kobold::moisture_type";

template <>
const std::map<std::string, kobold::moisture_type> enum_converter<kobold::moisture_type>::string_to_enum_map = {
	{ "none", kobold::moisture_type::none },
	{ "arid", kobold::moisture_type::arid },
	{ "semi_arid", kobold::moisture_type::semi_arid },
	{ "dry", kobold::moisture_type::dry },
	{ "moist", kobold::moisture_type::moist },
	{ "wet", kobold::moisture_type::wet }
};

template <>
const bool enum_converter<kobold::moisture_type>::initialized = enum_converter::initialize();

}
