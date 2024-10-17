#include "kobold.h"

#include "country/country_type.h"

namespace archimedes {

template class enum_converter<kobold::country_type>;

template <>
const std::string enum_converter<kobold::country_type>::property_class_identifier = "kobold::country_type";

template <>
const std::map<std::string, kobold::country_type> enum_converter<kobold::country_type>::string_to_enum_map = {
	{ "great_power", kobold::country_type::great_power },
	{ "minor_nation", kobold::country_type::minor_nation },
	{ "tribe", kobold::country_type::tribe }
};

template <>
const bool enum_converter<kobold::country_type>::initialized = enum_converter::initialize();

}
