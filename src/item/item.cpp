#include "kobold.h"

#include "item/item.h"

#include "item/item_type.h"
#include "util/assert_util.h"

namespace kobold {

item::item(const item_type *type) : type(type)
{
	assert_throw(this->get_type() != nullptr);

	this->update_name();
}

void item::update_name()
{
	this->set_name(this->get_type()->get_name());
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
