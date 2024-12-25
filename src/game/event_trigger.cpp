#include "kobold.h"

#include "game/event_trigger.h"

namespace archimedes {

template class enum_converter<kobold::event_trigger>;

template <>
const std::string enum_converter<kobold::event_trigger>::property_class_identifier = "kobold::event_trigger";

template <>
const std::map<std::string, kobold::event_trigger> enum_converter<kobold::event_trigger>::string_to_enum_map = {
	{ "none", kobold::event_trigger::none },
	{ "per_turn_pulse", kobold::event_trigger::per_turn_pulse },
	{ "quarterly_pulse", kobold::event_trigger::quarterly_pulse },
	{ "yearly_pulse", kobold::event_trigger::yearly_pulse },
	{ "ruler_death", kobold::event_trigger::ruler_death },
	{ "ruin_explored", kobold::event_trigger::ruin_explored }
};

template <>
const bool enum_converter<kobold::event_trigger>::initialized = enum_converter::initialize();

}
