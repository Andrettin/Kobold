#include "kobold.h"

#include "infrastructure/improvement_slot.h"

namespace archimedes {

template class enum_converter<kobold::improvement_slot>;

template <>
const std::string enum_converter<kobold::improvement_slot>::property_class_identifier = "kobold::improvement_slot";

template <>
const std::map<std::string, kobold::improvement_slot> enum_converter<kobold::improvement_slot>::string_to_enum_map = {
	{ "none", kobold::improvement_slot::none },
	{ "main", kobold::improvement_slot::main },
	{ "resource", kobold::improvement_slot::resource },
	{ "depot", kobold::improvement_slot::depot },
	{ "port", kobold::improvement_slot::port }
};

template <>
const bool enum_converter<kobold::improvement_slot>::initialized = enum_converter::initialize();

}
