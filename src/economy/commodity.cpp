#include "kobold.h"

#include "economy/commodity.h"

#include "economy/commodity_unit.h"
#include "util/assert_util.h"
#include "util/log_util.h"

namespace kobold {

commodity::commodity(const std::string &identifier)
	: named_data_entry(identifier)
{
}

void commodity::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "units") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			const int unit_value = std::stoi(key);
			const commodity_unit *unit = commodity_unit::get(value);

			this->units[unit_value] = unit;
			this->unit_values[unit] = unit_value;
		});
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void commodity::check() const
{
	assert_throw(this->get_icon() != nullptr);
	assert_throw(this->get_wealth_value() >= 0);

	if (this->is_local()) {
		if (!this->is_abstract()) {
			throw std::runtime_error(std::format("Commodity \"{}\" is local but is not abstract, which is not supported.", this->get_identifier()));
		}

		if (this->is_storable()) {
			throw std::runtime_error(std::format("Commodity \"{}\" is both local and storable, which is not supported.", this->get_identifier()));
		}
	}

	if (!this->is_abstract() && this->is_storable() && this->get_base_price() == 0 && this->get_wealth_value() == 0) {
		throw std::runtime_error(std::format("Non-abstract storable commodity \"{}\" has neither a base price nor a wealth value.", this->get_identifier()));
	}

	if (this->get_base_price() != 0 && this->get_wealth_value() != 0) {
		throw std::runtime_error(std::format("Commodity \"{}\" has both a base price and a wealth value.", this->get_identifier()));
	}
}

int commodity::get_unit_value(const commodity_unit *unit) const
{
	const auto find_iterator = this->unit_values.find(unit);

	if (find_iterator != this->unit_values.end()) {
		return find_iterator->second;
	}

	log::log_error(std::format("Commodity \"{}\" has no value for unit \"{}\".", this->get_identifier(), unit->get_identifier()));
	return 1;
}

int commodity::string_to_value(const std::string &str) const
{
	size_t suffix_pos = std::string::npos;

	for (size_t i = 0; i < str.size(); ++i) {
		const char c = str[i];

		if (i == 0 && c == '-') {
			continue;
		}

		if (std::isdigit(c)) {
			continue;
		}

		suffix_pos = i;
		break;
	}

	if (suffix_pos == std::string::npos) {
		return std::stoi(str);
	}

	const std::string number_str = str.substr(0, suffix_pos);
	const std::string suffix = str.substr(suffix_pos);
	const commodity_unit *unit = commodity_unit::get(suffix);
	return std::stoi(number_str) * this->get_unit_value(unit);
}


}
