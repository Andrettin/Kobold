#include "kobold.h"

#include "character/skill.h"

namespace kobold {

void skill::check() const
{
	if (this->get_attribute() == nullptr) {
		throw std::runtime_error(std::format("Saving throw type \"{}\" has no attribute.", this->get_identifier()));
	}
}

}
