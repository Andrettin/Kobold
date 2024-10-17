#include "kobold.h"

#include "unit/transporter_stat.h"

namespace archimedes {

template class enum_converter<kobold::transporter_stat>;

template <>
const std::string enum_converter<kobold::transporter_stat>::property_class_identifier = "kobold::transporter_stat";

template <>
const std::map<std::string, kobold::transporter_stat> enum_converter<kobold::transporter_stat>::string_to_enum_map = {
	{ "defense", kobold::transporter_stat::defense },
	{ "resistance", kobold::transporter_stat::resistance },
	{ "movement", kobold::transporter_stat::movement }
};

template <>
const bool enum_converter<kobold::transporter_stat>::initialized = enum_converter::initialize();

}
