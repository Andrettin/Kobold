#include "kobold.h"

#include "spell/spell_target.h"

namespace archimedes {

template class enum_converter<kobold::spell_target>;

template <>
const std::string enum_converter<kobold::spell_target>::property_class_identifier = "kobold::spell_target";

template <>
const std::map<std::string, kobold::spell_target> enum_converter<kobold::spell_target>::string_to_enum_map = {
	{ "none", kobold::spell_target::none },
	{ "enemy", kobold::spell_target::enemy },
	{ "ally", kobold::spell_target::ally }
};

template <>
const bool enum_converter<kobold::spell_target>::initialized = enum_converter::initialize();

}
