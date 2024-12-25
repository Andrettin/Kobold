#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class event_trigger {
	none,
	per_turn_pulse,
	quarterly_pulse,
	yearly_pulse,
	ruler_death,
	ruin_explored
};

}

extern template class archimedes::enum_converter<kobold::event_trigger>;

Q_DECLARE_METATYPE(kobold::event_trigger)
