#include "kobold.h"

#include "country/country_tier.h"

namespace archimedes {

template class enum_converter<kobold::country_tier>;

template <>
const std::string enum_converter<kobold::country_tier>::property_class_identifier = "kobold::country_tier";

template <>
const std::map<std::string, kobold::country_tier> enum_converter<kobold::country_tier>::string_to_enum_map = {
	{ "barony", kobold::country_tier::barony },
	{ "county", kobold::country_tier::county },
	{ "duchy", kobold::country_tier::duchy },
	{ "kingdom", kobold::country_tier::kingdom },
	{ "empire", kobold::country_tier::empire }
};

template <>
const bool enum_converter<kobold::country_tier>::initialized = enum_converter::initialize();

}
