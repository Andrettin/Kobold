#include "kobold.h"

#include "country/office.h"

#include "character/character_attribute.h"
#include "country/country_attribute.h"
#include "script/condition/and_condition.h"
#include "util/assert_util.h"

namespace kobold {

office::office(const std::string &identifier) : named_data_entry(identifier)
{
}

office::~office()
{
}

void office::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "character_attribute") {
		this->character_attributes.push_back(character_attribute::get(value));
	} else if (key == "country_attribute") {
		this->country_attributes.push_back(country_attribute::get(value));
	} else {
		data_entry::process_gsml_property(property);
	}
}

void office::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "character_attributes") {
		for (const std::string &value : values) {
			this->character_attributes.push_back(character_attribute::get(value));
		}
	} else if (tag == "country_attributes") {
		for (const std::string &value : values) {
			this->country_attributes.push_back(country_attribute::get(value));
		}
	} else if (tag == "conditions") {
		auto conditions = std::make_unique<and_condition<character>>();
		database::process_gsml_data(conditions, scope);
		this->conditions = std::move(conditions);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void office::check() const
{
	assert_throw(!this->get_character_attributes().empty());
	assert_throw(!this->get_country_attributes().empty());
}

}
