#include "kobold.h"

#include "character/advisor_category.h"

namespace archimedes {

template class enum_converter<kobold::advisor_category>;

template <>
const std::string enum_converter<kobold::advisor_category>::property_class_identifier = "kobold::advisor_category";

template <>
const std::map<std::string, kobold::advisor_category> enum_converter<kobold::advisor_category>::string_to_enum_map = {
	{ "none", kobold::advisor_category::none },
	{ "trade", kobold::advisor_category::trade },
	{ "exploration", kobold::advisor_category::exploration },
	{ "military", kobold::advisor_category::military },
	{ "political", kobold::advisor_category::political },
	{ "religious", kobold::advisor_category::religious }
};

template <>
const bool enum_converter<kobold::advisor_category>::initialized = enum_converter::initialize();

}
