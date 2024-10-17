#include "kobold.h"

#include "technology/technology_category.h"

namespace archimedes {

template class enum_converter<kobold::technology_category>;

template <>
const std::string enum_converter<kobold::technology_category>::property_class_identifier = "kobold::technology_category";

template <>
const std::map<std::string, kobold::technology_category> enum_converter<kobold::technology_category>::string_to_enum_map = {
	{ "none", kobold::technology_category::none },
	{ "gathering", kobold::technology_category::gathering },
	{ "industry", kobold::technology_category::industry },
	{ "army", kobold::technology_category::army },
	{ "navy", kobold::technology_category::navy },
	{ "finance", kobold::technology_category::finance },
	{ "culture", kobold::technology_category::culture }
};

template <>
const bool enum_converter<kobold::technology_category>::initialized = enum_converter::initialize();

}
