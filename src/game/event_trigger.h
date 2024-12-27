#pragma once

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

Q_DECLARE_METATYPE(kobold::event_trigger)
