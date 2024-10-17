#include "kobold.h"

#include "character/character_attribute.h"

namespace archimedes {

template class enum_converter<kobold::character_attribute>;

template <>
const std::string enum_converter<kobold::character_attribute>::property_class_identifier = "kobold::character_attribute";

template <>
const std::map<std::string, kobold::character_attribute> enum_converter<kobold::character_attribute>::string_to_enum_map = {
	{ "none", kobold::character_attribute::none },
	{ "diplomacy", kobold::character_attribute::diplomacy },
	{ "martial", kobold::character_attribute::martial },
	{ "stewardship", kobold::character_attribute::stewardship },
	{ "intrigue", kobold::character_attribute::intrigue },
	{ "learning", kobold::character_attribute::learning }
};

template <>
const bool enum_converter<kobold::character_attribute>::initialized = enum_converter::initialize();

}
