#include "kobold.h"

#include "character/personality.h"

namespace kobold {

void personality::check() const
{
	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Personality \"{}\" has no icon.", this->get_identifier()));
	}

	if (this->get_opposite_name().empty()) {
		throw std::runtime_error(std::format("Personality \"{}\" has no opposite name.", this->get_identifier()));
	}
}

}
