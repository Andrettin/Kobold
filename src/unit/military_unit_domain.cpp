#include "kobold.h"

#include "unit/military_unit_domain.h"

namespace archimedes {

template class enum_converter<kobold::military_unit_domain>;

template <>
const std::string enum_converter<kobold::military_unit_domain>::property_class_identifier = "kobold::military_unit_domain";

template <>
const std::map<std::string, kobold::military_unit_domain> enum_converter<kobold::military_unit_domain>::string_to_enum_map = {
	{ "none", kobold::military_unit_domain::none },
	{ "land", kobold::military_unit_domain::land },
	{ "water", kobold::military_unit_domain::water },
	{ "air", kobold::military_unit_domain::air }
};

template <>
const bool enum_converter<kobold::military_unit_domain>::initialized = enum_converter::initialize();

}
