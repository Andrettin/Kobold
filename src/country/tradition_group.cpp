#include "kobold.h"

#include "country/tradition_group.h"

#include "country/tradition.h"

namespace kobold {

void tradition_group::check() const
{
	if (this->get_traditions().empty()) {
		throw std::runtime_error(std::format("Tradition group \"{}\" has no traditions.", this->get_identifier()));
	}
}

}
