#include "kobold.h"

#include "country/diplomacy_state.h"

namespace archimedes {

template class enum_converter<kobold::diplomacy_state>;

template <>
const std::string enum_converter<kobold::diplomacy_state>::property_class_identifier = "kobold::diplomacy_state";

template <>
const std::map<std::string, kobold::diplomacy_state> enum_converter<kobold::diplomacy_state>::string_to_enum_map = {
	{ "peace", kobold::diplomacy_state::peace },
	{ "alliance", kobold::diplomacy_state::alliance },
	{ "war", kobold::diplomacy_state::war },
	{ "non_aggression_pact", kobold::diplomacy_state::non_aggression_pact },
	{ "vassal", kobold::diplomacy_state::vassal },
	{ "overlord", kobold::diplomacy_state::overlord }
};

template <>
const bool enum_converter<kobold::diplomacy_state>::initialized = enum_converter::initialize();

}
