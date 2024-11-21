#include "kobold.h"

#include "item/enchantment.h"

#include "item/affix_type.h"
#include "item/item_type.h"
#include "script/modifier.h"

namespace kobold {

enchantment::enchantment(const std::string &identifier) : named_data_entry(identifier)
{
}

enchantment::~enchantment()
{
}

void enchantment::process_gsml_scope(const gsml_data &scope)
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

void enchantment::check() const
{
	if (this->get_affix_type() == affix_type::none) {
		throw std::runtime_error(std::format("Enchantment \"{}\" has no affix type.", this->get_identifier()));
	}

	if (this->get_item_types().empty()) {
		throw std::runtime_error(std::format("Enchantment \"{}\" has no item types.", this->get_identifier()));
	}
}

}
