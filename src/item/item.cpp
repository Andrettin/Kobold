#include "kobold.h"

#include "item/item.h"

#include "item/item_slot.h"

namespace kobold {

void item::check() const
{
	if (this->get_slot() == nullptr) {
		throw std::runtime_error(std::format("Item \"{}\" has no slot.", this->get_identifier()));
	}

	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Item \"{}\" has no icon.", this->get_identifier()));
	}
}

}
