#include "kobold.h"

#include "unit/transporter_category.h"

namespace archimedes {

template class enum_converter<kobold::transporter_category>;

template <>
const std::string enum_converter<kobold::transporter_category>::property_class_identifier = "kobold::transporter_category";

template <>
const std::map<std::string, kobold::transporter_category> enum_converter<kobold::transporter_category>::string_to_enum_map = {
	{ "land_transporter", kobold::transporter_category::land_transporter },
	{ "small_merchant_ship", kobold::transporter_category::small_merchant_ship },
	{ "large_merchant_ship", kobold::transporter_category::large_merchant_ship }
};

template <>
const bool enum_converter<kobold::transporter_category>::initialized = enum_converter::initialize();

}
