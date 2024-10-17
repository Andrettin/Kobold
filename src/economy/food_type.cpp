#include "kobold.h"

#include "economy/food_type.h"

namespace archimedes {

template class enum_converter<kobold::food_type>;

template <>
const std::string enum_converter<kobold::food_type>::property_class_identifier = "kobold::food_type";

template <>
const std::map<std::string, kobold::food_type> enum_converter<kobold::food_type>::string_to_enum_map = {
	{ "none", kobold::food_type::none },
	{ "starch", kobold::food_type::starch },
	{ "meat", kobold::food_type::meat },
	{ "fruit", kobold::food_type::fruit }
};

template <>
const bool enum_converter<kobold::food_type>::initialized = enum_converter::initialize();

}
