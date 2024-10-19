#include "kobold.h"

#include "character/character_role.h"

namespace archimedes {

template class enum_converter<kobold::character_role>;

template <>
const std::string enum_converter<kobold::character_role>::property_class_identifier = "kobold::character_role";

template <>
const std::map<std::string, kobold::character_role> enum_converter<kobold::character_role>::string_to_enum_map = {
	{ "none", kobold::character_role::none },
	{ "ruler", kobold::character_role::ruler },
	{ "leader", kobold::character_role::leader },
	{ "civilian", kobold::character_role::civilian }
};

template <>
const bool enum_converter<kobold::character_role>::initialized = enum_converter::initialize();

}
