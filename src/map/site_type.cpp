#include "kobold.h"

#include "map/site_type.h"

namespace archimedes {

template class enum_converter<kobold::site_type>;

template <>
const std::string enum_converter<kobold::site_type>::property_class_identifier = "kobold::site_type";

template <>
const std::map<std::string, kobold::site_type> enum_converter<kobold::site_type>::string_to_enum_map = {
	{ "none", kobold::site_type::none },
	{ "settlement", kobold::site_type::settlement },
	{ "terrain", kobold::site_type::terrain },
	{ "resource", kobold::site_type::resource }
};

template <>
const bool enum_converter<kobold::site_type>::initialized = enum_converter::initialize();

}
