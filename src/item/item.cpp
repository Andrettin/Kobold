#include "kobold.h"

#include "item/item.h"

#include "item/affix_type.h"
#include "item/enchantment.h"
#include "item/item_type.h"
#include "util/assert_util.h"

namespace kobold {

item::item(const item_type *type, const kobold::enchantment *enchantment)
	: type(type), enchantment(enchantment)
{
	assert_throw(this->get_type() != nullptr);

	this->update_name();
}

void item::update_name()
{
	std::string name = this->get_type()->get_name();

	if (this->get_enchantment() != nullptr) {
		switch (this->get_enchantment()->get_affix_type()) {
			case affix_type::prefix:
				name = this->get_enchantment()->get_name() + " " + name;
				break;
			case affix_type::suffix:
				name += " " + this->get_enchantment()->get_name();
				break;
			case affix_type::stem:
				name = this->get_enchantment()->get_name();
				break;
		}
	}

	this->set_name(name);
}

const item_slot *item::get_slot() const
{
	return this->get_type()->get_slot();
}

const icon *item::get_icon() const
{
	return this->get_type()->get_icon();
}

}
