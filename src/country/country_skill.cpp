#include "kobold.h"

#include "country/country_skill.h"

namespace kobold {

void country_skill::check() const
{
	if (this->get_attribute() == nullptr) {
		throw std::runtime_error(std::format("Country skill \"{}\" has no attribute.", this->get_identifier()));
	}

	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Country skill \"{}\" has no icon.", this->get_identifier()));
	}
}

}
