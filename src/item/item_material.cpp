#include "kobold.h"

#include "item/item_material.h"

#include "item/item_type.h"
#include "script/modifier.h"

namespace kobold {

item_material::item_material(const std::string &identifier) : named_data_entry(identifier)
{
}

item_material::~item_material()
{
}

void item_material::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "item_types") {
		for (const std::string &value : values) {
			this->item_types.insert(item_type::get(value));
		}
	} else if (tag == "modifier") {
		auto modifier = std::make_unique<kobold::modifier<const character>>();
		database::process_gsml_data(modifier, scope);
		this->modifier = std::move(modifier);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void item_material::check() const
{
	if (this->get_item_types().empty()) {
		throw std::runtime_error(std::format("Item material \"{}\" has no item types.", this->get_identifier()));
	}
}

}
