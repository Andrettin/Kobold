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

std::pair<std::string, const commodity_unit *> commodity::string_to_number_string_and_unit(const std::string &str) const
{
	size_t suffix_pos = std::string::npos;
	bool has_suffix = false;

	for (int i = (static_cast<int>(str.size()) - 1); i >= 0; --i) {
		const char c = str[i];

		if (!std::isdigit(c)) {
			has_suffix = true;
			continue;
		}

		if (!has_suffix) {
			break;
		}

		suffix_pos = i + 1;
		break;
	}

	if (suffix_pos == std::string::npos) {
		return { str, nullptr };
	}

	const std::string number_str = str.substr(0, suffix_pos);
	const std::string suffix = str.substr(suffix_pos);
	const commodity_unit *unit = commodity_unit::get(suffix);

	return { number_str, unit };
}

int commodity::string_to_value(const std::string &str) const
{
	const auto [number_str, unit] = this->string_to_number_string_and_unit(str);
	int value = std::stoi(number_str);
	if (unit != nullptr) {
		value *= this->get_unit_value(unit);
	}
	return value;
}

std::pair<std::variant<int, dice>, const commodity_unit *> commodity::string_to_value_variant_with_unit(const std::string &str) const
{
	const auto [number_str, unit] = this->string_to_number_string_and_unit(str);

	if (number_str.find("d") != std::string::npos) {
		return { dice(number_str), unit };
	} else {
		return { std::stoi(number_str), unit };
	}
}

}
