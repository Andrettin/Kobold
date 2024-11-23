#include "kobold.h"

#include "item/item_class.h"

#include "item/item_slot.h"

namespace kobold {

item_class::item_class(const std::string &identifier) : named_data_entry(identifier)
{
}

item_class::~item_class()
{
}

}
