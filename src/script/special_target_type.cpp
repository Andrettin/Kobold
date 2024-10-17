#include "kobold.h"

#include "script/special_target_type.h"

namespace archimedes {

template class enum_converter<kobold::special_target_type>;

template <>
const std::string enum_converter<kobold::special_target_type>::property_class_identifier = "kobold::special_target_type";

template <>
const std::map<std::string, kobold::special_target_type> enum_converter<kobold::special_target_type>::string_to_enum_map = {
	{ "root", kobold::special_target_type::root },
	{ "source", kobold::special_target_type::source },
	{ "previous", kobold::special_target_type::previous }
};

template <>
const bool enum_converter<kobold::special_target_type>::initialized = enum_converter::initialize();

}
