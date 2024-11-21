#include "kobold.h"

#include "item/affix_type.h"

namespace archimedes {

template class enum_converter<kobold::affix_type>;

template <>
const std::string enum_converter<kobold::affix_type>::property_class_identifier = "kobold::affix_type";

template <>
const std::map<std::string, kobold::affix_type> enum_converter<kobold::affix_type>::string_to_enum_map = {
	{ "prefix", kobold::affix_type::prefix },
	{ "suffix", kobold::affix_type::suffix },
	{ "stem", kobold::affix_type::stem }
};

template <>
const bool enum_converter<kobold::affix_type>::initialized = enum_converter::initialize();

}
