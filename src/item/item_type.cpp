#include "kobold.h"

#include "item/item_type.h"

#include "item/item_slot.h"
#include "script/modifier.h"

namespace kobold {

item_type::item_type(const std::string &identifier) : named_data_entry(identifier)
{
}

item_type::~item_type()
{
}

void item_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier") {
		auto modifier = std::make_unique<kobold::modifier<const character>>();
		database::process_gsml_data(modifier, scope);
		this->modifier = std::move(modifier);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void item_type::check() const
{
	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Item type \"{}\" has no icon.", this->get_identifier()));
	}
}

}
