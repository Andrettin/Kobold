#include "kobold.h"

#include "character/skill_category.h"

namespace kobold {

void skill_category::check() const
{
	if (this->get_attribute() == nullptr) {
		throw std::runtime_error(std::format("Skill category \"{}\" has no attribute.", this->get_identifier()));
	}

	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Skill category \"{}\" has no icon.", this->get_identifier()));
	}

	if (this->get_skills().empty()) {
		throw std::runtime_error(std::format("Skill category \"{}\" has no skills.", this->get_identifier()));
	}
}

}
