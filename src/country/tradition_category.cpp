#include "kobold.h"

#include "country/tradition_category.h"

namespace archimedes {

template class enum_converter<kobold::tradition_category>;

template <>
const std::string enum_converter<kobold::tradition_category>::property_class_identifier = "kobold::tradition_category";

template <>
const std::map<std::string, kobold::tradition_category> enum_converter<kobold::tradition_category>::string_to_enum_map = {
	{ "none", kobold::tradition_category::none },
	{ "tradition", kobold::tradition_category::tradition },
	{ "belief", kobold::tradition_category::belief }
};

template <>
const bool enum_converter<kobold::tradition_category>::initialized = enum_converter::initialize();

}
