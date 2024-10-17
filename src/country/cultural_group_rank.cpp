#include "kobold.h"

#include "country/cultural_group_rank.h"

namespace archimedes {

template class enum_converter<kobold::cultural_group_rank>;

template <>
const std::string enum_converter<kobold::cultural_group_rank>::property_class_identifier = "kobold::cultural_group_rank";

template <>
const std::map<std::string, kobold::cultural_group_rank> enum_converter<kobold::cultural_group_rank>::string_to_enum_map = {
	{ "none", kobold::cultural_group_rank::none },
	{ "infragroup", kobold::cultural_group_rank::infragroup },
	{ "subgroup", kobold::cultural_group_rank::subgroup },
	{ "group", kobold::cultural_group_rank::group },
	{ "supergroup", kobold::cultural_group_rank::supergroup }
};

template <>
const bool enum_converter<kobold::cultural_group_rank>::initialized = enum_converter::initialize();

}
